/**
@file PackWriter.cpp
@author t-sakai
@date 2012/03/21 create

*/

#include "PackWriter.h"
#include <lcore/clibrary.h>

namespace lsound
{
    //-------------------------------------------------
    PackWriter::PackWriter()
    {
        header_.reserved0_ = 0;
        header_.reserved1_ = 0;
        header_.numFiles_ = 0;
    }

    //-------------------------------------------------
    PackWriter::~PackWriter()
    {
        listStream_.close();
        for(MemPtrArray::iterator itr = mempts_.begin();
            itr != mempts_.end();
            ++itr)
        {
            LIME_DELETE_ARRAY(*itr);
        }
    }

    //-------------------------------------------------
    bool PackWriter::push_back(const Char* name, u32 size, const void* buffer)
    {
        LASSERT(NULL != name);

        FileEntry entry;

        entry.size_ = size;

        entries_.push_back(entry);
        
        u8* tmp = LIME_NEW u8[size];
        lcore::memcpy(tmp, buffer, size);

        mempts_.push_back(tmp);

        if(listStream_.is_open()){
            listStream_.print("%s\r\n", name);
        }

        return true;
    }

    //-------------------------------------------------
    bool PackWriter::write(const Char* path)
    {
        if(false == open(path)){
            return false;
        }

        header_.numFiles_ = entries_.size();

        lcore::lsize_t ret = lcore::io::write(stream_, header_);
        if(0 == ret){
            return false;
        }

        s32 offset = 0;//stream_.tellg() + sizeof(FileEntry) * header_.numFiles_;

        for(FileEntryArray::iterator itr = entries_.begin();
            itr != entries_.end();
            ++itr)
        {
            (*itr).offset_ = offset;
            offset += (*itr).size_;

            ret = lcore::io::write(stream_, (*itr));
            if(0 == ret){
                return false;
            }
        }

        FileEntryArray::iterator entryItr = entries_.begin();
        for(MemPtrArray::iterator itr = mempts_.begin();
            itr != mempts_.end();
            ++itr)
        {
            ret = lcore::io::write( stream_, (*itr), (*entryItr).size_ );
            if(0 == ret){
                return false;
            }
            ++entryItr;
        }

        close();
        return true;
    }

}
