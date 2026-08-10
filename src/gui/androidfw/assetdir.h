// Port of AOSP frameworks/base/libs/androidfw/include/androidfw/AssetDir.h.
//
// Vector-style access to a slice of the asset hierarchy. Faithful public API;
// String8 -> std::string, SortedVector -> a sorted std::vector. Lives in
// namespace android (isolated from cdroid::Assets).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
#ifndef __CDROID_ANDROIDFW_ASSETDIR_H__
#define __CDROID_ANDROIDFW_ASSETDIR_H__

#include <sys/types.h>
#include <string>
#include <vector>

#include "misc.h"

namespace android {

// Access a chunk of the asset hierarchy as if it were a single directory. The
// list of files is sorted ascending by ASCII name. Populated by AssetManager.
class AssetDir {
public:
    AssetDir() {}
    virtual ~AssetDir() {}

    // Vector-style access.
    size_t getFileCount() { return mFileInfo.size(); }
    const std::string& getFileName(int idx) { return mFileInfo[idx].getFileName(); }
    const std::string& getSourceName(int idx) { return mFileInfo[idx].getSourceName(); }
    FileType getFileType(int idx) { return mFileInfo[idx].getFileType(); }

private:
    AssetDir(const AssetDir&) = delete;
    const AssetDir& operator=(const AssetDir&) = delete;

    friend class AssetManager;
    friend class AssetManager2;   // match AOSP friend list

    // Information about one file in the asset hierarchy.
    class FileInfo {
    public:
        FileInfo() {}
        explicit FileInfo(const std::string& path) : mFileName(path) {}
        ~FileInfo() {}

        // Ordered by file name only (for sort/merge, like AOSP SortedVector).
        bool operator<(const FileInfo& rhs) const { return mFileName < rhs.mFileName; }
        bool operator==(const FileInfo& rhs) const { return mFileName == rhs.mFileName; }

        void set(const std::string& path, FileType type) { mFileName = path; mFileType = type; }

        const std::string& getFileName() const { return mFileName; }
        void setFileName(const std::string& path) { mFileName = path; }
        FileType getFileType() const { return mFileType; }
        void setFileType(FileType type) { mFileType = type; }
        const std::string& getSourceName() const { return mSourceName; }
        void setSourceName(const std::string& path) { mSourceName = path; }

        // Find a file name in a sorted vector; returns index or -1.
        static int findEntry(const std::vector<FileInfo>* vec, const std::string& fileName);

    private:
        std::string mFileName;     // filename only
        FileType    mFileType = kFileTypeUnknown;
        std::string mSourceName;   // debug only
    };

    // AssetManager initializes us with a sorted list (takes ownership).
    void setFileList(std::vector<FileInfo>* list) {
        mFileInfo.clear();
        if (list) std::swap(mFileInfo, *list);
        delete list;
    }

    std::vector<FileInfo> mFileInfo;
};

} // namespace android
#endif // __CDROID_ANDROIDFW_ASSETDIR_H__
