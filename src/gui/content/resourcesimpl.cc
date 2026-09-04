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
#include <fstream>
#include <sstream>

// androidfw native readers — hidden from resourcesimpl.h (the facade contract).
#include <content/androidfw/restable.h>       // ResTable, ResTable_config, Res_value, pakPathCandidates
#include <content/androidfw/LocaleData.h>     // localeDataComputeScript (arsc locale config)
#include <content/LocaleList.h>               // LocaleList::getDefault (start-up locale seed)
#include <content/assetmanager.h>        // AssetManager
#include <content/asset.h>               // Asset
#include <content/typedvalue.h>     // TypedValue
#include <core/typeface.h>               // Typeface (getFont → createFromResourcePath)

#include <drawable/drawable.h>        // Drawable::ConstantState
#include <drawable/colordrawable.h>   // ColorDrawable (color-drawable path)
#include <drawable/colorstatelist.h>  // ColorStateList cache + createFromXml
#include <drawable/drawableinflater.h>  // DrawableInflater::inflateFromXml (xml drawable self-load)
#include <image-decoders/imagedecoder.h>  // ImageDecoder::createAsDrawable(id) (image self-load)
#include <core/context.h>             // inflation bridge (mCtx)
#include <core/xmlpullparser.h>       // ColorStateList::createFromXml inline inflate
#include <animation/animator.h>             // AnimatorCache: ConstantState<Animator*>
#include <animation/statelistanimator.h>    // StateListAnimatorCache

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
    // ...and from the device locale. On CDROID the "device" locale is the
    // POSIX environment (LC_ALL/LC_MESSAGES/LANG), which Locale::getDefault()
    // already parses — seed it so framework strings (ime_action_*, date
    // formats, ...) resolve localized from process start. An explicit
    // updateConfiguration (printerdemo's language switch) still overrides.
    if (mConfiguration.getLocales().isEmpty()) {
        mConfiguration.setLocales(LocaleList::getDefault());
    }
    if (metrics != nullptr) mConfiguration.densityDpi = mMetrics.densityDpi;
    mDrawableCache = std::make_unique<DrawableCache>();
    mColorStateListCache = std::make_unique<ColorStateListCache>();
    mAnimatorCache = std::make_unique<AnimatorCache>();
    mStateListAnimatorCache = std::make_unique<StateListAnimatorCache>();
}

ResourcesImpl::~ResourcesImpl() {
}


// AOSP ResourcesImpl.updateConfigurationImpl → mAssets.setConfigurationInternal:
// map the live Configuration onto the arsc ResTable_config so -night/-land/...
// resource variants reselect. The locale comes from the Configuration's
// LocaleList primary (getLocales() reconciles the deprecated tag-string field);
// language/region are packed the way AOSP's JNI bridge does (language 2-3
// chars, region 2 chars/3 digits, script char[4] when the locale carries one).
static ResTable_config toResTableConfig(const Configuration& c, const DisplayMetrics& m) {
    ResTable_config cfg = {};
    cfg.size = sizeof(ResTable_config);
    cfg.mcc = (uint16_t)c.mcc;
    cfg.mnc = (uint16_t)c.mnc;
    const Locale primary = c.getLocales().get(0);
    if (!primary.getLanguage().empty()) {
        cfg.packLanguage(primary.getLanguage().c_str());
        if (primary.getCountry().size() == 2) {
            cfg.packRegion(primary.getCountry().c_str());
        }
        const std::string script = primary.getScript();
        if (!script.empty()) {
            strncpy(cfg.localeScript, script.c_str(), sizeof(cfg.localeScript));
            cfg.localeScriptWasComputed = false;
        } else {
            // AOSP android_util_AssetManager: a locale without an explicit
            // script gets one computed from language+region.
            char computed[4] = {0, 0, 0, 0};
            localeDataComputeScript(computed, cfg.language, cfg.country);
            memcpy(cfg.localeScript, computed, 4);
            cfg.localeScriptWasComputed = true;
        }
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

std::unique_ptr<XmlPullParser> ResourcesImpl::loadXmlResourceParser(int resid) const {
    // ID-based path: fetch the bytes via the resource face. Text paks have
    // no arsc -> getXml returns null -> empty parser.
    Asset* asset = getXml(resid);
    std::unique_ptr<std::istream> strm;
    if(asset){
        const off64_t sz = asset->getLength();
        std::string buf((size_t)(sz > 0 ? sz : 0), '\0');
        if (sz > 0) asset->read(&buf[0], (size_t)sz);
        delete asset;
        strm = std::make_unique<std::istringstream>(buf);
    }
    return XmlPullParser::detectAndCreate(mCtx, std::move(strm), std::to_string(resid));
}

// ---- GUI-object factories: ResourcesImpl owns the AOSP mDrawableCache /
// mComplexColorCache + loadDrawable/loadComplexColor (cairo + the Context
// inflation bridge are available now that ResourcesImpl is in the cdroid
// target). Defined here: DrawableCache/ColorStateListCache own GUI types
// (Drawable::ConstantState is nested; ColorStateList) the header can only
// forward-declare. ----

// AOSP ThemedResourceCache key: (resource id, theme). The engine ADDRESS is
// not a safe theme identity — Assets::setTheme() rebuilds the engine with
// delete+new and the allocator typically hands the same block back, so a
// toggled theme would collide with (and be served) the previous theme's
// cached drawables/CSLs. Key on the theme's monotonic generation instead.
static uint64_t themedCacheKey(int id, const void* themeEngine) {
    const uint64_t theme = themeEngine
            ? static_cast<const ResTable::Theme*>(themeEngine)->cacheGeneration()
            : 0;
    return (theme << 32) | (uint32_t)id;
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

// AOSP ConfigurationBoundResourceCache<Animator> / <StateListAnimator> on
// ResourcesImpl. Entries are ConstantStates; getInstance == get + newInstance,
// so callers NEVER receive the cached source animator (AOSP: "create a new
// animator so that cached version is never used by the user"). Shared (not
// weak) storage: the ConstantState is the sole owner of the source animator —
// a weak entry would expire the moment the put() call returns.
class ResourcesImpl::AnimatorCache {
public:
    void clear() { mEntries.clear(); }   // AOSP onConfigurationChange drops entries
    std::shared_ptr<ConstantState<Animator*>> get(uint64_t key) const {
        auto it = mEntries.find(key);
        return it == mEntries.end() ? nullptr : it->second;
    }
    void put(uint64_t key, const std::shared_ptr<ConstantState<Animator*>>& cs) {
        if (cs) mEntries[key] = cs;
    }
private:
    std::unordered_map<uint64_t, std::shared_ptr<ConstantState<Animator*>>> mEntries;
};

class ResourcesImpl::StateListAnimatorCache {
public:
    void clear() { mEntries.clear(); }   // AOSP onConfigurationChange drops entries
    std::shared_ptr<ConstantState<StateListAnimator*>> get(uint64_t key) const {
        auto it = mEntries.find(key);
        return it == mEntries.end() ? nullptr : it->second;
    }
    void put(uint64_t key, const std::shared_ptr<ConstantState<StateListAnimator*>>& cs) {
        if (cs) mEntries[key] = cs;
    }
private:
    std::unordered_map<uint64_t, std::shared_ptr<ConstantState<StateListAnimator*>>> mEntries;
};

// AOSP ResourcesImpl.updateConfiguration(config, metrics, compat): apply the
// new configuration; the change bits drive resource-variant reselection
// (arsc setParameters) and resource-cache invalidation.
void ResourcesImpl::updateConfiguration(const Configuration* config, const DisplayMetrics* metrics) {
    if (metrics != nullptr) mMetrics = *metrics;
    const int changes = calcConfigChanges(config);
    if (config != nullptr) mConfiguration = *config;

    // AOSP updateConfigurationImpl: if even after the update there are no
    // Locales set, grab the default locales. And when the locale list changed
    // and has more than one entry, pick the best match among the locales the
    // resources actually carry (getNonSystemLocales→getLocales upstream; the
    // LocaleConfig/default-locale Flags path is not ported), reordering the
    // configuration so the best locale becomes primary.
    bool localesChanged = false;
    LocaleList locales = mConfiguration.getLocales();
    if (locales.isEmpty()) {
        locales = LocaleList::getDefault();
        mConfiguration.setLocales(locales);
        localesChanged = true;
    }
    if ((changes & Configuration::CONFIG_LOCALE) != 0 && locales.size() > 1) {
        std::vector<std::string> availableLocales;
        // getResources() is const (AOSP facade); the table is a mutable cache.
        const_cast<ResTable&>(mAssets->getResources(false)).getLocales(availableLocales);
        if (LocaleList::isPseudoLocalesOnly(&availableLocales)) {
            availableLocales.clear();
        }
        if (!availableLocales.empty()) {
            const Locale bestLocale = locales.getFirstMatchWithEnglishSupported(availableLocales);
            if (!bestLocale.getLanguage().empty() && !(bestLocale == locales.get(0))) {
                mConfiguration.setLocales(LocaleList(bestLocale, &locales));
                localesChanged = true;
            }
        }
    }
    if (changes == 0 && !localesChanged) return;

    // AOSP: metrics follow densityDpi / fontScale.
    if (mConfiguration.densityDpi != Configuration::DENSITY_DPI_UNDEFINED) {
        mMetrics.densityDpi = mConfiguration.densityDpi;
        // AOSP: mMetrics.density = densityDpi * DENSITY_DEFAULT_SCALE (the
        // scale is dpi→density = 1/160; a stray reciprocal here made density
        // 160*160=25600 and every sp dimen 25600x too large).
        mMetrics.density = mConfiguration.densityDpi * DisplayMetrics::DENSITY_DEFAULT_SCALE;
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
    // AOSP mAnimatorCache/mStateListAnimatorCache.onConfigurationChange(changes)
    // — ConfigurationBoundResourceCache prunes by changingConfigs and bumps its
    // generation; CDROID drops the entries wholesale (same policy as above).
    if (mAnimatorCache) mAnimatorCache->clear();
    if (mStateListAnimatorCache) mStateListAnimatorCache->clear();
}

// AOSP ResourcesImpl.getAnimatorCache().getInstance(id, resources, theme): a
// hit already applies newInstance(), so callers never receive the cached
// source animator ("create a new animator so that cached version is never
// used by the user").
Animator* ResourcesImpl::obtainCachedAnimator(int id, const void* themeEngine) const {
    if (!mAnimatorCache) return nullptr;
    const std::shared_ptr<ConstantState<Animator*>> cs =
            mAnimatorCache->get(themedCacheKey(id, themeEngine));
    return cs ? cs->newInstance() : nullptr;
}

void ResourcesImpl::cacheAnimator(int id, const void* themeEngine,
        const std::shared_ptr<ConstantState<Animator*>>& cs) const {
    if (mAnimatorCache) mAnimatorCache->put(themedCacheKey(id, themeEngine), cs);
}

StateListAnimator* ResourcesImpl::obtainCachedStateListAnimator(int id, const void* themeEngine) const {
    if (!mStateListAnimatorCache) return nullptr;
    const std::shared_ptr<ConstantState<StateListAnimator*>> cs =
            mStateListAnimatorCache->get(themedCacheKey(id, themeEngine));
    return cs ? cs->newInstance() : nullptr;
}

void ResourcesImpl::cacheStateListAnimator(int id, const void* themeEngine,
        const std::shared_ptr<ConstantState<StateListAnimator*>>& cs) const {
    if (mStateListAnimatorCache) mStateListAnimatorCache->put(themedCacheKey(id, themeEngine), cs);
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
        // AOSP ResourcesImpl.loadDrawable delegates TYPE_STRING entirely to
        // DrawableInflater.loadDrawableForDensity: .xml inflates themed (AOSP
        // loadDrawableForCookie inflates null-themed then re-applies
        // applyTheme(); CDROID has no mThemeAttrs deferred machinery, so the
        // theme goes straight into the inflation — the same route AOSP uses
        // for ColorStateList), other files decode through ImageDecoder
        // (createSource(Resources, id)).
        if (themeEngine) {
            Resources::Theme themed(mCtx->getResources(),
                                    const_cast<void*>(themeEngine));
            d = DrawableInflater::loadDrawableForDensity(mCtx->getResources(), value, id, 0, &themed);
        } else {
            d = DrawableInflater::loadDrawableForDensity(mCtx->getResources(), value, id, 0, nullptr);
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
            auto parser = loadXmlResourceParser(id);
            // Wrap the caller's engine handle in the Theme view (same shape as
            // the themed drawable load above) for ColorStateList::createFromXml.
            Resources::Theme themed(mCtx->getResources(), const_cast<void*>(themeEngine));
            csl = ColorStateList::createFromXml(mCtx->getResources(), *parser,
                                                themeEngine ? &themed : nullptr);
        } catch (const std::exception&) {
            csl = nullptr;
        }
    }
    if (csl && mColorStateListCache) {
        mColorStateListCache->put(themedCacheKey(id, themeEngine), csl);
    }
    return csl;
}

cdroid::Typeface* ResourcesImpl::getFont(int id) const {
    // AOSP Resources.getFont: the value of a raw-ttf R.font entry is the
    // pak-relative file path (TYPE_STRING); Typeface.createFromResources loads
    // it via openNonAsset with a per-path instance cache.
    TypedValue value;
    if (!getValue(id, &value, true) || value.type != Res_value::TYPE_STRING) return nullptr;
    std::string path = u16to8(value.string, value.stringLen);
    // Font resource values are pak-relative FILE paths ("font/x.ttf");
    // plain family names ("sans-serif") that reached here through a string
    // attr are not font resources — return null like AOSP instead of routing
    // them into the asset loader.
    if (path.find('/') == std::string::npos) return nullptr;
    return cdroid::Typeface::createFromResourcePath(path);
}

cdroid::Movie* ResourcesImpl::getMovie(int /*id*/) const {
    return nullptr;
}

