// cdroid::Resources — forwarding to the aggregated ResourcesImpl + own GUI
// factories + obtainStyledAttributes/obtainTypedArray. resourcesimpl.h is
// included HERE (not in resources.h) so the heavy androidfw headers stay hidden
// from the many files that include resources.h via context.h.

#include "resources.h"
#include "context.h"
#include <core/app.h>                  // App::getInstance() (getSystem)
#include <core/attributeset.h>
#include <core/typedarray.h>
#include <core/xmlpullparser.h>
#include <androidfw/restable.h>        // obtainStyledAttributes resolver, ResXMLTree, StyledAttr
#include <core/resourcesimpl.h>   // ResourcesImpl (aggregated)
#include <drawable/drawable.h>
#include <drawable/colordrawable.h>
#include <drawable/colorstatelist.h>
#include <unordered_map>

namespace cdroid {

// ===========================================================================
// DrawableCache / ColorStateListCache — AOSP ResourcesImpl.mDrawableCache /
// mComplexColorCache equivalents. Defined in the .cc (not resources.h) because
// they own GUI types the header can only forward-declare (Drawable::ConstantState
// is a nested type; ColorStateList). They live in cdroid::Resources, not
// ResourcesImpl, because androidfw is a cairo-free OBJECT library and these GUI
// headers require cairo (see resourcesimpl.h).
//
// Drawable cache holds ConstantState WEAKLY (getDrawable returns a fresh Drawable
// via ConstantState::newDrawable(), so the cache need not keep it alive).
// ColorStateList cache holds the instance STRONGLY (getColorStateList returns
// the SAME instance, so the cache keeps it alive — matches AOSP caching the
// ComplexColor instance).
// ===========================================================================
class Resources::DrawableCache {
public:
    std::shared_ptr<Drawable::ConstantState> get(int id) {
        auto it = mEntries.find(id);
        if (it == mEntries.end()) return nullptr;
        if (it->second.expired()) { mEntries.erase(it); return nullptr; }
        return it->second.lock();
    }
    void put(int id, const std::shared_ptr<Drawable::ConstantState>& cs) {
        if (cs) mEntries[id] = cs;
    }
private:
    std::unordered_map<int, std::weak_ptr<Drawable::ConstantState>> mEntries;
};

class Resources::ColorStateListCache {
public:
    std::shared_ptr<ColorStateList> get(int id) const {
        auto it = mEntries.find(id);
        return it == mEntries.end() ? nullptr : it->second;
    }
    void put(int id, const std::shared_ptr<ColorStateList>& csl) {
        if (csl) mEntries[id] = csl;
    }
private:
    std::unordered_map<int, std::shared_ptr<ColorStateList>> mEntries;
};

// ===========================================================================
// Construction / destruction (ResourcesImpl complete here)
// ===========================================================================

Resources::Resources(AssetManager* am, cdroid::Context* ctx)
    : mImpl(std::make_unique<ResourcesImpl>(am, nullptr,
              ctx ? &ctx->getDisplayMetrics() : nullptr)),
      mCtx(ctx),
      mDrawableCache(std::make_unique<DrawableCache>()),
      mColorStateListCache(std::make_unique<ColorStateListCache>()) {
}

Resources::~Resources() = default;   // unique_ptr<ResourcesImpl> dtor instantiated here

// ===========================================================================
// Forwarded to mImpl (AOSP Resources -> ResourcesImpl)
// ===========================================================================

AssetManager* Resources::getAssets() const {
    return mImpl->getAssets();
}

const ResTable_config& Resources::getConfiguration() const {
    return mImpl->getConfiguration();
}

const DisplayMetrics& Resources::getDisplayMetrics() const {
    return mImpl->getDisplayMetrics();
}

void Resources::setConfiguration(const ResTable_config& c) {
    mImpl->setConfiguration(c);
}

void Resources::setDisplayMetrics(const DisplayMetrics& m) {
    mImpl->setDisplayMetrics(m);
}

int Resources::getIdentifier(const std::string& n, const std::string& t, const std::string& p) const {
    return mImpl->getIdentifier(n, t, p);
}

bool Resources::getResourceName(int id, std::string* o) const {
    return mImpl->getResourceName(id, o);
}

bool Resources::getResourceEntryName(int id, std::string* o) const {
    return mImpl->getResourceEntryName(id, o);
}

bool Resources::getResourceTypeName(int id, std::string* o) const {
    return mImpl->getResourceTypeName(id, o);
}

bool Resources::getResourcePackageName(int id, std::string* o) const {
    return mImpl->getResourcePackageName(id, o);
}

bool Resources::getValue(int id, TypedValue* o, bool r) const {
    return mImpl->getValue(id, o, r);
}

bool Resources::getValue(const std::string& n, TypedValue* o, bool r) const {
    return mImpl->getValue(n, o, r);
}

std::string Resources::getString(int id) const {
    return mImpl->getString(id);
}

std::u16string Resources::getText(int id) const {
    return mImpl->getText(id);
}

std::u16string Resources::getText(int id, const std::u16string& def) const {
    return mImpl->getText(id, def);
}

int Resources::getInteger(int id) const {
    return mImpl->getInteger(id);
}

bool Resources::getBoolean(int id) const {
    return mImpl->getBoolean(id);
}

float Resources::getFloat(int id) const {
    return mImpl->getFloat(id);
}

int Resources::getColor(int id) const {
    return mImpl->getColor(id);
}

float Resources::getDimension(int id) const {
    return mImpl->getDimension(id);
}

int Resources::getDimensionPixelOffset(int id) const {
    return mImpl->getDimensionPixelOffset(id);
}

int Resources::getDimensionPixelSize(int id) const {
    return mImpl->getDimensionPixelSize(id);
}

float Resources::getFraction(int id, float b, float p) const {
    return mImpl->getFraction(id, b, p);
}

std::string Resources::getQuantityString(int id, int q) const {
    return mImpl->getQuantityString(id, q);
}

std::u16string Resources::getQuantityText(int id, int q) const {
    return mImpl->getQuantityText(id, q);
}

std::vector<std::string> Resources::getStringArray(int id) const {
    return mImpl->getStringArray(id);
}

std::vector<std::u16string> Resources::getTextArray(int id) const {
    return mImpl->getTextArray(id);
}

std::vector<int> Resources::getIntArray(int id) const {
    return mImpl->getIntArray(id);
}

Asset* Resources::openRawResource(int id, TypedValue* o) const {
    return mImpl->openRawResource(id, o);
}

Asset* Resources::getXml(int id) const {
    return mImpl->getXml(id);
}

Asset* Resources::getLayout(int id) const {
    return mImpl->getLayout(id);
}

Asset* Resources::getAnimation(int id) const {
    return mImpl->getAnimation(id);
}

Typeface* Resources::getFont(int id) const {
    return mImpl->getFont(id);
}

// AOSP ResourcesImpl.loadComplexColor(Resources, TypedValue, id, Theme).
//
//   1. cache hit → return cached instance              (mColorStateListCache)
//   2. getValue(id) → TypedValue                        (ResourcesImpl.getValue)
//   3. TYPE_FIRST/LAST_COLOR_INT → valueOf(data)        (getColorStateListFromInt)
//   4. TYPE_STRING (xml) → createFromXml inline          (loadComplexColorForCookie)
//   5. cache the instance, return it
//
// Returns a shared_ptr so callers (Assets::getColorStateList) can retain the
// cached instance; getColorStateList(id) unwraps to a raw pointer.
std::shared_ptr<ComplexColor> Resources::loadComplexColor(int id) const {
    if (id == 0 || mCtx == nullptr) return nullptr;
    // 1. Cache hit (AOSP mComplexColorCache).
    if (mColorStateListCache) {
        if (auto csl = mColorStateListCache->get(id)) return csl;
    }
    // 2. Resolve the typed value (AOSP ResourcesImpl.getValue).
    TypedValue value;
    if (!getValue(id, &value, true)) return nullptr;
    std::shared_ptr<ColorStateList> csl;
    // 3. Inline color (AOSP getColorStateListFromInt): a raw color resolves
    //    straight to a single-color ColorStateList (valueOf caches it).
    if (value.type >= TypedValue::TYPE_FIRST_COLOR_INT &&
        value.type <= TypedValue::TYPE_LAST_COLOR_INT) {
        csl = ColorStateList::valueOf(value.data);
    } else {
        // 4. XML color-state-list (AOSP loadComplexColorForCookie): inflate the
        //    binary AXML INLINE via ColorStateList.createFromXml. Do NOT call
        //    mCtx->getColorStateList(name) here -- that re-enters
        //    Assets::getColorStateList, which routes back to loadComplexColor
        //    and would recurse infinitely.
        std::string ref;
        if (getResourceName(id, &ref)) {
            try {
                XmlPullParser parser(mCtx, ref);
                // createFromXml takes a non-const Resources&; route through the
                // Context (same Resources object) since loadComplexColor is const.
                csl = ColorStateList::createFromXml(mCtx->getResources(), parser);
            } catch (const std::exception&) {
                csl = nullptr;
            }
        }
    }
    // 5. Cache (AOSP cache.put).
    if (csl && mColorStateListCache) mColorStateListCache->put(id, csl);
    return csl;
}

Movie* Resources::getMovie(int id) const {
    return mImpl->getMovie(id);
}

// ===========================================================================
// ===========================================================================
// AOSP parity additions (getSystem, format-arg overloads, openRawResourceFd)
// ===========================================================================

Resources& Resources::getSystem() {
    return App::getInstance().getResources();
}

// Simple %s/%d/%f placeholder replacement (not full Java Formatter — enough for
// the common getString(R.string.x, arg) pattern).
namespace {
std::string formatWithArgs(const std::string& fmt, const std::vector<std::string>& args) {
    if (args.empty()) return fmt;
    std::string out;
    size_t ai = 0;
    for (size_t i = 0; i < fmt.size(); i++) {
        if (fmt[i] == '%' && i + 1 < fmt.size() && ai < args.size()) {
            const char c = fmt[i + 1];
            if (c == 's' || c == 'd' || c == 'f') { out += args[ai++]; i++; continue; }
        }
        out += fmt[i];
    }
    return out;
}
} // anon

std::string Resources::getString(int id, const std::vector<std::string>& formatArgs) const {
    return formatWithArgs(mImpl->getString(id), formatArgs);
}

std::string Resources::getQuantityString(int id, int quantity, const std::vector<std::string>& formatArgs) const {
    return formatWithArgs(mImpl->getQuantityString(id, quantity), formatArgs);
}

Asset* Resources::openRawResourceFd(int id) const {
    (void)id;
    return nullptr;   // CDROID has no AssetFileDescriptor (fd-based assets)
}

// ===========================================================================
// GUI factories — own the AOSP mDrawableCache / mComplexColorCache + load logic.
// These live in cdroid::Resources (not ResourcesImpl) because androidfw is a
// cairo-free OBJECT library and Drawable/ColorStateList headers require cairo.
// ===========================================================================

// AOSP Resources.getDrawable(id) → getDrawableForDensity(id, 0).
cdroid::Drawable* Resources::getDrawable(int id) const {
    return getDrawableForDensity(id, 0);
}

// AOSP Resources.getDrawableForDensity(id, density) → ResourcesImpl.loadDrawable:
//   1. cache hit → ConstantState::newDrawable()        (mDrawableCache)
//   2. getValue(id) → TypedValue
//   3. TYPE_FIRST/LAST_COLOR_INT → ColorDrawable(data)  (AOSP isColorDrawable)
//   4. TYPE_STRING (file/xml) → inflate                 (loadDrawableForCookie)
//   5. cache the ConstantState, return the drawable
// CDROID has a single density/configuration, so the AOSP density-adjustment of
// value.density is a no-op here.
cdroid::Drawable* Resources::getDrawableForDensity(int id, int /*density*/) const {
    if (id == 0 || mCtx == nullptr) return nullptr;
    // 1. Cache hit — reuse the inflated ConstantState (AOSP mDrawableCache).
    if (mDrawableCache) {
        if (auto cs = mDrawableCache->get(id)) return cs->newDrawable();
    }
    // 2. Resolve the typed value (AOSP ResourcesImpl.getValue).
    TypedValue value;
    if (!getValue(id, &value, true)) return nullptr;
    // 3. Color-drawable path (AOSP isColorDrawable): a raw color value resolves
    //    straight to a ColorDrawable — no file round-trip.
    Drawable* d = nullptr;
    if (value.type >= TypedValue::TYPE_FIRST_COLOR_INT &&
        value.type <= TypedValue::TYPE_LAST_COLOR_INT) {
        d = new ColorDrawable(value.data);
    } else {
        // 4. File/xml drawable (AOSP loadDrawableForCookie): resolve the
        //    resource name and inflate through the existing string-based loader
        //    (Assets.getDrawable → DrawableInflater / ImageDecoder), which owns
        //    the 9-patch / theme-ref / color-state-list edge cases.
        std::string ref;
        if (getResourceName(id, &ref)) d = mCtx->getDrawable(ref);
    }
    // 5. Cache the ConstantState (AOSP cacheDrawable).
    if (d && mDrawableCache) mDrawableCache->put(id, d->getConstantState());
    return d;
}

// AOSP Resources.getColorStateList(id) → loadColorStateList → loadComplexColor,
// downcast to ColorStateList (ComplexColor also covers GradientColor).
cdroid::ColorStateList* Resources::getColorStateList(int id) const {
    return dynamic_cast<ColorStateList*>(loadComplexColor(id).get());
}

// ===========================================================================
// AOSP Resources.obtainStyledAttributes(...)
// ===========================================================================

std::unique_ptr<TypedArray> Resources::obtainStyledAttributes(const AttributeSet* set,
         const uint32_t* attrs, int defStyleAttr, int defStyleRes) const {
    if (mCtx == nullptr) return nullptr;
    const ResTable& rt = getAssets()->getResources(false);
    ResTable::Theme* theme = &mCtx->getTheme();
    size_t count = 0;
    while (attrs[count]) count++;
    std::vector<StyledAttr> styled(count);

    if (set != nullptr) {
        const XmlPullParser* parser = dynamic_cast<const XmlPullParser*>(set);
        if (parser && parser->isBinaryAXML()) {
            const ResXMLTree* xml = static_cast<const ResXMLTree*>(parser->getBinaryAXMLTree());
            if (xml) {
                cdroid::obtainStyledAttributes(*xml, rt, theme, attrs,
                                               (uint32_t)defStyleAttr, (uint32_t)defStyleRes, styled.data());
                return std::make_unique<TypedArray>(rt, std::move(styled), xml, getDisplayMetrics().density, this);
            }
        }
        const int styleResId = set->getStyleResourceId();
        if (styleResId != 0) {
            cdroid::obtainStyledAttributes(rt, theme, attrs,
                                           (uint32_t)defStyleAttr, (uint32_t)styleResId, styled.data());
            return std::make_unique<TypedArray>(rt, std::move(styled), nullptr, getDisplayMetrics().density, this);
        }
    }
    cdroid::obtainStyledAttributes(rt, theme, attrs,
                                   (uint32_t)defStyleAttr, (uint32_t)defStyleRes, styled.data());
    return std::make_unique<TypedArray>(rt, std::move(styled), nullptr, getDisplayMetrics().density, this);
}

// Convenience: AttributeSet& → AttributeSet* (for AOSP callers passing the reference).
std::unique_ptr<TypedArray> Resources::obtainStyledAttributes(const AttributeSet& set,
        const uint32_t* attrs, int defStyleAttr, int defStyleRes) const {
    return obtainStyledAttributes(&set, attrs, defStyleAttr, defStyleRes);
}

std::unique_ptr<TypedArray> Resources::obtainStyledAttributes(const uint32_t* attrs) const {
    return obtainStyledAttributes(0, attrs);
}

std::unique_ptr<TypedArray> Resources::obtainStyledAttributes(int resid, const uint32_t* attrs) const {
    if (mCtx == nullptr) return nullptr;
    const ResTable& rt = getAssets()->getResources(false);
    ResTable::Theme* theme = &mCtx->getTheme();
    size_t count = 0;
    while (attrs[count]) count++;
    std::vector<StyledAttr> styled(count);
    cdroid::obtainStyledAttributes(rt, theme, attrs, 0, (uint32_t)resid, styled.data());
    return std::make_unique<TypedArray>(rt, std::move(styled), nullptr, getDisplayMetrics().density, this);
}

// AOSP Resources.obtainTypedArray(@ArrayRes int id) — TypedArray view over a
// typed array resource; each index is one element.
std::unique_ptr<TypedArray> Resources::obtainTypedArray(int id) const {
    if (mCtx == nullptr) return nullptr;
    const ResTable& rt = getAssets()->getResources(false);
    size_t count = 0;
    ssize_t block = -1;
    const ResTable_map* map = rt.getBag((uint32_t)id, &count, nullptr, &block);
    if (!map || count == 0) return nullptr;
    std::vector<StyledAttr> styled(count);
    for (size_t i = 0; i < count; i++) {
        styled[i].value = map[i].value;
        styled[i].stringBlock = block;
        styled[i].set = true;
    }
    return std::make_unique<TypedArray>(rt, std::move(styled), nullptr, getDisplayMetrics().density, this);
}

} // namespace cdroid
