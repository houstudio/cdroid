// Port of AOSP androidfw/Chunk.h (frameworks/base/libs/androidfw/include/
// androidfw/Chunk.h), namespace cdroid.
//
// Helpful wrapper around a ResChunk_header that provides getter methods
// that handle endianness conversions and provide access to the data portion
// of the chunk.
//
// Copyright (C) 2016 The Android Open Source Project
// Licensed under the Apache License, Version 2.0.
#ifndef __CDROID_ANDROIDFW_CHUNK_H__
#define __CDROID_ANDROIDFW_CHUNK_H__

#include <string>

#include <porting/cdlog.h>
#include <content/androidfw/mapptr.h>
#include <content/androidfw/resourcetypes.h>

namespace cdroid {

// Helpful wrapper around a ResChunk_header that provides getter methods
// that handle endianness conversions and provide access to the data portion
// of the chunk.
class Chunk {
public:
    explicit Chunk(incfs::verified_map_ptr<ResChunk_header> chunk) : device_chunk_(chunk) {}

    // Returns the type of the chunk. Caller need not worry about endianness.
    inline int type() const { return dtohs(device_chunk_->type); }

    // Returns the size of the entire chunk. This can be useful for skipping
    // over the entire chunk. Caller need not worry about endianness.
    inline size_t size() const { return dtohl(device_chunk_->size); }

    // Returns the size of the header. Caller need not worry about endianness.
    inline size_t header_size() const { return dtohs(device_chunk_->headerSize); }

    template <typename T, size_t MinSize = sizeof(T)>
    inline incfs::map_ptr<T> header() const {
        return (header_size() >= MinSize) ? device_chunk_.convert<T>() : nullptr;
    }

    inline incfs::map_ptr<void> data_ptr() const {
        return device_chunk_.offset(header_size());
    }

    inline size_t data_size() const { return size() - header_size(); }

private:
    const incfs::verified_map_ptr<ResChunk_header> device_chunk_;
};

// Provides a Java style iterator over an array of ResChunk_header's.
// Validation is performed while iterating.
// The caller should check if there was an error during chunk validation
// by calling HadError() and GetLastError() to get the reason for failure.
// Example:
//
//   ChunkIterator iter(data_ptr, data_len);
//   while (iter.HasNext()) {
//     const Chunk chunk = iter.Next();
//     ...
//   }
//
//   if (iter.HadError()) {
//     LOG(ERROR) << iter.GetLastError();
//   }
//
class ChunkIterator {
public:
    ChunkIterator(incfs::map_ptr<void> data, size_t len)
        : next_chunk_(data.convert<ResChunk_header>()),
          len_(len),
          last_error_(nullptr) {
        FATAL_IF(!(bool) next_chunk_, "data can't be null");
        if (len_ != 0) {
            VerifyNextChunk();
        }
    }

    Chunk Next();
    inline bool HasNext() const { return !HadError() && len_ != 0; }
    // Returns whether there was an error and processing should stop
    inline bool HadError() const { return last_error_ != nullptr; }
    inline std::string GetLastError() const { return last_error_; }
    // Returns whether there was an error and processing should stop. For legacy purposes,
    // some errors are considered "non fatal". Fatal errors stop processing new chunks and
    // throw away any chunks already processed. Non fatal errors also stop processing new
    // chunks, but, will retain and use any valid chunks already processed.
    inline bool HadFatalError() const { return HadError() && last_error_was_fatal_; }

private:
    ChunkIterator(const ChunkIterator&) = delete;
    ChunkIterator& operator=(const ChunkIterator&) = delete;

    // Returns false if there was an error.
    bool VerifyNextChunk();
    // Returns false if there was an error. For legacy purposes.
    bool VerifyNextChunkNonFatal();

    incfs::map_ptr<ResChunk_header> next_chunk_;
    size_t len_;
    const char* last_error_;
    bool last_error_was_fatal_ = true;
};

}  // namespace cdroid

#endif  // __CDROID_ANDROIDFW_CHUNK_H__
