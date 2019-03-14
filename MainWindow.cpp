#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDebug>
#include <QDateTime>
#include "SDLPlayer.h"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    AudioPlayer *mAudioPlayer = new AudioPlayer();
//    mAudioPlayer->startPlay("D:\\20180827152227_ch55.mp4");

    SDLPlayer::getSDLPlayer()->setSdlOpt(1,32000);
    SDLPlayer::getSDLPlayer()->startSDL();

    AudioPlayer *audioPlayer2 = new AudioPlayer();
    audioPlayer2->startPlay("D:\\work\\qtProject\\build-SoundToTextPro-Desktop_Qt_5_11_1_MSVC2015_32bit-Debug\\baiyang.wav");

    AudioPlayer *audioPlayer1 = new AudioPlayer();
    audioPlayer1->startPlay("D:\\work\\WeiWuErYu\\2.wav");

    AudioPlayer *audioPlayer3 = new AudioPlayer();
    audioPlayer3->startPlay("rtsp://192.168.31.203:554/forward");

//    mAudioPlayer->startPlay("rtsp://192.168.1.129/forward");
//    mAudioPlayer->startPlay("rtsp://192.168.1.118/ch013d4c180");

//    mAudioPlayerBak = new AudioPlayer();
//    mAudioPlayerBak->startPlay("D:\\work\\WeiWuErYu\\2.wav");

//    mVideoPlayer = new VideoPlayer();
//    connect(mVideoPlayer,SIGNAL(updateImageSignal(QImage)),this,SLOT(onUpdateImage(QImage)));
//    mVideoPlayer->startPlay("D:/news.mp4");
//    mVideoPlayer->startPlay("rtsp://192.168.1.202/forward");

//    AudioEncoder *mAudioEncoder = new AudioEncoder();
//    mAudioEncoder->creatFile("1.mp4");
//    mAudioEncoder->closeFile();

//    this->playAudio();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setBrush(Qt::black);
    painter.drawRect(0, ui->widgetTop->height(), this->width(), this->height()-ui->widgetControl->height()-ui->widgetTop->height()); //先画成黑色

    if (mImage.size().width() <= 0) return;
    painter.drawImage(0,ui->widgetTop->height(),mImage);
}

void MainWindow::on_pushButtonStart_clicked()
{
//    mAudioPlayer->startPlay("rtsp://192.168.1.202/forward");
    ui->pushButtonStart->setEnabled(false);
    mAudioPlayer->startPlay(ui->lineEditAddr->text());
    mVideoPlayer->startPlay(ui->lineEditAddr->text());
}

void MainWindow::on_pushButtonStop_clicked()
{
    ui->pushButtonStart->setEnabled(true);
    this->mAudioPlayer->stopPlay();
    this->mVideoPlayer->stopPlay();
}

void MainWindow::onUpdateTime()
{

}

void MainWindow::onUpdateImage(QImage image)
{
    //将图像按比例缩放成和窗口一样大小
    mImage = image.scaled(QSize(this->width(),this->height()-ui->widgetControl->height()-ui->widgetTop->height()),Qt::KeepAspectRatio);
    this->update();
}

Uint8 *srcData = NULL;
int srcLen = 0;
double playLen = 0;
int64_t MainWindow::mBaseAudioPts = 0;
int64_t MainWindow::mAudioPts = 0;

void MainWindow::audioCallBack(void *userData,Uint8 *stream,int len)
{
    memset(stream,0,len);
    if(srcLen == 0)return;
    len = srcLen>len?len:srcLen;
    SDL_MixAudio(stream,srcData,len,SDL_MIX_MAXVOLUME);
    mAudioPts = playLen/44100.0+mBaseAudioPts;
    qDebug()<<"playLen:"<<playLen;
    qDebug()<<"mAudioPts:"<<mAudioPts;
    playLen += len;
    srcData += len;
    srcLen -= len;
}


void MainWindow::playAudio()
{
    char fileName[] = "E:\\Kugou\\1.mp3";

    //3.打开输入流
    AVFormatContext *formatContext = avformat_alloc_context();
    if(avformat_open_input(&formatContext,fileName,NULL,NULL) < 0){
        qDebug()<<"avformat_open_input failure";
        return;
    }

    //4.找到输入流
    if(avformat_find_stream_info(formatContext,NULL) < 0){
        qDebug()<<"avformat_find_stream_info failure";
        return;
    }

    //5.遍历并找到音频流
    int audioIndex = -1;
    for(int i=0;i < formatContext->nb_streams;++i){
        if(formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
            audioIndex = i;
            break;
        }
    }
    if(audioIndex == -1){
        qDebug()<<"Don't find audio data";
        avformat_close_input(&formatContext);
        return;
    }

    //6.读取编解码器上下文，并找到解码器
    AVCodecContext *codecContext = formatContext->streams[audioIndex]->codec;
    AVCodec *codec = avcodec_find_decoder(codecContext->codec_id);
    if(codec == NULL){
        qDebug()<<"Don't find audio decoder";
        avformat_close_input(&formatContext);
        return;
    }

    //7.打开解码器
    if(avcodec_open2(codecContext,codec,NULL) < 0){
        qDebug()<<"open audio decoder failure";
        avformat_close_input(&formatContext);
        return;
    }

    //SDL处理
    SDL_AudioSpec wantAudioSpec;
    wantAudioSpec.freq = 44100;//采样率
    wantAudioSpec.channels = 2;//通道数
    wantAudioSpec.silence = 0;//是否禁音
    wantAudioSpec.samples = codecContext->frame_size;//帧大小
    wantAudioSpec.format = AUDIO_S16SYS;//采样格式
    wantAudioSpec.callback = audioCallBack;//回调函数
    wantAudioSpec.userdata = codecContext;
    qDebug()<<"wantAudioSpec.samples:"<<wantAudioSpec.samples;

    if(SDL_OpenAudio(&wantAudioSpec, NULL)<0){
        qDebug("SDL_OpenAudio failure %s",SDL_GetError());
        avformat_close_input(&formatContext);
        return;
    }

    SDL_PauseAudio(0);

    //8.创建想要的解码格式方便播放
    SwrContext *swrContext = swr_alloc();
    swr_alloc_set_opts(swrContext,AV_CH_LAYOUT_STEREO,AV_SAMPLE_FMT_S16,wantAudioSpec.freq,
                       av_get_default_channel_layout(codecContext->channels),codecContext->sample_fmt,codecContext->sample_rate
                       ,0,NULL);
    swr_init(swrContext);

    //9.读取流信息
    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    AVFrame *frame = av_frame_alloc();
    int frameSize = 0;
    Uint8 *audioData = (Uint8 *)av_malloc(MAX_AUDIO_SIZE_PER_SECOND*2);
    int out_buffer_size=av_samples_get_buffer_size(NULL,wantAudioSpec.channels ,wantAudioSpec.samples,AV_SAMPLE_FMT_S16, 1);
    qDebug()<<"out_buffer_size:"<<out_buffer_size;

    while(av_read_frame(formatContext,packet)>=0){
        if(packet->stream_index == audioIndex){
            mBaseAudioPts = packet->pts;
            if(avcodec_decode_audio4(codecContext,frame,&frameSize,packet) < 0){
                qDebug()<<"avcodec_decode_audio4 failure";
                break;
            }

            if(frameSize > 0){
                //解码数据处理
                swr_convert(swrContext,&audioData,MAX_AUDIO_SIZE_PER_SECOND,(const Uint8 **)frame->data,frame->nb_samples);

                //等待数据播放完成
                while(srcLen > 0)SDL_Delay(10);
                playLen = 0;

                //添加数据
                srcData = audioData;
                srcLen += out_buffer_size;
            }
        }
    }

    SDL_CloseAudio();
    SDL_Quit();

    avformat_close_input(&formatContext);
    avcodec_close(codecContext);
    av_free(packet);
    av_frame_free(&frame);
}

void MainWindow::playVideo()
{
    //3.分配AVFormatContext上下文
    AVFormatContext *formatCtx = avformat_alloc_context();

    char fileName[] = "E:\\Kugou\\1.avi";

    //4.流媒体解封装 注意：当失败的时候会free(formatCtx);
    if(avformat_open_input(&formatCtx,fileName,NULL,NULL) < 0){
        qDebug()<<"avformat_open_input failure";
        return;
    }

    //5.找到流信息
    if(avformat_find_stream_info(formatCtx,NULL) < 0){
        qDebug()<<"avformat_find_stream_info failure";
        return;
    }

    //6.获取视频流
    int vIndex = -1;
    for(int i=0;i < formatCtx->nb_streams;++i){
        if(formatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            vIndex = i;
            break;
        }
    }
    if(vIndex == -1){
        qDebug()<<"video stream find failure";
    }

    //7.打开解码器
    AVCodecContext *codecContext = formatCtx->streams[vIndex]->codec;
    AVCodec* codec = avcodec_find_decoder(codecContext->codec_id);
    if(codec == NULL){
        qDebug()<<"avcodec_find_decoder not find";
    }
    if(avcodec_open2(codecContext,codec,NULL) < 0){
        qDebug()<<"avcodec_open2 failure";
        return;
    }

    //8.指定你的格式转换器
    //8.1分配AVFrame空间
    AVFrame *wantFrame = av_frame_alloc();
    unsigned char *wantFrameData = (unsigned char*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24,codecContext->width,codecContext->height,1));
    av_image_fill_arrays(wantFrame->data,wantFrame->linesize,wantFrameData,AV_PIX_FMT_RGB24,codecContext->width,codecContext->height,1);

    //8.2创建转换器
    SwsContext *swsContent = sws_getContext(codecContext->width,codecContext->height,codecContext->pix_fmt
                                            ,codecContext->width,codecContext->height,AV_PIX_FMT_RGB24
                                            ,SWS_BICUBIC, NULL, NULL, NULL);

    //SDL
    SDL_Window *window=SDL_CreateWindow("simple player",200,200,480,320,SDL_WINDOW_OPENGL);
    if(window==NULL){
        qDebug()<<"SDL_CreateWidnow failure";
        return;
    }

    SDL_Renderer *renderer=SDL_CreateRenderer(window,-1,0);
    if(renderer==NULL){
        qDebug()<<"SDL_CreateRenderer failure";
        return;
    }

    SDL_Texture *rgbTexture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGB24,SDL_TEXTUREACCESS_STREAMING,codecContext->width,codecContext->height);
    if(!rgbTexture){
        qDebug()<<"SDL_CreateTexture failure";
        return;
    }

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
    while(av_read_frame(formatCtx,packet) >= 0){
        if(packet->stream_index == vIndex){
            if(avcodec_decode_video2(codecContext,frame,&getFrameCount,packet) < 0){
                qDebug()<<"avcodec_decode_video2 failure";
                return;
            }

            if(getFrameCount > 0){
                //延时处理
                double normal = (mPreVideoPts - mAudioPts)* av_q2d(codecContext->time_base);
                mCurTimeStamp = QDateTime::currentDateTime().toTime_t();

                //转换
                sws_scale(swsContent,(const unsigned char* const*)frame->data,frame->linesize,0,codecContext->height
                          ,wantFrame->data,wantFrame->linesize);

                //显示
                SDL_UpdateTexture(rgbTexture,NULL,wantFrame->data[0],wantFrame->linesize[0]);

                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer,rgbTexture,NULL,&rect);
                SDL_RenderPresent(renderer);

//                SDL_Delay(40);
                double delay = packet->duration * av_q2d(codecContext->time_base) * 1000;
                SDL_Delay((int)delay);
                mPreVideoPts = packet->pts;
            }
        }

    }
}
