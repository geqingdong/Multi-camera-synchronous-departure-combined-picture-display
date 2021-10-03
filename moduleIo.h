#pragma once

#include "cameraSync.h"
#include "combinePicture.h"
#include "common.h"
#include "camera.h"
#include "cameraSync.h"

typedef struct tagContax
{
    camera *pCam[SUPPORT_MAX_CHANNEL];
    std::map<int, TImageBuf> mapImage;
    camerasyncu *pSyncu; 
    CcvtImage *pCvtImg;
    TImageInfo stImageInfo[SUPPORT_MAX_CHANNEL];
    TImageInfo stImageOut;
    int count;
    int imgReady;
} TContax;
