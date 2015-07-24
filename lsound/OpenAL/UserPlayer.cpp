/**
@file UserPlayer.cpp
@author t-sakai
@date 2014/07/17 create
*/
#include "UserPlayer.h"
#include "Player.h"

namespace lsound
{
    UserPlayer::UserPlayer()
        :flags_(0)
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
        impl_.player_->play();
    }

    void UserPlayer::pause()
    {
        LASSERT(NULL != impl_.player_);
        impl_.player_->pause();
    }

    State UserPlayer::getState()
    {
        LASSERT(NULL != impl_.player_);
        return impl_.player_->getState();
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
