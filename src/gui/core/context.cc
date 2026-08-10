// Default implementations of cdroid::Context's AOSP-aligned ID-based resource
// getters — delegate to getResources() (matching android.content.Context). The
// ID-based face coexists with the legacy string-based methods; concrete
// subclasses (Assets) supply getResources()/getAssets()/getDrawable(int)/
// getColorStateList(int). Header stays declaration-only.

#include "context.h"
#include "androidfw/resources.h"   // android::Resources (full def)

namespace cdroid {

std::string Context::getString(int id) {
    return getResources().getString(id);
}

std::u16string Context::getText(int id) {
    return getResources().getText(id);
}

std::string Context::getQuantityString(int id, int quantity) {
    return getResources().getQuantityString(id, quantity);
}

int Context::getColor(int id) {
    return getResources().getColor(id);
}

bool Context::getBoolean(int id) {
    return getResources().getBoolean(id);
}

int Context::getInteger(int id) {
    return getResources().getInteger(id);
}

float Context::getDimension(int id) {
    return getResources().getDimension(id);
}

int Context::getDimensionPixelSize(int id) {
    return getResources().getDimensionPixelSize(id);
}

android::Asset* Context::openRawResource(int id) {
    return getResources().openRawResource(id);
}

// Font resource resolution is deferred (stub returns nullptr).
Typeface* Context::getFont(int id) {
    (void)id;
    return nullptr;
}

} // namespace cdroid
