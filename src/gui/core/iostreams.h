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
#ifndef __ZIP_STREAM_H__
#define __ZIP_STREAM_H__
#include <istream>
namespace cdroid{

class Asset;   // content/asset.h

class MemoryBuf: public std::streambuf {
private:
    std::streambuf::pos_type buffpos,buffersize;
public:
    MemoryBuf(char const* base, size_t size);
    std::streambuf::pos_type  seekoff(std::streambuf::off_type off, std::ios_base::seekdir way,
        std::ios_base::openmode mode/*ios_base::in | std::ios_base::out*/)override;
};

struct MemoryInputStream: virtual MemoryBuf, std::istream {
    MemoryInputStream(char const* base, size_t size)
        : MemoryBuf(base, size)
        , std::istream(static_cast<std::streambuf*>(this)) {
    }
};

// Owning istream over a buffer-backed Asset (Context::openAsset's face):
// deletes the Asset (and with it the buffer) on destruction. Replaces the
// former ZipInputStream (istream-over-zip_file_t glue).
class AssetInputStream: virtual public MemoryBuf, public std::istream {
private:
    Asset* mAsset;
public:
    AssetInputStream(Asset* asset);
    ~AssetInputStream()override;
};

}
#endif
