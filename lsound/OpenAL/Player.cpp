/**
@file Player.cpp
@author t-sakai
@date 2014/07/08 create
*/
#include "Player.h"
#include "Context.h"
#include "../opus/Stream.h"
#include "UserPlayer.h"

namespace lsound
{
    Player::Player()
        :userFlags_(0)
        ,innerFlags_(0)
        ,source_(0)
        ,numBuffers_(0)
        ,stream_(NULL)
        ,userPlayer_(NULL)
    {
        for(s32 i=0; i<NumMaxBuffers; ++i){
            buffers_[i] = 0;
        }
    }

    Player::~Player()
    {
        if(0 != source_){
            clear();
            alDeleteSources(1, &source_);
            source_ = 0;
        }
        if(0<numBuffers_){
            alDeleteBuffers(numBuffers_, buffers_);
            for(s32 i=0; i<NumMaxBuffers; ++i){
                buffers_[i] = 0;
            }
        }
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
        play();
        stop();
        s32 processed = getProcessed();
        LSuint bufids[NumMaxBuffers];
        unqueueBuffers(processed, bufids);
        setPitch(1.0f);
    }

    bool Player::create(s8 numBuffers)
    {
        LASSERT(0<numBuffers && numBuffers<=NumMaxBuffers);

        alGenSources(1, &source_);
        numBuffers_ = numBuffers;
        alGenBuffers(numBuffers_, buffers_);
        return true;
    }

    void Player::update(s32 index, LSuint sampleRate, LSenum internalFormat, LSsizei samples, LSenum channels, LSenum type, const LSvoid* data)
    {
        LASSERT(0<=index && index<numBuffers_);
        Context::updateBuffer(buffers_[index], sampleRate, internalFormat, samples, channels, type, data);
    }
}
