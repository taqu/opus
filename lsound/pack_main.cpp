#include <cassert>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>
#include <iostream>
#include <lcore/liostream.h>

#include "opus/Pack.h"
#include "opus/PackWriter.h"

namespace lsound
{
    bool isFile(const WIN32_FIND_DATA& data)
    {
        static const DWORD checks[] =
        {
            FILE_ATTRIBUTE_HIDDEN,
            FILE_ATTRIBUTE_SYSTEM,
            FILE_ATTRIBUTE_DIRECTORY,
            FILE_ATTRIBUTE_ENCRYPTED,
            FILE_ATTRIBUTE_TEMPORARY,
            FILE_ATTRIBUTE_SPARSE_FILE,
        };
        static const s32 Num = sizeof(checks)/sizeof(DWORD);
        for(int i=0; i<Num; ++i){
            DWORD check = checks[i] & data.dwFileAttributes;
            if(check != 0){
                return false;
            }
        }
        return true;
    }
}

int main(int argc, char** argv)
{
    //static char* debug[] = {"cmd", "./tmp/", "out.pak", "-list", "list.txt"};
    //argc = 5;
    //argv = debug;

    if(argc<3){
        std::cerr << argv[0] << " directory name <-list filename>" << std::endl;
        return 0;
    }

    int inIndex = -1;
    int outIndex = -1;
    int listIndex = -1;
    for(int i=1; i<argc; ++i){
        if(strcmp(argv[i], "-list") == 0){
            int j = i+1;
            if(j<argc){
                listIndex = j;
                ++i;
            }
        }else if(inIndex<0){
            inIndex = i;
        }else{
            outIndex = i;
        }
    }
    if(inIndex<0 || outIndex<0){
        std::cerr << "invalid args" << std::endl;
        return 0;
    }

    {
        std::string path = argv[inIndex];
        lsound::u32 len = path.size();
        if('\\' != path[len-1]
        && '/' != path[len-1])
        {
            path += '\\';
        }
        std::string inpath = path;

        path += "*";

        WIN32_FIND_DATA findData;
        HANDLE handle = FindFirstFile(path.c_str(), &findData);
        if(INVALID_HANDLE_VALUE == handle){
            return 0;
        }

        std::string filepath;
        lsound::PackWriter writer;
        lsound::u32 size = 0;
        lcore::ifstream in;
        lsound::s8* buffer = NULL;

        if(listIndex>=0){
            writer.openListFile(argv[listIndex]);
        }
        for(;;){
            if(lsound::isFile(findData)){
                std::cerr << findData.cFileName << std::endl;
                filepath = inpath;
                filepath.append(findData.cFileName);
                in.open(filepath.c_str(), lcore::ios::binary);
                if(in.is_open()){
                    in.seekg(0, lcore::ios::end);
                    size = in.tellg();
                    in.seekg(0, lcore::ios::beg);
                    buffer = new lsound::s8[size];
                    in.read(buffer, size);
                    in.close();

                    writer.push_back(findData.cFileName, size, buffer);

                    delete[] buffer;
                }
                
            }
            if(FALSE == FindNextFile(handle, &findData)){
                break;
            }
        }
        FindClose(handle);

        writer.write(argv[outIndex]);
    }

    return 0;
}
