#ifndef INC_LSOUND_WASAPI_CONTEXT_H__
#define INC_LSOUND_WASAPI_CONTEXT_H__
/**
@file Context.h
@author t-sakai
@date 2015/07/06 create
*/
#include "../lsound.h"
#include <lcore/async/SyncObject.h>

#include "Device.h"
#include "Player.h"

namespace lcore
{
    class Thread;
}

namespace lsound
{
    class PackResource;
    class Stream;

    class Context
    {
    public:
        struct InitParam
        {
            InitParam()
                :numQueuedBuffers_(3)
                ,maxPlayers_(128)
                ,maxUserPlayers_(4)
                ,waitTime_(30)
            {}

            s32 numQueuedBuffers_;
            s32 maxPlayers_;
            s32 maxUserPlayers_;
            u32 waitTime_;
        };

        static bool initialize(const InitParam& initParam);
        static void terminate();

        static Context& getInstance(){ return *instance_;}

        void clear();

        void setGain(f32 gain);
        void setPause(bool pause);
        void updateRequests();

        s32 loadResourcePack(s32 id, const Char* path, bool stream);

        bool play(s32 packId, s32 id, f32 gain=1.0f);
        UserPlayer* createUserPlayer(s32 packId, s32 id);
        void destroyUserPlayer(UserPlayer* player);

    private:
        //friend class UserPlayer;

        Context(const Context&);
        Context& operator=(const Context&);

        static u32 __stdcall proc(void* args);

        Player* updatePlayers();

        struct StreamEntry
        {
            StreamEntry* next_;
        };

        bool createDevice();
        void destroyDevice();
        inline Device& getDevice();

        u32 getWaitTime();

        void initStreams();
        void initPlayers();
        void initUserPlayers();

        bool initPlayer(Player* player);

        StreamEntry* getStream();
        void releaseStream(Stream* stream);

        Player* getPlayer();
        void releasePlayer(Player* player);

        UserPlayer* getUserPlayer();
        void releaseUserPlayer(UserPlayer* player);

        void addRequest(Player* player);
        Player* getRequestList();

        Context(const InitParam& initParam);
        ~Context();

        static Context* instance_;

        HANDLE thread_;
        HANDLE initEvent_;
        HANDLE exitEvent_;
        HANDLE waitEvent_;
        lcore::CriticalSection contextLock_;

        InitParam initParam_;
        u32 waitTime_;
        Device device_;

        s32 numStreams_;
        StreamEntry* streamTop_;
        StreamEntry* streams_;

        s32 numPlayers_;
        PlayerLink playerTop_;
        Player* players_;

        s32 numUserPlayers_;
        UserPlayer* userPlayerTop_;
        UserPlayer* userPlayers_;

        PackResource* packResources_[NumMaxPacks];

        u32 numRequests_;
        Player* requestList_;
        PlayerLink playList_;
    };

    inline Device& Context::getDevice()
    {
        return device_;
    }
}
#endif //INC_LSOUND_WASAPI_CONTEXT_H__
