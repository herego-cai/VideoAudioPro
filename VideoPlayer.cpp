#include "VideoPlayer.h"
#include "AudioPlayer.h"
#include <QDateTime>
#include <QDebug>
#include <QImage>

VideoPlayer::VideoPlayer(QObject *parent) : QThread(parent)
{
}

void VideoPlayer::startPlay(QString fileName)
{
    //3.分配AVFormatContext上下文
    mFormatContext = avformat_alloc_context();

    //4.流媒体解封装 注意：当失败的时候会free(formatCtx);
    if(avformat_open_input(&mFormatContext,fileName.toStdString().data(),NULL,NULL) < 0){
        qDebug()<<"avformat_open_input failure";
        avformat_free_context(mFormatContext);
        return;
    }

    //5.找到流信息
    if(avformat_find_stream_info(mFormatContext,NULL) < 0){
        qDebug()<<"avformat_find_stream_info failure";
        avformat_close_input(&mFormatContext);
        return;
    }

    if (mFormatContext->duration != AV_NOPTS_VALUE) {
        int64_t duration = mFormatContext->duration + (mFormatContext->duration <= INT64_MAX - 5000 ? 5000 : 0);
        mTotalPlayTime = duration / AV_TIME_BASE;
    }

    //6.获取视频流
    mVideoIndex = -1;
    for(int i=0;i < mFormatContext->nb_streams;++i){
        if(mFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            mVideoIndex = i;
            break;
        }
    }
    if(mVideoIndex == -1){
        qDebug()<<"video stream find failure";
        avformat_free_context(mFormatContext);
        return;
    }

    mTimeBase.den = mFormatContext->streams[mVideoIndex]->time_base.den;
    mTimeBase.num = mFormatContext->streams[mVideoIndex]->time_base.num;

    //7.打开解码器
    mCodecContext = mFormatContext->streams[mVideoIndex]->codec;
    AVCodec* codec = avcodec_find_decoder(mCodecContext->codec_id);
    if(codec == NULL){
        qDebug()<<"avcodec_find_decoder not find";
        avformat_close_input(&mFormatContext);
        return;
    }

    if(avcodec_open2(mCodecContext,codec,NULL) < 0){
        qDebug()<<"avcodec_open2 failure";
        avformat_close_input(&mFormatContext);
        return;
    }

    printAVCodecCtx(mCodecContext);

    //8.指定你的格式转换器
    //8.1分配AVFrame空间
    mWantFrame = av_frame_alloc();
    unsigned char *wantFrameData = (unsigned char*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24,mCodecContext->width,mCodecContext->height,1));
    av_image_fill_arrays(mWantFrame->data,mWantFrame->linesize,wantFrameData,AV_PIX_FMT_RGB24,mCodecContext->width,mCodecContext->height,1);

    //8.2创建转换器
    mSwsContext = sws_getContext(mCodecContext->width,mCodecContext->height,mCodecContext->pix_fmt
                                            ,mCodecContext->width,mCodecContext->height,AV_PIX_FMT_RGB24
                                            ,SWS_BICUBIC, NULL, NULL, NULL);

    //SDL
//    SDL_Window *window=SDL_CreateWindow("simple player",200,200,480,320,SDL_WINDOW_OPENGL);
//    if(window==NULL){
//        qDebug()<<"SDL_CreateWidnow failure";
//        return;
//    }

//    mSDL_Renderer=SDL_CreateRenderer(window,-1,0);
//    if(mSDL_Renderer==NULL){
//        qDebug()<<"SDL_CreateRenderer failure";
//        return;
//    }

//    mSDL_Texture= SDL_CreateTexture(mSDL_Renderer,SDL_PIXELFORMAT_RGB24,SDL_TEXTUREACCESS_STREAMING,mCodecContext->width,mCodecContext->height);
//    if(!mSDL_Texture){
//        qDebug()<<"SDL_CreateTexture failure";
//        return;
//    }

    this->start();
}

void VideoPlayer::printAVCodecCtx(AVCodecContext *codec)
{
    qDebug()<<"============codecContext=======";
    qDebug()<<"avcodecID:"<<codec->codec_id;
    qDebug()<<"codec_type:"<<codec->codec_type;
    qDebug()<<"time_base.den:"<<codec->time_base.den;
    qDebug()<<"time_base.num:"<<codec->time_base.num;
    qDebug()<<"channels:"<<codec->channels;
    qDebug()<<"sample_fmt:"<<codec->sample_fmt;
    qDebug()<<"pix_fmt:"<<codec->pix_fmt;
    qDebug()<<"sample_rate:"<<codec->sample_rate;
    qDebug()<<"width:"<<codec->width;
    qDebug()<<"height:"<<codec->height;
//    qDebug()<<"sample_fmt:"<<codec->sample_fmt;
    qDebug()<<"===============================";
}


void VideoPlayer::run()
{
    mStopFlag = false;

    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = 480;
    rect.h = 320;

    //9.循环读取帧数据
    //9.1分配编码数据缓存
    AVPacket *packet = (AVPacket*)av_malloc(sizeof(AVPacket));
    if(packet == NULL){
        qDebug()<<"av_malloc packet failure";
        return;
    }
    av_init_packet(packet);

    //9.2分配源数据缓存
    AVFrame *frame = av_frame_alloc();

    //9.3开始解码
    int getFrameCount = 0;
    mPreVideoPts = 0;
    mPreTimeStamp = QDateTime::currentDateTime().toTime_t();
    double startTime = QDateTime::currentDateTime().toTime_t();
    int frameCount = 0;
    while(!mStopFlag && av_read_frame(mFormatContext,packet) >= 0){

        if(packet->stream_index == mVideoIndex){
            if(avcodec_decode_video2(mCodecContext,frame,&getFrameCount,packet) < 0){
                qDebug()<<"avcodec_decode_video2 failure";
                return;
            }

            if(getFrameCount > 0){
                qDebug("----VideoPts:%lf",mPreVideoPts);

//                qDebug("mCodecContext->time_base:%lf",av_q2d(mCodecContext->time_base));
//                qDebug("stream->time_base:%lf",av_q2d(mTimeBase));
//                qDebug()<<"duration:"<<packet->duration;

//                //延时处理 快进mPreTimeStamp变大 mPreVideoPts变大
//                double sysDiff = mPreTimeStamp - AudioPlayer::mAudioStamp;//当前差值
//                qDebug("sysDiff:%lf",sysDiff);
//                double ptsDiff = mPreVideoPts*av_q2d(mTimeBase)- AudioPlayer::mAudioPts;//合理差值
//                qDebug("++++mPreVideoPts:%lf",mPreVideoPts*av_q2d(mCodecContext->time_base));
//                qDebug("++++mAudioPts:%lf",AudioPlayer::mAudioPts);
//                double diff = (sysDiff + ptsDiff)*1000;//获取最终差值
//                qDebug()<<"diff:"<<diff<<"ms";

                double duration = packet->duration * av_q2d(mTimeBase) * 1000;

//                if(diff > 100){
//                    qDebug()<<"slow";
////                    QThread::msleep(100);
//                    QThread::msleep(diff);
//                }else if(diff < -100){
//                    if(duration+diff > 0)
//                        QThread::msleep(duration+diff);
//                    qDebug()<<"quick";
////                    mPreVideoPts = packet->pts;
////                    mPreTimeStamp = QDateTime::currentDateTime().toTime_t();
////                    continue;
//                }

                double pts = 0;
                if((pts = av_frame_get_best_effort_timestamp(frame)) == AV_NOPTS_VALUE)
                    mPreVideoPts += packet->duration;
                else
                    mPreVideoPts = pts;

                //转换
                sws_scale(mSwsContext,(const unsigned char* const*)frame->data,frame->linesize,0,mCodecContext->height
                          ,mWantFrame->data,mWantFrame->linesize);

                //显示
//                SDL_UpdateTexture(mSDL_Texture,NULL,mWantFrame->data[0],mWantFrame->linesize[0]);

                QImage tmpImg((uchar *)mWantFrame->data[0],mCodecContext->width,mCodecContext->height,QImage::Format_RGB888);
                mImage = tmpImg.copy();
                emit updateImageSignal(mImage);

//                SDL_RenderClear(mSDL_Renderer);
//                SDL_RenderCopy(mSDL_Renderer,mSDL_Texture,NULL,&rect);
//                SDL_RenderPresent(mSDL_Renderer);

                mPreTimeStamp = QDateTime::currentDateTime().toTime_t();

//                ++frameCount;
//                if(QDateTime::currentDateTime().toTime_t()-startTime>0)
//                    qDebug()<<"frame_rate:"<<frameCount/(QDateTime::currentDateTime().toTime_t()-startTime);

//                SDL_Delay(40);
//                double delay = packet->duration * av_q2d(mCodecContext->time_base) * 1000;
//                SDL_Delay((int)delay);
                QThread::msleep(40);
            }
        }
        av_free_packet(packet);
    }
    qDebug()<<"---------video thread quit";


    if(mFormatContext){
        avformat_close_input(&mFormatContext);
        mFormatContext = NULL;
    }


    if(packet){
        av_free(packet);
    }

    if(mSwsContext){
        sws_freeContext(mSwsContext);
        mSwsContext = NULL;
    }

//    if(mCodecContext){
//        avcodec_close(mCodecContext);
//        avcodec_free_context(&mCodecContext);
//        mCodecContext = NULL;
//    }
}

void VideoPlayer::stopPlay()
{
    if(!mStopFlag){
        mStopFlag = true;
    }
}
