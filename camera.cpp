#include "camera.h"


void printErr(const char *s)
{
        printf("%s error %d, %s\n", s, errno, strerror(errno));
}


int xioctl(int fh, int request, void *arg)
{
        int r;
        do
        {
                r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);
        return r;
}

/*
    brief             相机取图线程函数
    description:      相机取图          
    param pHandle     指针句柄    
    return            无
*/
static void cameraGrabingThread(void *pHandle)
{
        camera *pCamera = (camera *)pHandle;
        //int ret = 0;
        fd_set fds;
        //struct timeval tv_timeout;

        printf("%s\n", __func__);

        FD_ZERO(&fds);
        FD_SET(pCamera->m_fdVideo, &fds);

        while (1)
        {
                sem_wait(&pCamera->m_semTrig);
                if (pCamera->read_frame() != 0)
                {
                        printf("[%s:%d] read camera frame failed!\n", __func__, __LINE__);
                }
        }
}

/*
    brief             相机参数初始化函数
    description:      对相机通道 线程 时间等初始化           
    param chan        相机通道    
    return            无
*/
camera::camera(int chan)
{
        m_pCalllBackFunc = NULL;
        m_pUserData = NULL;
        memset(m_devName, 0, sizeof(m_devName));
        m_chan = chan;
        m_fdVideo = -1;
        m_pGrabingThread = nullptr;
        m_pV4L2Buffs = NULL;
        m_imgSec = 0;
        m_imgNan = 0;
}
camera::~camera()
{
}

/*
    brief             相机初始化函数
    description:      图片Buffer清空 打开设备 设备初始化           
    return            无
*/
bool camera::Init(void)
{
        memset(m_pBuffer, 0, sizeof(m_pBuffer));
        snprintf(m_devName, sizeof(m_devName), "/dev/video%d", m_chan+1);
        sem_init(&m_semTrig, 0, 0);
        openDevice();
        initDevice();
        
        /*new一个相机取图线程*/
        m_pGrabingThread = new std::thread(cameraGrabingThread, this);
        m_pGrabingThread->detach();

        return true;
}

/*
    brief             相机数据回调函数
    description       对相机回调函数及用户数据赋值
    param pFunc       相机回调参数           
    param pUserData   用户数据           
    return            true
*/
bool camera::SetDataCallBack(callBackFunc pFunc, void *pUserData)
{
        m_pCalllBackFunc = pFunc;
        m_pUserData = pUserData;

        return true;
}

/*
    brief             相机启动函数
    description       相机开始取图         
    return            true
*/
bool camera::Start(void)
{

        startCapturing();
        return true;
}

/*
    brief             相机信号通知函数
    description       相机取到图之后发出一个信号给拼图程序
    param pFunc       时间 秒           
    param pUserData   时间 毫秒           
    return            true
*/
bool camera::OnNotify(unsigned int sec, unsigned int nan)//信号
{
        m_imgSec = sec;
        m_imgNan = nan;
        sem_post(&m_semTrig);

        return true;
}

/*
    brief             相机停止函数
    description       相机停止取图          
    return            true
*/
bool camera::Stop(void)
{
        stopCapturing();
        
        return true;
}

/*
    brief             相机Release函数
    description       关闭设备，delete抓图线程       
    return            true
*/
bool camera::Release(void)
{
        unsigned int i;
        printf("%s\n", __func__);

        closeDevice();
        
        delete m_pGrabingThread;
        uninit_device();
        return true;
}

/*
    brief             初始化设备函数
    description       v4l2取图初始化 用户空间（initmap）初始化        
    return            无
*/
void camera::initDevice(void)
{
        struct v4l2_capability cap;
        struct v4l2_format fmt;
        printf("%s\n", __func__);
        if (-1 == xioctl(m_fdVideo, VIDIOC_QUERYCAP, &cap))
        {
                if (EINVAL == errno)
                {
                        fprintf(stderr, "%s is no V4L2 device\n",
                                m_devName);
                        return;
                }
                else
                {
                        printErr("VIDIOC_QUERYCAP");
                }
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
        {
                fprintf(stderr, "%s is no video capture device\n",
                        m_devName);
                return;
        }
        

        if (!(cap.capabilities & V4L2_CAP_STREAMING))
        {
                fprintf(stderr, "%s does not support streaming i/o\n",
                        m_devName);
                return;
        }

        CLEAR(fmt);

        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = PIXL_WIDTH;
        fmt.fmt.pix.height = PIXL_HEIGHT;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

        if (-1 == xioctl(m_fdVideo, VIDIOC_S_FMT, &fmt))
                printErr("VIDIOC_S_FMT");

        usleep(10000);

        initMmap();
}

/*
    brief             用户空间初始化函数
    description       对用户空间进行初始化       
    return            无
*/
void camera::initMmap(void)
{
        int n_buffers;
        struct v4l2_requestbuffers req;
        printf("%s\n", __func__);
        CLEAR(req);

        req.count = BUFFER_LENGHT;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl(m_fdVideo, VIDIOC_REQBUFS, &req))
        {
                if (EINVAL == errno)
                {
                        fprintf(stderr, "%s does not support "
                                        "memory mapping\n",
                                m_devName);
                        return;
                }
                else
                {
                        printErr("VIDIOC_REQBUFS");
                }
        }

        if (req.count < 2)
        {
                fprintf(stderr, "Insufficient buffer memory on %s\n",
                        m_devName);
                return;
        }

        m_pV4L2Buffs = (struct buffer *)calloc(req.count, sizeof(*m_pV4L2Buffs));

        if (!m_pV4L2Buffs)
        {
                fprintf(stderr, "Out of memory\n");
                return;
        }

        for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
        {
                struct v4l2_buffer buf;

                CLEAR(buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = n_buffers;

                if (-1 == xioctl(m_fdVideo, VIDIOC_QUERYBUF, &buf))
                        printErr("VIDIOC_QUERYBUF");

                m_pV4L2Buffs[n_buffers].length = buf.length;
                m_pV4L2Buffs[n_buffers].start =
                    mmap(NULL /* start anywhere */,
                         buf.length,
                         PROT_READ | PROT_WRITE /* required */,
                         MAP_SHARED /* recommended */,
                         m_fdVideo, buf.m.offset);

                if (MAP_FAILED == m_pV4L2Buffs[n_buffers].start)
                        printErr("mmap");
        }
}

/*
    brief             打开设备函数
    description       打开设备        
    return            无
*/
void camera::openDevice(void)
{
        printf("%s\n", __func__);
        m_fdVideo = open(m_devName, O_RDWR /* required */ /*| O_NONBLOCK*/, 0);

        if (-1 == m_fdVideo)
        {
                fprintf(stderr, "Cannot open '%s': %d, %s\n",
                        m_devName, errno, strerror(errno));
                return;
        }
}

/*
    brief             相机开始取图函数
    description       相机开始取图        
    return            无
*/
void camera::startCapturing(void)
{
        unsigned int i;
        enum v4l2_buf_type type;

        struct v4l2_control ctrl_mode;

        printf("%s\n", __func__);

        for (i = 0; i < BUFFER_LENGHT; ++i)
        {
                struct v4l2_buffer buf;

                CLEAR(buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;

                if (-1 == xioctl(m_fdVideo, VIDIOC_QBUF, &buf))
                        printErr("VIDIOC_QBUF");
        }

        //set streaming on
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl(m_fdVideo, VIDIOC_STREAMON, &type))
                printErr("VIDIOC_STREAMON");
        return;
}

/*
    brief             相机停止取图函数
    description       相机停止取图        
    return            无
*/
void camera::stopCapturing(void)
{
        printf("%s\n", __func__);
        enum v4l2_buf_type type;

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl(m_fdVideo, VIDIOC_STREAMOFF, &type))
                printErr("VIDIOC_STREAMOFF");
}

void camera::uninit_device(void)
{
        unsigned int i;
        printf("%s\n", __func__);

        for (i = 0; i < BUFFER_LENGHT; ++i)
                if (-1 == munmap(m_pV4L2Buffs[i].start, m_pV4L2Buffs[i].length))
                        printErr("munmap");
        free(m_pV4L2Buffs);
}

/*
    *brief             关闭设备函数
    *description       相机停止取图        
    *return            无
*/
void camera::closeDevice(void)
{
        printf("%s\n", __func__);
        if (-1 == close(m_fdVideo))
        {
                printErr("close m_fdVideo");
        }
        m_fdVideo = -1;
}

/*
    *brief             读取图片函数
    *description       读取一帧一帧图片        
    *return            无
*/
int camera::read_frame()
{
        struct timeval ts;
        struct v4l2_buffer buf;
        CLEAR(buf);
        int dataLen = 0;

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        // get picture

        if (-1 == xioctl(m_fdVideo, VIDIOC_DQBUF, &buf))
        {
                printErr("VIDIOC_DQBUF");
                return -1;
        }

        dataLen = buf.bytesused;
        memset(m_pBuffer, 0, sizeof(m_pBuffer));
        memcpy(m_pBuffer, m_pV4L2Buffs[buf.index].start, dataLen);
        m_pCalllBackFunc(m_pBuffer, dataLen, m_imgSec, m_imgNan, (void *)&m_chan);
        m_imgSec = 0;
        m_imgNan = 0;

        if (-1 == xioctl(m_fdVideo, VIDIOC_QBUF, &buf))
        {
                return -1;
                printErr("VIDIOC_QBUF");
        }

        return 0;
}
