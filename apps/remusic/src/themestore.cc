#include "themestore.h"

#include <content/sharedpreferences.h>

using namespace cdroid;

namespace remusic {

ThemeStore& ThemeStore::get() {
    static ThemeStore instance;
    return instance;
}

const std::vector<std::pair<const char*, uint32_t>>& ThemeStore::palette() {
    static const std::vector<std::pair<const char*, uint32_t>> kPalette = {
        {"樱花粉", 0xFFD44A6A}, {"风暴蓝", 0xFF3F51B5}, {"木质棕", 0xFF795548},
        {"轻盈青", 0xFF009688}, {"雷霆紫", 0xFF673AB7}, {"沙岩橙", 0xFFEF6C00},
        {"烈焰红", 0xFFD43C33}, {"希望绿", 0xFF4CAF50},
    };
    return kPalette;
}

uint32_t ThemeStore::accent() const {
    if (mContext == nullptr) return 0xFFD43C33;
    return (uint32_t) mContext->getSharedPreferences("theme", 0)->getInt(
            "accent", (int)0xFFD43C33);
}

void ThemeStore::setAccent(uint32_t color) {
    if (mContext == nullptr) return;
    mContext->getSharedPreferences("theme", 0)->edit().putInt(
            "accent", (int)color).apply();
}

bool ThemeStore::night() const {
    if (mContext == nullptr) return false;
    return mContext->getSharedPreferences("theme", 0)->getBoolean("night", false);
}

void ThemeStore::setNight(bool night) {
    if (mContext == nullptr) return;
    mContext->getSharedPreferences("theme", 0)->edit().putBoolean(
            "night", night).apply();
}

} // namespace remusic
