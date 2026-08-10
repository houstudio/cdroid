// Port of AOSP android.content.res.Resources (+ ResourcesImpl) — value/meta/asset
// resolution layer. See resources.h for the isolated-port contract (GUI-object
// factories are stubbed, filled at the cdroid.so merge).

#define LOG_TAG "resources"

#include "resources.h"

#include <porting/cdlog.h>

#include <cstring>
#include <memory>
#include <string>

using namespace android;
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

// ---- applyDimension (faithful to AOSP TypedValue.applyDimension) ----
float android::applyDimension(int unit, float value, const DisplayMetrics& m) {
    switch (unit) {
    case Res_value::COMPLEX_UNIT_PX:  return value;
    case Res_value::COMPLEX_UNIT_DIP: return value * m.density;
    case Res_value::COMPLEX_UNIT_SP:  return value * m.scaledDensity;
    case Res_value::COMPLEX_UNIT_PT:  return value * m.xdpi * (1.0f / 72.0f);
    case Res_value::COMPLEX_UNIT_IN:  return value * m.xdpi;
    case Res_value::COMPLEX_UNIT_MM:  return value * m.xdpi * (1.0f / 25.4f);
    default:                          return 0.0f;
    }
}

float TypedValue::complexToFloat(uint32_t data) { return cdroid::complexToFloat(data); }

float TypedValue::getFloat() const {
    float f;
    memcpy(&f, &data, sizeof(f));
    return f;
}

float TypedValue::complexToDimension(const DisplayMetrics& m) const {
    const float value = cdroid::complexToFloat(data);
    const int unit = (data >> Res_value::COMPLEX_UNIT_SHIFT) & Res_value::COMPLEX_UNIT_MASK;
    return applyDimension(unit, value, m);
}

int TypedValue::complexToDimensionPixelOffset(const DisplayMetrics& m) const {
    return (int)complexToDimension(m);
}

int TypedValue::complexToDimensionPixelSize(const DisplayMetrics& m) const {
    const float mag = cdroid::complexToFloat(data);
    const int unit = (data >> Res_value::COMPLEX_UNIT_SHIFT) & Res_value::COMPLEX_UNIT_MASK;
    const float f = applyDimension(unit, mag, m);
    const int res = (int)(f + 0.5f);
    if (res != 0) return res;
    if (mag == 0.0f) return 0;
    return mag > 0 ? 1 : -1;
}

// ---- Resources ----

Resources::Resources(AssetManager* am, const ResTable_config* config,
                     const DisplayMetrics* metrics) : mAssets(am) {
    if (config != nullptr) mConfig = *config; else memset(&mConfig, 0, sizeof(mConfig));
    if (metrics != nullptr) mMetrics = *metrics;  // else default density=1
}

Resources::~Resources() {
}

// AOSP Resources.newTheme(): a Theme over this Resources' AssetManager table.
// The engine is cdroid::ResTable::Theme (aliased as Resources::Theme); it owns
// no state until applyStyle() is called on it.
std::unique_ptr<Resources::Theme> Resources::newTheme() {
    if (mAssets == nullptr) return nullptr;
    return std::make_unique<Theme>(mAssets->getResources(false));
}

int Resources::getIdentifier(const std::string& name, const std::string& type,
                             const std::string& package) const {
    if (mAssets == nullptr) return 0;
    return (int)mAssets->getResources(false).getIdentifier(name, type, package);
}

bool Resources::getResourceName(int id, std::string* out) const {
    std::string pkg, type, key;
    if (mAssets == nullptr) return false;
    if (!mAssets->getResources(false).getResourceName((uint32_t)id, &pkg, &type, &key)) return false;
    if (out) *out = pkg + ":" + type + "/" + key;
    return true;
}

bool Resources::getResourcePackageName(int id, std::string* out) const {
    std::string pkg, type, key;
    if (mAssets == nullptr) return false;
    if (!mAssets->getResources(false).getResourceName((uint32_t)id, &pkg, &type, &key)) return false;
    if (out) *out = pkg;
    return true;
}

bool Resources::getResourceTypeName(int id, std::string* out) const {
    std::string pkg, type, key;
    if (mAssets == nullptr) return false;
    if (!mAssets->getResources(false).getResourceName((uint32_t)id, &pkg, &type, &key)) return false;
    if (out) *out = type;
    return true;
}

bool Resources::getResourceEntryName(int id, std::string* out) const {
    std::string pkg, type, key;
    if (mAssets == nullptr) return false;
    if (!mAssets->getResources(false).getResourceName((uint32_t)id, &pkg, &type, &key)) return false;
    if (out) *out = key;
    return true;
}

bool Resources::getValue(int id, TypedValue* outValue, bool resolveRefs) const {
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

bool Resources::getValue(const std::string& name, TypedValue* outValue, bool resolveRefs) const {
    const int id = getIdentifier(name, "", "");
    if (id == 0) return false;
    return getValue(id, outValue, resolveRefs);
}

std::string Resources::getString(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true) || tv.type != Res_value::TYPE_STRING) return std::string();
    return u16to8(tv.string, tv.stringLen);
}

std::u16string Resources::getText(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true) || tv.type != Res_value::TYPE_STRING) return std::u16string();
    return std::u16string(tv.string, tv.stringLen);
}

int Resources::getInteger(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true)) return 0;
    if (tv.type >= Res_value::TYPE_FIRST_INT && tv.type <= Res_value::TYPE_LAST_INT) return (int)tv.data;
    if (tv.type == Res_value::TYPE_INT_BOOLEAN) return tv.data != 0 ? 1 : 0;
    return 0;
}

bool Resources::getBoolean(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true)) return false;
    if (tv.type == Res_value::TYPE_INT_BOOLEAN) return tv.data != 0;
    if (tv.type >= Res_value::TYPE_FIRST_INT && tv.type <= Res_value::TYPE_LAST_INT) return tv.data != 0;
    return false;
}

float Resources::getFloat(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true)) return 0.0f;
    if (tv.type == Res_value::TYPE_FLOAT) return tv.getFloat();
    if (tv.type >= Res_value::TYPE_FIRST_INT && tv.type <= Res_value::TYPE_LAST_INT) return (float)(int)tv.data;
    return 0.0f;
}

int Resources::getColor(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true)) return 0;
    if (tv.type >= Res_value::TYPE_FIRST_COLOR_INT && tv.type <= Res_value::TYPE_LAST_COLOR_INT) return (int)tv.data;
    if (tv.type == Res_value::TYPE_INT_HEX) return (int)tv.data;
    return 0;
}

float Resources::getDimension(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true) || tv.type != Res_value::TYPE_DIMENSION) return 0.0f;
    return tv.complexToDimension(mMetrics);
}

int Resources::getDimensionPixelOffset(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true) || tv.type != Res_value::TYPE_DIMENSION) return 0;
    return tv.complexToDimensionPixelOffset(mMetrics);
}

int Resources::getDimensionPixelSize(int id) const {
    TypedValue tv;
    if (!getValue(id, &tv, true) || tv.type != Res_value::TYPE_DIMENSION) return 0;
    return tv.complexToDimensionPixelSize(mMetrics);
}

float Resources::getFraction(int id, float base, float pbase) const {
    TypedValue tv;
    if (!getValue(id, &tv, true) || tv.type != Res_value::TYPE_FRACTION) return 0.0f;
    const float f = cdroid::complexToFloat(tv.data);
    const int unit = (tv.data >> Res_value::COMPLEX_UNIT_SHIFT) & Res_value::COMPLEX_UNIT_MASK;
    return (unit == Res_value::COMPLEX_UNIT_FRACTION_PARENT) ? f * pbase : f * base;
}

std::string Resources::getQuantityString(int id, int /*quantity*/) const {
    return getString(id);   // plural selection deferred (needs ICU plural rules)
}

std::u16string Resources::getQuantityText(int id, int /*quantity*/) const {
    return getText(id);
}

bool Resources::pathOf(int id, std::string* out) const {
    TypedValue tv;
    if (!getValue(id, &tv, true) || tv.type != Res_value::TYPE_STRING) return false;
    if (out) *out = u16to8(tv.string, tv.stringLen);
    return true;
}

Asset* Resources::openByStringId(int id) const {
    std::string path;
    if (!pathOf(id, &path) || path.empty()) return nullptr;
    return mAssets->openNonAsset(path.c_str(), Asset::ACCESS_BUFFER);
}

Asset* Resources::openRawResource(int id, TypedValue* outValue) const {
    TypedValue tv;
    if (!getValue(id, &tv, true)) return nullptr;
    if (outValue != nullptr) *outValue = tv;
    if (tv.type != Res_value::TYPE_STRING) return nullptr;
    const std::string path = u16to8(tv.string, tv.stringLen);
    if (path.empty()) return nullptr;
    return mAssets->openNonAsset(path.c_str(), Asset::ACCESS_BUFFER);
}

Asset* Resources::getXml(int id) const {
    return openByStringId(id);
}

// ---- GUI-object factory stubs (isolated port; overridden by cdroid::Resources
// at the cdroid.so merge). ----

cdroid::Drawable* Resources::getDrawable(int /*id*/, int /*density*/) const { return nullptr; }
cdroid::Drawable* Resources::getDrawableForDensity(int /*id*/, int /*density*/) const { return nullptr; }
cdroid::ColorStateList* Resources::getColorStateList(int /*id*/) const { return nullptr; }
cdroid::Typeface* Resources::getFont(int /*id*/) const { return nullptr; }
cdroid::ComplexColor* Resources::loadComplexColor(int /*id*/) const { return nullptr; }
cdroid::Movie* Resources::getMovie(int /*id*/) const { return nullptr; }

