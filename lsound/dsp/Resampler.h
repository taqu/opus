#ifndef INC_LSOUND_RESAMPLER_H__
#define INC_LSOUND_RESAMPLER_H__
/**
@file Resampler.h
@author t-sakai
@date 2015/07/09 create
*/
#include "../lsound.h"
#include "dsp.h"
#include <speex/speex_resampler.h>

namespace lsound
{
    class Stream;

    class Resampler
    {
    public:
        static const s32 MaxTableHalfLength = 8;
        static const s32 MaxTableLength = (MaxTableHalfLength*2+1);

        Resampler();
        ~Resampler();

        void setSource(
            u16 srcNumChannels,
            u16 srcBytesPerSample,
            s32 srcSamplesPerSec);

        void setDest(
            u16 dstNumChannels,
            u16 dstBytesPerSample,
            s32 dstSamplesPerSec);

        void initialize();
        void reset();

        inline u16 getSrcNumChannels() const;
        inline u16 getSrcBytesPerSample() const;
        inline s32 getSrcSamplesPerSec() const;

        inline u16 getDstNumChannels() const;
        inline u16 getDstBytesPerSample() const;
        inline s32 getDstSamplesPerSec() const;

        s32 read(LSshort* pcm, u32 numSamples, Stream* stream);
        s32 read(LSfloat* pcm, u32 numSamples, Stream* stream);
    private:
        Resampler(const Resampler&);
        Resampler& operator=(const Resampler&);

        void destroyResampler();

        ConvTypeFunc convertTypeFunc_;

        u16 srcNumChannels_;
        u16 srcBytesPerSample_;
        s32 srcSamplesPerSec_;
        u16 dstNumChannels_;
        u16 dstBytesPerSample_;
        s32 dstSamplesPerSec_;

        s32 resamplerChannels_;
        f32 resampleRate_;
        SpeexResamplerState* resampler_;
        static LSfloat sharedBuffer0_[SharedBufferLength*2];
        static LSfloat sharedBuffer1_[SharedBufferLength*2];
    };

    inline u16 Resampler::getSrcNumChannels() const
    {
        return srcNumChannels_;
    }

    inline u16 Resampler::getSrcBytesPerSample() const
    {
        return srcBytesPerSample_;
    }

    inline s32 Resampler::getSrcSamplesPerSec() const
    {
        return srcSamplesPerSec_;
    }

    inline u16 Resampler::getDstNumChannels() const
    {
        return dstNumChannels_;
    }

    inline u16 Resampler::getDstBytesPerSample() const
    {
        return dstBytesPerSample_;
    }

    inline s32 Resampler::getDstSamplesPerSec() const
    {
        return dstSamplesPerSec_;
    }
}
#endif //INC_LSOUND_RESAMPLER_H__
