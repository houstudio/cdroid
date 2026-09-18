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
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <core/displaymetrics.h>
#include <content/configuration.h>   // Configuration (live config face)      // cdroid::DisplayMetrics (value member mMetrics)

// ResourcesImpl is the facade that HIDES the androidfw native readers
// (AssetManager2, ResTable_config, Asset, TypedValue) — those headers live in
// the .cc only. So the many files that include this (via context.h) don't transitively
// pull androidfw.
namespace cdroid {
class AssetManager;        // core — opaque (pointer member + params)
class Asset;               // core — opaque (pointer return)
class ResTable_config;     // androidfw — opaque (held via unique_ptr)
class TypedValue;          // androidfw — opaque (pointer params)
class Drawable;            // GUI — opaque (pointer return)
class ColorStateList;      // GUI — opaque (pointer return)
class Typeface;
class ComplexColor;
class Movie;
class XmlPullParser;      // core — loadXmlResourceParser return
class Context;
template <class T> class ConstantState;   // animation/animator.h — opaque here
class Animator;            // animation — opaque (pointer return)
class StateListAnimator;   // animation — opaque (pointer return)
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

    // Theme lives at the Resources level now (Resources::Theme; cdroid::Resources
    // owns the engine view). ResourcesImpl exposes only config/metrics here.
    // AOSP ResourcesImpl.getConfiguration(): the LIVE Configuration object
    // (android.content.res.Configuration; updateFrom/calcConfigChanges operate
    // on it).
    const Configuration&   getConfiguration() const;
    const DisplayMetrics&  getDisplayMetrics() const { return mMetrics; }
    // AOSP ResourcesImpl.calcConfigChanges(@Nullable Configuration): the change
    // bits between the live configuration and `config` (null → all changed).
    int  calcConfigChanges(const Configuration* config);
    // AOSP ResourcesImpl.updateConfiguration(@Nullable Configuration, @Nullable
    // DisplayMetrics): applies the new configuration — the change bits drive
    // resource-variant reselection (arsc setParameters) and cache invalidation.
    void updateConfiguration(const Configuration* config, const DisplayMetrics* metrics);
    // Internal: raw arsc config seeding (device defaults at construction).
    const ResTable_config& getResTableConfig() const;      // out-of-line (mConfig opaque)
    void setConfiguration(const ResTable_config& config);  // out-of-line
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
    Asset* getXml(int id) const;        // binary AXML bytes (internal byte fetcher)
    // AOSP ResourcesImpl.loadXmlResourceParser: opens the xml resource and
    // returns its parser (the one text/binary sniffing point lives in
    // XmlPullParser::detectAndCreate). The int overload is the AOSP shape;
    // the string overload is the CDROID text-pak transitional face (string
    // refs cannot resolve through the arsc on text paks).
    std::unique_ptr<XmlPullParser> loadXmlResourceParser(int resid) const;

    // --- GUI-object factories. ResourcesImpl owns the AOSP mDrawableCache /
    // mComplexColorCache + loadDrawable/loadComplexColor (it lives in the cdroid
    // target, so cairo + the Context inflation bridge are available). getFont/
    // getMovie stay stubbed (out of scope).
    // themeEngine = AOSP Theme parameter (cdroid::Theme*, borrowed; getTheme()
    // ._engineHandle()): themes the ComplexColor inflation (AOSP
    // loadComplexColor passes it into createFromXml) and keys the caches, so
    // entries loaded under one theme never leak into another (AOSP
    // ThemedResourceCache semantics; nullptr = unthemed shared entries).
    // Drawable XML inflation resolves ?attr through the owning Context's theme
    // (AOSP inflates with null theme + applyTheme(); CDROID has no applyTheme
    // pass yet — themed drawables come from ContextThemeWrapper contexts). ---
    virtual Drawable*       getDrawable(int id, int density = 0, const void* themeEngine = nullptr) const;
    virtual Drawable*       getDrawableForDensity(int id, int density, const void* themeEngine = nullptr) const;
    virtual std::shared_ptr<ColorStateList> getColorStateList(int id, const void* themeEngine = nullptr) const;
    virtual std::shared_ptr<ComplexColor> loadComplexColor(int id, const void* themeEngine = nullptr) const;
    virtual Typeface*       getFont(int id) const;
    virtual Movie*          getMovie(int id) const;

    // Inflation bridge: AOSP passes the Resources wrapper into loadDrawable so
    // ResourcesImpl can inflate (Drawable.createFromXml etc.); CDROID's
    // aggregation can't reach it, so the owning Resources hands its Context (the
    // DrawableInflater/ImageDecoder/ColorStateList engine) to ResourcesImpl once,
    // after construction. Null until set → GUI factories that need inflation
    // return nullptr.
    void setContext(Context* ctx) { mCtx = ctx; }
    // The Context that bridged this impl (inflation bridge, see setContext);
    // AOSP Resources has none — CDROID seam used by resource-level loaders
    // that need a Context (e.g. animated-image reopen by path).
    Context* getContext() const { return mCtx; }

    // AOSP ResourcesImpl.getAnimatorCache()/getStateListAnimatorCache()
    // (ConfigurationBoundResourceCache) — used by AnimatorInflater's int-id
    // loadAnimator/loadStateListAnimator. obtain* applies newInstance() on a
    // hit (callers never receive the cached source animator); themeEngine is
    // the opaque cdroid::Theme* from Resources::Theme::_engineHandle().
    Animator* obtainCachedAnimator(int id, const void* themeEngine) const;
    void cacheAnimator(int id, const void* themeEngine,
                       const std::shared_ptr<ConstantState<Animator*>>& cs) const;
    StateListAnimator* obtainCachedStateListAnimator(int id, const void* themeEngine) const;
    void cacheStateListAnimator(int id, const void* themeEngine,
                                const std::shared_ptr<ConstantState<StateListAnimator*>>& cs) const;

private:
    class DrawableCache;        // id → Drawable::ConstantState (defined in .cc)
    class ColorStateListCache;  // id → ColorStateList          (defined in .cc)
    class AnimatorCache;        // id → ConstantState<Animator*>  (AOSP mAnimatorCache)
    class StateListAnimatorCache;  // id → ConstantState<StateListAnimator*>

    Asset* openByStringId(int id) const;
    // Open an arsc-recorded path against the pak layout (res/ prefix strip +
    // aapt2 2.19 "-vN" config-dir suffix strip). See resourcesimpl.cc.
    Asset* openPakPath(const std::string& path) const;
    bool   pathOf(int id, std::string* out) const;

    AssetManager*       mAssets;
    Configuration       mConfiguration;   // AOSP mConfiguration (live config)
    std::unique_ptr<ResTable_config> mConfig;  // opaque (resourcetypes.h hidden in .cc)
    DisplayMetrics      mMetrics;
    Context*            mCtx = nullptr;   // inflation bridge (see setContext)
    // AOSP mDrawableCache / mComplexColorCache — keyed by resource id. mutable:
    // populated from the const getDrawable/loadComplexColor. PImpl (defined in
    // .cc): they own GUI types (Drawable::ConstantState / ColorStateList).
    mutable std::unique_ptr<DrawableCache>       mDrawableCache;
    mutable std::unique_ptr<ColorStateListCache> mColorStateListCache;
    // AOSP ResourcesImpl.mAnimatorCache / mStateListAnimatorCache feeding
    // AnimatorInflater.loadAnimator/loadStateListAnimator (ConfigurationBound-
    // ResourceCache semantics). Shared (not weak like DrawableCache): the
    // animator ConstantState is the sole owner of the cached source animator.
    mutable std::unique_ptr<AnimatorCache>           mAnimatorCache;
    mutable std::unique_ptr<StateListAnimatorCache>  mStateListAnimatorCache;
};

} // namespace cdroid
#endif // __CDROID_ANDROIDFW_RESOURCES_H__
