// Default implementations of cdroid::Context's AOSP-aligned ID-based resource
// getters — delegate to getResources() (matching android.content.Context). The
// ID-based face coexists with the legacy string-based methods; concrete
// subclasses (Assets) supply getResources()/getAssets()/getDrawable(int)/
// getColorStateList(int). Header stays declaration-only.

#include "context.h"
#include "androidfw/resourcesimpl.h"   // cdroid::ResourcesImpl (+ Theme)
#include "resources.h"      // cdroid::Resources (full def — getResources() returns it)
#include <core/typedarray.h>       // TypedArray (constructed below)

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

Asset* Context::openRawResource(int id) {
    return getResources().openRawResource(id);
}

// Font resource resolution is deferred (stub returns nullptr).
Typeface* Context::getFont(int id) {
    (void)id;
    return nullptr;
}

// AOSP Resources.Theme.obtainStyledAttributes(attrs): resolve each attr against
// the live theme (defStyleAttr=0, defStyleRes=0). Delegates to getTheme() like
// the Java final in android.content.Context.
std::unique_ptr<TypedArray> Context::obtainStyledAttributes(const std::vector<int>& attrs) {
    ResTable::Theme& theme = getTheme();
    const ResTable& table = theme.getResTable();
    std::vector<uint32_t> ids(attrs.begin(), attrs.end());  // int[] -> uint32_t[] for the resolver
    std::vector<StyledAttr> styled(ids.size());
    cdroid::obtainStyledAttributes(table, &theme, ids.data(), ids.size(), 0, 0, styled.data());
    return std::make_unique<TypedArray>(table, std::move(styled), nullptr,
                                        getResources().getDisplayMetrics().density, this);
}

// AOSP Theme.obtainStyledAttributes(resId, attrs): resolve against a style on
// top of the theme (defStyleAttr=0, defStyleRes=resId).
std::unique_ptr<TypedArray> Context::obtainStyledAttributes(int resid, const std::vector<int>& attrs) {
    ResTable::Theme& theme = getTheme();
    const ResTable& table = theme.getResTable();
    std::vector<uint32_t> ids(attrs.begin(), attrs.end());
    std::vector<StyledAttr> styled(ids.size());
    cdroid::obtainStyledAttributes(table, &theme, ids.data(), ids.size(), 0, (uint32_t)resid, styled.data());
    return std::make_unique<TypedArray>(table, std::move(styled), nullptr,
                                        getResources().getDisplayMetrics().density, this);
}

} // namespace cdroid
