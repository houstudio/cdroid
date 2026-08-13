// Port of AOSP frameworks/base/libs/androidfw/include/androidfw/AssetManager.h.
//
// Asset management class. A faithful C++14 port of the LEGACY AssetManager that
// wraps a ResTable (the engine CDROID already has). Public class & method
// signatures match AOSP (String8 -> std::string, Vector -> std::vector). Internal
// adaptations, all under "内部隐藏类可适当裁剪":
//   - SharedZip / ZipSet (framework shared-table cache + RefBase/sp/wp) replaced
//     by a per-path libzip handle cache. CDROID has no use for the cross-process
//     shared framework table.
//   - ZipFileRO (libziparchive) -> libzip (zip_t), used only inside assetmanager.cc.
//   - idmap / runtime-resource-overlay machinery stubbed (no RRO in CDROID).
//   - Mutex/AutoMutex dropped (single-threaded UI resource access).
//
// Lives in namespace cdroid (isolated from cdroid::Assets). The wrapped engine
// types are brought in from the existing cdroid:: port via using-declarations.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
#ifndef __CDROID_ANDROIDFW_ASSETMANAGER_H__
#define __CDROID_ANDROIDFW_ASSETMANAGER_H__

#include <sys/types.h>
#include <stdint.h>
#include <string>
#include <vector>

#include <androidfw/restable.h>        // cdroid::ResTable (also pulls resourcetypes.h)
#include <androidfw/misc.h>
#include "core/asset.h"
#include "core/assetdir.h"

// Native-app access is via the opaque AAssetManager (C namespace). Matches AOSP.
struct AAssetManager { };

// libzip archive forward declaration. MUST be at global scope: <zip.h> (included
// only in assetmanager.cc) defines `typedef struct zip zip_t;` at global scope,
// so `struct zip*` here must refer to ::zip, not android::zip.
struct zip;

namespace cdroid {

using cdroid::ResTable;
using cdroid::ResTable_config;

// Every application that uses assets needs one instance. The AssetManager's
// purpose is to create Asset objects and to lazily build a ResTable from the
// resources.arsc found in each registered asset path.
class AssetManager : public AAssetManager {
public:
    static const char* RESOURCES_FILENAME;
    static const char* IDMAP_BIN;
    static const char* VENDOR_OVERLAY_DIR;
    static const char* SYSTEM_RESOURCES_PATH;   // framework-res path

    AssetManager();
    virtual ~AssetManager();

    static int32_t getGlobalCount();

    // Add a source for assets (directory or ZIP/APK). New paths are added at the
    // end and searched LAST (later = lower priority, matching AOSP). On success
    // *cookie is set to the 1-based index. Returns true on success.
    bool addAssetPath(const std::string& path, int32_t* cookie,
                      bool appAsLib = false, bool isSystemAsset = false);

    // RRO overlay path — no runtime resource overlays in CDROID; faithful stub.
    bool addOverlayPath(const std::string& packagePath, int32_t* cookie);

    // Add an already-open fd as an asset source. (Stub: returns false — not wired
    // in CDROID; asset sources are path-based.)
    bool addAssetFd(int fd, const std::string& debugPathName, int32_t* cookie,
                    bool appAsLib = false, bool assume_ownership = true);

    // Convenience for adding the standard system assets (ANDROID_ROOT/framework).
    bool addDefaultAssets();

    // Iterate asset-path cookies. Call with 0 to get the first; -1 means end.
    int32_t nextAssetPath(int32_t cookie) const;
    // The path string for a cookie previously returned by addAssetPath.
    std::string getAssetPath(int32_t cookie) const;

    // Set device configuration (orientation, size, locale, etc.). The optional
    // `locale` (bcp47) takes precedence over the locale in `config`.
    void setConfiguration(const ResTable_config& config, const char* locale = nullptr);
    void getConfiguration(ResTable_config* outConfig) const;

    typedef Asset::AccessMode AccessMode;

    // Open an asset (under assets/). Caller frees via delete / Asset::close().
    Asset* open(const char* fileName, AccessMode mode);
    // Open a non-asset file (e.g. resources.arsc, res/*, AndroidManifest.xml).
    Asset* openNonAsset(const char* fileName, AccessMode mode, int32_t* outCookie = nullptr);
    Asset* openNonAsset(int32_t cookie, const char* fileName, AccessMode mode);
    // Open a directory under assets/ ("" for the root), merged across paths.
    AssetDir* openDir(const char* dirName);
    AssetDir* openNonAssetDir(int32_t cookie, const char* dirName);

    // Quick existence/type test (regular files only).
    FileType getFileType(const char* fileName);

    // The complete resource table (lazily built from each path's resources.arsc).
    const ResTable& getResources(bool required = true) const;

    // Inject a pre-built ResTable so getResources()/getResTable() return it
    // verbatim instead of re-reading and re-parsing resources.arsc from each
    // asset path. The table is BORROWED (non-owning): the caller owns it and it
    // must outlive this AssetManager. This lets a host that already parsed the
    // arsc (e.g. cdroid::Assets::mResTable) share its table with this AOSP layer,
    // avoiding a duplicate parse of the same data. If this manager had already
    // built its own table, that owned copy is released first.
    void setResTable(ResTable* table);

    // True if no referenced file has changed since this manager was created.
    bool isUpToDate();

    // Known locales for this manager (from the loaded resource tables).
    void getLocales(std::vector<std::string>* locales, bool includeSystemLocales = true) const;

private:
    struct asset_path {
        std::string  path;
        int32_t      cookie = 0;       // 1-based index into mAssetPaths
        int          rawFd = -1;
        FileType     type = kFileTypeRegular;
        std::string  idmap;
        bool         isSystemOverlay = false;
        bool         isSystemAsset = false;
        bool         assumeOwnership = false;
        mutable struct zip* zip = nullptr;   // cached libzip handle
        mutable bool zipTried = false;
        time_t modWhen = 0;                  // mtime at addAssetPath time (isUpToDate)
        ~asset_path();
    };

    Asset* openNonAssetInPath(const char* fileName, AccessMode mode, asset_path& ap);
    std::string createPathName(const asset_path& ap, const char* rootDir);
    std::string createZipSourceName(const std::string& zipFileName,
                                    const std::string& dirName, const std::string& fileName);

    // libzip handle cache (replaces AOSP ZipSet/SharedZip). Caches into the
    // mutable per-path handle, so it takes a const asset_path&.
    struct zip* getZipFile(const asset_path& ap);
    Asset* openAssetFromFile(const std::string& pathName, AccessMode mode);
    Asset* openAssetFromZip(struct zip* zip, int64_t entry, AccessMode mode,
                            const std::string& entryName);

    const ResTable* getResTable(bool required = true) const;
    void setLocale(const char* locale);
    void updateResourceParams() const;
    bool appendPathToResTable(asset_path& ap, bool appAsLib = false);

    bool scanAndMergeDir(std::vector<AssetDir::FileInfo>* merged, const asset_path& ap,
                         const char* rootDir, const char* dirName);
    bool scanDirInto(const std::string& path, std::vector<AssetDir::FileInfo>* out);
    bool scanAndMergeZip(std::vector<AssetDir::FileInfo>* merged, const asset_path& ap,
                         const char* rootDir, const char* baseDirName);
    void mergeInfo(std::vector<AssetDir::FileInfo>* merged,
                   const std::vector<AssetDir::FileInfo>* contents);

    std::vector<asset_path> mAssetPaths;
    char*                   mLocale = nullptr;
    mutable ResTable*       mResources = nullptr;
    mutable bool            mOwnsResources = false;   // true when getResTable() new'd mResources
    ResTable_config*        mConfig;
};

} // namespace cdroid
#endif // __CDROID_ANDROIDFW_ASSETMANAGER_H__
