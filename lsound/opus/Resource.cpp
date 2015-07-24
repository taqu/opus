/**
@file Resource.cpp
@author t-sakai
@date 2014/07/15 create
*/
#include "Resource.h"

namespace lsound
{
    //-------------------------------------------
    //---
    //--- File
    //---
    //-------------------------------------------
    File::File()
        :refCount_(0)
        ,file_(NULL)
    {
    }

    File::File(FILE* file)
        :refCount_(0)
        ,file_(file)
    {
    }

    File::~File()
    {
        if(NULL != file_){
            fclose(file_);
            file_ = NULL;
        }
    }

    void File::addRef()
    {
        ++refCount_;
    }

    void File::release()
    {
        if(--refCount_ == 0){
            LIME_DELETE_NONULL(this);
        }
    }

    //-------------------------------------------
    //---
    //--- Memory
    //---
    //-------------------------------------------
    Memory::Memory()
        :refCount_(0)
        ,size_(0)
        ,memory_(NULL)
    {
    }

    Memory::Memory(u32 size, u8* memory)
        :refCount_(0)
        ,size_(size)
        ,memory_(memory)
    {
    }

    Memory::~Memory()
    {
        LIME_DELETE_ARRAY(memory_);
    }

    void Memory::addRef()
    {
        ++refCount_;
    }

    void Memory::release()
    {
        if(--refCount_ == 0){
            LIME_DELETE_NONULL(this);
        }
    }

#ifdef ANDROID
    //-------------------------------------------
    //---
    //--- Asset
    //---
    //-------------------------------------------
    Asset::Asset()
        :refCount_(0)
        ,asset_(NULL)
    {
    }

    Asset::Asset(AAsset* asset)
        :refCount_(0)
        ,asset_(asset)
    {
    }

    Asset::~Asset()
    {
        if(NULL != asset_){
            AAsset_close(asset_);
            asset_ = NULL;
        }
    }

    void Asset::addRef()
    {
        ++refCount_;
    }

    void Asset::release()
    {
        if(--refCount_ == 0){
            LIME_DELETE_NONULL(this);
        }
    }
#endif

    //-------------------------------------------
    //---
    //--- PackFile
    //---
    //-------------------------------------------
    PackFile::PackFile()
        :entries_(NULL)
        ,file_(NULL)
    {
    }

    PackFile::~PackFile()
    {
        LIME_DELETE_ARRAY(entries_);
        if(NULL != file_){
            file_->release();
        }
    }

    void PackFile::get(s32 index, File*& file, s32& start, s32& end)
    {
        LASSERT(0<=index && index<numFiles_);
        file = file_;
        start = entries_[index].offset_;
        end = start + entries_[index].size_;
    }

    PackFile* PackFile::open(const Char* path)
    {
        LASSERT(NULL != path);
        FILE* f = NULL;
#if defined(_WIN32) || defined(_WIN64)
        fopen_s(&f, path, "rb");
#else
        f = fopen(path, "r");
#endif
        //lcore::Log("PackFile::open %s %s", path, (NULL==f)?"false":"true");
        if(NULL == f){
            return NULL;
        }

        PackHeader header;
        if(0>=fread(&header, sizeof(PackHeader), 1, f)){
            fclose(f);
            return NULL;
        }
        FileEntry* entries = LIME_NEW FileEntry[header.numFiles_];
        if(0>=fread(entries, sizeof(FileEntry)*header.numFiles_, 1, f)){
            LIME_DELETE_ARRAY(entries);
            fclose(f);
            return NULL;
        }

        //ファイル先頭からのオフセットに変換
        s32 dataTop = ftell(f);
        for(s32 i=0; i<header.numFiles_; ++i){
            entries[i].offset_ += dataTop;
        }

        PackFile* packFile = LIME_NEW PackFile();
        packFile->numFiles_ = header.numFiles_;
        packFile->entries_ = entries;
        packFile->file_ = LIME_NEW File(f);
        packFile->file_->addRef();
        return packFile;
    }

    //-------------------------------------------
    //---
    //--- PackMemory
    //---
    //-------------------------------------------
    PackMemory::PackMemory()
        :entries_(NULL)
        ,memory_(NULL)
    {
    }

    PackMemory::~PackMemory()
    {
        LIME_DELETE_ARRAY(entries_);
        if(NULL != memory_){
            memory_->release();
        }
    }

    void PackMemory::get(s32 index, Memory*& memory, u32& size, s32& offset)
    {
        LASSERT(0<=index && index<numFiles_);
        memory = memory_;
        size = entries_[index].size_;
        offset = entries_[index].offset_;
    }

    PackMemory* PackMemory::open(const Char* path)
    {
        LASSERT(NULL != path);
        FILE* f = NULL;
#if defined(_WIN32) || defined(_WIN64)
        fopen_s(&f, path, "rb");
#else
        f = fopen(path, "rb");
#endif
        if(NULL == f){
            return NULL;
        }

        PackHeader header;
        if(0>=fread(&header, sizeof(PackHeader), 1, f)){
            fclose(f);
            return NULL;
        }
        FileEntry* entries = LIME_NEW FileEntry[header.numFiles_];
        if(0>=fread(entries, sizeof(FileEntry)*header.numFiles_, 1, f)){
            LIME_DELETE_ARRAY(entries);
            fclose(f);
            return NULL;
        }

        //データサイズ計算
        s32 dataTop = ftell(f);
        fseek(f, 0, SEEK_END);
        u32 size = ftell(f) - dataTop;
        fseek(f, dataTop, SEEK_SET);

        u8* memory = LIME_NEW u8[size];
        if(0>=fread(memory, size, 1, f)){
            LIME_DELETE_ARRAY(memory);
            LIME_DELETE_ARRAY(entries);
            fclose(f);
            return NULL;
        }
        fclose(f);

        PackMemory* packMemory = LIME_NEW PackMemory();
        packMemory->numFiles_ = header.numFiles_;
        packMemory->entries_ = entries;
        packMemory->memory_ = LIME_NEW Memory(size, memory);
        packMemory->memory_->addRef();
        return packMemory;
    }

#ifdef ANDROID
    PackMemory* PackMemory::openFromAsset(AAssetManager* assetManager, const Char* path, s32 mode)
    {
        LASSERT(NULL != assetManager);
        LASSERT(NULL != path);
        AAsset* asset = AAssetManager_open(assetManager, path, mode);

        if(NULL == asset){
            return NULL;
        }

        s32 pos = 0;
        s32 ret;
        PackHeader header;
        ret = AAsset_read(asset, &header, sizeof(PackHeader));
        if(0>=ret){
            AAsset_close(asset);
            return NULL;
        }
        pos += ret;

        FileEntry* entries = LIME_NEW FileEntry[header.numFiles_];
        ret = AAsset_read(asset, entries, sizeof(FileEntry)*header.numFiles_);
        if(0>=ret){
            LIME_DELETE_ARRAY(entries);
            AAsset_close(asset);
            return NULL;
        }
        pos += ret;

        //データサイズ計算
        s32 dataTop = pos;
        u32 size = AAsset_getLength(asset) - dataTop;

        u8* memory = LIME_NEW u8[size];
        ret = AAsset_read(asset, memory, size);
        AAsset_close(asset);
        if(0>=ret){
            LIME_DELETE_ARRAY(memory);
            LIME_DELETE_ARRAY(entries);
            return NULL;
        }

        PackMemory* packMemory = LIME_NEW PackMemory();
        packMemory->numFiles_ = header.numFiles_;
        packMemory->entries_ = entries;
        packMemory->memory_ = LIME_NEW Memory(size, memory);
        packMemory->memory_->addRef();
        return packMemory;
    }

    //-------------------------------------------
    //---
    //--- PackAsset
    //---
    //-------------------------------------------
    PackAsset::PackAsset()
        :entries_(NULL)
        ,asset_(NULL)
    {
    }

    PackAsset::~PackAsset()
    {
        LIME_DELETE_ARRAY(entries_);
        if(NULL != asset_){
            asset_->release();
        }
    }

    void PackAsset::get(s32 index, Asset*& asset, s32& start, s32& end)
    {
        LASSERT(0<=index && index<numFiles_);
        asset = asset_;
        start = entries_[index].offset_;
        end = start + entries_[index].size_;
    }

    PackAsset* PackAsset::open(AAssetManager* assetManager, const Char* path, s32 mode)
    {
        LASSERT(NULL != assetManager);
        LASSERT(NULL != path);

        AAsset* asset = AAssetManager_open(assetManager, path, mode);

        if(NULL == asset){
            return NULL;
        }

        s32 pos = 0;
        s32 ret;
        PackHeader header;
        ret = AAsset_read(asset, &header, sizeof(PackHeader));
        if(0>=ret){
            AAsset_close(asset);
            return NULL;
        }
        pos += ret;

        FileEntry* entries = LIME_NEW FileEntry[header.numFiles_];
        ret = AAsset_read(asset, entries, sizeof(FileEntry)*header.numFiles_);
        if(0>=ret){
            LIME_DELETE_ARRAY(entries);
            AAsset_close(asset);
            return NULL;
        }
        pos += ret;

        //ファイル先頭からのオフセットに変換
        s32 dataTop = pos;
        for(s32 i=0; i<header.numFiles_; ++i){
            entries[i].offset_ += dataTop;
        }

        PackAsset* packAsset = LIME_NEW PackAsset();
        packAsset->numFiles_ = header.numFiles_;
        packAsset->entries_ = entries;
        packAsset->asset_ = LIME_NEW Asset(asset);
        packAsset->asset_->addRef();
        return packAsset;
    }
#endif
}
