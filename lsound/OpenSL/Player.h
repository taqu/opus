#ifndef INC_LSOUND_OPENSL_PLAYER_H__
#define INC_LSOUND_OPENSL_PLAYER_H__
/**
@file Player.h
@author t-sakai
@date 2015/07/19 create
*/
#include "../lsound.h"
#include "internal/SLObject.h"
#include "internal/SLPlay.h"
#include "internal/SLVolume.h"
#include "internal/SLBufferQueue.h"

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

    class Player : public PlayerLink
    {
    public:
        enum InnerFlag
        {
            InnerFlag_UserPlayer = (0x01U<<0),
        };

        ~Player();
        
        u16 getUserFlags();

        inline bool checkUserFlag(PlayerFlag flag) const;
        inline void setUserFlag(PlayerFlag flag);
        inline void resetUserFlag(PlayerFlag flag);

        inline bool checkInnerFlag(InnerFlag flag) const;
        inline void setInnerFlag(InnerFlag flag);
        inline void resetInnerFlag(InnerFlag flag);

        void clear();

        void rewind();
        void play();
        void pause();
        void stop();

        State getState();

        s32 getQueued();
        u32 getProcessed();
        u32 getSampleOffset();
        bool isSampleEnd();

        void setPosition(f32 x, f32 y, f32 z);
        void setGain(f32 gain);
        void setPitch(f32 pitch);
        void setRelative(s32 enable);
        void setRolloff(s32 rolloff);

        bool create(s32 numBuffers, SLObjectItf playObj);

        void setStream(Stream* stream);

        bool initialize();
        bool update();
    private:
        friend class Context;

        static const s32 NumChannels = 2;
        static const u32 BufferRoundMask = NumMaxBuffers-1;

        Player(const Player&);
        Player& operator=(const Player&);

        static void callback(SLAndroidSimpleBufferQueueItf caller, void* context);

        void clearQueuedBuffers();
        inline void advanceBufferIndex()
        {
            ++nextBufferIndex_;
            nextBufferIndex_ &= BufferRoundMask;
        }
        void enqueue(u32 size);
        void dequeue(SampleType*& buffer, u32& size);

        void systemPause(bool pause);

        Player();

        struct QueuedBuffer
        {
            s32 index_;
            u32 size_;
        };

        s32 fillBuffer(bool isLoop);

        u16 userFlags_;
        u16 innerFlags_;
        s16 numBuffers_;
        s16 prevState_;
        SLmillibel maxVolumeLevel_;

        //SLObject outputMixObject_;
        SLObject playObject_;
        SLPlay play_;
        SLBufferQueue bufferQueue_;
        SLVolume volume_;

        Stream* stream_;
        UserPlayer* userPlayer_;

        s16 nextBufferIndex_;
        s16 numQueuedBuffers_;
        QueuedBuffer queuedBuffers_[NumMaxBuffers];
        SampleType buffers_[NumMaxBuffers][BufferNumSamples];
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
#endif //INC_LSOUND_OPENSL_PLAYER_H__