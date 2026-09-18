// cdroid::Resources — the libcdroid Resources. AOSP-aligned: Resources AGGREGATES
// a ResourcesImpl (HAS-A) and forwards the value/meta/asset surface to it.
//
// resources.h uses forward declarations only — the heavy androidfw headers
// (resourcesimpl.h -> assetmanager.h -> androidfw) are included in
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
#include <content/configuration.h>   // Configuration (getConfiguration face)

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
class XmlPullParser;    // core — getXml(int) return (AOSP XmlResourceParser role)
// android.content.res.XmlResourceParser (AOSP: the same package as Resources)
// — the merged XmlPullParser + AttributeSet interface getXml/getLayout
// return. CDROID's XmlPullParser base already carries AttributeSet through
// single inheritance (the XmlPullAttributes role is built in), so the
// resource-parser role type is an alias; AOSP's AutoCloseable close() is the
// owning unique_ptr's destructor (RAII for the manual lifecycle).
using XmlResourceParser = XmlPullParser;
class ColorStateList;
class Typeface;
class ComplexColor;
class Movie;
class AttributeSet;
class TypedArray;
template <class T> class ConstantState;   // animation/animator.h — opaque here
class Animator;            // animation — opaque (pointer return)
class StateListAnimator;   // animation — opaque (pointer return)

class Resources {
public:
    class Theme;   // defined below (Resources::Theme over this impl)
    Resources(AssetManager* am, cdroid::Context* ctx);
    ~Resources();

    // AOSP Resources.getSystem() — the global system Resources singleton.
    // CDROID's App is the system singleton; its Resources include the framework
    // arsc (cdroid.pak).
    static Resources& getSystem();

    // --- forwarded to the aggregated ResourcesImpl (AOSP Resources delegates) ---
    AssetManager*       getAssets() const;
    // AOSP Resources.getConfiguration(): the live Configuration.
    const Configuration& getConfiguration() const;
    // AOSP Resources.updateConfiguration(@Nullable Configuration, @Nullable DisplayMetrics).
    void updateConfiguration(const Configuration* config, const DisplayMetrics* metrics);
    // AOSP Resources.calcConfigChanges(@Nullable Configuration): the change bits.
    int  calcConfigChanges(const Configuration* config) const;
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
    // AOSP Resources.getColor(int id, @Nullable Theme theme) — null theme =
    // unthemed load (same as the plain overload).
    int   getColor(int id, const Theme* theme) const;
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
    // CDROID seam: the Context that owns this Resources (inflation bridge);
    // AOSP Resources has none. Used by resource-level loaders that need a
    // Context (e.g. ImageDecoder's animated-image reopen by path).
    Context* getContext() const;
    // AOSP Resources.openRawResourceFd — returns AssetFileDescriptor. CDROID has
    // no AssetFileDescriptor (fd-based assets); stub returns nullptr.
    Asset* openRawResourceFd(int id) const;
    // AOSP Resources.getXml(@XmlRes int) -> XmlResourceParser.
    std::unique_ptr<XmlResourceParser> getXml(int id) const;
    // AOSP Resources.getLayout(@LayoutRes int): the same loadXmlPullParser
    // under a layout-flavored name (types the resource as a layout; no extra
    // behavior).
    std::unique_ptr<XmlResourceParser> getLayout(int id) const;

    // --- GUI-object factories (Resources' own; bridge to string-based inflation) ---
    // AOSP face: @Nullable Theme — null theme = unthemed load (shared cache
    // entries). The theme reaches ResourcesImpl as the raw engine handle
    // (getTheme()._engineHandle()) for ComplexColor inflation and themed cache
    // isolation (AOSP ThemedResourceCache semantics).
    cdroid::Drawable*       getDrawable(int id, const Theme* theme = nullptr) const;
    cdroid::Drawable*       getDrawableForDensity(int id, int density, const Theme* theme = nullptr) const;
    std::shared_ptr<ColorStateList> getColorStateList(int id, const Theme* theme = nullptr) const;
    Typeface*               getFont(int id) const;
    std::shared_ptr<ComplexColor> loadComplexColor(int id, const Theme* theme = nullptr) const;
    Movie*                  getMovie(int id) const;

    // --- AOSP Resources.obtainStyledAttributes(...) ---
    // AOSP Resources.obtainAttributes(set, attrs): theme-less — only the
    // attributes explicitly set in the XML (no style/theme resolution).
    std::unique_ptr<TypedArray> obtainAttributes(const AttributeSet* set, const uint32_t* attrs) const;
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

    // --- Animator caches (AOSP ResourcesImpl.getAnimatorCache() /
    // getStateListAnimatorCache(), ConfigurationBoundResourceCache) ---
    // Used by AnimatorInflater.loadAnimator/loadStateListAnimator(int).
    // obtain* returns newInstance() on a hit — never the cached source.
    Animator* obtainCachedAnimator(int id, const void* themeEngine) const;
    void cacheAnimator(int id, const void* themeEngine,
                       const std::shared_ptr<ConstantState<Animator*>>& cs) const;
    StateListAnimator* obtainCachedStateListAnimator(int id, const void* themeEngine) const;
    void cacheStateListAnimator(int id, const void* themeEngine,
                                const std::shared_ptr<ConstantState<StateListAnimator*>>& cs) const;

    class Theme;   // AOSP Resources.Theme — defined below (view over cdroid::Theme)

    // AOSP Resources.newTheme(): a NEW empty theme over this Resources' table
    // (framework-internal consumers build their own theme without a Context).
    // The returned Theme OWNS its engine; themes obtained via getTheme() borrow
    // the Context's engine.
    Theme newTheme();

private:
    std::unique_ptr<ResourcesImpl> mImpl;   // aggregated (AOSP Resources -> ResourcesImpl); owns the drawable/ComplexColor caches
    cdroid::Context* mCtx;
};

// AOSP Resources.Theme — a framework-level theme handle. A lightweight,
// non-owning VIEW over the underlying engine (cdroid::Theme, the AM2 Theme,
// owned by App or the creating wrapper); getTheme() returns it by value.
// Methods are out-of-line (resources.cc) so this header need not include
// androidfw. _engineHandle() exposes the engine as void* for the resource
// layer's obtainStyledAttributes (resources.cc / context.cc) — the only
// places that need the raw cdroid::Theme*.
class Resources::Theme {
public:
    Resources& getResources() const { return mRes; }   // CDROID extension (AOSP has getAssets)
    AssetManager* getAssets() const;
    // AOSP Resources.Theme.getDrawable/getColor(@Res int): resource loads
    // resolved through this theme's Resources.
    Drawable* getDrawable(int id) const;
    int getColor(int id) const;
    // AOSP Resources.Theme face: applyStyle/setTo/resolveAttribute/obtainStyledAttributes.
    void applyStyle(int resId, bool force = false);
    void setTo(const Theme& other);
    // AOSP Theme.resolveAttributes(@Nullable int[] themeAttrs, int[] attrs):
    // re-resolve the ?attr ids recorded by TypedArray.extractThemeAttrs()
    // through THIS theme — the applyTheme() re-resolution engine.
    std::unique_ptr<TypedArray> resolveAttributes(const std::vector<int>& themeAttrs,
                                                  const uint32_t* attrs) const;
    bool resolveAttribute(int resId, TypedValue* outValue, bool resolveRefs) const;
    std::unique_ptr<TypedArray> obtainStyledAttributes(const uint32_t* attrs) const;
    std::unique_ptr<TypedArray> obtainStyledAttributes(int resid, const uint32_t* attrs) const;
    std::unique_ptr<TypedArray> obtainStyledAttributes(const AttributeSet* set,
            const uint32_t* attrs) const;
    std::unique_ptr<TypedArray> obtainStyledAttributes(const AttributeSet* set,
            const uint32_t* attrs, int defStyleAttr, int defStyleRes) const;
    // --- AOSP @hide face (used by framework-internal theme consumers) ---------
    // Attr resIDs this theme currently has values for.
    std::vector<uint32_t> getAllAttributes() const;
    // Bit mask of CONFIG_* changes that would impact this theme.
    int getChangingConfigurations() const;
    // Reset to the last setTo() state (or initial state), erasing applyStyle()
    // changes made since.
    void rebase();
    // Log the theme's attribute values (AOSP dump(priority, tag, prefix)).
    void dump(const char* tag, const char* prefix = "") const;
    void* _engineHandle() const { return mEngine; }   // cdroid::Theme* (AM2; borrowed or owned)
private:
    friend class Resources;       // newTheme() (owned engine)
    friend class App;           // App constructs it from its engine
    friend class ResourcesImpl;   // wraps a caller-passed engine (themed drawable loads)
    Theme(Resources& res, void* engine) : mRes(res), mEngine(engine) {}
    Resources& mRes;
    void*      mEngine;
    std::shared_ptr<void> mOwned;  // engine ownership when created by newTheme()
};

} // namespace cdroid
#endif // __RESOURCES_CDROID_H__
