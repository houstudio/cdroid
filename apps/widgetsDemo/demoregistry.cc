#include "demoregistry.h"
#include <set>
#include <algorithm>

using namespace cdroid;

namespace {
std::vector<std::string> splitPath(const std::string& path) {
    std::vector<std::string> out;
    if (path.empty()) return out;  // "" is the root prefix — zero segments
    size_t start = 0;
    while (true) {
        const size_t slash = path.find('/', start);
        if (slash == std::string::npos) {
            out.push_back(path.substr(start));
            break;
        }
        out.push_back(path.substr(start, slash - start));
        start = slash + 1;
    }
    return out;
}

// true when `segs` begins with the `prefix` segments ("" prefixes match all).
bool hasPrefix(const std::vector<std::string>& segs, const std::vector<std::string>& prefix) {
    if (segs.size() < prefix.size()) return false;
    for (size_t i = 0; i < prefix.size(); i++) {
        if (segs[i] != prefix[i]) return false;
    }
    return true;
}
} // namespace

DemoRegistry& DemoRegistry::get() {
    static DemoRegistry sInstance;
    return sInstance;
}

void DemoRegistry::add(std::string path, std::function<Fragment*()> create, int rank) {
    mEntries.push_back(DemoEntry{std::move(path), std::move(create), rank});
}

std::vector<std::string> DemoRegistry::categories(const std::string& prefix) const {
    const std::vector<std::string> pre = splitPath(prefix);
    std::set<std::string> out;  // sorted + unique
    for (const DemoEntry& e : mEntries) {
        const std::vector<std::string> segs = splitPath(e.path);
        // Directly under prefix = one deeper; anything deeper contributes the
        // immediate child category name instead.
        if (!hasPrefix(segs, pre) || segs.size() <= pre.size() + 1) continue;
        out.insert(segs[pre.size()]);
    }
    return std::vector<std::string>(out.begin(), out.end());
}

std::vector<const DemoEntry*> DemoRegistry::demos(const std::string& prefix) const {
    const std::vector<std::string> pre = splitPath(prefix);
    std::vector<const DemoEntry*> out;
    for (const DemoEntry& e : mEntries) {
        const std::vector<std::string> segs = splitPath(e.path);
        if (hasPrefix(segs, pre) && segs.size() == pre.size() + 1) out.push_back(&e);
    }
    std::sort(out.begin(), out.end(), [](const DemoEntry* a, const DemoEntry* b) {
        return splitPath(a->path).back() < splitPath(b->path).back();
    });
    return out;
}

std::vector<const DemoEntry*> DemoRegistry::leavesUnder(const std::string& prefix) const {
    const std::vector<std::string> pre = splitPath(prefix);
    std::vector<const DemoEntry*> out;
    for (const DemoEntry& e : mEntries) {
        if (hasPrefix(splitPath(e.path), pre)) out.push_back(&e);
    }
    // Tab order: rank first (pre-refactor pages 1..N keep their original tab
    // order), then path — ported ApiDemos pages default after them.
    std::sort(out.begin(), out.end(), [](const DemoEntry* a, const DemoEntry* b) {
        if (a->rank != b->rank) return a->rank < b->rank;
        return a->path < b->path;
    });
    return out;
}
