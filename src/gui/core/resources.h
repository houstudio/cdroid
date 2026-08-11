// cdroid::Resources — the libcdroid Resources. AOSP-aligned: Resources AGGREGATES
// a ResourcesImpl (HAS-A, not IS-A) and forwards the value/meta/asset surface to
// it, keeping its own GUI-object factories (getDrawable/getColorStateList, which
// bridge to cdroid's string-based inflation) and obtainStyledAttributes (which
// need the Context).
//
// Declaration + inline forwarding here; the GUI factories + obtainStyledAttributes
// are in resources.cc.
#ifndef __RESOURCES_CDROID_H__
#define __RESOURCES_CDROID_H__

#include <string>
#include <memory>
#include <cstdint>
#include <vector>
#include "androidfw/resourcesimpl.h"   // ResourcesImpl (aggregated)

namespace cdroid {

class Context;
class Drawable;
class ColorStateList;
class AttributeSet;
class TypedArray;

class Resources {
public:
    using Theme = ResourcesImpl::Theme;

    // `am` is NOT owned (must outlive this Resources); `ctx` is the bridge to the
    // existing string-based Drawable/ColorStateList inflation.
    Resources(AssetManager* am, cdroid::Context* ctx);

    // --- forwarded to the aggregated ResourcesImpl (AOSP Resources delegates) ---
    AssetManager*       getAssets() const { return mImpl.getAssets(); }
    std::unique_ptr<Theme> newTheme() { return mImpl.newTheme(); }
    const ResTable_config& getConfiguration() const { return mImpl.getConfiguration(); }
    const DisplayMetrics&  getDisplayMetrics() const { return mImpl.getDisplayMetrics(); }
    void setConfiguration(const ResTable_config& c) { mImpl.setConfiguration(c); }
    void setDisplayMetrics(const DisplayMetrics& m) { mImpl.setDisplayMetrics(m); }

    int  getIdentifier(const std::string& name, const std::string& type,
                       const std::string& pkg) const { return mImpl.getIdentifier(name, type, pkg); }
    bool getResourceName(int id, std::string* out) const { return mImpl.getResourceName(id, out); }
    bool getResourceEntryName(int id, std::string* out) const { return mImpl.getResourceEntryName(id, out); }
    bool getResourceTypeName(int id, std::string* out) const { return mImpl.getResourceTypeName(id, out); }
    bool getResourcePackageName(int id, std::string* out) const { return mImpl.getResourcePackageName(id, out); }

    bool getValue(int id, TypedValue* outValue, bool resolveRefs) const { return mImpl.getValue(id, outValue, resolveRefs); }
    bool getValue(const std::string& name, TypedValue* outValue, bool resolveRefs) const { return mImpl.getValue(name, outValue, resolveRefs); }

    std::string    getString(int id) const { return mImpl.getString(id); }
    std::u16string getText(int id) const { return mImpl.getText(id); }
    std::u16string getText(int id, const std::u16string& def) const { return mImpl.getText(id, def); }
    int   getInteger(int id) const { return mImpl.getInteger(id); }
    bool  getBoolean(int id) const { return mImpl.getBoolean(id); }
    float getFloat(int id) const { return mImpl.getFloat(id); }
    int   getColor(int id) const { return mImpl.getColor(id); }
    float getDimension(int id) const { return mImpl.getDimension(id); }
    int   getDimensionPixelOffset(int id) const { return mImpl.getDimensionPixelOffset(id); }
    int   getDimensionPixelSize(int id) const { return mImpl.getDimensionPixelSize(id); }
    float getFraction(int id, float base, float pbase) const { return mImpl.getFraction(id, base, pbase); }
    std::string    getQuantityString(int id, int quantity) const { return mImpl.getQuantityString(id, quantity); }
    std::u16string getQuantityText(int id, int quantity) const { return mImpl.getQuantityText(id, quantity); }

    std::vector<std::string>    getStringArray(int id) const { return mImpl.getStringArray(id); }
    std::vector<std::u16string> getTextArray(int id) const { return mImpl.getTextArray(id); }
    std::vector<int>            getIntArray(int id) const { return mImpl.getIntArray(id); }

    Asset* openRawResource(int id, TypedValue* outValue = nullptr) const { return mImpl.openRawResource(id, outValue); }
    Asset* getXml(int id) const { return mImpl.getXml(id); }
    Asset* getLayout(int id) const { return mImpl.getLayout(id); }
    Asset* getAnimation(int id) const { return mImpl.getAnimation(id); }

    // --- GUI-object factories (Resources' own; bridge to string-based inflation) ---
    cdroid::Drawable*       getDrawable(int id, int density = 0) const;
    cdroid::Drawable*       getDrawableForDensity(int id, int density) const { return mImpl.getDrawableForDensity(id, density); }
    cdroid::ColorStateList* getColorStateList(int id) const;
    Typeface*               getFont(int id) const { return mImpl.getFont(id); }
    ComplexColor*           loadComplexColor(int id) const { return mImpl.loadComplexColor(id); }
    Movie*                  getMovie(int id) const { return mImpl.getMovie(id); }

    // --- AOSP Resources.obtainStyledAttributes(...) ---
    // AttributeSet is nullable (AOSP @Nullable).
    std::unique_ptr<TypedArray> obtainStyledAttributes(const AttributeSet* set,
        const uint32_t* attrs, int defStyleAttr = 0, int defStyleRes = 0) const;
    std::unique_ptr<TypedArray> obtainStyledAttributes(const uint32_t* attrs) const;
    std::unique_ptr<TypedArray> obtainStyledAttributes(int resid, const uint32_t* attrs) const;

private:
    ResourcesImpl   mImpl;   // aggregated (AOSP Resources -> ResourcesImpl)
    cdroid::Context* mCtx;
};

} // namespace cdroid
#endif // __RESOURCES_CDROID_H__
