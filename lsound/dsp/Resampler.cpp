/**
@file Resampler.cpp
@author t-sakai
@date 2015/07/09 create
*/
#include "Resampler.h"
#include "../opus/Stream.h"

namespace lsound
{
    LSfloat Resampler::sharedBuffer0_[SharedBufferLength*2];
    LSfloat Resampler::sharedBuffer1_[SharedBufferLength*2];

    Resampler::Resampler()
        :convertTypeFunc_(NULL)
        ,srcNumChannels_(0)
        ,srcBytesPerSample_(0)
        ,srcSamplesPerSec_(0)
        ,dstNumChannels_(0)
        ,dstBytesPerSample_(0)
        ,dstSamplesPerSec_(0)
        ,resamplerChannels_(0)
        ,resampler_(NULL)
    {
    }

    Resampler::~Resampler()
    {
        destroyResampler();
    }

    void Resampler::setSource(
        u16 srcNumChannels,
        u16 srcBytesPerSample,
        s32 srcSamplesPerSec)
    {
        srcNumChannels_ = srcNumChannels;
        srcBytesPerSample_ = srcBytesPerSample;
        srcSamplesPerSec_ = srcSamplesPerSec;
    }

    void Resampler::setDest(
        u16 dstNumChannels,
        u16 dstBytesPerSample,
        s32 dstSamplesPerSec)
    {
        dstNumChannels_ = dstNumChannels;
        dstBytesPerSample_ = dstBytesPerSample;
        dstSamplesPerSec_ = dstSamplesPerSec;
    }

    void Resampler::initialize()
    {
        if(dstSamplesPerSec_ == srcSamplesPerSec_){
            convertTypeFunc_ = getConvTypeFunc(
                dstNumChannels_,
                dstBytesPerSample_,
                srcNumChannels_,
                srcBytesPerSample_);

        }else{
            if(resamplerChannels_ != srcNumChannels_){
                destroyResampler();
            }
            if(NULL == resampler_){
                resampler_ = speex_resampler_init(srcNumChannels_, srcSamplesPerSec_, dstSamplesPerSec_, SPEEX_RESAMPLER_QUALITY_DEFAULT, NULL);
            }else{
                speex_resampler_set_rate(resampler_, srcSamplesPerSec_, dstSamplesPerSec_);
                speex_resampler_reset_mem(resampler_);
            }

            resampleRate_ = static_cast<f32>(dstSamplesPerSec_)/srcSamplesPerSec_;

            convertTypeFunc_ = getConvTypeFunc(
                dstNumChannels_,
                dstBytesPerSample_,
                srcNumChannels_,
                dstBytesPerSample_);
        }
    }

    void Resampler::reset()
    {
        if(NULL != resampler_){
            speex_resampler_reset_mem(resampler_);
        }
    }

    s32 Resampler::read(LSshort* pcm, u32 numSamples, Stream* stream)
    {
        if(dstSamplesPerSec_ == srcSamplesPerSec_){

            if(NULL != convertTypeFunc_){
                LSshort* buffer = reinterpret_cast<LSshort*>(sharedBuffer0_);
                s32 readSamples = stream->read(buffer, numSamples);
                if(readSamples<0){
                    return readSamples;
                }
                convertTypeFunc_(pcm, buffer, readSamples);
                return readSamples;

            } else{
                return stream->read(pcm, numSamples);
            }

        }else{
            LSshort* buffer0 = reinterpret_cast<LSshort*>(sharedBuffer0_);
            s32 readSamples = stream->read(buffer0, numSamples);
            if(readSamples<0){
                return readSamples;
            }

            u32 inN = numSamples;
            u32 outN = static_cast<u32>(resampleRate_*numSamples);
            if(NULL != convertTypeFunc_){
                LSshort* buffer1 = reinterpret_cast<LSshort*>(sharedBuffer1_);
                speex_resampler_process_interleaved_int(resampler_, buffer0, &inN, buffer1, &outN);
                convertTypeFunc_(pcm, buffer1, outN);
            }else{
                speex_resampler_process_interleaved_int(resampler_, buffer0, &inN, pcm, &outN);
            }
            return static_cast<s32>(outN);
        }
    }

    s32 Resampler::read(LSfloat* pcm, u32 numSamples, Stream* stream)
    {
        if(dstSamplesPerSec_ == srcSamplesPerSec_){
            if(NULL != convertTypeFunc_){
                s32 readSamples = stream->read_float(sharedBuffer0_, numSamples);

                if(readSamples<0){
                    return readSamples;
                }
                convertTypeFunc_(pcm, sharedBuffer0_, readSamples);
                return readSamples;

            } else{
                return stream->read_float(pcm, numSamples);
            }

        }else{
            s32 readSamples = stream->read_float(sharedBuffer0_, numSamples);
            if(readSamples<0){
                return readSamples;
            }

            u32 inN = readSamples;
            u32 outN = static_cast<u32>(resampleRate_*readSamples);
            if(NULL != convertTypeFunc_){
                speex_resampler_process_interleaved_float(resampler_, sharedBuffer0_, &inN, sharedBuffer1_, &outN);
                convertTypeFunc_(pcm, sharedBuffer1_, outN);
            }else{
                speex_resampler_process_interleaved_float(resampler_, sharedBuffer0_, &inN, pcm, &outN);
            }
            return static_cast<s32>(outN);
        }
    }

    void Resampler::destroyResampler()
    {
        if(NULL != resampler_){
            speex_resampler_destroy(resampler_);
            resampler_ = NULL;
        }
    }
}
