// Port of AOSP android.content.res.Resources (+ ResourcesImpl) — value/meta/asset
// resolution layer. See resources.h for the isolated-port contract (GUI-object
// factories are stubbed, filled at the cdroid.so merge).

#define LOG_TAG "resources"

#include "resourcesimpl.h"

#include <porting/cdlog.h>

#include <cstring>
#include <memory>
#include <string>

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
    if (config != nullptr) mConfig = *config; else memset(&mConfig, 0, sizeof(mConfig));
    if (metrics != nullptr) mMetrics = *metrics;  // else default density=1
}

ResourcesImpl::~ResourcesImpl() {
}

// AOSP Resources.newTheme(): a Theme over this ResourcesImpl's AssetManager
// table. The engine is cdroid::ResTable::Theme (aliased as ResourcesImpl::Theme);
// it owns no state until applyStyle() is called on it.
std::unique_ptr<ResourcesImpl::Theme> ResourcesImpl::newTheme() {
    if (mAssets == nullptr) return nullptr;
    return std::make_unique<Theme>(mAssets->getResources(false));
}

int ResourcesImpl::getIdentifier(const std::string& name, const std::string& type,
                             const std::string& package) const {
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
    if (tv.type >= Res_value::TYPE_FIRST_INT && tv.type <= Res_value::TYPE_LAST_INT) return (int)tv.data;
    if (tv.type == Res_value::TYPE_INT_BOOLEAN) return tv.data != 0 ? 1 : 0;
    return 0;
}

bool ResourcesImpl::getBoolean(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true)) return false;
    if (tv.type == Res_value::TYPE_INT_BOOLEAN) return tv.data != 0;
    if (tv.type >= Res_value::TYPE_FIRST_INT && tv.type <= Res_value::TYPE_LAST_INT) return tv.data != 0;
    return false;
}

float ResourcesImpl::getFloat(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true)) return 0.0f;
    if (tv.type == Res_value::TYPE_FLOAT) return tv.getFloat();
    if (tv.type >= Res_value::TYPE_FIRST_INT && tv.type <= Res_value::TYPE_LAST_INT) return (float)(int)tv.data;
    return 0.0f;
}

int ResourcesImpl::getColor(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true)) return 0;
    if (tv.type >= Res_value::TYPE_FIRST_COLOR_INT && tv.type <= Res_value::TYPE_LAST_COLOR_INT) return (int)tv.data;
    if (tv.type == Res_value::TYPE_INT_HEX) return (int)tv.data;
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

Asset* ResourcesImpl::openByStringId(int id) const {
    std::string path;
    if (!pathOf(id, &path) || path.empty()) return nullptr;
    Asset* a = mAssets->openNonAsset(path.c_str(), Asset::ACCESS_BUFFER);
    if (a) return a;
    // arsc records the full "res/..." path, but pakbuilder strips the "res/"
    // prefix when packing entries (pak holds "layout/main.xml", not
    // "res/layout/main.xml"). Retry without the prefix so binary getXml(int) /
    // openRawResource(int) resolve the same way text inflate always has.
    if (path.compare(0, 4, "res/") == 0)
        a = mAssets->openNonAsset(path.substr(4).c_str(), Asset::ACCESS_BUFFER);
    return a;
}

Asset* ResourcesImpl::openRawResource(int id, TypedValue* outValue) const {
    TypedValue tv;
    if (!getValue(id, &tv, true)) return nullptr;
    if (outValue != nullptr) *outValue = tv;
    if (tv.type != Res_value::TYPE_STRING) return nullptr;
    const std::string path = u16to8(tv.string, tv.stringLen);
    if (path.empty()) return nullptr;
    Asset* a = mAssets->openNonAsset(path.c_str(), Asset::ACCESS_BUFFER);
    if (a) return a;
    // Same res/ strip as openByStringId: arsc stores "res/...", pak omits "res/".
    if (path.compare(0, 4, "res/") == 0)
        a = mAssets->openNonAsset(path.substr(4).c_str(), Asset::ACCESS_BUFFER);
    return a;
}

Asset* ResourcesImpl::getXml(int id) const {
    return openByStringId(id);
}

// ---- GUI-object factory stubs (isolated port; overridden by cdroid::Resources
// at the cdroid.so merge). ----

cdroid::Drawable* ResourcesImpl::getDrawable(int /*id*/, int /*density*/) const { return nullptr; }
cdroid::Drawable* ResourcesImpl::getDrawableForDensity(int /*id*/, int /*density*/) const { return nullptr; }
cdroid::ColorStateList* ResourcesImpl::getColorStateList(int /*id*/) const { return nullptr; }
cdroid::Typeface* ResourcesImpl::getFont(int /*id*/) const { return nullptr; }
cdroid::ComplexColor* ResourcesImpl::loadComplexColor(int /*id*/) const { return nullptr; }
cdroid::Movie* ResourcesImpl::getMovie(int /*id*/) const { return nullptr; }

