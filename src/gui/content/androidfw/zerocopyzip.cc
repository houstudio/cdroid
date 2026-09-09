// Zero-copy zip read header implementation. See zerocopyzip.h.
#include <content/androidfw/zerocopyzip.h>

#include <cstring>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <zlib.h>

#include <porting/cdlog.h>

namespace cdroid {

namespace {

constexpr uint32_t kEocdSig = 0x06054b50;
constexpr uint32_t kZip64EocdSig = 0x06064b50;
constexpr uint32_t kZip64EocdLocatorSig = 0x07064b50;
constexpr uint32_t kCentralSig = 0x02014b50;
constexpr uint32_t kLocalSig = 0x04034b50;
constexpr size_t kEocdMin = 22;            // fixed part
constexpr size_t kEocdSearchBack = 65536 + kEocdMin;   // max comment 64KiB
constexpr size_t kCentralFixed = 46;
constexpr size_t kLocalFixed = 30;

inline uint16_t rd16(const uint8_t* p) { return (uint16_t)(p[0] | (p[1] << 8)); }
inline uint32_t rd32(const uint8_t* p) {
    return (uint32_t)(p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24));
}
inline uint64_t rd64(const uint8_t* p) {
    return (uint64_t)rd32(p) | ((uint64_t)rd32(p + 4) << 32);
}

bool inBounds(size_t size, size_t off, size_t len) {
    return off <= size && len <= size - off;
}

}  // namespace

ZeroCopyZip::~ZeroCopyZip() {
    close();
}

bool ZeroCopyZip::open(const char* path) {
    close();
    fd_ = ::open(path, O_RDONLY);
    if (fd_ < 0) {
        LOGE("ZeroCopyZip: cannot open %s", path);
        return false;
    }
    struct stat st;
    if (fstat(fd_, &st) != 0 || !S_ISREG(st.st_mode) || st.st_size <= 0) {
        LOGE("ZeroCopyZip: %s is not a regular non-empty file", path);
        close();
        return false;
    }
    mapSize_ = (size_t)st.st_size;
    void* m = mmap(nullptr, mapSize_, PROT_READ, MAP_SHARED, fd_, 0);
    if (m == MAP_FAILED) {
        LOGE("ZeroCopyZip: mmap(%s, %zu) failed", path, mapSize_);
        close();
        return false;
    }
    map_ = (uint8_t*)m;
    path_ = path;
    if (!parseCentralDir()) {
        close();
        return false;
    }
    return true;
}

void ZeroCopyZip::close() {
    if (map_) {
        munmap(map_, mapSize_);
        map_ = nullptr;
        mapSize_ = 0;
    }
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
    entries_.clear();
    order_.clear();
    path_.clear();
}

bool ZeroCopyZip::parseCentralDir() {
    // Find the EOCD record: scan backwards over the possible comment.
    if (mapSize_ < kEocdMin) return false;
    const size_t searchStart = mapSize_ > kEocdSearchBack ? mapSize_ - kEocdSearchBack : 0;
    size_t eocdOff = (size_t)-1;
    for (size_t i = mapSize_ - kEocdMin + 1; i-- > searchStart;) {
        if (rd32(map_ + i) == kEocdSig) {
            eocdOff = i;
            break;
        }
    }
    if (eocdOff == (size_t)-1) {
        LOGE("ZeroCopyZip: EOCD not found");
        return false;
    }

    uint64_t cdSize = rd32(map_ + eocdOff + 12);
    uint64_t cdOffset = rd32(map_ + eocdOff + 16);
    const uint16_t totalEntries = rd16(map_ + eocdOff + 10);

    // ZIP64: count/size/offset 0xffffffff in the classic EOCD point at the
    // ZIP64 EOCD (located via the 20-byte locator right before the EOCD).
    if ((cdSize == 0xffffffffULL || cdOffset == 0xffffffffULL || totalEntries == 0xffff)
            && eocdOff >= 20 && rd32(map_ + eocdOff - 20) == kZip64EocdLocatorSig) {
        const uint64_t z64Off = rd64(map_ + eocdOff - 20 + 8);
        if (inBounds(mapSize_, (size_t)z64Off, 56)
                && rd32(map_ + z64Off) == kZip64EocdSig) {
            const uint8_t* z = map_ + z64Off;
            const uint64_t z64Total = rd64(z + 32);
            cdSize = rd64(z + 40);
            cdOffset = rd64(z + 48);
            if (z64Total > 0 && z64Total != 0xffffffffffffffffULL
                    && z64Total <= mapSize_ / kCentralFixed) {
                order_.reserve((size_t)z64Total);
            }
        }
    }
    if (!inBounds(mapSize_, (size_t)cdOffset, (size_t)cdSize)) {
        LOGE("ZeroCopyZip: central directory out of bounds (off=%llu size=%llu file=%zu)",
             (unsigned long long)cdOffset, (unsigned long long)cdSize, mapSize_);
        return false;
    }

    size_t off = (size_t)cdOffset;
    const size_t end = (size_t)cdOffset + (size_t)cdSize;
    while (off + kCentralFixed <= end) {
        const uint8_t* c = map_ + off;
        if (rd32(c) != kCentralSig) break;

        Entry e;
        e.flags = rd16(c + 8);
        e.method = rd16(c + 10);
        e.crc32 = rd32(c + 16);
        e.compressedSize = rd32(c + 20);
        e.uncompressedSize = rd32(c + 24);
        const uint16_t nameLen = rd16(c + 28);
        const uint16_t extraLen = rd16(c + 30);
        const uint16_t commentLen = rd16(c + 32);
        uint64_t localOff = rd32(c + 42);

        // ZIP64 extra field (0x0001): 64-bit sizes/offset when the 32-bit
        // fields are saturated, in ORDER: size, compressed, uncompressed, offset.
        const uint8_t* extra = c + kCentralFixed + nameLen;
        size_t extraLeft = extraLen;
        while (extraLeft >= 4) {
            const uint16_t id = rd16(extra);
            const uint16_t sz = rd16(extra + 2);
            if (extraLeft < (size_t)sz + 4) break;
            if (id == 0x0001) {
                const uint8_t* z = extra + 4;
                size_t zleft = sz;
                if (e.uncompressedSize == 0xffffffffULL && zleft >= 8) {
                    e.uncompressedSize = rd64(z); z += 8; zleft -= 8;
                }
                if (e.compressedSize == 0xffffffffULL && zleft >= 8) {
                    e.compressedSize = rd64(z); z += 8; zleft -= 8;
                }
                if (localOff == 0xffffffffULL && zleft >= 8) {
                    localOff = rd64(z); z += 8; zleft -= 8;
                }
                break;
            }
            extra += (size_t)sz + 4;
            extraLeft -= (size_t)sz + 4;
        }

        if (!inBounds(mapSize_, off + kCentralFixed, nameLen)) break;
        std::string name((const char*)c + kCentralFixed, nameLen);

        // Data offset from the LOCAL header (its name/extra lengths may
        // differ from the central record's — appnote 4.3.7 / local-header
        // extra fields get rewritten by some tools).
        if (!inBounds(mapSize_, (size_t)localOff, kLocalFixed)
                || rd32(map_ + localOff) != kLocalSig) {
            LOGW("ZeroCopyZip: bad local header for %s", name.c_str());
            off += kCentralFixed + nameLen + extraLen + commentLen;
            continue;
        }
        const uint8_t* l = map_ + localOff;
        const uint16_t localNameLen = rd16(l + 26);
        const uint16_t localExtraLen = rd16(l + 28);
        e.dataOffset = localOff + kLocalFixed + localNameLen + localExtraLen;

        // Directory markers (name ends '/') carry no data — keep them out.
        if (!name.empty() && name.back() != '/') {
            entries_.emplace(name, e);
            order_.push_back(name);
        }

        off += kCentralFixed + nameLen + extraLen + commentLen;
    }

    if (entries_.empty()) {
        LOGE("ZeroCopyZip: no entries parsed (cdOff=%llu cdSize=%llu)",
             (unsigned long long)cdOffset, (unsigned long long)cdSize);
        return false;
    }
    return true;
}

const ZeroCopyZip::Entry* ZeroCopyZip::find(const std::string& name) const {
    const auto it = entries_.find(name);
    return it == entries_.end() ? nullptr : &it->second;
}

void ZeroCopyZip::forEach(
        const std::function<void(const std::string&, const Entry&)>& f) const {
    for (const std::string& name : order_) {
        const auto it = entries_.find(name);
        if (it != entries_.end()) f(name, it->second);
    }
}

uint8_t* ZeroCopyZip::inflate(const Entry& e) const {
    if (!inBounds(mapSize_, (size_t)e.dataOffset, (size_t)e.compressedSize)) {
        LOGE("ZeroCopyZip: entry data out of window");
        return nullptr;
    }
    uint8_t* out = new (std::nothrow) uint8_t[(size_t)e.uncompressedSize];
    if (out == nullptr) return nullptr;

    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    // Raw deflate (zip entries carry no zlib header): windowBits = -15.
    if (inflateInit2(&zs, -15) != Z_OK) {
        delete[] out;
        return nullptr;
    }
    zs.next_in = const_cast<uint8_t*>(map_ + e.dataOffset);
    zs.avail_in = (uInt)e.compressedSize;
    zs.next_out = out;
    zs.avail_out = (uInt)e.uncompressedSize;
    const int rc = ::inflate(&zs, Z_FINISH);
    ::inflateEnd(&zs);
    if (rc != Z_STREAM_END || zs.total_out != e.uncompressedSize) {
        LOGE("ZeroCopyZip: inflate failed (rc=%d out=%lu want=%llu)", rc,
             zs.total_out, (unsigned long long)e.uncompressedSize);
        delete[] out;
        return nullptr;
    }
    return out;
}

}  // namespace cdroid
