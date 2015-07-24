#ifndef INC_LSOUND_STREAM_H__
#define INC_LSOUND_STREAM_H__
/**
@file Stream.h
@author t-sakai
@date 2014/07/11 create
*/
#include "../lsound.h"
#include <opus/opusfile.h>


namespace lsound
{
    class File;
    class Memory;
    class Asset;

    enum Error
    {
        Error_Hole = OP_HOLE,
        Error_Read = OP_EREAD,
        Error_Fault = OP_EFAULT,
        Error_Impl = OP_EIMPL,
        Error_Inval = OP_EINVAL,
        Error_NotFormat = OP_ENOTFORMAT,
        Error_BadHeader = OP_EBADHEADER,
        Error_Version = OP_EVERSION,
        Error_BadPacket = OP_EBADPACKET,
        Error_BadLink = OP_EBADLINK,
        Error_BadTimestamp = OP_EBADTIMESTAMP,
    };

#ifndef LSOUND_USE_WAVE
    //-------------------------------------------
    //---
    //--- Stream
    //---
    //-------------------------------------------
    class Stream
    {
    public:
        void close();
        virtual bool open() =0;

        /**
        @return サンプル数
        */
        inline s32 read(opus_int16* pcm, s32 size);
        /**
        @return サンプル数
        */
        inline s32 read_stereo(opus_int16* pcm, s32 size);

        /**
        @return サンプル数
        */
        inline s32 read_float(f32* pcm, s32 size);
        /**
        @return サンプル数
        */
        inline s32 read_float_stereo(f32* pcm, s32 size);

        inline opus_int64 tell();
        inline s32 seek(opus_int64 offset);

        inline opus_int64 getTotal() const;
        inline opus_int64 getPosition() const;
        inline LSuint getSampleRate() const;
        inline LSint getBitsPerSample() const;
        inline LSenum getFormat() const;
        inline LSenum getChannels() const;
        inline LSenum getType() const;
    protected:
        friend class Context;

        Stream(const Stream&);
        Stream& operator=(const Stream&);

        Stream();
        virtual ~Stream();

        void setInfo();
        void swap(Stream& rhs);

        OggOpusFile* opusFile_;
        opus_int64 total_; //number of samples at 48kHz
        opus_int64 position_;
        LSenum format_;
        LSenum channels_;
    };

    inline s32 Stream::read(opus_int16* pcm, s32 size)
    {
        LASSERT(NULL != opusFile_);
        s32 ret = op_read(opusFile_, pcm, size, NULL);
        if(0<ret){
            position_ += ret;
        }
        return ret;
    }

    inline s32 Stream::read_stereo(opus_int16* pcm, s32 size)
    {
        LASSERT(NULL != opusFile_);
        s32 ret = op_read_stereo(opusFile_, pcm, size);
        if(0<ret){
            position_ += ret;
        }
        return ret;
    }

    inline s32 Stream::read_float(f32* pcm, s32 size)
    {
        LASSERT(NULL != opusFile_);
        s32 ret = op_read_float(opusFile_, pcm, size, NULL);
        if(0<ret){
            position_ += ret;
        }
        return ret;
    }

    inline s32 Stream::read_float_stereo(f32* pcm, s32 size)
    {
        LASSERT(NULL != opusFile_);
        s32 ret = op_read_float_stereo(opusFile_, pcm, size);
        if(0<ret){
            position_ += ret;
        }
        return ret;
    }

    inline opus_int64 Stream::tell()
    {
        LASSERT(NULL != opusFile_);
        return op_pcm_tell(opusFile_);
    }

    inline s32 Stream::seek(opus_int64 offset)
    {
        LASSERT(NULL != opusFile_);
        s32 ret = op_pcm_seek(opusFile_, offset);
        if(0==ret){
            position_ = offset;
        }
        return ret;
    }

    inline opus_int64 Stream::getTotal() const
    {
        return total_;
    }

    inline opus_int64 Stream::getPosition() const
    {
        return position_;
    }

    inline LSuint Stream::getSampleRate() const
    {
        return SampleRate_48000;
    }

    inline LSint Stream::getBitsPerSample() const
    {
        return BitsPerSample;
    }

    inline LSenum Stream::getFormat() const
    {
        return format_;
    }

    inline LSenum Stream::getChannels() const
    {
        return channels_;
    }

    inline LSenum Stream::getType() const
    {
        return PCMFormat_Short;
    }

    //-------------------------------------------
    //---
    //--- FileStream
    //---
    //-------------------------------------------
    class FileStream : public Stream
    {
    public:
        FileStream();
        virtual ~FileStream();

        void set(File* file, opus_int64 start, opus_int64 end);
        virtual bool open();

        void swap(FileStream& rhs);
    private:
        static int read_func(void* stream, unsigned char* ptr, int nbytes);
        static int seek_func(void* stream, opus_int64 offset, int whence);
        static opus_int64 tell_func(void* stream);
        static int close_func(void* stream);

        FileStream(const Stream&);
        FileStream& operator=(const FileStream&);

        static OpusFileCallbacks callbacks_;

        opus_int64 start_;
        opus_int64 end_;
        opus_int64 current_;
        File* file_;
    };

    //-------------------------------------------
    //---
    //--- MemoryStream
    //---
    //-------------------------------------------
    class MemoryStream : public Stream
    {
    public:
        MemoryStream();
        virtual ~MemoryStream();

        void set(u32 size, u32 offset, Memory* memory);
        virtual bool open();

        void swap(MemoryStream& rhs);
    private:
        MemoryStream(const MemoryStream&);
        MemoryStream& operator=(const MemoryStream&);

        u32 size_;
        u32 offset_;
        Memory* memory_;
    };

#ifdef ANDROID
    //-------------------------------------------
    //---
    //--- AssetStream
    //---
    //-------------------------------------------
    class AssetStream : public Stream
    {
    public:
        AssetStream();
        virtual ~AssetStream();

        void set(Asset* asset, opus_int64 start, opus_int64 end);
        virtual bool open();

        void swap(AssetStream& rhs);
    private:
        AssetStream(const AssetStream&);
        AssetStream& operator=(const AssetStream&);

        static int read_func(void* stream, unsigned char* ptr, int nbytes);
        static int seek_func(void* stream, opus_int64 offset, int whence);
        static opus_int64 tell_func(void* stream);
        static int close_func(void* stream);

        static OpusFileCallbacks callbacks_;

        opus_int64 start_;
        opus_int64 end_;
        opus_int64 current_;
        Asset* asset_;
    };
#endif

#else //LSOUND_USE_WAVE


    static const u32 WaveID_RIFF='FFIR';
    static const u32 WaveID_WAVE='EVAW';
    static const u32 WaveID_fmt=' tmf';
    static const u32 WaveID_fact='tcaf';
    static const u32 WaveID_data='atad';
    static const u32 WaveID_LIST='TSIL';

    struct WaveHeader
    {
        u32 id_;
        u32 size_;
        u32 tag_;
    };

    struct Chunk
    {
        u32 id_;
        u32 size_;
    };

    struct ChunkFmt
    {
        u32 id_;
        u32 size_;
        u16 format_;
        u16 channels_;
        u32 samplesPerSec_;
        u32 bytesPerSec_;
        u16 blockSize_;
        u16 bitsPerSample_;
    };

    class Wave
    {
    public:
        struct Memory
        {
            Memory()
                :position_(0)
                ,memory_(NULL)
            {}
            Memory(u8* memory)
                :position_(0)
                ,memory_(memory)
            {}
            s32 tell() const;
            void advance(s32 offset);
            s32 read(void* dst, u32 size);

            s32 position_;
            u8* memory_;
        };

        Wave();
        ~Wave();

        bool read(FILE* file);
        Chunk readChunkHeader(FILE* file);
        void readThroughChunk(FILE* file, const Chunk& chunk);
        bool readFmt(FILE* file, const Chunk& chunk);
        bool readData(FILE* file, const Chunk& chunk);

        bool read(u8* mem);
        Chunk readChunkHeader(Memory& memory);
        void readThroughChunk(Memory& memory, const Chunk& chunk);
        bool readFmt(Memory& memory, const Chunk& chunk);
        bool readData(Memory& memory, const Chunk& chunk);

#ifdef ANDROID
        bool read(Asset* asset);
        Chunk readChunkHeader(Asset* asset);
        void readThroughChunk(Asset* asset, const Chunk& chunk);
        bool readFmt(Asset* asset, const Chunk& chunk);
        bool readData(Asset* asset, const Chunk& chunk);
#endif
        u16 format_;
        u16 channels_;
        u32 samplesPerSec_;
        u32 bytesPerSec_;
        u16 blockSize_;
        u16 bitsPerSample_;

        s64 dataOffset_;
        u32 dataSize_;
    };


    //-------------------------------------------
    //---
    //--- Stream
    //---
    //-------------------------------------------
    class Stream
    {
    public:
        void close();
        virtual bool open() =0;

        /**
        @return サンプル数
        */
        virtual s32 read(opus_int16* pcm, s32 size) =0;
        /**
        @return サンプル数
        */
        virtual s32 read_stereo(opus_int16* pcm, s32 size) =0;

        /**
        @return サンプル数
        */
        virtual s32 read_float(f32* pcm, s32 size) =0;
        /**
        @return サンプル数
        */
        virtual s32 read_float_stereo(f32* pcm, s32 size) =0;

        virtual opus_int64 tell() =0;
        virtual s32 seek(opus_int64 offset) =0;

        inline opus_int64 getTotal() const;
        inline opus_int64 getPosition() const;
        inline LSuint getSampleRate() const;
        inline LSint getBitsPerSample() const;
        inline LSenum getFormat() const;
        inline LSenum getChannels() const;
        inline LSenum getType() const;
    protected:
        friend class Context;

        Stream(const Stream&);
        Stream& operator=(const Stream&);

        Stream();
        virtual ~Stream();

        void setInfo();
        void swap(Stream& rhs);

        Wave wave_;
        opus_int64 total_; //number of samples at 48kHz
        opus_int64 position_;
        s32 format_;
    };

    inline opus_int64 Stream::getTotal() const
    {
        return total_;
    }

    inline opus_int64 Stream::getPosition() const
    {
        return position_;
    }

    inline LSuint Stream::getSampleRate() const
    {
        return wave_.samplesPerSec_;
    }

    inline LSint Stream::getBitsPerSample() const
    {
        return wave_.bitsPerSample_;
    }

    inline LSenum Stream::getFormat() const
    {
        return format_;
    }

    inline LSenum Stream::getChannels() const
    {
        return wave_.channels_;
    }

    inline LSenum Stream::getType() const
    {
        return PCMFormat_Short;
    }

    //-------------------------------------------
    //---
    //--- FileStream
    //---
    //-------------------------------------------
    class FileStream : public Stream
    {
    public:
        FileStream();
        virtual ~FileStream();

        void set(File* file, opus_int64 start, opus_int64 end);
        virtual bool open();

        virtual s32 read(opus_int16* pcm, s32 numSamples);
        virtual s32 read_stereo(opus_int16* pcm, s32 numSamples);
        virtual s32 read_float(f32* pcm, s32 numSamples);
        virtual s32 read_float_stereo(f32* pcm, s32 numSamples);

        virtual opus_int64 tell();
        virtual s32 seek(opus_int64 offset);

        void swap(FileStream& rhs);
    private:

        FileStream(const Stream&);
        FileStream& operator=(const FileStream&);

        opus_int64 start_;
        opus_int64 end_;
        opus_int64 current_;
        File* file_;
    };

    //-------------------------------------------
    //---
    //--- MemoryStream
    //---
    //-------------------------------------------
    class MemoryStream : public Stream
    {
    public:
        MemoryStream();
        virtual ~MemoryStream();

        void set(u32 size, u32 offset, Memory* memory);
        virtual bool open();

        virtual s32 read(opus_int16* pcm, s32 numSamples);
        virtual s32 read_stereo(opus_int16* pcm, s32 numSamples);
        virtual s32 read_float(f32* pcm, s32 numSamples);
        virtual s32 read_float_stereo(f32* pcm, s32 numSamples);

        virtual opus_int64 tell();
        virtual s32 seek(opus_int64 offset);

        void swap(MemoryStream& rhs);
    private:
        MemoryStream(const MemoryStream&);
        MemoryStream& operator=(const MemoryStream&);

        s32 memread(void* dst, s32 size) const;

        u32 size_;
        u32 offset_;
        Memory* memory_;
    };

#ifdef ANDROID
    //-------------------------------------------
    //---
    //--- AssetStream
    //---
    //-------------------------------------------
    class AssetStream : public Stream
    {
    public:
        AssetStream();
        virtual ~AssetStream();

        void set(Asset* asset, opus_int64 start, opus_int64 end);
        virtual bool open();

        virtual s32 read(opus_int16* pcm, s32 numSamples);
        virtual s32 read_stereo(opus_int16* pcm, s32 numSamples);
        virtual s32 read_float(f32* pcm, s32 numSamples);
        virtual s32 read_float_stereo(f32* pcm, s32 numSamples);

        virtual opus_int64 tell();
        virtual s32 seek(opus_int64 offset);

        void swap(AssetStream& rhs);
    private:
        AssetStream(const AssetStream&);
        AssetStream& operator=(const AssetStream&);

        opus_int64 start_;
        opus_int64 end_;
        opus_int64 current_;
        Asset* asset_;
    };
#endif
#endif //LSOUND_USE_WAVE
}
#endif //INC_LSOUND_STREAM_H__
