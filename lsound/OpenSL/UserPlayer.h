#ifndef INC_LSOUND_OPENSL_USERPLAYER_H__
#define INC_LSOUND_OPENSL_USERPLAYER_H__
/**
@file UserPlayer.h
@author t-sakai
@date 2015/07/19 create
*/
#include "../lsound.h"
#include <lcore/async/SyncObject.h>

namespace lsound
{
    class Player;

    class UserPlayer
    {
    public:
        bool checkFlag(PlayerFlag flag);
        void setFlag(PlayerFlag flag);
        void resetFlag(PlayerFlag flag);

        void play();
        void pause();

        State getState();

        void setGain(f32 gain);
        void setPitch(f32 pitch);

    private:
        friend class Context;
        friend class Player;

        UserPlayer(const UserPlayer&);
        UserPlayer& operator=(const UserPlayer&);

        UserPlayer();
        ~UserPlayer();

        lcore::CriticalSection lock_;
        u16 flags_;
        u16 initialized_;
        s32 state_;

        union Impl
        {
            UserPlayer* next_;
            Player* player_;
        };

        Impl impl_;
    };
}
#endif //INC_LSOUND_OPENSL_USERPLAYER_H__
