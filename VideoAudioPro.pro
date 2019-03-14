#-------------------------------------------------
#
# Project created by QtCreator 2018-05-07T10:03:33
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#QMAKE_CFLAGS_RELEASE += -MD
#QMAKE_CXXFLAGS_RELEASE += -MD


INCLUDEPATH += $$PWD\include\ffmpeg
INCLUDEPATH += $$PWD\include\sdl2.0
LIBS += -L$$PWD\lib\ffmpeg avcodec.lib avformat.lib avutil.lib swresample.lib swscale.lib
LIBS += -L$$PWD\lib\sdl2.0 SDL2.lib
TARGET = VideoAudioPro
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        MainWindow.cpp \
    AudioPlayer.cpp \
    CommonPlay.cpp \
    VideoPlayer.cpp \
    PCMAudioPlayer.cpp \
    SDLPlayer.cpp \
    SdlDataModel.cpp

HEADERS  += MainWindow.h \
    AudioPlayer.h \
    CommonPlay.h \
    VideoPlayer.h \
    PCMAudioPlayer.h \
    SDLPlayer.h \
    MediaQueue.h \
    SdlDataModel.h

FORMS    += MainWindow.ui
