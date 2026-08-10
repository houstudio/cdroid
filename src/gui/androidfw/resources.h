// Port of AOSP android.content.res.Resources (+ ResourcesImpl) — the ID-based
// resource facade that sits on top of AssetManager.
//
// This is the ISOLATED port (androidfw sub-lib): value/meta/asset resolution is
// fully implemented via android::AssetManager + cdroid::ResTable. GUI-object
// factories (getDrawable/getColorStateList/getFont/...) are DECLARED with
// forward-declared return types and STUBBED (return nullptr) — their bodies are
// filled when this code is merged into cdroid.so by source inheritance (a
// libcdroid Resources subclass overrides them, inheriting the value methods).
//
// androidfw is an intermediate product; these types (TypedValue, DisplayMetrics)
// are reconciled with the cdroid core equivalents at merge time.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
#ifndef __CDROID_ANDROIDFW_RESOURCES_H__
#define __CDROID_ANDROIDFW_RESOURCES_H__

#include <cstdint>
#include <cstring>
#include <string>
#include <utility>

#include "restable.h"        // cdroid::ResTable, ResTable_config, Res_value, complexToFloat
#include "assetmanager.h"    // android::AssetManager
#include "asset.h"           // android::Asset

// Forward declarations of cdroid GUI object types — returned by pointer only, so
// the full GUI headers are NOT needed to compile this isolated port. Their
// factory methods are stubbed (return nullptr) until the cdroid.so merge.
namespace cdroid {
class Drawable;
class ColorStateList;
class Typeface;
class ComplexColor;
class Movie;
}  // namespace cdroid

namespace android {

// Minimal DisplayMetrics (port of android.util.DisplayMetrics), reduced to the
// fields Resources needs to convert dimensions to pixels. Reconciled with
// core/displaymetrics.h at the cdroid.so merge.
struct DisplayMetrics {
    float density        = 1.0f;   // logical density (DPI/160)
    float scaledDensity  = 1.0f;   // font-scale-adjusted density (sp scaling)
    int   densityDpi     = 160;    // exact screen dots per inch
    int   widthPixels    = 0;
    int   heightPixels   = 0;
    float xdpi           = 160.0f; // exact physical pixels per inch (X)
    float ydpi           = 160.0f; // exact physical pixels per inch (Y)
};

// Container for a single resolved resource value. Port of android.util.TypedValue.
// `string` borrows from the ResTable string pool and is valid while the owning
// AssetManager/ResTable is alive.
struct TypedValue {
    uint8_t  type = 0;                 // Res_value::TYPE_*
    uint32_t data = 0;                 // raw payload (int / complex / pool index)
    uint32_t resourceId = 0;           // the resource id this value came from
    uint32_t changingConfigurations = 0;
    int      density = 0;             // density of the config the value came from
    int      assetCookie = 0;         // owning ApkAssets cookie (reserved)
    const char16_t* string = nullptr; // TYPE_STRING payload (borrowed)
    size_t   stringLen = 0;

    // AOSP TypedValue helpers (defined in resources.cc).
    static float complexToFloat(uint32_t data);
    float getFloat() const;
    float complexToDimension(const DisplayMetrics& m) const;
    int   complexToDimensionPixelOffset(const DisplayMetrics& m) const;
    int   complexToDimensionPixelSize(const DisplayMetrics& m) const;
};

// Convert a (unit, value) pair to pixels. Faithful to AOSP
// TypedValue.applyDimension.
float applyDimension(int unit, float value, const DisplayMetrics& metrics);

// Resources: the ID-based resource facade over an AssetManager.
class Resources {
public:
    // `am` is NOT owned (must outlive this Resources). If config/metrics are null,
    // defaults (zeroed config, density=1) are used.
    Resources(AssetManager* am, const ResTable_config* config = nullptr,
              const DisplayMetrics* metrics = nullptr);
    virtual ~Resources();

    Resources(const Resources&) = delete;
    Resources& operator=(const Resources&) = delete;

    AssetManager* getAssets() { return mAssets; }

    // AOSP Resources.Theme — the engine is the already-ported ResTable::Theme
    // (applyStyle/getAttribute/resolveAttributeReference/clear/...).
    using Theme = cdroid::ResTable::Theme;
    std::unique_ptr<Theme> newTheme();   // a Theme over this Resources' AssetManager table
    const ResTable_config& getConfiguration() const { return mConfig; }
    const DisplayMetrics& getDisplayMetrics() const { return mMetrics; }
    void setConfiguration(const ResTable_config& config) { mConfig = config; }
    void setDisplayMetrics(const DisplayMetrics& m) { mMetrics = m; }

    // --- identifier / naming ---
    int  getIdentifier(const std::string& name, const std::string& type,
                       const std::string& package) const;
    bool getResourceName(int id, std::string* out) const;             // "pkg:type/key"
    bool getResourceEntryName(int id, std::string* out) const;        // "key"
    bool getResourceTypeName(int id, std::string* out) const;         // "type"
    bool getResourcePackageName(int id, std::string* out) const;      // "pkg"

    // --- value resolution ---
    // Resolve `id` to a TypedValue; if resolveRefs is true, follow
    // REFERENCE/ATTRIBUTE chains. Returns false if not found.
    bool getValue(int id, TypedValue* outValue, bool resolveRefs) const;
    // Resolve by name "[package:]type/key".
    bool getValue(const std::string& name, TypedValue* outValue, bool resolveRefs) const;

    // --- typed getters (return defaults / 0 / "" on wrong type or not found) ---
    std::string getString(int id) const;        // UTF-8 (most callers want UTF-8)
    std::u16string getText(int id) const;        // UTF-16 (AOSP getText/getString are CharSequence)
    int   getInteger(int id) const;
    bool  getBoolean(int id) const;
    float getFloat(int id) const;
    int   getColor(int id) const;               // a packed ARGB color value
    float getDimension(int id) const;           // px
    int   getDimensionPixelOffset(int id) const;
    int   getDimensionPixelSize(int id) const;
    float getFraction(int id, float base, float pbase) const;
    // Plural selection (requires ICU plural rules): falls back to getString(id).
    std::string getQuantityString(int id, int quantity) const;
    std::u16string getQuantityText(int id, int quantity) const;

    // --- raw / xml assets ---
    // Opens the raw file for `id` (its value is the file path). outValue (optional)
    // receives the resolved value. Caller owns the returned Asset.
    Asset* openRawResource(int id, TypedValue* outValue = nullptr) const;
    Asset* getXml(int id) const;        // binary AXML bytes (wrap in ResXMLTree)
    Asset* getLayout(int id) const { return getXml(id); }
    Asset* getAnimation(int id) const { return getXml(id); }

    // --- GUI-object factories (virtual so a cdroid-side subclass can override
    // them via source inheritance; STUBBED returning nullptr in resources.cc).
    // Filled at the cdroid.so merge by cdroid::Resources. ---
    virtual cdroid::Drawable*       getDrawable(int id, int density = 0) const;
    virtual cdroid::Drawable*       getDrawableForDensity(int id, int density) const;
    virtual cdroid::ColorStateList* getColorStateList(int id) const;
    virtual cdroid::Typeface*       getFont(int id) const;
    virtual cdroid::ComplexColor*   loadComplexColor(int id) const;
    virtual cdroid::Movie*          getMovie(int id) const;

private:
    // Helper: open the file whose path is the string value of `id` (raw/xml/layout).
    Asset* openByStringId(int id) const;
    // Helper: read the UTF-8 path stored as the string value of `id`.
    bool   pathOf(int id, std::string* out) const;

    AssetManager*       mAssets;
    ResTable_config     mConfig;
    DisplayMetrics      mMetrics;
};

} // namespace android
#endif // __CDROID_ANDROIDFW_RESOURCES_H__
