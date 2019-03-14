#ifndef SDLPLAYER_H
#define SDLPLAYER_H

/////////////////////////////////////////
///
///  文件名：SDLPlayer.cpp SDLPlayer.h
///  功能： 同时播放多个音频源（混音）
///  实现： 1.多音频源播放队列
///        2.播放目标格式（通道数，采样率，音频格式）
///        3.同步时间戳
///        4.控制（开启，关闭，暂停，恢复）
///        5.多音频波形显示队列
//////////////////////////////////////////

#include <QObject>
#include <QThread>
#include <QDateTime>
#include "MediaQueue.h"
#include "SdlDataModel.h"

extern "C"{
#include "SDL.h"
}


class SDLPlayer : public QObject
{
    Q_OBJECT
public:
    static SDLPlayer* getSDLPlayer();

    ///播放参数
    void setSdlOpt(int channelCount,int audioSample,int sampleCountPerPacket = 1024,SDL_AudioFormat format = AUDIO_S16SYS);
    SDL_AudioSpec& getSDLAudioSpec();//当前播放参数设置

    ///数据源
    int addAudioSourceQueue(SdlDataModel& model,int index = -1);
    void delAudioSourceQueue(int index);

    ///波形
    MediaQueue<short>& getSDLShowQueue(int index = 0);

    ///时间戳
    void setVideoStamp(long videoStamp);
    long getVideoStamp();

    ///控制
    void stopSDL();//停止SDL
    void resumeSDL();//恢复SDL
    bool startSDL();//开始SDL
    void closeSDL();//关闭SDL
    bool isOpenSDL();//是否打开

    //播放音频参数
    int getAudioSample();
    int getChannelCount();
    int getAudioFormat();
public:
    static void audioCallBack(void *,Uint8 *stream,int len);
    static bool mIsAudioVideoSync;

signals:
    void showWaveSignal(int index,SdlDataModel model);//显示波形信号

public slots:
    void onAudioValueEnqueue(int index,SdlDataModel model);//处理波形数据槽

private:
    explicit SDLPlayer(QObject *parent = nullptr);

private:
    static SDLPlayer *mSDLPlayer;
    static int mMaxIndex;
    QThread *mThread;

    QMutex mAudioSourceMapMutex;
    static QMap<int,MediaQueue<SdlDataModel> > *mAudioSourceMap;//需要播放内容
    static QMap<int,SdlDataModel> *mPlayingModelMap;//正在播放内容
    static QMap<int,MediaQueue<short> > *mAudioValueMap;//波形显示队列

    ///时间戳
    QMutex mVideoStampMutex;
    static long mAudioStamp;
    long mVideoStamp;

    ///参数
    int mAudioSample;
    int mChannelCount;
    int mSampleCountPerPacket;
    SDL_AudioFormat mAudioFormat;
    ///波形参数
    int mIntervalSampleCount;
    int mIntervalMSecond;

    ///
    SDL_AudioSpec mWantAudioSpec;
    bool mIsOpenSDL;


};

#endif // SDLPLAYER_H
