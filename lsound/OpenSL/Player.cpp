/**
@file Player.cpp
@author t-sakai
@date 2015/07/19 create
*/
#include "Player.h"
#include <math.h>
#include "Context.h"
#include "../opus/Stream.h"
#include "UserPlayer.h"

namespace lsound
{
    void Player::callback(SLAndroidSimpleBufferQueueItf caller, void* data)
    {
        lsound::Context& context = lsound::Context::getInstance();
        Player* player = reinterpret_cast<Player*>(data);

        context.enterAPI();
        if(0<player->numQueuedBuffers_){
            SampleType* buffer = NULL;
            u32 size = 0;
            player->dequeue(buffer, size);
            player->bufferQueue_.Enqueue(buffer, size);
        }
        context.leaveAPI();        
    }

    //s16 Player::queuedBuffers_[BufferNumSamples];

    Player::Player()
        :userFlags_(0)
        ,innerFlags_(0)
        ,numBuffers_(0)
        ,prevState_(State_Initial)
        ,maxVolumeLevel_(0)
        ,stream_(NULL)
        ,userPlayer_(NULL)
        ,nextBufferIndex_(0)
        ,numQueuedBuffers_(0)
    {
        clearQueuedBuffers();
    }

    Player::~Player()
    {
        playObject_.Destroy();
        //outputMixObject_.Destroy();
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
        lsound::Context& context = lsound::Context::getInstance();
        context.enterAPI();
        prevState_ = State_Initial;
        play_.SetPlayState(State_Stopped);
        bufferQueue_.Clear();
        clearQueuedBuffers();
        context.leaveAPI();

        if(stream_){
            stream_->seek(0);
        }
    }

    bool Player::create(s32 numBuffers, SLObjectItf playObj)
    {
        numBuffers_ = numBuffers;

        LSresult result;
        //SLObject outObj(outputMixObj);
        //result = outObj.Realize(SL_BOOLEAN_FALSE);
        //if(result != SL_RESULT_SUCCESS){
        //    lcore::Log("error: OutputMixObj::Realize");
        //    return false;
        //}

        SLObject pObj(playObj);
        result = pObj.Realize(SL_BOOLEAN_FALSE);
        if(result != SL_RESULT_SUCCESS){
            lcore::Log("error: PlayObj::Realize");
            return false;
        }

        SLPlayItf play = NULL;
        result = pObj.GetInterface(SL_IID_PLAY, &play);
        if(result != SL_RESULT_SUCCESS){
            lcore::Log("error: GetInterface:SL_IID_PLAY");
            return false;
        }

        SLAndroidSimpleBufferQueueItf bufferQueue = NULL;
        result = pObj.GetInterface(SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &bufferQueue);
        if(result != SL_RESULT_SUCCESS){
            lcore::Log("error: GetInterface:SL_IID_ANDROIDSIMPLEBUFFERQUEUE");
            return false;
        }

        SLVolumeItf volume = NULL;
        result = pObj.GetInterface(SL_IID_VOLUME, &volume);
        if(result != SL_RESULT_SUCCESS){
            lcore::Log("error: GetInterface:SL_IID_VOLUME");
            return false;
        }

        //outputMixObject_.swap(outObj);
        playObject_.swap(pObj);
        SLPlay(play).swap(play_);
        SLBufferQueue(bufferQueue).swap(bufferQueue_);
        SLVolume(volume).swap(volume_);

        bufferQueue_.RegisterCallback(Player::callback, this);

        if(volume_.valid()){
            result = volume_.GetMaxVolumeLevel(&maxVolumeLevel_);
        }
        return true;
    }

    void Player::setStream(Stream* stream)
    {
        stream_ = stream;
    }

    void Player::rewind()
    {
        lsound::Context& context = lsound::Context::getInstance();
        context.enterAPI();
        play_.SetPlayState(State_Stopped);
        bufferQueue_.Clear();
        clearQueuedBuffers();
        context.leaveAPI();
        if(stream_){
            stream_->seek(0);
        }
    }

    void Player::play()
    {
        lsound::Context& context = lsound::Context::getInstance();
        context.enterAPI();
        play_.SetPlayState(State_Playing);
        context.leaveAPI();
    }

    void Player::pause()
    {
        lsound::Context& context = lsound::Context::getInstance();
        context.enterAPI();
        play_.SetPlayState(State_Paused);
        context.leaveAPI();
    }

    void Player::stop()
    {
        rewind();
    }

    State Player::getState()
    {
        lsound::Context& context = lsound::Context::getInstance();
        context.enterAPI();

        SLuint32 state = 0;
        play_.GetPlayState(&state);
        context.leaveAPI();

        return static_cast<lsound::State>(state);
    }

    void Player::setPosition(f32 x, f32 y, f32 z)
    {
        //alSource3f(source_, AL_POSITION, x, y, z);
    }

    void Player::setGain(f32 gain)
    {
        if(!volume_.valid()){
            return;
        }
        static const f32 InvLog10 = static_cast<f32>(0.4342944819032518276511289189166);
        SLmillibel level = static_cast<SLmillibel>(logf(gain)*InvLog10*1000.0f);
        level = lcore::clamp(level, SL_MILLIBEL_MIN, maxVolumeLevel_);

        //lcore::Log("set level %d", level);
        lsound::Context& context = lsound::Context::getInstance();
        context.enterAPI();
        volume_.SetVolumeLevel(level);
        context.leaveAPI();
    }

    void Player::setPitch(f32 pitch)
    {
        //alSourcef(source_, AL_PITCH, pitch);
    }

    void Player::setRelative(s32 enable)
    {
        //alSourcei(source_, AL_SOURCE_RELATIVE, enable);
    }

    void Player::setRolloff(s32 rolloff)
    {
        //alSourcei(source_, AL_ROLLOFF_FACTOR, rolloff);
    }

    s32 Player::getQueued()
    {
        lsound::Context& context = lsound::Context::getInstance();
        context.enterAPI();

        SLAndroidSimpleBufferQueueState state;

        if(SL_RESULT_SUCCESS != bufferQueue_.GetState(&state)){
            state.count = 0;
        }
        context.leaveAPI();
        return static_cast<s32>(state.count);
    }

    u32 Player::getProcessed()
    {
        lsound::Context& context = lsound::Context::getInstance();
        context.enterAPI();

        SLAndroidSimpleBufferQueueState state;

        if(SL_RESULT_SUCCESS != bufferQueue_.GetState(&state)){
            state.index = 0;
        }
        context.leaveAPI();
        return state.index;
    }

    u32 Player::getSampleOffset()
    {
        //s32 samples = 0;
        //alGetSourcei(source_, AL_SAMPLE_OFFSET, &samples);
        //return static_cast<u32>(samples);
        return 0;
    }

    bool Player::isSampleEnd()
    {
        lsound::Context& context = lsound::Context::getInstance();
        bool ret = (stream_->getTotal()<=stream_->getPosition());
        return ret;
    }

    s32 Player::fillBuffer(bool isLoop)
    {
        SampleType* buffer = buffers_[nextBufferIndex_];
        s32 requestSamples = BufferNumSamplesPerChannel;
        s32 readSamples = 0;
        for(s32 i=0; i<7; ++i){
            s32 s;
#if 1
            if(NumChannels == 1){
                s = stream_->read(buffer, requestSamples);
            }else{
                s = stream_->read_stereo(buffer, requestSamples);
            }
#else
            if(NumChannels == 1){
                s = stream_->read_float(buffer, requestSamples);
            }else{
                s = stream_->read_float_stereo(buffer, requestSamples);
            }
#endif
            if(0<s){
                buffer += s*NumChannels;
                readSamples += s;
                requestSamples -= s;
            }

            if(stream_->getTotal()<=stream_->getPosition()){
                //全て処理した
                //ループなら先頭にシーク
                if(isLoop){
                    stream_->seek(0);
                } else{
                    if(readSamples<=0){
                        readSamples = -1;
                    }
                    break;
                }
            }
            if(BufferNumSamplesPerChannel<=readSamples){
                break;
            }
        }
        return readSamples;
    }

    bool Player::initialize()
    {
        if(NULL != userPlayer_){
            userPlayer_->lock_.enter();
        }

        u16 userFlags = (NULL == userPlayer_)? userFlags_ : userPlayer_->flags_;
        //u32 bufferStart = getProcessed() & BufferRoundMask;

        static const s16 InitNumBuffers = 1;
        s32 requestBuffers = lcore::minimum(InitNumBuffers, numBuffers_);

        lsound::Context& context = lsound::Context::getInstance();

        for(s32 i=0; i<requestBuffers; ++i){
            s32 readSamples = fillBuffer(userFlags & PlayerFlag_Loop);
            if(readSamples<=0){
                break;
            }

            
            context.enterAPI();
            bufferQueue_.Enqueue(buffers_[nextBufferIndex_], sizeof(SampleType)*readSamples*NumChannels);
            //enqueue(sizeof(SampleType)*readSamples*NumChannels);
            advanceBufferIndex();
            context.leaveAPI();
        }

        prevState_ = State_Initial;
        if(NULL != userPlayer_){
            if(userPlayer_->state_ == State_Initial || userPlayer_->state_ == State_Playing){
                play();
            }else{
                pause();
            }
            userPlayer_->initialized_ = 1;
            userPlayer_->lock_.leave();
        }else{
            play();
        }
        return true;
    }

    void Player::clearQueuedBuffers()
    {
        nextBufferIndex_ = 0;
        numQueuedBuffers_ = 0;
        //for(s32 i=0; i<NumMaxBuffers; ++i){
        //    queuedBuffers_[i].index_ = 0;
        //    queuedBuffers_[i].size_ = 0;
        //}
    }

    void Player::enqueue(u32 size)
    {
        LASSERT(numQueuedBuffers_<numBuffers_);

        queuedBuffers_[numQueuedBuffers_].index_ = nextBufferIndex_;
        queuedBuffers_[numQueuedBuffers_].size_ = size;

        advanceBufferIndex();

        ++numQueuedBuffers_;
    }

    void Player::dequeue(SampleType*& buffer, u32& size)
    {
        LASSERT(0<numQueuedBuffers_);

        buffer = buffers_[queuedBuffers_[0].index_];
        size = queuedBuffers_[0].size_;
        for(s32 i=1; i<numQueuedBuffers_; ++i){
            queuedBuffers_[i-1] = queuedBuffers_[i];
        }
        --numQueuedBuffers_;
    }

    void Player::systemPause(bool pause)
    {
        lsound::Context& context = lsound::Context::getInstance();
        context.enterAPI();
        if(pause){
            if(prevState_ == State_Initial){
                SLuint32 state = 0;
                play_.GetPlayState(&state);

                prevState_ = static_cast<s16>(state);
                play_.SetPlayState(State_Paused);
            }
        }else{
            play_.SetPlayState(prevState_);
            prevState_ = State_Initial;
        }
        context.leaveAPI();
    }

    bool Player::update()
    {
        if(NULL == stream_){
            return false;
        }
        u16 userFlags = getUserFlags();

        lsound::Context& context = lsound::Context::getInstance();

        s32 queuedBuffers = 0;
        s32 enqueuedBuffers = 0;
        SLAndroidSimpleBufferQueueState state;
        context.enterAPI();
        if(SL_RESULT_SUCCESS == bufferQueue_.GetState(&state)){
            enqueuedBuffers = static_cast<s32>(state.count);
        }
        queuedBuffers = numQueuedBuffers_;
        context.leaveAPI();

        if(stream_->getTotal()<=stream_->getPosition()){
            //全て処理した
            //ループなら先頭にシーク
            if(userFlags & PlayerFlag_Loop){
                stream_->seek(0);

            } else {
                if(enqueuedBuffers<=0 && queuedBuffers<=0){
                    context.enterAPI();
                    play_.SetPlayState(State_Stopped);
                    context.leaveAPI();
                    return false;
                }
                return true;
            }
        }

        s32 requestBuffers = numBuffers_ - queuedBuffers;
        for(s32 i=0; i<requestBuffers; ++i){
            s32 readSamples = fillBuffer(userFlags & PlayerFlag_Loop);
            if(readSamples<=0){
                break;
            }
            context.enterAPI();
           // bufferQueue_.Enqueue(queuedBuffers_, sizeof(SampleType)*readSamples*NumChannels);
            enqueue(sizeof(SampleType)*readSamples*NumChannels);
            context.leaveAPI();
        }
        return true;
    }
}
