#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "combinePicture.h"
#include "libyuv.h"
#include <cuda_runtime.h>
#include "yuyv2rgb.cuh"
#include "format_conv.h"

//#define SUPPORT_SAVE_YUV_RAW 1
//#define SUPPORT_SAVE_YUV420_RAW 1
//#define SUPPORT_SAVE_YUVSCALE_RAW 1
//#define SUPPORT_SAVE_RGB_RAW 1

CcvtImage::CcvtImage()
{
    m_imgNum = 0;
    m_outWidth = 0;
    m_outHeight = 0;
    m_mode = 0;
    m_pCudaRgbBuf = NULL;
    m_pCudaYuvBuf = NULL;
}
CcvtImage::~CcvtImage()
{
}

/*
    *brief             拼图初始化函数
    *description       对拼图流程进行初始化 对图片进行横向拼接和纵向拼接
    *param imgNum      图片编号
    *param inWidth     输入图片宽度      
    *param inHeight    输入图片高度      
    *param outWidth    输出图片宽度      
    *param outHeight   输出图片高度      
    *param mode        拼接模式 横向拼接 纵向拼接      
    *return            true
*/
bool CcvtImage::Init(int imgNum, int inWidth, int inHeight, int outWidth, int outHeight, int mode)
{
    if (imgNum > SUPPORT_MAX_CHANNEL)
    {
        printf("[%s:%d] invalid imgNum:%d, max support:%d\n", __func__, __LINE__, imgNum, SUPPORT_MAX_CHANNEL);
        return false;
    }
    m_imgNum = imgNum;
    m_outWidth = outWidth;
    m_outHeight = outHeight;
    m_inWidth = inWidth;
    m_inHeight = inHeight;
    m_mode = mode;

    // malloc memory
    m_pstImgH1.width = m_inWidth * 2;
    m_pstImgH1.height = m_inHeight;
    m_pstImgH1.dataLen = m_inWidth * 2 * m_inHeight * 2;
    m_pstImgH1.pData = (char *)malloc(m_pstImgH1.dataLen);
    if (NULL == m_pstImgH1.pData)
    {
        printf("Malloc memory for  faild\n");
        return false;
    }
    memset(m_pstImgH1.pData, 0, m_pstImgH1.dataLen);

    m_pstImgH2.width = m_inWidth * 2;
    m_pstImgH2.height = m_inHeight;
    m_pstImgH2.dataLen = m_inWidth * 2 * m_inHeight * 2;
    m_pstImgH2.pData = (char *)malloc(m_pstImgH2.dataLen);
    if (NULL == m_pstImgH2.pData)
    {
        printf("Malloc memory for  faild\n");
        return false;
    }
    memset(m_pstImgH2.pData, 0, m_pstImgH2.dataLen);

    m_pstImgYuv1.width = m_inWidth * 2;
    m_pstImgYuv1.height = m_inHeight * 2;
    m_pstImgYuv1.dataLen = m_inWidth * 2 * m_inHeight * 2 * 2;
    m_pstImgYuv1.pData = (char *)malloc(m_pstImgYuv1.dataLen);
    if (NULL == m_pstImgYuv1.pData)
    {
        printf("Malloc memory for  faild\n");
        return false;
    }
    memset(m_pstImgYuv1.pData, 0, m_pstImgYuv1.dataLen);

    //for yuv420
    m_pstImgYuv2.width = m_inWidth * 2;
    m_pstImgYuv2.height = m_inHeight * 2;
    m_pstImgYuv2.dataLen = m_inWidth * 2 * m_inHeight * 2 * 3 / 2;
    m_pstImgYuv2.pData = (char *)malloc(m_pstImgYuv2.dataLen);
    if (NULL == m_pstImgYuv2.pData)
    {
        printf("Malloc memory for  faild\n");
        return false;
    }
    memset(m_pstImgYuv2.pData, 0, m_pstImgYuv2.dataLen);

    //for yuv420 display
    m_pstImgYuv3.width = DISPLAY_WIDTH;
    m_pstImgYuv3.height = DISPLAY_HEIGHT;
    m_pstImgYuv3.dataLen = m_pstImgYuv3.width * m_pstImgYuv3.height * 3 / 2;
    m_pstImgYuv3.pData = (char *)malloc(m_pstImgYuv3.dataLen);
    if (NULL == m_pstImgYuv3.pData)
    {
        printf("Malloc memory for  faild\n");
        return false;
    }
    memset(m_pstImgYuv3.pData, 0, m_pstImgYuv3.dataLen);

    //Allocate cuda memory for device
    int rgb_image_size = m_outWidth * m_outHeight * 3;
    int yuyv_image_size = m_outWidth * m_outHeight * 2;
    cudaError_t ret = cudaMalloc((void **)&m_pCudaYuvBuf, yuyv_image_size * sizeof(char));
    if (cudaSuccess != ret)
    {
        printf("Fail to allocate cuda memory %d\n", ret);
        return false;
    }
    ret = cudaMemset((void *)m_pCudaYuvBuf, 0, yuyv_image_size * sizeof(char));
    if (cudaSuccess != ret)
    {
        printf("Fail to set cuda memory1 %d\n", ret);
        return false;
    }
    ret = cudaMalloc((void **)&m_pCudaRgbBuf, rgb_image_size * sizeof(char));
    if (cudaSuccess != ret)
    {
        printf("Fail to allocate cuda memory %d\n", ret);
        return false;
    }
    ret = cudaMemset((void *)m_pCudaRgbBuf, 0, rgb_image_size * sizeof(char));
    if (cudaSuccess != ret)
    {
        printf("Fail to set cuda2 memory %d\n", ret);
        return false;
    }

    return true;
}

/*
    *brief             拼图开始函数
    *description       开始拼图     
    *return            true
*/
bool CcvtImage::Start()
{

    return true;
}

/*
    *brief             拼图开始函数
    *description       开始拼图     
    *return            true
*/
int CcvtImage::CvtImage(TImageInfo *pstImageArry, int imageNum, TImageInfo &pstOutImage)
{
    if (pstImageArry == NULL || imageNum < m_imgNum)
    {
        printf("[%s:%d] invalid input parameters!\n", __func__, __LINE__);
        return -1;
    }

    /*横向拼接*/
    imageMerge(pstImageArry, 2, &m_pstImgH1, YUV_MERGE_MODE_H);
    imageMerge(pstImageArry + 2, 2, &m_pstImgH2, YUV_MERGE_MODE_H);
    m_pstInImage[0].width = m_pstImgH1.width;
    m_pstInImage[0].height = m_pstImgH1.height;
    m_pstInImage[0].dataLen = m_pstImgH1.dataLen;
    m_pstInImage[0].pData = m_pstImgH1.pData;
    m_pstInImage[0].type = m_pstImgH1.type;

    m_pstInImage[1].width = m_pstImgH2.width;
    m_pstInImage[1].height = m_pstImgH2.height;
    m_pstInImage[1].dataLen = m_pstImgH2.dataLen;
    m_pstInImage[1].pData = m_pstImgH2.pData;
    m_pstInImage[1].type = m_pstImgH2.type;

    /*纵向拼接*/
    imageMerge(m_pstInImage, 2, &m_pstImgYuv1, YUV_MERGE_MODE_V);


#ifdef SUPPORT_SAVE_YUV_RAW
    {
        FILE *fp = NULL;
        static int count = 0;
        char buf[256] = {0};
        snprintf(buf, sizeof(buf), "merge%d.yuv", count++);
        fp = fopen(buf, "w");
        if (fp == NULL)
        {
            printf("[%s:%d] open file:%s failed\n", __func__, __LINE__, buf);
            return -1;
        }
        fwrite(m_pstImgYuv1.pData, 1, m_pstImgYuv1.dataLen, fp);
        fclose(fp);
    }
#endif

    /*convert yuyv422 to yuv420*/
    memset(m_pstImgYuv2.pData, 0, m_pstImgYuv2.dataLen);
    libyuv::YUY2ToI420((uint8_t *)m_pstImgYuv1.pData, 2 * m_pstImgYuv1.width,
                       (uint8_t *)m_pstImgYuv2.pData, m_pstImgYuv2.width,
                       (uint8_t *)m_pstImgYuv2.pData + m_pstImgYuv2.width * m_pstImgYuv2.height, m_pstImgYuv2.width / 2,
                       (uint8_t *)m_pstImgYuv2.pData + 5 * m_pstImgYuv2.width * m_pstImgYuv2.height / 4, m_pstImgYuv2.width / 2, m_pstImgYuv1.width, m_pstImgYuv1.height);

#ifdef SUPPORT_SAVE_YUV420_RAW
    {
        FILE *fp = NULL;
        static int count1 = 0;
        char buf[256] = {0};
        snprintf(buf, sizeof(buf), "merge420_%d.yuv", count1++);
        fp = fopen(buf, "w");
        if (fp == NULL)
        {
            printf("[%s:%d] open file:%s failed\n", __func__, __LINE__, buf);
            return -1;
        }
        fwrite(m_pstImgYuv2.pData, 1, m_pstImgYuv2.dataLen, fp);
        fclose(fp);
    }
#endif
    /*down scale to 1080p*/
    memset(m_pstImgYuv3.pData, 0, m_pstImgYuv3.dataLen);
    libyuv::I420Scale((uint8_t *)m_pstImgYuv2.pData, m_pstImgYuv2.width,
                      (uint8_t *)m_pstImgYuv2.pData + m_pstImgYuv2.width * m_pstImgYuv2.height, m_pstImgYuv2.width / 2,
                      (uint8_t *)m_pstImgYuv2.pData + 5 * m_pstImgYuv2.width * m_pstImgYuv2.height / 4, m_pstImgYuv2.width / 2,
                      m_pstImgYuv2.width, m_pstImgYuv2.height,
                      (uint8_t *)m_pstImgYuv3.pData, m_pstImgYuv3.width,
                      (uint8_t *)m_pstImgYuv3.pData + m_pstImgYuv3.width * m_pstImgYuv3.height, m_pstImgYuv3.width / 2,
                      (uint8_t *)m_pstImgYuv3.pData + 5 * m_pstImgYuv3.width * m_pstImgYuv3.height / 4, m_pstImgYuv3.width / 2,
                      m_pstImgYuv3.width, m_pstImgYuv3.height,
                      libyuv::kFilterNone);

#ifdef SUPPORT_SAVE_YUVSCALE_RAW
    {
        FILE *fp = NULL;
        static int count2 = 0;
        char buf[256] = {0};
        snprintf(buf, sizeof(buf), "mergeScale_%d.yuv", count2++);
        fp = fopen(buf, "w");
        if (fp == NULL)
        {
            printf("[%s:%d] open file:%s failed\n", __func__, __LINE__, buf);
            return -1;
        }
        fwrite(m_pstImgYuv3.pData, 1, m_pstImgYuv3.dataLen, fp);
        fclose(fp);
    }
#endif

    /*yuv420 to rgb*/
    pstOutImage.type = IMG_TYPE_RGB;
    pstOutImage.width = m_outWidth;
    pstOutImage.height = m_outHeight;
    pstOutImage.dataLen = m_outWidth * m_outHeight * 3;
    process_image_cuda(&m_pstImgYuv3, &pstOutImage);
#ifdef SUPPORT_SAVE_RGB_RAW
    {
        FILE *fp = NULL;
        static int count3 = 0;
        char buf[256] = {0};
        snprintf(buf, sizeof(buf), "merge%d.rgb", count3++);
        fp = fopen(buf, "w");
        if (fp == NULL)
        {
            printf("[%s:%d] open file:%s failed\n", __func__, __LINE__, buf);
            return -1;
        }
        fwrite(pstOutImage.pData, 1, pstOutImage.dataLen, fp);
        fclose(fp);
    }
#endif

    return 0;
}

bool CcvtImage::Stop()
{
    return true;
}

bool CcvtImage::Release()
{

    free(m_pstImgH1.pData);
    free(m_pstImgH2.pData);
    free(m_pstImgYuv1.pData);
    free(m_pstImgYuv2.pData);
    free(m_pstImgYuv3.pData);

    //cudaMemFree(m_pCudaRgbBuf);
    //cudaMemFree(m_pCudaYuvBuf);

    return true;
}

/*
 * merge image for yuyv422
 * need alloc memory for outImage
*/
int CcvtImage::imageMerge(TImageInfo *pstImageArry, int imageNum, TImageInfo *pOutImage, int mode)
{
    unsigned char *temp_y1 = NULL, *temp_y2 = NULL;
    unsigned char *temp_yuv = NULL;
    int width = 0;
    int height = 0;

    if (pstImageArry == NULL || imageNum <= 1 || pOutImage == NULL)
    {
        printf("[%s:%d] invalid input parameters!\n", __func__, __LINE__);
        return -1;
    }

    if ((pstImageArry[0].width != pstImageArry[1].width) || (pstImageArry[0].height != pstImageArry[1].height))
    {
        printf("[%s:%d] two image is not equal! img1:%d*%d, img2:%d*%d\n", __func__, __LINE__, pstImageArry[0].width, pstImageArry[0].height,
               pstImageArry[1].width, pstImageArry[1].height);
        return -1;
    }
    printf("[%s:%d] merge img1:%d*%d, img2:%d*%d to outImage:%d*%d\n", __func__, __LINE__, pstImageArry[0].width, pstImageArry[0].height,
           pstImageArry[1].width, pstImageArry[1].height, pOutImage->width, pOutImage->height);

    width = pstImageArry[0].width;
    height = pstImageArry[0].height;
    temp_yuv = (unsigned char *)pOutImage->pData;

    switch (mode)
    {
    case YUV_MERGE_MODE_H:
    {
        int i;
        temp_y1 = (unsigned char *)pstImageArry[0].pData;
        temp_y2 = (unsigned char *)pstImageArry[1].pData;

        for (i = 0; i < height; i++)
        {
            memcpy(temp_yuv, temp_y1, width * 2);
            temp_y1 += width * 2;
            temp_yuv += width * 2;

            memcpy(temp_yuv, temp_y2, width * 2);
            temp_y2 += width * 2;
            temp_yuv += width * 2;
        }

        break;
    }
    case YUV_MERGE_MODE_V:
    {
        temp_y1 = (unsigned char *)pstImageArry[0].pData;
        temp_y2 = (unsigned char *)pstImageArry[1].pData;

        memcpy(temp_yuv, temp_y1, width * height * 2);
        temp_yuv += width * height * 2;

        memcpy(temp_yuv, temp_y2, width * height * 2);
        break;
    }
    default:
        printf("[%s:%d] not support merger mode:%d\n", __func__, __LINE__, mode);
    }

    return 0;
}


int CcvtImage::process_image_cuda(TImageInfo *pstImgIn, TImageInfo *pstImgOut)
{
    struct timeval ts;

    int yuv_size = pstImgIn->dataLen * sizeof(char);

    gettimeofday(&ts, NULL);
    printf("[%lu.%lu]\tbefore copy image_data(CPU to GPU)\n", ts.tv_sec, ts.tv_usec);
    cudaError_t ret = cudaMemcpy(m_pCudaYuvBuf, pstImgIn->pData, yuv_size, cudaMemcpyHostToDevice);
    gettimeofday(&ts, NULL);
    printf("[%lu.%lu]\tcopy image_data(CPU to GPU) done\n", ts.tv_sec, ts.tv_usec);

    if (cudaSuccess != ret)
    {
        printf("cudaMemcpy fail %d\n", ret);
        return -1;
    }
    const int block_size = 256;
    const int num_blocks = yuv_size / (4 * block_size);

    gettimeofday(&ts, NULL);
    printf("[%lu.%lu]\tbefore yuyv2rgb computation\n", ts.tv_sec, ts.tv_usec);
    //yuyv2rgb_cuda(m_pCudaYuvBuf, m_pCudaRgbBuf, num_blocks, block_size);
    CUDA_yu12_to_rgb((unsigned char *)m_pCudaYuvBuf, (unsigned char *)m_pCudaRgbBuf, pstImgIn->width, pstImgIn->height);
    gettimeofday(&ts, NULL);
    printf("[%lu.%lu]\tyuyv2rgb computation done\n", ts.tv_sec, ts.tv_usec);

    int rgb_size = pstImgOut->dataLen * sizeof(char);

    gettimeofday(&ts, NULL);
    printf("[%lu.%lu]\tbefore copy image_data(GPU to CPU)\n", ts.tv_sec, ts.tv_usec);
    ret = cudaMemcpy(pstImgOut->pData, m_pCudaRgbBuf, rgb_size, cudaMemcpyDeviceToHost);
    gettimeofday(&ts, NULL);
    printf("[%lu.%lu]\tcopy image_data(GPU to CPU) done\n", ts.tv_sec, ts.tv_usec);
    printf("[%lu.%lu]\tcuda process image\n", ts.tv_sec, ts.tv_usec);

    if (cudaSuccess != ret)
    {
        printf("cudaMemcpy fail %d\n", ret);
        return -1;
    }

    return 0;
}
#if 0
int CcvtImage::process_image_cuda(TImageInfo *pstImgIn, TImageInfo *pstImgOut)
{
    struct timeval ts;

    int yuv_size = pstImgIn->dataLen * sizeof(char);

    gettimeofday(&ts, NULL);
    printf("[%lu.%lu]\tbefore copy image_data(CPU to GPU)\n", ts.tv_sec, ts.tv_usec);
    cudaError_t ret = cudaMemcpy(m_pCudaYuvBuf, pstImgIn->pData, yuv_size, cudaMemcpyHostToDevice);
    gettimeofday(&ts, NULL);
    printf("[%lu.%lu]\tcopy image_data(CPU to GPU) done\n", ts.tv_sec, ts.tv_usec);

    if (cudaSuccess != ret)
    {
        printf("cudaMemcpy fail %d\n", ret);
        return -1;
    }
    const int block_size = 256;
    const int num_blocks = yuv_size / (4 * block_size);

    gettimeofday(&ts, NULL);
    printf("[%lu.%lu]\tbefore yuyv2rgb computation\n", ts.tv_sec, ts.tv_usec);
    yuyv2rgb_cuda(m_pCudaYuvBuf, m_pCudaRgbBuf, num_blocks, block_size);
    gettimeofday(&ts, NULL);
    printf("[%lu.%lu]\tyuyv2rgb computation done\n", ts.tv_sec, ts.tv_usec);

    int rgb_size = pstImgOut->dataLen * sizeof(char);

    gettimeofday(&ts, NULL);
    printf("[%lu.%lu]\tbefore copy image_data(GPU to CPU)\n", ts.tv_sec, ts.tv_usec);
    ret = cudaMemcpy(pstImgOut->pData, m_pCudaRgbBuf, rgb_size, cudaMemcpyDeviceToHost);
    gettimeofday(&ts, NULL);
    printf("[%lu.%lu]\tcopy image_data(GPU to CPU) done\n", ts.tv_sec, ts.tv_usec);
    printf("[%lu.%lu]\tcuda process image\n", ts.tv_sec, ts.tv_usec);

    if (cudaSuccess != ret)
    {
        printf("cudaMemcpy fail %d\n", ret);
        return -1;
    }

    return 0;
}
#endif
