#ifndef SDLDATAMODEL_H
#define SDLDATAMODEL_H

class SdlDataModel
{
public:
    SdlDataModel();
    void clear();
    void print();

    unsigned char *mSdlData;
    int mSdlSize;
    int mPlaySize;
    double mAudioPts;
    int mAudioSample;
};

#endif // SDLDATAMODEL_H
