#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QThread>
#include <QImage>

extern "C"{
#include "libavcodec/avcodec.h" //编解码
#include "libavformat/avformat.h" //封装格式
#include "libavutil/common.h" //其他工具类
#include "libavutil/imgutils.h" //图形缓存处理
#include "libswscale/swscale.h"  //音频转换
#include "libswresample/swresample.h" //视频转换
#include "SDL.h"
}

class VideoPlayer : public QThread
{
    Q_OBJECT
public:
    explicit VideoPlayer(QObject *parent = 0);
    void startPlay(QString fileName);
    void stopPlay();
    void printAVCodecCtx(AVCodecContext *codec);

signals:
    void updateImageSignal(QImage image);

public slots:

protected:
    void run();

private:

    SwsContext *mSwsContext;
    AVFormatContext *mFormatContext;
    AVCodecContext *mCodecContext;
    int mVideoIndex;
    AVFrame *mWantFrame;

    SDL_Renderer *mSDL_Renderer;
    SDL_Texture *mSDL_Texture;

    int mPreVideoPts;
    double mCurTimeStamp;
    double mPreTimeStamp;

    AVRational mTimeBase;
    QImage mImage;
    int mTotalPlayTime;//秒

    bool mStopFlag;
};

#endif // VIDEOPLAYER_H
