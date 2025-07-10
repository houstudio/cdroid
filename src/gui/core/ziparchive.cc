/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <ziparchive.h>
#include <iostreams.h>
#ifndef _Nullable
#define _Nullable
#define _Nonnull
#endif
#include <zip.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(_WIN32)||defined(_WIN64)
  #include <fcntl.h>
  #include <io.h>
#elif defined(__linux__)||defined(__unix__)
  #include <unistd.h>
#endif
#include <dirent.h>
#include <chrono>
#include <cdlog.h>
using namespace std::chrono;

namespace cdroid{

ZIPArchive::ZIPArchive(const std::string&fname){
     int flags=0?(ZIP_CREATE|ZIP_TRUNCATE):(ZIP_CHECKCONS|ZIP_RDONLY);
     zip=zip_open(fname.c_str(),flags,nullptr);
     method=ZIP_CM_DEFAULT;
}

ZIPArchive::~ZIPArchive(){
    zip_close((zip_t*)zip);
}

int ZIPArchive::getEntries(std::vector<std::string>&entries)const{
    const zip_int64_t num = zip_get_num_entries((zip_t*)zip,ZIP_FL_UNCHANGED);
    for(int i=0;i<num;i++){
        const char*name=zip_get_name((zip_t*)zip,i,0);
        entries.push_back(name);
    }
    return static_cast<int>(num);
}

int ZIPArchive::forEachEntry(std::function<bool(const std::string&)>func)const{
    int count=0;
    if(func){
        const zip_int64_t num = zip_get_num_entries((zip_t*)zip,ZIP_FL_NODIR);
        for(int i=0;i<num;i++){
            const char*name=zip_get_name((zip_t*)zip,i,0);
            count+=(func(name)!=false);
        }
    }
    return count;

}

bool ZIPArchive::hasEntry(const std::string&name,bool excludeDirectories)const{
    int flags = ZIP_FL_ENC_UTF_8;//DEFAULLT_ENC_FLAG;
    if (excludeDirectories)flags = flags | ZIP_FL_NODIR;
    zip_int64_t index=zip_name_locate((zip_t*)zip,name.c_str(),flags);
    return index>=0;
}

std::istream* ZIPArchive::getInputStream(const std::string&fname)const{
    zip_file_t*zfile = zip_fopen((zip_t*)zip,fname.c_str(),ZIP_RDONLY);//ZIP_FL_ENC_UTF_8
    LOGV("zfile=%p [%s",zfile,fname.c_str());
    if(zfile==nullptr)return nullptr;
    return new ZipInputStream(zfile);
}

void*ZIPArchive::getZipHandle(const std::string&fname)const{
    return zip_fopen((zip_t*)zip,fname.c_str(),ZIP_RDONLY);
}

void ZIPArchive::remove(const std::string&fname)const{
    zip_int64_t idx=zip_name_locate((zip_t*)zip,fname.c_str(),0);
    zip_delete((zip_t*)zip,idx);
}

}

