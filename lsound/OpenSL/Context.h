#ifndef INC_LSOUND_OPENSL_CONTEXT_H__
#define INC_LSOUND_OPENSL_CONTEXT_H__
/**
@file Context.h
@author t-sakai
@date 2015/07/14 create
*/
#ifdef ANDROID
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#endif

#include <opus/opus_types.h>
#include <lcore/async/SyncObject.h>
#include "../lsound.h"
#include "Player.h"

#include "internal/SLObject.h"
#include "internal/SLEngine.h"
#include "internal/SLAudioIODeviceCapabilities.h"
#include "internal/SLEngineCapabilities.h"

namespace lcore
{
    class Thread;
}

namespace lsound
{
    class Stream;
    class FileStream;
    class MemoryStream;
    class PackResource;

    class Context
    {
    public:
        struct InitParam
        {
            InitParam()
                :numQueuedBuffers_(2)
                ,maxPlayers_(64)
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
        static bool exists(){ return NULL != instance_;}
        static Context& getInstance(){ return *instance_;}

        void setGain(f32 gain);
        void setPause(bool pause);

        void updateRequests();

        s32 loadResourcePack(s32 id, const Char* path, bool stream);

#ifdef ANDROID
        s32 loadResourcePackFromAsset(AAssetManager* assetManager, s32 id, const Char* path, s32 stream);
#endif
        bool play(s32 packId, s32 id, f32 gain=1.0f);
        UserPlayer* createUserPlayer(s32 packId, s32 id);
        void destroyUserPlayer(UserPlayer* player);

    private:
        friend class Player;
        friend class UserPlayer;
        friend class ContextThread;

        class ContextThread;

        Context(const Context&);
        Context& operator=(const Context&);

        struct StreamEntry
        {
            StreamEntry* next_;
        };

        Context();
        ~Context();

        void setThreadCanRun(bool canRun);
        void getThreadStatus(bool& canRun, bool& systemPaused);
        u32 getWaitTime();

        void enterAPI();
        void leaveAPI();

        void clear();
        void destroy();
        void initStreams();
        void initPlayers();
        void initUserPlayers();

        static void* threadProc(void* args);
        void update();

        StreamEntry* getStream();
        void releaseStream(Stream* stream);

        Player* getPlayer();
        void releasePlayer(Player* player);

        UserPlayer* getUserPlayer();
        void releaseUserPlayer(UserPlayer* player);

        void addRequest(Player* player);
        Player* getRequestList();

        static Context* instance_;

        pthread_t thread_;
        lcore::SpinLock lockThreadStatus_;

        lcore::Event waitEvent_;
        lcore::CriticalSection csContext_;
        lcore::SpinLock lockAPI_;

        bool canRun_;
        bool systemPaused_;

        InitParam initParam_;
        u32 waitTime_;

        SLObject engineObject_;
        SLEngine engine_;
        SLAudioIODeviceCapabilities audioIODeviceCapabilities_;
        SLEngineCapabilities engineCapabilities_;

        SLObject outputMixObject_;

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
}
#endif //INC_LSOUND_OPENSL_CONTEXT_H__
