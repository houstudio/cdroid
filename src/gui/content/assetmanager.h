// Port of AOSP frameworks/base/libs/androidfw/include/androidfw/AssetManager.h.
//
// Asset management class. Originally a C++14 port of the LEGACY AssetManager
// over ResTable; the AM2 switch (③-4b) moved the table half onto AssetManager2
// — the AOSP Java-side shape (AssetManager.java holds the ApkAssets list and
// commits it atomically via nativeSetApkAssets). Public class & method
// signatures match AOSP (String8 -> std::string, Vector -> std::vector).
// Internal adaptations, all under "内部隐藏类可适当裁剪":
//   - SharedZip / ZipSet (framework shared-table cache + RefBase/sp/wp) replaced
//     by a per-path libzip handle cache for the FILE half. The table half reads
//     through ApkAssets' AssetsProvider (mmap'd, STORED zero-copy).
//   - ZipFileRO (libziparchive) -> libzip (zip_t), used only inside assetmanager.cc.
//   - idmap / runtime-resource-overlay machinery stubbed (no RRO in CDROID).
//   - Mutex/AutoMutex dropped (single-threaded UI resource access).
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
#include <memory>
#include <string>
#include <vector>

#include <content/androidfw/apkassets.h>       // ApkAssets (owned list)
#include <content/androidfw/assetmanager2.h>   // AssetManager2 (the table engine)
#include <content/androidfw/misc.h>
#include "content/asset.h"
#include "content/assetdir.h"

// Native-app access is via the opaque AAssetManager (C namespace). Matches AOSP.
struct AAssetManager { };

// libzip archive forward declaration. MUST be at global scope: <zip.h> (included
// only in assetmanager.cc) defines `typedef struct zip zip_t;` at global scope,
// so `struct zip*` here must refer to ::zip, not android::zip.
struct zip;

namespace cdroid {

using cdroid::ResTable_config;

// Every application that uses assets needs one instance. The AssetManager's
// purpose is to create Asset objects and to hold the AssetManager2 table
// built from the resources.arsc of each registered asset path.
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
    ::FileType getFileType(const char* fileName);   // AOSP misc.h FileType

    // The table engine (AM2). AOSP's Java AssetManager holds the ApkAssets
    // list and commits it to the native AssetManager2; here the same object
    // owns both, and getResources()'s former ResTable& face is served by it.
    AssetManager2& getAssetManager2() const { return *mAm2; }

    // CDROID seam on the legacy ResTable::getIdentifier semantics (AOSP's
    // AM2 GetResourceId needs a package): aapt2 forces a dotted package name
    // ("cdroid.<ns>") that won't match the pak name, so try the requested
    // package, its "cdroid."-prefixed form, the framework ("android"), then
    // every registered package.
    int getIdentifier(const std::string& name, const std::string& type,
                      const std::string& package) const;
    // CDROID seam on the legacy ResTable::getResourceName(id, &pkg, &type,
    // &key): AM2's ResourceName carries u8-or-u16 faces; flatten here. Empty
    // parts (missing type/key) report false, like the legacy face.
    bool getResourceName(uint32_t id, std::string* pkg, std::string* type,
                         std::string* key) const;

    // AOSP AssetManager.getLocales(): the locales this resource table carries —
    // one "xx-YY" tag per distinct config (language lower-case, region
    // upper-case, no script; the default config carries no locale and is
    // skipped). Feed each tag to Locale::forLanguageTag() to pull the
    // language/script/country apart. Empty when no arsc is loaded.
    std::vector<std::string> getLocales() const;
    // AOSP AssetManager.getNonSystemLocales(): same, minus locales provided
    // ONLY by the framework (the android package) — the app's own languages.
    std::vector<std::string> getNonSystemLocales() const;
    // AOSP Resources.getSystem().getAssets().getLocales() equivalent: CDROID
    // merges framework and app paks into one table, so the "system" set is the
    // android-package (runtime id 0x01) set of that same table.
    std::vector<std::string> getSystemLocales() const;

    // True if no referenced file has changed since this manager was created.
    bool isUpToDate();

private:
    struct asset_path {
        std::string  path;
        int32_t      cookie = 0;       // 1-based index into mAssetPaths
        int          rawFd = -1;
        ::FileType   type = ::kFileTypeRegular;   // AOSP misc.h FileType (global scope)
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

    void setLocale(const char* locale);
    void updateResourceParams() const;

    bool scanAndMergeDir(std::vector<AssetDir::FileInfo>* merged, const asset_path& ap,
                         const char* rootDir, const char* dirName);
    bool scanDirInto(const std::string& path, std::vector<AssetDir::FileInfo>* out);
    bool scanAndMergeZip(std::vector<AssetDir::FileInfo>* merged, const asset_path& ap,
                         const char* rootDir, const char* baseDirName);
    void mergeInfo(std::vector<AssetDir::FileInfo>* merged,
                   const std::vector<AssetDir::FileInfo>* contents);

    std::vector<asset_path> mAssetPaths;
    char*                   mLocale = nullptr;
    // The table engine (AM2) + the ApkAssets it reads. One ApkAssets per
    // registered pak path (loaded once, kept alive), committed atomically via
    // SetApkAssets on every add — the AOSP Java setApkAssets shape.
    std::unique_ptr<AssetManager2>       mAm2;
    std::vector<std::unique_ptr<ApkAssets>> mApkAssets;
    ResTable_config*        mConfig;
};

} // namespace cdroid
#endif // __CDROID_ANDROIDFW_ASSETMANAGER_H__
