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
#ifndef __ATTRIBUTESET_H__
#define __ATTRIBUTESET_H__
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <core/displaymetrics.h>

namespace cdroid{
class Drawable;
class ColorStateList;
class Context;
class XmlPullParser;
template <typename T>
using RefPtr = std::shared_ptr<T>;
class AttributeSet{
protected:
    std::string mPackage;
    Context*mContext;
    std::shared_ptr<std::unordered_map<std::string,std::string>>mAttrs;
    // Optional attribute-NAME -> resource-id map (each attr's OWN id, for the
    // AOSP getAttributeNameResource). Populated when this AttributeSet is built
    // from a typed source (e.g. an arsc style bag); empty for plain string-built
    // sets (getAttributeNameResource then returns 0). Index methods iterate mAttrs
    // (small N; resolution matches by id/name, not position).
    std::shared_ptr<std::unordered_map<std::string,int>>mAttrResIds;
    // When this AttributeSet is a *style* resolved from the arsc (built by
    // obtainStyledAttributes(styleName)), the source style's resource id — so
    // obtainStyledAttributes can re-resolve it through the arsc theme
    // resolver (raw Res_values) instead of string-parsing. 0 = not a style set.
    int mStyleResId = 0;
public:
    AttributeSet();
    AttributeSet(const AttributeSet&);
    // Shared empty AttributeSet — non-null stand-in for a widget ctor's null
    // AttributeSet* (programmatic styled construction), so attribute reads return
    // defaults instead of dereferencing null.
    static const AttributeSet& empty();
    AttributeSet(Context*ctx,const std::string&package);
    virtual ~AttributeSet()=default;
    Context*getContext()const;
    void setContext(Context*,const std::string&package);
    bool add(const std::string&,const std::string&value);
    // CDROID bridge: record an attribute's resource id (its OWN id, for the AOSP
    // getAttributeNameResource). Used when this AttributeSet is built from a typed
    // source (arsc style bag) so the id-interface works for style-derived sets.
    void setAttributeResourceId(const std::string& name, int resId);
    // The source style resId if this AttributeSet is a resolved style (else 0).
    int getStyleResourceId() const { return mStyleResId; }
    void setStyleResourceId(int resId) { mStyleResId = resId; }
    virtual bool hasAttribute(const std::string&key)const;
    virtual size_t getAttributeCount()const;
    // Single-pass KV iteration over the present attributes (map order). Templated and header-only so
    // the callback inlines — O(n) with no std::function overhead (index-probing an unordered_map
    // would be O(n) per call → O(n²) for a full sweep).
    template<typename F>
    void forEachAttribute(F&& fn) const {
        for (const auto& kv : *mAttrs) {
            fn(kv.first, kv.second);
        }
    }
    int set(const char*atts[],int size=0);
    static std::string normalize(const std::string&pkg,const std::string&property);
    int inherit(const AttributeSet&other);
    int Override(const AttributeSet&other);
    // String-key value lookup. Virtual so a binary XmlPullParser can resolve by
    // name straight from its ResXMLTree (no mAttrs bridge). The const char*
    // overload below delegates here.
    virtual const std::string getAttributeValue(const std::string&key)const;
    const std::string getAttributeValue(const char*key)const;   // const char* overload (binds before the AOSP int-index overload)
    bool getBoolean(const std::string&key,bool def=false)const;
    int getInt(const std::string&key,int def=0)const;
    int getInt(const std::string&key,const std::unordered_map<std::string,int>&keyvaluemaps,int def=0)const;
    int getResourceId(const std::string&key,int def=0)const;
    int getColor(const std::string&key,int def=0xFFFFFFFF)const;
    int getColorWithException(const std::string&key)const;
    float getFloat(const std::string&key,float def=.0)const;
    const std::string getString(const std::string&key,const std::string&def=std::string())const;
    int getGravity(const std::string&key,int defvalue=0)const;
    int getTintMode(const std::string&key,int def)const;

    int getDimension(const std::string&key,int def=0)const;
    int getDimensionPixelSize(const std::string&key,int def=0)const;
    int getDimensionPixelOffset(const std::string&key,int def=0)const;
    int getLayoutDimension(const std::string&key,int def)const;
    float getFraction(const std::string&key,int base,int pbase,float def=.0)const;

    RefPtr<ColorStateList>getColorStateList(const std::string&key)const;
    Drawable*getDrawable(const std::string&key)const;
    int getArray(const std::string&key,std::vector<std::string>&array)const;
    int getArray(const std::string&key,std::vector<int>&array)const;

    // --- AOSP android.util.AttributeSet interface (index/id-based) -------------
    // Ported verbatim from frameworks/base/core/java/android/util/AttributeSet.java.
    // Coexists with the string-key getters above (CDROID additions); these are the
    // index/namespace-based, typed, AOSP-faithful methods. Virtual so XmlPullParser
    // (binary AXML) overrides them via ResXMLTree; the base impl works off mAttrs.
    // String return values use std::string (empty == AOSP null).
    virtual std::string getAttributeNamespace(int index) const;          // default ""
    virtual std::string getAttributeName(int index) const;               // "" if not found
    virtual std::string getAttributeValue(int index) const;              // "" if not found
    virtual std::string getAttributeValue(const std::string& namespace_,
                                          const std::string& name) const;
    virtual std::string getPositionDescription() const;
    // Resource id associated with the attribute NAME (the attr's own id), 0 if none.
    virtual int getAttributeNameResource(int index) const;
    virtual int getAttributeListValue(int index, const std::vector<std::string>& options,
                                      int defaultValue) const;
    virtual bool getAttributeBooleanValue(int index, bool defaultValue) const;
    // The attribute's VALUE as a resource id ("@type/key"), 0 if none.
    virtual int getAttributeResourceValue(int index, int defaultValue) const;
    virtual int getAttributeIntValue(int index, int defaultValue) const;
    virtual int getAttributeUnsignedIntValue(int index, int defaultValue) const;
    virtual float getAttributeFloatValue(int index, float defaultValue) const;
    virtual int getAttributeListValue(const std::string& namespace_,
                                      const std::string& attribute,
                                      const std::vector<std::string>& options,
                                      int defaultValue) const;
    virtual bool getAttributeBooleanValue(const std::string& namespace_,
                                          const std::string& attribute,
                                          bool defaultValue) const;
    virtual int getAttributeResourceValue(const std::string& namespace_,
                                          const std::string& attribute,
                                          int defaultValue) const;
    virtual int getAttributeIntValue(const std::string& namespace_,
                                     const std::string& attribute, int defaultValue) const;
    virtual int getAttributeUnsignedIntValue(const std::string& namespace_,
                                             const std::string& attribute,
                                             int defaultValue) const;
    virtual float getAttributeFloatValue(const std::string& namespace_,
                                         const std::string& attribute,
                                         float defaultValue) const;
    // The "id"/"class"/"style" special attributes (AOSP semantics).
    virtual std::string getIdAttribute() const;             // == getAttributeValue("id")
    virtual std::string getClassAttribute() const;          // == getAttributeValue("class")
    virtual int getIdAttributeResourceValue(int defaultValue) const;
    virtual int getStyleAttribute() const;                  // getAttributeResourceValue("style")
    AttributeSet& operator =(const AttributeSet&other);
    void dump()const;
};
}
#endif
