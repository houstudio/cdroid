// Default implementations of cdroid::Context's AOSP-aligned ID-based resource
// getters — delegate to getResources() (matching android.content.Context). The
// ID-based face coexists with the legacy string-based methods; concrete
// subclasses (Assets) supply getResources()/getAssets()/getDrawable(int)/
// getColorStateList(int). Header stays declaration-only.

#include "context.h"
#include "content/resourcesimpl.h"   // cdroid::ResourcesImpl (+ Theme)
#include "resources.h"      // cdroid::Resources (full def — getResources() returns it)
#include <content/typedarray.h>       // TypedArray (constructed below)
#include <content/androidfw/restable.h>     // ResTable::Theme + obtainStyledAttributes + StyledAttr

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

// AOSP Context.getDrawable(id) is final:
//   getResources().getDrawable(id, getTheme()) — the resource load is themed
// through the context's theme (ContextThemeWrapper overrides getTheme, so
// wrapped contexts resolve ?attr inside drawable/CSL XML with their overlay).
Drawable* Context::getDrawable(int id) {
    Resources::Theme t = getTheme();
    return getResources().getDrawable(id, &t);
}

// AOSP Context.getColorStateList(id) is final: loadColorStateList(id, getTheme()).
std::shared_ptr<ColorStateList> Context::getColorStateList(int id) {
    Resources::Theme t = getTheme();
    return getResources().getColorStateList(id, &t);
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
// the Java final in android.content.Context. `attrs` is sentinel-terminated.
std::unique_ptr<TypedArray> Context::obtainStyledAttributes(const uint32_t* attrs) {
    Resources::Theme _th = getTheme(); ResTable::Theme* theme = static_cast<ResTable::Theme*>(_th._engineHandle());
    const ResTable& table = theme->getResTable();
    size_t n = 0; while (attrs[n]) ++n;  // count up to the trailing-0 sentinel
    std::vector<StyledAttr> styled(n);
    cdroid::obtainStyledAttributes(table, theme, attrs, 0, 0, styled.data());
    return std::make_unique<TypedArray>(table, std::move(styled), nullptr,
                                        getResources().getDisplayMetrics().density, &getResources(), &_th);
}

// AOSP Theme.obtainStyledAttributes(resId, attrs): resolve against a style on
// top of the theme (defStyleAttr=0, defStyleRes=resId). `attrs` is sentinel-
// terminated.
std::unique_ptr<TypedArray> Context::obtainStyledAttributes(int resid, const uint32_t* attrs) {
    Resources::Theme _th = getTheme(); ResTable::Theme* theme = static_cast<ResTable::Theme*>(_th._engineHandle());
    const ResTable& table = theme->getResTable();
    size_t n = 0; while (attrs[n]) ++n;
    std::vector<StyledAttr> styled(n);
    cdroid::obtainStyledAttributes(table, theme, attrs, 0, (uint32_t)resid, styled.data());
    return std::make_unique<TypedArray>(table, std::move(styled), nullptr,
                                        getResources().getDisplayMetrics().density, &getResources(), &_th);
}

// AOSP Context.obtainStyledAttributes(set, attrs, defStyleAttr, defStyleRes)
// is final and resolves against getTheme(); Assets overrides with the same
// resolution straight through Resources (no virtual detour needed).
std::unique_ptr<TypedArray> Context::obtainStyledAttributes(const AttributeSet* attrs,
        const uint32_t* styleable, int32_t defStyleAttr, int32_t defStyleRes) {
    return getTheme().obtainStyledAttributes(attrs, styleable, defStyleAttr, defStyleRes);
}

// Convenience overload: non-null AttributeSet& delegates to the pointer form.
std::unique_ptr<TypedArray> Context::obtainStyledAttributes(const AttributeSet& attrs,
        const uint32_t* styleable, int32_t defStyleAttr, int32_t defStyleRes) {
    return obtainStyledAttributes(&attrs, styleable, defStyleAttr, defStyleRes);
}

} // namespace cdroid
