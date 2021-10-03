#include "moduleIo.h"
#include "showThread.h"
#include <sys/time.h>

TContax g_Ctx;

/*
    brief:图片数据回调函数
    description:QT    把取到的图片数据回调出去
    param pBuf        图片数据buf    
    param len         图片数据长度    
    param sec         时间 秒    
    param nan         时间 毫秒
    param pUserData   回调出去的数据
    return           无
*/
void imageDataCallBack(char *pBuf, int len, unsigned int sec, unsigned int nan, void *pUserData)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    int chan = *((int *)pUserData);
    printf("[%s:%d] chan:%d, got image len:%d, cpu timestamp:%010u.%09u, trig timestamp:%010u.%09u.\n", __func__, __LINE__, chan, len,
                                    ts.tv_sec,ts.tv_nsec, sec, nan);

    TImageBuf stImageBuf;

    memcpy(stImageBuf.buffer, pBuf, len);

    //时间
    stImageBuf.len = len; //image buff len
    stImageBuf.sec = sec;
    stImageBuf.nan = nan;

    g_Ctx.mapImage[chan] = stImageBuf; 

    //相机count++
    g_Ctx.count++; 

    return;
}

/*
    brief             触发时间戳回调函数
    description:      把触发的时间回调出去           
    param len         图片数据长度    
    param sec         时间 秒    
    param nan         时间 毫秒
    param pUserData   回调出去的数据
    return            无
*/
void syncuTimestampCallback(unsigned int sec, unsigned int nan, void *pUserData)
{
    printf("[%s:%d] got trigger time:%010u.%09u\n", __func__, __LINE__, sec, nan);

    for (int i = 0; i < SUPPORT_MAX_CHANNEL; i++) //
    {
        g_Ctx.pCam[i]->OnNotify(sec, nan); //on syncu on camera
    }
    return;
}

/*
    brief             显示线程函数
*/
showThread::showThread()
{
    stopped = false;
    majorindex = -1;
}

/*
    brief             显示线程析构函数
    description:      相机停止，释放图片缓冲区           
    param len         图片数据长度    
    param sec         时间 秒    
    param nan         时间 毫秒
    param pUserData   回调出去的数据
    return            无
*/
showThread::~showThread()
{
    /*相机release*/
    for (int i = 0; i < SUPPORT_MAX_CHANNEL; i++)
    {
        g_Ctx.pCam[i]->Stop();
        g_Ctx.pCam[i]->Release();
        delete g_Ctx.pCam[i];
    }

    /*触发release*/
    g_Ctx.pSyncu->Release();
    delete (g_Ctx.pSyncu);

    /*图片拼接release*/
    g_Ctx.pCvtImg->Stop();
    g_Ctx.pCvtImg->Release();
    delete (g_Ctx.pCvtImg);
}

/*
    brief             显示线程停止函数
    description:      线程停止，显示结束           
    return            true
*/
void showThread::stop()
{
    stopped = true;
}

/*
    brief             显示线程初始化函数
    description:      相机初始化 输出图片数据初始化         
    param index       图片数据长度    
    return            无
*/
void showThread::init(int index)
{
    stopped = false;
    majorindex = index;

    for (int j = 0; j < SUPPORT_MAX_CHANNEL; j++)
    {
        g_Ctx.pCam[j] = NULL;
        memset(&g_Ctx.stImageInfo[j], 0, sizeof(TImageInfo));
        g_Ctx.stImageInfo[j].pData = NULL;
    }
    memset(&g_Ctx.stImageOut, 0, sizeof(TImageInfo));

    g_Ctx.stImageOut.width = DISPLAY_WIDTH;
    g_Ctx.stImageOut.height = DISPLAY_HEIGHT;
    g_Ctx.stImageOut.dataLen = DISPLAY_WIDTH * DISPLAY_HEIGHT * 3;
    g_Ctx.stImageOut.pData = (char *)malloc(g_Ctx.stImageOut.dataLen);
    if (NULL == g_Ctx.stImageOut.pData)
    {
        printf("Malloc memory for  stImageOut faild\n");
    }
    memset(g_Ctx.stImageOut.pData, 0, g_Ctx.stImageOut.dataLen);
    g_Ctx.count = 0;
    g_Ctx.pSyncu = NULL;
    g_Ctx.imgReady = 0;
    g_Ctx.pCvtImg = NULL;
}

/*
    brief             显示线程运行函数
    description:      相机停止，释放图片缓冲区           
    param len         图片数据长度    
    param sec         时间 秒    
    param nan         时间 毫秒
    param pUserData   回调出去的数据
    return            无
*/
void showThread::run() 
{
    printf("[%s:%d] run...\n", __func__, __LINE__);
    g_Ctx.pCvtImg = new (std::nothrow) CcvtImage();
    if (g_Ctx.pCvtImg == NULL)
    {
        printf("[%s:%d] create CcvtImage failed.\n", __func__, __LINE__);
        return;
    }

    /*拼图初始化/开始 相机通道 原始图片宽度 原始图片高度 显示图片宽度 显示图片高度 拼接模式*/
    g_Ctx.pCvtImg->Init(SUPPORT_MAX_CHANNEL, PIXL_WIDTH, PIXL_HEIGHT, DISPLAY_WIDTH, DISPLAY_HEIGHT, YUV_MERGE_MODE_H);
    g_Ctx.pCvtImg->Start();

    /*触发初始化*/
    g_Ctx.pSyncu = new (std::nothrow) camerasyncu();
    if (g_Ctx.pSyncu == NULL)
    {
        printf("[%s:%d] create camerasyncu failed.\n", __func__, __LINE__);
        return;
    }
    g_Ctx.pSyncu->Init();                                        
    g_Ctx.pSyncu->SetDataCallBack(syncuTimestampCallback, NULL); 

    /*四路相机*/
    for (int i = 0; i < SUPPORT_MAX_CHANNEL; i++) 
    {
        g_Ctx.pCam[i] = new camera(i); 
        g_Ctx.pCam[i]->Init();         
        g_Ctx.pCam[i]->SetDataCallBack(imageDataCallBack, (void *)&i);
        g_Ctx.pCam[i]->Start(); 
    }
    //g_Ctx.pSyncu->Start();                                       

    while (1)
    {
        if (g_Ctx.count == SUPPORT_MAX_CHANNEL)
        {
            printf("[%s:%d] got %d iamges.\n", __func__, __LINE__,SUPPORT_MAX_CHANNEL);
            memset(g_Ctx.stImageInfo, 0, sizeof(TImageInfo) * SUPPORT_MAX_CHANNEL);
            memset(g_Ctx.stImageOut.pData, 0, g_Ctx.stImageOut.dataLen);
            int j = 0;
            for (auto itor = g_Ctx.mapImage.begin(); itor != g_Ctx.mapImage.end(); itor++)
            {
                g_Ctx.stImageInfo[j].pData = itor->second.buffer;
                g_Ctx.stImageInfo[j].width = PIXL_WIDTH;
                g_Ctx.stImageInfo[j].height = PIXL_HEIGHT;
                g_Ctx.stImageInfo[j].type = IMG_TYPE_YUYV422;
                g_Ctx.stImageInfo[j].dataLen = itor->second.len;
                g_Ctx.stImageInfo[j].sec = itor->second.sec;
                g_Ctx.stImageInfo[j].nan = itor->second.nan;
                j++;
            }
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            printf("[%s:%d] %d iamges. cpu timestamp:%010u.%09u, trigg time:%010u.%09u, converting ...\n", __func__, __LINE__,j,
                                            ts.tv_sec, ts.tv_nsec, g_Ctx.stImageInfo[0].sec, g_Ctx.stImageInfo[0].nan);
            /*拼图 g_Ctx.stImageInfo输入图片 g_Ctx.stImageOut拼图后的数据*/
            g_Ctx.pCvtImg->CvtImage(g_Ctx.stImageInfo, j, g_Ctx.stImageOut);
            g_Ctx.mapImage.clear();
            clock_gettime(CLOCK_REALTIME, &ts);
            printf("[%s:%d] %d iamges. cpu timestamp:%010u.%09u, trigg time:%010u.%09u, converting done.\n", __func__, __LINE__,j,
                                            ts.tv_sec, ts.tv_nsec, g_Ctx.stImageInfo[0].sec, g_Ctx.stImageInfo[0].nan);

            /*send to QT display*/
            QImage img;
            int ret = 0;
            fflush(stdout);
            
            /*输出图片数据 输出图片宽度 输出图片高度 输出图片格式*/
            img = QImage((unsigned char *)g_Ctx.stImageOut.pData, g_Ctx.stImageOut.width, g_Ctx.stImageOut.height, QImage::Format_RGB888);
            emit SendMajorImageProcessing(img, ret);

            /*after QT display finished, then reset resource.*/
            g_Ctx.count = 0;
        }
        usleep(1000);
    }
}
