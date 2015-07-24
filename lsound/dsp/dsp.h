#ifndef INC_LSOUND_DSP_H__
#define INC_LSOUND_DSP_H__
/**
@file dsp.h
@author t-sakai
@date 2015/07/08 create
*/
#include "../lsound.h"

namespace lsound
{
    static inline LSfloat toFloat(LSbyte v)
    {
        return (1.0f/127.0f)*v;
    }

    static inline LSfloat toFloat(LSshort v)
    {
        return (1.0f/32767.0f)*v;
    }

    static inline LSshort toShort(LSbyte v)
    {
        return static_cast<LSshort>((32768.0f/128.0f)*v);
    }

    static inline LSshort toShort(LSfloat v)
    {
        return static_cast<LSshort>(32768.0f*v);
    }

    static inline LSbyte toByte(LSshort v)
    {
        return static_cast<LSshort>((128.0f/32768.0f)*v);
    }

    static inline LSbyte toByte(LSfloat v)
    {
        return static_cast<LSshort>(128.0f*v);
    }

    typedef void(*ConvTypeFunc)(void* dst, const void* s, s32 numSamples);

    void conv_Short1ToShort2(void* dst, const void* src, s32 numSamples);
    void conv_Short2ToShort1(void* dst, const void* src, s32 numSamples);

    void conv_Float1ToFloat2(void* dst, const void* src, s32 numSamples);
    void conv_Float2ToFloat1(void* dst, const void* src, s32 numSamples);



    void conv_Short1ToByte1(void* dst, const void* src, s32 numSamples);
    void conv_Short2ToByte2(void* dst, const void* src, s32 numSamples);
    void conv_Short1ToByte2(void* dst, const void* src, s32 numSamples);
    void conv_Short2ToByte1(void* dst, const void* src, s32 numSamples);

    void conv_Short1ToFloat1(void* dst, const void* src, s32 numSamples);
    void conv_Short2ToFloat2(void* dst, const void* src, s32 numSamples);
    void conv_Short1ToFloat2(void* dst, const void* src, s32 numSamples);
    void conv_Short2ToFloat1(void* dst, const void* src, s32 numSamples);

    void conv_Float1ToByte1(void* dst, const void* src, s32 numSamples);
    void conv_Float2ToByte2(void* dst, const void* src, s32 numSamples);
    void conv_Float1ToByte2(void* dst, const void* src, s32 numSamples);
    void conv_Float2ToByte1(void* dst, const void* src, s32 numSamples);

    void conv_Float1ToShort1(void* dst, const void* src, s32 numSamples);
    void conv_Float2ToShort2(void* dst, const void* src, s32 numSamples);
    void conv_Float1ToShort2(void* dst, const void* src, s32 numSamples);
    void conv_Float2ToShort1(void* dst, const void* src, s32 numSamples);

    ConvTypeFunc getConvTypeFunc(u16 dstNumChannels, u16 dstBytesPerSample, u16 srcNumChannels, u16 srcBytesPerSample);
}
#endif //INC_LSOUND_DSP_H__
