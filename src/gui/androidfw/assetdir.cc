// Port of AOSP frameworks/base/libs/androidfw/AssetDir.cpp.
//
// Only the FileInfo::findEntry helper has real logic (the rest is inline in the
// header, matching how the AOSP class is mostly header-defined).

#include "assetdir.h"

#include <algorithm>

namespace cdroid {

// static. Binary-search a sorted vector of FileInfo for fileName.
/*static*/ int AssetDir::FileInfo::findEntry(const std::vector<AssetDir::FileInfo>* vec,
                                             const std::string& fileName) {
    if (vec == nullptr) return -1;
    auto it = std::lower_bound(vec->begin(), vec->end(), FileInfo(fileName));
    if (it != vec->end() && it->getFileName() == fileName) {
        return (int)(it - vec->begin());
    }
    return -1;
}

} // namespace cdroid
