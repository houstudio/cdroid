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
#ifndef __ZIP_STREAM_H__
#define __ZIP_STREAM_H__
#include <istream>
#include <stdio.h>/*BUFSIZ*/
namespace cdroid{

class ZipStreamBuf : public std::streambuf {
public:
  ZipStreamBuf(void*zipfile);
  virtual ~ZipStreamBuf()override;

  virtual pos_type  seekoff(off_type,std::ios_base::seekdir,std::ios_base::openmode /*__mode*/ = std::ios_base::in | std::ios_base::out)override;
  virtual pos_type  seekpos(pos_type,std::ios_base::openmode /*__mode*/ = std::ios_base::in | std::ios_base::out)override;
protected:
   int_type underflow()override;
private:
  static const unsigned BUFFER_SIZE = BUFSIZ;
  ZipStreamBuf(const ZipStreamBuf&);
  ZipStreamBuf& operator=(const ZipStreamBuf&);
  std::ios_base::openmode io_mode;
  void* zipfile;
  char_type* buffer;
  int64_t bufferPosition;
};

class ZipInputStream : public std::istream {
public:
  ZipInputStream(void*zfile);
  ~ZipInputStream()override{delete rdbuf();}
};

class MemoryBuf: public std::streambuf {
private:
    std::streambuf::pos_type buffpos,buffersize;
public:
    MemoryBuf(char const* base, size_t size);
    std::streambuf::pos_type  seekoff(std::streambuf::off_type off, std::ios_base::seekdir way,
        std::ios_base::openmode mode/*ios_base::in | ios_base::out*/)override;
};

struct MemoryInputStream: virtual MemoryBuf, std::istream {
    MemoryInputStream(char const* base, size_t size)
        : MemoryBuf(base, size)
        , std::istream(static_cast<std::streambuf*>(this)) {
    }
};

}
#endif
