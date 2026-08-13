// cdroid::Resources — the libcdroid Resources. AOSP-aligned: Resources AGGREGATES
// a ResourcesImpl (HAS-A) and forwards the value/meta/asset surface to it.
//
// resources.h uses forward declarations only — the heavy androidfw headers
// (resourcesimpl.h -> restable.h -> assetmanager.h -> asset.h) are included in
// resources.cc, NOT here. This keeps resources.h lightweight for the many files
// that include it (via context.h).
//
// Declaration-only; implementations (forwarding + GUI factories + obtainers) in
// resources.cc.
#ifndef __RESOURCES_CDROID_H__
#define __RESOURCES_CDROID_H__

#include <string>
#include <memory>
#include <cstdint>
#include <vector>

namespace cdroid {

// Forward declarations — their full definitions come from includes in resources.cc.
class ResourcesImpl;
class AssetManager;
class ResTable_config;
class DisplayMetrics;
class TypedValue;
class Asset;
class Context;
class Drawable;
class ColorStateList;
class Typeface;
class ComplexColor;
class Movie;
class AttributeSet;
class TypedArray;

class Resources {
public:
    Resources(AssetManager* am, cdroid::Context* ctx);
    ~Resources();

    // AOSP Resources.getSystem() — the global system Resources singleton.
    // CDROID's App is the system singleton; its Resources include the framework
    // arsc (cdroid.pak).
    static Resources& getSystem();

    // --- forwarded to the aggregated ResourcesImpl (AOSP Resources delegates) ---
    AssetManager*       getAssets() const;
    const ResTable_config& getConfiguration() const;
    const DisplayMetrics&  getDisplayMetrics() const;
    void setConfiguration(const ResTable_config& config);
    void setDisplayMetrics(const DisplayMetrics& metrics);

    int  getIdentifier(const std::string& name, const std::string& type,
                       const std::string& pkg) const;
    bool getResourceName(int id, std::string* out) const;
    bool getResourceEntryName(int id, std::string* out) const;
    bool getResourceTypeName(int id, std::string* out) const;
    bool getResourcePackageName(int id, std::string* out) const;

    bool getValue(int id, TypedValue* outValue, bool resolveRefs) const;
    bool getValue(const std::string& name, TypedValue* outValue, bool resolveRefs) const;

    std::string    getString(int id) const;
    std::string    getString(int id, const std::vector<std::string>& formatArgs) const;
    std::u16string getText(int id) const;
    std::u16string getText(int id, const std::u16string& def) const;
    int   getInteger(int id) const;
    bool  getBoolean(int id) const;
    float getFloat(int id) const;
    int   getColor(int id) const;
    float getDimension(int id) const;
    int   getDimensionPixelOffset(int id) const;
    int   getDimensionPixelSize(int id) const;
    float getFraction(int id, float base, float pbase) const;
    std::string    getQuantityString(int id, int quantity) const;
    std::string    getQuantityString(int id, int quantity, const std::vector<std::string>& formatArgs) const;
    std::u16string getQuantityText(int id, int quantity) const;

    std::vector<std::string>    getStringArray(int id) const;
    std::vector<std::u16string> getTextArray(int id) const;
    std::vector<int>            getIntArray(int id) const;

    Asset* openRawResource(int id, TypedValue* outValue = nullptr) const;
    // AOSP Resources.openRawResourceFd — returns AssetFileDescriptor. CDROID has
    // no AssetFileDescriptor (fd-based assets); stub returns nullptr.
    Asset* openRawResourceFd(int id) const;
    Asset* getXml(int id) const;
    Asset* getLayout(int id) const;
    Asset* getAnimation(int id) const;

    // --- GUI-object factories (Resources' own; bridge to string-based inflation) ---
    cdroid::Drawable*       getDrawable(int id) const;
    cdroid::Drawable*       getDrawableForDensity(int id, int density) const;
    cdroid::ColorStateList* getColorStateList(int id) const;
    Typeface*               getFont(int id) const;
    std::shared_ptr<ComplexColor> loadComplexColor(int id) const;
    Movie*                  getMovie(int id) const;

    // --- AOSP Resources.obtainStyledAttributes(...) ---
    // AttributeSet is nullable (AOSP @Nullable).
    std::unique_ptr<TypedArray> obtainStyledAttributes(const AttributeSet* set,
        const uint32_t* attrs, int defStyleAttr = 0, int defStyleRes = 0) const;
    // Convenience overload: AttributeSet& → AttributeSet*.
    std::unique_ptr<TypedArray> obtainStyledAttributes(const AttributeSet& set,
        const uint32_t* attrs, int defStyleAttr = 0, int defStyleRes = 0) const;
    std::unique_ptr<TypedArray> obtainStyledAttributes(const uint32_t* attrs) const;
    std::unique_ptr<TypedArray> obtainStyledAttributes(int resid, const uint32_t* attrs) const;

    // AOSP Resources.obtainTypedArray(@ArrayRes int id).
    std::unique_ptr<TypedArray> obtainTypedArray(int id) const;

    class Theme;   // AOSP Resources.Theme — defined below (view over ResTable::Theme)

private:
    std::unique_ptr<ResourcesImpl> mImpl;   // aggregated (AOSP Resources -> ResourcesImpl); owns the drawable/ComplexColor caches
    cdroid::Context* mCtx;
};

// AOSP Resources.Theme — a framework-level theme handle. A lightweight,
// non-owning VIEW over the underlying engine (cdroid::ResTable::Theme, owned by
// Assets); getTheme() returns it by value. Methods are out-of-line (resources.cc)
// so this header need not include restable.h. _engineHandle() exposes the engine
// as void* for the resource layer's obtainStyledAttributes (resources.cc /
// context.cc) — the only places that need the raw ResTable::Theme*.
class Resources::Theme {
public:
    Resources& getResources() const { return mRes; }
    void applyStyle(int resId, bool force = false);
    bool resolveAttribute(int resId, TypedValue* outValue, bool resolveRefs) const;
    void* _engineHandle() const { return mEngine; }   // cdroid::ResTable::Theme* (borrowed)
private:
    friend class Assets;          // Assets/App construct it from their engine
    Theme(Resources& res, void* engine) : mRes(res), mEngine(engine) {}
    Resources& mRes;
    void*      mEngine;
};

} // namespace cdroid
#endif // __RESOURCES_CDROID_H__
