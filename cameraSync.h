#pragma once

#include <stdio.h>
#include <unistd.h>
#include "syncu.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <thread>
#include <sys/time.h>
#include <time.h>
#include <iostream>
#include <string.h>
#include <poll.h>
#include <signal.h>
#include <queue>
#include <mqueue.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include "common.h"

class camerasyncu
{
public:
    typedef void (*callBackFunc)(unsigned int sec, unsigned int nan, void *pUserData);

    camerasyncu();
    ~camerasyncu();
    bool Init();
    bool SetDataCallBack(callBackFunc pFunc, void *pUserData);
    bool Start();
    bool Stop();
    bool Release();

private:
    //void trig_handler(int sig);
    int initTrigInterrupt(const char *dev_name);
    int syn();
private:
    std::thread *m_pSyncuThread;

public:
    callBackFunc m_pCallBackFunc;
    bool m_bRunning;
    void *m_pUserData;
    void *m_SyncHandle;
};
