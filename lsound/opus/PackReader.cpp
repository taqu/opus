/**
@file PackReader.cpp
@author t-sakai
@date 2013/02/12 create
*/
#include "PackReader.h"

namespace lsound
{

    //------------------------------------------
    //---
    //--- PackReader
    //---
    //------------------------------------------
    PackReader::PackReader()
    {
    }

    PackReader::~PackReader()
    {
        close();
    }

    // ファイルオープン
    bool PackReader::open(const Char* path)
    {
        LASSERT(NULL != path);
        bool isOpen = stream_.open(path, lcore::ios::in | lcore::ios::binary);
        if(isOpen){
            lcore::lsize_t size;
            dataSize_ = 0;

            size = lcore::io::read(stream_, pack_.header_);
            if(size<1){
                return false;
            }

            pack_.releaseEntries();
            pack_.entries_ = LIME_NEW FileEntry[pack_.header_.numFiles_];

            u32 entrySize = pack_.header_.numFiles_ * sizeof(FileEntry);
            size = lcore::io::read(stream_, pack_.entries_, entrySize);
            if(size<1){
                return false;
            }

            //データサイズ計算
            s32 dataTop = stream_.tellg();
            stream_.seekg(0, lcore::ios::end);
            dataSize_ = stream_.tellg() - dataTop;
            stream_.seekg(dataTop, lcore::ios::beg);
            pack_.dataTopOffset_ = dataTop;
        }
        return isOpen;
    }

    // ファイルクローズ
    void PackReader::close()
    {
        stream_.close();
    }

    bool PackReader::readEntries(Pack& pack)
    {
        LASSERT(stream_.is_open());
        pack_.swap(pack);
        return true;
    }

    bool PackReader::readAll(Pack& pack)
    {
        LASSERT(stream_.is_open());

        pack_.releaseData();
        pack_.data_ = LIME_NEW u8[dataSize_];
        u32 size = lcore::io::read(stream_, pack_.data_, dataSize_);
        if(size < 1){
            return false;
        }
        pack_.swap(pack);
        return true;
    }

}
