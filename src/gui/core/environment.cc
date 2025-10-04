#include <core/environment.h>
#include <utils/textutils.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
namespace cdroid{
#if 0
const std::string Environment::DIR_ANDROID_ROOT = getDirectory(ENV_ANDROID_ROOT, "/system");
const std::string Environment::DIR_ANDROID_DATA = getDirectory(ENV_ANDROID_DATA, "/data");
const std::string Environment::DIR_ANDROID_EXPAND = getDirectory(ENV_ANDROID_EXPAND, "/mnt/expand");
const std::string Environment::DIR_ANDROID_STORAGE = getDirectory(ENV_ANDROID_STORAGE, "/storage");
const std::string Environment::DIR_DOWNLOAD_CACHE = getDirectory(ENV_DOWNLOAD_CACHE, "/cache");
const std::string Environment::DIR_OEM_ROOT = getDirectory(ENV_OEM_ROOT, "/oem");
const std::string Environment::DIR_ODM_ROOT = getDirectory(ENV_ODM_ROOT, "/odm");
const std::string Environment::DIR_VENDOR_ROOT = getDirectory(ENV_VENDOR_ROOT, "/vendor");
const std::string Environment::DIR_PRODUCT_ROOT = getDirectory(ENV_PRODUCT_ROOT, "/product");
#endif
Environment::UserEnvironment Environment::sCurrentUser(-1);
bool Environment::sUserRequired=false;
void Environment::initForCurrentUser(int userId) {
    if(userId==-1)userId= getuid();
    sCurrentUser.mUserId = userId;
}

std::string Environment::getRootDirectory() {
    return getDirectory(ENV_ANDROID_ROOT, "/system");//DIR_ANDROID_ROOT;
}

std::string Environment::getStorageDirectory() {
    return getDirectory(ENV_ANDROID_STORAGE, "/storage");//DIR_ANDROID_STORAGE;
}

std::string Environment::getOemDirectory() {
    return getDirectory(ENV_OEM_ROOT, "/oem");//DIR_OEM_ROOT;
}

std::string Environment::getOdmDirectory() {
    return getDirectory(ENV_ODM_ROOT, "/odm");//DIR_ODM_ROOT;
}

std::string Environment::getVendorDirectory() {
    return getDirectory(ENV_VENDOR_ROOT, "/vendor");//DIR_VENDOR_ROOT;
}

std::string Environment::getProductDirectory() {
    return getDirectory(ENV_PRODUCT_ROOT, "/product");//DIR_PRODUCT_ROOT;
}

std::string Environment::getDataDirectory() {
    return getDirectory(ENV_ANDROID_DATA, "/data");//DIR_ANDROID_DATA;
}

std::string Environment::getDataDirectory(const std::string& volumeUuid) {
    if (TextUtils::isEmpty(volumeUuid)) {
        return getDataDirectory();//DIR_ANDROID_DATA;
    } else {
        return std::string("/mnt/expand/") + volumeUuid;
    }
}

std::string Environment::getExpandDirectory() {
    return getDirectory(ENV_ANDROID_EXPAND, "/mnt/expand");//DIR_ANDROID_EXPAND;
}

std::string Environment::getDataSystemDirectory() {
    return getDataDirectory()+"/system";
}

std::string Environment::getDataSystemDeDirectory() {
    return buildPath(getDataDirectory(), {"system_de"});
}

std::string Environment::getDataSystemCeDirectory() {
    return buildPath(getDataDirectory(), {"system_ce"});
}

std::string Environment::getDataSystemCeDirectory(int userId) {
    return buildPath(getDataDirectory(), {"system_ce", std::to_string(userId)});
}

std::string Environment::getDataSystemDeDirectory(int userId) {
    return buildPath(getDataDirectory(), {"system_de", std::to_string(userId)});
}

std::string Environment::getDataMiscDirectory() {
    return getDataDirectory()+"/misc";
}

std::string Environment::getDataMiscCeDirectory() {
    return buildPath(getDataDirectory(), {"misc_ce"});
}

std::string Environment::getDataMiscCeDirectory(int userId) {
    return buildPath(getDataDirectory(), {"misc_ce", std::to_string(userId)});
}

std::string Environment::getDataMiscDeDirectory(int userId) {
    return buildPath(getDataDirectory(), {"misc_de", std::to_string(userId)});
}

std::string Environment::getDataProfilesDeDirectory(int userId) {
    return buildPath(getDataDirectory(), {"misc", "profiles", "cur", std::to_string(userId)});
}

std::string Environment::getDataVendorCeDirectory(int userId) {
    return buildPath(getDataDirectory(), {"vendor_ce", std::to_string(userId)});
}

std::string Environment::getDataVendorDeDirectory(int userId) {
    return buildPath(getDataDirectory(), {"vendor_de", std::to_string(userId)});
}

std::string Environment::getDataRefProfilesDePackageDirectory(const std::string& packageName) {
    return buildPath(getDataDirectory(), {"misc", "profiles", "ref", packageName});
}

std::string Environment::getDataProfilesDePackageDirectory(int userId, const std::string& packageName) {
    return buildPath(getDataProfilesDeDirectory(userId), {packageName});
}

std::string Environment::getDataAppDirectory(const std::string& volumeUuid) {
    return getDataDirectory(volumeUuid)+"/app";
}

std::string Environment::getDataUserCeDirectory(const std::string& volumeUuid) {
    return getDataDirectory(volumeUuid)+"/user";
}

std::string Environment::getDataUserCeDirectory(const std::string& volumeUuid, int userId) {
    return getDataUserCeDirectory(volumeUuid)+std::to_string(userId);
}

std::string Environment::getDataUserCePackageDirectory(const std::string& volumeUuid, int userId, const std::string& packageName) {
    // TODO: keep consistent with installd
    return getDataUserCeDirectory(volumeUuid, userId)+"/"+packageName;
}

std::string Environment::getDataUserDeDirectory(const std::string& volumeUuid) {
    return getDataDirectory(volumeUuid)+ "/user_de";
}

std::string Environment::getDataUserDeDirectory(const std::string& volumeUuid, int userId) {
    return getDataUserDeDirectory(volumeUuid)+"/"+std::to_string(userId);
}

std::string Environment::getDataUserDePackageDirectory(const std::string& volumeUuid, int userId, const std::string& packageName) {
    // TODO: keep consistent with installd
    return getDataUserDeDirectory(volumeUuid, userId)+"/"+packageName;
}

std::string Environment::getDataPreloadsDirectory() {
    return getDataDirectory()+"/preloads";
}

std::string Environment::getDataPreloadsDemoDirectory() {
    //return File(getDataPreloadsDirectory(), "demo");
    return buildPath(getDataPreloadsDirectory(),{"demo"});
}

std::string Environment::getDataPreloadsAppsDirectory() {
    return getDataPreloadsDirectory()+"/apps";
}

std::string Environment::getDataPreloadsMediaDirectory() {
    return getDataPreloadsDirectory()+"/media";
}

std::string Environment::getDataPreloadsFileCacheDirectory(const std::string& packageName) {
    return getDataPreloadsFileCacheDirectory()+"/"+packageName;
}

std::string Environment::getDataPreloadsFileCacheDirectory() {
    return getDataPreloadsDirectory()+"/file_cache";
}

std::string Environment::getExternalStorageDirectory() {
    throwIfUserRequired();
    return sCurrentUser.getExternalDirs()[0];
}

std::string Environment::getLegacyExternalStorageDirectory() {
    return getenv(ENV_EXTERNAL_STORAGE);
}

std::string Environment::getLegacyExternalStorageObbDirectory() {
    return buildPath(getLegacyExternalStorageDirectory(),{DIR_ANDROID,DIR_OBB});
}

/**
 * List of standard storage directories.
 */
const std::vector<std::string> Environment::STANDARD_DIRECTORIES = {
        DIRECTORY_MUSIC,
        DIRECTORY_PODCASTS,
        DIRECTORY_RINGTONES,
        DIRECTORY_ALARMS,
        DIRECTORY_NOTIFICATIONS,
        DIRECTORY_PICTURES,
        DIRECTORY_MOVIES,
        DIRECTORY_DOWNLOADS,
        DIRECTORY_DCIM,
        DIRECTORY_DOCUMENTS
};

bool Environment::isStandardDirectory(const std::string& dir) {
    for (const std::string& valid : STANDARD_DIRECTORIES) {
        if (valid==dir) {
            return true;
        }
    }
    return false;
}

int Environment::classifyExternalStorageDirectory(const std::string& dir) {
    int res = 0;
    struct dirent* entry = nullptr;
    DIR* dp = opendir(dir.c_str());
    if(dp==nullptr)return 0;
    while((entry = readdir(dp))!= nullptr){
        if (entry->d_name[0] == '.') continue; 
        struct stat statbuf;
        std::string fullPath = dir + "/" + entry->d_name;
        if (stat(fullPath.c_str(), &statbuf) == -1) {
            std::cerr << "Could not stat file: " << fullPath << std::endl;
            continue;
        }

        if (S_ISREG(statbuf.st_mode) && isInterestingFile(fullPath)) {
            res |= HAS_OTHER;
        } else if (S_ISDIR(statbuf.st_mode) && hasInterestingFiles(fullPath)) {
            const std::string name = entry->d_name;
            if (name == DIRECTORY_MUSIC) res |= HAS_MUSIC;
            else if (name == DIRECTORY_PODCASTS) res |= HAS_PODCASTS;
            else if (name == DIRECTORY_RINGTONES) res |= HAS_RINGTONES;
            else if (name == DIRECTORY_ALARMS) res |= HAS_ALARMS;
            else if (name == DIRECTORY_NOTIFICATIONS) res |= HAS_NOTIFICATIONS;
            else if (name == DIRECTORY_PICTURES) res |= HAS_PICTURES;
            else if (name == DIRECTORY_MOVIES) res |= HAS_MOVIES;
            else if (name == DIRECTORY_DOWNLOADS) res |= HAS_DOWNLOADS;
            else if (name == DIRECTORY_DCIM) res |= HAS_DCIM;
            else if (name == DIRECTORY_DOCUMENTS) res |= HAS_DOCUMENTS;
            else if (name == DIRECTORY_ANDROID) res |= HAS_ANDROID;
            else res |= HAS_OTHER;
        }
    }
    closedir(dp);
    return res;
}

bool Environment::hasInterestingFiles(const std::string& dir) {
#if 0
    final LinkedList<File> explore = new LinkedList<>();
    explore.add(dir);
    while (!explore.isEmpty()) {
        dir = explore.pop();
        for (File f : FileUtils.listFilesOrEmpty(dir)) {
            if (isInterestingFile(f)) return true;
            if (f.isDirectory()) explore.add(f);
        }
    }
#endif
    return false;
}

bool Environment::isInterestingFile(const std::string& file) {
    /*if (file.isFile()) {
        const std::string name = file.getName().toLowerCase();
        if (name.endsWith(".exe") || name.equals("autorun.inf")
                || name.equals("launchpad.zip") || name.equals(".nomedia")) {
            return false;
        } else {
            return true;
        }
    } else */{
        return false;
    }
}

std::string Environment::getExternalStoragePublicDirectory(const std::string& type) {
    throwIfUserRequired();
    return std::move(sCurrentUser.buildExternalStoragePublicDirs(type)[0]);
}

std::vector<std::string> Environment::buildExternalStorageAndroidDataDirs() {
    throwIfUserRequired();
    return sCurrentUser.buildExternalStorageAndroidDataDirs();
}

std::vector<std::string> Environment::buildExternalStorageAppDataDirs(const std::string& packageName) {
    throwIfUserRequired();
    return sCurrentUser.buildExternalStorageAppDataDirs(packageName);
}

std::vector<std::string> Environment::buildExternalStorageAppMediaDirs(const std::string& packageName) {
    throwIfUserRequired();
    return sCurrentUser.buildExternalStorageAppMediaDirs(packageName);
}

std::vector<std::string> Environment::buildExternalStorageAppObbDirs(const std::string& packageName) {
    throwIfUserRequired();
    return sCurrentUser.buildExternalStorageAppObbDirs(packageName);
}

std::vector<std::string> Environment::buildExternalStorageAppFilesDirs(const std::string& packageName) {
    throwIfUserRequired();
    return sCurrentUser.buildExternalStorageAppFilesDirs(packageName);
}

std::vector<std::string> Environment::buildExternalStorageAppCacheDirs(const std::string& packageName) {
    throwIfUserRequired();
    return sCurrentUser.buildExternalStorageAppCacheDirs(packageName);
}

std::string Environment::getDownloadCacheDirectory() {
    return getDirectory(ENV_DOWNLOAD_CACHE, "/cache");//DIR_DOWNLOAD_CACHE;
}

const std::string Environment::getExternalStorageState() {
    return getExternalStorageState(sCurrentUser.getExternalDirs()[0]);
}

const std::string Environment::getStorageState(const std::string& path) {
    return getExternalStorageState(path);
}

const std::string Environment::getExternalStorageState(const std::string& path) {
    /*StorageVolume volume = StorageManager.getStorageVolume(path, UserHandle.myUserId());
    if (volume != null) {
        return volume.getState();
    } else */{
        return MEDIA_UNKNOWN;
    }
}

bool Environment::isExternalStorageRemovable() {
    return isExternalStorageRemovable(sCurrentUser.getExternalDirs()[0]);
}

bool Environment::isExternalStorageRemovable(const std::string& path) {
    /*final StorageVolume volume = StorageManager.getStorageVolume(path, UserHandle.myUserId());
    if (volume != null) {
        return volume.isRemovable();
    } else*/{
        throw std::logic_error(std::string("Failed to find storage device at ") + path);
    }
}

bool Environment::isExternalStorageEmulated() {
    return isExternalStorageEmulated(sCurrentUser.getExternalDirs()[0]);
}

bool Environment::isExternalStorageEmulated(const std::string& path) {
    /*StorageVolume volume = StorageManager.getStorageVolume(path, UserHandle.myUserId());
    if (volume != null) {
        return volume.isEmulated();
    } else */{
        throw std::logic_error("Failed to find storage device at " + path);
    }
}

std::string Environment::getDirectory(const std::string& variableName, const std::string& defaultPath) {
    const char* path = getenv(variableName.c_str());
    return (path==nullptr)?defaultPath:path;
}

void Environment::setUserRequired(bool userRequired) {
    sUserRequired = userRequired;
}

void Environment::throwIfUserRequired() {
    if (sUserRequired) {
        throw std::logic_error("Path requests must specify a user by using UserEnvironment");
    }
}

std::vector<std::string> Environment::buildPaths(const std::vector<std::string>& base,const std::vector<std::string>&segments) {
    std::vector<std::string> result;// = new File[base.length];
    for (int i = 0; i < base.size(); i++) {
        result.push_back(buildPath(base[i], segments));
    }
    return result;
}

std::string Environment::buildPath(const std::string& base,const std::vector<std::string>&segments) {
    std::string cur =base;
    for (std::string segment : segments) {
        if (cur.empty()) {
            cur = segment;
        } else {
            cur = cur+"/"+segment;
        }
    }
    return cur;
}

std::string Environment::maybeTranslateEmulatedPathToInternal(std::string& path) {
    return nullptr;//StorageManager.maybeTranslateEmulatedPathToInternal(path);
}

///////////////////////////////////////////////////////////////////////////
Environment::UserEnvironment::UserEnvironment(int userId)
    :mUserId (userId){
    struct passwd *pwd = nullptr;
    if(userId==-1){
        mUserId=getuid();
        pwd = getpwuid(userId);
    }
    LOGD("userId=%d %s",mUserId,(pwd?pwd->pw_name:"null"));
}

std::vector<std::string> Environment::UserEnvironment::getExternalDirs() {
    //StorageVolume[] volumes = StorageManager.getVolumeList(mUserId,StorageManager.FLAG_FOR_WRITE);
    std::vector<std::string> files;// = new File[volumes.length];
    /*for (int i = 0; i < volumes.length; i++) {
        files[i] = volumes[i].getPathFile();
    }*/
    return files;
}

std::vector<std::string> Environment::UserEnvironment::buildExternalStoragePublicDirs(const std::string& type) {
    return buildPaths(getExternalDirs(), {type});
}

std::vector<std::string> Environment::UserEnvironment::buildExternalStorageAndroidDataDirs() {
    return buildPaths(getExternalDirs(), {DIR_ANDROID, DIR_DATA});
}

std::vector<std::string> Environment::UserEnvironment::buildExternalStorageAndroidObbDirs() {
    return buildPaths(getExternalDirs(), {DIR_ANDROID, DIR_OBB});
}

std::vector<std::string> Environment::UserEnvironment::buildExternalStorageAppDataDirs(const std::string& packageName) {
    return buildPaths(getExternalDirs(), {DIR_ANDROID, DIR_DATA, packageName});
}

std::vector<std::string> Environment::UserEnvironment::buildExternalStorageAppMediaDirs(const std::string& packageName) {
    return buildPaths(getExternalDirs(), {DIR_ANDROID, DIR_MEDIA, packageName});
}

std::vector<std::string> Environment::UserEnvironment::buildExternalStorageAppObbDirs(const std::string& packageName) {
    return buildPaths(getExternalDirs(), {DIR_ANDROID, DIR_OBB, packageName});
}

std::vector<std::string> Environment::UserEnvironment::buildExternalStorageAppFilesDirs(const std::string& packageName) {
    return buildPaths(getExternalDirs(), {DIR_ANDROID, DIR_DATA, packageName, DIR_FILES});
}

std::vector<std::string> Environment::UserEnvironment::buildExternalStorageAppCacheDirs(const std::string& packageName) {
    return buildPaths(getExternalDirs(), {DIR_ANDROID, DIR_DATA, packageName, DIR_CACHE});
}
////////////////////////////////////////////////////////////////////////////////////
}/*endof namespace*/
