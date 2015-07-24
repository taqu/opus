/**
@file liostream.cpp
@author t-sakai
@date 2010/07/26 create
*/
#include <stdarg.h>
#include <stdio.h>
#include "lcore.h"
#include "liostream.h"

namespace lcore
{
//#if defined(ANDROID)
#if 1

    int ios::ModeInt[Mode_Num] =
    {
        ios::in,
        ios::out,
        ios::app,
        (ios::in | ios::binary),
        (ios::out | ios::binary),
        (ios::app | ios::out | ios::binary),
        (ios::in | ios::out),
        (ios::in | ios::out| ios::trunc),
        (ios::in | ios::out| ios::app),
        (ios::in | ios::out| ios::binary),
        (ios::in | ios::out| ios::trunc | ios::binary),
        (ios::in | ios::out| ios::app | ios::binary),
    };

    const Char* ios::ModeString[] =
    {
        "r",
        "w",
        "a",
        "rb",
        "wb",
        "ab",
        "r+",
        "w+",
        "a+",
        "rb+",
        "wb+",
        "ab+",
    };


    const Char* ios::getModeString(int mode)
    {
        for(int i=0; i<Mode_Num; ++i){
            if(mode == ModeInt[i]){
                return ModeString[i];
            }
        }
        return NULL;
    }

#ifdef _WIN32
    u32 ios::getDesiredAccess(s32 mode)
    {
        u32 access = 0;
        if(mode & ios::in){
            access |= GENERIC_READ;
        }
        if(mode & ios::out){
            access |= GENERIC_WRITE;
        }
        return access;
    }

    u32 ios::getCreationDisposition(s32 mode)
    {
        u32 disposition = CREATE_ALWAYS;
        if(mode == ModeInt[Mode_R]){
            disposition = OPEN_EXISTING;

        }else if(mode == ModeInt[Mode_W]){
            disposition = CREATE_ALWAYS;

        }else if(mode == ModeInt[Mode_A]){
            disposition = OPEN_ALWAYS;

        }else if(mode == ModeInt[Mode_RP]){
            disposition = CREATE_ALWAYS;

        }else if(mode == ModeInt[Mode_WP]){
            disposition = CREATE_ALWAYS;

        }else if(mode == ModeInt[Mode_AP]){
            disposition = OPEN_ALWAYS;
        }
        return disposition;
    }
#endif

    //----------------------------------------------------------
    //---
    //--- ifstream
    //---
    //----------------------------------------------------------
#if defined(_WIN32) || defined(_WIN64)
    s32 ifstream::get()
    {
        Char c = 0;
        ReadFile(file_, &c, 1, NULL, NULL);
        return c;
    }

#else
    s32 ifstream::get()
    {
        return fgetc(file_);
    }
#endif

    //----------------------------------------------------------
    //---
    //--- ofstream
    //---
    //----------------------------------------------------------
#if defined(_WIN32) || defined(_WIN64)
    int ofstream::print(const Char* format, ... )
    {
        LASSERT(NULL != file_);
        LASSERT(NULL != format);

        va_list ap;
        va_start(ap, format);

        s32 size = _vscprintf(format, ap) + 1;
        Char* buffer = (Char*)LIME_MALLOC(size*sizeof(Char));
        vsprintf_s(buffer, size, format, ap);
        DWORD written = 0;
        s32 ret = WriteFile(file_, buffer, size-1, &written, NULL);
        LIME_FREE(buffer);

        va_end(ap);
        return ret;
    }
#else
    int ofstream::print(const Char* format, ... )
    {
        LASSERT(NULL != file_);
        LASSERT(NULL != format);

        va_list ap;
        va_start(ap, format);

        int ret = vfprintf(file_, format, ap);

        va_end(ap);
        return ret;
    }
#endif

    //----------------------------------------------------------
    //---
    //--- sstream_base<istream>
    //---
    //----------------------------------------------------------
    sstream_base<istream>::sstream_base()
        :buffer_(NULL)
        ,current_(0)
        ,capacity_(0)
    {
    }

    sstream_base<istream>::sstream_base(const Char* buffer, u32 size)
        :buffer_(buffer)
        ,current_(0)
        ,capacity_(size)
    {
    }

    bool sstream_base<istream>::eof()
    {
        return (current_>=capacity_);
    }

    bool sstream_base<istream>::good()
    {
        if(buffer_ == NULL){
            return false;
        }
        return (current_<=capacity_);
    }

    bool sstream_base<istream>::seekg(s32 offset, int dir)
    {
        switch(dir)
        {
        case ios::beg:
            if(capacity_<=offset){
                current_ = capacity_;
            }else{
                current_ += offset;
            }
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

    s32 sstream_base<istream>::tellg()
    {
        return current_;
    }

    lsize_t sstream_base<istream>::read(void* dst, u32 count)
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
#endif
}
