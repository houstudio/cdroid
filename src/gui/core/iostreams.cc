#define _Nullable
#define _Nonnull
#include "iostreams.h"
#include <zip.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <cstdio>


namespace cdroid {

ZipStreamBuf::ZipStreamBuf()
    : zipfile(nullptr)
    , bufferPosition(0){
    buffer = new char_type[BUFSIZ];
}

ZipStreamBuf::~ZipStreamBuf() {
    delete []buffer;
    if(zipfile){
        zip_fclose((zip_file_t*)zipfile);
        zipfile = nullptr;
    }
}

ZipStreamBuf* ZipStreamBuf::select(void* zfile) {
    if (zfile == nullptr) return nullptr;
    zipfile = zfile;
    // allocate buffer
    setg(buffer, buffer, buffer);
    return this;
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

    return gptr()==egptr()?traits_type::eof():traits_type::to_int_type(*gptr());
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
            size = zip_fseek((zip_file_t*)zipfile, 0, SEEK_END);
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

ZipInputStream::ZipInputStream(void*zfile) : std::istream(nullptr) {
    init(&_sb);
    if(!_sb.select(zfile))setstate(std::ios_base::failbit);
    else clear();
}

}//namespace
