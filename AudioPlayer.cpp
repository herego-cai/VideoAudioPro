#include "AudioPlayer.h"
#include <QDateTime>
#include <QDebug>
#include <SDLPlayer.h>

#define SDL_AUDIO_MIN_BUFFER_SIZE 512
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30
#define AUDIO_CHANNEL_COUNT 1
#define AUDIO_SAMPLE 16000

AudioPlayer::AudioPlayer(QObject *parent) : QThread(parent)
{
    mStreamUri = "";
    mStreamFlag = false;
    mSdlIndex = -1;

    mCodecContext = NULL;
    mSwrContext = NULL;
}

void AudioPlayer::setStreamUri(QString streamUri)
{
    mStreamUri = streamUri;
}

void AudioPlayer::startPlay(QString streamUri)
{
    SDLPlayer *sdlPlayer= SDLPlayer::getSDLPlayer();
    if(!sdlPlayer->isOpenSDL()){
        qDebug()<<"sdl don't open";
        return;
    }

    if(streamUri != ""){
        setStreamUri(streamUri);
    }

    if(mStreamUri == ""){
        qDebug()<<"mStreamUri is null";
        return;
    }

    qDebug()<<"mStreamUri:"<<mStreamUri;

    //使用TCP连接打开RTSP，设置最大延迟时间
//    AVDictionary *avdic=NULL;
//    char option_key[]="rtsp_transport";
//    char option_value[]="tcp";
//    av_dict_set(&avdic,option_key,option_value,0);
//    char option_key2[]="max_delay";
//    char option_value2[]="0";
//    av_dict_set(&avdic,option_key2,option_value2,0);

    if(mStreamUri.contains("rtsp") || mStreamUri.contains("rtmp")){
        mStreamFlag = true;
    }else{
        mStreamFlag = false;
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

    printAVCodecCtx(mCodecContext);
    mBytesPerSecond = mWantAudioSpec.freq * mWantAudioSpec.channels * 2;

    //8.有播放开启转换器
    mSwrContext = swr_alloc();
    swr_alloc_set_opts(mSwrContext,av_get_default_channel_layout(sdlPlayer->getChannelCount()),AV_SAMPLE_FMT_S16P,sdlPlayer->getAudioSample(),
                       av_get_default_channel_layout(mCodecContext->channels),mCodecContext->sample_fmt,mCodecContext->sample_rate
                       ,0,NULL);
    swr_init(mSwrContext);

    mWantAudioFrame = av_frame_alloc();
    unsigned char *wantFrameData = (unsigned char*)av_malloc(av_samples_get_buffer_size( NULL, sdlPlayer->getChannelCount(), sdlPlayer->getAudioSample(), AV_SAMPLE_FMT_S16P, 0));
    av_samples_fill_arrays(mWantAudioFrame->data,mWantAudioFrame->linesize,wantFrameData,sdlPlayer->getChannelCount(), sdlPlayer->getAudioSample(), AV_SAMPLE_FMT_S16P,0);
    mWantAudioFrame->channels = sdlPlayer->getChannelCount();
    mWantAudioFrame->sample_rate = sdlPlayer->getAudioSample();
    mWantAudioFrame->format = AV_SAMPLE_FMT_S16P;

    this->start();
}

void AudioPlayer::run()
{
    qDebug()<<"void AudioPlayer::run()";
    mStopFlag = false;
    int got_frame = -1;
    int convert_len = 0;
    AVFrame *frame = av_frame_alloc();

    //9.读取流信息
    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    mAudioStamp = QDateTime::currentDateTime().toTime_t();
    mBaseAudioPts = 0;
    while(!mStopFlag && av_read_frame(mFormatContext,packet)>=0){
        if(packet->stream_index == mAudioIndex){
            int ret = avcodec_decode_audio4(mCodecContext,frame,&got_frame,packet);
            if(ret < 0){
                qDebug()<<"avcodec_decode_audio4 failure errno:"<<ret;
                continue;
            }

            if (got_frame){
                convert_len = swr_convert(mSwrContext,(uint8_t **)mWantAudioFrame->data,mWantAudioFrame->linesize[0],(const Uint8 **)frame->data,frame->nb_samples);
                qDebug()<<"convert_len:"<<convert_len;
                if(convert_len > 0){
                    mWantAudioFrame->nb_samples = convert_len;
                    SdlDataModel sdlDataModel;
                    sdlDataModel.mSdlSize=mWantAudioFrame->nb_samples*2;
                    sdlDataModel.mSdlData = (unsigned char *)malloc(sdlDataModel.mSdlSize);
                    sdlDataModel.mAudioPts = mWantAudioFrame->pts;
                    sdlDataModel.mAudioSample = mWantAudioFrame->sample_rate;
                    memset(sdlDataModel.mSdlData,0,sdlDataModel.mSdlSize);
                    memcpy(sdlDataModel.mSdlData,mWantAudioFrame->data[0],sdlDataModel.mSdlSize);
                    qDebug()<<"mSdlIndex:"<<mSdlIndex;
                    mSdlIndex = SDLPlayer::getSDLPlayer()->addAudioSourceQueue(sdlDataModel,mSdlIndex);
                }
            }

            av_free_packet(packet);
        }else{
            av_free_packet(packet);
        }
    }

    qDebug()<<"---------audio thread quit";
    if(mFormatContext){
        avformat_close_input(&mFormatContext);
        mFormatContext = NULL;
    }

    if(mSwrContext){
        swr_close(mSwrContext);
        swr_free(&mSwrContext);
        mSwrContext = NULL;
    }

//    if(mCodecContext){
////        avcodec_close(mCodecContext);
//        avcodec_free_context(&mCodecContext);
//        mCodecContext = NULL;
//    }
    //删除
//    SDLPlayer::getSDLPlayer()->delAudioSourceQueue(mSdlIndex);
}

void AudioPlayer::stopPlay()
{
    if(!mStopFlag){
        mStopFlag = true;
        QThread::usleep(10000);

        SDL_CloseAudio();
    }
}

void AudioPlayer::printAVPacket(AVPacket *packet)
{
    qDebug()<<"=============================";
//    qDebug()<<"packet->buf:"<<packet->buf;
//    qDebug()<<"packet->convergence_duration:"<<packet->convergence_duration;
//    qDebug()<<"packet->data:"<<packet->data;
//    qDebug()<<"packet->dts:"<<packet->dts;
//    qDebug()<<"packet->duration:"<<packet->duration;
//    qDebug()<<"packet->flags:"<<packet->flags;
    qDebug()<<"packet->pos:"<<packet->pos;
    qDebug()<<"packet->pts:"<<packet->pts;
    qDebug()<<"packet->size:"<<packet->size;
//    qDebug()<<"packet->side_data:"<<packet->side_data;
//    qDebug()<<"packet->side_data_elems:"<<packet->side_data_elems;
//    qDebug()<<"packet->stream_index:"<<packet->stream_index;
    qDebug()<<"=============================";
}


void AudioPlayer::printAVCodecCtx(AVCodecContext *codec)
{
    qDebug()<<"============codecContext=======";
    qDebug("avcodecID:%x",codec->codec_id);
    qDebug()<<"codec_type:"<<codec->codec_type;
    qDebug()<<"time_base.den:"<<codec->time_base.den;
    qDebug()<<"time_base.num:"<<codec->time_base.num;
    qDebug()<<"channels:"<<codec->channels;
    qDebug()<<"sample_fmt:"<<codec->sample_fmt;
    qDebug()<<"sample_rate:"<<codec->sample_rate;
    qDebug()<<"===============================";
}
