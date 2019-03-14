#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QPaintEvent>
#include <QImage>
#include <QPainter>
#include "AudioPlayer.h"
#include "VideoPlayer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void playVideo();
    void playAudio();
    static void audioCallBack(void *userData,Uint8 *stream,int len);

protected:
    void paintEvent(QPaintEvent *event);

public slots:
    void onUpdateImage(QImage image);
    void onUpdateTime();

private slots:
    void on_pushButtonStart_clicked();

    void on_pushButtonStop_clicked();

private:
    static int64_t mBaseAudioPts;
    static int64_t mAudioPts;

    int64_t mPreVideoPts;
    int64_t mVideoPts;
    uint mCurTimeStamp;

    AudioPlayer *mAudioPlayer;
    AudioPlayer *mAudioPlayerBak;
    VideoPlayer *mVideoPlayer;

private:
    Ui::MainWindow *ui;
    QImage mImage;
};

#endif // MAINWINDOW_H
