#ifndef PCMAUDIOPLAYER_H
#define PCMAUDIOPLAYER_H

#include <QThread>
#include <QQueue>

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

class DataPacket
{
public:
    DataPacket()
    {
        mBuf = NULL;
        mBufSize = 0;
    }

    char *mBuf;
    int mBufSize;
};


class PCMAudioPlayer : public QThread
{
    Q_OBJECT
public:
    explicit PCMAudioPlayer(QObject *parent = 0);
    static void audioCallBack(void *userData,Uint8 *stream,int len);
    void setStreamUri(QString streamUri);
    void startPlay(QString streamUri);
    static int audio_decode_frame(uint8_t *audioData, int audioDataLen);

signals:

public slots:

protected:
    void run();

private:
    static Uint8* mSrcData;
    static int mSrcLen;

    QString mStreamUri;

    static SwrContext *mSwrContext;
    AVFormatContext *mFormatContext;
    static AVCodecContext *mCodecContext;
    SDL_AudioSpec mWantAudioSpec;
    int mAudioIndex;


public:
    static int mBaseAudioPts;
    static AVRational mTimeBase;
    static double mAudioPts;
    static double mAudioStamp;
    static double mBytesPerSecond;
    static QQueue<AVPacket*> *mFrameQueue;

    static int mPlayedLen;//已经播放长度
};

#endif // PCMAUDIOPLAYER_H
