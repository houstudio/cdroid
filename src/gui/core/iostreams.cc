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
#define _Nullable
#define _Nonnull
#include "iostreams.h"
#include <zip.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <cstdio>
#include <iostream>

namespace cdroid {

ZipStreamBuf::ZipStreamBuf(void*zf)
    : zipfile(zf)
    , bufferPosition(0){
    buffer = new char_type[BUFSIZ];
    setg(buffer, buffer, buffer);
}

ZipStreamBuf::~ZipStreamBuf() {
    delete []buffer;
    if(zipfile){
        zip_fclose((zip_file_t*)zipfile);
        zipfile = nullptr;
    }
}

std::streambuf::int_type ZipStreamBuf::underflow() {//Unbuffered get
    if (zipfile==nullptr)
        return traits_type::eof();

    if (gptr() < egptr()) {
        return traits_type::to_int_type(*gptr());
    }

    zip_int64_t n = zip_fread((zip_file_t*)zipfile, buffer, BUFFER_SIZE);
    if (n <= 0) {
        setg(buffer,buffer,buffer);
        return traits_type::eof();
    }
    setg(buffer,buffer,buffer + n);

    return traits_type::to_int_type(*gptr());
}

std::streambuf::pos_type  ZipStreamBuf::seekoff(std::streambuf::off_type off, std::ios_base::seekdir way,
    std::ios_base::openmode mode/*ios_base::in | ios_base::out*/){
    zip_int64_t currentPos,newPos,size;
    if(mode&std::ios_base::in){
        switch(way){
        case std::ios_base::beg:
            zip_fseek((zip_file_t*)zipfile, off, SEEK_SET);
            // Reset buffer
            setg(buffer, buffer, buffer);
            bufferPosition = off;
            break;
        case std::ios_base::cur:
            currentPos = zip_ftell((zip_file_t*)zipfile);
            // Calculate new position after offset
            newPos = currentPos + off;

            // Check if newPos is still within the current buffer
            if (newPos >= bufferPosition && newPos < bufferPosition + BUFFER_SIZE) {
                // Update buffer pointers accordingly
                setg(buffer, gptr() + off, buffer + BUFFER_SIZE);
            } else {
                // If newPos is outside of current buffer, seek to newPos
                zip_fseek((zip_file_t*)zipfile, newPos, SEEK_SET);
                // Reset buffer
                setg(buffer, buffer, buffer);
                bufferPosition = newPos;
            }
            break;
        case std::ios_base::end:
            zip_fseek((zip_file_t*)zipfile, 0, SEEK_END);
            size = zip_ftell((zip_file_t*)zipfile);
            zip_fseek((zip_file_t*)zipfile, size + off, SEEK_SET);
            // Reset buffer
            setg(buffer, buffer, buffer);
            bufferPosition = size + off;
            break;
        }
    }else if (mode & std::ios_base::out) {
        LOGE("Output operations not supported, so do nothing");
    }
    off=zip_ftell((zip_file_t*)zipfile);
    return off;
}

std::streambuf::pos_type  ZipStreamBuf::seekpos(std::streambuf::pos_type pos,
    std::ios_base::openmode mode/*ios_base::in | ios_base::out*/){
    return seekoff(pos,std::ios_base::beg,mode);
}

MemoryBuf::MemoryBuf(char const* base, size_t size){
    char* p(const_cast<char*>(base));
    setg(p, p, p + size);
    setp(p,p+size);
    buffersize=size;
    buffpos=0;
}

std::streambuf::pos_type  MemoryBuf::seekoff(std::streambuf::off_type off, std::ios_base::seekdir way,
    std::ios_base::openmode mode/*ios_base::in | ios_base::out*/){
    switch(way){
    case std::ios_base::beg: buffpos = off; break;
    case std::ios_base::cur: buffpos +=off; break;
    case std::ios_base::end: buffpos = buffersize - off; break;
    }
    setg(pbase() + buffpos,pbase() + buffpos,pbase() + buffersize);
    return buffpos;
}

ZipInputStream::ZipInputStream(void*zipfile):std::istream(new ZipStreamBuf(zipfile)){
    init((ZipStreamBuf*)rdbuf());
    if(zipfile==nullptr){
        setstate(std::ios_base::failbit);
    }else{
        clear();
    }
}

}//namespace
