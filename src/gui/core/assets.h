/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __ASSETS_H__
#define __ASSETS_H__
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <core/variant.h>
#include <drawable/drawable.h>
#include "androidfw/restable.h"   // ResTable: arsc resource resolution
#include "androidfw/resourcesimpl.h"  // Resources (+ Resources::Theme)
#include "core/typedarray.h"      // TypedArray: consumer-side typed attr view

namespace cdroid{
class Resources;  // cdroid::Resources (resources.h) — lazy ID-based facade
// AssetManager is forward-declared at global scope in context.h.

class Assets:public Context{
private:
    // Lazy ID-based resource layer (AOSP Resources/AssetManager), built
    // on first getResources()/getAssets() from the pak paths recorded in
    // addResource(). Coexists with the legacy string-based members below.
    std::vector<std::string>        mPakPaths;
    mutable AssetManager*  mAssetManager = nullptr;
    mutable cdroid::Resources*      mCdroidResources = nullptr;
    void ensureCdroidResources() const;

    int mNextAutofillViewId;
    std::string mLanguage;
    std::string mThemeName;
    AttributeSet mTheme;
    std::unordered_map<std::string,std::string>mStrings;
    std::unordered_map<std::string,int>mIDS;
    std::unordered_map<std::string,std::vector<std::string>>mArraies;
    std::unordered_map<std::string,std::weak_ptr<Drawable::ConstantState>>mDrawables;
    std::unordered_map<std::string,class ZIPArchive*>mResources;
    std::unordered_map<std::string,AttributeSet>mStyles;
    std::unordered_map<std::string,uint32_t>mColors;
    std::unordered_map<std::string,nonstd::variant<int,float>>mDimensions;
    std::unordered_map<std::string,std::shared_ptr<ColorStateList>>mStateColors;
    ResTable* mResTable;   // loaded from resources.arsc in pak (null if no arsc)
    ResTable::Theme* mArscTheme = nullptr;  // theme built from arsc (null if none)
    // arsc identifier lookup: tries the given package first, then "android"
    // (framework arsc compiled with package="android" via aapt2 -x, but pak
    // registered under "cdroid" — the names don't match, so we fall back).
    uint32_t arscGetIdentifier(const std::string& name, const std::string& type, const std::string& pkg) const;
    bool arscResolveHexRef(const std::string& s, Res_value* out) const;
    // If resid is a "?type/key" theme-attribute reference, resolve it through
    // the arsc Theme to a concrete value string ("#color", "@drawable/...", a
    // dimension); otherwise return resid unchanged.
    std::string resolveThemeRef(const std::string& resid) const;
    const std::string parseResource(const std::string&fullresid,std::string*res,std::string*ns)const;
    void parseItem(const std::string&package,const std::string&resid,const std::vector<std::string>&tag,std::vector<AttributeSet>atts,const std::string&value,void*);
    ZIPArchive*getResource(const std::string & fullresid, std::string* relativeResid,std::string*package)const;
    std::string resolveAttrValue(const std::string&name)const;
protected:
    std::string mName;
    DisplayMetrics mDisplayMetrics;
    void loadStrings(const std::string&lan);
    void applyLocale(const std::string&lan);
    int addResource(const std::string&path,const std::string&name=std::string());
    int loadKeyValues(const std::string&package,const std::string&resid,void*p);
public:
    Assets();
    Assets(const std::string&path);
    ~Assets()override;
    // Binary-AXML bridge (transitional): resolve a resource ID / fetch a string
    // from the loaded arsc so xmlpullparser can render typed attribute values.
    bool arscResolveId(uint32_t resId, Res_value* out) const;
    const char16_t* arscStringAt(uint32_t resId, size_t* outLen) const;
    // Render a resource ID as an "@type/key" reference string (e.g.
    // "@drawable/bg", "@string/hello") matching text-XML form, so CDROID's
    // existing string-based resolvers consume binary-AXML references unchanged.
    std::string getResourceName(uint32_t resId) const override;
    // Resolve a theme-attribute reference (?attr/<id>) through the arsc Theme:
    // getAttribute + resolveAttributeReference, so ?android:colorPrimary etc.
    // flatten to a concrete value. Returns true if the theme had the attr.
    // When outBlock != null, *outBlock receives the owning string-pool block of
    // the resolved value (needed to resolve TYPE_STRING values via stringAtBlock).
    bool arscThemeAttribute(uint32_t attrId, Res_value* out, ssize_t* outBlock = nullptr) const;
    // Resolve a theme attribute NAME to its value string. Uses the text mTheme
    // first; in SDK/binary mode mTheme is empty (values only in resources.arsc),
    // so it falls back to the arsc Theme. pkg is a package hint (arscGetIdentifier
    // also tries "android" and any package).
    std::string themeString(const std::string& key, const std::string& pkg) const;
    int loadStyles(const std::string&resid);
    void clearStyles();
    const std::string getPackageName()const override;
    ResTable::Theme& getTheme() override;
    const std::string getThemeName() const override;
    void setTheme(const std::string&theme)override;
    void setTheme(int resid) override;
    const DisplayMetrics&getDisplayMetrics()const override;
    int getId(const std::string&)const override;
    int getNextAutofillId()override;
    const std::string getString(const std::string&id,const std::string&lan="")override;
    Cairo::RefPtr<Cairo::ImageSurface> loadImage(std::istream&,int width,int height)override;
    Cairo::RefPtr<Cairo::ImageSurface> loadImage(const std::string&resname,int width,int height)override;
    std::vector<std::string> getStringArray(const std::string&resname,const std::string&arrayname)const;
    std::unique_ptr<std::istream> getInputStream(const std::string&resname,std::string*outpkg=nullptr)override;
    Drawable * getDrawable(const std::string&resid)override;
    // AOSP ID-based overrides (cdroid::Context resource face).
    Resources&      getResources() override;
    AssetManager&   getAssets() override;
    Drawable*                getDrawable(int id) override;
    ColorStateList*          getColorStateList(int id) override;
    bool getBoolean(const std::string&resid)const override;
    int getColor(const std::string&resid)override;
    int getDimension(const std::string&resid)const override;
    int getDimensionPixelSize(const std::string&key,int def=0)const override;
    float getFloat(const std::string&resid,float def=0)const override;
    size_t getArray(const std::string&resid,std::vector<int>&)override;
    size_t getArray(const std::string&resid,std::vector<std::string>&)override;
    RefPtr<ColorStateList> getColorStateList(const std::string&resid)override;
    // Bring the ID-based obtainStyledAttributes(const uint32_t*) overloads from
    // Context into Assets scope; otherwise the string overload above hides them
    // (C++ name hiding).
    using Context::obtainStyledAttributes;
    AttributeSet obtainStyledAttributes(const std::string&)override;
    // Phase 2 TypedArray bridge: extract typed attr values from binary AXML.
    // AOSP Context.obtainStyledAttributes(AttributeSet, int[], defStyleAttr, defStyleRes).
    // `attrs` is nullable (AOSP new View(ctx, null, defStyleAttr)); `styleable` is a
    // sentinel-terminated attr-id array (styleable::X::IDS). Overrides Context's pure
    // virtual with arsc resolution (element > style= > defStyleAttr > defStyleRes).
    std::unique_ptr<TypedArray> obtainStyledAttributes(
        const AttributeSet* attrs, const uint32_t* styleable,
        int32_t defStyleAttr = 0, int32_t defStyleRes = 0) override;
};

}//namespace
#endif
