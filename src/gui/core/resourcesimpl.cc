// Port of AOSP android.content.res.Resources (+ ResourcesImpl) — value/meta/asset
// resolution layer. See resources.h for the isolated-port contract (GUI-object
// factories are stubbed, filled at the cdroid.so merge).

#define LOG_TAG "resources"

#include "resourcesimpl.h"

#include <porting/cdlog.h>

#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>

// androidfw native readers — hidden from resourcesimpl.h (the facade contract).
#include <androidfw/restable.h>       // ResTable, ResTable_config, Res_value, pakPathCandidates
#include <core/assetmanager.h>        // AssetManager
#include <core/asset.h>               // Asset
#include <core/typedvalue.h>     // TypedValue

#include <drawable/drawable.h>        // Drawable::ConstantState
#include <drawable/colordrawable.h>   // ColorDrawable (color-drawable path)
#include <drawable/colorstatelist.h>  // ColorStateList cache + createFromXml
#include <drawable/drawableinflater.h>  // DrawableInflater::inflateFromXml (xml drawable self-load)
#include <image-decoders/imagedecoder.h>  // ImageDecoder::createAsDrawable(id) (image self-load)
#include <core/context.h>             // inflation bridge (mCtx)
#include <core/xmlpullparser.h>       // ColorStateList::createFromXml inline inflate

using namespace cdroid;
using cdroid::ResTable;
using cdroid::ResTable_config;
using cdroid::Res_value;

// ---- UTF-16 (char16_t run, with surrogate collapse) -> UTF-8 ----
static std::string u16to8(const char16_t* s, size_t len) {
    std::string out;
    if (s == nullptr) return out;
    out.reserve(len);
    for (size_t i = 0; i < len; ++i) {
        uint32_t c = s[i];
        if (c >= 0xD800 && c <= 0xDBFF && i + 1 < len && s[i + 1] >= 0xDC00 && s[i + 1] <= 0xDFFF) {
            c = 0x10000 + ((c - 0xD800) << 10) + (s[++i] - 0xDC00);
        }
        if (c < 0x80) {
            out += (char)c;
        } else if (c < 0x800) {
            out += (char)(0xC0 | (c >> 6));
            out += (char)(0x80 | (c & 0x3F));
        } else if (c < 0x10000) {
            out += (char)(0xE0 | (c >> 12));
            out += (char)(0x80 | ((c >> 6) & 0x3F));
            out += (char)(0x80 | (c & 0x3F));
        } else {
            out += (char)(0xF0 | (c >> 18));
            out += (char)(0x80 | ((c >> 12) & 0x3F));
            out += (char)(0x80 | ((c >> 6) & 0x3F));
            out += (char)(0x80 | (c & 0x3F));
        }
    }
    return out;
}

// ---- ResourcesImpl ----
// (TypedValue methods + applyDimension moved to androidfw/typedvalue.cc.)

ResourcesImpl::ResourcesImpl(AssetManager* am, const ResTable_config* config,
                     const DisplayMetrics* metrics) : mAssets(am) {
    mConfig = std::make_unique<ResTable_config>();
    if (config != nullptr) *mConfig = *config;
    else memset(mConfig.get(), 0, sizeof(ResTable_config));
    if (metrics != nullptr) mMetrics = *metrics;  // else default density=1
    // AOSP seeds the live configuration from the device defaults (density).
    mConfiguration.setToDefaults();
    if (metrics != nullptr) mConfiguration.densityDpi = mMetrics.densityDpi;
    mDrawableCache = std::make_unique<DrawableCache>();
    mColorStateListCache = std::make_unique<ColorStateListCache>();
}

ResourcesImpl::~ResourcesImpl() {
}


// AOSP ResourcesImpl.updateConfigurationImpl → mAssets.setConfigurationInternal:
// map the live Configuration onto the arsc ResTable_config so -night/-land/...
// resource variants reselect. CDROID's locale is a BCP-47 tag string
// ("zh-CN" / "xx-rYY"), packed via ResTable_config::packLanguage/packRegion.
static ResTable_config toResTableConfig(const Configuration& c, const DisplayMetrics& m) {
    ResTable_config cfg = {};
    cfg.size = sizeof(ResTable_config);
    cfg.mcc = (uint16_t)c.mcc;
    cfg.mnc = (uint16_t)c.mnc;
    if (!c.locale.empty()) {
        const size_t dash = c.locale.find('-');
        const std::string lang = c.locale.substr(0, 2);
        std::string region = (dash != std::string::npos) ? c.locale.substr(dash + 1) : std::string();
        if (region.size() > 2 && region[0] == 'r') region = region.substr(1);   // xx-rYY
        if (region.size() >= 2) region = region.substr(region.size() - 2);
        cfg.packLanguage(lang.c_str());
        if (region.size() == 2) cfg.packRegion(region.c_str());
    }
    cfg.orientation = (uint8_t)c.orientation;
    cfg.touchscreen = (uint8_t)c.touchscreen;
    cfg.density = (uint16_t)((c.densityDpi != Configuration::DENSITY_DPI_UNDEFINED)
                             ? c.densityDpi : m.densityDpi);
    cfg.keyboard = (uint8_t)c.keyboard;
    // keyboardHidden/navigationHidden share the inputFlags byte (AOSP native layout).
    uint8_t inputFlags = 0;
    switch (c.keyboardHidden) {
        case Configuration::KEYBOARDHIDDEN_NO:  inputFlags |= ResTable_config::KEYSHIDDEN_NO;  break;
        case Configuration::KEYBOARDHIDDEN_YES: inputFlags |= ResTable_config::KEYSHIDDEN_YES; break;
        case Configuration::KEYBOARDHIDDEN_SOFT:inputFlags |= ResTable_config::KEYSHIDDEN_SOFT;break;
    }
    switch (c.navigationHidden) {
        case Configuration::NAVIGATIONHIDDEN_NO:  inputFlags |= ResTable_config::NAVHIDDEN_NO;  break;
        case Configuration::NAVIGATIONHIDDEN_YES: inputFlags |= ResTable_config::NAVHIDDEN_YES; break;
    }
    cfg.inputFlags = inputFlags;
    cfg.navigation = (uint8_t)c.navigation;
    const int w = (m.widthPixels >= m.heightPixels) ? m.widthPixels : m.heightPixels;
    const int h = (m.widthPixels >= m.heightPixels) ? m.heightPixels : m.widthPixels;
    cfg.screenWidth = (uint16_t)w;
    cfg.screenHeight = (uint16_t)h;
    cfg.screenLayout = (uint8_t)c.screenLayout;
    cfg.uiMode = (uint8_t)c.uiMode;
    cfg.smallestScreenWidthDp = (uint16_t)c.smallestScreenWidthDp;
    cfg.screenWidthDp = (uint16_t)c.screenWidthDp;
    cfg.screenHeightDp = (uint16_t)c.screenHeightDp;
    return cfg;
}

const Configuration& ResourcesImpl::getConfiguration() const {
    return mConfiguration;
}

const ResTable_config& ResourcesImpl::getResTableConfig() const {
    return *mConfig;
}

void ResourcesImpl::setConfiguration(const ResTable_config& config) {
    *mConfig = config;
}

// AOSP ResourcesImpl.calcConfigChanges(@Nullable Configuration): null → all
// flags changed; otherwise the updateFrom() delta against the live config.
int ResourcesImpl::calcConfigChanges(const Configuration* config) {
    if (config == nullptr) return 0xFFFFFFFF;
    // AOSP calculates on a scratch copy (mTmpConfig.setTo(config)) so the live
    // configuration is not modified by the calculation.
    Configuration tmp = mConfiguration;
    return tmp.updateFrom(*config);
}


int ResourcesImpl::getIdentifier(const std::string& name,
        const std::string& type, const std::string& package) const {
    if (mAssets == nullptr) return 0;
    return (int)mAssets->getResources(false).getIdentifier(name, type, package);
}

bool ResourcesImpl::getResourceName(int id, std::string* out) const {
    std::string pkg, type, key;
    if (mAssets == nullptr) return false;
    if (!mAssets->getResources(false).getResourceName((uint32_t)id, &pkg, &type, &key)) return false;
    if (out) *out = pkg + ":" + type + "/" + key;
    return true;
}

bool ResourcesImpl::getResourcePackageName(int id, std::string* out) const {
    std::string pkg, type, key;
    if (mAssets == nullptr) return false;
    if (!mAssets->getResources(false).getResourceName((uint32_t)id, &pkg, &type, &key)) return false;
    if (out) *out = pkg;
    return true;
}

bool ResourcesImpl::getResourceTypeName(int id, std::string* out) const {
    std::string pkg, type, key;
    if (mAssets == nullptr) return false;
    if (!mAssets->getResources(false).getResourceName((uint32_t)id, &pkg, &type, &key)) return false;
    if (out) *out = type;
    return true;
}

bool ResourcesImpl::getResourceEntryName(int id, std::string* out) const {
    std::string pkg, type, key;
    if (mAssets == nullptr) return false;
    if (!mAssets->getResources(false).getResourceName((uint32_t)id, &pkg, &type, &key)) return false;
    if (out) *out = key;
    return true;
}

bool ResourcesImpl::getValue(int id, TypedValue* outValue, bool resolveRefs) const {
    if (mAssets == nullptr || id <= 0 || outValue == nullptr) return false;
    const ResTable& rt = mAssets->getResources(false);

    Res_value v;
    ResTable_config cfg;
    memset(&cfg, 0, sizeof(cfg));
    ssize_t block = rt.getResource((uint32_t)id, &v, false, 0, nullptr, &cfg);
    if (block < 0) return false;

    if (resolveRefs) {
        block = rt.resolveReference(&v, block, nullptr, nullptr, &cfg);
        if (block < 0) return false;
    }

    outValue->type = v.dataType;
    outValue->data = v.data;
    outValue->resourceId = (uint32_t)id;
    outValue->density = (int)cfg.density;
    outValue->changingConfigurations = 0;
    outValue->string = nullptr;
    outValue->stringLen = 0;
    if (v.dataType == Res_value::TYPE_STRING) {
        size_t len = 0;
        const char16_t* s = rt.stringAtBlock(block, v.data, &len);
        outValue->string = s;
        outValue->stringLen = len;
    }
    return true;
}

bool ResourcesImpl::getValue(const std::string& name, TypedValue* outValue, bool resolveRefs) const {
    const int id = getIdentifier(name, "", "");
    if (id == 0) return false;
    return getValue(id, outValue, resolveRefs);
}

std::string ResourcesImpl::getString(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true) || tv.type != Res_value::TYPE_STRING) return std::string();
    return u16to8(tv.string, tv.stringLen);
}

std::u16string ResourcesImpl::getText(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true) || tv.type != Res_value::TYPE_STRING) return std::u16string();
    return std::u16string(tv.string, tv.stringLen);
}

std::u16string ResourcesImpl::getText(int id, const std::u16string& def) const {
    std::u16string s = getText(id);
    return s.empty() ? def : s;
}

// AOSP Resources.getStringArray/getIntArray/getTextArray — a typed array
// resource (<string-array>/<integer-array>) is stored as a bag; each map entry
// is one element. Reads raw values; reference entries (@string/...) are left
// for the caller to follow (rare in arrays).
std::vector<std::string> ResourcesImpl::getStringArray(int id) const {
    std::vector<std::string> out;
    const ResTable& rt = getAssets()->getResources(false);
    size_t count = 0; ssize_t block = -1;
    const ResTable_map* map = rt.getBag((uint32_t)id, &count, nullptr, &block);
    if (!map) return out;
    for (size_t i = 0; i < count; i++) {
        const Res_value& v = map[i].value;
        if (v.dataType == Res_value::TYPE_STRING) {
            size_t len = 0;
            const char16_t* s = rt.stringAtBlock(block, v.data, &len);
            if (s && len) out.push_back(u16to8(s, len));
        }
    }
    return out;
}

std::vector<std::u16string> ResourcesImpl::getTextArray(int id) const {
    std::vector<std::u16string> out;
    const ResTable& rt = getAssets()->getResources(false);
    size_t count = 0; ssize_t block = -1;
    const ResTable_map* map = rt.getBag((uint32_t)id, &count, nullptr, &block);
    if (!map) return out;
    for (size_t i = 0; i < count; i++) {
        const Res_value& v = map[i].value;
        if (v.dataType == Res_value::TYPE_STRING) {
            size_t len = 0;
            const char16_t* s = rt.stringAtBlock(block, v.data, &len);
            if (s && len) out.emplace_back(s, len);
        }
    }
    return out;
}

std::vector<int> ResourcesImpl::getIntArray(int id) const {
    std::vector<int> out;
    const ResTable& rt = getAssets()->getResources(false);
    size_t count = 0; ssize_t block = -1;
    const ResTable_map* map = rt.getBag((uint32_t)id, &count, nullptr, &block);
    if (!map) return out;
    for (size_t i = 0; i < count; i++) {
        const Res_value& v = map[i].value;
        if (v.dataType == Res_value::TYPE_INT_DEC || v.dataType == Res_value::TYPE_INT_HEX
            || v.dataType == Res_value::TYPE_INT_BOOLEAN) {
            out.push_back((int)v.data);
        }
    }
    return out;
}

int ResourcesImpl::getInteger(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true)) return 0;
    if (tv.type >= Res_value::TYPE_FIRST_INT && tv.type <= Res_value::TYPE_LAST_INT) {
        return (int)tv.data;
    }
    if (tv.type == Res_value::TYPE_INT_BOOLEAN) {
        return tv.data != 0 ? 1 : 0;
    }
    return 0;
}

bool ResourcesImpl::getBoolean(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true)) return false;
    if (tv.type == Res_value::TYPE_INT_BOOLEAN) {
        return tv.data != 0;
    }
    if (tv.type >= Res_value::TYPE_FIRST_INT && tv.type <= Res_value::TYPE_LAST_INT) {
        return tv.data != 0;
    }
    return false;
}

float ResourcesImpl::getFloat(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true)) return 0.0f;
    if (tv.type == Res_value::TYPE_FLOAT) {
        return tv.getFloat();
    }
    if (tv.type >= Res_value::TYPE_FIRST_INT && tv.type <= Res_value::TYPE_LAST_INT) {
        return (float)(int)tv.data;
    }
    return 0.0f;
}

int ResourcesImpl::getColor(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true)) return 0;
    if (tv.type >= Res_value::TYPE_FIRST_COLOR_INT && tv.type <= Res_value::TYPE_LAST_COLOR_INT){
        return (int)tv.data;
    }
    if (tv.type == Res_value::TYPE_INT_HEX) {
        return (int)tv.data;
    }
    return 0;
}

float ResourcesImpl::getDimension(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true) || tv.type != Res_value::TYPE_DIMENSION) return 0.0f;
    return tv.complexToDimension(mMetrics);
}

int ResourcesImpl::getDimensionPixelOffset(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true) || tv.type != Res_value::TYPE_DIMENSION) return 0;
    return tv.complexToDimensionPixelOffset(mMetrics);
}

int ResourcesImpl::getDimensionPixelSize(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true) || tv.type != Res_value::TYPE_DIMENSION) return 0;
    return tv.complexToDimensionPixelSize(mMetrics);
}

float ResourcesImpl::getFraction(int id, float base, float pbase) const {
    TypedValue tv;
    if (!getValue(id, &tv, true) || tv.type != Res_value::TYPE_FRACTION) return 0.0f;
    const float f = cdroid::complexToFloat(tv.data);
    const int unit = (tv.data >> Res_value::COMPLEX_UNIT_SHIFT) & Res_value::COMPLEX_UNIT_MASK;
    return (unit == Res_value::COMPLEX_UNIT_FRACTION_PARENT) ? f * pbase : f * base;
}

std::string ResourcesImpl::getQuantityString(int id, int /*quantity*/) const {
    return getString(id);   // plural selection deferred (needs ICU plural rules)
}

std::u16string ResourcesImpl::getQuantityText(int id, int /*quantity*/) const {
    return getText(id);
}

bool ResourcesImpl::pathOf(int id, std::string* out) const {
    TypedValue tv;
    if (!getValue(id, &tv, true) || tv.type != Res_value::TYPE_STRING) return false;
    if (out) *out = u16to8(tv.string, tv.stringLen);
    return true;
}

// Open an arsc-recorded file path against the pak layout. Two normalizations
// (candidates generated by androidfw pakPathCandidates):
//  - arsc paths carry the "res/" prefix, pakbuilder strips it when packing
//    (pak holds "layout/main.xml", not "res/layout/main.xml"). Retry without
//    the prefix so binary getXml(int) / openRawResource(int) resolve the same
//    way text inflate always has.
//  - aapt2 auto-appends a "-vN" version suffix to config-qualified dirs
//    (drawable-hdpi -> "drawable-hdpi-v4") — standard apk layout, unaffected by
//    manifest/uses-sdk or --min-sdk-version — while the pak stores the source
//    dir name. Retry with "-vN" stripped from directory segments so
//    hdpi/land/... resources open without repacking.
Asset* ResourcesImpl::openPakPath(const std::string& path) const {
    std::vector<std::string> cands;
    pakPathCandidates(path, cands);
    for (const auto& c : cands) {
        Asset* a = mAssets->openNonAsset(c.c_str(), Asset::ACCESS_BUFFER);
        if (a) return a;
    }
    return nullptr;
}

Asset* ResourcesImpl::openByStringId(int id) const {
    std::string path;
    if (!pathOf(id, &path) || path.empty()) return nullptr;
    return openPakPath(path);
}

Asset* ResourcesImpl::openRawResource(int id, TypedValue* outValue) const {
    TypedValue tv;
    if (!getValue(id, &tv, true)) return nullptr;
    if (outValue != nullptr) *outValue = tv;
    if (tv.type != Res_value::TYPE_STRING) return nullptr;
    const std::string path = u16to8(tv.string, tv.stringLen);
    if (path.empty()) return nullptr;
    return openPakPath(path);
}

Asset* ResourcesImpl::getXml(int id) const {
    return openByStringId(id);
}

// ---- GUI-object factories: ResourcesImpl owns the AOSP mDrawableCache /
// mComplexColorCache + loadDrawable/loadComplexColor (cairo + the Context
// inflation bridge are available now that ResourcesImpl is in the cdroid
// target). Defined here: DrawableCache/ColorStateListCache own GUI types
// (Drawable::ConstantState is nested; ColorStateList) the header can only
// forward-declare. ----

// AOSP ThemedResourceCache key: (resource id, theme). CDROID's engine is an
// opaque ResTable::Theme* kept stable per applied theme (Assets::setTheme
// rebuilds it), so the pointer identifies the theme — pairs of
// (theme, id) never collide across themes.
static uint64_t themedCacheKey(int id, const void* themeEngine) {
    return ((uint64_t)(uintptr_t)themeEngine << 32) | (uint32_t)id;
}

class ResourcesImpl::DrawableCache {
public:
    void clear() { mEntries.clear(); }   // AOSP onConfigurationChange drops entries
    std::shared_ptr<Drawable::ConstantState> get(uint64_t key) {
        auto it = mEntries.find(key);
        if (it == mEntries.end()) return nullptr;
        if (it->second.expired()) { mEntries.erase(it); return nullptr; }
        return it->second.lock();
    }
    void put(uint64_t key, const std::shared_ptr<Drawable::ConstantState>& cs) {
        if (cs) mEntries[key] = cs;
    }
private:
    std::unordered_map<uint64_t, std::weak_ptr<Drawable::ConstantState>> mEntries;
};

class ResourcesImpl::ColorStateListCache {
public:
    void clear() { mEntries.clear(); }   // AOSP onConfigurationChange drops entries
    std::shared_ptr<ColorStateList> get(uint64_t key) const {
        auto it = mEntries.find(key);
        return it == mEntries.end() ? nullptr : it->second;
    }
    void put(uint64_t key, const std::shared_ptr<ColorStateList>& csl) {
        if (csl) mEntries[key] = csl;
    }
private:
    std::unordered_map<uint64_t, std::shared_ptr<ColorStateList>> mEntries;
};

// AOSP ResourcesImpl.updateConfiguration(config, metrics, compat): apply the
// new configuration; the change bits drive resource-variant reselection
// (arsc setParameters) and resource-cache invalidation.
void ResourcesImpl::updateConfiguration(const Configuration* config, const DisplayMetrics* metrics) {
    if (metrics != nullptr) mMetrics = *metrics;
    const int changes = calcConfigChanges(config);
    if (config != nullptr) mConfiguration = *config;
    if (changes == 0) return;

    // AOSP: metrics follow densityDpi / fontScale.
    if (mConfiguration.densityDpi != Configuration::DENSITY_DPI_UNDEFINED) {
        mMetrics.densityDpi = mConfiguration.densityDpi;
        mMetrics.density = mConfiguration.densityDpi * (1.0f / DisplayMetrics::DENSITY_DEFAULT_SCALE);
    }
    mMetrics.scaledDensity = mMetrics.density *
            (mConfiguration.fontScale != 0 ? mConfiguration.fontScale : 1.0f);

    // AOSP mAssets.setConfigurationInternal(...): reselect resource variants.
    ResTable_config cfg = toResTableConfig(mConfiguration, mMetrics);
    // getResources() is const (AOSP facade); the underlying table is a mutable
    // cache (AssetManager owns it via mutable members), so the cast is safe.
    const_cast<ResTable&>(mAssets->getResources(false)).setParameters(&cfg);
    *mConfig = cfg;

    // AOSP: mDrawableCache/mColorDrawableCache/mComplexColorCache/...
    // .onConfigurationChange(changes) — CDROID drops the cached entries.
    if (mDrawableCache) mDrawableCache->clear();
    if (mColorStateListCache) mColorStateListCache->clear();
}

// AOSP Resources.getDrawable(id) → getDrawableForDensity(id, 0).
cdroid::Drawable* ResourcesImpl::getDrawable(int id, int density, const void* themeEngine) const {
    return getDrawableForDensity(id, density, themeEngine);
}

// AOSP Resources.getDrawableForDensity(id, density) → ResourcesImpl.loadDrawable:
//   1. cache hit → ConstantState::newDrawable()        (mDrawableCache)
//   2. getValue(id) → TypedValue
//   3. TYPE_FIRST/LAST_COLOR_INT → ColorDrawable(data)  (AOSP isColorDrawable)
//   4. TYPE_STRING (file/xml) → inflate via Context     (loadDrawableForCookie)
//   5. cache the ConstantState, return the drawable
cdroid::Drawable* ResourcesImpl::getDrawableForDensity(int id, int /*density*/, const void* themeEngine) const {
    if (id == 0 || mCtx == nullptr) return nullptr;
    if (mDrawableCache) {
        if (auto cs = mDrawableCache->get(themedCacheKey(id, themeEngine))) return cs->newDrawable();
    }
    TypedValue value;
    if (!getValue(id, &value, true)) return nullptr;
    Drawable* d = nullptr;
    if (value.type >= TypedValue::TYPE_FIRST_COLOR_INT &&
        value.type <= TypedValue::TYPE_LAST_COLOR_INT) {
        d = new ColorDrawable(value.data);
    } else if (value.type == TypedValue::TYPE_STRING) {
        // AOSP loadDrawableForCookie: ResourcesImpl loads the file drawable ITSELF
        // (opens by id, inflates) — not via Context.getDrawable(string). value.string
        // is the file path; .xml → DrawableInflater (id-based parser + inflateFromXml,
        // which already takes Resources&), else → ImageDecoder::createAsDrawable(id).
        std::string path = u16to8(value.string, value.stringLen);
        if (path.find(".xml") != std::string::npos) {
            XmlPullParser parser(mCtx, id);
            int type;
            while ((type = parser.next()) != XmlPullParser::START_TAG &&
                   type != XmlPullParser::END_DOCUMENT) {}
            if (type == XmlPullParser::START_TAG) {
                const AttributeSet& attrs = parser;
                // AOSP loadDrawableForCookie inflates with a null theme and
                // re-applies via applyTheme(); CDROID has no mThemeAttrs
                // deferred machinery, so the theme goes straight into the
                // inflation (the same route AOSP uses for ColorStateList).
                if (themeEngine) {
                    Resources::Theme themed(mCtx->getResources(),
                                            const_cast<void*>(themeEngine));
                    d = DrawableInflater::inflateFromXml(mCtx->getResources(),
                                                         parser.getName(), parser, attrs, &themed);
                } else {
                    d = DrawableInflater::inflateFromXml(mCtx->getResources(),
                                                         parser.getName(), parser, attrs);
                }
            }
        } else {
            d = ImageDecoder::createAsDrawable(mCtx, id);
        }
    }
    if (d && mDrawableCache) mDrawableCache->put(themedCacheKey(id, themeEngine), d->getConstantState());
    return d;
}

// AOSP Resources.getColorStateList(id) → loadComplexColor (CSL branch). The
// cached instance (shared_ptr) keeps the ColorStateList alive.
std::shared_ptr<cdroid::ColorStateList> ResourcesImpl::getColorStateList(int id, const void* themeEngine) const {
    auto cc = loadComplexColor(id, themeEngine);
    return cc ? std::dynamic_pointer_cast<ColorStateList>(cc) : nullptr;
}

// AOSP ResourcesImpl.loadComplexColor:
//   1. cache hit → return cached instance              (mColorStateListCache)
//   2. getValue(id) → TypedValue
//   3. TYPE_FIRST/LAST_COLOR_INT → valueOf(data)        (getColorStateListFromInt)
//   4. TYPE_STRING (xml) → createFromXml(theme)          (loadComplexColorForCookie)
//   5. cache the instance, return it
std::shared_ptr<cdroid::ComplexColor> ResourcesImpl::loadComplexColor(int id, const void* themeEngine) const {
    if (id == 0 || mCtx == nullptr) return nullptr;
    if (mColorStateListCache) {
        if (auto csl = mColorStateListCache->get(themedCacheKey(id, themeEngine))) return csl;
    }
    TypedValue value;
    if (!getValue(id, &value, true)) return nullptr;
    std::shared_ptr<ColorStateList> csl;
    if (value.type >= TypedValue::TYPE_FIRST_COLOR_INT &&
        value.type <= TypedValue::TYPE_LAST_COLOR_INT) {
        csl = ColorStateList::valueOf(value.data);
    } else {
        // Binary face: load by resource id (getXml → openByStringId strips the
        // arsc's "res/" prefix). The string ctor can't open "pkg:type/key" refs
        // from a binary pak (no text path table). AOSP passes the theme into
        // createFromXml so ?attr inside the selector resolves against it.
        try {
            XmlPullParser parser(mCtx, id);
            csl = ColorStateList::createFromXml(mCtx->getResources(), parser,
                                                (ResTable::Theme*)themeEngine);
        } catch (const std::exception&) {
            csl = nullptr;
        }
    }
    if (csl && mColorStateListCache) {
        mColorStateListCache->put(themedCacheKey(id, themeEngine), csl);
    }
    return csl;
}

cdroid::Typeface* ResourcesImpl::getFont(int /*id*/) const {
    return nullptr;
}

cdroid::Movie* ResourcesImpl::getMovie(int /*id*/) const {
    return nullptr;
}

