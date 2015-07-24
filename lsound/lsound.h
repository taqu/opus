#ifndef INC_LSOUND_LSOUND_H__
#define INC_LSOUND_LSOUND_H__
/**
@file lsound.h
@author t-sakai
@date 2015/07/06 create
*/
#include <lcore/lcore.h>
#include "lsound_api.h"

#if defined(LSOUND_API_WASAPI)
#include <mmdeviceapi.h>

#elif defined(LSOUND_API_OPENAL)
#include <AL/al.h>
#include <AL/alext.h>

#elif defined(LSOUND_API_OPENSL)
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#endif

namespace lsound
{
//#define LSOUND_USE_WAVE (1)

    using lcore::Char;
    using lcore::s8;
    using lcore::s16;
    using lcore::s32;
    using lcore::s64;

    using lcore::u8;
    using lcore::u16;
    using lcore::u32;
    using lcore::u64;

    using lcore::f32;
    using lcore::f64;

    using lcore::intptr_t;
    using lcore::uintptr_t;
    using lcore::ptrdiff_t;
    using lcore::lsize_t;

    using lcore::ClockType;

    static const s16 NumMaxBuffers = 4;
    static const s32 NumMaxPacks = 8;
    static const s32 BufferNumSamplesPerChannel = 5760;
    static const s32 BufferNumSamples = BufferNumSamplesPerChannel*2;
    static const u16 BitsPerSample = 16;

    enum SampleRate
    {
        SampleRate_11025  = 11025,
        SampleRate_22050  = 22050,
        SampleRate_44100  = 44100,
        SampleRate_48000  = 48000,
        SampleRate_96000  = 96000,
        SampleRate_192000 = 192000,
    };


    enum PlayerFlag
    {
        PlayerFlag_Loop = (0x01U<<0),
    };

#if defined(LSOUND_API_WASAPI)

#define LSOUND_TASKMEMFREE(ptr) if(NULL != (ptr)){CoTaskMemFree((ptr)); (ptr)=NULL;}
#define LSOUND_RELEASE(ptr) if(NULL!=(ptr)){(ptr)->Release(); (ptr)=NULL;}
#define LSOUND_CLOSEHANDLE(handle) if(NULL!=(handle)){CloseHandle(handle); (handle)=NULL;}
#define LSOUND_DITHER_ENABLE 1

    typedef size_t LSsizei;
    typedef s8 LSbyte;
    typedef s16 LSshort;
    typedef s32 LSint;
    typedef u32 LSuint;
    typedef f32 LSfloat;
    typedef s32 LSenum;
    typedef void LSvoid;

    enum PCMFormat
    {
        PCMFormat_Short =0,
    };

    enum Format
    {
        Format_Mono8 = 0,
        Format_Mono16 = 1,
        Format_Stereo8 = 2,
        Format_Stereo16 = 3,
    };

    enum Channels
    {
        Channels_Mono = 1,
        Channels_Stereo = 2,
    };

    enum State
    {
        State_Initial = 0,
        State_Playing = 1,
        State_Paused  = 2,
        State_Stopped = 3,
    };

    static const s32 SharedBufferLength = BufferNumSamplesPerChannel*Channels_Stereo;

#elif defined(LSOUND_API_OPENAL)

#define LSOUND_DITHER_ENABLE 1
    typedef ALsizei LSsizei;
    typedef ALbyte LSbyte;
    typedef ALshort LSshort;
    typedef ALint LSint;
    typedef ALuint LSuint;
    typedef ALfloat LSfloat;
    typedef ALenum LSenum;
    typedef ALvoid LSvoid;

    enum PCMFormat
    {
        PCMFormat_Short =AL_SHORT_SOFT,
    };

    enum Format
    {
        Format_Mono8 = AL_FORMAT_MONO8,
        Format_Mono16 = AL_FORMAT_MONO16,
        Format_Stereo8 = AL_FORMAT_STEREO8,
        Format_Stereo16 = AL_FORMAT_STEREO16,
    };

    enum Channels
    {
        Channels_Mono = AL_MONO_SOFT,
        Channels_Stereo = AL_STEREO_SOFT,
    };

    enum State
    {
        State_Initial = AL_INITIAL,
        State_Playing = AL_PLAYING,
        State_Paused  = AL_PAUSED,
        State_Stopped = AL_STOPPED,
    };

#elif defined(LSOUND_API_OPENSL)

#define LSOUND_SL_DESTROY(ptr) if(NULL!=(ptr)){(*(ptr))->Destroy((ptr)); (ptr)=NULL;}
#define LSOUND_DITHER_ENABLE 1

    typedef SLuint8 LSbyte;
    typedef SLint16 LSshort;
    typedef SLint32 LSint;
    typedef SLuint32 LSuint;
    typedef f32 LSfloat;
    typedef SLint32 LSenum;
    typedef SLresult LSresult;
    typedef void LSvoid;

    enum PCMFormat
    {
        PCMFormat_Short =0,
    };

    enum Format
    {
        Format_Mono8 = 0,
        Format_Mono16 = 1,
        Format_Stereo8 = 2,
        Format_Stereo16 = 3,
    };

    enum Channels
    {
        Channels_Mono = 1,
        Channels_Stereo = 2,
    };

    enum State
    {
        State_Initial = 0,
        State_Playing = SL_PLAYSTATE_PLAYING,
        State_Paused  = SL_PLAYSTATE_PAUSED,
        State_Stopped = SL_PLAYSTATE_STOPPED,
    };

    //typedef f32 SampleType;
    typedef s16 SampleType;
#endif

}
#endif //INC_LSOUND_LSOUND_H__
