/**
@file Player.cpp
@author t-sakai
@date 2014/07/08 create
*/
#include "Player.h"
#include <emmintrin.h>

#include "../opus/Stream.h"
#include "UserPlayer.h"


namespace lsound
{
    LSfloat Player::sharedBuffer_[SharedBufferLength];


    Player::Player()
        :userFlags_(0)
        ,innerFlags_(0)
        ,bufferFrames_(0)
        ,halfBufferFrames_(0)
        ,state_(State_Initial)
        ,stream_(NULL)
        ,audioClient_(NULL)
        ,audioRenderClient_(NULL)
        ,audioClock_(NULL)
        ,audioStreamVolume_(NULL)
        //,fillEvent_(NULL)
        ,userPlayer_(NULL)
    {
    }

    Player::~Player()
    {
        destroy();
    }

    void Player::destroy()
    {
        LSOUND_RELEASE(audioStreamVolume_);
        LSOUND_RELEASE(audioClock_);
        LSOUND_RELEASE(audioRenderClient_);
        //LSOUND_CLOSEHANDLE(fillEvent_);
        LSOUND_RELEASE(audioClient_);
    }

    u16 Player::getUserFlags()
    {
        if(NULL == userPlayer_){
            return userFlags_;
        }

        lcore::CSLock lock(userPlayer_->lock_);
        return userPlayer_->flags_;
    }

    void Player::clear()
    {
        if(stream_){
            stream_->seek(0);
        }
        audioClient_->Stop();
        audioClient_->Reset();
        state_ = State_Initial;
    }

    bool Player::create(s8 numBuffers, IAudioClient* audioClient, const WAVEFORMATEX& fmt)
    {
        LASSERT(0<numBuffers && numBuffers<=NumMaxBuffers);
        LASSERT(NULL != audioClient);

        destroy();
        audioClient_ = audioClient;

        resampler_.setDest(fmt.nChannels, fmt.wBitsPerSample/8, fmt.nSamplesPerSec);

        HRESULT hr;
        f64 duration = 1000.0 * 1000.0 * 10.0*((f64)BufferNumSamplesPerChannel/resampler_.getDstSamplesPerSec());
        REFERENCE_TIME requestedDuration = static_cast<REFERENCE_TIME>(duration*numBuffers + 0.5);

        hr = audioClient_->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            AUDCLNT_STREAMFLAGS_NOPERSIST,//AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST,
            requestedDuration,
            0,
            &fmt,
            NULL);
        if(FAILED(hr)){
            destroy();
            return false;
        }

        //fillEvent_ = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE|SYNCHRONIZE);
        //if(NULL == fillEvent_){
        //    destroy();
        //    return false;
        //}
        //audioClient_->SetEventHandle(fillEvent_);

        hr = audioClient_->GetService(IID_IAudioRenderClient, (void**)&audioRenderClient_);
        if(FAILED(hr)){
            destroy();
            return false;
        }

        hr = audioClient_->GetService(IID_IAudioClock, (void**)&audioClock_);
        if(FAILED(hr)){
            destroy();
            return false;
        }

        hr = audioClient_->GetService(IID_IAudioStreamVolume, (void**)&audioStreamVolume_);
        if(FAILED(hr)){
            destroy();
            return false;
        }

        bufferFrames_ = 0;
        audioClient_->GetBufferSize(&bufferFrames_);
        halfBufferFrames_ = bufferFrames_>>1;
        return true;
    }

    void Player::rewind()
    {
        audioClient_->Stop();
        audioClient_->Reset();
        if(stream_){
            stream_->seek(0);
        }
        resampler_.reset();
        state_ = State_Stopped;
    }

    void Player::play()
    {
        audioClient_->Start();
        state_ = State_Playing;
    }

    void Player::pause()
    {
        audioClient_->Stop();
        state_ = State_Paused;
    }

    void Player::stop()
    {
        rewind();
    }

    State Player::getState()
    {
        return (State)state_;
    }

    void Player::setState(s32 state)
    {
        state_ = state;
    }


    u32 Player::getSampleOffset()
    {
        UINT64 position = 0;
        audioClock_->GetPosition(&position, NULL);
        return static_cast<u32>(position);
    }

    void Player::setPosition(f32 x, f32 y, f32 z)
    {
    }

    void Player::setGain(f32 gain)
    {
        f32 volumes[] = {gain, gain};
        audioStreamVolume_->SetAllVolumes(2, volumes);
    }

    void Player::setPitch(f32 pitch)
    {
    }

    void Player::setStream(Stream* stream)
    {
        stream_ = stream;
        if(NULL == stream_){
            return;
        }

        resampler_.setSource(stream_->getChannels(), stream_->getBitsPerSample()/8, stream_->getSampleRate());
        resampler_.initialize();
    }


    u32 Player::fillShort(LSshort* pcm, u32 requestFrames, u16 userFlags)
    {
        u32 readFrames = 0;
        u32 frames = requestFrames;
        for(s32 i=0; i<4; ++i){
            s32 s = resampler_.read(pcm, frames, stream_);
            if(s<0){
                break;
            }
            readFrames += s;
            if(stream_->getTotal()<=stream_->getPosition()){
                //全て処理した
                //ループなら先頭にシーク
                if(userFlags & PlayerFlag_Loop){
                    stream_->seek(0);
                }
            }

            if(requestFrames<=readFrames){
                break;
            }
            frames -= s;
            pcm += s*resampler_.getDstNumChannels();
        }
        return readFrames;
    }

    u32 Player::fillFloat(LSfloat* pcm, u32 requestFrames, u16 userFlags)
    {
        u32 readFrames = 0;
        u32 frames = requestFrames;
        for(s32 i=0; i<4; ++i){
            s32 s = resampler_.read(pcm, frames, stream_);
            if(s<0){
                break;
            }
            readFrames += s;
            if(stream_->getTotal()<=stream_->getPosition()){
                //全て処理した
                //ループなら先頭にシーク
                if(userFlags & PlayerFlag_Loop){
                    stream_->seek(0);
                }
            }

            if(requestFrames<=readFrames){
                break;
            }
            frames -= s;
            pcm += s*resampler_.getDstNumChannels();
        }
        return readFrames;
    }

    void Player::copyFromSharedBuffer(u32 sizeInByte, BYTE* d)
    {
        s32 num = sizeInByte>>4; //16byte単位でコピー
        s32 remain = sizeInByte - (num << 4);

        const f32* src = sharedBuffer_;
        f32* dst = reinterpret_cast<f32*>(d);
        for(s32 i=0; i<num; ++i){
            _mm_storeu_ps(dst, _mm_load_ps(src));
            src += 4;
            dst += 4;
        }

        const BYTE* bsrc = reinterpret_cast<const BYTE*>(src);
        BYTE* bdst = reinterpret_cast<BYTE*>(dst);
        for(s32 i=0; i<remain; ++i){
            bdst[i] = bsrc[i];
        }
    }

    bool Player::initialize()
    {
        if(NULL != userPlayer_){
            userPlayer_->lock_.enter();
        }
        u16 userFlags = (NULL == userPlayer_)? userFlags_ : userPlayer_->flags_;
        u32 requestFrames = lcore::minimum((u32)BufferNumSamplesPerChannel, bufferFrames_);
        u32 readFrames = 0;
        switch(resampler_.getDstBytesPerSample())
        {
        case 2:
            {
                LSshort* pcm = reinterpret_cast<LSshort*>(sharedBuffer_);
                readFrames = fillShort(pcm, requestFrames, userFlags);
            }
            break;
        case 4:
            {
                LSfloat* pcm = reinterpret_cast<LSfloat*>(sharedBuffer_);
                readFrames = fillFloat(pcm, requestFrames, userFlags);
            }
            break;
        }

        u32 sizeInByte = readFrames*resampler_.getDstBytesPerSample()*resampler_.getDstNumChannels();

        BYTE* data = NULL;
        HRESULT hr = audioRenderClient_->GetBuffer(readFrames, &data);
        if(FAILED(hr)){
            if(NULL != userPlayer_){
                userPlayer_->lock_.leave();
            }
            return false;
        }
        
        copyFromSharedBuffer(sizeInByte, data);
        audioRenderClient_->ReleaseBuffer(readFrames, 0);

        if(NULL != userPlayer_){
            if(userPlayer_->state_ == State_Initial || userPlayer_->state_ == State_Playing){
                audioClient_->Start();
                state_ = State_Playing;
            }else{
                audioClient_->Stop();
                state_ = State_Paused;
            }
            userPlayer_->initialized_ = 1;
            userPlayer_->lock_.leave();
        }else{
            audioClient_->Start();
                state_ = State_Playing;
        }
        return true;
    }

    bool Player::update()
    {
        State state = getState();
        switch(state)
        {
        case State_Initial:
        case State_Stopped:
            return false;
        }

        u32 numFramesPadding = 0;
        HRESULT hr = audioClient_->GetCurrentPadding(&numFramesPadding);
        if(FAILED(hr)){
            return false;
        }

        u16 userFlags = getUserFlags();
        if(stream_->getTotal()<=stream_->getPosition()
            && numFramesPadding<=0)
        {
            //全て処理した
            //ループなら先頭にシーク
            if(userFlags & PlayerFlag_Loop){
                stream_->seek(0);
            }else{
                return false;
            }
        }

        u32 requestFrames = bufferFrames_ - numFramesPadding;
        if(requestFrames<halfBufferFrames_){
            return true;
        }
        u32 readFrames = 0;
        switch(resampler_.getDstBytesPerSample())
        {
        case 2:
            {
                LSshort* pcm = reinterpret_cast<LSshort*>(sharedBuffer_);
                readFrames = fillShort(pcm, requestFrames, userFlags);
            }
            break;
        case 4:
            {
                LSfloat* pcm = reinterpret_cast<LSfloat*>(sharedBuffer_);
                readFrames = fillFloat(pcm, requestFrames, userFlags);
            }
            break;
        }

        u32 sizeInByte = readFrames*resampler_.getDstBytesPerSample()*resampler_.getDstNumChannels();

        BYTE* data = NULL;
        hr = audioRenderClient_->GetBuffer(requestFrames, &data);
        if(FAILED(hr)){
            return false;
        }
        copyFromSharedBuffer(sizeInByte, data);
        audioRenderClient_->ReleaseBuffer(readFrames, 0);
        return true;
    }
}
