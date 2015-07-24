/**
@file Context.cpp
@author t-sakai
@date 2014/07/07 create
*/
#include "Context.h"
#include <lcore/async/Thread.h>
#include "../opus/Stream.h"
#include "../opus/Resource.h"
#include "UserPlayer.h"

#define LSOUND_CONTEXT_DEBUG

namespace lsound
{
namespace
{
    ALsizei getSizeInBytes(ALsizei size, ALenum channels, ALenum type)
    {
        switch(channels)
        {
        case AL_MONO_SOFT:
            break;
        case AL_STEREO_SOFT:
            size *= 2;
            break;
        case AL_REAR_SOFT:
            size *= 2;
            break;
        case AL_QUAD_SOFT:
            size *= 4;
            break;
        case AL_5POINT1_SOFT:
            size *= 6;
            break;
        case AL_6POINT1_SOFT:
            size *= 7;
            break;
        case AL_7POINT1_SOFT:
            size *= 8;
            break;
        };

        switch(type)
        {
        case AL_BYTE_SOFT:
            size *= sizeof(ALbyte);
            break;
        case AL_UNSIGNED_BYTE_SOFT:
            size *= sizeof(ALubyte);
            break;
        case AL_SHORT_SOFT:
            size *= sizeof(ALshort);
            break;
        case AL_UNSIGNED_SHORT_SOFT:
            size *= sizeof(ALushort);
            break;
        case AL_INT_SOFT:
            size *= sizeof(ALint);
            break;
        case AL_UNSIGNED_INT_SOFT:
            size *= sizeof(ALuint);
            break;
        case AL_FLOAT_SOFT:
            size *= sizeof(ALfloat);
            break;
        case AL_DOUBLE_SOFT:
            size *= sizeof(ALdouble);
            break;
        };
        return size;
    }

    void AL_APIENTRY alBufferData_(ALuint buffer, ALuint sampleRate, ALenum internalFormat, ALsizei samples, ALenum channels, ALenum type, const ALvoid* data)
    {
        alBufferData(buffer, internalFormat, data, getSizeInBytes(samples, channels, type), sampleRate);
    }

    LPALBUFFERSAMPLESSOFT lpAlBufferSamplesSOFT = alBufferData_;
    //LPALISBUFFERFORMATSUPPORTEDSOFT lpAlIsBufferFormatSupportedSOFT = NULL;

    void AL_APIENTRY alGetSourcedv_(ALuint source, ALenum id, ALdouble* value)
    {
        *value = 0.0;
    }

    LPALGETSOURCEDVSOFT lpAlGetSourcedvSOFT = alGetSourcedv_;


#ifdef LSOUND_CONTEXT_DEBUG
    class StopWatch
    {
    public:

        void start()
        {
            begin_ = lcore::getPerformanceCounter();
        }

        f32 stop()
        {
            return lcore::calcTime(begin_, lcore::getPerformanceCounter());
        }
        
        ClockType begin_;
    };

    inline void printTime(const Char* str, f32 time)
    {
        printf(str, time);
    }

#define LSOUND_DECL_STOPWATCH StopWatch stopWatch;
#define LSOUND_DECL_START stopWatch.start();
#define LSOUND_DECL_STOP(str) printTime((str), stopWatch.stop());

    inline void printLatency(LSuint source)
    {
        ALdouble offsets[2];
        lpAlGetSourcedvSOFT(source, AL_SEC_OFFSET_LATENCY_SOFT, offsets);
        printf("Offset:%f - Latency:%3u ms\n", offsets[0], (ALuint)(offsets[1]*1000));
    }
#define LSOUND_DECL_LATENCY(source) printLatency(source);

#else
#define LSOUND_DECL_STOPWATCH
#define LSOUND_DECL_START
#define LSOUND_DECL_STOP(str)
#define LSOUND_DECL_LATENCY(source)
#endif

}

    class Context::ContextThread : public lcore::Thread
    {
    public:
        ContextThread();
        virtual ~ContextThread();

        virtual void run();
    };

    Context::ContextThread::ContextThread()
    {
    }

    Context::ContextThread::~ContextThread()
    {
    }

    void Context::ContextThread::run()
    {
        Context& context = Context::getInstance();
        while(this->canRun()){
            context.update();
        }
    }

    Context* Context::instance_ = NULL;

    bool Context::initialize(const InitParam& initParam)
    {
        LASSERT(0<initParam.numQueuedBuffers_ && initParam.numQueuedBuffers_<=lsound::NumMaxBuffers);
        LASSERT(0<initParam.maxPlayers_);

        if(NULL != instance_){
            return true;
        }
        instance_ = LIME_NEW Context();
        instance_->initParam_ = initParam;
        instance_->waitTime_ = initParam.waitTime_;

        ALCdevice* device = alcOpenDevice(NULL);
        if(NULL == device){
            LIME_DELETE(instance_);
            return false;
        }
        instance_->device_ = device;

        ALCint attrs[]=
        {
            ALC_FREQUENCY, SampleRate_48000,
            ALC_MONO_SOURCES, instance_->initParam_.maxPlayers_,
            ALC_STEREO_SOURCES, instance_->initParam_.maxPlayers_,
        };
        instance_->context_ = alcCreateContext(instance_->device_, attrs);
        if(NULL == instance_->context_ || alcMakeContextCurrent(instance_->context_) == AL_FALSE){
            instance_->destroy();
            LIME_DELETE(instance_);
            return false;
        }

#ifdef _DEBUG
        {
            const char* deviceList = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);

            const char* vendor = alGetString(AL_VENDOR);
            if(NULL != vendor){
                lcore::Log("Vendor: %s", vendor);
            }

            const char* version = alGetString(AL_VERSION);
            if(NULL != version){
                lcore::Log("Version: %s", version);
            }

            const char* renderer = alGetString(AL_RENDERER);
            if(NULL != renderer){
                lcore::Log("Renderer: %s", renderer);
            }
            const char* extString = alGetString(AL_EXTENSIONS);
            if(NULL != extString){
                lcore::Log("Extensions: %s", extString);
            }
        }
#endif

        if(alIsExtensionPresent("AL_SOFT_buffer_samples")){
            lpAlBufferSamplesSOFT = (LPALBUFFERSAMPLESSOFT)alGetProcAddress("alBufferSamplesSOFT");
            //lpAlIsBufferFormatSupportedSOFT = (LPALISBUFFERFORMATSUPPORTEDSOFT)alGetProcAddress("alIsBufferFormatSupportedSOFT");
        }

        if(alIsExtensionPresent("AL_SOFT_source_latency")){
            lpAlGetSourcedvSOFT = (LPALGETSOURCEDVSOFT)alGetProcAddress("alGetSourcedvSOFT");
        }

        alDistanceModel(AL_NONE);

        instance_->initStreams();
        instance_->initPlayers();
        instance_->initUserPlayers();

        instance_->thread_ = LIME_NEW ContextThread();
        instance_->thread_->create();
        instance_->thread_->start();
        return true;
    }

    void Context::terminate()
    {
        LIME_DELETE(instance_);
    }

    Context::Context()
        :device_(NULL)
        ,context_(NULL)
        ,numStreams_(0)
        ,streamTop_(NULL)
        ,streams_(NULL)
        ,numPlayers_(0)
        ,players_(NULL)
        ,numUserPlayers_(0)
        ,userPlayerTop_(NULL)
        ,userPlayers_(NULL)
        ,numRequests_(0)
        ,requestList_(NULL)
        ,waitEvent_(false, false)
    {
        playerTop_.resetLink();
        playList_.resetLink();

        for(s32 i=0; i<NumMaxPacks; ++i){
            packResources_[i] = NULL;
        }
    }

    Context::~Context()
    {
        if(NULL != thread_){
            thread_->stop();
            setPause(false);
            for(s32 i=0; i<10; ++i){
                if(thread_->join(initParam_.waitTime_) == lcore::thread::Wait_Success){
                    break;
                }
            }
            LIME_DELETE(thread_);
        }

        clear();

        for(s32 i=0; i<NumMaxPacks; ++i){
            LIME_DELETE(packResources_[i]);
        }

        LIME_DELETE_ARRAY(userPlayers_);
        LIME_DELETE_ARRAY(players_);
        
        u8* streams = reinterpret_cast<u8*>(streams_);
        LIME_FREE(streams);

        destroy();
    }

    void Context::setGain(f32 gain)
    {
        alListenerf(AL_GAIN, gain);
    }

    void Context::setPause(bool pause)
    {
        lcore::CSLock lock(playerLock_);
        if(pause){
            waitTime_ = lcore::thread::Infinite;
        }else{
            waitTime_ = initParam_.waitTime_;
            waitEvent_.set();
        }
    }

    void Context::updateRequests()
    {
        if(numRequests_){
            waitEvent_.set();
            numRequests_ = 0;
        }
    }

    void Context::update()
    {
        LSOUND_DECL_STOPWATCH

        u32 waitTime = getWaitTime();

        LSOUND_DECL_START
        waitEvent_.wait(waitTime);
        LSOUND_DECL_STOP("wait: %f\n")

        //新規リクエスト処理
        playerLock_.enter();
        Player* requestList = getRequestList();
        playerLock_.leave();
        while(NULL != requestList){
            LSOUND_DECL_START;
            Player* request = requestList;
            requestList = requestList->getNext();
            initPlayer(request);
            request->link(&playList_);
            LSOUND_DECL_STOP("initial:%f\n")
        }

        Player* endList = NULL;

        Player* player = playList_.getNext();
        while(player != &playList_){
            //LSOUND_DECL_START

            Player* current = player;
            player = player->getNext();

            s32 processed = current->getProcessed();
            ALenum err = getError();
            if(err != AL_NO_ERROR){
                logError(err);
                continue;
            }

            u16 userFlags = current->getUserFlags();
            while(0<processed){
                LSuint bufid;
                current->unqueueBuffers(1, &bufid);
                --processed;

                lsound::Stream* stream = current->stream_;
                s32 numSamples = stream->read(pcm_, BufferNumSamples);
                if(0<numSamples){
                    lpAlBufferSamplesSOFT(bufid, stream->getSampleRate(), stream->getFormat(), numSamples, stream->getChannels(), stream->getType(), pcm_);
                    current->queueBuffers(1, &bufid);
                }

                if(stream->getTotal()<=stream->getPosition()){
                    //全て処理した
                    //ループなら先頭にシーク
                    if((userFlags & PlayerFlag_Loop)){
                        stream->seek(0);
                    }else{
                        break;
                    }
                }
            }

            State state = current->getState();
            if(State_Playing != state && State_Paused != state){
                if(current->getQueued()<=0){
                    current->clear();
                    current->unlink();
                    current->setNext(endList);
                    endList = current;
                    continue;
                }
                if(!current->checkInnerFlag(Player::InnerFlag_UserPlayer)){
                    current->play();
                }
            }

            //終了リクエスト処理
            {
                lcore::CSLock lock(playerLock_);
                while(NULL != endList){
                    Player* current = endList;
                    endList = endList->getNext();
                    current->resetLink();
                    releasePlayer(current);
                }
            }

            //LSOUND_DECL_LATENCY(current->source_);
            //LSOUND_DECL_STOP("update:%f\n")
        }
    }

    void Context::initPlayer(Player* player)
    {
        player->resetLink();
        u16 userFlags = player->getUserFlags();
        for(s32 i=0; i<1; ++i){
            lsound::Stream* stream = player->stream_;
            s32 numSamples = stream->read(pcm_, BufferNumSamples);
            if(0<numSamples){
                lpAlBufferSamplesSOFT(player->buffers_[i], stream->getSampleRate(), stream->getFormat(), numSamples, stream->getChannels(), stream->getType(), pcm_);
                player->queueBuffers(1, &player->buffers_[i]);

                ALenum err = getError();
                if(err != AL_NO_ERROR){
                    logError(err);
                    //request->clear();

                    //lcore::CSLock lock(playerLock_);
                    //releasePlayer(request);
                    //request = NULL;
                    //break;
                }
            }

            if(stream->getTotal()<=stream->getPosition()){
                //全て処理した
                //ループなら先頭にシーク
                if(userFlags & PlayerFlag_Loop){
                    stream->seek(0);
                }else{
                    break;
                }
            }
        }
        State state = player->getState();
        if(State_Playing != state && State_Paused != state){
            player->play();
            //if(!player->checkInnerFlag(Player::InnerFlag_UserPlayer)
            //    || State_Initial != state)
            //{
            //    player->play();
            //}
            ALenum err = getError();
            if(AL_NO_ERROR != err){
                logError(err);
            }
        }
    }


    s32 Context::loadResourcePack(s32 id, const Char* path, bool stream)
    {
        LASSERT(0<=id && id<NumMaxPacks);

        PackResource* packResource = NULL;
        if(stream){
            packResource = PackFile::open(path);
        }else{
            packResource = PackMemory::open(path);
        }

        if(NULL == packResource){
            return -1;
        }

        LIME_DELETE(packResources_[id]);
        packResources_[id] = packResource;
        return id;
    }

    bool Context::play(s32 packId, s32 id, f32 gain)
    {
        LASSERT(0<=packId && packId<NumMaxPacks);
        LASSERT(0<=id);

        PackResource* packResource = packResources_[packId];
        if(NULL == packResource){
            return false;
        }
        if(packResource->getNumFiles()<=id){
            return false;
        }

        Player* player;
        StreamEntry* streamEntry;
        {
            lcore::CSLock lock(playerLock_);
            if(numPlayers_<=0){
                return false;
            }
            player = getPlayer();

            if(NULL == streamTop_){
                releasePlayer(player);
                return false;
            }
            streamEntry = getStream();
        }

        Stream* stream;
        switch(packResource->getType())
        {
        case PackResource::ResourceType_File:
            {

                FileStream* fileStream = LIME_PLACEMENT_NEW(streamEntry) FileStream();
                PackFile* packFile = reinterpret_cast<PackFile*>(packResource);
                File* file;
                s32 start, end;
                packFile->get(id, file, start, end);
                fileStream->set(file, start, end);
                stream = fileStream;
            }
            break;

        case PackResource::ResourceType_Memory:
            {
                MemoryStream* memoryStream = LIME_PLACEMENT_NEW(streamEntry) MemoryStream();
                PackMemory* packMemory = reinterpret_cast<PackMemory*>(packResource);
                Memory* memory;
                u32 size;
                s32 offset;
                packMemory->get(id, memory, size, offset);
                memoryStream->set(size, offset, memory);
                stream = memoryStream;
            }
            break;

        default:
            {
                lcore::CSLock lock(playerLock_);
                releasePlayer(player);
                releaseStream((Stream*)streamEntry);
            }
            return false;
        }

        if(!stream->open() || stream->getTotal()<=0){
            lcore::CSLock lock(playerLock_);
            releasePlayer(player);
            releaseStream(stream);
            return false;
        }
        player->setStream(stream);
        player->setGain(gain);
        player->rewind();

        {
            lcore::CSLock lock(playerLock_);
            player->setNext(requestList_);
            requestList_ = player;
        }
        ++numRequests_;
        return true;
    }

    UserPlayer* Context::createUserPlayer(s32 packId, s32 id)
    {
        LASSERT(0<=packId && packId<NumMaxPacks);
        LASSERT(0<=id);

        PackResource* packResource = packResources_[packId];
        if(NULL == packResource){
            return NULL;
        }
        if(packResource->getNumFiles()<=id){
            return NULL;
        }

        UserPlayer* userPlayer;
        Player* player;
        StreamEntry* streamEntry;
        {
            if(numUserPlayers_<=0){
                return NULL;
            }

            userPlayer = getUserPlayer();

            lcore::CSLock lock(playerLock_);
            if(numPlayers_<=0){
                releaseUserPlayer(userPlayer);
                return NULL;
            }
            player = getPlayer();

            if(NULL == streamTop_){
                releaseUserPlayer(userPlayer);
                releasePlayer(player);
                return NULL;
            }
            streamEntry = getStream();
        }

        Stream* stream;
        switch(packResource->getType())
        {
        case PackResource::ResourceType_File:
            {

                FileStream* fileStream = LIME_PLACEMENT_NEW(streamEntry) FileStream();
                PackFile* packFile = reinterpret_cast<PackFile*>(packResource);
                File* file;
                s32 start, end;
                packFile->get(id, file, start, end);
                fileStream->set(file, start, end);
                stream = fileStream;
            }
            break;

        case PackResource::ResourceType_Memory:
            {
                MemoryStream* memoryStream = LIME_PLACEMENT_NEW(streamEntry) MemoryStream();
                PackMemory* packMemory = reinterpret_cast<PackMemory*>(packResource);
                Memory* memory;
                u32 size;
                s32 offset;
                packMemory->get(id, memory, size, offset);
                memoryStream->set(size, offset, memory);
                stream = memoryStream;
            }
            break;

        default:
            {
                lcore::CSLock lock(playerLock_);
                releaseUserPlayer(userPlayer);
                releasePlayer(player);
                releaseStream((Stream*)streamEntry);
            }
            return NULL;
        }

        if(!stream->open() || stream->getTotal()<=0){
            lcore::CSLock lock(playerLock_);
            releaseUserPlayer(userPlayer);
            releasePlayer(player);
            releaseStream(stream);
            return NULL;
        }
        player->setStream(stream);
        player->setGain(1.0f);
        player->setInnerFlag(Player::InnerFlag_UserPlayer);
        
        player->rewind();
        player->userPlayer_ = userPlayer;
        userPlayer->impl_.player_ = player;

        {
            lcore::CSLock lock(playerLock_);
            player->setNext(requestList_);
            requestList_ = player;
        }
        ++numRequests_;
        return userPlayer;
    }

    void Context::destroyUserPlayer(UserPlayer* player)
    {
        if(NULL == player){
            return;
        }
        player->impl_.player_->stop();
        releaseUserPlayer(player);
    }

    LSenum Context::getFormat(LSenum channels, LSenum type, LPALISBUFFERFORMATSUPPORTEDSOFT isBufferSupportedSOFT)
    {
        LSenum format = AL_NONE;

        if(NULL != lpAlBufferSamplesSOFT){
            if(AL_UNSIGNED_BYTE_SOFT == type || AL_BYTE_SOFT == type){
                switch(channels)
                {
                case AL_MONO_SOFT:
                    format = AL_MONO8_SOFT;
                    break;
                case AL_STEREO_SOFT:
                    format = AL_STEREO8_SOFT;
                    break;
                default:
                    LASSERT(false);
                    break;
                }

            }else if(AL_UNSIGNED_SHORT_SOFT == type || AL_SHORT_SOFT == type){
                switch(channels)
                {
                case AL_MONO_SOFT:
                    format = AL_MONO16_SOFT;
                    break;
                case AL_STEREO_SOFT:
                    format = AL_STEREO16_SOFT;
                    break;
                default:
                    LASSERT(false);
                    break;
                }
            }

            if(AL_NONE != format && isBufferSupportedSOFT(format)){
                return format;
            }

            switch(channels)
            {
            case AL_MONO_SOFT:
                format = AL_MONO16_SOFT;
                break;
            case AL_STEREO_SOFT:
                format = AL_STEREO16_SOFT;
                break;
            default:
                LASSERT(false);
                break;
            }

            if(AL_NONE != format && isBufferSupportedSOFT(format)){
                return format;
            }

            switch(channels)
            {
            case AL_MONO_SOFT:
                format = AL_MONO8_SOFT;
                break;
            case AL_STEREO_SOFT:
                format = AL_STEREO8_SOFT;
                break;
            default:
                LASSERT(false);
                break;
            }

            if(AL_NONE != format && isBufferSupportedSOFT(format)){
                return format;
            }

            return AL_NONE;
        }

        switch(type)
        {
        case AL_BYTE_SOFT:
        case AL_UNSIGNED_BYTE_SOFT:
            switch(channels)
            {
            case AL_MONO_SOFT:
                format = AL_MONO8_SOFT;
                break;
            case AL_STEREO_SOFT:
                format = AL_STEREO8_SOFT;
                break;
            default:
                LASSERT(false);
                break;
            }
            break;

        case AL_SHORT_SOFT:
        case AL_UNSIGNED_SHORT_SOFT:
            switch(channels)
            {
            case AL_MONO_SOFT:
                format = AL_MONO16_SOFT;
                break;
            case AL_STEREO_SOFT:
                format = AL_STEREO16_SOFT;
                break;
            default:
                LASSERT(false);
                break;
            }
            break;
        default:
            LASSERT(false);
            break;
        }
        return format;
    }

    void Context::updateBuffer(LSuint buffer, LSuint sampleRate, LSenum internalFormat, LSsizei samples, LSenum channels, LSenum type, const LSvoid* data)
    {
        lpAlBufferSamplesSOFT(buffer, sampleRate, internalFormat, samples, channels, type, data);
    }

    void Context::clear()
    {
        Player* endList = NULL;

        Player* player = playList_.getNext();
        while(player != &playList_){
            Player* current = player;
            player = player->getNext();

            current->clear();
            current->unlink();
            current->setNext(endList);
            endList = current;
        }

        //終了リクエスト、新規リクエスト処理
        Player* requestList = NULL;
        {
            lcore::CSLock lock(playerLock_);
            requestList = getRequestList();
        }

        while(NULL != requestList){
            Player* request = requestList;
            requestList = requestList->getNext();
            request->resetLink();

            request->clear();
            request->setNext(endList);
            endList = request;
        }

        {
            lcore::CSLock lock(playerLock_);
            while(NULL != endList){
                Player* current = endList;
                endList = endList->getNext();
                current->resetLink();
                releasePlayer(current);
            }
        }
    }

    void Context::destroy()
    {
        alcMakeContextCurrent(NULL);

        if(NULL != context_){
            alcDestroyContext(context_);
            context_ = NULL;
        }
        if(NULL != device_){
            alcCloseDevice(device_);
            device_ = NULL;
        }
    }

    void Context::initStreams()
    {
        LASSERT(NULL == streams_);
        numStreams_ = initParam_.maxPlayers_;
        u32 elemSize = lcore::maximum(sizeof(FileStream), sizeof(MemoryStream));
        u8* buffer = (u8*)LIME_MALLOC(elemSize * initParam_.maxPlayers_);

        StreamEntry* entry;
        for(s32 i=0; i<(initParam_.maxPlayers_-1); ++i){
            entry = reinterpret_cast<StreamEntry*>(buffer + i*elemSize);
            entry->next_ = reinterpret_cast<StreamEntry*>(buffer + (i+1)*elemSize);
        }
        entry = reinterpret_cast<StreamEntry*>(buffer + (initParam_.maxPlayers_-1)*elemSize);
        entry->next_ = NULL;

        streamTop_ = reinterpret_cast<StreamEntry*>(buffer);
        streams_ = reinterpret_cast<StreamEntry*>(buffer);
    }

    void Context::initPlayers()
    {
        LASSERT(NULL == players_);
        numPlayers_ = initParam_.maxPlayers_;
        players_ = LIME_NEW Player[initParam_.maxPlayers_];

        for(s32 i=initParam_.maxPlayers_-1; 0<=i; --i){
            players_[i].create(initParam_.numQueuedBuffers_);
            players_[i].link(playerTop_.getNext());
        }
    }

    void Context::initUserPlayers()
    {
        LASSERT(NULL == userPlayers_);
        numUserPlayers_ = initParam_.maxUserPlayers_;
        userPlayers_ = LIME_NEW UserPlayer[initParam_.maxUserPlayers_];

        for(s32 i=0; i<(initParam_.maxUserPlayers_-1); ++i){
            userPlayers_[i].impl_.next_ = &userPlayers_[i+1];
        }
        userPlayers_[initParam_.maxUserPlayers_-1].impl_.next_ = NULL;
        userPlayerTop_ = &userPlayers_[0];
    }

    u32 Context::getWaitTime()
    {
        lcore::CSLock lock(playerLock_);
        return waitTime_;
    }

    Context::StreamEntry* Context::getStream()
    {
        StreamEntry* entry = streamTop_;
        streamTop_ = streamTop_->next_;
        --numStreams_;
        return entry;
    }

    void Context::releaseStream(Stream* stream)
    {
        LASSERT(NULL != stream);
        stream->~Stream();
        StreamEntry* entry = reinterpret_cast<StreamEntry*>(stream);
        entry->next_ = streamTop_;
        streamTop_ = entry;
        ++numStreams_;
    }

    Player* Context::getPlayer()
    {
        Player* player = playerTop_.getNext();
        player->unlink();
        player->userFlags_ = 0;
        player->innerFlags_ = 0;
        player->userPlayer_ = NULL;

        --numPlayers_;
        return player;
    }

    void Context::releasePlayer(Player* player)
    {
        LASSERT(NULL != player);
        //player->resetLink();

        if(NULL != player->stream_){
            releaseStream(player->stream_);
            player->stream_ = NULL;
        }
        player->link(&playerTop_);
        ++numPlayers_;
    }

    UserPlayer* Context::getUserPlayer()
    {
        UserPlayer* player = userPlayerTop_;
        userPlayerTop_ = userPlayerTop_->impl_.next_;
        player->flags_ = 0;
        --numUserPlayers_;
        return player;
    }

    void Context::releaseUserPlayer(UserPlayer* player)
    {
        LASSERT(NULL != player);
        player->impl_.next_ = userPlayerTop_;
        userPlayerTop_ = player;
        ++numUserPlayers_;
    }

    void Context::addRequest(Player* player)
    {
        LASSERT(NULL != player);
        player->setNext(requestList_);
        requestList_ = player;
    }

    Player* Context::getRequestList()
    {
        Player* ret = requestList_;
        requestList_ = NULL;
        return ret;
    }
}
