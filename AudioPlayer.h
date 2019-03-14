#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QThread>
#include <QQueue>
#include <QMutex>

extern "C"{
#include "libavcodec/avcodec.h" //编解码
#include "libavformat/avformat.h" //封装格式
#include "libavutil/common.h" //其他工具类
#include "libavutil/imgutils.h" //图形缓存处理
#include "libswscale/swscale.h"  //音频转换
#include "libswresample/swresample.h" //视频转换
#include "SDL.h"
}
#define MAX_AUDIO_SIZE_PER_SECOND 192000

class AudioPlayer : public QThread
{
    Q_OBJECT
public:
    explicit AudioPlayer(QObject *parent = 0);
    void setStreamUri(QString streamUri);
    void startPlay(QString streamUri);
    static void printAVPacket(AVPacket *packet);
    void printAVCodecCtx(AVCodecContext *codec);
    void stopPlay();

signals:

public slots:

protected:
    void run();

private:
    Uint8* mSrcData;
    int mSrcLen;

    QString mStreamUri;

    SwrContext *mSwrContext;
    AVFormatContext *mFormatContext;
    AVCodecContext *mCodecContext;
    SDL_AudioSpec mWantAudioSpec;
    int mAudioIndex;

    bool mStopFlag;
    bool mStreamFlag;


public:
    int mBaseAudioPts;
    AVRational mTimeBase;
    double mAudioPts;
    double mAudioStamp;
    double mBytesPerSecond;

    int mPlayedLen;//已经播放长度
    bool mIsOpenSdlSuccess;
    int mSdlIndex;
    AVFrame *mWantAudioFrame;
};

#endif // AUDIOPLAYER_H
