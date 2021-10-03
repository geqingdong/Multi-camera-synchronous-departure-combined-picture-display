#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h> /* getopt_long() */
#include <fcntl.h>  /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <semaphore.h>
#include <dlfcn.h>
#include <sys/time.h>
#include <linux/videodev2.h>
#include <poll.h>
#include <unistd.h>
#include <sched.h>
#include <cuda_runtime.h>

#include <string.h>

#include <thread>
#include "yuyv2rgb.cuh"
#include "common.h"


#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define BUFFER_LENGHT 4

struct buffer
{
    void *start;
    size_t length;
};

class camera
{
public:
  typedef void (*callBackFunc)(char *pBuf, int len,  unsigned int sec, unsigned int nan, void *pUserData);

  camera(int chan);
  ~camera();
  bool Init(void);
  bool SetDataCallBack(callBackFunc pFunc, void *pUserData);
  bool Start(void);
  bool OnNotify(unsigned int sec, unsigned int nan);
  bool Stop(void);
  bool Release(void);

public:
  int read_frame();
  int m_fdVideo = -1;
  sem_t m_semTrig;

private:
  void initDevice(void);
  void initMmap(void);
  void openDevice(void);
  void startCapturing(void);
  void stopCapturing(void);
  void closeDevice(void);
  void uninit_device(void);

private:
  int m_chan;
  char m_devName[32];
  callBackFunc m_pCalllBackFunc;
  void *m_pUserData;
  std::thread *m_pGrabingThread;
  char m_pBuffer[PIXL_WIDTH * PIXL_HEIGHT * 3];
  struct buffer *m_pV4L2Buffs;
  unsigned int m_imgSec;
  unsigned int m_imgNan;
};

