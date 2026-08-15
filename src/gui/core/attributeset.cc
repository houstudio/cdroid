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
#include <core/attributeset.h>
#include <widget/linearlayout.h>
#include <core/windowmanager.h>
#include <core/porterduff.h>
#include <core/xmlpullparser.h>
#include <core/color.h>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <porting/cdlog.h>

namespace cdroid{

static std::vector<std::string> split(const std::string & path) {
    std::vector<std::string> vec;
    size_t begin = path.find_first_not_of("|");
    while (begin != std::string::npos) {
        size_t end = path.find_first_of("|", begin);
        vec.push_back(path.substr(begin, end-begin));
        begin = path.find_first_not_of("|", end);
    }
    return vec;
}

AttributeSet::AttributeSet():AttributeSet(nullptr,""){
}

AttributeSet::AttributeSet(Context*ctx,const std::string&package)
    :mContext(ctx),mPackage(package){
    mAttrs = std::make_shared<std::unordered_map<std::string,std::string>>();
}

AttributeSet::AttributeSet(const AttributeSet&other):AttributeSet(other.mContext,other.mPackage){
    for(auto& a:*other.mAttrs){
        mAttrs->insert({a.first,a.second});
    }
    mAttrResIds = other.mAttrResIds;   // share the name->resId map (if any)
    mStyleResId = other.mStyleResId;
}

AttributeSet& AttributeSet::operator =(const AttributeSet&other){
    mContext = other.mContext;
    mPackage = other.mPackage;
    for(auto& a:*other.mAttrs){
        mAttrs->insert({a.first,a.second});
    }
    mAttrResIds = other.mAttrResIds;
    mStyleResId = other.mStyleResId;
    return *this;
}

Context*AttributeSet::getContext()const{
    return mContext;
}

const AttributeSet& AttributeSet::empty() {
    // Meyer's singleton — a default-constructed (no Context, no attrs) AttributeSet
    // used as a non-null stand-in for a null AttributeSet* in widget ctors.
    static const AttributeSet inst;
    return inst;
}

void AttributeSet::setContext(Context*ctx,const std::string&package){
    mContext = ctx;
    mPackage = package;
}

/*@android:+id/title ,?android:attr/windowContentOverlay*/
std::string AttributeSet::normalize(const std::string&pkg,const std::string&property){
    const bool hasColon = property.find(':')!=std::string::npos;
    const bool hasAT = property.size() && (property[0]=='@');
    if(hasColon&&(hasAT==false)) {
        if(property.compare(0,8,"android:")==0){
            std::string value = property;
            value[1] = 'c';/*android cahnge to cdroid*/
            return value.substr(1);
        }
        return property;
    }else {
        std::string value= property;
        const bool hasAsk= value.size() && (property[0]=='?');
        const bool hasSlash = value.find('/')!=std::string::npos;
        const bool isRes = (hasAT|hasAsk);// && hasSlash;
        if(isRes && (property.size()>1) ) {
            value.erase(0,1);
        }
        if(hasColon==false) {
            if( isRes && hasSlash ){
                value = std::string(pkg+":"+value);
            }else if(hasAsk && (property.size()>1) ) {
                value = std::string(pkg + ":attr/" + value);
            }
        }
        return value;
    }
}

int AttributeSet::set(const char*atts[],int size){
    int rc = 0;
    for(int i = 0;atts[i]&&(size==0||i<size);i+=2,rc+=1){
        const char* key = strrchr(atts[i],' ');
        if(key) key++;
        else key = atts[i];
        const std::string k(key);
        mAttrs->insert({k,normalize(mPackage,std::string(atts[i+1]))});
        // Record the attribute's own resource id (for getAttributeNameResource)
        // via the Android-aligned Resources.getIdentifier. No-op without a Context
        // or when already known; absent/unknown attrs stay at 0 (id interface returns 0).
        if (mContext && !(mAttrResIds && mAttrResIds->count(k))) {
            const int rid = mContext->getResources().getIdentifier(k, "attr", mPackage);
            if (rid) setAttributeResourceId(k, rid);
        }
    }
    return (int)mAttrs->size();
}

int AttributeSet::inherit(const AttributeSet&other){
    int inheritedCount = 0;
    const bool isSamePackage = (mPackage.compare(other.mPackage)==0);
    for(auto it = other.mAttrs->begin(); it != other.mAttrs->end() ; it++){
        if(mAttrs->find(it->first)==mAttrs->end()){
            if(isSamePackage){
                mAttrs->insert({it->first.c_str(),it->second});
            }else{
                mAttrs->insert({it->first,normalize(other.mPackage,it->second)});
            }
            inheritedCount++;
            // carry the attribute's resource id (so inherited style attrs keep
            // their resId for getAttributeNameResource).
            if (other.mAttrResIds) {
                auto ri = other.mAttrResIds->find(it->first);
                if (ri != other.mAttrResIds->end()) setAttributeResourceId(it->first, ri->second);
            }
        }
    }
    // Carry the source style resId: when a base AttributeSet inherits a resolved
    // style (e.g. TextView merges its textAppearance style into the element set),
    // the merged set must keep the style's resId so obtainStyledAttributes
    // routes it through the arsc theme resolver (non-binary styleResId branch).
    if (mStyleResId == 0) mStyleResId = other.mStyleResId;
    return inheritedCount;
}

int AttributeSet::Override(const AttributeSet&other){
    int overrideCount = 0;
    const bool isSamePackage = (mPackage.compare(other.mPackage)==0);
    for(auto it = other.mAttrs->begin(); it != other.mAttrs->end() ; it++){
        auto thisIter = mAttrs->find(it->first);
        if(thisIter==mAttrs->end()){
            if(isSamePackage){
                mAttrs->insert({it->first.c_str(),it->second});
            }else{
                mAttrs->insert({it->first,normalize(other.mPackage,it->second)});
            }
            overrideCount++;
        }else{
            if(isSamePackage){
                thisIter->second=it->second;
            }else{
                thisIter->second=normalize(other.mPackage,it->second);
            }
        }
    }
    return overrideCount;
}

bool AttributeSet::add(const std::string&key,const std::string&value){
    auto itr = mAttrs->find(key);
    std::string ks = key;
    size_t pos = ks.find(' ');
    if( pos != std::string::npos )ks = ks.substr(pos+1);
    if(itr == mAttrs->end()) {
        mAttrs->insert({(std::string)ks,normalize(mPackage,value)});
        // Record the attribute's own resource id (see set()).
        if (mContext && !(mAttrResIds && mAttrResIds->count(ks))) {
            const int rid = mContext->getResources().getIdentifier(ks, "attr", mPackage);
            if (rid) setAttributeResourceId(ks, rid);
        }
    } else {
        itr->second = value;
    }
    return true;
}

bool AttributeSet::hasAttribute(const std::string&key)const{
    return mAttrs->find(key)!=mAttrs->end();
}

void AttributeSet::setAttributeResourceId(const std::string& name, int resId) {
    if (!mAttrResIds) mAttrResIds = std::make_shared<std::unordered_map<std::string,int>>();
    (*mAttrResIds)[name] = resId;
}

size_t AttributeSet::getAttributeCount()const{
    return mAttrs->size();
}
const std::string AttributeSet::getAttributeValue(const std::string&key)const{
    auto it = mAttrs->find(key);
    if(it != mAttrs->end())
        return it->second;
    return std::string();
}

// ----------------------------------------------------------------------------
// AOSP android.util.AttributeSet — index/id-based methods (base / text impl).
// Ported from frameworks/base/core/java/android/util/AttributeSet.java. Index
// iterates mAttrs (small N; resolution matches by resId/name, not position, so
// the unordered order is fine). Each typed getter delegates to the existing
// string-key getter (reusing the parsing). Binary XmlPullParser overrides these
// via ResXMLTree (stable AXML order + typed Res_value + real attr resIds); the
// text/style path stays here.
// ----------------------------------------------------------------------------
namespace {
bool keyAt(const std::unordered_map<std::string,std::string>& m, size_t idx, std::string* out) {
    if (idx >= m.size()) return false;
    size_t i = 0;
    for (const auto& kv : m) {
        if (i == idx) {
            *out = kv.first;
            return true; 
        }
        i++;
    }
    return false;
}
}

std::string AttributeSet::getAttributeNamespace(int /*index*/) const {
    return std::string();   // text AttributeSet carries no namespace (bare localname keys)
}

std::string AttributeSet::getAttributeName(int index) const {
    std::string k;
    return keyAt(*mAttrs, (size_t)index, &k) ? k : std::string();
}

std::string AttributeSet::getAttributeValue(int index) const {
    std::string k;
    return keyAt(*mAttrs, (size_t)index, &k) ? getAttributeValue(k) : std::string();
}

std::string AttributeSet::getAttributeValue(const std::string& /*namespace_*/,
                        const std::string& name) const {
    return getAttributeValue(name);// namespace-agnostic for text (bare localname)
}

std::string AttributeSet::getPositionDescription() const {
    return std::string();
}

int AttributeSet::getAttributeNameResource(int index) const {
    std::string k;
    if (!keyAt(*mAttrs, (size_t)index, &k) || !mAttrResIds){
        return 0;
    }
    auto it = mAttrResIds->find(k);
    return it != mAttrResIds->end() ? it->second : 0;
}

int AttributeSet::getAttributeListValue(int index,
        const std::vector<std::string>& options, int defaultValue) const {
    const std::string v = getAttributeValue(index);
    for (size_t i = 0; i < options.size(); i++){
        if (options[i] == v) return (int)i;
    }
    return defaultValue;
}

bool AttributeSet::getAttributeBooleanValue(int index, bool defaultValue) const {
    std::string k;
    return keyAt(*mAttrs, (size_t)index, &k) ? getAttributeBooleanValue(std::string(), k, defaultValue) : defaultValue;
}

int AttributeSet::getAttributeResourceValue(int index, int defaultValue) const {
    std::string k;
    return keyAt(*mAttrs, (size_t)index, &k) ? getAttributeResourceValue(std::string(), k, defaultValue) : defaultValue;
}

int AttributeSet::getAttributeIntValue(int index, int defaultValue) const {
    std::string k;
    return keyAt(*mAttrs, (size_t)index, &k) ? getAttributeIntValue(std::string(), k, defaultValue) : defaultValue;
}

int AttributeSet::getAttributeUnsignedIntValue(int index, int defaultValue) const {
    std::string k;
    if (!keyAt(*mAttrs, (size_t)index, &k)) return defaultValue;
    const std::string v = getAttributeValue(k);
    if (!v.empty()) {
        if (v[0] == '#') return (int)Color::parseColor(v);
        if (v.size() >= 2 && v[0] == '0' && (v[1] == 'x' || v[1] == 'X'))
            return (int)strtoul(v.c_str() + 2, nullptr, 16);
    }
    return getAttributeIntValue(std::string(), k, defaultValue);
}

float AttributeSet::getAttributeFloatValue(int index, float defaultValue) const {
    std::string k;
    return keyAt(*mAttrs, (size_t)index, &k) ? getAttributeFloatValue(std::string(), k, defaultValue) : defaultValue;
}

int AttributeSet::getAttributeListValue(const std::string& /*namespace_*/,const std::string& attribute,
            const std::vector<std::string>& options, int defaultValue) const {
    const std::string v = getAttributeValue(attribute);
    for (size_t i = 0; i < options.size(); i++) if (options[i] == v) return (int)i;
    return defaultValue;
}

bool AttributeSet::getAttributeBooleanValue(const std::string& /*namespace_*/,
            const std::string& attribute, bool defaultValue) const {
    const std::string v = getAttributeValue(attribute);
    if (v.empty()) return defaultValue;
    return v.compare("true") == 0;
}

int AttributeSet::getAttributeResourceValue(const std::string& /*namespace_*/,
            const std::string& attribute,int defaultValue) const {
    const std::string v = getAttributeValue(attribute);
    if (v.empty()) return defaultValue;
    // "parent" is the ConstraintLayout/RelativeLayout anchor sentinel meaning
    // the parent view (id 0) — NOT a named resource; resolving it hits an
    // unrelated arsc entry named "parent" and breaks parent anchors.
    if (v == "parent") return 0;
    if (v.find_first_of("@+/") != std::string::npos) {
        std::string name = v;
        const size_t slash = name.rfind('/');
        if (slash != std::string::npos) name = name.substr(slash + 1);
        size_t at = 0;
        while (at < name.size() && (name[at]=='@'||name[at]=='+')) at++;
        if (at > 0) name = name.substr(at);
        const int value = mContext ? mContext->getResources().getIdentifier(name, "id", "") : 0;
        return value ? value : defaultValue;
    }
    return (int)std::strtoul(v.c_str(), nullptr, 10);
}

int AttributeSet::getAttributeIntValue(const std::string& /*namespace_*/,
            const std::string& attribute, int defaultValue) const {
    const std::string v = getAttributeValue(attribute);
    if (v.empty() || ((v[0] >= 'a') && (v[0] <= 'z'))) return defaultValue;
    const int base = (((v.length() > 2) && (v[1]=='x'||v[1]=='X')) || (v[0]=='#')) ? 16 : 10;
    return (int)std::strtol(v.c_str(), nullptr, base);
}

int AttributeSet::getAttributeUnsignedIntValue(const std::string& /*namespace_*/,
            const std::string& attribute, int defaultValue) const {
    const std::string v = getAttributeValue(attribute);
    if (!v.empty()) {
        if (v[0] == '#')
            return (int)Color::parseColor(v);
        if (v.size() >= 2 && v[0] == '0' && (v[1] == 'x' || v[1] == 'X'))
            return (int)strtoul(v.c_str() + 2, nullptr, 16);
    }
    return getAttributeIntValue(std::string(), attribute, defaultValue);
}

float AttributeSet::getAttributeFloatValue(const std::string& /*namespace_*/,
            const std::string& attribute,float defaultValue) const {
    const std::string v = getAttributeValue(attribute);
    if (v.empty()) return defaultValue;
    return std::strtof(v.c_str(), nullptr);
}

std::string AttributeSet::getIdAttribute() const {
    return getAttributeValue("id");
}

std::string AttributeSet::getClassAttribute() const {
    return getAttributeValue("class");
}

int AttributeSet::getIdAttributeResourceValue(int defaultValue) const {
    return getAttributeResourceValue(std::string(), "id", defaultValue);
}

int AttributeSet::getStyleAttribute() const {
    return getAttributeResourceValue(std::string(), "style", 0);
}

void AttributeSet::dump()const{
    for(auto it = mAttrs->begin();it != mAttrs->end();it++){
        LOGD("%s = %s",it->first.c_str(),it->second.c_str());
    }
}

}
