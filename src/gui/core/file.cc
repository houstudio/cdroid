#include <core/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <fstream>
namespace cdroid{

File::File(const std::string& pathname) {
    initialize(pathname);
}

File::File(const std::string& parent, const std::string& child) {
    initialize(parent + separator/*"/"*/ + child);
}

File::File(const File* parent, const std::string& child) {
    if(parent==nullptr)initialize(child);
    else initialize(parent->getPath() + separator/*"/"*/ + child);
}

void File::initialize(const std::string& pathname) {
    path = pathname;
    size_t pos = path.find_last_of(separatorChar/*'/'*/);
    if (pos != std::string::npos) {
        parent = path.substr(0, pos);
        name = path.substr(pos + 1);
    } else {
        parent = "";
        name = path;
    }
}

std::string File::getName() const {
    return name;
}

std::string File::getParent() const {
    return parent;
}

std::string File::getPath() const {
    return path;
}

std::string File::getAbsolutePath() const{
    char cwd[PATH_MAX];
    if (path.empty()) {
        return getCurrentDirectory();
    }
    if (path[0] == '/') {
        return path;
    }
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        return std::string(cwd) + "/" + path;
    } else {
        return path;
    }
}

std::string File::getCanonicalPath() const{
    char resolvedPath[PATH_MAX];
    if(realpath(path.c_str(), resolvedPath))
        return path;
    return std::string(resolvedPath);    
}

std::string File::getCurrentDirectory() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        return std::string(cwd);
    } else {
        return "";
    }
}

bool File::exists() const {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool File::isDirectory() const {
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0) return false;
    return S_ISDIR(buffer.st_mode);
}

bool File::isFile() const {
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0) return false;
    return S_ISREG(buffer.st_mode);
}

bool File::canRead() const {
    return (access(path.c_str(), R_OK) == 0);
}

bool File::canWrite() const {
    return (access(path.c_str(), W_OK) == 0);
}

bool File::canExecute() const {
    return (access(path.c_str(), X_OK) == 0);
}

bool File::createNewFile() {
    if (exists()) return false;
    std::ofstream file(path);
    bool created = file.good();
    file.close();
    return created;
}

bool File::deleteFile() {
    return (remove(path.c_str()) == 0);
}

std::vector<File> File::listFiles() const {
    std::vector<File> files;
    if (!isDirectory()) return files;

    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) return files;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            files.emplace_back(path, entry->d_name);
        }
    }
    closedir(dir);
    return files;
}

bool File::mkdir() {
    return (::mkdir(path.c_str(), 0755) == 0);
}

bool File::mkdirs() {
    if (exists()) return false;
    if (::mkdir(path.c_str(), 0755) == 0) return true;

    size_t pos = path.find_last_of(separatorChar/*'/'*/);
    if (pos == std::string::npos) return false;

    File parentDir(path.substr(0, pos));
    if (!parentDir.mkdirs()) return false;

    return ::mkdir(path.c_str(), 0755) == 0;
}

bool File::renameTo(const File& dest) {
    return (rename(path.c_str(), dest.getPath().c_str()) == 0);
}

long File::length() const {
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0) return 0;
    return buffer.st_size;
}
}
