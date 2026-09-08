/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
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
#include <content/asset.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <cstdio>
#include <iostream>

namespace cdroid {

MemoryBuf::MemoryBuf(char const* base, size_t size){
    char* p(const_cast<char*>(base));
    setg(p, p, p + size);
    setp(p,p+size);
    buffersize=size;
    buffpos=0;
}

std::streambuf::pos_type  MemoryBuf::seekoff(std::streambuf::off_type off, std::ios_base::seekdir way,
    std::ios_base::openmode mode){
    switch(way){
    case std::ios_base::beg: buffpos = off; break;
    case std::ios_base::cur: buffpos +=off; break;
    case std::ios_base::end: buffpos = buffersize - off; break;
    }
    setg(pbase() + buffpos,pbase() + buffpos,pbase() + buffersize);
    return buffpos;
}

AssetInputStream::AssetInputStream(Asset* asset)
    : MemoryBuf((char const*)asset->getBuffer(false), (size_t)asset->getLength())
    , std::istream(static_cast<std::streambuf*>(this))
    , mAsset(asset) {
    if (asset->getBuffer(false) == nullptr) {
        setstate(std::ios_base::failbit);
    } else {
        clear();
    }
}

AssetInputStream::~AssetInputStream() {
    delete mAsset;
}

}//namespace
