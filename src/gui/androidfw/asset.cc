// Port of AOSP frameworks/base/libs/androidfw/Asset.cpp.
//
// Provide access to read-only assets. Logic translated verbatim from the AOSP
// original; adaptations (IncFs -> heap buffers, StreamingZipInflater -> zlib,
// String8 -> std::string) are called out inline. No locking: CDROID resource
// access is single-threaded on the UI thread (AOSP's gAssetLock dropped).

#define LOG_TAG "asset"

#include "asset.h"

#include <porting/cdlog.h>

#include <zlib.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef O_BINARY
# define O_BINARY 0
#endif

using namespace cdroid;

static const bool kIsDebug = false;

// Global debug/accounting list (faithful to AOSP; lock dropped).
static int32_t gCount = 0;
static Asset* gHead = nullptr;
static Asset* gTail = nullptr;

void Asset::registerAsset(Asset* asset) {
    gCount++;
    asset->mNext = asset->mPrev = nullptr;
    if (gTail == nullptr) {
        gHead = gTail = asset;
    } else {
        asset->mPrev = gTail;
        gTail->mNext = asset;
        gTail = asset;
    }
    if (kIsDebug) LOGI("Creating Asset %p #%d", asset, gCount);
}

void Asset::unregisterAsset(Asset* asset) {
    gCount--;
    if (gHead == asset) gHead = asset->mNext;
    if (gTail == asset) gTail = asset->mPrev;
    if (asset->mNext != nullptr) asset->mNext->mPrev = asset->mPrev;
    if (asset->mPrev != nullptr) asset->mPrev->mNext = asset->mNext;
    asset->mNext = asset->mPrev = nullptr;
    if (kIsDebug) LOGI("Destroying Asset in %p #%d", asset, gCount);
}

int32_t Asset::getGlobalCount() {
    return gCount;
}

std::string Asset::getAssetAllocations() {
    std::string res;
    for (Asset* cur = gHead; cur != nullptr; cur = cur->mNext) {
        if (cur->isAllocated()) {
            off64_t size = (cur->getLength() + 512) / 1024;
            char buf[64];
            snprintf(buf, sizeof(buf), ": %lldK\n", (long long)size);
            res += "    ";
            res += cur->getAssetSource();
            res += buf;
        }
    }
    return res;
}

Asset::Asset() : mAccessMode(ACCESS_UNKNOWN), mNext(nullptr), mPrev(nullptr) {
}

// ---------------------------------------------------------------------------
// Static factories
// ---------------------------------------------------------------------------

// static
Asset* Asset::createFromFile(const char* fileName, AccessMode mode) {
    return createFromFd(open(fileName, O_RDONLY | O_BINARY), fileName, mode);
}

// static
Asset* Asset::createFromFd(int fd, const char* fileName, AccessMode mode) {
    if (fd < 0) return nullptr;

    off64_t length = lseek64(fd, 0, SEEK_END);
    if (length < 0) {
        ::close(fd);
        return nullptr;
    }
    (void)lseek64(fd, 0, SEEK_SET);

    _FileAsset* pAsset = new _FileAsset;
    status_t result = pAsset->openChunk(fileName, fd, 0, length);
    if (result != NO_ERROR) {
        delete pAsset;
        return nullptr;
    }
    pAsset->mAccessMode = mode;
    return pAsset;
}

// static. Opens a gzip (.gz) file. Faithful to AOSP createFromCompressedFile;
// the gzip header probe (AOSP ZipUtils::examineGzip) is replaced by a direct
// gzip-magic + ISIZE-trailer check, then _CompressedAsset gzip-inflates lazily.
Asset* Asset::createFromCompressedFile(const char* fileName, AccessMode mode) {
    int fd = open(fileName, O_RDONLY | O_BINARY);
    if (fd < 0) return nullptr;

    off64_t fileLen = lseek(fd, 0, SEEK_END);
    if (fileLen < 18) {                 // min gzip: 10-byte header + 8-byte trailer
        ::close(fd);
        return nullptr;
    }

    // gzip magic + ISIZE (uncompressed size, last 4 bytes, little-endian).
    unsigned char hdr[2];
    unsigned char trailer[4];
    (void)lseek(fd, 0, SEEK_SET);
    if (::read(fd, hdr, 2) != 2 || hdr[0] != 0x1f || hdr[1] != 0x8b) {
        LOGD("File '%s' is not in gzip format", fileName);
        ::close(fd);
        return nullptr;
    }
    (void)lseek(fd, fileLen - 4, SEEK_SET);
    if (::read(fd, trailer, 4) != 4) {
        ::close(fd);
        return nullptr;
    }
    (void)lseek(fd, 0, SEEK_SET);
    size_t uncompressedLen = (size_t)((uint32_t)trailer[0] | ((uint32_t)trailer[1] << 8) |
                                      ((uint32_t)trailer[2] << 16) | ((uint32_t)trailer[3] << 24));

    _CompressedAsset* pAsset = new _CompressedAsset;
    status_t result = pAsset->openChunk(fd, 0, uncompressedLen, (size_t)fileLen);
    if (result != NO_ERROR) {
        delete pAsset;
        return nullptr;
    }
    pAsset->mAccessMode = mode;
    return pAsset;
}

// static
std::unique_ptr<Asset> Asset::createFromUncompressedBuffer(const void* data, size_t length,
                                                           AccessMode mode, bool owned) {
    auto pAsset = std::unique_ptr<_FileAsset>(new _FileAsset);
    status_t result = pAsset->openChunk(data, length, owned);
    if (result != NO_ERROR) return nullptr;
    pAsset->mAccessMode = mode;
    return pAsset;
}

// Generic seek() housekeeping. Verbatim from AOSP.
/*static-like*/ off64_t Asset::handleSeek(off64_t offset, int whence,
                                          off64_t curPosn, off64_t maxPosn) {
    off64_t newOffset;
    switch (whence) {
    case SEEK_SET: newOffset = offset; break;
    case SEEK_CUR: newOffset = curPosn + offset; break;
    case SEEK_END: newOffset = maxPosn + offset; break;
    default:
        LOGW("unexpected whence %d", whence);
        assert(false);
        return (off64_t)-1;
    }
    if (newOffset < 0 || newOffset > maxPosn) {
        LOGW("seek out of range: want %lld, end=%lld", (long long)newOffset, (long long)maxPosn);
        return (off64_t)-1;
    }
    return newOffset;
}

Asset::~Asset() {
}

// ===========================================================================
// _FileAsset
// ===========================================================================

_FileAsset::_FileAsset() : Asset() {
    registerAsset(this);
}

_FileAsset::~_FileAsset() {
    close();
    unregisterAsset(this);
}

status_t _FileAsset::openChunk(const char* fileName, int fd, off64_t offset, size_t length) {
    assert(mFp == nullptr);
    assert(mData == nullptr);
    assert(fd >= 0);
    assert(offset >= 0);

    off64_t fileLength = lseek64(fd, 0, SEEK_END);
    if (fileLength == (off64_t)-1) {
        LOGD("failed lseek (errno=%d)", errno);
        return UNKNOWN_ERROR;
    }
    if ((off64_t)(offset + length) > fileLength) {
        LOGD("start (%lld) + len (%zu) > end (%lld)", (long long)offset, length, (long long)fileLength);
        return BAD_INDEX;
    }

    mFp = fdopen(fd, "rb");   // after fdopen, fd is closed on fclose()
    if (mFp == nullptr) return UNKNOWN_ERROR;

    mStart = offset;
    mLength = length;
    if (fseek(mFp, (long)mStart, SEEK_SET) != 0) {
        assert(false);
    }
    mFileName = fileName != nullptr ? strdup(fileName) : nullptr;
    return NO_ERROR;
}

status_t _FileAsset::openChunk(const void* data, size_t length, bool owned) {
    assert(mFp == nullptr);
    assert(mData == nullptr);
    assert(data != nullptr || length == 0);

    mData = reinterpret_cast<const unsigned char*>(data);
    mDataOwned = owned;
    mStart = 0;
    mLength = length;
    return NO_ERROR;
}

ssize_t _FileAsset::read(void* buf, size_t count) {
    assert(mOffset >= 0 && mOffset <= mLength);

    if (getAccessMode() == ACCESS_BUFFER && mBuf == nullptr) {
        getBuffer(false);
    }

    size_t maxLen = (size_t)(mLength - mOffset);
    if (count > maxLen) count = maxLen;
    if (!count) return 0;

    size_t actual;
    if (mData != nullptr) {
        memcpy(buf, mData + mOffset, count);
        actual = count;
    } else if (mBuf != nullptr) {
        memcpy(buf, mBuf + mOffset, count);
        actual = count;
    } else {
        if (ftell(mFp) != (long)(mStart + mOffset)) {
            LOGE("Hosed: %ld != %lld+%lld", ftell(mFp), (long long)mStart, (long long)mOffset);
            assert(false);
        }
        actual = fread(buf, 1, count, mFp);
        if (actual == 0) return -1;
        assert(actual == count);
    }

    mOffset += actual;
    return (ssize_t)actual;
}

off64_t _FileAsset::seek(off64_t offset, int whence) {
    off64_t newPosn = handleSeek(offset, whence, mOffset, mLength);
    if (newPosn == (off64_t)-1) return newPosn;

    off64_t actualOffset = mStart + newPosn;
    if (mFp != nullptr) {
        if (fseek(mFp, (long)actualOffset, SEEK_SET) != 0) return (off64_t)-1;
    }
    mOffset = actualOffset - mStart;
    return mOffset;
}

void _FileAsset::close() {
    if (mBuf != nullptr) { delete[] mBuf; mBuf = nullptr; }
    if (mFileName != nullptr) { free(mFileName); mFileName = nullptr; }
    if (mDataOwned && mData != nullptr) {
        delete[] const_cast<unsigned char*>(mData);
        mData = nullptr;
        mDataOwned = false;
    } else {
        mData = nullptr;
    }
    if (mFp != nullptr) { fclose(mFp); mFp = nullptr; }
}

const void* _FileAsset::getBuffer(bool /*aligned*/) {
    if (mBuf != nullptr) return mBuf;
    if (mData != nullptr) return mData;
    assert(mFp != nullptr);

    // Small files: read into a heap buffer. (AOSP maps large files; CDROID has
    // no IncFsFileMap, so we always read in — the public behavior is identical.)
    long allocLen = mLength == 0 ? 1 : (long)mLength;
    unsigned char* buf = new (std::nothrow) unsigned char[allocLen];
    if (buf == nullptr) {
        LOGE("alloc of %ld bytes failed", allocLen);
        return nullptr;
    }
    if (mLength > 0) {
        long oldPosn = ftell(mFp);
        fseek(mFp, (long)mStart, SEEK_SET);
        if (fread(buf, 1, (size_t)mLength, mFp) != (size_t)mLength) {
            LOGE("failed reading %lld bytes", (long long)mLength);
            delete[] buf;
            return nullptr;
        }
        fseek(mFp, oldPosn, SEEK_SET);
    }
    mBuf = buf;
    return mBuf;
}

int _FileAsset::openFileDescriptor(off64_t* outStart, off64_t* outLength) const {
    if (mData != nullptr) return -1;   // buffer-backed: no fd
    if (mFileName == nullptr) return -1;
    *outStart = mStart;
    *outLength = mLength;
    return open(mFileName, O_RDONLY | O_BINARY);
}

// ===========================================================================
// _CompressedAsset  (gzip; inflated eagerly into a heap buffer with zlib)
// ===========================================================================

_CompressedAsset::_CompressedAsset() : Asset() {
    registerAsset(this);
}

_CompressedAsset::~_CompressedAsset() {
    close();
    unregisterAsset(this);
}

status_t _CompressedAsset::openChunk(int fd, off64_t offset, size_t uncompressedLen,
                                     size_t compressedLen) {
    assert(mFd < 0);
    assert(mData == nullptr);
    assert(fd >= 0);
    assert(compressedLen > 0);

    mStart = offset;
    mCompressedLen = compressedLen;
    mUncompressedLen = uncompressedLen;
    mFd = fd;
    return NO_ERROR;
}

status_t _CompressedAsset::openChunk(const void* data, size_t compressedLen,
                                     size_t uncompressedLen, bool owned) {
    assert(mFd < 0);
    assert(mData == nullptr);
    assert(data != nullptr);

    mData = reinterpret_cast<const unsigned char*>(data);
    mDataOwned = owned;
    mStart = 0;
    mCompressedLen = compressedLen;
    mUncompressedLen = uncompressedLen;
    return NO_ERROR;
}

ssize_t _CompressedAsset::read(void* buf, size_t count) {
    assert(mOffset >= 0 && mOffset <= mUncompressedLen);
    if (mBuf == nullptr) {
        if (getBuffer(false) == nullptr) return -1;
    }
    size_t maxLen = (size_t)(mUncompressedLen - mOffset);
    if (count > maxLen) count = maxLen;
    if (!count) return 0;
    memcpy(buf, mBuf + mOffset, count);
    mOffset += count;
    return (ssize_t)count;
}

off64_t _CompressedAsset::seek(off64_t offset, int whence) {
    off64_t newPosn = handleSeek(offset, whence, mOffset, mUncompressedLen);
    if (newPosn == (off64_t)-1) return newPosn;
    mOffset = newPosn;
    return mOffset;
}

void _CompressedAsset::close() {
    delete[] mBuf; mBuf = nullptr;
    if (mDataOwned && mData != nullptr) {
        delete[] const_cast<unsigned char*>(mData);
    }
    mData = nullptr; mDataOwned = false;
    if (mFd > 0) { ::close(mFd); mFd = -1; }
}

// Inflate gzip `in` (inLen bytes) into a new[] buffer of *outLen bytes. Replaces
// AOSP ZipUtils::inflateToBuffer + StreamingZipInflater (gzip path).
static unsigned char* gzip_inflate(const unsigned char* in, size_t inLen,
                                   size_t expectedOut, size_t* outLen) {
    z_stream strm;
    memset(&strm, 0, sizeof(strm));
    if (inflateInit2(&strm, 32 + MAX_WBITS) != Z_OK) return nullptr;  // gzip auto-detect

    size_t cap = expectedOut ? expectedOut : 1;
    unsigned char* out = new (std::nothrow) unsigned char[cap];
    if (out == nullptr) { inflateEnd(&strm); return nullptr; }

    strm.next_in = const_cast<Bytef*>(in);
    strm.avail_in = (uInt)inLen;
    strm.next_out = out;
    strm.avail_out = (uInt)cap;

    int rc;
    while ((rc = inflate(&strm, Z_NO_FLUSH)) == Z_OK && strm.avail_out == 0) {
        size_t newcap = cap * 2;
        unsigned char* nb = new (std::nothrow) unsigned char[newcap];
        if (nb == nullptr) { inflateEnd(&strm); delete[] out; return nullptr; }
        memcpy(nb, out, cap);
        delete[] out; out = nb;
        strm.next_out = out + cap;
        strm.avail_out = (uInt)(newcap - cap);
        cap = newcap;
    }
    inflateEnd(&strm);
    if (rc != Z_STREAM_END) { LOGW("gzip inflate failed (rc=%d)", rc); delete[] out; return nullptr; }
    *outLen = strm.total_out;
    return out;
}

const void* _CompressedAsset::getBuffer(bool /*aligned*/) {
    if (mBuf != nullptr) return mBuf;

    // Gather the compressed bytes.
    const unsigned char* in = nullptr;
    unsigned char* inOwned = nullptr;
    if (mData != nullptr) {
        in = mData;
    } else {
        assert(mFd >= 0);
        inOwned = new (std::nothrow) unsigned char[(size_t)mCompressedLen];
        if (inOwned == nullptr) return nullptr;
        if (lseek(mFd, mStart, SEEK_SET) != mStart) { delete[] inOwned; return nullptr; }
        ssize_t got = ::read(mFd, inOwned, (size_t)mCompressedLen);
        // ::read(2) may deliver short counts; loop until the chunk is filled.
        size_t total = got > 0 ? (size_t)got : 0;
        while (got > 0 && total < (size_t)mCompressedLen) {
            got = ::read(mFd, inOwned + total, (size_t)mCompressedLen - total);
            if (got <= 0) break;
            total += (size_t)got;
        }
        if (total != (size_t)mCompressedLen) { LOGW("short read of compressed data"); delete[] inOwned; return nullptr; }
        in = inOwned;
    }

    size_t outLen = 0;
    mBuf = gzip_inflate(in, (size_t)mCompressedLen, (size_t)mUncompressedLen, &outLen);
    delete[] inOwned;
    if (mBuf == nullptr) return nullptr;
    // For a valid gzip stream the inflated size equals ISIZE; trust mUncompressedLen.
    (void)outLen;
    return mBuf;
}
