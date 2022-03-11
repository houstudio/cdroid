#define _Nullable
#define _Nonnull
#include "iostreams.h"
#include <zip.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <cstdio>


namespace cdroid {

ZipStreamBuf::ZipStreamBuf()
    : _select(false)
    , zipfile(NULL)
    , buffer(NULL) {
}

ZipStreamBuf::~ZipStreamBuf() {
    close();
}

ZipStreamBuf* ZipStreamBuf::select(void* zfile) {
    if (is_open()||zfile==NULL) 
        return NULL;
    this->zipfile = zfile;
    // allocate buffer
    buffer = new char_type[BUFSIZ];
    setg(buffer, buffer, buffer);
    return this;
}

ZipStreamBuf* ZipStreamBuf::close() {
    if (!is_open())
        return NULL;

    if (buffer) {
        delete[] buffer;
        buffer = NULL;
        setg(0, 0, 0);
    }

    if ( zip_fclose((zip_file_t*)zipfile) != ZIP_ER_OK)
        return NULL;
    zipfile = NULL;

    return this;
}

std::streambuf::int_type ZipStreamBuf::underflow() {//Unbuffered get
    if (!is_open())
        return traits_type::eof();

    if (gptr() < egptr()) {
        return traits_type::to_int_type(*gptr());
    }

    int n = zip_fread((zip_file_t*)zipfile, buffer, BUFFER_SIZE);
    if (n <= 0) {
        setg(buffer,buffer,buffer);
        return traits_type::eof();
    }
    setg(buffer,buffer,buffer + n);

    return gptr()==egptr()?traits_type::eof():traits_type::to_int_type(*gptr());
}

std::streambuf::pos_type  ZipStreamBuf::seekoff(std::streambuf::off_type off, std::ios_base::seekdir way,
    std::ios_base::openmode mode/*ios_base::in | ios_base::out*/){
    int whence=SEEK_SET;
    switch(way){
    case std::ios_base::beg: whence=SEEK_SET; break;
    case std::ios_base::cur: whence=SEEK_CUR; break;
    case std::ios_base::end: whence=SEEK_END; break; 
    }
    int rc=zip_fseek((zip_file_t*)zipfile,off,whence);//only worked for uncompressed data
    off=zip_ftell((zip_file_t*)zipfile);
    setg(buffer,buffer,buffer);
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
    case std::ios_base::beg: buffpos=off; break;
    case std::ios_base::cur: buffpos+=off; break;
    case std::ios_base::end: buffpos=buffersize-off; break;
    }
    setg(pbase()+buffpos,pbase()+buffpos,pbase()+buffersize);
    return buffpos;
}

ZipInputStream::ZipInputStream(void*zfile) : std::istream(NULL) {
    init(&_sb);
    select(zfile);
}

void ZipInputStream::select(void*zfile) {
    if (!_sb.select(zfile)) {
        setstate(std::ios_base::failbit);
    } else {
        clear();
    }
}

void ZipInputStream::close() {
    if (!_sb.close()) {
        setstate(std::ios_base::failbit);
    }
}

}//namespace
