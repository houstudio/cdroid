// Port of AOSP frameworks/base/libs/androidfw/AssetManager.cpp.
//
// Provide access to read-only assets. Logic translated verbatim from the AOSP
// legacy AssetManager file half; the table half is AssetManager2 (see header);
// adaptations (all internal) called out inline:
//   - SharedZip/ZipSet  -> a per-path libzip (zip_t*) handle cache (getZipFile).
//   - ZipFileRO         -> libzip (zip_open/zip_name_locate/zip_fopen_index/...).
//   - idmap/RRO/overlay   -> trimmed (stubs return false).
//   - Mutex/AutoMutex     -> dropped (single-threaded UI resource access).

#define LOG_TAG "asset"

#include "content/assetmanager.h"
#include "content/androidfw/resourcetypes.h"   // pakPathCandidates (res/-prefix bridging)
#include <text/textutils.h>                    // utf16_utf8 (getResourceName)

#include <porting/cdlog.h>
#include <zip.h>

#include <algorithm>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <set>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace cdroid;

static const bool kIsDebug = false;

static const char* kAssetsRoot = "assets";
static const char* kSystemAssets = "framework/framework-res.apk";

static int32_t gAmCount = 0;

const char* AssetManager::RESOURCES_FILENAME = "resources.arsc";
const char* AssetManager::IDMAP_BIN = "/system/bin/idmap";
const char* AssetManager::VENDOR_OVERLAY_DIR = "/vendor/overlay";
const char* AssetManager::SYSTEM_RESOURCES_PATH = "framework/framework-res.apk";

// Like strdup(), but uses new[] (freed with delete[]) to match the rest of this port.
static char* strdupNew(const char* str) {
    if (str == nullptr) return nullptr;
    size_t len = strlen(str);
    char* s = new char[len + 1];
    memcpy(s, str, len + 1);
    return s;
}

// ===========================================================================
// asset_path
// ===========================================================================

AssetManager::asset_path::~asset_path() {
    if (zip != nullptr) {
        zip_close(zip);
        zip = nullptr;
    }
}

// ===========================================================================
// AssetManager
// ===========================================================================

// static
int32_t AssetManager::getGlobalCount() {
    return gAmCount;
}

AssetManager::AssetManager() : mLocale(nullptr), mConfig(new ResTable_config) {
    gAmCount++;
    memset(mConfig, 0, sizeof(ResTable_config));
    mAm2.reset(new AssetManager2());
    if (kIsDebug) LOGI("Creating AssetManager %p #%d", this, gAmCount);
}

AssetManager::~AssetManager() {
    gAmCount--;
    if (kIsDebug) LOGI("Destroying AssetManager %p #%d", this, gAmCount);

    for (size_t i = 0; i < mAssetPaths.size(); i++) {
        if (mAssetPaths[i].rawFd >= 0 && mAssetPaths[i].zip == nullptr) {
            close(mAssetPaths[i].rawFd);
        }
    }
    delete mConfig;
    delete[] mLocale;
}

bool AssetManager::addAssetPath(const std::string& path, int32_t* cookie,
                                bool appAsLib, bool isSystemAsset) {
    asset_path ap;

    std::string realPath(path);
    ap.type = ::getFileType(realPath.c_str());
    if (ap.type == ::kFileTypeRegular) {
        ap.path = realPath;
    } else {
        ap.path = path;
        ap.type = ::getFileType(path.c_str());
        if (ap.type != ::kFileTypeDirectory && ap.type != ::kFileTypeRegular) {
            LOGW("Asset path %s is neither a directory nor file (type=%d).", path.c_str(), (int)ap.type);
            return false;
        }
    }

    // Skip if we already have it.
    for (size_t i = 0; i < mAssetPaths.size(); i++) {
        if (mAssetPaths[i].path == ap.path) {
            if (cookie) *cookie = mAssetPaths[i].cookie;
            return true;
        }
    }

    LOGV("In %p Asset %s path: %s", this,
          ap.type == ::kFileTypeDirectory ? "dir" : "zip", ap.path.c_str());

    ap.isSystemAsset = isSystemAsset;
    ap.modWhen = getFileModDate(ap.path.c_str());
    ap.cookie = (int32_t)(mAssetPaths.size() + 1);   // 1-based index
    mAssetPaths.push_back(ap);

    if (cookie) *cookie = mAssetPaths.back().cookie;

    // AM2: one immutable ApkAssets per pak path, then an atomic commit of the
    // whole set (AOSP Java AssetManager collects apkAssets and calls
    // nativeSetApkAssets). Loading per add keeps later paks visible to the
    // table without invalidating anything earlier paths resolved —
    // SetApkAssets rebuilds package groups and flushes the bag caches.
    if (ap.type == ::kFileTypeRegular) {
        package_property_t flags = 0U;
        if (isSystemAsset) flags |= PROPERTY_SYSTEM;
        if (appAsLib) flags |= PROPERTY_DYNAMIC;   // LoadAsSharedLibrary semantics
        std::unique_ptr<ApkAssets> loaded = ApkAssets::Load(ap.path, flags);
        if (loaded != nullptr) {
            mApkAssets.push_back(std::move(loaded));
            std::vector<const ApkAssets*> commit;
            commit.reserve(mApkAssets.size());
            for (const auto& a : mApkAssets) commit.push_back(a.get());
            mAm2->SetApkAssets(std::move(commit));
            updateResourceParams();
        } else {
            LOGW("addAssetPath: ApkAssets::Load(%s) failed (no table from this path)",
                 ap.path.c_str());
        }
    }
    return true;
}

// RRO overlays are not supported in CDROID; faithful stub.
bool AssetManager::addOverlayPath(const std::string& /*packagePath*/, int32_t* /*cookie*/) {
    return false;
}

// fd-based asset sources are not wired in CDROID; faithful stub.
bool AssetManager::addAssetFd(int /*fd*/, const std::string& /*debugPathName*/,
                              int32_t* /*cookie*/, bool /*appAsLib*/, bool /*assume_ownership*/) {
    return false;
}

bool AssetManager::addDefaultAssets() {
    const char* root = getenv("ANDROID_ROOT");
    if (root == nullptr) return false;   // CDROID has no ANDROID_ROOT; graceful no-op
    std::string path(root);
    path += "/";
    path += kSystemAssets;
    return addAssetPath(path, nullptr, false /* appAsLib */, true /* isSystemAsset */);
}

int32_t AssetManager::nextAssetPath(int32_t cookie) const {
    const size_t next = (size_t)cookie + 1;
    return next > mAssetPaths.size() ? -1 : (int32_t)next;
}

std::string AssetManager::getAssetPath(int32_t cookie) const {
    const size_t which = (size_t)cookie - 1;
    if (which < mAssetPaths.size()) return mAssetPaths[which].path;
    return std::string();
}

void AssetManager::setLocale(const char* locale) {
    delete[] mLocale;
    mLocale = strdupNew(locale);
    updateResourceParams();
}

void AssetManager::setConfiguration(const ResTable_config& config, const char* locale) {
    *mConfig = config;
    // AOSP repacks a bcp47 `locale` into mConfig via setBcp47Locale; this port's
    // ResTable_config has no setBcp47Locale, so the caller is expected to set the
    // language/country fields in `config` directly. We still honor the bcp47
    // string for record-keeping (mLocale) and push mConfig to the table.
    if (locale != nullptr) {
        setLocale(locale);
    } else {
        updateResourceParams();
    }
}

void AssetManager::getConfiguration(ResTable_config* outConfig) const {
    *outConfig = *mConfig;
}

Asset* AssetManager::open(const char* fileName, AccessMode mode) {
    LOGI_IF(kIsDebug, "open(%s)", fileName);
    FATAL_IF(mAssetPaths.empty(), "No assets added to AssetManager");

    std::string assetName(kAssetsRoot);
    assetName += "/";
    assetName += fileName;

    // Search paths in reverse (later-added = lower priority, matches AOSP).
    for (size_t i = mAssetPaths.size(); i > 0; ) {
        --i;
        Asset* pAsset = openNonAssetInPath(assetName.c_str(), mode, mAssetPaths[i]);
        if (pAsset != nullptr) return pAsset;
    }
    return nullptr;
}

Asset* AssetManager::openNonAsset(const char* fileName, AccessMode mode, int32_t* outCookie) {
    FATAL_IF(mAssetPaths.empty(), "No assets added to AssetManager");

    for (size_t i = mAssetPaths.size(); i > 0; ) {
        --i;
        Asset* pAsset = openNonAssetInPath(fileName, mode, mAssetPaths[i]);
        if (pAsset != nullptr) {
            if (outCookie != nullptr) *outCookie = mAssetPaths[i].cookie;
            return pAsset;
        }
    }
    return nullptr;
}

Asset* AssetManager::openNonAsset(int32_t cookie, const char* fileName, AccessMode mode) {
    const size_t which = (size_t)cookie - 1;
    FATAL_IF(mAssetPaths.empty(), "No assets added to AssetManager");
    if (which < mAssetPaths.size()) {
        return openNonAssetInPath(fileName, mode, mAssetPaths[which]);
    }
    return nullptr;
}

::FileType AssetManager::getFileType(const char* fileName) {
    Asset* pAsset = open(fileName, Asset::ACCESS_STREAMING);
    bool exists = (pAsset != nullptr);
    delete pAsset;
    return exists ? ::kFileTypeRegular : ::kFileTypeNonexistent;
}

// Table half is AssetManager2 now (see header). The arsc is read through each
// path's ApkAssets (mmap'd; STORED entries are zero-copy) — there is no
// ResTable to append into and no borrowed-table injection seam.

void AssetManager::updateResourceParams() const {
    mAm2->SetConfiguration(*mConfig);
}

// getIdentifier ladder (see header): the legacy ResTable face the resource
// layers were written against. GetResourceId's own fallbacks only fill empty
// parts of the name string, so the package retry sequence is explicit here.
int AssetManager::getIdentifier(const std::string& name, const std::string& type,
                                const std::string& package) const {
    // Strip a type prefix carried inside the name ("attr/foo" → "foo"): the
    // callers pass type separately.
    std::string cleanName = name;
    const size_t slash = cleanName.find('/');
    if (slash != std::string::npos) cleanName = cleanName.substr(slash + 1);
    if (cleanName.empty()) return 0;

    std::vector<std::string> tried;
    if (!package.empty()) {
        tried.push_back(package);
        auto id = mAm2->GetResourceId(package + ":" + type + "/" + cleanName);
        if (id.has_value()) return (int)*id;
        tried.push_back("cdroid." + package);
        id = mAm2->GetResourceId("cdroid." + package + ":" + type + "/" + cleanName);
        if (id.has_value()) return (int)*id;
    }
    auto id = mAm2->GetResourceId("android:" + type + "/" + cleanName);
    if (id.has_value()) return (int)*id;

    // Any registered package (the legacy getIdentifier(name, type, "") tail).
    std::vector<std::string> packages;
    mAm2->ForEachPackage([&](const std::string& pname, uint8_t) {
        packages.push_back(pname);
        return true;
    });
    for (const auto& p : packages) {
        bool seen = false;
        for (const auto& t : tried) { if (t == p) { seen = true; break; } }
        if (seen) continue;
        auto anyId = mAm2->GetResourceId(p + ":" + type + "/" + cleanName);
        if (anyId.has_value()) return (int)*anyId;
    }
    return 0;
}

bool AssetManager::getResourceName(uint32_t id, std::string* pkg, std::string* type,
                                   std::string* key) const {
    auto name = mAm2->GetResourceName(id);
    if (!name.has_value()) return false;
    // Utf8 faces preferred; the pak pools are UTF-16, so the u16 variants are
    // the usual ones.
    if (pkg != nullptr) {
        if (name->package != nullptr) pkg->assign(name->package, name->package_len);
        else pkg->clear();
    }
    auto flatten = [](const char* s8, const char16_t* s16, size_t len, std::string* out) {
        if (s8 != nullptr) { out->assign(s8, len); return true; }
        if (s16 != nullptr) { *out = TextUtils::utf16_utf8((const uint16_t*)s16, len); return true; }
        return false;
    };
    std::string typeStr, keyStr;
    const bool haveType = flatten(name->type, name->type16, name->type_len, &typeStr);
    const bool haveKey = flatten(name->entry, name->entry16, name->entry_len, &keyStr);
    if (!haveType || !haveKey) return false;
    if (type != nullptr) *type = typeStr;
    if (key != nullptr) *key = keyStr;
    return true;
}

// Locale split by package id (legacy ResTable semantics preserved exactly):
// runtime package 0x01 is the framework ("system") set, everything else the
// app's. AOSP AM2 offers only the exclude_system flag, which keys off
// PROPERTY_SYSTEM — this split is what the callers actually consume.
static void collectLocalesByPackage(const std::vector<std::unique_ptr<ApkAssets>>& apks,
                                    bool systemSide, std::vector<std::string>* out) {
    std::set<std::string> seen;
    std::set<std::string> locales;
    for (const auto& apk : apks) {
        const LoadedArsc* arsc = apk->GetLoadedArsc();
        if (arsc == nullptr) continue;
        for (const auto& pkg : arsc->GetPackages()) {
            const bool isFramework = (pkg->GetPackageId() == 0x01);
            if (isFramework != systemSide) continue;
            pkg->CollectLocales(false /* canonicalize */, &locales);
        }
    }
    for (const auto& l : locales) {
        if (seen.insert(l).second) out->push_back(l);
    }
}

std::vector<std::string> AssetManager::getLocales() const
{
    // AOSP AssetManager.getLocales → AssetManager2::GetResourceLocales: the
    // distinct "xx-YY" configs across every registered asset path's arsc.
    const std::set<std::string> locs = mAm2->GetResourceLocales();
    return std::vector<std::string>(locs.begin(), locs.end());
}

std::vector<std::string> AssetManager::getNonSystemLocales() const
{
    std::vector<std::string> out;
    collectLocalesByPackage(mApkAssets, false, &out);
    return out;
}

std::vector<std::string> AssetManager::getSystemLocales() const
{
    std::vector<std::string> out;
    collectLocalesByPackage(mApkAssets, true, &out);
    return out;
}

bool AssetManager::isUpToDate() {
    for (const auto& ap : mAssetPaths) {
        if (ap.type == ::kFileTypeRegular && getFileModDate(ap.path.c_str()) != ap.modWhen) {
            return false;
        }
    }
    return true;
}

// Open a file by name within one asset path (directory or zip). Faithful to
// AOSP openNonAssetInPathLocked, minus the kExcludedAsset/.EXCLUDE sentinel.
Asset* AssetManager::openNonAssetInPath(const char* fileName, AccessMode mode, asset_path& ap) {
    LOGV("openNonAssetInPath: name=%s type=%d", fileName, (int)ap.type);

    if (ap.type == ::kFileTypeDirectory) {
        std::string path(ap.path);
        path += "/";
        path += fileName;

        Asset* pAsset = openAssetFromFile(path, mode);
        if (pAsset == nullptr) {
            path += ".gz";           // try again with ".gz"
            pAsset = openAssetFromFile(path, mode);
        }
        if (pAsset != nullptr) {
            LOGV("FOUND NA '%s' on disk", fileName);
            pAsset->setAssetSource(path);
        }
        return pAsset;
    }

    // Look inside the zip archive.
    zip_t* zip = getZipFile(ap);
    if (zip == nullptr) return nullptr;

    zip_int64_t entry = zip_name_locate(zip, fileName, 0);
    if (entry < 0) {
        // Unified res mode: pak entries carry the apk's "res/" prefix (older
        // paks don't) and AssetManager callers spell either form — probe the
        // path-candidate variants instead of failing the exact lookup, the
        // same bridging ResourcesImpl::openPakPath applies to arsc paths.
        std::vector<std::string> cands;
        pakPathCandidates(fileName, cands);
        for (const std::string& c : cands) {
            if (c == fileName) continue;
            entry = zip_name_locate(zip, c.c_str(), 0);
            if (entry >= 0) break;
        }
        if (entry < 0) return nullptr;
    }

    LOGV("FOUND NA in Zip file for %s", fileName);
    Asset* pAsset = openAssetFromZip(zip, entry, mode, std::string(fileName));
    if (pAsset != nullptr) {
        pAsset->setAssetSource(createZipSourceName(ap.path, std::string(), std::string(fileName)));
    }
    return pAsset;
}

std::string AssetManager::createZipSourceName(const std::string& zipFileName,
                                              const std::string& dirName,
                                              const std::string& fileName) {
    std::string sourceName = "zip:";
    sourceName += zipFileName;
    sourceName += ":";
    if (!dirName.empty()) { sourceName += dirName; sourceName += "/"; }
    sourceName += fileName;
    return sourceName;
}

std::string AssetManager::createPathName(const asset_path& ap, const char* rootDir) {
    std::string path(ap.path);
    if (rootDir != nullptr) { path += "/"; path += rootDir; }
    return path;
}

// Get (lazily opening) the libzip handle for an asset path.
struct zip* AssetManager::getZipFile(const asset_path& ap) {
    if (ap.zip != nullptr) return ap.zip;
    if (ap.zipTried) return nullptr;
    ap.zipTried = true;

    int err = 0;
    zip_t* z = zip_open(ap.path.c_str(), ZIP_RDONLY, &err);
    if (z == nullptr) {
        LOGW("Failed opening zip %s (libzip err=%d)", ap.path.c_str(), err);
        return nullptr;
    }
    ap.zip = z;
    return z;
}

Asset* AssetManager::openAssetFromFile(const std::string& pathName, AccessMode mode) {
    // .gz suffix -> gzip-compressed; otherwise a plain file.
    if (pathName.size() >= 3 &&
        strcasecmp(pathName.c_str() + pathName.size() - 3, ".gz") == 0) {
        return Asset::createFromCompressedFile(pathName.c_str(), mode);
    }
    return Asset::createFromFile(pathName.c_str(), mode);
}

// Read a zip entry into memory (libzip decompresses deflated entries) and wrap
// it as an uncompressed-buffer-backed _FileAsset. Faithful to AOSP
// openAssetFromZipLocked's observable result; IncFsFileMap replaced by a heap
// buffer (CDROID has no IncFs).
Asset* AssetManager::openAssetFromZip(struct zip* zip, int64_t entry, AccessMode mode,
                                      const std::string& entryName) {
    (void)entryName;
    zip_stat_t st;
    zip_stat_init(&st);
    if (zip_stat_index(zip, (zip_int64_t)entry, 0, &st) != 0 || !(st.valid & ZIP_STAT_SIZE)) {
        LOGW("getEntryInfo failed");
        return nullptr;
    }
    const size_t uncompressedLen = (size_t)st.size;

    zip_file_t* zf = zip_fopen_index(zip, (zip_uint64_t)entry, 0);
    if (zf == nullptr) {
        LOGW("zip_fopen_index failed");
        return nullptr;
    }

    unsigned char* buf = new (std::nothrow) unsigned char[uncompressedLen ? uncompressedLen : 1];
    if (buf == nullptr) {
        zip_fclose(zf);
        return nullptr;
    }
    size_t got = 0;
    while (got < uncompressedLen) {
        ssize_t n = zip_fread(zf, buf + got, uncompressedLen - got);
        if (n <= 0) break;
        got += (size_t)n;
    }
    zip_fclose(zf);
    if (got != uncompressedLen) {
        LOGW("short read of zip entry %s", entryName.c_str());
        delete[] buf;
        return nullptr;
    }

    std::unique_ptr<Asset> pAsset = Asset::createFromUncompressedBuffer(buf, uncompressedLen, mode, true);
    if (pAsset == nullptr) {
        delete[] buf;   // createFromUncompressedBuffer only fails on bad args; be safe
        return nullptr;
    }
    return pAsset.release();
}

// ===========================================================================
// Directory listing (openDir / openNonAssetDir)
// ===========================================================================

AssetDir* AssetManager::openDir(const char* dirName) {
    FATAL_IF(mAssetPaths.empty(), "No assets added to AssetManager");
    assert(dirName != nullptr);

    AssetDir* pDir = new AssetDir;
    std::vector<AssetDir::FileInfo>* merged = new std::vector<AssetDir::FileInfo>;

    // Reverse priority order, zip archives first then loose files (matches AOSP).
    for (size_t i = mAssetPaths.size(); i > 0; ) {
        --i;
        asset_path& ap = mAssetPaths[i];
        if (ap.type == ::kFileTypeRegular) {
            scanAndMergeZip(merged, ap, kAssetsRoot, dirName);
        } else {
            scanAndMergeDir(merged, ap, kAssetsRoot, dirName);
        }
    }
    std::sort(merged->begin(), merged->end());
    pDir->setFileList(merged);
    return pDir;
}

AssetDir* AssetManager::openNonAssetDir(int32_t cookie, const char* dirName) {
    FATAL_IF(mAssetPaths.empty(), "No assets added to AssetManager");
    assert(dirName != nullptr);

    AssetDir* pDir = new AssetDir;
    std::vector<AssetDir::FileInfo>* merged = new std::vector<AssetDir::FileInfo>;

    const size_t which = (size_t)cookie - 1;
    if (which < mAssetPaths.size()) {
        asset_path& ap = mAssetPaths[which];
        if (ap.type == ::kFileTypeRegular) {
            scanAndMergeZip(merged, ap, nullptr, dirName);
        } else {
            scanAndMergeDir(merged, ap, nullptr, dirName);
        }
    }
    std::sort(merged->begin(), merged->end());
    pDir->setFileList(merged);
    return pDir;
}

bool AssetManager::scanAndMergeDir(std::vector<AssetDir::FileInfo>* merged,
                                   const asset_path& ap, const char* rootDir, const char* dirName) {
    std::string path = createPathName(ap, rootDir);
    if (dirName[0] != '\0') { path += "/"; path += dirName; }

    std::vector<AssetDir::FileInfo> contents;
    if (!scanDirInto(path, &contents)) return false;

    // .EXCLUDE directive processing (trimmed in this port: no kExcludedAsset
    // sentinel). Files keep their names; ".gz" suffixes are stripped.
    mergeInfo(merged, &contents);
    return true;
}

// Scan a filesystem directory into a sorted FileInfo vector. .gz suffix removed.
bool AssetManager::scanDirInto(const std::string& path, std::vector<AssetDir::FileInfo>* out) {
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) return false;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        ::FileType fileType;
#ifdef _DIRENT_HAVE_D_TYPE
        if (entry->d_type == DT_REG) fileType = ::kFileTypeRegular;
        else if (entry->d_type == DT_DIR) fileType = ::kFileTypeDirectory;
        else fileType = ::kFileTypeUnknown;
#else
        std::string p = path + "/" + entry->d_name;
        fileType = ::getFileType(p.c_str());
#endif
        if (fileType != ::kFileTypeRegular && fileType != ::kFileTypeDirectory) continue;

        AssetDir::FileInfo info;
        std::string name(entry->d_name);
        if (name.size() >= 3 && strcasecmp(name.c_str() + name.size() - 3, ".gz") == 0) {
            name.erase(name.size() - 3);
        }
        info.set(name, fileType);
        info.setSourceName(path + "/" + name);
        out->push_back(info);
    }
    closedir(dir);
    std::sort(out->begin(), out->end());
    return true;
}

// Scan a zip archive's entries under baseDirName (rootDir prefix), inferring
// subdirectories from path context. Faithful to AOSP scanAndMergeZipLocked;
// libzip iteration replaces ZipFileRO::startIteration/nextEntry.
bool AssetManager::scanAndMergeZip(std::vector<AssetDir::FileInfo>* merged,
                                   const asset_path& ap, const char* rootDir, const char* baseDirName) {
    zip_t* zip = getZipFile(ap);
    if (zip == nullptr) {
        LOGW("Failure opening zip %s", ap.path.c_str());
        return false;
    }

    std::string dirName;
    if (rootDir != nullptr) dirName = rootDir;
    if (baseDirName[0] != '\0') {
        if (!dirName.empty()) dirName += "/";
        dirName += baseDirName;
    }
    const size_t dirNameLen = dirName.size();

    std::vector<AssetDir::FileInfo> contents;
    std::vector<std::string> dirs;
    const std::string zipName = ap.path;

    const zip_int64_t num = zip_get_num_entries(zip, 0);
    for (zip_int64_t i = 0; i < num; i++) {
        const char* nameC = zip_get_name(zip, i, ZIP_FL_ENC_GUESS);
        if (nameC == nullptr) continue;
        std::string nameBuf(nameC);

        if (dirNameLen == 0 || (nameBuf.size() > dirNameLen && nameBuf[dirNameLen] == '/')) {
            size_t cp = dirNameLen;
            if (dirNameLen != 0) cp++;   // advance past the '/'

            // First '/' after the prefix marks a subdirectory; no '/' => a file.
            std::string rest = (cp < nameBuf.size()) ? nameBuf.substr(cp) : std::string();
            size_t slash = rest.find('/');
            if (slash == std::string::npos) {
                AssetDir::FileInfo info;
                info.set(rest, ::kFileTypeRegular);
                info.setSourceName(createZipSourceName(zipName, dirName, rest));
                contents.push_back(info);
            } else {
                std::string subdirName = rest.substr(0, slash);
                bool have = false;
                for (const auto& d : dirs) { if (d == subdirName) { have = true; break; } }
                if (!have) dirs.push_back(subdirName);
            }
        }
    }

    for (const auto& d : dirs) {
        AssetDir::FileInfo info;
        info.set(d, ::kFileTypeDirectory);
        info.setSourceName(createZipSourceName(zipName, dirName, d));
        contents.push_back(info);
    }

    mergeInfo(merged, &contents);
    return true;
}

// Merge a sorted `contents` vector into the sorted `merged` vector; on equal
// names the contents (newer) entry wins. Faithful to AOSP mergeInfoLocked.
void AssetManager::mergeInfo(std::vector<AssetDir::FileInfo>* merged,
                             const std::vector<AssetDir::FileInfo>* contents) {
    std::vector<AssetDir::FileInfo> result;
    size_t mergeIdx = 0, contIdx = 0;
    const size_t mergeMax = merged->size();
    const size_t contMax = contents->size();

    while (mergeIdx < mergeMax || contIdx < contMax) {
        if (mergeIdx == mergeMax) {
            result.push_back(contents->at(contIdx++));
        } else if (contIdx == contMax) {
            result.push_back(merged->at(mergeIdx++));
        } else if (merged->at(mergeIdx) == contents->at(contIdx)) {
            result.push_back(contents->at(contIdx++));   // identical: newer wins
            mergeIdx++;
        } else if (merged->at(mergeIdx) < contents->at(contIdx)) {
            result.push_back(merged->at(mergeIdx++));
        } else {
            result.push_back(contents->at(contIdx++));
        }
    }
    merged->swap(result);
}
