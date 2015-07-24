#ifndef INC_LSOUND_OPENAL_CONTEXT_H__
#define INC_LSOUND_OPENAL_CONTEXT_H__
/**
@file Context.h
@author t-sakai
@date 2014/07/07 create
*/

#include <AL/alc.h>
#include <AL/alext.h>
#include <opus/opus_types.h>
#include <lcore/async/SyncObject.h>
#include "../lsound.h"
#include "Player.h"

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
                :numQueuedBuffers_(3)
                ,maxPlayers_(128)
                ,maxUserPlayers_(8)
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

        void setGain(f32 gain);
        void setPause(bool pause);

        void updateRequests();

        s32 loadResourcePack(s32 id, const Char* path, bool stream);

        bool play(s32 packId, s32 id, f32 gain=1.0f);
        UserPlayer* createUserPlayer(s32 packId, s32 id);
        void destroyUserPlayer(UserPlayer* player);

        static LSenum getFormat(LSenum channels, LSenum type, LPALISBUFFERFORMATSUPPORTEDSOFT isBufferSupportedSOFT);
        
        static void updateBuffer(LSuint buffer, LSuint sampleRate, LSenum internalFormat, LSsizei samples, LSenum channels, LSenum type, const LSvoid* data);
    private:
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

        inline ALenum getError() const
        {
            return alGetError();
        }

        inline void logError(ALenum error) const
        {
#if _DEBUG
            lcore::Log("OpenAL Error: %s", alGetString(error));
#endif
        }

        void clear();
        void destroy();
        void initStreams();
        void initPlayers();
        void initUserPlayers();

        void initPlayer(Player* player);

        void update();

        u32 getWaitTime();

        StreamEntry* getStream();
        void releaseStream(Stream* stream);

        Player* getPlayer();
        void releasePlayer(Player* player);

        UserPlayer* getUserPlayer();
        void releaseUserPlayer(UserPlayer* player);

        void addRequest(Player* player);
        Player* getRequestList();

        static Context* instance_;

        InitParam initParam_;
        u32 waitTime_;
        lcore::Thread* thread_;
        ALCdevice* device_;
        ALCcontext* context_;

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

        lcore::CriticalSection playerLock_;
        lcore::Event waitEvent_;

        opus_int16 pcm_[BufferNumSamples];
    };
}
#endif //INC_LSOUND_OPENAL_CONTEXT_H__
