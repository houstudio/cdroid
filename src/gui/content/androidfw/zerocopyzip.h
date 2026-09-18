// Zero-copy zip read header (the campaign ② runtime piece).
//
// Replaces libzip for the AM2 AssetsProvider path: the whole pak is opened
// once and mmap'd; the central directory is parsed into a name -> entry map.
// STORED entries hand out base+offset views (zero copies, zero syscalls —
// the AOSP IncFsFileMap equivalent); DEFLATED entries fall back to a raw
// zlib inflate into one heap buffer (the necessary copy; AOSP's
// _CompressedAsset). Legacy all-compressed paks keep working — the method is
// probed per entry, no versioning, no migration.
//
// Layout knowledge (appnote.txt):
//   EOCD           sig 0x06054b50, 22 bytes fixed, at/near file end
//   ZIP64 EOCD     sig 0x06064b50, 56 bytes fixed
//   central record sig 0x02014b50, 46 bytes fixed + name + extra + comment
//   local header   sig 0x04034b50, 30 bytes fixed + name + extra
// The local header's name/extra lengths may differ from the central record's
// (tools rewrite extra), so the data offset MUST be computed from the local
// header (offset + 30 + localNameLen + localExtraLen).
#ifndef __CDROID_ANDROIDFW_ZEROCOPYZIP_H__
#define __CDROID_ANDROIDFW_ZEROCOPYZIP_H__

#include <cstdint>
#include <cstddef>
#include <functional>
#include <vector>
#include <string>
#include <unordered_map>

namespace cdroid {

class ZeroCopyZip {
public:
    struct Entry {
        uint16_t method;          // 0 = STORED, 8 = DEFLATED
        uint16_t flags;
        uint32_t crc32;
        uint64_t compressedSize;
        uint64_t uncompressedSize;
        uint64_t dataOffset;      // absolute offset of the entry DATA in the file
    };

    ZeroCopyZip() = default;
    ~ZeroCopyZip();
    ZeroCopyZip(const ZeroCopyZip&) = delete;
    ZeroCopyZip& operator=(const ZeroCopyZip&) = delete;

    // open + fstat + mmap the whole file + parse the central directory.
    bool open(const char* path);
    void close();
    bool isOpen() const { return map_ != nullptr; }

    const Entry* find(const std::string& name) const;
    size_t entryCount() const { return entries_.size(); }

    // Stable-order iteration over (name, entry).
    void forEach(const std::function<void(const std::string&, const Entry&)>& f) const;

    // The mapped window — outlives `this`'s own lifetime only while open().
    const uint8_t* base() const { return map_; }
    size_t size() const { return mapSize_; }
    const char* path() const { return path_.c_str(); }

    // Inflate one DEFLATED entry into a fresh heap buffer (deleted[] by the
    // caller). Returns nullptr on error.
    uint8_t* inflate(const Entry& e) const;

private:
    bool parseCentralDir();

    int fd_ = -1;
    uint8_t* map_ = nullptr;
    size_t mapSize_ = 0;
    std::string path_;

    // Central-directory order is kept for ForEachFile's stable iteration.
    std::vector<std::string> order_;
    std::unordered_map<std::string, Entry> entries_;
};

}  // namespace cdroid

#endif  // __CDROID_ANDROIDFW_ZEROCOPYZIP_H__
