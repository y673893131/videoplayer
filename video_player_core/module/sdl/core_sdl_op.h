#ifndef CORE_SDL_OP_H
#define CORE_SDL_OP_H

#include "../sample/core_audio_sample.h"

#ifdef unix
    typedef signed char         int8_t;
    typedef signed short        int16_t;
    typedef signed int          int32_t;
    typedef unsigned char       uint8_t;
    typedef unsigned short      uint16_t;
    typedef unsigned int        uint32_t;
    typedef unsigned long       DWORD;
    typedef int                 BOOL;
    typedef unsigned char       BYTE;
    typedef unsigned short      WORD;
    typedef float               FLOAT;
    typedef FLOAT               *PFLOAT;
    typedef int                 INT;
    typedef unsigned int        UINT;
    typedef unsigned int        *PUINT;

    typedef unsigned long ULONG_PTR, *PULONG_PTR;
    typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;
    #define MAKEWORD(a, b)      ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
    #define MAKELONG(a, b)      ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
    #define LOWORD(l)           ((WORD)(((DWORD_PTR)(l)) & 0xffff))
    #define HIWORD(l)           ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
    #define LOBYTE(w)           ((BYTE)(((DWORD_PTR)(w)) & 0xff))
    #define HIBYTE(w)           ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))
#endif

typedef void (*audioCallback)(void*, unsigned char*, int);
class core_thread_audio;
class core_sdl_op
{
public:
    core_sdl_op();
    virtual ~core_sdl_op();

    bool initResample(AVCodecContext* ctx);
    bool init(audioCallback cb, void* userdata);
    void setVol(int vol);
    int getVol();

    void resampleFrame(AVFrame*, int& bufferSize);

    int initSDL();
    int openSDL();
    void closeSDL();
    void startSDL();
    void pauseSDL();

private:
    int nAudioId;
    AVFrame* pFrameReSample;
    core_audio_sample in;
    core_audio_sample out;
    SDL_AudioSpec spec;
    SDL_AudioSpec target;
    DECLARE_ALIGNED(16,uint8_t,buff) [192000 * 4];
    unsigned int nBuffIndex;
    unsigned int nBuffSize;
    float fVolPercent;
    int nVol;
    SwrContext *swrCtx;

    friend class core_thread_audio;
};

#endif // CORE_SDL_OP_H
