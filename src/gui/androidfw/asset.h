// Port of AOSP frameworks/base/libs/androidfw/include/androidfw/Asset.h.
//
// A faithful C++14 port of the read-only Asset abstraction. Public class &
// method signatures match AOSP. Adaptations (all internal plumbing, allowed
// under "内部隐藏类可适当裁剪"):
//   - IncFsFileMap / map_ptr / getIncFsBuffer() and the createFrom*Map overloads
//     taking IncFsFileMap are removed (CDROID has no Incremental FS). The zip
//     entry path uses buffer-backed factories instead.
//   - android::String8 -> std::string, std::optional<IncFsFileMap> -> a buffer
//     pointer + ownership flag.
//   - StreamingZipInflater is not ported; compressed data is inflated eagerly
//     into a heap buffer with zlib (same observable read/seek/getBuffer behavior).
//
// Lives in namespace cdroid (AOSP-faithful) to isolate it from the unrelated
// cdroid::Assets (core/assets.h). The shared status_t / error codes are brought
// in from the existing cdroid:: resourcetypes port via using-declarations.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
#ifndef __CDROID_ANDROIDFW_ASSET_H__
#define __CDROID_ANDROIDFW_ASSET_H__

#include <sys/types.h>
#include <memory>
#include <string>

#include "resourcetypes.h"   // cdroid::status_t + error codes

namespace cdroid {

// Shared AOSP-style status codes live in cdroid:: (see resourcetypes.h). Bring
// them into android:: so the Asset code reads like the AOSP original.
using cdroid::status_t;
using cdroid::NO_ERROR;
using cdroid::UNKNOWN_ERROR;
using cdroid::BAD_INDEX;
using cdroid::BAD_VALUE;

// Instances of this class provide read-only operations on a byte stream. Asset
// objects are NOT thread-safe, and should not be shared across threads.
class Asset {
public:
    virtual ~Asset();
    Asset(const Asset& src) = delete;
    Asset& operator=(const Asset& src) = delete;

    static int32_t getGlobalCount();
    static std::string getAssetAllocations();

    // Used when opening an asset.
    typedef enum AccessMode {
        ACCESS_UNKNOWN = 0,
        ACCESS_RANDOM,      // read chunks, seek forward and backward
        ACCESS_STREAMING,   // read sequentially, occasional forward seek
        ACCESS_BUFFER,      // caller plans to ask for a read-only buffer of all data
    } AccessMode;

    // Read data from the current offset. Returns bytes read, 0 on EOF, -1 error.
    virtual ssize_t read(void* buf, size_t count) = 0;

    // Seek: whence uses lseek/fseek values. Returns new position or (off64_t)-1.
    virtual off64_t seek(off64_t offset, int whence) = 0;

    // Close, freeing all associated resources.
    virtual void close() = 0;

    // Pointer to a buffer with the entire contents (aligned to 4 bytes when
    // `aligned` is true).
    virtual const void* getBuffer(bool aligned) = 0;

    // Total amount of data that can be read.
    virtual off64_t getLength() const = 0;

    // Total amount of data that can be read from the current position.
    virtual off64_t getRemainingLength() const = 0;

    // Open a new fd for reading this asset, or -1 if not possible (compressed).
    virtual int openFileDescriptor(off64_t* outStart, off64_t* outLength) const = 0;

    // Whether the buffer is allocated in RAM (not mmapped).
    virtual bool isAllocated() const { return false; }

    // Debug-only source identifier (path or zip:name). Do not parse.
    const char* getAssetSource() const { return mAssetSource.c_str(); }

    // Create the asset from a file descriptor (public factory, AOSP-faithful).
    static Asset* createFromFd(int fd, const char* fileName, AccessMode mode);

protected:
    static void registerAsset(Asset* asset);
    static void unregisterAsset(Asset* asset);

    Asset();

    // Common seek() housekeeping. Returns the new chunk offset, or -1 if illegal.
    off64_t handleSeek(off64_t offset, int whence, off64_t curPosn, off64_t maxPosn);

    void setAssetSource(const std::string& path) { mAssetSource = path; }
    AccessMode getAccessMode() const { return mAccessMode; }

private:
    // AssetManager needs the create-from-* factories.
    friend class AssetManager;

    static Asset* createFromFile(const char* fileName, AccessMode mode);
    static Asset* createFromCompressedFile(const char* fileName, AccessMode mode);

    // Internal: wrap an uncompressed byte buffer (e.g. a stored zip entry, or an
    // entry libzip already decompressed). When `owned` the buffer is freed on
    // close()/destruction via delete[] (so it must have been new[]-allocated).
    static std::unique_ptr<Asset> createFromUncompressedBuffer(const void* data, size_t length,
                                                               AccessMode mode, bool owned);

    AccessMode   mAccessMode = ACCESS_UNKNOWN;
    std::string  mAssetSource;
    Asset*       mNext = nullptr;   // global Asset linked list (debug accounting)
    Asset*       mPrev = nullptr;
};


// An asset backed by an uncompressed file (FILE*) or an in-memory buffer.
// Replaces AOSP _FileAsset's IncFsFileMap mode with a plain buffer mode.
class _FileAsset : public Asset {
public:
    _FileAsset();
    ~_FileAsset() override;

    // FILE*-backed: takes ownership of fd (via fdopen); [offset, offset+length).
    status_t openChunk(const char* fileName, int fd, off64_t offset, size_t length);
    // Buffer-backed: [data, data+length); freed on close() iff owned.
    status_t openChunk(const void* data, size_t length, bool owned);

    ssize_t read(void* buf, size_t count) override;
    off64_t seek(off64_t offset, int whence) override;
    void close() override;
    const void* getBuffer(bool aligned) override;
    off64_t getLength() const override { return mLength; }
    off64_t getRemainingLength() const override { return mLength - mOffset; }
    int openFileDescriptor(off64_t* outStart, off64_t* outLength) const override;
    bool isAllocated() const override { return mBuf != nullptr; }

private:
    off64_t               mStart = 0;        // absolute file offset of chunk start
    off64_t               mLength = 0;
    off64_t               mOffset = 0;       // current local offset
    FILE*                 mFp = nullptr;     // FILE-backed read/seek
    char*                 mFileName = nullptr; // for re-opening an fd
    const unsigned char*  mData = nullptr;   // buffer-backed source
    bool                  mDataOwned = false;
    unsigned char*        mBuf = nullptr;    // getBuffer() result for FILE mode
    enum { kReadVsMapThreshold = 4096 };
};


// An asset backed by compressed (gzip) data, inflated lazily into a heap buffer.
class _CompressedAsset : public Asset {
public:
    _CompressedAsset();
    ~_CompressedAsset() override;

    // Gzip data inside an open file: [offset, offset+compressedLen), inflating to
    // uncompressedLen bytes. Takes ownership of fd.
    status_t openChunk(int fd, off64_t offset, size_t uncompressedLen, size_t compressedLen);
    // Gzip data in a buffer; freed on close() iff owned.
    status_t openChunk(const void* data, size_t compressedLen, size_t uncompressedLen, bool owned);

    ssize_t read(void* buf, size_t count) override;
    off64_t seek(off64_t offset, int whence) override;
    void close() override;
    const void* getBuffer(bool aligned) override;
    off64_t getLength() const override { return mUncompressedLen; }
    off64_t getRemainingLength() const override { return mUncompressedLen - mOffset; }
    int openFileDescriptor(off64_t* /*outStart*/, off64_t* /*outLength*/) const override { return -1; }
    bool isAllocated() const override { return mBuf != nullptr; }

private:
    off64_t               mStart = 0;
    off64_t               mCompressedLen = 0;
    off64_t               mUncompressedLen = 0;
    off64_t               mOffset = 0;
    int                   mFd = -1;
    const unsigned char*  mData = nullptr;   // compressed buffer source
    bool                  mDataOwned = false;
    unsigned char*        mBuf = nullptr;    // inflated buffer
};

} // namespace cdroid
#endif // __CDROID_ANDROIDFW_ASSET_H__
