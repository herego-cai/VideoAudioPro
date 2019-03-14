#include "PCMAudioPlayer.h"
#include <QDateTime>
#include <QDebug>

#define SDL_AUDIO_MIN_BUFFER_SIZE 512
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30

PCMAudioPlayer::PCMAudioPlayer(QObject *parent) : QThread(parent)
{
    mStreamUri = "";
}

Uint8* PCMAudioPlayer::mSrcData = NULL;
int PCMAudioPlayer::mSrcLen = 0;
int PCMAudioPlayer::mPlayedLen = 0;

AVRational PCMAudioPlayer::mTimeBase;
int PCMAudioPlayer::mBaseAudioPts = 0;
double PCMAudioPlayer::mAudioPts = 0;
double PCMAudioPlayer::mAudioStamp = 0;
double PCMAudioPlayer::mBytesPerSecond = 0;

QQueue<AVPacket*> *PCMAudioPlayer::mFrameQueue = new QQueue<AVPacket*>();
AVCodecContext *PCMAudioPlayer::mCodecContext = NULL;
SwrContext *PCMAudioPlayer::mSwrContext = NULL;

void PCMAudioPlayer::audioCallBack(void *,Uint8 *stream,int len)
{
    memset(stream,0,len);

    //注意是static
    //为什么要分那么大？
    static uint8_t audio_buf[(MAX_AUDIO_SIZE_PER_SECOND * 3) / 2];
    while(len > 0){
        if(mPlayedLen >= mSrcLen){
            //重新读取包数据
            //数据全部发送，再去获取
            //自定义的一个函数
            int audio_size = audio_decode_frame(audio_buf, sizeof(audio_buf));
//            qDebug()<<"audio_size:"<<audio_size;
            if (audio_size < 0){
                //错误则静音
                mSrcLen = 1024;
                memset(audio_buf, 0, mSrcLen);
            }else{
                mSrcLen = audio_size;
            }
            mPlayedLen = 0;      //回到缓冲区开头
        }

        //播放长度
        int leaveLen = mSrcLen - mPlayedLen;//剩下播放长度
        int playLen = leaveLen>len?len:leaveLen;
//        qDebug()<<"---playLen:"<<playLen;

//        memcpy(stream, mSrcData+mPlayedLen, playLen);//如果声音大小是SDL_MIX_MAXVOLUME就等价于SDL_MixAudioFormat(stream,mSrcData,AUDIO_S16SYS,len,SDL_MIX_MAXVOLUME);
//    //    SDL_MixAudio(stream,mSrcData,len,SDL_MIX_MAXVOLUME);
        SDL_MixAudioFormat(stream,audio_buf+mPlayedLen,AUDIO_S16SYS,playLen,SDL_MIX_MAXVOLUME);

        //同步使用
        mAudioPts = mPlayedLen/mBytesPerSecond+(mBaseAudioPts*av_q2d(mTimeBase));
        mAudioStamp = QDateTime::currentDateTime().toTime_t();

//        //播放长度更新
        len -= playLen;
        stream += playLen;
        mPlayedLen += playLen;
    }
}

int PCMAudioPlayer::audio_decode_frame(uint8_t *audioData, int )
{
    int        got_frame;
    if(mFrameQueue->isEmpty())
    {
        qDebug("mFrameQueue is Empty\n");
        return -1;
    }
    AVPacket *packet = mFrameQueue->dequeue();
    AVFrame *frame = av_frame_alloc();

//    qDebug()<<"packet->size:"<<packet->size;
    int audio_buf_index  = 0;
    int  convert_len = 0;
    int  convert_all = 0;
    int packetSize = packet->size;
    while(packetSize > 0)
    {
        if(avcodec_decode_audio4(mCodecContext,frame,&got_frame,packet) < 0){
            qDebug()<<"avcodec_decode_audio4 failure";
            break;
        }

        if (got_frame){
            mBaseAudioPts = packet->pts;

            //解码数据处理
            convert_len = swr_convert(mSwrContext,&audioData+audio_buf_index,MAX_AUDIO_SIZE_PER_SECOND,(const Uint8 **)frame->data,frame->nb_samples);

            //添加数据
            packetSize -= convert_len;
            audio_buf_index += convert_len;//data_size;
            convert_all += convert_len;
        }
    }
    av_free_packet(packet);
    return convert_all * 2 * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
}

void PCMAudioPlayer::setStreamUri(QString streamUri)
{
    mStreamUri = streamUri;
}

void PCMAudioPlayer::startPlay(QString streamUri)
{
    if(streamUri != ""){
        setStreamUri(streamUri);
    }

    if(mStreamUri == ""){
        qDebug()<<"mStreamUri is null";
        return;
    }

    //SDL处理
    mWantAudioSpec.freq = 32000;//采样率 static const int next_sample_rates[] = {0, 44100, 48000, 96000, 192000};
    mWantAudioSpec.channels = 1;//通道数static const int next_nb_channels[] = {0, 0, 1, 6, 2, 6, 4, 6};
    mWantAudioSpec.silence = 0;//是否禁音
    mWantAudioSpec.samples = 1024;//FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE, 2<<av_log2(mWantAudioSpec.freq/SDL_AUDIO_MAX_CALLBACKS_PER_SEC));//帧大小 AAC:1024  MP3:1152
    mWantAudioSpec.format = AUDIO_S16SYS;//采样格式
    mWantAudioSpec.callback = audioCallBack;//回调函数
    mWantAudioSpec.userdata = mCodecContext;

    mBytesPerSecond = mWantAudioSpec.freq * mWantAudioSpec.channels * 2;

    if(SDL_OpenAudio(&mWantAudioSpec, NULL)<0){
        qDebug("SDL_OpenAudio failure %s",SDL_GetError());
        return;
    }

    SDL_PauseAudio(0);

    this->start();
}

void PCMAudioPlayer::run()
{
    //9.读取流信息
    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    mAudioStamp = QDateTime::currentDateTime().toTime_t();
    mBaseAudioPts = 0;
    while(av_read_frame(mFormatContext,packet)>=0){
        if(packet->stream_index == mAudioIndex){
            if(mFrameQueue->size() >= 40){
                QThread::msleep(50);
            }
            mFrameQueue->enqueue(packet);
            packet = (AVPacket *)av_malloc(sizeof(AVPacket));
        }else{
            av_free_packet(packet);
        }
    }
}

