#ifndef INC_LSOUND_OPENAL_PLAYER_H__
#define INC_LSOUND_OPENAL_PLAYER_H__
/**
@file Player.h
@author t-sakai
@date 2014/07/08 create
*/
#include "../lsound.h"

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
        static const s16 NumMaxBuffers = lsound::NumMaxBuffers;

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
        inline void rewind();

        inline void play();
        inline void pause();
        inline void stop();

        inline State getState();
        inline s32 getQueued();
        inline s32 getProcessed();
        inline u32 getSampleOffset();

        inline void setPosition(f32 x, f32 y, f32 z);
        inline void setGain(f32 gain);
        inline void setPitch(f32 pitch);
        inline void setRelative(s32 enable);
        inline void setRolloff(s32 rolloff);

        inline s32 getNumBuffers() const;
        void update(s32 index, LSuint sampleRate, LSenum internalFormat, LSsizei samples, LSenum channels, LSenum type, const LSvoid* data);
    private:
        friend class Context;

        Player(const Player&);
        Player& operator=(const Player&);

        inline void setStream(Stream* stream);

        Player();
        bool create(s8 numBuffers);

        inline void queueBuffers(s32 numBuffers, ALuint* buffers);
        inline void unqueueBuffers(s32 numBuffers, ALuint* buffers);

        u16 userFlags_;
        u16 innerFlags_;
        LSuint source_;
        s32 numBuffers_;
        LSuint buffers_[NumMaxBuffers];
        Stream* stream_;
        UserPlayer* userPlayer_;
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

    inline void Player::rewind()
    {
        alSourceRewind(source_);
    }

    inline void Player::play()
    {
        alSourcePlay(source_);
    }

    inline void Player::pause()
    {
        alSourcePause(source_);
    }

    inline void Player::stop()
    {
        alSourceStop(source_);
    }

    inline State Player::getState()
    {
        s32 state;
        alGetSourcei(source_, AL_SOURCE_STATE, &state);
        return (State)state;
    }

    inline s32 Player::getQueued()
    {
        s32 queued = 0;
        alGetSourcei(source_, AL_BUFFERS_QUEUED, &queued);
        return queued;
    }

    inline s32 Player::getProcessed()
    {
        s32 processed = 0;
        alGetSourcei(source_, AL_BUFFERS_PROCESSED, &processed);
        return processed;
    }

    inline u32 Player::getSampleOffset()
    {
        s32 samples = 0;
        alGetSourcei(source_, AL_SAMPLE_OFFSET, &samples);
        return static_cast<u32>(samples);
    }

    inline void Player::setPosition(f32 x, f32 y, f32 z)
    {
        alSource3f(source_, AL_POSITION, x, y, z);
    }

    inline void Player::setGain(f32 gain)
    {
        alSourcef(source_, AL_GAIN, gain);
    }

    inline void Player::setPitch(f32 pitch)
    {
        alSourcef(source_, AL_PITCH, pitch);
    }

    inline void Player::setRelative(s32 enable)
    {
        alSourcei(source_, AL_SOURCE_RELATIVE, enable);
    }

    inline void Player::setRolloff(s32 rolloff)
    {
        alSourcei(source_, AL_ROLLOFF_FACTOR, rolloff);
    }

    inline void Player::queueBuffers(s32 numBuffers, ALuint* buffers)
    {
        LASSERT(NULL != buffers);
        alSourceQueueBuffers(source_, numBuffers, buffers);
    }

    inline void Player::unqueueBuffers(s32 numBuffers, ALuint* buffers)
    {
        LASSERT(NULL != buffers);
        alSourceUnqueueBuffers(source_, numBuffers, buffers);
    }

    inline void Player::setStream(Stream* stream)
    {
        stream_ = stream;
    }

    inline s32 Player::getNumBuffers() const
    {
        return numBuffers_;
    }
}
#endif //INC_LSOUND_OPENAL_PLAYER_H__