#include "SdlDataModel.h"
#include <QDebug>

SdlDataModel::SdlDataModel()
{
    clear();
}

void SdlDataModel::clear()
{
    mSdlData = NULL;
    mSdlSize = 0;
    mPlaySize =0;
    mAudioPts = -1;
    mAudioSample = 32000;
}

void SdlDataModel::print()
{
    qDebug()<<"mSdlData:"<<mSdlData
              <<" mSdlSize:"<<mSdlSize
                <<" mPlaySize:"<<mPlaySize
                  <<" mAudioPts:"<<mAudioPts
                    <<" mAudioSample:"<<mAudioSample;
}
