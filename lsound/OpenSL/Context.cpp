/**
@file Context.cpp
@author t-sakai
@date 2015/07/14 create
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
        lcore::Log(str, time);
    }

#define LSOUND_DECL_STOPWATCH StopWatch stopWatch;
#define LSOUND_DECL_START stopWatch.start();
#define LSOUND_DECL_STOP(str) printTime((str), stopWatch.stop());

#else
#define LSOUND_DECL_STOPWATCH
#define LSOUND_DECL_START
#define LSOUND_DECL_STOP(str)
#endif

}

    void* Context::threadProc(void* args)
    {
        Context* context = reinterpret_cast<Context*>(args);
        context->update();
        return NULL;
    }

    Context* Context::instance_ = NULL;

    bool Context::initialize(const InitParam& initParam)
    {
        LASSERT(0<initParam.numQueuedBuffers_ && initParam.numQueuedBuffers_<=lsound::NumMaxBuffers);
        LASSERT(0<initParam.maxPlayers_);

        if(NULL != instance_){
            terminate();
        }
        instance_ = LIME_NEW Context();
        instance_->initParam_ = initParam;
        instance_->waitTime_ = initParam.waitTime_;

        //Engine
        //----------------------------------------------------------
        SLEngineOption engineOption[] =
        {
            (SLuint32)SL_ENGINEOPTION_THREADSAFE,
            //(SLuint32)SL_BOOLEAN_FALSE,
            (SLuint32)SL_BOOLEAN_TRUE,
        };

        const SLInterfaceID engineInterfaceIDs[3] =
        {
            SL_IID_DEVICEVOLUME,
            SL_IID_AUDIOIODEVICECAPABILITIES,
            SL_IID_ENGINECAPABILITIES,
        };
        const SLboolean engineRequired[3] =
        {
            SL_BOOLEAN_FALSE,
            SL_BOOLEAN_FALSE,
            SL_BOOLEAN_FALSE,
        };

        SLObjectItf engineObject = NULL;
        LSresult result = slCreateEngine(&engineObject, 1, engineOption, 0, engineInterfaceIDs, engineRequired);

        if(result != SL_RESULT_SUCCESS){
            lcore::Log("error: slCreateEngine");
            return false;
        }
        SLObject(engineObject).swap(instance_->engineObject_);

        result = instance_->engineObject_.Realize(SL_BOOLEAN_FALSE);
        if(result != SL_RESULT_SUCCESS){
            lcore::Log("error: Realize");
            return false;
        }

        //----------------------------------------------------
        SLAudioIODeviceCapabilitiesItf audioIODeviceCapabilities = NULL;
        result = instance_->engineObject_.GetInterface(SL_IID_AUDIOIODEVICECAPABILITIES, &audioIODeviceCapabilities);
        if(result == SL_RESULT_SUCCESS){
            SLAudioIODeviceCapabilities(audioIODeviceCapabilities).swap(instance_->audioIODeviceCapabilities_);
        }else{
            lcore::Log("error: GetInterface:SL_IID_AUDIOIODEVICECAPABILITIES (%d)", result);
            //LIME_DELETE(instance_);
            //return false;
        }

        //----------------------------------------------------
        SLEngineCapabilitiesItf engineCapabilities = NULL;
        result = instance_->engineObject_.GetInterface(SL_IID_ENGINECAPABILITIES, &engineCapabilities);
        if(result == SL_RESULT_SUCCESS){
            SLEngineCapabilities(engineCapabilities).swap(instance_->engineCapabilities_);
        }else{
            lcore::Log("error: GetInterface:SL_IID_ENGINECAPABILITIES (%d)", result);
            //LIME_DELETE(instance_);
            //return false;
        }

        //----------------------------------------------------
        SLEngineItf engine = NULL;
        result = instance_->engineObject_.GetInterface(SL_IID_ENGINE, &engine);
        if(result != SL_RESULT_SUCCESS){
            lcore::Log("error: GetInterface:SL_IID_ENGINE (%d)", result);
            LIME_DELETE(instance_);
            return false;
        }
        SLEngine(engine).swap(instance_->engine_);


        if(instance_->engineCapabilities_.valid()){
            SLint16 numMaxVoices = 0;
            SLboolean isAbsoluteMax = SL_BOOLEAN_FALSE;
            SLint16 numFreeVoices = 0;
            result = instance_->engineCapabilities_.QueryAvailableVoices(
                SL_VOICETYPE_2D_AUDIO,
                &numMaxVoices,
                &isAbsoluteMax,
                &numFreeVoices);
            if(result != SL_RESULT_SUCCESS){
                lcore::Log("error: QueryAvailableVoices");
                LIME_DELETE(instance_);
                return false;
            }
            lcore::Log("max voices %d, absolute:%d, free voices:%d", numMaxVoices, isAbsoluteMax, numFreeVoices);
            if(numFreeVoices<instance_->initParam_.maxPlayers_){
                instance_->initParam_.maxPlayers_ = numFreeVoices;
            }

            SLboolean isThreadSafe = SL_BOOLEAN_FALSE;
            instance_->engineCapabilities_.IsThreadSafe(&isThreadSafe);
            lcore::Log("thread safe: %s", (isThreadSafe)?"TRUE":"FALSE");
        }

        //OutputMix
        //----------------------------------------------------------
        const SLInterfaceID outMixInterfaceIDs[2] =
        {
            SL_IID_VOLUME,
            SL_IID_ENVIRONMENTALREVERB,
        };
        const SLboolean outMixRequired[2] =
        {
            SL_BOOLEAN_FALSE,
            SL_BOOLEAN_FALSE,
        };
        SLObjectItf outputMixObject = NULL;
        result = instance_->engine_.CreateOutputMix(&outputMixObject, 0, outMixInterfaceIDs, outMixRequired);
        if(result != SL_RESULT_SUCCESS){
            lcore::Log("error: CreateOutputMix");
            LIME_DELETE(instance_);
            return false;
        }
        SLObject(outputMixObject).swap(instance_->outputMixObject_);

        result = instance_->outputMixObject_.Realize(SL_BOOLEAN_FALSE);
        if(result != SL_RESULT_SUCCESS){
            lcore::Log("error: Realize");
            LIME_DELETE(instance_);
            return false;
        }

        //SLVolumeItf outputVolume = NULL;
        //result = instance_->outputMixObject_.GetInterface(SL_IID_VOLUME, &outputVolume);
        //if(result != SL_RESULT_SUCCESS){
        //    lcore::Log("error: GetInterface:SL_IID_VOLUME");
        //    LIME_DELETE(instance_);
        //    return false;
        //}

        instance_->initStreams();
        instance_->initPlayers();
        instance_->initUserPlayers();

        instance_->setThreadCanRun(true);
        s32 ret = pthread_create(&instance_->thread_, NULL, Context::threadProc, instance_);
        if(0 != ret){
            lcore::Log("error: pthread_create");
            return false;
        }
        lcore::Log("success: initialize");
        return true;
    }

    void Context::terminate()
    {
        LIME_DELETE(instance_);
    }

    Context::Context()
        :thread_(0)
        ,waitEvent_(false, false)
        ,canRun_(false)
        ,systemPaused_(false)
        ,waitTime_(0)
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
        //,waitEvent_(false, false)
    {
        playerTop_.resetLink();
        playList_.resetLink();

        for(s32 i=0; i<NumMaxPacks; ++i){
            packResources_[i] = NULL;
        }
    }

    Context::~Context()
    {
        if(0 != thread_){
            setThreadCanRun(false);
            setPause(false);
            pthread_join(thread_, NULL);
            thread_ = 0;
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

    void Context::setThreadCanRun(bool canRun)
    {
        lockThreadStatus_.enter();
        canRun_ = canRun;
        lockThreadStatus_.leave();
    }

    void Context::getThreadStatus(bool& canRun, bool& systemPaused)
    {
        lockThreadStatus_.enter();
        canRun = canRun_;
        systemPaused = systemPaused_;
        lockThreadStatus_.leave();
    }

    u32 Context::getWaitTime()
    {
        lockThreadStatus_.enter();
        u32 waitTime = waitTime_;
        lockThreadStatus_.leave();
        return waitTime;
    }

    void Context::enterAPI()
    {
        lockAPI_.enter();
    }

    void Context::leaveAPI()
    {
        lockAPI_.leave();
    }

/*
    void Context::setGain(f32 gain)
    {
        //alListenerf(AL_GAIN, gain);
    }
*/

    void Context::setPause(bool pause)
    {
        lockThreadStatus_.enter();
        if(pause){
            waitTime_ = lcore::thread::Infinite;
        }else{
            waitTime_ = initParam_.waitTime_;
        }
        systemPaused_ = pause;
        lockThreadStatus_.leave();

        waitEvent_.set();
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
        LSOUND_DECL_STOPWATCH;

        bool canRun;
        bool systemPaused;
        bool prevSystemPaused;
        getThreadStatus(canRun, prevSystemPaused);

        for(;;){
            u32 waitTime = getWaitTime();
            //LSOUND_DECL_START
            waitEvent_.wait(waitTime);
            //LSOUND_DECL_STOP("wait: %f\n")

            getThreadStatus(canRun, systemPaused);
            if(!canRun){
                break;
            }
           // LSOUND_DECL_START;

            //新規リクエスト処理
            csContext_.enter();
            Player* requestList = getRequestList();
            csContext_.leave();
            while(NULL != requestList){
                //LSOUND_DECL_START;
                Player* request = requestList;
                requestList = requestList->getNext();
                request->resetLink();
                request->initialize();
                request->link(&playList_);
                //LSOUND_DECL_STOP("initial:%f\n");
            }

            Player* player;

            if(systemPaused != prevSystemPaused){
                prevSystemPaused = systemPaused;
                player = playList_.getNext();
                while(player != &playList_){
                    player->systemPause(systemPaused);
                    player = player->getNext();
                }
            }

            player = playList_.getNext();
            Player* endList = NULL;
            while(player != &playList_){
                //LSOUND_DECL_START

                Player* current = player;
                player = player->getNext();

                //State state = current->getState();
                //if(State_Playing != state && State_Paused != state){
                if(!current->update()){
                    current->clear();
                    current->unlink();
                    current->setNext(endList);
                    endList = current;
                }
            }

            //終了リクエスト処理
            csContext_.enter();
            while(NULL != endList){
                Player* current = endList;
                endList = endList->getNext();
                current->resetLink();
                releasePlayer(current);
            }
            csContext_.leave();

            //LSOUND_DECL_STOP("update:%f\n");
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

#ifdef ANDROID
    s32 Context::loadResourcePackFromAsset(AAssetManager* assetManager, s32 id, const Char* path, s32 stream)
    {
        LASSERT(0<=id && id<NumMaxPacks);

        PackResource* packResource = NULL;
        if(stream){
            packResource = PackAsset::open(assetManager, path, AASSET_MODE_RANDOM);
        }else{
            packResource = PackMemory::openFromAsset(assetManager, path, AASSET_MODE_STREAMING);
        }

        if(NULL == packResource){
            return -1;
        }

        LIME_DELETE(packResources_[id]);
        packResources_[id] = packResource;
        return id;
    }
#endif

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
            lcore::CSLock lock(csContext_);
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

        case PackResource::ResourceType_Asset:
            {
                AssetStream* assetStream = LIME_PLACEMENT_NEW(streamEntry) AssetStream();
                PackAsset* packAsset = reinterpret_cast<PackAsset*>(packResource);
                Asset* asset;
                s32 start, end;
                packAsset->get(id, asset, start, end);
                assetStream->set(asset, start, end);
                stream = assetStream;
            }
            break;

        default:
            {
                lcore::CSLock lock(csContext_);
                releasePlayer(player);
                releaseStream((Stream*)streamEntry);
            }
            return false;
        }

        if(!stream->open() || stream->getTotal()<=0){
            lcore::CSLock lock(csContext_);
            releasePlayer(player);
            releaseStream(stream);
            return false;
        }
        player->setStream(stream);
        player->setGain(gain);
        player->rewind();

        {
            lcore::CSLock lock(csContext_);
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

            lcore::CSLock lock(csContext_);
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

        case PackResource::ResourceType_Asset:
            {
                AssetStream* assetStream = LIME_PLACEMENT_NEW(streamEntry) AssetStream();
                PackAsset* packAsset = reinterpret_cast<PackAsset*>(packResource);
                Asset* asset;
                s32 start, end;
                packAsset->get(id, asset, start, end);
                assetStream->set(asset, start, end);
                stream = assetStream;
            }
            break;

        default:
            {
                lcore::CSLock lock(csContext_);
                releaseUserPlayer(userPlayer);
                releasePlayer(player);
                releaseStream((Stream*)streamEntry);
            }
            return NULL;
        }

        if(!stream->open() || stream->getTotal()<=0){
            lcore::CSLock lock(csContext_);
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
        userPlayer->initialized_ = 0;
        userPlayer->state_ = State_Initial;

        {
            lcore::CSLock lock(csContext_);
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
            lcore::CSLock lock(csContext_);
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
            lcore::CSLock lock(csContext_);
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
        outputMixObject_.Destroy();

        engineObject_.Destroy();
        SLEngine().swap(engine_);
    }


    void Context::initStreams()
    {
        LASSERT(NULL == streams_);
        numStreams_ = initParam_.maxPlayers_;
        u32 elemSize = lcore::maximum(sizeof(FileStream), sizeof(MemoryStream));
        elemSize = lcore::maximum(elemSize, sizeof(AssetStream));
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
        players_ = LIME_NEW Player[initParam_.maxPlayers_];
        numPlayers_ = 0;

        SLDataLocator_BufferQueue bufferQueue;
        SLDataFormat_PCM pcmFormat;
        SLDataSource dataSource;
        SLDataLocator_OutputMix outputMix;
        SLDataSink dataSink;

        bufferQueue.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;//SL_DATALOCATOR_BUFFERQUEUE;
        bufferQueue.numBuffers = 2;//initParam_.numQueuedBuffers_;

        pcmFormat.formatType = SL_DATAFORMAT_PCM;
        pcmFormat.numChannels = 2;
        pcmFormat.samplesPerSec = SL_SAMPLINGRATE_48;
        pcmFormat.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
        pcmFormat.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
        pcmFormat.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
        pcmFormat.endianness = SL_BYTEORDER_LITTLEENDIAN; //SL_BYTEORDER_BIGENDIAN

        dataSource.pFormat = &pcmFormat;
        dataSource.pLocator = &bufferQueue;

        outputMix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
        outputMix.outputMix = outputMixObject_;
        dataSink.pFormat = NULL;
        dataSink.pLocator = &outputMix;

        const SLInterfaceID playInterfaceIDs[2] =
        {
            //SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
            SL_IID_BUFFERQUEUE,
            SL_IID_VOLUME,
        };
        const SLboolean playRequired[2] =
        {
            SL_BOOLEAN_TRUE,
            SL_BOOLEAN_FALSE,
        };

        SLresult result = 0;
        SLObjectItf outputMixObj = NULL;
        SLObjectItf playerObj = NULL;
        for(s32 i=initParam_.maxPlayers_-1; 0<=i; --i){
            //OutputMix
            //result = engine_.CreateOutputMix(&outputMixObj, 0, NULL, NULL);
            //if(result != SL_RESULT_SUCCESS){
            //    lcore::Log("error: CreateOutputMix");
            //    break;
            //}
            //result = (*outputMixObj)->Realize(outputMixObj, SL_BOOLEAN_FALSE);
            //if(result != SL_RESULT_SUCCESS){
            //    lcore::Log("error: OutputMixObj::Realize");
            //    break;
            //}

            //result = instance_->outputMixObject_.Realize(SL_BOOLEAN_FALSE);
            //if(result != SL_RESULT_SUCCESS){
            //    lcore::Log("error: Realize");
            //    LIME_DELETE(instance_);
            //    return false;
            //}

            //Player
            result = engine_.CreateAudioPlayer(
                &playerObj,
                &dataSource,
                &dataSink,
                2,
                playInterfaceIDs,
                playRequired);
            if(result != SL_RESULT_SUCCESS){
                lcore::Log("error: CreateAudioPlayer %d", result);
                break;
            }

            if(!players_[i].create(initParam_.numQueuedBuffers_, playerObj)){
                break;
            }
            players_[i].link(playerTop_.getNext());
            ++numPlayers_;
        }
        lcore::Log("num created voices %d", numPlayers_);
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
