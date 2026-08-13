// Port of AOSP android.content.res.ResourcesImpl — the ID-based resource
// facade that sits on top of AssetManager.
//
// Named ResourcesImpl (AOSP's internal ResourcesImpl layer) to avoid clashing
// with the cdroid::Resources subclass (resources.h) which fills in the
// GUI-object factories. This is the value/meta/asset layer; GUI factories
// (getDrawable/getColorStateList/getFont/...) are virtual and stubbed (return
// nullptr) here — overridden by cdroid::Resources.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
#ifndef __CDROID_RESOURCESIMPL_H__
#define __CDROID_RESOURCESIMPL_H__

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <androidfw/restable.h>       // cdroid::ResTable, ResTable_config, Res_value
#include <core/assetmanager.h>   // cdroid::AssetManager
#include <core/asset.h>          // cdroid::Asset
#include <androidfw/typedvalue.h>     // cdroid::TypedValue, applyDimension
#include <core/displaymetrics.h>      // cdroid::DisplayMetrics

// Forward declarations of cdroid GUI object types — returned by pointer only,
// so the full GUI headers are NOT needed to compile this. Their factory methods
// are implemented in cdroid::Resources (resources.cc), which owns the caches.
namespace cdroid {
class Drawable;
class ColorStateList;
class Typeface;
class ComplexColor;
class Movie;
class Context;
}  // namespace cdroid

namespace cdroid {

// ResourcesImpl: the ID-based resource facade over an AssetManager. AOSP's
// internal ResourcesImpl layer; cdroid::Resources (resources.h) is the
// public Resources subclass that overrides the GUI factories.
class ResourcesImpl {
public:
    // `am` is NOT owned (must outlive this ResourcesImpl). If config/metrics are
    // null, defaults (zeroed config, density=1) are used.
    ResourcesImpl(AssetManager* am, const ResTable_config* config = nullptr,
                  const DisplayMetrics* metrics = nullptr);
    virtual ~ResourcesImpl();

    ResourcesImpl(const ResourcesImpl&) = delete;
    ResourcesImpl& operator=(const ResourcesImpl&) = delete;

    AssetManager* getAssets() const { return mAssets; }

    // AOSP Resources.Theme — the engine is ResTable::Theme (applyStyle/
    // getAttribute/resolveAttribute/clear/...). TODO: promote to a Resources.Theme
    // wrapper class at the Resources level (hides ResTable::Theme from public API).
    using Theme = ResTable::Theme;
    std::unique_ptr<Theme> newTheme();   // a Theme over this ResourcesImpl's table
    const ResTable_config& getConfiguration() const { return mConfig; }
    const DisplayMetrics&  getDisplayMetrics() const { return mMetrics; }
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
    bool getValue(int id, TypedValue* outValue, bool resolveRefs) const;
    bool getValue(const std::string& name, TypedValue* outValue, bool resolveRefs) const;

    // --- typed getters (return defaults / 0 / "" on wrong type or not found) ---
    std::string getString(int id) const;        // UTF-8
    std::u16string getText(int id) const;        // UTF-16
    std::u16string getText(int id, const std::u16string& def) const;  // AOSP getText(id, def)
    int   getInteger(int id) const;
    bool  getBoolean(int id) const;
    float getFloat(int id) const;
    int   getColor(int id) const;               // a packed ARGB color value
    float getDimension(int id) const;           // px
    int   getDimensionPixelOffset(int id) const;
    int   getDimensionPixelSize(int id) const;
    float getFraction(int id, float base, float pbase) const;
    std::string getQuantityString(int id, int quantity) const;
    std::u16string getQuantityText(int id, int quantity) const;

    // AOSP Resources.getStringArray/getIntArray/getTextArray — read a typed
    // array resource (<string-array>/<integer-array>) by id. Returns an empty
    // vector when the id is not an array.
    std::vector<std::string>   getStringArray(int id) const;   // UTF-8
    std::vector<std::u16string> getTextArray(int id) const;    // UTF-16 (CharSequence)
    std::vector<int>           getIntArray(int id) const;

    // --- raw / xml assets ---
    Asset* openRawResource(int id, TypedValue* outValue = nullptr) const;
    Asset* getXml(int id) const;        // binary AXML bytes (wrap in ResXMLTree)
    Asset* getLayout(int id) const { return getXml(id); }
    Asset* getAnimation(int id) const { return getXml(id); }

    // --- GUI-object factories. ResourcesImpl owns the AOSP mDrawableCache /
    // mComplexColorCache + loadDrawable/loadComplexColor (it lives in the cdroid
    // target, so cairo + the Context inflation bridge are available). getFont/
    // getMovie stay stubbed (out of scope). ---
    virtual Drawable*       getDrawable(int id, int density = 0) const;
    virtual Drawable*       getDrawableForDensity(int id, int density) const;
    virtual ColorStateList* getColorStateList(int id) const;
    virtual std::shared_ptr<ComplexColor> loadComplexColor(int id) const;
    virtual Typeface*       getFont(int id) const;
    virtual Movie*          getMovie(int id) const;

    // Inflation bridge: AOSP passes the Resources wrapper into loadDrawable so
    // ResourcesImpl can inflate (Drawable.createFromXml etc.); CDROID's
    // aggregation can't reach it, so the owning Resources hands its Context (the
    // DrawableInflater/ImageDecoder/ColorStateList engine) to ResourcesImpl once,
    // after construction. Null until set → GUI factories that need inflation
    // return nullptr.
    void setContext(Context* ctx) { mCtx = ctx; }

private:
    class DrawableCache;        // id → Drawable::ConstantState (defined in .cc)
    class ColorStateListCache;  // id → ColorStateList          (defined in .cc)

    Asset* openByStringId(int id) const;
    bool   pathOf(int id, std::string* out) const;

    AssetManager*       mAssets;
    ResTable_config     mConfig;
    DisplayMetrics      mMetrics;
    Context*            mCtx = nullptr;   // inflation bridge (see setContext)
    // AOSP mDrawableCache / mComplexColorCache — keyed by resource id. mutable:
    // populated from the const getDrawable/loadComplexColor. PImpl (defined in
    // .cc): they own GUI types (Drawable::ConstantState / ColorStateList).
    mutable std::unique_ptr<DrawableCache>       mDrawableCache;
    mutable std::unique_ptr<ColorStateListCache> mColorStateListCache;
};

} // namespace cdroid
#endif // __CDROID_ANDROIDFW_RESOURCES_H__
