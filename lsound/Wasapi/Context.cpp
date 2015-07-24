/**
@file Context.cpp
@author t-sakai
@date 2014/07/07 create
*/
#include "Context.h"
#include <process.h>
#include <Audioclient.h>
#include <Audiopolicy.h>
#include <lcore/CLibrary.h>
#include "../opus/Stream.h"
#include "../opus/Resource.h"
#include "UserPlayer.h"

#define LSOUND_CONTEXT_DEBUG

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
const IID IID_IAudioClock = __uuidof(IAudioClock);
const IID IID_IAudioStreamVolume = __uuidof(IAudioStreamVolume);
const IID IID_IAudioSessionManager = __uuidof(IAudioSessionManager);

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
        printf(str, time);
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

    u32 __stdcall Context::proc(void* args)
    {
        LSOUND_DECL_STOPWATCH;

        LSOUND_DECL_START;
        //CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        CoInitializeEx(NULL, COINIT_MULTITHREADED);
        Context* context = (Context*)args;
        if(!context->createDevice()){
            context->destroyDevice();

            CoUninitialize();
            _endthreadex(-1);
            return 0;
        }
        LSOUND_DECL_STOP("initialize: %f\n");
        SetEvent(context->initEvent_);

        HANDLE stateEvents[2] = {context->exitEvent_, context->waitEvent_};
        for(;;){
            u32 waitTime = context->getWaitTime();
            LSOUND_DECL_START;
            DWORD ret = WaitForMultipleObjects(2, stateEvents, FALSE, waitTime);
            LSOUND_DECL_STOP("wait: %f\n");

            if((WAIT_OBJECT_0+0) == ret){
                break;
            }

            //DWORD deviceState=0;
            //context->device_.device_->GetState(&deviceState);
            //printf("device state: %d\n", deviceState);

            //AudioSessionState audioSessionState;
            //context->device_.audioSessionControl_->GetState(&audioSessionState);
            //printf("session state: %d\n", audioSessionState);

            //新規リクエスト処理
            context->contextLock_.enter();
            Player* requestList = context->getRequestList();
            context->contextLock_.leave();
            while(NULL != requestList){
                LSOUND_DECL_START;
                Player* request = requestList;
                requestList = requestList->getNext();
                
                if(context->initPlayer(request)){
                    request->link(&context->playList_);
                }else{
                    context->releasePlayer(request);
                }
                LSOUND_DECL_STOP("initial:%f\n")
            }

            //更新
            Player* endList = context->updatePlayers();

            //終了リクエスト処理
            {
                lcore::CSLock lock(context->contextLock_);
                while(NULL != endList){
                    Player* current = endList;
                    endList = endList->getNext();
                    current->resetLink();
                    context->releasePlayer(current);
                }
            }
        }

        context->clear();
        context->destroyDevice();
        CoUninitialize();
        _endthreadex(0);
        return 0;
    }

    Player* Context::updatePlayers()
    {
        Player* endList = NULL;
        Player* player = playList_.getNext();
        while(player != &playList_){

            Player* current = player;
            player = player->getNext();

            //DWORD ret = WaitForSingleObject(player->fillEvent_, 0);
            //if(WAIT_OBJECT_0 != ret){
            //    continue;
            //}

            if(!current->update()){
                current->clear();
                current->unlink();
                current->setNext(endList);
                endList = current;
            }
        }
        return endList;
    }

    bool Context::initPlayer(Player* player)
    {
        player->resetLink();
        return player->initialize();
    }


    Context* Context::instance_ = NULL;

    bool Context::initialize(const InitParam& initParam)
    {
        LASSERT(0<initParam.numQueuedBuffers_ && initParam.numQueuedBuffers_<=lsound::NumMaxBuffers);
        LASSERT(0<initParam.maxPlayers_);

        if(NULL != instance_){
            return true;
        }
        instance_ = LIME_NEW Context(initParam);

        DWORD ret;
        for(s32 i=0; i<10; ++i){
            ret = WaitForSingleObject(instance_->initEvent_, 1000);
            if(WAIT_OBJECT_0 == ret){
                break;
            }
        }
        if(ret != WAIT_OBJECT_0){
            return false;
        }
        return true;
    }

    void Context::terminate()
    {
        LIME_DELETE(instance_);
    }

    bool Context::createDevice()
    {
        HRESULT hr;
        IMMDeviceEnumerator *enumerator = NULL;

        device_.destroy();

        hr = CoCreateInstance(
            CLSID_MMDeviceEnumerator,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IMMDeviceEnumerator,
            (void**)&enumerator);
        if(FAILED(hr)){
            //CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
            CoInitializeEx(NULL, COINIT_MULTITHREADED);
            hr = CoCreateInstance(
                CLSID_MMDeviceEnumerator,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IMMDeviceEnumerator,
                (void**)&enumerator);

            if(FAILED(hr)){
                return false;
            }
        }

        IMMDevice* device = NULL;
        hr = enumerator->GetDefaultAudioEndpoint(
            eRender,
            eConsole,
            &device);
        LSOUND_RELEASE(enumerator);
        if(FAILED(hr)){
            return false;
        }
        device_.device_ = device;

        IAudioSessionManager* audioSessionManager = NULL;
        hr = device->Activate(
            IID_IAudioSessionManager,
            CLSCTX_INPROC_SERVER,
            NULL,
            (void**)&audioSessionManager);

        if(FAILED(hr)){
            device_.destroy();
            return false;
        }
        device_.audioSessionManager_ = audioSessionManager;

        //IAudioSessionControl* audioSessionControl = NULL;
        //hr = device_.audioSessionManager_->GetAudioSessionControl(NULL, 0, &audioSessionControl);
        //if(FAILED(hr)){
        //    device_.destroy();
        //    return false;
        //}
        //device_.audioSessionControl_ = audioSessionControl;

        ISimpleAudioVolume* simpleVolume = NULL;
        hr = device_.audioSessionManager_->GetSimpleAudioVolume(NULL, 0, &simpleVolume);
        if(FAILED(hr)){
            device_.destroy();
            return false;
        }
        device_.simpleVolume_ = simpleVolume;
        device_.simpleVolume_->SetMasterVolume(1.0f, NULL);

        initStreams();
        initPlayers();
        initUserPlayers();
        return true;
    }

    void Context::destroyDevice()
    {
        LIME_DELETE_ARRAY(userPlayers_);
        LIME_DELETE_ARRAY(players_);

        u8* streams = reinterpret_cast<u8*>(streams_);
        LIME_FREE(streams);
        streams = NULL;

        device_.destroy();
    }

    Context::Context(const InitParam& initParam)
        :numStreams_(0)
        ,streamTop_(NULL)
        ,streams_(NULL)
        ,numPlayers_(0)
        ,players_(NULL)
        ,numUserPlayers_(0)
        ,userPlayerTop_(NULL)
        ,userPlayers_(NULL)
        ,numRequests_(0)
        ,requestList_(NULL)
    {
        initParam_ = initParam;
        waitTime_ = initParam_.waitTime_;

        playerTop_.resetLink();
        playList_.resetLink();

        for(s32 i=0; i<NumMaxPacks; ++i){
            packResources_[i] = NULL;
        }

        initEvent_ = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE|SYNCHRONIZE);
        exitEvent_ = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE|SYNCHRONIZE);
        waitEvent_ = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE|SYNCHRONIZE);
        thread_ = reinterpret_cast<HANDLE>( _beginthreadex(
            NULL,
            0,
            Context::proc,
            this,
            0,
            NULL));
    }

    Context::~Context()
    {
        SetEvent(exitEvent_);
        if(NULL != thread_){
            for(s32 i=0; i<10; ++i){
                DWORD ret = WaitForSingleObject(thread_, 1000);
                if(WAIT_OBJECT_0 == ret){
                    break;
                }
            }
            CloseHandle(thread_);
            thread_ = NULL;
        }
        LSOUND_CLOSEHANDLE(waitEvent_);
        LSOUND_CLOSEHANDLE(exitEvent_);
        LSOUND_CLOSEHANDLE(initEvent_);

        for(s32 i=0; i<NumMaxPacks; ++i){
            LIME_DELETE(packResources_[i]);
        }
    }

    void Context::clear()
    {
        lcore::CSLock lock(contextLock_);

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
        Player* requestList = getRequestList();

        while(NULL != requestList){
            Player* request = requestList;
            requestList = requestList->getNext();
            request->resetLink();

            request->clear();
            request->setNext(endList);
            endList = request;
        }

        while(NULL != endList){
            Player* current = endList;
            endList = endList->getNext();
            current->resetLink();
            releasePlayer(current);
        }
    }

    void Context::setGain(f32 gain)
    {
        gain = lcore::clamp01(gain);
        HRESULT hr = device_.simpleVolume_->SetMasterVolume(gain, NULL);
    }

    void Context::setPause(bool pause)
    {
        lcore::CSLock lock(contextLock_);
        if(pause){
            waitTime_ = lcore::thread::Infinite;
        }else{
            waitTime_ = initParam_.waitTime_;
            SetEvent(waitEvent_);
        }
    }

    void Context::updateRequests()
    {
        if(numRequests_){
            SetEvent(waitEvent_);
            numRequests_ = 0;
        }
    }

    s32 Context::loadResourcePack(s32 id, const Char* path, bool stream)
    {
        LASSERT(0<=id && id<NumMaxPacks);
        lcore::CSLock lock(contextLock_);

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
        lcore::CSLock lock(contextLock_);

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
                releasePlayer(player);
                releaseStream((Stream*)streamEntry);
            }
            return false;
        }

        if(!stream->open() || stream->getTotal()<=0){
            releasePlayer(player);
            releaseStream(stream);
            return false;
        }
        player->setStream(stream);
        player->setGain(gain);
        player->clear();

        player->setNext(requestList_);
        requestList_ = player;

        ++numRequests_;
        return true;
    }

    UserPlayer* Context::createUserPlayer(s32 packId, s32 id)
    {
        LASSERT(0<=packId && packId<NumMaxPacks);
        LASSERT(0<=id);
        lcore::CSLock lock(contextLock_);

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
                releaseUserPlayer(userPlayer);
                releasePlayer(player);
                releaseStream((Stream*)streamEntry);
            }
            return NULL;
        }

        if(!stream->open() || stream->getTotal()<=0){
            releaseUserPlayer(userPlayer);
            releasePlayer(player);
            releaseStream(stream);
            return NULL;
        }
        player->setStream(stream);
        player->setGain(1.0f);
        player->setInnerFlag(Player::InnerFlag_UserPlayer);
        
        player->clear();
        player->userPlayer_ = userPlayer;
        userPlayer->impl_.player_ = player;
        userPlayer->initialized_ = 0;
        userPlayer->state_ = State_Initial;

        player->setNext(requestList_);
        requestList_ = player;
        ++numRequests_;
        return userPlayer;
    }

    void Context::destroyUserPlayer(UserPlayer* player)
    {
        if(NULL == player){
            return;
        }
        lcore::CSLock lock(contextLock_);
        player->impl_.player_->stop();
        releaseUserPlayer(player);
    }

    u32 Context::getWaitTime()
    {
        lcore::CSLock lock(contextLock_);
        return waitTime_;
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
        players_ = LIME_NEW Player[initParam_.maxPlayers_];

        numPlayers_ = 0;

        HRESULT hr;
        IAudioClient* audioClient = NULL;

#if 1
        static const s32 channels = 2;
        static const s32 bitsPerSample = BitsPerSample;
        WAVEFORMATEXTENSIBLE fmt;
        fmt.Format.wFormatTag = WAVE_FORMAT_PCM;
        fmt.Format.nChannels = channels;
        fmt.Format.nSamplesPerSec = SampleRate_48000;
        fmt.Format.nAvgBytesPerSec = SampleRate_48000 * channels * (bitsPerSample/8);
        fmt.Format.nBlockAlign = (bitsPerSample/8) * channels;
        fmt.Format.wBitsPerSample = bitsPerSample;
        fmt.Format.cbSize = 0;
#else
        static const s32 channels = 2;
        static const s32 bitsPerSample = 32;
        WAVEFORMATEXTENSIBLE fmt;
        fmt.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        fmt.Format.nChannels = channels;
        fmt.Format.nSamplesPerSec = SampleRate_48000;
        fmt.Format.nAvgBytesPerSec = SampleRate_48000 * channels * (bitsPerSample/8);
        fmt.Format.nBlockAlign = (bitsPerSample/8) * channels;
        fmt.Format.wBitsPerSample = bitsPerSample;
        fmt.Format.cbSize = 22;
        fmt.Samples.wValidBitsPerSample = bitsPerSample;
        fmt.Samples.wSamplesPerBlock = bitsPerSample;
        fmt.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
#endif
        hr = device_.device_->Activate(
                IID_IAudioClient,
                CLSCTX_INPROC_SERVER,
                NULL,
                (void**)&audioClient);
        if(FAILED(hr)){
            return;
        }

        WAVEFORMATEX* mixFormat = NULL;
        hr = audioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, &fmt.Format, &mixFormat);
        if(FAILED(hr)){
            audioClient->Release();
            return;
        }
        if(NULL != mixFormat){
            if(mixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE){
                lcore::memcpy(&fmt, mixFormat, sizeof(WAVEFORMATEXTENSIBLE));

                //2channelまでサポート
                if(Channels_Stereo<fmt.Format.nChannels){
                    fmt.Format.nChannels = Channels_Stereo;
                }

                if(32 == fmt.Format.wBitsPerSample && fmt.SubFormat != KSDATAFORMAT_SUBTYPE_IEEE_FLOAT){
                    fmt.Format.wFormatTag = WAVE_FORMAT_PCM;
                    fmt.Format.nBlockAlign = (BitsPerSample/8) * channels;
                    fmt.Format.wBitsPerSample = BitsPerSample;
                    fmt.Format.cbSize = 0;
                }
            } else{
                lcore::memcpy(&fmt, mixFormat, sizeof(WAVEFORMATEX));
                fmt.Format.wFormatTag = WAVE_FORMAT_PCM;
                fmt.Format.cbSize = 0;

                //2channelまでサポート
                if(Channels_Stereo<fmt.Format.nChannels){
                    fmt.Format.nChannels = Channels_Stereo;
                }
            }
            LSOUND_TASKMEMFREE(mixFormat);
            hr = audioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, &fmt.Format, &mixFormat);
            if(FAILED(hr)){
                audioClient->Release();
                return;
            }
            LSOUND_TASKMEMFREE(mixFormat);
            if(fmt.Format.wBitsPerSample == 8){
                audioClient->Release();
                return;
            }
            if(SampleRate_96000<fmt.Format.nSamplesPerSec){
                audioClient->Release();
                return;
            }
        }
        audioClient->Release();
        audioClient = NULL;

        for(s32 i=initParam_.maxPlayers_-1; 0<=i; --i){
            hr = device_.device_->Activate(
                IID_IAudioClient,
                CLSCTX_INPROC_SERVER,
                NULL,
                (void**)&audioClient);
            if(FAILED(hr)){
                continue;
            }
            if(!players_[i].create(initParam_.numQueuedBuffers_, audioClient, fmt.Format)){
                continue;
            }
            players_[i].link(playerTop_.getNext());
            ++numPlayers_;
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
        player->state_ = lsound::State_Initial;
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
