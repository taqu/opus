#ifndef INC_LSOUND_RESOURCE_H__
#define INC_LSOUND_RESOURCE_H__
/**
@file Resource.h
@author t-sakai
@date 2014/07/15 create
*/
#include "../lsound.h"
#include <stdio.h>
#ifdef ANDROID
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#endif

#include "Pack.h"

//#define LSOUND_RESOURCE_ENABLE_SYNC
#ifdef LSOUND_RESOURCE_ENABLE_SYNC
#include <lcore/async/SyncObject.h>
#endif

namespace lsound
{
    //-------------------------------------------
    //---
    //--- File
    //---
    //-------------------------------------------
    class File
    {
    public:
        File();
        File(FILE* file);
        ~File();

        void addRef();
        void release();
    private:
        friend class FileStream;

        File(const File&);
        File& operator=(const File&);

        s32 refCount_;
        FILE* file_;
#ifdef LSOUND_RESOURCE_ENABLE_SYNC
        lcore::CriticalSection cs_;
#endif
    };

    //-------------------------------------------
    //---
    //--- Memory
    //---
    //-------------------------------------------
    class Memory
    {
    public:
        Memory();
        Memory(u32 size, u8* memory);
        ~Memory();

        void addRef();
        void release();
    private:
        friend class MemoryStream;

        Memory(const Memory&);
        Memory& operator=(const Memory&);

        s32 refCount_;
        u32 size_;
        u8* memory_;
    };

#ifdef ANDROID
    //-------------------------------------------
    //---
    //--- Asset
    //---
    //-------------------------------------------
    class Asset
    {
    public:
        Asset();
        Asset(AAsset* asset);
        ~Asset();

        void addRef();
        void release();
    private:
        friend class AssetStream;
#ifdef LSOUND_USE_WAVE
        friend class Wave;
#endif
        Asset(const Asset&);
        Asset& operator=(const Asset&);

        s32 refCount_;
        AAsset* asset_;
#ifdef LSOUND_RESOURCE_ENABLE_SYNC
        lcore::CriticalSection cs_;
#endif
    };
#endif

    //-------------------------------------------
    //---
    //--- PackResource
    //---
    //-------------------------------------------
    class PackResource
    {
    public:
        enum ResourceType
        {
            ResourceType_File,
            ResourceType_Memory,
            ResourceType_Asset,
        };

        virtual ~PackResource()
        {}

        /// ÉtÉ@ÉCÉãêîéÊìæ
        s32 getNumFiles() const{ return numFiles_;}

        virtual s32 getType() const =0;
    protected:
        PackResource()
            :numFiles_(0)
        {}

        s32 numFiles_;
    };

    //-------------------------------------------
    //---
    //--- PackFile
    //---
    //-------------------------------------------
    class PackFile : public PackResource
    {
    public:
        PackFile();
        virtual ~PackFile();

        virtual s32 getType() const{ return PackResource::ResourceType_File;}

        void get(s32 index, File*& file, s32& start, s32& end);

        static PackFile* open(const Char* path);
    private:
        PackFile(const PackFile&);
        PackFile& operator=(const PackFile&);

        FileEntry* entries_;
        File* file_;
    };

    //-------------------------------------------
    //---
    //--- PackMemory
    //---
    //-------------------------------------------
    class PackMemory : public PackResource
    {
    public:
        PackMemory();
        virtual ~PackMemory();

        virtual s32 getType() const{ return PackResource::ResourceType_Memory;}

        void get(s32 index, Memory*& memory, u32& size, s32& offset);

        static PackMemory* open(const Char* path);

#ifdef ANDROID
        static PackMemory* openFromAsset(AAssetManager* assetManager, const Char* path, s32 mode);
#endif

    private:
        PackMemory(const PackMemory&);
        PackMemory& operator=(const PackMemory&);

        FileEntry* entries_;
        Memory* memory_;
    };

#ifdef ANDROID
    //-------------------------------------------
    //---
    //--- PackAsset
    //---
    //-------------------------------------------
    class PackAsset : public PackResource
    {
    public:
        PackAsset();
        virtual ~PackAsset();

        virtual s32 getType() const{ return PackResource::ResourceType_Asset;}

        void get(s32 index, Asset*& asset, s32& start, s32& end);

        static PackAsset* open(AAssetManager* assetManager, const Char* path, s32 mode);
    private:
        PackAsset(const PackAsset&);
        PackAsset& operator=(const PackAsset&);

        FileEntry* entries_;
        Asset* asset_;
    };
#endif
}
#endif //INC_LSOUND_RESOURCE_H__
