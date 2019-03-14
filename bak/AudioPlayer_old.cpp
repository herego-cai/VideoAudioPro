#include "AudioPlayer.h"
#include <QDateTime>
#include <QDebug>

#define SDL_AUDIO_MIN_BUFFER_SIZE 512
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30

AudioPlayer::AudioPlayer(QObject *parent) : QThread(parent)
{
    mStreamUri = "";
}

Uint8* AudioPlayer::mSrcData = NULL;
int AudioPlayer::mSrcLen = 0;

AVRational AudioPlayer::mTimeBase;
int AudioPlayer::mBaseAudioPts = 0;
double AudioPlayer::mAudioPts = 0;
double AudioPlayer::mAudioStamp = 0;
int AudioPlayer::mPlayLen = 0;
double AudioPlayer::mBytesPerSecond = 0;
QQueue *AudioPlayer::mFrameQueue = new QQueue();

void AudioPlayer::audioCallBack(void *userData,Uint8 *stream,int len)
{
    qDebug()<<"len:"<<len;
//    qDebug()<<"mSrcLen:"<<mSrcLen;
    memset(stream,0,len);
    if(mSrcLen == 0 || mSrcLen == NULL)return;
    len = mSrcLen>len?len:mSrcLen;

    qDebug()<<"---len:"<<len;

    memcpy(stream, mSrcData, len);
//    SDL_MixAudio(stream,mSrcData,len,SDL_MIX_MAXVOLUME);
//    SDL_MixAudioFormat(stream,mSrcData,AUDIO_S16SYS,len,SDL_MIX_MAXVOLUME);
    mAudioPts = mPlayLen/mBytesPerSecond+(mBaseAudioPts*av_q2d(mTimeBase));
    mAudioStamp = QDateTime::currentDateTime().toTime_t();
//    qDebug()<<"mAudioPts:"<<mAudioPts;
//    qDebug("mAudioStamp:%lf",mAudioStamp);
    mPlayLen += len;
    mSrcData += len;
    mSrcLen -= len;
}

void AudioPlayer::setStreamUri(QString streamUri)
{
    mStreamUri = streamUri;
}

void AudioPlayer::startPlay(QString streamUri)
{
    if(streamUri != ""){
        setStreamUri(streamUri);
    }

    if(mStreamUri == ""){
        qDebug()<<"mStreamUri is null";
        return;
    }

    //1.打开输入流
    mFormatContext = avformat_alloc_context();
    if(avformat_open_input(&mFormatContext,mStreamUri.toStdString().data(),NULL,NULL) < 0){
        qDebug()<<"avformat_open_input failure";
        return;
    }

    //4.找到输入流
    if(avformat_find_stream_info(mFormatContext,NULL) < 0){
        qDebug()<<"avformat_find_stream_info failure";
        return;
    }

    //5.遍历并找到音频流
    mAudioIndex = -1;
    for(int i=0;i < mFormatContext->nb_streams;++i){
        if(mFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
            mAudioIndex = i;
            break;
        }
    }
    if(mAudioIndex == -1){
        qDebug()<<"Don't find audio data";
        avformat_close_input(&mFormatContext);
        return;
    }

    mTimeBase.den = mFormatContext->streams[mAudioIndex]->time_base.den;
    mTimeBase.num = mFormatContext->streams[mAudioIndex]->time_base.num;

    //6.读取编解码器上下文，并找到解码器
    mCodecContext = mFormatContext->streams[mAudioIndex]->codec;
    AVCodec *codec = avcodec_find_decoder(mCodecContext->codec_id);
    if(codec == NULL){
        qDebug()<<"Don't find audio decoder";
        avformat_close_input(&mFormatContext);
        return;
    }

    //7.打开解码器
    if(avcodec_open2(mCodecContext,codec,NULL) < 0){
        qDebug()<<"open audio decoder failure";
        avformat_close_input(&mFormatContext);
        return;
    }

    //SDL处理
    mWantAudioSpec.freq = 44100;//采样率 static const int next_sample_rates[] = {0, 44100, 48000, 96000, 192000};
    mWantAudioSpec.channels = 2;//通道数static const int next_nb_channels[] = {0, 0, 1, 6, 2, 6, 4, 6};
    mWantAudioSpec.silence = 0;//是否禁音
    mWantAudioSpec.samples = 1024;//FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE, 2<<av_log2(mWantAudioSpec.freq/SDL_AUDIO_MAX_CALLBACKS_PER_SEC));//帧大小 AAC:1024  MP3:1152
    mWantAudioSpec.format = AUDIO_S16SYS;//采样格式
    mWantAudioSpec.callback = audioCallBack;//回调函数
    mWantAudioSpec.userdata = mCodecContext;

    //32位%llu,64位%lu就行
    qDebug("mWantAudioSpec.samples:%u",mWantAudioSpec.samples);
    qDebug("mWantAudioSpec.samples:%u",mWantAudioSpec.freq/SDL_AUDIO_MAX_CALLBACKS_PER_SEC);
    qDebug("mWantAudioSpec.samples:%u",av_log2(mWantAudioSpec.freq/SDL_AUDIO_MAX_CALLBACKS_PER_SEC));
    qDebug("mWantAudioSpec.samples:%u",2<<av_log2(mWantAudioSpec.freq/SDL_AUDIO_MAX_CALLBACKS_PER_SEC));

    mBytesPerSecond = mWantAudioSpec.freq * mWantAudioSpec.channels * 2;

    if(SDL_OpenAudio(&mWantAudioSpec, NULL)<0){
        qDebug("SDL_OpenAudio failure %s",SDL_GetError());
        avformat_close_input(&mFormatContext);
        return;
    }

    SDL_PauseAudio(0);

    //8.创建想要的解码格式方便播放
    mSwrContext = swr_alloc();
    swr_alloc_set_opts(mSwrContext,AV_CH_LAYOUT_STEREO,AV_SAMPLE_FMT_S16,mWantAudioSpec.freq,
                       av_get_default_channel_layout(mCodecContext->channels),mCodecContext->sample_fmt,mCodecContext->sample_rate
                       ,0,NULL);
    swr_init(mSwrContext);

    qDebug("mCodecContext->time_base:%lf",av_q2d(mCodecContext->time_base));
    qDebug("mTimeBase:%lf",av_q2d(mTimeBase));

    this->start();
}

void AudioPlayer::run()
{
    //9.读取流信息
    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    AVFrame *frame = av_frame_alloc();
    int frameSize = 0;
    Uint8 *audioData = (Uint8 *)av_malloc(MAX_AUDIO_SIZE_PER_SECOND*2);
    int out_buffer_size=av_samples_get_buffer_size(NULL,mWantAudioSpec.channels ,mWantAudioSpec.samples,AV_SAMPLE_FMT_S16, 1);
    qDebug()<<"out_buffer_size:"<<out_buffer_size;
    if(out_buffer_size <= 0){
        qDebug()<<"av_samples_get_buffer_size failure";
        return;
    }

    mAudioStamp = QDateTime::currentDateTime().toTime_t();
    mBaseAudioPts = 0;
    while(av_read_frame(mFormatContext,packet)>=0){
        if(packet->stream_index == mAudioIndex){
            if(avcodec_decode_audio4(mCodecContext,frame,&frameSize,packet) < 0){
                qDebug()<<"avcodec_decode_audio4 failure";
                break;
            }

            if(frameSize > 0){
                qDebug()<<"======================================";
                mBaseAudioPts = packet->pts;

//                if(mWantAudioSpec.samples != frame->nb_samples){
//                    swr_set_compensation(mSwrContext, (mWantAudioSpec.samples - frame->nb_samples) * mWantAudioSpec.freq / frame->sample_rate,
//                                                            mWantAudioSpec.samples * mWantAudioSpec.freq / frame->sample_rate);
//                }

                //当前音频的数据
                int out_count = (int64_t)frame->nb_samples * mWantAudioSpec.freq / frame->sample_rate + 256;
                int out_size  = av_samples_get_buffer_size(NULL, mWantAudioSpec.channels, out_count, AV_SAMPLE_FMT_S16, 0);
                if (out_size < 0) {
                    qDebug("av_samples_get_buffer_size() failed");
                    return;
                }
                qDebug()<<"out_size:"<<out_size;

                unsigned int audio_buf1_size = 0;
                av_fast_malloc(&audioData, &audio_buf1_size, out_size);
                if (!audioData)
                    return;

                //解码数据处理
                int len2 = swr_convert(mSwrContext,&audioData,MAX_AUDIO_SIZE_PER_SECOND,(const Uint8 **)frame->data,frame->nb_samples);
                qDebug()<<"frame->nb_samples:"<<frame->nb_samples;
//                if(frame->nb_samples < mWantAudioSpec.samples){
//                    continue;
//                }


                qDebug()<<"mSrcLen:"<<len2 * 2 * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
                //等待数据播放完成
                while(mSrcLen > 0)SDL_Delay(10);
                mPlayLen = 0;

                //添加数据
                mSrcData = audioData;
                mSrcLen = len2 * 2 * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
            }
        }
    }

    SDL_CloseAudio();
    SDL_Quit();

    avformat_close_input(&mFormatContext);
    avcodec_close(mCodecContext);
    av_free(packet);
    av_frame_free(&frame);
}
