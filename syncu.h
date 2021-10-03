#pragma once

#if defined(_WIN32) || defined(_WIN64)
#ifndef CALLBACK
#define CALLBACK __stdcall
#endif
#ifdef SYNCU_EXPORTS
#define SYNCU_API extern "C" __declspec(dllexport)
#else
#define SYNCU_API extern "C" __declspec(dllimport)
#endif
#elif defined(__linux__)
#define SYNCU_API extern "C"
#ifndef CALLBACK
#define CALLBACK
#endif
#else
#define SYNCU_API
#ifndef CALLBACK
#define CALLBACK
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct tagTSyncuStamp
    {
        long long sec;
        long long nan;
    } TSyncuStamp;

    typedef enum
    {
        SYNCU_VERSION_V0 = 0,
        SYNCU_VERSION_V1 = 1,
    } SYNCU_VERSION;

#define MAX_TRIG_CHAN_NUM 10
    /*
    function:初始化
    return:成功0，失败-1
*/
    SYNCU_API int CALLBACK SYNCU_Init();

    /*
    function:反初始化
    return:成功0，失败-1
*/
    SYNCU_API int CALLBACK SYNCU_Release();

    /*
      function:创建句柄
      gpstype:0:内部GPS     1:外接GPS      2:室内模式
      version：协议版本，参见SYNCU_VERSION
      devname：串口或I2C设备名称
      return:成功返回句柄，失败返回NULL
*/
    SYNCU_API void *CALLBACK SYNCU_CreateHandle(int mode, int version, const char *devname);

    /*
      function:销毁句柄
      handle：句柄
      return:成功返回0，失败返回-1
*/
    SYNCU_API int CALLBACK SYNCU_DestroyHandle(void *handle);

    /*
    function:禁用触发
    handle：句柄
    return:成功返回0，失败返回-1
*/
    SYNCU_API int CALLBACK SYNCU_DisableTrig(void *handle);

    /*
    function:触发使能handle：句柄
    return:成功返回0，失败返回-1
*/
    SYNCU_API int CALLBACK SYNCU_EnableTrig(void *handle);

    /*
    function: SetInputSerialParam
    handle：句柄
    channel: Reserved
    level: "232"or"485"
    baudRate: 115200|9600
    return:成功返回0，失败返回-1
*/
    SYNCU_API int CALLBACK SYNCU_SetInputSerialParam(void *handle, int channel, const char *level, int baudRate);

    /*
    function:SetOutputSerialParam
    handle：句柄
    channel: Reserved
    level: Reserved
    baudRate: 115200||9600
    return:成功返回0，失败返回-1
*/
    SYNCU_API int CALLBACK SYNCU_SetOutputSerialParam(void *handle, int channel, const char *level, int baudRate);

    /*
    function:设置定时触发
    handle：句柄
    chan: 通道
    hz：帧率
    width:脉宽，单位us
    offset:触发偏移值，单位us
    return:成功返回0，失败返回-1
*/
    SYNCU_API int CALLBACK SYNCU_SetTimeMode(void *handle, int chan, int hz, double width, int offset);

    /*
    function:设置手动触发
    handle：句柄
    chan: 通道
    width:脉宽，单位us
    offset:触发偏移值，单位us
    return:成功返回0，失败返回-1
*/
    SYNCU_API int CALLBACK SYNCU_SetManualMode(void *handle, int chan, double width, int offset);

    /*
        function:手动触发
        handle：句柄
        chan: 通道
        width：脉宽，单位us
        offset:延时触发时间，单位us
        return:成功返回0，失败返回-1
*/
    SYNCU_API int CALLBACK SYNCU_DoTrig(void *handle, int chan, double width, int offset);

    /*
    function:读取时间戳
    handle：句柄
    tv：时间戳，输出值
    return:成功返回0，失败返回-1
*/
    SYNCU_API int CALLBACK SYNCU_ReadTimeStamp(void *handle, TSyncuStamp *tv);

    /*
    function:读取版本
    handle：句柄
    buf：输出值，存放版本字符串
    len:  buf长度
    return:成功返回0，失败返回-1
*/
    SYNCU_API int CALLBACK SYNCU_ReadVersion(void *handle, char *buf, int len);

    /*
    function:读寄存器
    handle：句柄
    dwReg:寄存器

    dwVal：返回值
    return:成功返回0，失败返回-1
*/
    SYNCU_API int CALLBACK SYNCU_Readl(void *handle, unsigned int dwReg, unsigned int *dwVal);

    /*
    function:写寄存器
    handle：句柄
    dwReg:寄存器
    dwVal：值
    return:成功返回0，失败返回-1
*/
    SYNCU_API int CALLBACK SYNCU_Writel(void *handle, unsigned int dwReg, unsigned int dwVal);

#ifdef __cplusplus
};
#endif
