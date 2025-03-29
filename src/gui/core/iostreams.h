#ifndef __ZIP_STREAM_H__
#define __ZIP_STREAM_H__
#include <istream>
#include <stdio.h>/*BUFSIZ*/
namespace cdroid{

class ZipStreamBuf : public std::streambuf {
public:
  ZipStreamBuf(void*zipfile);
  virtual ~ZipStreamBuf();

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
private:
  ZipStreamBuf* _sb;
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
