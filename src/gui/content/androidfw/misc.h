// Port of AOSP frameworks/base/libs/androidfw/include/androidfw/misc.h.
// Only the FileType enum + getFileType() helper are needed by the AssetManager
// port; lifted verbatim. FileType stays at global scope, matching AOSP.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
#ifndef __CDROID_ANDROIDFW_MISC_H__
#define __CDROID_ANDROIDFW_MISC_H__

#include <sys/stat.h>
#include <sys/types.h>

typedef enum FileType {
    kFileTypeUnknown = 0,
    kFileTypeNonexistent,       // i.e. ENOENT
    kFileTypeRegular,
    kFileTypeDirectory,
    kFileTypeCharDev,
    kFileTypeBlockDev,
    kFileTypeFifo,
    kFileTypeSymlink,
    kFileTypeSocket,
} FileType;

// Returns the type of the file named by fileName, or kFileTypeNonexistent if it
// cannot be stat()ed. Faithful to AOSP misc.cpp getFileType().
inline FileType getFileType(const char* fileName) {
    struct stat sb;
    if (stat(fileName, &sb) < 0) {
        return kFileTypeNonexistent;
    }
    if (S_ISREG(sb.st_mode)) return kFileTypeRegular;
    if (S_ISDIR(sb.st_mode)) return kFileTypeDirectory;
    if (S_ISCHR(sb.st_mode)) return kFileTypeCharDev;
    if (S_ISBLK(sb.st_mode)) return kFileTypeBlockDev;
    if (S_ISFIFO(sb.st_mode)) return kFileTypeFifo;
    if (S_ISLNK(sb.st_mode)) return kFileTypeSymlink;
    if (S_ISSOCK(sb.st_mode)) return kFileTypeSocket;
    return kFileTypeUnknown;
}

// Modification time of a file (0 if it cannot be stat()ed). Faithful to AOSP
// misc.cpp getFileModDate().
inline time_t getFileModDate(const char* fileName) {
    struct stat sb;
    if (stat(fileName, &sb) < 0) {
        return 0;
    }
    return sb.st_mtime;
}

#endif // __CDROID_ANDROIDFW_MISC_H__
