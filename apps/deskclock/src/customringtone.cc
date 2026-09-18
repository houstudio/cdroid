#include <customringtone.h>

#include <algorithm>
#include <cctype>

namespace cdroid {
namespace deskclock {
namespace data {

// String.CASE_INSENSITIVE_ORDER over the titles (upstream delegates to it).
static bool titleLess(const std::string& a, const std::string& b) {
    const size_t n = std::min(a.size(), b.size());
    for (size_t i = 0; i < n; i++) {
        const int ca = std::tolower((unsigned char) a[i]);
        const int cb = std::tolower((unsigned char) b[i]);
        if (ca != cb) return ca < cb;
    }
    return a.size() < b.size();
}

int CustomRingtone::compareTo(const CustomRingtone& other) const {
    if (titleLess(title, other.title)) return -1;
    if (titleLess(other.title, title)) return 1;
    return 0;
}

} // namespace data
} // namespace deskclock
} // namespace cdroid
