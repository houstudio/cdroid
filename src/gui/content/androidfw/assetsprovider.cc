// Port of AOSP androidfw/AssetsProvider.cpp (frameworks/base/libs/androidfw/
// AssetsProvider.cpp), namespace cdroid — the zero-copy read-header edition.
//
// Copyright (C) 2021 The Android Open Source Project
// Licensed under the Apache License, Version 2.0.

#include <content/androidfw/assetsprovider.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

#include <algorithm>
#include <cstring>
#include <set>

#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <porting/cdlog.h>

namespace cdroid {

std::unique_ptr<Asset> AssetsProvider::Open(const std::string& path,
                                            Asset::AccessMode mode,
                                            bool* file_exists) const {
    return OpenInternal(path, mode, file_exists);
}

// The AOSP createFrom*Map factories are Asset friends; ours is protected via
// setAccessMode — but createFromUncompressedBuffer stays private, so route the
// owned-buffer wrap through a small subclass shim granting AssetsProvider the
// same access AOSP gives it.

std::unique_ptr<Asset> AssetsProvider::CreateAssetFromFile(const std::string& path) {
    const int fd = ::open(path.c_str(), O_RDONLY | O_BINARY);
    if (fd < 0) return nullptr;
    return CreateAssetFromFd(fd, path.c_str());
}

std::unique_ptr<Asset> AssetsProvider::CreateAssetFromFd(int fd,
                                                         const char* path,
                                                         off64_t offset,
                                                         off64_t length) {
    if (length == kUnknownLength) {
        if (offset != 0) return nullptr;
        struct stat st;
        if (fstat(fd, &st) != 0) return nullptr;
        length = st.st_size;
    }
    // Zero-copy for the whole-file case too: a private mmap window (the AOSP
    // createFromUncompressedMap shape).
    return std::unique_ptr<Asset>(
            Asset::createFromMappedWindow(fd, path, offset, (size_t)length,
                                          Asset::ACCESS_BUFFER));
}

/*============================================================================*
 * ZipAssetsProvider
 *============================================================================*/

ZipAssetsProvider::ZipAssetsProvider(std::string&& path, package_property_t flags,
                                     time_t last_mod_time, int fd)
    : path_(std::move(path)), flags_(flags), last_mod_time_(last_mod_time), fd_(fd) {}

std::unique_ptr<ZipAssetsProvider> ZipAssetsProvider::Create(std::string path,
                                                             package_property_t flags) {
    // The read header opens + maps the pak; a dup of its fd backs per-asset
    // window mmaps (AOSP dups the archive fd for the same purpose).
    ZeroCopyZip zip;
    if (!zip.open(path.c_str())) {
        return nullptr;
    }
    struct stat sb;
    time_t mtime = -1;
    if (stat(path.c_str(), &sb) == 0) mtime = sb.st_mtime;

    const int fd = ::open(path.c_str(), O_RDONLY | O_BINARY);
    if (fd < 0) {
        return nullptr;
    }

    auto out = std::unique_ptr<ZipAssetsProvider>(
            new ZipAssetsProvider(std::move(path), flags, mtime, fd));
    // Re-open onto the provider's own header (zip was a probe).
    if (!out->zip_.open(out->path_.c_str())) {
        return nullptr;
    }
    return out;
}

std::unique_ptr<Asset> ZipAssetsProvider::OpenInternal(const std::string& path,
                                                       Asset::AccessMode mode,
                                                       bool* file_exists) const {
    if (file_exists != nullptr) {
        *file_exists = false;
    }

    const ZeroCopyZip::Entry* entry = zip_.find(path);
    if (entry == nullptr) {
        return nullptr;
    }

    if (file_exists != nullptr) {
        *file_exists = true;
    }

    if (entry->method == 8 /* kCompressDeflated */) {
        // Inflate once into an owned buffer (AOSP createFromCompressedMap's
        // eager path; our Asset has no lazy StreamingZipInflater — every
        // consumer slurps in the ctor anyway).
        uint8_t* data = zip_.inflate(*entry);
        if (data == nullptr) {
            LOGE("Failed to decompress '%s' in APK '%s'", path.c_str(),
                 GetDebugName().c_str());
            return nullptr;
        }
        std::unique_ptr<Asset> asset(
                Asset::createFromUncompressedBuffer(data, (size_t)entry->uncompressedSize,
                                                    mode, /*owned*/ true));
        if (asset == nullptr) delete[] data;
        return asset;
    }

    // STORED: a private mmap window of [dataOffset, +uncompressedSize) — zero
    // copies; the asset owns its mapping and outlives the provider safely.
    return std::unique_ptr<Asset>(
            Asset::createFromMappedWindow(fd_, path.c_str(),
                                          (off64_t)entry->dataOffset,
                                          (size_t)entry->uncompressedSize, mode));
}

bool ZipAssetsProvider::ForEachFile(const std::string& root_path,
                                    const std::function<void(const std::string&, FileType)>& f)
                                     const {
    std::string root_path_full = root_path;
    if (root_path_full.empty() || root_path_full.back() != '/') {
        root_path_full += '/';
    }

    // We need to hold back directories because many paths will contain them and we want to only
    // surface one.
    std::set<std::string> dirs{};

    zip_.forEach([&](const std::string& name, const ZeroCopyZip::Entry&) {
        if (name.compare(0, root_path_full.size(), root_path_full) != 0) return;
        const std::string leaf = name.substr(root_path_full.size());
        if (leaf.empty()) return;
        const size_t slash = leaf.find('/');
        if (slash != std::string::npos) {
            dirs.insert(leaf.substr(0, slash));
        } else {
            f(leaf, kFileTypeRegular);
        }
    });

    // Now present the unique directories.
    for (const std::string& dir : dirs) {
        f(dir, kFileTypeDirectory);
    }
    return true;
}

const std::string* ZipAssetsProvider::GetPath() const {
    return &path_;
}

const std::string& ZipAssetsProvider::GetDebugName() const {
    return path_;
}

bool ZipAssetsProvider::IsUpToDate() const {
    struct stat sb;
    if (stat(path_.c_str(), &sb) < 0) {
        // If stat fails on the zip archive, return true so the zip archive the resource system does
        // attempt to refresh the ApkAsset.
        return true;
    }
    return last_mod_time_ == sb.st_mtime;
}

/*============================================================================*
 * DirectoryAssetsProvider
 *============================================================================*/

DirectoryAssetsProvider::DirectoryAssetsProvider(std::string&& path, time_t last_mod_time)
    : dir_(std::move(path)), last_mod_time_(last_mod_time) {}

std::unique_ptr<DirectoryAssetsProvider> DirectoryAssetsProvider::Create(std::string path) {
    struct stat sb;
    const int result = stat(path.c_str(), &sb);
    if (result == -1) {
        LOGE("Failed to find directory '%s'.", path.c_str());
        return nullptr;
    }

    if (!S_ISDIR(sb.st_mode)) {
        LOGE("Path '%s' is not a directory.", path.c_str());
        return nullptr;
    }

    if (path[path.size() - 1] != '/') {
        path += '/';
    }

    return std::unique_ptr<DirectoryAssetsProvider>(
            new DirectoryAssetsProvider(std::move(path), sb.st_mtime));
}

std::unique_ptr<Asset> DirectoryAssetsProvider::OpenInternal(const std::string& path,
                                                             Asset::AccessMode /* mode */,
                                                             bool* file_exists) const {
    const std::string resolved_path = dir_ + path;
    if (file_exists != nullptr) {
        struct stat sb;
        *file_exists = (stat(resolved_path.c_str(), &sb) != -1) && S_ISREG(sb.st_mode);
    }
    return CreateAssetFromFile(resolved_path);
}

bool DirectoryAssetsProvider::ForEachFile(
        const std::string& root_path,
        const std::function<void(const std::string&, FileType)>& f) const {
    const std::string root_path_full = dir_ + root_path + (root_path.empty() || root_path.back() == '/' ? "" : "/");
    DIR* d = opendir(root_path_full.c_str());
    if (d == nullptr) return false;
    struct dirent* de;
    while ((de = readdir(d)) != nullptr) {
        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) continue;
        struct stat sb;
        const std::string full = root_path_full + de->d_name;
        if (stat(full.c_str(), &sb) == 0) {
            f(de->d_name, S_ISDIR(sb.st_mode) ? kFileTypeDirectory : kFileTypeRegular);
        }
    }
    closedir(d);
    return true;
}

const std::string* DirectoryAssetsProvider::GetPath() const {
    return &dir_;
}

const std::string& DirectoryAssetsProvider::GetDebugName() const {
    return dir_;
}

bool DirectoryAssetsProvider::IsUpToDate() const {
    struct stat sb;
    if (stat(dir_.c_str(), &sb) < 0) return true;
    return last_mod_time_ == sb.st_mtime;
}

/*============================================================================*
 * MultiAssetsProvider
 *============================================================================*/

MultiAssetsProvider::MultiAssetsProvider(std::unique_ptr<AssetsProvider>&& primary,
                                         std::unique_ptr<AssetsProvider>&& secondary)
    : primary_(std::move(primary)), secondary_(std::move(secondary)) {
    const std::string* p = primary_->GetPath();
    const std::string* s = secondary_->GetPath();
    debug_name_ = (p ? *p : primary_->GetDebugName()) + std::string(":")
                  + (s ? *s : secondary_->GetDebugName());
}

std::unique_ptr<AssetsProvider> MultiAssetsProvider::Create(
        std::unique_ptr<AssetsProvider>&& primary, std::unique_ptr<AssetsProvider>&& secondary) {
    if (primary == nullptr) return std::move(secondary);
    if (secondary == nullptr) return std::move(primary);
    return std::unique_ptr<AssetsProvider>(
            new MultiAssetsProvider(std::move(primary), std::move(secondary)));
}

std::unique_ptr<Asset> MultiAssetsProvider::OpenInternal(const std::string& path,
                                                         Asset::AccessMode mode,
                                                         bool* file_exists) const {
    auto asset = primary_->Open(path, mode, file_exists);
    if (asset != nullptr || (file_exists != nullptr && *file_exists)) {
        return asset;
    }
    return secondary_->Open(path, mode, file_exists);
}

bool MultiAssetsProvider::ForEachFile(
        const std::string& root_path,
        const std::function<void(const std::string&, FileType)>& f) const {
    // Record the entries of the primary provider to avoid duplicating entries
    // implemented by both providers.
    std::set<std::string> entries;
    const auto f_primary = [&entries, &f](const std::string& path, FileType type) {
        f(path, type);
        entries.insert(path);
    };
    if (!primary_->ForEachFile(root_path, f_primary)) return false;

    return secondary_->ForEachFile(root_path,
                                   [&f, &entries](const std::string& path, FileType type) {
                                       if (entries.find(path) == entries.end()) {
                                           f(path, type);
                                       }
                                   });
}

const std::string* MultiAssetsProvider::GetPath() const {
    return primary_->GetPath();
}

const std::string& MultiAssetsProvider::GetDebugName() const {
    return debug_name_;
}

bool MultiAssetsProvider::IsUpToDate() const {
    return primary_->IsUpToDate() && secondary_->IsUpToDate();
}

/*============================================================================*
 * EmptyAssetsProvider
 *============================================================================*/

EmptyAssetsProvider::EmptyAssetsProvider(std::string&& path) : path_(std::move(path)) {}

std::unique_ptr<AssetsProvider> EmptyAssetsProvider::Create() {
    return std::unique_ptr<AssetsProvider>(new EmptyAssetsProvider(std::string()));
}

std::unique_ptr<AssetsProvider> EmptyAssetsProvider::Create(const std::string& path) {
    return std::unique_ptr<AssetsProvider>(new EmptyAssetsProvider(std::string(path)));
}

std::unique_ptr<Asset> EmptyAssetsProvider::OpenInternal(const std::string&,
                                                         Asset::AccessMode,
                                                         bool* file_exists) const {
    if (file_exists != nullptr) {
        *file_exists = false;
    }
    return nullptr;
}

bool EmptyAssetsProvider::ForEachFile(const std::string&,
                                      const std::function<void(const std::string&, FileType)>&)
                                        const {
    return true;
}

const std::string* EmptyAssetsProvider::GetPath() const {
    return path_.empty() ? nullptr : &path_;
}

const std::string& EmptyAssetsProvider::GetDebugName() const {
    return path_;
}

bool EmptyAssetsProvider::IsUpToDate() const {
    return true;
}

}  // namespace cdroid
