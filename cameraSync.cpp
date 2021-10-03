#include "cameraSync.h"
#include "syncu.h"
#include "camera.h"

sem_t g_semTrig;

/*
    *brief             触发线程函数
    *description       把读到的时间戳和UserData回调出去        
    *return            0
*/
static int syncThreadHandle(void *pUserData)
{
        camerasyncu *pHandle = (camerasyncu *)pUserData;
        printf("[%s:%d] thread start ...\n", __func__, __LINE__);

        while (pHandle->m_bRunning)
        {
                printf("[%s:%d] got trigger signal!\n", __func__, __LINE__);
                sem_wait(&g_semTrig);
                TSyncuStamp tv;
                printf("[%s:%d] hello\n", __func__, __LINE__);
                if (SYNCU_ReadTimeStamp(pHandle->m_SyncHandle, &tv) < 0)
                {
                        printf("read timestamp failed\n");
                }
                else
                {
                        printf("cpld:[%lld.%09lld]\n", tv.sec, tv.nan);
                        pHandle->m_pCallBackFunc(tv.sec, tv.nan, pHandle->m_pUserData);//got data to callback
                }
        }

        printf("[%s:%d] thread exit! ...\n", __func__, __LINE__);

        return 0;
}

/*
    *brief             触发处理函数函数
    *description       发送一个触发信号      
    *return            0
*/
static void trig_handler(int sig)//触发信号处理
{
        sem_post(&g_semTrig);
}

/*
    *brief             触发参数初始化函数
    *description       对触发的线程 用户数据等进行初始化        
    *return            无
*/
camerasyncu::camerasyncu()
{
        m_SyncHandle = NULL;
        m_bRunning = false;
        m_pSyncuThread = nullptr;
        m_pUserData = NULL;
        m_pCallBackFunc = NULL;
}

camerasyncu::~camerasyncu()
{
}

/*
    *brief             触发初始化函数
    *description               
    *return            0
*/
bool camerasyncu::Init()
{
        int hz = 30;
        int channum = 10;
        printf("[%s:%d] Init ...\n", __func__, __LINE__);
        sem_init(&g_semTrig, 0, 0);
        SYNCU_Init();
        m_SyncHandle = SYNCU_CreateHandle(0, 1, "/dev/ttyTHS4");
        if (m_SyncHandle == NULL)
        {
                printf("SYNCU_CreateHandle failed\n");
                return -1;
        }
        SYNCU_DisableTrig(m_SyncHandle);
        initTrigInterrupt("/dev/camera_trigger");

        char buf[64];
        SYNCU_ReadVersion(m_SyncHandle, buf, sizeof(buf));
        printf("cpld version:%s\n", buf);
        for (int i = 0; i < channum; i++)
        {
                SYNCU_SetTimeMode(m_SyncHandle, i, hz, 1000, 0);
        }

        usleep(1000 * 1000);
        m_bRunning = true;
        m_pSyncuThread = new std::thread(syncThreadHandle, this);
        m_pSyncuThread->detach();

        printf("[%s:%d] Init success!\n", __func__, __LINE__);
        return true;
}

/*
    *brief             设置回调函数
    *description       对用户数据进行赋值
    *return            true
*/
bool camerasyncu::SetDataCallBack(callBackFunc pFunc, void *pUserData)
{
        m_pCallBackFunc = pFunc;
        m_pUserData = pUserData;

        return true;
}

/*
    *brief             设置回调函数
    *description       对用户数据进行赋值
    *return            true
*/
bool camerasyncu::Start()
{
        printf("[%s:%d] trigger started ...\n", __func__, __LINE__);
        m_bRunning = true;
        SYNCU_EnableTrig(m_SyncHandle);
        return true;
}

/*
    *brief             触发停止函数
    *description       触发disable
    *return            true
*/
bool camerasyncu::Stop()
{
        printf("[%s:%d] trigger stopped ...\n", __func__, __LINE__);
        SYNCU_DisableTrig(m_SyncHandle);
        return true;
}

/*
    *brief             触发Release函数
    *description       触发销毁，关闭信号
    *return            true
*/
bool camerasyncu::Release()
{
        m_bRunning = false;
        delete m_pSyncuThread;
        CALLBACK SYNCU_DestroyHandle(m_SyncHandle);
        SYNCU_Release();
        sem_close(&g_semTrig);

        printf("[%s:%d] trigger Released ...\n", __func__, __LINE__);
        return true;
}

/*
    *brief             触发中断函数
    *description       收到数据之后触发中断
    *return            ret
*/
int camerasyncu::initTrigInterrupt(const char *dev_name)
{
        int fd;
        int ret = 0;
        int oflags;
        do
        {
                if ((fd = open(dev_name, O_RDONLY)) < 0)
                {
                        ret = -1;
                        std::cout << "open trig interrupt false..." << std::endl;
                        break;
                }
                else
                {
                        signal(SIGIO, trig_handler);
                        fcntl(fd, F_SETOWN, getpid());
                        oflags = fcntl(fd, F_GETFL);
                        fcntl(fd, F_SETFL, oflags | FASYNC);
                }

        } while (0);
        return ret;
}
