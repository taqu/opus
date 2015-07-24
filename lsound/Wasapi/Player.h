#ifndef INC_LSOUND_WASAPI_PLAYER_H__
#define INC_LSOUND_WASAPI_PLAYER_H__
/**
@file Player.h
@author t-sakai
@date 2015/07/06 create
*/
#include "../lsound.h"
#include <Audioclient.h>
#include "../dsp/dsp.h"
#include "../dsp/Resampler.h"

namespace lsound
{
    class Stream;
    class Player;
    class UserPlayer;

    class PlayerLink
    {
    public:
        PlayerLink()
            :prev_(this)
            ,next_(this)
        {}

        ~PlayerLink()
        {}

        inline void resetLink();
        inline void unlink();
        inline void link(PlayerLink* node);

        inline Player* getPrev();
        inline void setPrev(PlayerLink* prev);

        inline Player* getNext();
        inline void setNext(PlayerLink* next);

    protected:
        PlayerLink* prev_;
        PlayerLink* next_;
    };

    inline void PlayerLink::resetLink()
    {
        prev_ = next_ = this;
    }

    inline void PlayerLink::unlink()
    {
        next_->prev_ = prev_;
        prev_->next_ = next_;
        prev_ = next_ = this;
    }

    inline void PlayerLink::link(PlayerLink* node)
    {
        prev_ = node->prev_;
        next_ = node;
        prev_->next_ = node->prev_ = this;
    }

    inline Player* PlayerLink::getPrev()
    {
        return reinterpret_cast<Player*>(prev_);
    }

    inline void PlayerLink::setPrev(PlayerLink* prev)
    {
        prev_ = prev;
    }

    inline Player* PlayerLink::getNext()
    {
        return reinterpret_cast<Player*>(next_);
    }

    inline void PlayerLink::setNext(PlayerLink* next)
    {
        next_ = next;
    }

    //--------------------------------------------------------
    //---
    //--- Player
    //---
    //--------------------------------------------------------
    class Player : public PlayerLink
    {
    public:
        static const s16 NumMaxBuffers = lsound::NumMaxBuffers;

        enum InnerFlag
        {
            InnerFlag_UserPlayer = (0x01U<<0),
        };

        Player();
        ~Player();
        
        void destroy();

        u16 getUserFlags();

        inline bool checkUserFlag(PlayerFlag flag) const;
        inline void setUserFlag(PlayerFlag flag);
        inline void resetUserFlag(PlayerFlag flag);

        inline bool checkInnerFlag(InnerFlag flag) const;
        inline void setInnerFlag(InnerFlag flag);
        inline void resetInnerFlag(InnerFlag flag);

        void rewind();

        void play();
        void pause();
        void stop();

        State getState();
        void setState(s32 state);
        u32 getSampleOffset();

        void setPosition(f32 x, f32 y, f32 z);
        void setGain(f32 gain);
        void setPitch(f32 pitch);

        bool create(s8 numBuffers, IAudioClient* audioClient, const WAVEFORMATEX& fmt);

        void setStream(Stream* stream);

        bool initialize();
        bool update();
    private:
        friend class Context;
        friend class Device;

        Player(const Player& rhs);
        Player& operator=(const Player& rhs);

        void clear();

        u32 fillShort(LSshort* pcm, u32 requestFrames, u16 userFlags);
        u32 fillFloat(LSfloat* pcm, u32 requestFrames, u16 userFlags);
        static void copyFromSharedBuffer(u32 sizeInByte, BYTE* dst);

        u16 userFlags_;
        u16 innerFlags_;

        u32 bufferFrames_;
        u32 halfBufferFrames_;
        s32 state_;
        Stream* stream_;
        IAudioClient* audioClient_;
        IAudioRenderClient* audioRenderClient_;
        IAudioClock* audioClock_;
        IAudioStreamVolume* audioStreamVolume_;
        //HANDLE fillEvent_;
        UserPlayer* userPlayer_;

        Resampler resampler_;
        LIME_ALIGN16 static LSfloat sharedBuffer_[SharedBufferLength];
    };

    inline bool Player::checkUserFlag(PlayerFlag flag) const
    {
        return 0 != (userFlags_ & flag);
    }

    inline void Player::setUserFlag(PlayerFlag flag)
    {
        userFlags_ |= flag;
    }

    inline void Player::resetUserFlag(PlayerFlag flag)
    {
        userFlags_ &= ~flag;
    }

    inline bool Player::checkInnerFlag(InnerFlag flag) const
    {
        return 0 != (innerFlags_ & flag);
    }

    inline void Player::setInnerFlag(InnerFlag flag)
    {
        innerFlags_ |= flag;
    }

    inline void Player::resetInnerFlag(InnerFlag flag)
    {
        innerFlags_ &= ~flag;
    }
}
#endif //INC_LSOUND_WASAPI_PLAYER_H__
