// Port of AOSP androidfw/AssetsProvider.h (frameworks/base/libs/androidfw/
// include/androidfw/AssetsProvider.h), namespace cdroid.
//
// ZipAssetsProvider rides the ZeroCopyZip read header instead of libzip's
// ZipArchiveHandle: STORED entries become per-asset mmap windows
// (Asset::createFromMappedWindow — the AOSP createFromUncompressedMap face),
// DEFLATED entries inflate once into an owned buffer (createFromUncompressedBuffer).
// The provider keeps the pak fd + whole-file mapping alive for its lifetime;
// the arsc view is consumed under that lifetime by LoadedArsc.
//
// Trim boundary: no IncFs verification (no incremental FS), idmap untouched.
//
// Copyright (C) 2021 The Android Open Source Project
// Licensed under the Apache License, Version 2.0.
#ifndef __CDROID_ANDROIDFW_ASSETSPROVIDER_H__
#define __CDROID_ANDROIDFW_ASSETSPROVIDER_H__

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <content/androidfw/expected.h>        // NullOrIOError seam (optional)
#include <content/androidfw/loadedarsc.h>      // package_property_t
#include <content/androidfw/misc.h>           // FileType (AOSP: the one global-scope enum)
#include <content/androidfw/zerocopyzip.h>
#include <content/asset.h>

namespace cdroid {

// AOSP AssetsProvider.h includes androidfw/misc.h and uses its (global-scope)
// FileType family — exactly one FileType exists in androidfw; the enumerators
// below (kFileTypeRegular/...) resolve to ::kFileType* through misc.h.

// Interface responsible for opening and iterating through asset files.
struct AssetsProvider {
    static constexpr off64_t kUnknownLength = -1;

    // Opens a file for reading. If `file_exists` is not null, it will be set to `true` if the file
    // exists. This is useful for determining if the file exists but was unable to be opened due to
    // an I/O error.
    std::unique_ptr<Asset> Open(const std::string& path,
                                Asset::AccessMode mode = Asset::AccessMode::ACCESS_RANDOM,
                                bool* file_exists = nullptr) const;

    // Iterate over all files and directories provided by the interface. The order of iteration is
    // stable.
    virtual bool ForEachFile(const std::string& path,
                             const std::function<void(const std::string&, FileType)>& f) const = 0;

    // Retrieves the path to the contents of the AssetsProvider on disk. The path could represent an
    // APk, a directory, or some other file type.
    virtual const std::string* GetPath() const = 0;

    // Retrieves a name that represents the interface. This may or may not be the path of the
    // interface source.
    virtual const std::string& GetDebugName() const = 0;

    // Returns whether the interface provides the most recent version of its files.
    virtual bool IsUpToDate() const = 0;

    // Creates an Asset from a file on disk.
    static std::unique_ptr<Asset> CreateAssetFromFile(const std::string& path);

    // Creates an Asset from a file descriptor.
    //
    // The asset takes ownership of the file descriptor. If `length` equals kUnknownLength, offset
    // must equal 0; otherwise, the asset data will be read using the `offset` into the file
    // descriptor and will be `length` bytes long.
    static std::unique_ptr<Asset> CreateAssetFromFd(int fd,
                                                    const char* path,
                                                    off64_t offset = 0,
                                                    off64_t length = AssetsProvider::kUnknownLength);

    virtual ~AssetsProvider() = default;
protected:
    virtual std::unique_ptr<Asset> OpenInternal(const std::string& path, Asset::AccessMode mode,
                                                bool* file_exists) const = 0;
};

// Supplies assets from a zip archive.
struct ZipAssetsProvider : public AssetsProvider {
    static std::unique_ptr<ZipAssetsProvider> Create(std::string path,
                                                     package_property_t flags);

    bool ForEachFile(const std::string& root_path,
                     const std::function<void(const std::string&, FileType)>& f) const override;

    const std::string* GetPath() const override;
    const std::string& GetDebugName() const override;
    bool IsUpToDate() const override;

    ~ZipAssetsProvider() override = default;
protected:
    std::unique_ptr<Asset> OpenInternal(const std::string& path, Asset::AccessMode mode,
                                        bool* file_exists) const override;

private:
    ZipAssetsProvider(std::string&& path, package_property_t flags, time_t last_mod_time,
                      int fd);

    // The read header: holds the pak fd, the whole-file mmap and the parsed
    // central directory. Assets handed out either carry their own window mmap
    // (STORED) or an owned inflate buffer (DEFLATED) — neither borrows zip_.
    ZeroCopyZip zip_;
    std::string path_;
    bool path_is_debug_name_ = true;
    package_property_t flags_ = 0;
    time_t last_mod_time_ = -1;
    int fd_ = -1;   // zip_ keeps its own fd; this dup backs per-asset windows
};

// Supplies assets from a root directory.
struct DirectoryAssetsProvider : public AssetsProvider {
    static std::unique_ptr<DirectoryAssetsProvider> Create(std::string root_dir);

    bool ForEachFile(const std::string& path,
                     const std::function<void(const std::string&, FileType)>& f) const override;

    const std::string* GetPath() const override;
    const std::string& GetDebugName() const override;
    bool IsUpToDate() const override;

    ~DirectoryAssetsProvider() override = default;
protected:
    std::unique_ptr<Asset> OpenInternal(const std::string& path,
                                        Asset::AccessMode mode,
                                        bool* file_exists) const override;

private:
    explicit DirectoryAssetsProvider(std::string&& path, time_t last_mod_time);
    std::string dir_;
    time_t last_mod_time_;
};

// Supplies assets from a `primary` asset provider and falls back to supplying assets from the
// `secondary` asset provider if the asset cannot be found in the `primary`.
struct MultiAssetsProvider : public AssetsProvider {
    static std::unique_ptr<AssetsProvider> Create(std::unique_ptr<AssetsProvider>&& primary,
                                                  std::unique_ptr<AssetsProvider>&& secondary);

    bool ForEachFile(const std::string& root_path,
                     const std::function<void(const std::string&, FileType)>& f) const override;

    const std::string* GetPath() const override;
    const std::string& GetDebugName() const override;
    bool IsUpToDate() const override;

    ~MultiAssetsProvider() override = default;
protected:
    std::unique_ptr<Asset> OpenInternal(
            const std::string& path, Asset::AccessMode mode, bool* file_exists) const override;

private:
    MultiAssetsProvider(std::unique_ptr<AssetsProvider>&& primary,
                        std::unique_ptr<AssetsProvider>&& secondary);

    std::unique_ptr<AssetsProvider> primary_;
    std::unique_ptr<AssetsProvider> secondary_;
    std::string debug_name_;
};

// Does not provide any assets.
struct EmptyAssetsProvider : public AssetsProvider {
    static std::unique_ptr<AssetsProvider> Create();
    static std::unique_ptr<AssetsProvider> Create(const std::string& path);

    bool ForEachFile(const std::string& path,
                     const std::function<void(const std::string&, FileType)>& f) const override;

    const std::string* GetPath() const override;
    const std::string& GetDebugName() const override;
    bool IsUpToDate() const override;

    ~EmptyAssetsProvider() override = default;
protected:
    std::unique_ptr<Asset> OpenInternal(const std::string& path, Asset::AccessMode mode,
                                        bool* file_exists) const override;

private:
    explicit EmptyAssetsProvider(std::string&& path);
    std::string path_;
};

}  // namespace cdroid

#endif  // __CDROID_ANDROIDFW_ASSETSPROVIDER_H__
