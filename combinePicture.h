#pragma once

#include "common.h"

class CcvtImage
{
public:
    CcvtImage();
    ~CcvtImage();
    bool Init(int imgNum, int inWidth, int inHeight, int outWidth, int outHeight, int mode);
    bool Start();
    int CvtImage(TImageInfo *pstImageArry, int imageNum, TImageInfo &pstOutImage);
    bool Stop();
    bool Release();

private:
    int imageMerge(TImageInfo *pstImageArry, int imageNum, TImageInfo *pOutImage, int mode);
    int process_image_cuda(TImageInfo *pstImgIn, TImageInfo *pstImgOut);

private:
    TImageInfo m_pstInImage[SUPPORT_MAX_CHANNEL];
    int m_imgNum;
    int m_outWidth;
    int m_outHeight;
    int m_inWidth;
    int m_inHeight;
    int m_mode;
    TImageInfo m_pstImgH1;
    TImageInfo m_pstImgH2;
    TImageInfo m_pstImgYuv1;//yuv422
    TImageInfo m_pstImgYuv2;//yuv420 
    TImageInfo m_pstImgYuv3;//yuv420 1080p

    char *m_pCudaRgbBuf;
    char *m_pCudaYuvBuf;
};
