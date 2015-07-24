/**
@file UserPlayer.cpp
@author t-sakai
@date 2015/07/07 create
*/
#include "UserPlayer.h"
#include "Player.h"

namespace lsound
{
    UserPlayer::UserPlayer()
        :flags_(0)
        ,initialized_(0)
        ,state_(State_Initial)
    {
        impl_.player_ = NULL;
    }

    UserPlayer::~UserPlayer()
    {
    }

    bool UserPlayer::checkFlag(PlayerFlag flag)
    {
        lcore::CSLock lock(lock_);
        return 0 != (flags_ & flag);
    }

    void UserPlayer::setFlag(PlayerFlag flag)
    {
        lcore::CSLock lock(lock_);
        flags_ |= flag;
    }

    void UserPlayer::resetFlag(PlayerFlag flag)
    {
        lcore::CSLock lock(lock_);
        flags_ &= ~flag;
    }

    void UserPlayer::play()
    {
        LASSERT(NULL != impl_.player_);
        lcore::CSLock lock(lock_);
        if(initialized_){
            impl_.player_->play();
        }else{
            state_ = State_Playing;
        }
    }

    void UserPlayer::pause()
    {
        LASSERT(NULL != impl_.player_);
        lcore::CSLock lock(lock_);
        if(initialized_){
            impl_.player_->pause();
        }else{
            state_ = State_Paused;
        }
    }

    State UserPlayer::getState()
    {
        LASSERT(NULL != impl_.player_);
        lcore::CSLock lock(lock_);
        if(initialized_){
            return impl_.player_->getState();
        }else{
            return static_cast<State>(state_);
        }
    }

    void UserPlayer::setGain(f32 gain)
    {
        impl_.player_->setGain(gain);
    }

    void UserPlayer::setPitch(f32 pitch)
    {
        impl_.player_->setPitch(pitch);
    }
}
