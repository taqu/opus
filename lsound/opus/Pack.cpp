/**
@file Pack.cpp
@author t-sakai
@date 2013/02/11 create
*/
#include "Pack.h"

namespace lsound
{
    //------------------------------------------
    //---
    //--- Pack
    //---
    //------------------------------------------
    Pack::Pack()
        :entries_(NULL)
        ,dataTopOffset_(0)
        ,data_(NULL)
    {
    }

    Pack::~Pack()
    {
        releaseData();
        releaseEntries();
    }

    // ƒtƒ@ƒCƒ‹”æ“¾
    u32 Pack::getNumFiles() const
    {
        return header_.numFiles_;
    }

    u32 Pack::getSize(u32 index) const
    {
        LASSERT(0<=index && index<header_.numFiles_);
        return entries_[index].size_;
    }

    const u8* Pack::getData(u32 index) const
    {
        LASSERT(0<=index && index<header_.numFiles_);
        return data_ + entries_[index].offset_;
    }

    s32 Pack::getFileOffset(u32 index) const
    {
        LASSERT(0<=index && index<header_.numFiles_);
        return dataTopOffset_ + entries_[index].offset_;
    }

    void Pack::swap(Pack& rhs)
    {
        lcore::swap(header_, rhs.header_);
        lcore::swap(entries_, rhs.entries_);
        lcore::swap(dataTopOffset_, rhs.dataTopOffset_);
        lcore::swap(data_, rhs.data_);
    }

    void Pack::releaseEntries()
    {
        LIME_DELETE_ARRAY(entries_);
    }

    void Pack::releaseData()
    {
        LIME_DELETE_ARRAY(data_);
    }
}
