#ifndef AUDIODECODEHELPER_H
#define AUDIODECODEHELPER_H

#include <QObject>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/frame.h"
}

class AudioDecodeHelper : public QObject
{
    Q_OBJECT
public:
    explicit AudioDecodeHelper(QObject *parent = 0);

signals:

public slots:
};

#endif // AUDIODECODEHELPER_H
