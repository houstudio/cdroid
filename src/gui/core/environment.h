#ifndef __ENVIROMENT_H__
#define __ENVIROMENT_H__
#include <string>
#include <vector>
#include <memory>
namespace cdroid{
class Environment {
public:
    class UserEnvironment;
    static constexpr const char*const MEDIA_UNKNOWN = "unknown";
    static constexpr const char*const MEDIA_REMOVED = "removed";
    static constexpr const char*const MEDIA_UNMOUNTED = "unmounted";
    static constexpr const char*const MEDIA_CHECKING = "checking";
    static constexpr const char*const MEDIA_NOFS = "nofs";
    static constexpr const char*const MEDIA_MOUNTED = "mounted";
    static constexpr const char*const MEDIA_MOUNTED_READ_ONLY = "mounted_ro";
    static constexpr const char*const MEDIA_SHARED = "shared";
    static constexpr const char*const MEDIA_BAD_REMOVAL = "bad_removal";
    static constexpr const char*const MEDIA_UNMOUNTABLE = "unmountable";
    static constexpr const char*const MEDIA_EJECTING = "ejecting";
    /**
     * List of standard storage directories.
     */
    static const std::vector<std::string> STANDARD_DIRECTORIES;

    static constexpr const char*const DIRECTORY_ANDROID = "Android";
    static constexpr const char*const DIRECTORY_MUSIC = "Music";
    static constexpr const char*const DIRECTORY_PODCASTS = "Podcasts";
    static constexpr const char*const DIRECTORY_RINGTONES = "Ringtones";
    static constexpr const char*const DIRECTORY_ALARMS = "Alarms";
    static constexpr const char*const DIRECTORY_NOTIFICATIONS = "Notifications";
    static constexpr const char*const DIRECTORY_PICTURES = "Pictures";
    static constexpr const char*const DIRECTORY_MOVIES = "Movies";
    static constexpr const char*const DIRECTORY_DOWNLOADS = "Download";
    static constexpr const char*const DIRECTORY_DCIM = "DCIM";
    static constexpr const char*const DIRECTORY_DOCUMENTS = "Documents";

    static constexpr int HAS_MUSIC = 1 << 0;
    static constexpr int HAS_PODCASTS = 1 << 1;
    static constexpr int HAS_RINGTONES = 1 << 2;
    static constexpr int HAS_ALARMS = 1 << 3;
    static constexpr int HAS_NOTIFICATIONS = 1 << 4;
    static constexpr int HAS_PICTURES = 1 << 5;
    static constexpr int HAS_MOVIES = 1 << 6;
    static constexpr int HAS_DOWNLOADS = 1 << 7;
    static constexpr int HAS_DCIM = 1 << 8;
    static constexpr int HAS_DOCUMENTS = 1 << 9;

    static constexpr int HAS_ANDROID = 1 << 16;
    static constexpr int HAS_OTHER = 1 << 17;
private:
    static constexpr const char*const ENV_EXTERNAL_STORAGE = "EXTERNAL_STORAGE";
    static constexpr const char*const ENV_ANDROID_ROOT = "ANDROID_ROOT";
    static constexpr const char*const ENV_ANDROID_DATA = "ANDROID_DATA";
    static constexpr const char*const ENV_ANDROID_EXPAND = "ANDROID_EXPAND";
    static constexpr const char*const ENV_ANDROID_STORAGE = "ANDROID_STORAGE";
    static constexpr const char*const ENV_DOWNLOAD_CACHE = "DOWNLOAD_CACHE";
    static constexpr const char*const ENV_OEM_ROOT = "OEM_ROOT";
    static constexpr const char*const ENV_ODM_ROOT = "ODM_ROOT";
    static constexpr const char*const ENV_VENDOR_ROOT = "VENDOR_ROOT";
    static constexpr const char*const ENV_PRODUCT_ROOT = "PRODUCT_ROOT";

    static constexpr const char*const DIR_DATA = "data";
    static constexpr const char*const DIR_MEDIA = "media";
    static constexpr const char*const DIR_OBB = "obb";
    static constexpr const char*const DIR_FILES = "files";
    static constexpr const char*const DIR_CACHE = "cache";

    static UserEnvironment sCurrentUser;
    static bool sUserRequired;
private:
    static std::string getDataProfilesDeDirectory(int userId);
    static bool hasInterestingFiles(const std::string& dir);

    static bool isInterestingFile(const std::string& file);
    static void throwIfUserRequired();
public:
    static constexpr const char*const DIR_ANDROID = "Android";
    static void initForCurrentUser();
    static std::string getRootDirectory();
    static std::string getStorageDirectory();
    static std::string getOemDirectory();

    static std::string getOdmDirectory();
    static std::string getVendorDirectory();
    static std::string getProductDirectory();
    static std::string getDataDirectory();

    static std::string getDataDirectory(const std::string& volumeUuid);
    static std::string getExpandDirectory();

    static std::string getDataSystemDirectory();

    static std::string getDataSystemDeDirectory();

    static std::string getDataSystemCeDirectory();

    static std::string getDataSystemCeDirectory(int userId);

    static std::string getDataSystemDeDirectory(int userId);

    static std::string getDataMiscDirectory();

    static std::string getDataMiscCeDirectory();

    static std::string getDataMiscCeDirectory(int userId);

    static std::string getDataMiscDeDirectory(int userId);

    static std::string getDataVendorCeDirectory(int userId);

    static std::string getDataVendorDeDirectory(int userId);

    static std::string getDataRefProfilesDePackageDirectory(const std::string& packageName);

    static std::string getDataProfilesDePackageDirectory(int userId, const std::string& packageName);

    static std::string getDataAppDirectory(const std::string& volumeUuid);

    static std::string getDataUserCeDirectory(const std::string& volumeUuid);

    static std::string getDataUserCeDirectory(const std::string& volumeUuid, int userId);

    static std::string getDataUserCePackageDirectory(const std::string& volumeUuid, int userId,const std::string& packageName);

    static std::string getDataUserDeDirectory(const std::string& volumeUuid);

    static std::string getDataUserDeDirectory(const std::string& volumeUuid, int userId);

    static std::string getDataUserDePackageDirectory(const std::string& volumeUuid, int userId, const std::string& packageName);

    static std::string getDataPreloadsDirectory();

    static std::string getDataPreloadsDemoDirectory();

    static std::string getDataPreloadsAppsDirectory();

    static std::string getDataPreloadsMediaDirectory();

    static std::string getDataPreloadsFileCacheDirectory(const std::string& packageName);

    static std::string getDataPreloadsFileCacheDirectory();

    static std::string getExternalStorageDirectory();

    static std::string getLegacyExternalStorageDirectory();

    static std::string getLegacyExternalStorageObbDirectory();

    static bool isStandardDirectory(const std::string& dir);

    static int classifyExternalStorageDirectory(const std::string& dir);

    static std::string getExternalStoragePublicDirectory(const std::string& type);

    static std::vector<std::string> buildExternalStorageAndroidDataDirs();

    static std::vector<std::string> buildExternalStorageAppDataDirs(const std::string& packageName);

    static std::vector<std::string> buildExternalStorageAppMediaDirs(const std::string& packageName);

    static std::vector<std::string> buildExternalStorageAppObbDirs(const std::string& packageName);

    static std::vector<std::string> buildExternalStorageAppFilesDirs(const std::string& packageName);

    static std::vector<std::string> buildExternalStorageAppCacheDirs(const std::string& packageName);

    static std::string getDownloadCacheDirectory();

    static const std::string  getExternalStorageState();

    static const std::string getStorageState(const std::string& path);

    static const std::string getExternalStorageState(const std::string& path);

    static bool isExternalStorageRemovable();

    static bool isExternalStorageRemovable(const std::string& path);

    static bool isExternalStorageEmulated();

    static bool isExternalStorageEmulated(const std::string& path);

    static std::string getDirectory(const std::string& variableName, const std::string& defaultPath);

    static void setUserRequired(bool userRequired);

    static std::vector<std::string> buildPaths(const std::vector<std::string>& base,const std::vector<std::string>&segments);

    static std::string buildPath(const std::string& base, const std::vector<std::string>&segments);

    static std::string maybeTranslateEmulatedPathToInternal(std::string& path);
};

class Environment::UserEnvironment {
private:
    const int mUserId;
public:
    UserEnvironment(int userId);

    std::vector<std::string> getExternalDirs();

    std::vector<std::string> buildExternalStoragePublicDirs(const std::string& type);

    std::vector<std::string> buildExternalStorageAndroidDataDirs();

    std::vector<std::string> buildExternalStorageAndroidObbDirs();

    std::vector<std::string> buildExternalStorageAppDataDirs(const std::string& packageName);

    std::vector<std::string> buildExternalStorageAppMediaDirs(const std::string& packageName);

    std::vector<std::string> buildExternalStorageAppObbDirs(const std::string& packageName);

    std::vector<std::string> buildExternalStorageAppFilesDirs(const std::string& packageName);

    std::vector<std::string> buildExternalStorageAppCacheDirs(const std::string& packageName);
};

}/*endof namespace*/
#endif/*__ENVIROMENT_H__*/
