#pragma once

#include <string>
#include <iostream>
#include <map>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SUPPORT_MAX_CHANNEL 4

#define PIXL_WIDTH 1280
#define PIXL_HEIGHT 720

#define DISPLAY_WIDTH 1280
#define DISPLAY_HEIGHT 720

#define H_ImgWidth 2560
#define H_ImgHeight 720

#define Z_ImgWidth 1280
#define Z_ImgHeight 1440

#define HZ_ImgWidth 2560
#define HZ_ImgHeigth 1440

typedef struct tagImageInfo
{
    int type;
    int width;
    int height;
    int dataLen;
    unsigned int sec;
    unsigned int nan;
    char *pData;
} TImageInfo;

typedef enum tagMergeMode
{
    YUV_MERGE_MODE_H = 1,
    YUV_MERGE_MODE_V = 2,

    YUV_MERGE_MODE_MAX,
} EMergeMode;

typedef enum tagImageType
{
    IMG_TYPE_YUYV422 = 0,
    IMG_TYPE_YV16P = 1,
    IMG_TYPE_RGB = 2,
    IMG_TYPE_ARGB = 3,
    IMG_TYPE_RGBA = 4,

    IMG_TYPE_MAX,
} EImageType;

typedef struct tagImagebuf
{
    int len;
    char buffer[PIXL_WIDTH * PIXL_HEIGHT * 2];
    unsigned int sec;
    unsigned int nan;
} TImageBuf;
