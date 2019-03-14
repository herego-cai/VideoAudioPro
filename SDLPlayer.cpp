#include "SDLPlayer.h"
#include <QDebug>

#define MAX_AUDIO_SIZE_PER_SECOND 192000//19200*10

SDLPlayer::SDLPlayer(QObject *parent) : QObject(parent)
{
    mAudioSourceMap->clear();
    mIsOpenSDL = false;

    mThread = new QThread();
    this->moveToThread(mThread);
    mThread->start();

    connect(this,SIGNAL(showWaveSignal(int,SdlDataModel)),this,SLOT(onAudioValueEnqueue(int,SdlDataModel)),Qt::QueuedConnection);
}

SDLPlayer *SDLPlayer::mSDLPlayer = NULL;
int SDLPlayer::mMaxIndex = 1;
QMap<int,MediaQueue<SdlDataModel> > *SDLPlayer::mAudioSourceMap = new QMap<int,MediaQueue<SdlDataModel> >();
QMap<int,SdlDataModel> *SDLPlayer::mPlayingModelMap = new QMap<int,SdlDataModel>();
QMap<int,MediaQueue<short> > *SDLPlayer::mAudioValueMap = new QMap<int,MediaQueue<short> >();

SDLPlayer *SDLPlayer::getSDLPlayer()
{
    if(mSDLPlayer == NULL){
        mSDLPlayer = new SDLPlayer();
    }
    return mSDLPlayer;
}

void SDLPlayer::stopSDL()
{
    mIsAudioVideoSync = false;
    mAudioSourceMap->clear();
    mAudioValueMap->clear();
    mPlayingModelMap->clear();
    SDL_PauseAudio(1);
}

void SDLPlayer::resumeSDL()
{
    mIsAudioVideoSync = true;
    mAudioSourceMap->clear();
    mAudioValueMap->clear();
    mPlayingModelMap->clear();
    SDL_PauseAudio(0);
}

void SDLPlayer::closeSDL()
{
    stopSDL();
    SDL_CloseAudio();
    mIsOpenSDL = false;
}

MediaQueue<short>& SDLPlayer::getSDLShowQueue(int index)
{
    return *(this->mAudioValueMap->find(index));
}

void SDLPlayer::setVideoStamp(long videoStamp)
{
    mVideoStampMutex.lock();
    mVideoStamp = videoStamp;
    mVideoStampMutex.unlock();
}

long SDLPlayer::getVideoStamp()
{
    return mVideoStamp;
}

bool SDLPlayer::isOpenSDL()
{
    return mIsOpenSDL;
}

///////////////////////波形显示采样/////////////////////////////////
void SDLPlayer::onAudioValueEnqueue(int index,SdlDataModel model)
{
    if(getSDLShowQueue(index).count() >= 400){
        getSDLShowQueue(index).clear();
    }

    double total = 0;
    int count = 0;
    int interval = mIntervalSampleCount/2;

    if(model.mSdlData != NULL){
        unsigned char *data = model.mSdlData;
        int dataSize = model.mSdlSize;

//        qDebug()<<"dataSize:"<<dataSize;
        for(int i=0;i < dataSize;i=i+2){
//            qDebug()<<" i:"<<i<<" data+i:"<<*((short *)(data+i));
            total = total + abs(*((short *)(data+i)));
            if(count%interval == 0){
//                qDebug()<<"total/interval:"<<total/interval<<"(short)(total/interval):"<<(short)(total/interval);
                getSDLShowQueue(index).enqueue((short)(total/interval));
                total = 0;
            }
            ++count;
        }
    }
}

/////////////////////////音频播放//////////////////////////
long SDLPlayer::mAudioStamp = -1;
bool SDLPlayer::mIsAudioVideoSync = false;
void SDLPlayer::audioCallBack(void *,Uint8 *stream,int len)
{
    memset(stream,0,len);

    while(len > 0){
        //有无播放内容,无则静音
        bool isQuietFlag = true;
        //遍历播放内容
        foreach(int index,mAudioSourceMap->keys()){

            if(mPlayingModelMap->contains(index)){//已在播放队列则取下一个播放包
                //读取
                SdlDataModel &playingModel = *(mPlayingModelMap->find(index));
                if(playingModel.mPlaySize >= playingModel.mSdlSize){//播放完成，取下一个播放块
                    //1.释放空间,移到波形中释放
                    if(playingModel.mSdlData){
                        free(playingModel.mSdlData);
                        playingModel.mSdlData = NULL;
                    }

                    //2.取下一个播放数据
                    if(mAudioSourceMap->find(index)->count() > 0){
                        SDLPlayer::getSDLPlayer()->mAudioSourceMapMutex.lock();
                        playingModel = mAudioSourceMap->find(index)->dequeue();
                        SDLPlayer::getSDLPlayer()->mAudioSourceMapMutex.unlock();
                        isQuietFlag = false;
                    }
                    playingModel.mPlaySize = 0;
                }else{//未播放完成
                    isQuietFlag = false;
                }
            }else{//不在播放队列则添加到播放队列
                if(mAudioSourceMap->find(index)->count()>0){
                    SDLPlayer::getSDLPlayer()->mAudioSourceMapMutex.lock();
                    SdlDataModel playingModel = mAudioSourceMap->find(index)->dequeue();
                    SDLPlayer::getSDLPlayer()->mAudioSourceMapMutex.unlock();
                    mPlayingModelMap->insert(index,playingModel);
                    isQuietFlag = false;
                }
            }
        }

        //静音播放
        if(isQuietFlag){
            //播放静音数据，防止不再回调
            SdlDataModel playingModel;
            playingModel.mAudioPts = -1;
            playingModel.mSdlSize = len;
            playingModel.mSdlData = (unsigned char*)malloc(playingModel.mSdlSize);
            memset(playingModel.mSdlData, 0, playingModel.mSdlSize);
            QThread::usleep(10);

            //播放长度
            int leaveLen = playingModel.mSdlSize - playingModel.mPlaySize;//剩下播放长度
            int playLen = leaveLen>len?len:leaveLen;//播放长度

            //播放声音
            SDL_MixAudioFormat(stream,playingModel.mSdlData+playingModel.mPlaySize,AUDIO_S16SYS,playLen,SDL_MIX_MAXVOLUME);//SDL_MIX_MAXVOLUME 0-128
            return;
        }

        //播放
        int maxLen = 0;
        foreach(int index,mPlayingModelMap->keys()){
            SdlDataModel &playingModel = *(mPlayingModelMap->find(index));
            if(playingModel.mSdlData != NULL){
                //播放长度
                int leaveLen = playingModel.mSdlSize - playingModel.mPlaySize;//剩下播放长度
                int playLen = leaveLen>len?len:leaveLen;//播放长度
                if(maxLen <= playLen){
                    maxLen = playLen;
                }

                qDebug()<<"index:"<<index<<" dequeue---count():"<<mAudioSourceMap->find(index)->count()<<"playingModel.mSdlData:"<<playingModel.mSdlData;

                //播放声音
                SDL_MixAudioFormat(stream,playingModel.mSdlData+playingModel.mPlaySize,AUDIO_S16SYS,playLen,SDL_MIX_MAXVOLUME);//SDL_MIX_MAXVOLUME 0-128

                //播放长度更新
                playingModel.mPlaySize += playLen;
            }
        }

        if(maxLen){
            len -= maxLen;
            stream += maxLen;
        }
    }
}

int SDLPlayer::addAudioSourceQueue(SdlDataModel& model,int index)
{
    if(index <= 0){
        index = mMaxIndex++;
        MediaQueue<SdlDataModel> sdlDataModelQueue;
        mAudioSourceMapMutex.lock();
        mAudioSourceMap->insert(index,sdlDataModelQueue);
        mAudioSourceMapMutex.unlock();
    }

    if(mAudioSourceMap->contains(index)){
        mAudioSourceMapMutex.lock();
        mAudioSourceMap->find(index)->enqueue(model);
        mAudioSourceMapMutex.unlock();
        qDebug()<<"index:"<<index<<" enqueue---count():"<<mAudioSourceMap->find(index)->count()<<" model.mSdlData:"<<model.mSdlData;
    }else{
        index = -1;
    }
    return index;
}

void SDLPlayer::delAudioSourceQueue(int index)
{
    if(index){
        mAudioSourceMapMutex.lock();
        mAudioSourceMap->remove(index);
        mPlayingModelMap->remove(index);
        mAudioValueMap->remove(index);
        mAudioSourceMapMutex.unlock();
    }
}

void SDLPlayer::setSdlOpt(int channelCount,int audioSample,int sampleCountPerPacket,SDL_AudioFormat format)
{
    mChannelCount = channelCount;
    mAudioSample = audioSample;
    mSampleCountPerPacket = sampleCountPerPacket;
    mAudioFormat = format;
    mIntervalMSecond = 8;//8ms

    int audioSamplePointSize = 2;
    switch(mAudioFormat){
    case AUDIO_U8:
    case AUDIO_S8:
        audioSamplePointSize = 1;
        break;
    case AUDIO_U16LSB:
    case AUDIO_S16LSB:
    case AUDIO_U16MSB:
    case AUDIO_S16MSB:
        audioSamplePointSize = 2;
        break;
    case AUDIO_S32LSB:
    case AUDIO_S32MSB:
    case AUDIO_F32LSB:
    case AUDIO_F32MSB:
        audioSamplePointSize = 4;
        break;
    }

    mIntervalSampleCount = mAudioSample/1000*mIntervalMSecond*audioSamplePointSize*mChannelCount; //每个64个字节 32000/1000*1*2*1=64
}

bool SDLPlayer::startSDL()
{
    if(mAudioSample <= 0 || mChannelCount <= 0){
        qDebug()<<"audio option error";
        return false;
    }

    //已经打开时先关闭
    if(mIsOpenSDL){
        stopSDL();
    }

    //重新打开
    mWantAudioSpec.freq = mAudioSample;//mAudioSample;//采样率 static const int next_sample_rates[] = {0, 44100, 48000, 96000, 192000};
    mWantAudioSpec.channels = mChannelCount;//mChannelCount;//mAudioChannelCount;//通道数static const int next_nb_channels[] = {0, 0, 1, 6, 2, 6, 4, 6};
    mWantAudioSpec.silence = 0;//是否禁音
    mWantAudioSpec.samples = mSampleCountPerPacket;//FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE, 2<<av_log2(mWantAudioSpec.freq/SDL_AUDIO_MAX_CALLBACKS_PER_SEC));//帧大小 AAC:1024  MP3:1152
    mWantAudioSpec.format = mAudioFormat;//采样格式
    mWantAudioSpec.callback = audioCallBack;//回调函数

    if(SDL_OpenAudio(&mWantAudioSpec, NULL)<0){
        qDebug("SDL_OpenAudio failure %s",SDL_GetError());
        return false;
    }
    mIsOpenSDL = true;

    SDL_PauseAudio(0);
    return true;
}

int SDLPlayer::getAudioSample()
{
    return mAudioSample;
}

int SDLPlayer::getChannelCount()
{
    return mChannelCount;
}

int SDLPlayer::getAudioFormat()
{
    return mAudioFormat;
}


