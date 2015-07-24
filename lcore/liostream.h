#ifndef INC_LCORE_LIOSTREAM_
#define INC_LCORE_LIOSTREAM_
/**
@file lfstream
@author t-sakai
@date 2010/05/25 create
*/
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#endif

#include "lcore.h"

namespace lcore
{
    typedef int OpenMode;
    
    class ios
    {
    public:
        enum OpenMode
        {
            OpenMode_ForceByte = 0xFFU,
        };
        static const int in = 0x01;
	    static const int out = 0x02;
	    static const int ate = 0x04;
	    static const int app = 0x08;
	    static const int trunc = 0x10;
	    static const int binary = 0x20;
	    
	    enum SeekDir
	    {
	        SeekDir_ForceByte = 0xFFU,
	    };

#if defined(_WIN32) || defined(_WIN64)
        static const int beg = FILE_BEGIN;
	    static const int cur = FILE_CURRENT;
	    static const int end = FILE_END;
#else
	    static const int beg = SEEK_SET;
	    static const int cur = SEEK_CUR;
	    static const int end = SEEK_END;
#endif	    
	    
	    enum Mode
        {
            Mode_R = 0,
            Mode_W,
            Mode_A,
            Mode_RB,
            Mode_WB,
            Mode_AB,
            Mode_RP,
            Mode_WP,
            Mode_AP,
            Mode_RBP,
            Mode_WBP,
            Mode_ABP,
            Mode_Num,
        };
        
        static int ModeInt[Mode_Num];
        
        static const Char* ModeString[Mode_Num];
        
        static const Char* getModeString(int mode);

#if defined(_WIN32) || defined(_WIN64)
        static u32 getDesiredAccess(s32 mode);
        static u32 getCreationDisposition(s32 mode);
#endif
    };
    
    //----------------------------------------------------------
    //---
    //--- istream
    //---
    //----------------------------------------------------------
    class istream
    {
    public:
        virtual bool eof() =0;
        virtual bool good() =0;
        
        virtual bool seekg(s32 offset, int dir) =0;
        virtual s32 tellg() =0;
        
        virtual lsize_t read(void* dst, u32 count) =0;
        
        virtual u32 getSize() =0;
        virtual u32 getSize(s32 afterOffset) =0;

    protected:
        istream(){}
        ~istream(){}

        istream(const istream&);
        istream& operator=(const istream&);

        template<class T> friend istream& operator>>(istream& strm, T& t);
    };
    
    template<class T>
    inline istream& operator>>(istream& strm, T& t)
    {
        strm.read((Char*)&t, sizeof(T));
        return strm;
    }
    
    //----------------------------------------------------------
    //---
    //--- ostream
    //---
    //----------------------------------------------------------
    class ostream
    {
    public:
        virtual bool eof() =0;
        virtual bool good() =0;
        
        virtual bool seekg(s32 offset, int dir) =0;
        virtual s32 tellg() =0;
        
        virtual lsize_t write(const void* src, u32 count) =0;

        virtual u32 getSize() =0;
        virtual u32 getSize(s32 afterOffset) =0;
    protected:
        ostream(){}
        ~ostream(){}
        ostream(const ostream&);
        ostream& operator=(const ostream&);
    };
    
    
    //----------------------------------------------------------
    //---
    //--- fstream_base
    //---
    //----------------------------------------------------------
#if defined(_WIN32) || defined(_WIN64)
    template<class Base>
    class fstream_base : public Base
    {
    public:
        bool is_open() const{ return (file_ != NULL);}
        void close();
        
        virtual bool eof();
        virtual bool good();
        
        virtual bool seekg(s32 offset, int dir);
        virtual s32 tellg();
        
        virtual u32 getSize();
        virtual u32 getSize(s32 afterOffset);

        void swap(fstream_base& rhs)
        {
            lcore::swap(file_, rhs.file_);
        }

        inline bool flush();

        u64 getID();
    protected:
        fstream_base();
        fstream_base(const Char* filepath, int mode);
        ~fstream_base();

        bool open(const Char* filepath, int mode);
        
        virtual lsize_t read(void* dst, u32 count);
        virtual lsize_t write(const void* src, u32 count);
        
        HANDLE file_;
    };
    
    template<class Base>
    fstream_base<Base>::fstream_base()
        :file_(NULL)
    {
    }

    template<class Base>
    fstream_base<Base>::fstream_base(const Char* filepath, int mode)
        :file_(NULL)
    {
        open(filepath, mode);
    }

    template<class Base>
    fstream_base<Base>::~fstream_base()
    {
        close();
    }

    template<class Base>
    bool fstream_base<Base>::open(const Char* filepath, int mode)
    {
        LASSERT(NULL != filepath);
        close();
        mode &= ~ios::binary;

        HANDLE file = CreateFile(
            filepath,
            ios::getDesiredAccess(mode),
            FILE_SHARE_READ,
            NULL,
            ios::getCreationDisposition(mode),
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        if(INVALID_HANDLE_VALUE != file){
            file_ = file;
            return true;
        }else{
            return false;
        }
    }

    template<class Base>
    void fstream_base<Base>::close()
    {
        if(file_){
            CloseHandle(file_);
            file_ = NULL;
        }
    }

    template<class Base>
    bool fstream_base<Base>::eof()
    {
        LASSERT(NULL != file_);
        u32 pos = SetFilePointer(file_, 0, NULL, FILE_CURRENT);
        u32 size = GetFileSize(file_, NULL);
        return (size<=pos);
    }

    template<class Base>
    bool fstream_base<Base>::good()
    {
        if(NULL == file_){
            return false;
        }
        return true;
    }

    template<class Base>
    inline bool fstream_base<Base>::seekg(s32 offset, int dir)
    {
        LASSERT(NULL != file_);
        LASSERT(0<=ios::beg && dir<=ios::end);
        return (INVALID_SET_FILE_POINTER != SetFilePointer(file_, offset, NULL, dir));

    }

    template<class Base>
    inline s32 fstream_base<Base>::tellg()
    {
        LASSERT(NULL != file_);
        return SetFilePointer(file_, 0, NULL, FILE_CURRENT);
    }

    template<class Base>
    u32 fstream_base<Base>::getSize()
    {
        LASSERT(NULL != file_);
        return GetFileSize(file_, NULL);
    }

    template<class Base>
    u32 fstream_base<Base>::getSize(s32 afterOffset)
    {
        LASSERT(NULL != file_);
        u32 size = GetFileSize(file_, NULL);
        SetFilePointer(file_, afterOffset, NULL, FILE_BEGIN);
        return size;
    }

    template<class Base>
    inline lsize_t fstream_base<Base>::read(void* dst, u32 count)
    {
        LASSERT(NULL != file_);
        DWORD numBytesRead;
        return ReadFile(file_, dst, count, &numBytesRead, NULL);
    }

    template<class Base>
    inline lsize_t fstream_base<Base>::write(const void* src, u32 count)
    {
        LASSERT(NULL != file_);
        DWORD numBytesWritten;
        return WriteFile(file_, src, count, &numBytesWritten, NULL);
    }

    template<class Base>
    inline bool fstream_base<Base>::flush()
    {
        LASSERT(NULL != file_);
        return TRUE == FlushFileBuffers(file_);
    }

    template<class Base>
    u64 fstream_base<Base>::getID()
    {
        LASSERT(NULL != file_);
        BY_HANDLE_FILE_INFORMATION fileInfo;
        if(GetFileInformationByHandle(file_, &fileInfo)){
            u64 ret = fileInfo.nFileIndexHigh;
            return (ret<<32) | fileInfo.nFileIndexLow;
        }else{
            return 0;
        }
    }

#else
    template<class Base>
    class fstream_base : public Base
    {
    public:
        bool is_open() const{ return (file_ != NULL);}
        void close();
        
        virtual bool eof();
        virtual bool good();
        
        virtual bool seekg(s32 offset, int dir);
        virtual s32 tellg();
        
        virtual u32 getSize()
        {
            seekg(0, lcore::ios::end);
            u32 size = tellg();
            seekg(0, lcore::ios::beg);
            return size;
        }

        virtual u32 getSize(s32 afterOffset)
        {
            seekg(0, lcore::ios::end);
            u32 size = tellg();
            seekg(afterOffset, lcore::ios::beg);
            return size;
        }

        void swap(fstream_base& rhs)
        {
            lcore::swap(file_, rhs.file_);
        }

        inline bool flush();

        u64 getID();
    protected:
        fstream_base();
        fstream_base(const Char* filepath, int mode);
        ~fstream_base();

        bool open(const Char* filepath, int mode);
        
        virtual lsize_t read(void* dst, u32 count);
        virtual lsize_t write(const void* src, u32 count);
        
        FILE* file_;
    };
    
    template<class Base>
    fstream_base<Base>::fstream_base()
        :file_(NULL)
    {
    }

    template<class Base>
    fstream_base<Base>::fstream_base(const Char* filepath, int mode)
        :file_(NULL)
    {
        open(filepath, mode);
    }

    template<class Base>
    fstream_base<Base>::~fstream_base()
    {
        close();
    }

    template<class Base>
    bool fstream_base<Base>::open(const Char* filepath, int mode)
    {
        LASSERT(filepath != NULL);
        close();
        const Char* modeStr = ios::getModeString(mode);
        LASSERT(modeStr != NULL);

        file_ = fopen(filepath, modeStr);
        return (file_ != NULL);
    }

    template<class Base>
    void fstream_base<Base>::close()
    {
        if(file_){
            fclose(file_);
            file_ = NULL;
        }
    }

    template<class Base>
    bool fstream_base<Base>::eof()
    {
        LASSERT(NULL != file_);
        return (0 == ferror(file_)) && (0 != feof(file_));
    }

    template<class Base>
    bool fstream_base<Base>::good()
    {
        if(NULL == file_){
            return false;
        }
        return (0 == ferror(file_));
    }

    template<class Base>
    inline bool fstream_base<Base>::seekg(s32 offset, int dir)
    {
        LASSERT(NULL != file_);
        LASSERT(0<=ios::beg && dir<=ios::end);
        return (0 == fseek(file_, offset, dir));

    }

    template<class Base>
    inline s32 fstream_base<Base>::tellg()
    {
        LASSERT(NULL != file_);
        return ftell(file_);
    }

    template<class Base>
    inline lsize_t fstream_base<Base>::read(void* dst, u32 count)
    {
        LASSERT(NULL != file_);
        return fread(dst, count, 1, file_);
    }

    template<class Base>
    inline lsize_t fstream_base<Base>::write(const void* src, u32 count)
    {
        return fwrite(src, count, 1, file_);
    }

    template<class Base>
    inline bool fstream_base<Base>::flush()
    {
        LASSERT(NULL != file_);
        return 0 == fflush(file_);
    }

    template<class Base>
    u64 fstream_base<Base>::getID()
    {
        s32 fd = fileno(file_);
        struct stat fileStat;
        if(fstat(fd, &fileStat)<0){
            return 0;
        }
        return fileStat.st_ino;
    }
#endif

    //----------------------------------------------------------
    //---
    //--- ifstream
    //---
    //----------------------------------------------------------
    class ifstream : public fstream_base<istream>
    {
    public:
        typedef fstream_base<istream> super_type;
        
        ifstream(){}
        ifstream(const Char* filepath, int mode=ios::in)
            :super_type(filepath, mode|ios::in)
        {
        }
        
        bool open(const Char* filepath, int mode=ios::in)
        {
            return super_type::open(filepath, mode|ios::in);
        }
        
        virtual lsize_t read(void* dst, u32 count){ return super_type::read(dst, count);}

        s32 get();
    private:
        ifstream(const ifstream&);
        ifstream& operator=(const ifstream&);
    };

    //----------------------------------------------------------
    //---
    //--- ofstream
    //---
    //----------------------------------------------------------
    class ofstream : public fstream_base<ostream>
    {
    public:
        typedef fstream_base<ostream> super_type;
    
        ofstream(){}
        ofstream(const Char* filepath, int mode=ios::out)
            :super_type(filepath, mode|ios::out)
        {
        }
        
        bool open(const Char* filepath, int mode=ios::out)
        {
            return super_type::open(filepath, mode|ios::out);
        }
        
        virtual lsize_t write(const void* src, u32 count){ return super_type::write(src, count);}

        int print(const Char* format, ... );
    private:
        ofstream(const ofstream&);
        ofstream& operator=(const ofstream&);
    };
    
    
    
    //----------------------------------------------------------
    //---
    //--- sstream_base
    //---
    //----------------------------------------------------------
    template<class Base>
    class sstream_base : public Base
    {
    public:
        virtual bool eof();
        virtual bool good();
        
        virtual bool seekg(s32 offset, int dir);
        virtual s32 tellg();
        virtual u32 getSize();
        virtual u32 getSize(s32 afterOffset);
    protected:
        sstream_base();
        sstream_base(Char* buffer, u32 size);
        ~sstream_base(){}
        
        virtual lsize_t read(void* dst, u32 count);
        virtual lsize_t write(const void* src, u32 count);

        void expand();
        
        Char* buffer_;
        s32 current_;
        s32 capacity_;
    };
    
    template<class Base>
    sstream_base<Base>::sstream_base()
        :buffer_(NULL)
        ,current_(0)
        ,capacity_(0)
    {
    }

    template<class Base>
    sstream_base<Base>::sstream_base(Char* buffer, u32 size)
        :buffer_(buffer)
        ,current_(0)
        ,capacity_(size)
    {
    }

    template<class Base>
    bool sstream_base<Base>::eof()
    {
        return (current_>=capacity_);
    }

    template<class Base>
    bool sstream_base<Base>::good()
    {
        if(buffer_ == NULL){
            return false;
        }
        return (current_<=capacity_);
    }

    template<class Base>
    bool sstream_base<Base>::seekg(s32 offset, int dir)
    {
        switch(dir)
        {
        case ios::beg:
            current_ = (capacity_<=offset)? capacity_ : offset;
            break;
            
        case ios::cur:
            if(offset>=0){
                if((current_+offset)>=capacity_){
                    current_ = capacity_;
                    break;
                }
            }else{
                if((current_-offset)<0){
                    current_ = 0;
                    break;
                }
            }
            current_ += offset;
            break;
                        
        case ios::end:
            if(capacity_ <= offset){
                current_ = 0;
            }else{
                current_ = capacity_ - offset;
            }
            break;
            
        default:
            break;
        };
        
        return true;
    }

    template<class Base>
    s32 sstream_base<Base>::tellg()
    {
        return current_;
    }

    template<class Base>
    u32 sstream_base<Base>::getSize()
    {
        return capacity_;
    }

    template<class Base>
    u32 sstream_base<Base>::getSize(s32 afterOffset)
    {
        current_ = (capacity_<=afterOffset)? capacity_ : afterOffset;
        return capacity_;
    }

    template<class Base>
    lsize_t sstream_base<Base>::read(void* dst, u32 count)
    {
        s32 end = current_ + count;
        end = (end>capacity_)? capacity_ : end;
        count = end - current_;

        Char* d = reinterpret_cast<Char*>(dst);
        while(current_<end){
            *d = buffer_[current_];
            ++d;
            ++current_;
        }
        return count;
    }

    template<class Base>
    lsize_t sstream_base<Base>::write(const void* src, u32 count)
    {
        s32 end = current_ + count;
        if(end > capacity_){
            expand();
        }
        
        const Char* s = reinterpret_cast<const Char*>(src);
        while(current_<end){
            buffer_[current_] = *s;
            ++s;
            ++current_;
        }
        return count;
    }
    
    template<class Base>
    void sstream_base<Base>::expand()
    {
        s32 newSize = capacity_ << 1;
        //printf("size %d => %d\n", capacity_, newSize);
        
        Char* newBuffer = LIME_NEW Char[newSize];
        
        if(buffer_ != NULL){
            for(s32 i=0; i<capacity_; ++i){
                newBuffer[i] = buffer_[i];
            }
        }
        LIME_DELETE_ARRAY(buffer_);
        
        buffer_ = newBuffer;
        capacity_ = newSize;
    }

    //----------------------------------------------------------
    //---
    //--- sstream_base<istream>
    //---
    //----------------------------------------------------------
    template<>
    class sstream_base<istream> : public istream
    {
    public:
        virtual bool eof();
        virtual bool good();
        
        virtual bool seekg(s32 offset, int dir);
        virtual s32 tellg();
        
    protected:
        sstream_base();
        sstream_base(const Char* buffer, u32 size);
        ~sstream_base(){}
        
        virtual lsize_t read(void* dst, u32 count);
        virtual lsize_t write(const void* /*src*/, u32 /*count*/){return 0;}; //何もしない
        
        const Char* buffer_;
        s32 current_;
        s32 capacity_;
    };

    
    //----------------------------------------------------------
    //---
    //--- isstream
    //---
    //----------------------------------------------------------
    class isstream : public sstream_base<istream>
    {
    public:
        typedef sstream_base<istream> super_type;
        
        isstream(){}
        isstream(const Char* buffer, u32 size)
            :super_type(buffer, size)
        {
        }

        ~isstream()
        {
            buffer_ = NULL;
            current_ = 0;
            capacity_ = 0;
        }
        
        virtual lsize_t read(void* dst, u32 count){ return super_type::read(dst, count);}
    private:
        isstream(const isstream&);
        isstream& operator=(const isstream&);
    };
    
    
    //----------------------------------------------------------
    //---
    //--- osstream
    //---
    //----------------------------------------------------------
    class osstream : public sstream_base<ostream>
    {
    public:
        typedef sstream_base<ostream> super_type;
    
        osstream(){}
        osstream(u32 size)
        {
            LASSERT(size>0);
            buffer_ = LIME_NEW Char[size];
            current_ = 0;
            capacity_ = size;
        }
        
        ~osstream()
        {
            LIME_DELETE_ARRAY(buffer_);
            current_ = 0;
            capacity_ = 0;
        }
        
        virtual lsize_t write(const void* src, u32 count){ return super_type::write(src, count);}
        
        const Char* c_str() const{ return buffer_;}
    private:
        osstream(const osstream&);
        osstream& operator=(const osstream&);
    };

    //----------------------------------------------------------
    //---
    //--- range_stream_base
    //---
    //----------------------------------------------------------
    template<class Base>
    class range_stream_base : public Base
    {
    public:
        range_stream_base(Base* stream, s32 begin, s32 end)
            :stream_(stream)
            ,begin_(begin)
            ,end_(end)
            ,pos_(begin)
        {
            LASSERT(NULL != stream_);
            LASSERT(0<=begin_);
            LASSERT(begin_<=end_);
            LASSERT(check());
        }

        ~range_stream_base()
        {}

        virtual bool eof()
        {
            return (end_<=pos_);
        }

        virtual bool good()
        {
            return stream_->good();
        }
        
        virtual bool seekg(s32 offset, int dir);

        virtual s32 tellg()
        {
            return pos_;
        }

    protected:
        bool check()
        {
            s32 cur = stream_->tellg();
            stream_->seekg(0, lcore::ios::end);
            s32 end = stream_->tellg();
            stream_->seekg(cur, lcore::ios::beg);

            return (end_<=end);
        }

        Base* stream_;
        s32 begin_;
        s32 end_;
        s32 pos_;
    };

    template<class Base>
    bool range_stream_base<Base>::seekg(s32 offset, int dir)
    {
        switch(dir)
        {
        case ios::beg:
            if(stream_->seekg(offset+begin_, ios::beg)){
                pos_ = offset;
            }else{
                return false;
            }
            break;

        case ios::cur:
            if(stream_->seekg(offset+pos_+begin_, ios::beg)){
                pos_ += offset;
            }else{
                return false;
            }
            break;

        case ios::end:
            if(stream_->seekg(offset+end_, ios::beg)){
                pos_ = end_ - begin_ - offset;
            }else{
                return false;
            }
            break;

        default:
            return false;
        }
        return true;
    }

    //----------------------------------------------------------
    //---
    //--- range_istream
    //---
    //----------------------------------------------------------
    class range_istream : public range_stream_base<istream>
    {
    public:
        typedef range_stream_base<istream> parent_type;

        range_istream(istream* stream, s32 begin, s32 end)
            :parent_type(stream, begin, end)
        {
        }

        ~range_istream()
        {}

        virtual lsize_t read(void* dst, u32 count)
        {
            if(!stream_->seekg(begin_+pos_, lcore::ios::beg)){
                return 0;
            }
            lsize_t size = stream_->read(dst, count);

            pos_ += size;
            return size;
        }
    };

    //----------------------------------------------------------
    //---
    //--- range_ostream
    //---
    //----------------------------------------------------------
    class range_ostream : public range_stream_base<ostream>
    {
    public:
        typedef range_stream_base<ostream> parent_type;

        range_ostream(ostream* stream, s32 begin, s32 end)
            :parent_type(stream, begin, end)
        {
        }

        ~range_ostream()
        {}

        virtual lsize_t write(const void* src, u32 count)
        {
            if(!stream_->seekg(begin_+pos_, lcore::ios::beg)){
                return 0;
            }
            lsize_t size = stream_->write(src, count);

            pos_ += size;
            return size;
        }
    };

namespace io
{
    template<class T>
    inline lsize_t write(lcore::ostream& of, const T& value)
    {
        return of.write(reinterpret_cast<const void*>(&value), sizeof(T));
    }

    template<class T>
    inline lsize_t write(lcore::ostream& of, const T* value, u32 size)
    {
        return of.write(reinterpret_cast<const void*>(value), size);
    }

    template<class T>
    inline lsize_t read(lcore::istream& in, T& value)
    {
        return in.read(reinterpret_cast<void*>(&value), sizeof(T));
    }

    template<class T>
    inline lsize_t read(lcore::istream& in, T* value, u32 size)
    {
        return in.read(reinterpret_cast<void*>(value), size);
    }
}
}

#endif //INC_LCORE_LIOSTREAM_
