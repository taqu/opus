/**
@file Stream.h
@author t-sakai
@date 2014/07/11 create
*/
#include "Stream.h"
#include "Resource.h"

#ifdef LSOUND_USE_WAVE
#if defined(_WIN32) || defined(_WIN64)
#include "../dsp/dsp.h"
#else
#include <string.h>
namespace lsound
{
namespace
{
    static inline LSfloat toFloat(LSbyte v)
    {
        return (1.0f/127.0f)*v;
    }

    static inline LSfloat toFloat(LSshort v)
    {
        return (1.0f/32767.0f)*v;
    }
}
}
#endif
#endif

namespace lsound
{
#ifndef LSOUND_USE_WAVE
    //-------------------------------------------
    //---
    //--- Stream
    //---
    //-------------------------------------------
    Stream::Stream()
        :opusFile_(NULL)
        ,total_(0)
        ,position_(0)
        ,format_(Format_Stereo16)
        ,channels_(Channels_Stereo)
    {
    }

    Stream::~Stream()
    {
        close();
    }

    void Stream::close()
    {
        if(NULL == opusFile_){
            return;
        }
        op_free(opusFile_);
        opusFile_ = NULL;
    }

    void Stream::setInfo()
    {
        total_ = op_pcm_total(opusFile_, 0);
        position_ = op_pcm_tell(opusFile_);
        s32 numChannels = op_channel_count(opusFile_, 0);

        switch(numChannels)
        {
        case 1:
            format_ = Format_Mono16;
            channels_ = Channels_Mono;
            break;

        default:
            format_ = Format_Stereo16;
            channels_ = Channels_Stereo;
            break;
        };
    }

    void Stream::swap(Stream& rhs)
    {
        lcore::swap(opusFile_, rhs.opusFile_);
        lcore::swap(total_, rhs.total_);
        lcore::swap(position_, rhs.position_);
        lcore::swap(format_, rhs.format_);
        lcore::swap(channels_, rhs.channels_);
    }

    //-------------------------------------------
    //---
    //--- FileStream
    //---
    //-------------------------------------------
    OpusFileCallbacks FileStream::callbacks_ =
    {
        FileStream::read_func,
        FileStream::seek_func,
        FileStream::tell_func,
        FileStream::close_func,
    };

    FileStream::FileStream()
        :start_(0)
        ,end_(0)
        ,current_(0)
        ,file_(NULL)
    {
    }

    FileStream::~FileStream()
    {
        if(NULL != file_){
            file_->release();
        }
    }

    void FileStream::set(File* file, opus_int64 start, opus_int64 end)
    {
        LASSERT(NULL != file);
        LASSERT(0<=start);
        LASSERT(start<=end);

        if(NULL != file_){
            file_->release();
        }

        file_ = file;
        file_->addRef();

        start_ = start;
        end_ = end;
        current_ = start_;
    }

    bool FileStream::open()
    {
        LASSERT(NULL == opusFile_);
        opusFile_ = op_open_callbacks(this, &callbacks_, NULL, 0, NULL);
        if(NULL == opusFile_){
            return false;
        }
        op_set_dither_enabled(opusFile_, LSOUND_DITHER_ENABLE);
        setInfo();
        return true;
    }


    void FileStream::swap(FileStream& rhs)
    {
        Stream::swap(rhs);

        lcore::swap(start_, rhs.start_);
        lcore::swap(end_, rhs.end_);
        lcore::swap(current_, rhs.current_);
        lcore::swap(file_, rhs.file_);
    }

    int FileStream::read_func(void* stream, unsigned char* ptr, int nbytes)
    {
        FileStream* fileStream = (FileStream*)stream;
#ifdef LSOUND_RESOURCE_ENABLE_SYNC
        lcore::CSLock lock(fileStream->file_->cs_);
#endif
        FILE* file = fileStream->file_->file_;

        fseek(file, fileStream->current_, SEEK_SET);
        size_t bytes = fread(ptr, 1, nbytes, file);
        fileStream->current_ += bytes;
        return static_cast<int>(bytes);
    }

    int FileStream::seek_func(void* stream, opus_int64 offset, int whence)
    {
        FileStream* fileStream = (FileStream*)stream;

        switch(whence)
        {
        case SEEK_SET:
            fileStream->current_ = fileStream->start_ + offset;
            break;

        case SEEK_CUR:
            fileStream->current_ += offset;
            break;

        case SEEK_END:
            fileStream->current_ = fileStream->end_ - offset;
            break;
        }
        return 0;
    }

    opus_int64 FileStream::tell_func(void* stream)
    {
        FileStream* fileStream = (FileStream*)stream;
        return (fileStream->current_ - fileStream->start_);
    }

    int FileStream::close_func(void*)
    {
        return 0;
    }

    //-------------------------------------------
    //---
    //--- MemoryStream
    //---
    //-------------------------------------------
    MemoryStream::MemoryStream()
        :size_(0)
        ,memory_(NULL)
    {
    }

    MemoryStream::~MemoryStream()
    {
        if(NULL != memory_){
            memory_->release();
        }
    }

    void MemoryStream::set(u32 size, u32 offset, Memory* memory)
    {
        LASSERT(0<size);
        LASSERT(NULL != memory);
        if(NULL != memory_){
            memory_->release();
        }

        size_ = size;
        offset_ = offset;
        memory_ = memory;
        memory_->addRef();
    }

    bool MemoryStream::open()
    {
        LASSERT(NULL == opusFile_);
        opusFile_ = op_open_memory(memory_->memory_+offset_, size_, NULL);
        if(NULL == opusFile_){
            return false;
        }
        op_set_dither_enabled(opusFile_, LSOUND_DITHER_ENABLE);
        setInfo();
        return true;
    }

    void MemoryStream::swap(MemoryStream& rhs)
    {
        Stream::swap(rhs);

        lcore::swap(size_, rhs.size_);
        lcore::swap(offset_, rhs.offset_);
        lcore::swap(memory_, rhs.memory_);
    }

#ifdef ANDROID
    //-------------------------------------------
    //---
    //--- AssetStream
    //---
    //-------------------------------------------
    OpusFileCallbacks AssetStream::callbacks_ =
    {
        AssetStream::read_func,
        AssetStream::seek_func,
        AssetStream::tell_func,
        AssetStream::close_func,
    };

    AssetStream::AssetStream()
        :start_(0)
        ,end_(0)
        ,current_(0)
        ,asset_(NULL)
    {
    }

    AssetStream::~AssetStream()
    {
        if(NULL != asset_){
            asset_->release();
        }
    }

    void AssetStream::set(Asset* asset, opus_int64 start, opus_int64 end)
    {
        LASSERT(NULL != asset);
        LASSERT(0<=start);
        LASSERT(start<=end);

        if(NULL != asset_){
            asset_->release();
        }

        asset_ = asset;
        asset_->addRef();

        start_ = start;
        end_ = end;
        current_ = start_;
    }

    bool AssetStream::open()
    {
        LASSERT(NULL == opusFile_);
        opusFile_ = op_open_callbacks(this, &callbacks_, NULL, 0, NULL);
        if(NULL == opusFile_){
            return false;
        }
        op_set_dither_enabled(opusFile_, LSOUND_DITHER_ENABLE);
        setInfo();
        return true;
    }


    void AssetStream::swap(AssetStream& rhs)
    {
        Stream::swap(rhs);

        lcore::swap(start_, rhs.start_);
        lcore::swap(end_, rhs.end_);
        lcore::swap(current_, rhs.current_);
        lcore::swap(asset_, rhs.asset_);
    }

    int AssetStream::read_func(void* stream, unsigned char* ptr, int nbytes)
    {
        AssetStream* assetStream = (AssetStream*)stream;
#ifdef LSOUND_RESOURCE_ENABLE_SYNC
        lcore::CSLock lock(assetStream->asset_->cs_);
#endif
        AAsset* asset = assetStream->asset_->asset_;

        AAsset_seek64(asset, assetStream->current_, SEEK_SET);
        s32 bytes = AAsset_read(asset, ptr, nbytes);

        if(0<bytes){
            assetStream->current_ += bytes;
        }
        return bytes;
    }

    int AssetStream::seek_func(void* stream, opus_int64 offset, int whence)
    {
        AssetStream* assetStream = (AssetStream*)stream;

        switch(whence)
        {
        case SEEK_SET:
            assetStream->current_ = assetStream->start_ + offset;
            break;

        case SEEK_CUR:
            assetStream->current_ += offset;
            break;

        case SEEK_END:
            assetStream->current_ = assetStream->end_ - offset;
            break;
        }
        return 0;
    }

    opus_int64 AssetStream::tell_func(void* stream)
    {
        AssetStream* assetStream = (AssetStream*)stream;
        return (assetStream->current_ - assetStream->start_);
    }

    int AssetStream::close_func(void*)
    {
        return 0;
    }
#endif
#else //LSOUND_USE_WAVE


    s32 Wave::Memory::tell() const
    {
        return position_;
    }

    void Wave::Memory::advance(s32 offset)
    {
        position_ += offset;
    }

    s32 Wave::Memory::read(void* dst, u32 size)
    {
        memcpy(dst, memory_+position_, size);
        position_ += size;
        return 1;
    }

    Wave::Wave()
    {}

    Wave::~Wave()
    {}

    bool Wave::read(FILE* file)
    {
        WaveHeader header;
        if(1 != fread(&header, sizeof(WaveHeader), 1, file)){
            return false;
        }
        if(header.id_ != WaveID_RIFF || header.tag_ != WaveID_WAVE){
            return false;
        }

        Chunk chunk = readChunkHeader(file);
        while(0 != chunk.id_){
            switch(chunk.id_)
            {
            case WaveID_fmt:
                if(!readFmt(file, chunk)){
                    return false;
                }

                break;
            case WaveID_fact:
                readThroughChunk(file, chunk);
                break;
            case WaveID_data:
                if(!readData(file, chunk)){
                    return false;
                }
                break;
            case WaveID_LIST:
                readThroughChunk(file, chunk);
                break;
            default:
                readThroughChunk(file, chunk);
                break;
            }
            chunk = readChunkHeader(file);
        }
        return true;
    }

    Chunk Wave::readChunkHeader(FILE* file)
    {
        Chunk chunk;
        if(1 != fread(&chunk, sizeof(Chunk), 1, file)){
            chunk.id_ = 0;
        }
        return chunk;
    }

    void Wave::readThroughChunk(FILE* file, const Chunk& chunk)
    {
        s32 offset = chunk.size_;
        fseek(file, offset, SEEK_CUR);
    }

    bool Wave::readFmt(FILE* file, const Chunk& chunk)
    {
        ChunkFmt chunkFmt;
        if(1!=fread(&(chunkFmt.format_), sizeof(ChunkFmt)-sizeof(Chunk), 1, file)){
            return false;
        }
        format_ = chunkFmt.format_;
        channels_ = chunkFmt.channels_;
        samplesPerSec_ = chunkFmt.samplesPerSec_;
        bytesPerSec_ = chunkFmt.bytesPerSec_;
        blockSize_ = chunkFmt.blockSize_;
        bitsPerSample_ = chunkFmt.bitsPerSample_;
        if(sizeof(ChunkFmt)<chunk.size_){
            s32 offset = chunk.size_ - sizeof(ChunkFmt);
            fseek(file, offset, SEEK_CUR);
        }
        return true;
    }

    bool Wave::readData(FILE* file, const Chunk& chunk)
    {
        dataOffset_ = ftell(file);
        dataSize_ = chunk.size_;
        readThroughChunk(file, chunk);
        return true;
    }

    bool Wave::read(u8* mem)
    {
        Memory memory(mem);

        WaveHeader header;
        if(1 != memory.read(&header, sizeof(WaveHeader))){
            return 0;
        }
        if(header.id_ != WaveID_RIFF || header.tag_ != WaveID_WAVE){
            return false;
        }

        Chunk chunk = readChunkHeader(memory);
        while(0 != chunk.id_){
            switch(chunk.id_)
            {
            case WaveID_fmt:
                if(!readFmt(memory, chunk)){
                    return false;
                }

                break;
            case WaveID_fact:
                readThroughChunk(memory, chunk);
                break;
            case WaveID_data:
                if(!readData(memory, chunk)){
                    return false;
                }
                break;
            case WaveID_LIST:
                readThroughChunk(memory, chunk);
                break;
            default:
                readThroughChunk(memory, chunk);
                break;
            }
            chunk = readChunkHeader(memory);
        }
        return true;
    }

    Chunk Wave::readChunkHeader(Memory& memory)
    {
        Chunk chunk;
        if(1 != memory.read(&chunk, sizeof(Chunk))){
            chunk.id_ = 0;
        }
        return chunk;
    }

    void Wave::readThroughChunk(Memory& memory, const Chunk& chunk)
    {
        s32 offset = chunk.size_;
        memory.advance(offset);
    }

    bool Wave::readFmt(Memory& memory, const Chunk& chunk)
    {
        ChunkFmt chunkFmt;
        if(1!=memory.read(&(chunkFmt.format_), sizeof(ChunkFmt)-sizeof(Chunk))){
            return false;
        }
        format_ = chunkFmt.format_;
        channels_ = chunkFmt.channels_;
        samplesPerSec_ = chunkFmt.samplesPerSec_;
        bytesPerSec_ = chunkFmt.bytesPerSec_;
        blockSize_ = chunkFmt.blockSize_;
        bitsPerSample_ = chunkFmt.bitsPerSample_;
        if(sizeof(ChunkFmt)<chunk.size_){
            s32 offset = chunk.size_ - sizeof(ChunkFmt);
            memory.advance(offset);
        }
        return true;
    }

    bool Wave::readData(Memory& memory, const Chunk& chunk)
    {
        dataOffset_ = memory.tell();
        dataSize_ = chunk.size_;
        readThroughChunk(memory, chunk);
        return true;
    }

#ifdef ANDROID
    bool Wave::read(Asset* asset)
    {
        WaveHeader header;
        if(AAsset_read(asset->asset_, &header, sizeof(WaveHeader)) <= 0){
            return false;
        }
        if(header.id_ != WaveID_RIFF || header.tag_ != WaveID_WAVE){
            return false;
        }

        Chunk chunk = readChunkHeader(asset);
        while(0 != chunk.id_){
            switch(chunk.id_)
            {
            case WaveID_fmt:
                if(!readFmt(asset, chunk)){
                    return false;
                }

                break;
            case WaveID_fact:
                readThroughChunk(asset, chunk);
                break;
            case WaveID_data:
                if(!readData(asset, chunk)){
                    return false;
                }
                break;
            case WaveID_LIST:
                readThroughChunk(asset, chunk);
                break;
            default:
                readThroughChunk(asset, chunk);
                break;
            }
            chunk = readChunkHeader(asset);
        }
        return true;
    }

    Chunk Wave::readChunkHeader(Asset* asset)
    {
        Chunk chunk;
        if(AAsset_read(asset->asset_, &chunk, sizeof(Chunk)) <= 0){
            chunk.id_ = 0;
        }
        return chunk;
    }

    void Wave::readThroughChunk(Asset* asset, const Chunk& chunk)
    {
        s32 offset = chunk.size_;
        AAsset_seek64(asset->asset_, offset, SEEK_CUR);
    }

    bool Wave::readFmt(Asset* asset, const Chunk& chunk)
    {
        ChunkFmt chunkFmt;
        if(AAsset_read(asset->asset_, &(chunkFmt.format_), sizeof(ChunkFmt)-sizeof(Chunk)) <= 0){
            return false;
        }
        format_ = chunkFmt.format_;
        channels_ = chunkFmt.channels_;
        samplesPerSec_ = chunkFmt.samplesPerSec_;
        bytesPerSec_ = chunkFmt.bytesPerSec_;
        blockSize_ = chunkFmt.blockSize_;
        bitsPerSample_ = chunkFmt.bitsPerSample_;
        if(sizeof(ChunkFmt)<chunk.size_){
            s32 offset = chunk.size_ - sizeof(ChunkFmt);
            AAsset_seek64(asset->asset_, offset, SEEK_CUR);
        }
        return true;
    }

    bool Wave::readData(Asset* asset, const Chunk& chunk)
    {
        dataOffset_ = AAsset_getLength64(asset->asset_) - AAsset_getRemainingLength64(asset->asset_);
        dataSize_ = chunk.size_;
        readThroughChunk(asset, chunk);
        return true;
    }
#endif

    //-------------------------------------------
    //---
    //--- Stream
    //---
    //-------------------------------------------
    Stream::Stream()
        :total_(0)
        ,position_(0)
        ,format_(Format_Stereo16)
    {
    }

    Stream::~Stream()
    {
        close();
    }

    void Stream::close()
    {
    }

    void Stream::setInfo()
    {
        total_ = wave_.dataSize_/wave_.channels_/(wave_.bitsPerSample_/8);
        position_ = 0;
        switch(wave_.channels_)
        {
        case 1:
            format_ = Format_Mono16;
            break;

        default:
            format_ = Format_Stereo16;
            break;
        };
    }

    void Stream::swap(Stream& rhs)
    {
        lcore::swap(wave_, rhs.wave_);
        lcore::swap(total_, rhs.total_);
        lcore::swap(position_, rhs.position_);
        lcore::swap(format_, rhs.format_);
    }

    //-------------------------------------------
    //---
    //--- FileStream
    //---
    //-------------------------------------------
    FileStream::FileStream()
        :start_(0)
        ,end_(0)
        ,current_(0)
        ,file_(NULL)
    {
    }

    FileStream::~FileStream()
    {
        if(NULL != file_){
            file_->release();
        }
    }

    void FileStream::set(File* file, opus_int64 start, opus_int64 end)
    {
        LASSERT(NULL != file);
        LASSERT(0<=start);
        LASSERT(start<=end);

        if(NULL != file_){
            file_->release();
        }

        file_ = file;
        file_->addRef();

        start_ = start;
        end_ = end;
        current_ = start_;
    }

    bool FileStream::open()
    {
        fseek(file_->file_, start_, SEEK_SET);
        if(!wave_.read(file_->file_)){
            return false;
        }
        wave_.dataOffset_ -= start_;
        setInfo();
        return true;
    }


    void FileStream::swap(FileStream& rhs)
    {
        Stream::swap(rhs);

        lcore::swap(start_, rhs.start_);
        lcore::swap(end_, rhs.end_);
        lcore::swap(current_, rhs.current_);
        lcore::swap(file_, rhs.file_);
    }

    s32 FileStream::read(opus_int16* pcm, s32 numSamples)
    {
#ifdef LSOUND_RESOURCE_ENABLE_SYNC
        lcore::CSLock lock(fileStream->file_->cs_);
#endif
        fseek(file_->file_, current_+wave_.dataOffset_, SEEK_SET);
        numSamples = lcore::minimum(numSamples, total_-position_);

        u32 bytesPerFrame = wave_.channels_ * (wave_.bitsPerSample_>>3);
        u32 bsize = numSamples * bytesPerFrame;
        size_t bytes = fread(pcm, 1, bsize, file_->file_);
        current_ += bytes;

        s32 samples = bytes/bytesPerFrame;
        if(0<samples){
            position_ += samples;
        }
        return samples;
    }

    s32 FileStream::read_stereo(opus_int16* pcm, s32 numSamples)
    {
#ifdef LSOUND_RESOURCE_ENABLE_SYNC
        lcore::CSLock lock(fileStream->file_->cs_);
#endif
        fseek(file_->file_, current_+wave_.dataOffset_, SEEK_SET);
        numSamples = lcore::minimum(numSamples, total_-position_);

        u32 bytesPerFrame = wave_.channels_ * (wave_.bitsPerSample_>>3);
        u32 bsize = numSamples * bytesPerFrame;
        size_t bytes = fread(pcm, 1, bsize, file_->file_);
        current_ += bytes;

        s32 samples = bytes/bytesPerFrame;
        if(0<samples){
            position_ += samples;
        }

        if(wave_.channels_<2){
            for(s32 i=samples-1; 0<=i; --i){
                s32 j=i<<1;
                pcm[j+0] = pcm[i];
                pcm[j+1] = pcm[i];
            }
        }
        return samples;
    }

    s32 FileStream::read_float(f32* pcm, s32 numSamples)
    {
#ifdef LSOUND_RESOURCE_ENABLE_SYNC
        lcore::CSLock lock(fileStream->file_->cs_);
#endif
        fseek(file_->file_, current_+wave_.dataOffset_, SEEK_SET);
        numSamples = lcore::minimum(numSamples, total_-position_);

        u32 bytesPerFrame = wave_.channels_ * (wave_.bitsPerSample_>>3);
        u32 bsize = numSamples * bytesPerFrame;
        size_t bytes = fread(pcm, 1, bsize, file_->file_);
        current_ += bytes;

        s32 samples = bytes/bytesPerFrame;
        if(0<samples){
            position_ += samples;
        }

        samples *= wave_.channels_;
        opus_int16* pcm16 = reinterpret_cast<opus_int16*>(pcm);
        for(s32 i=samples-1; 0<=i; --i){
            pcm[i] = toFloat(pcm16[i]);
        }
        return samples;
    }

    s32 FileStream::read_float_stereo(f32* pcm, s32 numSamples)
    {
#ifdef LSOUND_RESOURCE_ENABLE_SYNC
        lcore::CSLock lock(fileStream->file_->cs_);
#endif
        fseek(file_->file_, current_+wave_.dataOffset_, SEEK_SET);
        numSamples = lcore::minimum(numSamples, total_-position_);

        u32 bytesPerFrame = wave_.channels_ * (wave_.bitsPerSample_>>3);
        u32 bsize = numSamples * bytesPerFrame;
        size_t bytes = fread(pcm, 1, bsize, file_->file_);
        current_ += bytes;

        s32 samples = bytes/bytesPerFrame;
        if(0<samples){
            position_ += samples;
        }

        opus_int16* pcm16 = reinterpret_cast<opus_int16*>(pcm);
        if(wave_.channels_<2){
            for(s32 i=samples-1; 0<=i; --i){
                s32 j=i<<1;
                pcm[j+0] = pcm[j+1] = toFloat(pcm16[i]);
            }
        } else{
            samples *= wave_.channels_;
            for(s32 i=samples-1; 0<=i; --i){
                pcm[i] = toFloat(pcm16[i]);
            }
        }
        return samples;
    }

    opus_int64 FileStream::tell()
    {
        return position_;
    }

    s32 FileStream::seek(opus_int64 offset)
    {
        offset = lcore::clamp(offset, 0LL, total_-1);
        s32 bytesPerFrame = wave_.channels_ * (wave_.bitsPerSample_>>3);
        current_ = start_ + offset*bytesPerFrame;
        position_ = offset;
        return 0;
    }


    //-------------------------------------------
    //---
    //--- MemoryStream
    //---
    //-------------------------------------------
    MemoryStream::MemoryStream()
        :size_(0)
        ,memory_(NULL)
    {
    }

    MemoryStream::~MemoryStream()
    {
        if(NULL != memory_){
            memory_->release();
        }
    }

    void MemoryStream::set(u32 size, u32 offset, Memory* memory)
    {
        LASSERT(0<size);
        LASSERT(NULL != memory);
        if(NULL != memory_){
            memory_->release();
        }

        size_ = size;
        offset_ = offset;
        memory_ = memory;
        memory_->addRef();
    }

    bool MemoryStream::open()
    {
        if(!wave_.read(memory_->memory_ + offset_)){
            return false;
        }
        wave_.dataOffset_ += offset_;
        setInfo();
        return true;
    }

    void MemoryStream::swap(MemoryStream& rhs)
    {
        Stream::swap(rhs);

        lcore::swap(size_, rhs.size_);
        lcore::swap(offset_, rhs.offset_);
        lcore::swap(memory_, rhs.memory_);
    }

    s32 MemoryStream::read(opus_int16* pcm, s32 size)
    {
#ifdef LSOUND_RESOURCE_ENABLE_SYNC
        lcore::CSLock lock(fileStream->file_->cs_);
#endif
        u32 bytesPerFrame = wave_.channels_ * (wave_.bitsPerSample_>>3);
        u32 bsize = size * bytesPerFrame;

        size_t bytes = memread(pcm, bsize);

        s32 samples = bytes/bytesPerFrame;
        if(0<samples){
            position_ += samples;
        }
        return samples;
    }

    s32 MemoryStream::read_stereo(opus_int16* pcm, s32 size)
    {
#ifdef LSOUND_RESOURCE_ENABLE_SYNC
        lcore::CSLock lock(fileStream->file_->cs_);
#endif
        u32 bytesPerFrame = wave_.channels_ * (wave_.bitsPerSample_>>3);
        u32 bsize = size * bytesPerFrame;
        size_t bytes = memread(pcm, bsize);

        s32 samples = bytes/bytesPerFrame;
        if(0<samples){
            position_ += samples;
        }

        if(wave_.channels_<2){
            for(s32 i=samples-1; 0<=i; --i){
                s32 j=i<<1;
                pcm[j+0] = pcm[i];
                pcm[j+1] = pcm[i];
            }
        }
        return samples;
    }

    s32 MemoryStream::read_float(f32* pcm, s32 size)
    {
#ifdef LSOUND_RESOURCE_ENABLE_SYNC
        lcore::CSLock lock(fileStream->file_->cs_);
#endif
        u32 bytesPerFrame = wave_.channels_ * (wave_.bitsPerSample_>>3);
        u32 bsize = size * bytesPerFrame;
        size_t bytes = memread(pcm, bsize);

        s32 samples = bytes/bytesPerFrame;
        if(0<samples){
            position_ += samples;
        }

        samples *= wave_.channels_;
        opus_int16* pcm16 = reinterpret_cast<opus_int16*>(pcm);
        for(s32 i=samples-1; 0<=i; --i){
            pcm[i] = toFloat(pcm16[i]);
        }
        return samples;
    }

    s32 MemoryStream::read_float_stereo(f32* pcm, s32 size)
    {
#ifdef LSOUND_RESOURCE_ENABLE_SYNC
        lcore::CSLock lock(fileStream->file_->cs_);
#endif
        u32 bytesPerFrame = wave_.channels_ * (wave_.bitsPerSample_>>3);
        u32 bsize = size * bytesPerFrame;
        size_t bytes = memread(pcm, bsize);

        s32 samples = bytes/bytesPerFrame;
        if(0<samples){
            position_ += samples;
        }

        opus_int16* pcm16 = reinterpret_cast<opus_int16*>(pcm);
        if(wave_.channels_<2){
            for(s32 i=samples-1; 0<=i; --i){
                s32 j=i<<1;
                pcm[j+0] = pcm[j+1] = toFloat(pcm16[i]);
            }
        } else{
            samples *= wave_.channels_;
            for(s32 i=samples-1; 0<=i; --i){
                pcm[i] = toFloat(pcm16[i]);
            }
        }
        return samples;
    }

    opus_int64 MemoryStream::tell()
    {
        return position_;
    }

    s32 MemoryStream::seek(opus_int64 offset)
    {
        offset = lcore::clamp(offset, 0LL, total_-1);
        position_ = offset;
        return 0;
    }

    s32 MemoryStream::memread(void* dst, s32 size) const
    {
        s32 bytesPerFrame = wave_.channels_ * (wave_.bitsPerSample_>>3);
        s64 s = position_*bytesPerFrame;
        size = (size_<(s+size))? size_-s : size;

        u8* ptr = memory_->memory_ + offset_ + s;
        memcpy(dst, ptr, size);
        return size;
    }

#ifdef ANDROID
    //-------------------------------------------
    //---
    //--- AssetStream
    //---
    //-------------------------------------------
    AssetStream::AssetStream()
        :start_(0)
        ,end_(0)
        ,current_(0)
        ,asset_(NULL)
    {
    }

    AssetStream::~AssetStream()
    {
        if(NULL != asset_){
            asset_->release();
        }
    }

    void AssetStream::set(Asset* asset, opus_int64 start, opus_int64 end)
    {
        LASSERT(NULL != asset);
        LASSERT(0<=start);
        LASSERT(start<=end);

        if(NULL != asset_){
            asset_->release();
        }

        asset_ = asset;
        asset_->addRef();

        start_ = start;
        end_ = end;
        current_ = start_;
    }

    bool AssetStream::open()
    {
        AAsset* asset = asset_->asset_;
        AAsset_seek64(asset, start_, SEEK_SET);
        lcore::Log("asset start: %lld", start_);
        if(!wave_.read(asset_)){
            return false;
        }
        wave_.dataOffset_ -= start_;
        setInfo();
        return true;
    }


    void AssetStream::swap(AssetStream& rhs)
    {
        Stream::swap(rhs);

        lcore::swap(start_, rhs.start_);
        lcore::swap(end_, rhs.end_);
        lcore::swap(current_, rhs.current_);
        lcore::swap(asset_, rhs.asset_);
    }

    s32 AssetStream::read(opus_int16* pcm, s32 numSamples)
    {
#ifdef LSOUND_RESOURCE_ENABLE_SYNC
        lcore::CSLock lock(asset_->cs_);
#endif
        AAsset* asset = asset_->asset_;

        AAsset_seek64(asset, current_+wave_.dataOffset_, SEEK_SET);
        numSamples = lcore::minimum(numSamples, total_-position_);

        u32 bytesPerFrame = wave_.channels_ * (wave_.bitsPerSample_>>3);
        u32 bsize = numSamples * bytesPerFrame;
        size_t bytes = AAsset_read(asset, pcm, bsize);
        current_ += bytes;

        s32 samples = bytes/bytesPerFrame;
        if(0<samples){
            position_ += samples;
        }
        return samples;
    }

    s32 AssetStream::read_stereo(opus_int16* pcm, s32 numSamples)
    {
#ifdef LSOUND_RESOURCE_ENABLE_SYNC
        lcore::CSLock lock(asset_->cs_);
#endif
        AAsset* asset = asset_->asset_;

        AAsset_seek64(asset, current_+wave_.dataOffset_, SEEK_SET);
        numSamples = lcore::minimum(numSamples, total_-position_);

        u32 bytesPerFrame = wave_.channels_ * (wave_.bitsPerSample_>>3);
        u32 bsize = numSamples * bytesPerFrame;
        size_t bytes = AAsset_read(asset, pcm, bsize);
        current_ += bytes;

        s32 samples = bytes/bytesPerFrame;
        if(0<samples){
            position_ += samples;
        }

        if(wave_.channels_<2){
            for(s32 i=samples-1; 0<=i; --i){
                s32 j=i<<1;
                pcm[j+0] = pcm[i];
                pcm[j+1] = pcm[i];
            }
        }
        return samples;
    }

    s32 AssetStream::read_float(f32* pcm, s32 numSamples)
    {
#ifdef LSOUND_RESOURCE_ENABLE_SYNC
        lcore::CSLock lock(asset_->cs_);
#endif
        AAsset* asset = asset_->asset_;

        AAsset_seek64(asset, current_+wave_.dataOffset_, SEEK_SET);
        numSamples = lcore::minimum(numSamples, total_-position_);

        u32 bytesPerFrame = wave_.channels_ * (wave_.bitsPerSample_>>3);
        u32 bsize = numSamples * bytesPerFrame;
        size_t bytes = AAsset_read(asset, pcm, bsize);
        current_ += bytes;

        s32 samples = bytes/bytesPerFrame;
        if(0<samples){
            position_ += samples;
        }

        samples *= wave_.channels_;
        opus_int16* pcm16 = reinterpret_cast<opus_int16*>(pcm);
        for(s32 i=samples-1; 0<=i; --i){
            pcm[i] = toFloat(pcm16[i]);
        }
        return samples;
    }

    s32 AssetStream::read_float_stereo(f32* pcm, s32 numSamples)
    {
#ifdef LSOUND_RESOURCE_ENABLE_SYNC
        lcore::CSLock lock(asset_->cs_);
#endif
        AAsset* asset = asset_->asset_;

        AAsset_seek64(asset, current_+wave_.dataOffset_, SEEK_SET);
        numSamples = lcore::minimum(numSamples, total_-position_);

        u32 bytesPerFrame = wave_.channels_ * (wave_.bitsPerSample_>>3);
        u32 bsize = numSamples * bytesPerFrame;
        size_t bytes = AAsset_read(asset, pcm, bsize);
        current_ += bytes;

        s32 samples = bytes/bytesPerFrame;
        if(0<samples){
            position_ += samples;
        }

        opus_int16* pcm16 = reinterpret_cast<opus_int16*>(pcm);
        if(wave_.channels_<2){
            for(s32 i=samples-1; 0<=i; --i){
                s32 j=i<<1;
                pcm[j+0] = pcm[j+1] = toFloat(pcm16[i]);
            }
        } else{
            samples *= wave_.channels_;
            for(s32 i=samples-1; 0<=i; --i){
                pcm[i] = toFloat(pcm16[i]);
            }
        }
        return samples;
    }

    opus_int64 AssetStream::tell()
    {
        return position_;
    }

    s32 AssetStream::seek(opus_int64 offset)
    {
        offset = lcore::clamp(offset, 0LL, total_-1);
        s32 bytesPerFrame = wave_.channels_ * (wave_.bitsPerSample_>>3);
        current_ = start_ + offset*bytesPerFrame;
        position_ = offset;
        return 0;
    }
#endif
#endif //LSOUND_USE_WAVE
}
