#ifndef _H_LIBFORMAT_CONV_H
#define  _H_LIBFORMAT_CONV_H


#if defined(_WIN32) || defined(_WIN64)
#ifndef CALLBACK
#define CALLBACK __stdcall
#endif
#ifdef  LIBFORMATCONV_EXPORTS
#define LIBFORMATCONV_API extern "C" __declspec(dllexport)
#else
#define LIBFORMATCONV_API extern "C" __declspec(dllimport)
#endif
#elif defined (__linux__)
#define LIBFORMATCONV_API extern "C"
#ifndef CALLBACK
#define CALLBACK 
#endif
#else
#define LIBFORMATCONV_API
#ifndef CALLBACK
#define CALLBACK 
#endif
#endif
//cpu
LIBFORMATCONV_API void CALLBACK CPU_yuyv_to_rgb(unsigned char *yuyv, unsigned char *rgb, int width, int height);
LIBFORMATCONV_API void CALLBACK CPU_yu12_to_rgb(unsigned char *yuv, unsigned char *rgb, int width, int height);
LIBFORMATCONV_API void CALLBACK CPU_nv12_to_rgb(unsigned char* nv12, unsigned char* rgb, int width,int height);
LIBFORMATCONV_API void CALLBACK CPU_yuyv_to_nv12(char *yuyv,char *nv12,int width,int height);
LIBFORMATCONV_API void CALLBACK CPU_yuyv_to_yu12(char *yuyv,char *yuv,int width,int height);
LIBFORMATCONV_API void CALLBACK CPU_bayerrg8_to_yu12(unsigned char *bayer, unsigned char *yuv, int width, int height);
LIBFORMATCONV_API void CALLBACK CPU_bayerrg8_to_rgb(unsigned char *bayer, unsigned char *rgb, int width, int height);
//cuda

LIBFORMATCONV_API   void CALLBACK CUDA_rgb_to_yu12(unsigned char* rgb, unsigned char* yuv, int width, int height);
LIBFORMATCONV_API   void CALLBACK CUDA_abgr_to_yu12(unsigned char* abgr, unsigned char* yuv, int width, int height);
LIBFORMATCONV_API   void CALLBACK CUDA_yuyv_to_rgb(unsigned char *yuyv, unsigned char *rgb, int width, int height);
LIBFORMATCONV_API   void CALLBACK CUDA_yu12_to_rgb(unsigned char *yuv, unsigned char *rgb, int width, int height);
LIBFORMATCONV_API   void CALLBACK CUDA_yu12_to_abgr(unsigned char *yuv, unsigned char *abgr, int width, int height);
LIBFORMATCONV_API   void CALLBACK CUDA_bayerrg8_to_yu12(unsigned char *bayer, unsigned char *yuv, int width, int height);
LIBFORMATCONV_API   void CALLBACK CUDA_bayerrg8_to_rgb(unsigned char *bayer, unsigned char *rgb, int width, int height);
#endif
