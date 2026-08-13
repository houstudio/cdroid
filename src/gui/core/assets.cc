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
#include <assets.h>
#include <core/typedarray.h>   // TypedArray (constructed in obtainStyledAttributes)
#include "androidfw/LocaleData.h"  // localeDataComputeScript (arsc locale config)
#include "core/assetmanager.h"   // AssetManager
#include "resources.h"         // cdroid::Resources
#include <algorithm>
#include <cdtypes.h>
#include <cdlog.h>
#include <ziparchive.h>
#include <iostreams.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <utils/textutils.h>
#include <limits.h>
#include <unistd.h>
#include <core/systemclock.h>
#include <drawable/drawables.h>
#include <drawable/drawableinflater.h>
#include <image-decoders/imagedecoder.h>

using namespace Cairo;
namespace cdroid {

static std::string renderResValue(const Assets* a, const Res_value& v);  // fwd
static std::string u16toUtf8(const char16_t* s, size_t len);  // fwd (def below)

// Resolve a resource ID through the loaded arsc.
bool Assets::arscResolveId(uint32_t resId, Res_value* out) const {
    if (!mResTable || resId == 0 || resId == 0xFFFFFFFF) return false;
    return mResTable->getResource(resId, out) >= 0;
}

// Get a string from the arsc string pool by resource ID.
const char16_t* Assets::arscStringAt(uint32_t resId, size_t* outLen) const {
    if (!mResTable || resId == 0) return nullptr;
    return mResTable->getResourceString(resId, outLen);
}

// Render a resource ID as "@type/key" (text-XML reference form) so binary-AXML
// references flow through the same resolution paths as text XML. Returns "" if
// the arsc can't name the resource (caller falls back to "@0x..").
std::string Assets::getResourceName(uint32_t resId) const {
    if (!mResTable || resId == 0) return "";
    std::string pkg, type, key;
    if (mResTable->getResourceName(resId, &pkg, &type, &key) && !type.empty() && !key.empty()) {
        // Framework resources need the explicit package prefix ("@android:...");
        // app resources resolve under the default package, so omit it (matches
        // text-XML conventions).
        if (pkg == "android") return "@android:" + type + "/" + key;
        return "@" + type + "/" + key;
    }
    return "";
}

// Resolve a theme-attribute reference (?attr/<id>) through the arsc Theme.
bool Assets::arscThemeAttribute(uint32_t attrId, Res_value* out, ssize_t* outBlock) const {
    if (!mArscTheme || !out) return false;
    Res_value v;
    ssize_t blk = mArscTheme->getAttribute(attrId, &v);
    if (blk < 0) return false;
    // Flatten ?attr / @ref chains to a concrete value.
    blk = mArscTheme->resolveAttributeReference(&v, blk);
    if (blk < 0) return false;
    *out = v;
    if (outBlock) *outBlock = blk;
    return true;
}

// Resolve a theme attribute NAME to its value string. Text mTheme first; in
// SDK/binary mode mTheme is empty (values only in resources.arsc), so fall back
// to the arsc Theme. pkg is a hint (arscGetIdentifier also tries android/any).
std::string Assets::themeString(const std::string& key, const std::string& pkg) const {
    std::string v = mTheme.getString(key);
    if (!v.empty() || !mArscTheme || !mResTable) return v;
    uint32_t attrId = arscGetIdentifier(key, "attr", pkg);
    Res_value tv;
    ssize_t blk = -1;
    if (attrId && arscThemeAttribute(attrId, &tv, &blk) && tv.data != 0) {
        // aapt2 stores color/drawable theme values as the file path (TYPE_STRING),
        // not a reference id; resolve the string from the owning pool block so the
        // caller (e.g. getColorStateList) can re-resolve it. renderResValue returns
        // empty for TYPE_STRING, so handle it here.
        if (tv.dataType == Res_value::TYPE_STRING && blk >= 0) {
            size_t len = 0;
            const char16_t* s = mResTable->stringAtBlock(blk, tv.data, &len);
            if (s && len) return u16toUtf8(s, len);
        }
        return renderResValue(this, tv);
    }
    return std::string();
}

// Resolve a "?type/key" theme-attribute reference to a concrete value string.
std::string Assets::resolveThemeRef(const std::string& resid) const {
    if (resid.empty() || resid[0] != '?' || !mArscTheme) return resid;
    std::string pkg, name = resid.substr(1);  // strip '?'
    parseResource(name, &name, &pkg);
    size_t slash = name.find('/');
    std::string type = (slash != std::string::npos) ? name.substr(0, slash) : "attr";
    std::string key  = (slash != std::string::npos) ? name.substr(slash + 1) : name;
    uint32_t attrId = arscGetIdentifier(key, type, pkg);
    if (!attrId) return resid;
    Res_value v;
    if (!arscThemeAttribute(attrId, &v)) return resid;
    std::string rendered = renderResValue(this, v);
    return rendered.empty() ? resid : rendered;
}

// Try to resolve a "@0xPPtteeee" hex resource ID string through the arsc.
bool Assets::arscResolveHexRef(const std::string& s, Res_value* out) const {
    if (!mResTable || s.empty()) return false;
    // Accept "@0x...", "0x...", or a bare hex tail after the last '@'.
    size_t at = s.rfind('@');
    std::string hex = (at != std::string::npos) ? s.substr(at + 1) : s;
    if (hex.compare(0, 2, "0x") != 0 && hex.compare(0, 2, "0X") != 0) return false;
    char* end = nullptr;
    errno = 0;
    unsigned long id = strtoul(hex.c_str() + 2, &end, 16);
    if (errno || end == hex.c_str() + 2 || id == 0 || id == 0xFFFFFFFF) return false;
    return mResTable->getResource((uint32_t)id, out) >= 0;
}

// arsc identifier lookup. aapt2 forces a dotted package name (e.g.
// "cdroid.axmlapp") that won't match CDROID's pak name ("axmlapp"), so after
// trying the requested package and the framework ("android"), fall back to a
// name-only search across ALL loaded packages (empty package = search all).
uint32_t Assets::arscGetIdentifier(const std::string& name, const std::string& type, const std::string& pkg) const {
    if (!mResTable || mResTable->getError() != 0) return 0;
    // Strip type prefix: "attr/colorOnPrimary" → "colorOnPrimary"
    std::string cleanName = name;
    size_t slash = cleanName.find('/');
    if (slash != std::string::npos) cleanName = cleanName.substr(slash + 1);
    if (cleanName.empty()) return 0;
    if (!pkg.empty()) {
        uint32_t id = mResTable->getIdentifier(cleanName, type, pkg);
        if (id) return id;
        // aapt2 forces a dotted package name ("cdroid.<ns>"); try that prefix
        // before the expensive all-package scan.
        id = mResTable->getIdentifier(cleanName, type, "cdroid." + pkg);
        if (id) return id;
    }
    uint32_t id = mResTable->getIdentifier(cleanName, type, "android");
    if (id) return id;
    return mResTable->getIdentifier(cleanName, type, "");  // any package
}

// complexToFloat is provided inline by <androidfw/resourcetypes.h>.

// Render a Res_value to the same string form renderTypedValue produces, so a
// theme-resolved value can flow through the string-based getters. Only the
// types a theme attribute realistically resolves to (color/int/dimension/ref).
static std::string renderResValue(const Assets* a, const Res_value& v) {
    char buf[32];
    switch (v.dataType) {
        case Res_value::TYPE_INT_COLOR_ARGB8:
        case Res_value::TYPE_INT_COLOR_RGB8:
        case Res_value::TYPE_INT_COLOR_ARGB4:
        case Res_value::TYPE_INT_COLOR_RGB4:
            snprintf(buf, sizeof(buf), "#%08x", v.data); return buf;
        case Res_value::TYPE_INT_DEC:  snprintf(buf, sizeof(buf), "%d", (int)v.data); return buf;
        case Res_value::TYPE_INT_HEX:  snprintf(buf, sizeof(buf), "0x%x", v.data); return buf;
        case Res_value::TYPE_INT_BOOLEAN: return v.data ? "true" : "false";
        case Res_value::TYPE_DIMENSION: {
            float mag = complexToFloat(v.data);
            int unit = (v.data >> Res_value::COMPLEX_UNIT_SHIFT) & Res_value::COMPLEX_UNIT_MASK;
            const char* u = unit == Res_value::COMPLEX_UNIT_SP ? "sp"
                          : unit == Res_value::COMPLEX_UNIT_DIP ? "dp" : "px";
            snprintf(buf, sizeof(buf), "%d%s", (int)mag, u); return buf;
        }
        case Res_value::TYPE_REFERENCE:
        case Res_value::TYPE_DYNAMIC_REFERENCE:
            return a->getResourceName(v.data);
        default: return std::string();
    }
}

// char16_t -> UTF-8 (for ResTable string values).
static std::string u16toUtf8(const char16_t* s, size_t len) {
    std::string out;
    for (size_t i = 0; s && i < len; i++) {
        uint32_t c = s[i];
        if (c >= 0xD800 && c <= 0xDBFF && i + 1 < len && s[i+1] >= 0xDC00)
            c = 0x10000 + ((c - 0xD800) << 10) + (s[++i] - 0xDC00);
        if (c < 0x80) out += (char)c;
        else if (c < 0x800) { out += (char)(0xC0|(c>>6)); out += (char)(0x80|(c&0x3F)); }
        else if (c < 0x10000) { out += (char)(0xE0|(c>>12)); out += (char)(0x80|((c>>6)&0x3F)); out += (char)(0x80|(c&0x3F)); }
        else { out += (char)(0xF0|(c>>18)); out += (char)(0x80|((c>>12)&0x3F)); out += (char)(0x80|((c>>6)&0x3F)); out += (char)(0x80|(c&0x3F)); }
    }
    return out;
}

Assets::Assets() {
    mNextAutofillViewId=100000;
    mResTable = nullptr;
}

Assets::Assets(const std::string&path):Assets() {
    addResource(path);
}

Assets::~Assets() {
    delete mCdroidResources;   // holds mAssetManager as a borrowed pointer
    delete mAssetManager;
    delete mArscTheme;
    delete mResTable;
    for(auto& cls:mStateColors){
        //delete cls.second;
    }
    mStateColors.clear();

    for(auto it=mResources.begin(); it!=mResources.end(); it++) {
        delete it->second;
    }
    for(auto& d:mDrawables) {
        LOGV_IF(d.second.use_count(),"%s reference=%d",d.first.c_str(),d.second.use_count());
    }
    mDrawables.clear();
    mIDS.clear();
    mResources.clear();
    mStrings.clear();
    mStyles.clear();
    LOGD("~Assets %p!",this);
}

// --- Lazy ID-based resource layer (AOSP Resources/AssetManager) ---
// Built on first use from the pak paths recorded in addResource(); the legacy
// string-based mResTable path is untouched.
void Assets::ensureCdroidResources() const {
    if (mCdroidResources != nullptr) return;
    if (mAssetManager == nullptr) {
        mAssetManager = new AssetManager();
        for (const auto& p : mPakPaths) {
            mAssetManager->addAssetPath(p, nullptr);
        }
        // Share the arsc table Assets already parsed (addResource reads each
        // pak's resources.arsc into mResTable once). Without this, the lazy
        // AssetManager would re-read and re-parse the very same arsc a second
        // time when getResources() first touches it. mResTable is borrowed here
        // and freed by ~Assets after the AssetManager is destroyed. The Header
        // cookie differs (-1 here vs 1-based in appendPathToResTable) but the
        // engine never reads the cookie, and raw files are opened by path
        // (no-cookie openNonAsset), so sharing is safe.
        if (mResTable) mAssetManager->setResTable(mResTable);
    }
    mCdroidResources = new cdroid::Resources(mAssetManager, const_cast<Assets*>(this));
}

Resources& Assets::getResources() {
    ensureCdroidResources();
    return *mCdroidResources;
}

AssetManager& Assets::getAssets() {
    ensureCdroidResources();
    return *mAssetManager;
}

Drawable* Assets::getDrawable(int id) {
    ensureCdroidResources();
    if (mCdroidResources == nullptr) return nullptr;
    return mCdroidResources->getDrawable(id);   // delegates to the ID path (Assets retirement)
}

ColorStateList* Assets::getColorStateList(int id) {
    ensureCdroidResources();
    if (mCdroidResources == nullptr) return nullptr;
    return mCdroidResources->getColorStateList(id);
}

const DisplayMetrics& Assets::getDisplayMetrics()const{
    return mDisplayMetrics;
}

const std::string Assets::getPackageName()const {
    return mName;
}

const std::string Assets::getThemeName() const {
    return mThemeName;
}

ResTable::Theme& Assets::getTheme() {
    // Lazily build an arsc theme if none has been applied yet, so the returned
    // reference is always valid (AOSP getTheme() never returns null). Binary
    // mode always has mResTable; the static fallback covers text-only paks.
    if (!mArscTheme && mResTable) {
        mArscTheme = new ResTable::Theme(*mResTable);
    }
    if (mArscTheme) return *mArscTheme;
    static ResTable sEmptyTable;
    static ResTable::Theme sEmptyTheme(sEmptyTable);
    return sEmptyTheme;
}

void Assets::setTheme(const std::string&theme) {
    auto it = mStyles.find(theme);
    if(it!=mStyles.end()) {
        std::string pkg;
        mThemeName= theme;
        mTheme = it->second;
        parseResource(theme,nullptr,&pkg);
        LOGD("set Theme to %s",theme.c_str());
    } else {
        LOGE("Theme %s not found,[cdroid.pak %s] must be copied to your work directory!",theme.c_str(),
             mName.empty()?"":(mName+".pak").c_str());
    }
    // (Re)build the arsc-backed Theme for ?attr resolution. The text mTheme
    // above stays for the existing string path; mArscTheme adds typed theme
    // lookups (?android:colorPrimary etc.) via ResTable::Theme.
    delete mArscTheme;
    mArscTheme = nullptr;
    if (mResTable) {
        std::string pkg, name = theme;
        parseResource(theme, &name, &pkg);
        size_t slash = name.rfind('/');
        std::string styleName = (slash != std::string::npos) ? name.substr(slash + 1) : name;
        uint32_t styleId = arscGetIdentifier(styleName, "style", pkg);
        if (styleId) {
            mArscTheme = new ResTable::Theme(*mResTable);
            if (mArscTheme->applyStyle(styleId) != 0) {
                LOGW("arsc Theme applyStyle(%s) failed", theme.c_str());
                delete mArscTheme;
                mArscTheme = nullptr;
            } else {
                LOGD("arsc Theme built from %s (resId=0x%08x)", theme.c_str(), styleId);
            }
        }
    }
}

void Assets::setTheme(int resid) {
    // ID-based theme apply (AOSP Context.setTheme(int @StyleRes)): rebuild the
    // arsc-backed theme directly from a style resource id, skipping the name
    // lookup the string overload performs.
    delete mArscTheme;
    mArscTheme = nullptr;
    if (mResTable && resid) {
        mArscTheme = new ResTable::Theme(*mResTable);
        if (mArscTheme->applyStyle((uint32_t)resid) != 0) {
            LOGW("arsc Theme applyStyle(resId=0x%08x) failed", resid);
            delete mArscTheme;
            mArscTheme = nullptr;
        } else {
            LOGD("arsc Theme built from resId=0x%08x", resid);
        }
    }
}


static std::string convertXmlToCString(const std::string& xml) {
    static std::unordered_map<std::string, std::string> escapeMap = {
        {"\\n", "\n"},     {"\\'", "\'"},   {"\\\"", "\""}
    };
    std::string result;
    result.reserve(xml.length());

    for (size_t i = 0; i < xml.length(); ++i) {
        bool replaced = false;
        for (const auto& pair : escapeMap) {
            if (xml.compare(i, pair.first.length(), pair.first) == 0) {
                result.append(pair.second);
                i += pair.first.length() - 1;
                replaced = true;
                break;
            }
        }
        if (!replaced) {
            result += xml[i];
        }
    }
    return result;
}

typedef struct{
    std::unordered_map<std::string,const std::string>colors;
    std::unordered_map<std::string,const std::string>dimens;
    std::unordered_map<std::string,std::vector<AttributeSet>>colorStateList;
}PENDINGRESOURCE;

static std::string getTrimedValue(XmlPullParser&parser){
    int type;
    std::string value;
    while((type=parser.next())!=XmlPullParser::END_TAG){
        if(type==XmlPullParser::TEXT){
            value.append(parser.getText());
        }
    }
    TextUtils::trim(value);
    return value;
}

int Assets::loadKeyValues(const std::string&package,const std::string&resid,void*params){
    int type,depth;
    XmlPullParser parser(this,resid);
    const AttributeSet& attrs=(AttributeSet&)parser;
    PENDINGRESOURCE*pending=(PENDINGRESOURCE*)params;
    while((type=parser.next())!=XmlPullParser::END_DOCUMENT){
        const std::string tag = parser.getName();
        if(type!=XmlPullParser::START_TAG)continue;
        if(tag.compare("id")==0){
            std::string key = package +":id/"+attrs.getString("name");
            std::string value= getTrimedValue(parser);
            mIDS[key] = TextUtils::strtol(value);
        }else if((tag.compare("dimen")==0)||(tag.compare("integer")==0)||(tag.compare("bool")==0)){
            const std::string resUri = package+":"+tag+"/"+attrs.getString("name");
            std::string value = getTrimedValue(parser);
            const std::string dimenRes = AttributeSet::normalize(package,value);
            auto itc = mDimensions.find(dimenRes);
            if(value.find("/")==std::string::npos){
                char*endP;
                int v = std::strtol(value.c_str(),&endP,10);
                if(*endP){
                    const DisplayMetrics& dm = getDisplayMetrics();
                    if(*endP=='s'/*sp*/) v = int(dm.scaledDensity * v /*+0.5f*/);
                    else if(*endP=='d'/*dp dip*/)v =int(dm.density * v /*+0.5f*/);
                }
                if(tag.compare("bool")==0){
                    v = value[0]=='t'?true:false;
                }
                mDimensions.insert({resUri,v});
            }else if(itc!=mDimensions.end()){
                mDimensions.insert({resUri,itc->second});
            }else{
                pending->dimens.insert({resUri,dimenRes});
            }
        }else if(tag.compare("color")==0){
            std::string colorUri = package+":color/"+attrs.getString("name");
            std::string value = getTrimedValue(parser);
            const std::string colorRef = AttributeSet::normalize(package,value);
            auto itc = mColors.find(colorRef);
            if((value[0]=='#')||(itc!=mColors.end())){
                const uint32_t color = (value[0]=='#')?Color::parseColor(value):itc->second;
                mColors.insert({colorUri,color});
            }else if (itc==mColors.end()){
                pending->colors.insert({colorUri,colorRef});
            }
        }else if(tag.compare("string")==0){
            std::string key = package+":string/"+attrs.getString("name");
            std::string value = getTrimedValue(parser);
            mStrings[key] = convertXmlToCString(value);
        }else if(tag.compare("item")==0){
            const std::string type = attrs.getString("type");
            if(type.compare("dimen")==0||type.compare("integer")==0||type.compare("bool")==0||type.compare("fraction")==0){
                const std::string resUri = package+":dimen/"+attrs.getString("name");
                const std::string format = attrs.getString("format");
                std::string value = getTrimedValue(parser);
                if((format.compare("float")==0)||(type[0]=='f')){
                    float fv =std::strtof(value.c_str(),nullptr);
                    if(type[0]=='f') fv/=100.f;
                    mDimensions.insert({resUri,fv});
                }else{
                    const int32_t v = std::stol(value);
                    mDimensions.insert({resUri,v});
                }
            }else if(type.compare("id")==0){
                // <item type="id" name="x">value</item> declares an id resource,
                // the item-form equivalent of the <id> tag above.
                std::string key = package+":id/"+attrs.getString("name");
                std::string value = getTrimedValue(parser);
                mIDS[key] = TextUtils::strtol(value);
            }
        }else if(tag.compare("selector")==0){//for colorstatelist
            std::string key = attrs.getString("name");
            depth = parser.getDepth()+1;
            std::string resUri = resid.substr(0,resid.find(".xml"));
            std::unordered_map<std::string,std::vector<AttributeSet>>::iterator it;
            it = pending->colorStateList.end();
            while(((type=parser.next())!=XmlPullParser::END_DOCUMENT) && (parser.getDepth()>=depth) ){
                if(type!=XmlPullParser::START_TAG)continue;
                AttributeSet itemAtts(attrs);
                //itemAtts = attrs;
                if(it==pending->colorStateList.end()){
                    it = pending->colorStateList.insert({resUri,{itemAtts}}).first;
                }else
                    it->second.emplace_back(itemAtts);
            }
        }else if(tag.compare("style")==0){
            const std::string styleName = package+":style/"+attrs.getString("name");
            auto its =mStyles.find(styleName);
            if(its==mStyles.end()){
                const std::string styleParent = attrs.getString("parent");
                its =mStyles.insert(its,{styleName,AttributeSet(this,package)});
                if(styleParent.size())its->second.add("parent",styleParent);
            }
            depth = parser.getDepth()+1;
            while(((type=parser.next())!=XmlPullParser::END_DOCUMENT) && (parser.getDepth()>=depth) ){
                if(type!=XmlPullParser::START_TAG)continue;
                std::string key  = attrs.getString("name");
                std::string value= getTrimedValue(parser);
                value = AttributeSet::normalize(package,value);
                const size_t pos =key.find(':');
                if(pos!=std::string::npos)key=key.substr(pos+1);
                its->second.add(key,value);
            }
        }else if(tag.find("array")!=std::string::npos){
            const std::string key = package+":array/"+attrs.getString("name");
            std::vector<std::string>array;
            depth = parser.getDepth()+1;
            while(((type=parser.next())!=XmlPullParser::END_DOCUMENT) && (parser.getDepth()>=depth) ){
                if(type!=XmlPullParser::START_TAG)continue;
                std::string value= getTrimedValue(parser);
                array.emplace_back(value);
            }
            mArraies.emplace(key,std::move(array));
        }
    }
    return 0;
}

int Assets::addResource(const std::string&path,const std::string&name) {
    mPakPaths.push_back(path);   // recorded for the lazy ID-based AssetManager
    // If the lazy AssetManager was already built — which happens when an earlier
    // pak's addResource triggered ensureCdroidResources() via the pending
    // color-state-list resolve (getColorStateList → getResources) — register this
    // pak with it too. Otherwise files in later paks (e.g. app layouts in
    // uidemo1.pak, added after cdroid.pak) are invisible to openNonAsset, and
    // every app layout inflate returns null.
    if (mAssetManager) mAssetManager->addAssetPath(path, nullptr);
    ZIPArchive*pak = new ZIPArchive(path);
    std::string package = name;
    if(name.empty()) {
        size_t pos=path.find_last_of('/');
        if(pos != std::string::npos)
            package = path.substr(pos+1);
        pos = package.find('.');
        if( pos != std::string::npos)
            package = package.substr(0,pos);
    }
    mResources.insert({package,pak});

    int count=0;
    PENDINGRESOURCE pending;
    auto sttm = SystemClock::uptimeMillis();
    pak->forEachEntry([this,package,pak,&count,&pending](const std::string&res) {
        count++;
        if((res.size()>6)&&(TextUtils::startWith(res,"values")||TextUtils::startWith(res,"color"))) {
            // Skip binary AXML entries (SDK framework res — already in arsc).
            // Binary color/selector files have <selector> root, not <resources>;
            // parsing them as text corrupts mColors/mStateColors.
            std::istream* s = pak->getInputStream(res);
            if (s) {
                char magic[2] = {0};
                s->read(magic, 2);
                delete s;
                if ((uint8_t)magic[0] == 0x03) return 0; // binary AXML — skip
            }
            LOGV("LoadKeyValues from:%s",res.c_str());
            loadKeyValues(package,package+":"+res,&pending);
        }
        return 0;
    });
    // Load resources.arsc if present. Try getInputStream directly rather than
    // hasEntry: cdroid.pak carries duplicate color/ entries (SDK + own), and
    // libzip's zip_name_locate (used by hasEntry) fails to resolve some names
    // in such archives, while zip_fopen (getInputStream) still works. Each pak
    // is one add() = one owning Header; copyData=true makes ResTable malloc its
    // own copy, so the local buffer can be freed safely across multiple paks.
    auto stream = std::unique_ptr<std::istream>(pak->getInputStream("resources.arsc"));
    if (stream && *stream) {
        std::string data((std::istreambuf_iterator<char>(*stream)),
                         std::istreambuf_iterator<char>());
        if (!mResTable) mResTable = new ResTable();
        // NOTE: the 4-arg form is required so `true` binds to copyData, not to
        // the int32_t cookie of the 3-arg overload — otherwise copyData defaults
        // to false, hdr->data aliases the local buffer, and freeing it on return
        // leaves every Package type/key pointer dangling (UAF).
        mResTable->add(data.data(), data.size(), /*cookie*/-1, /*copyData*/true);
        LOGD("Loaded resources.arsc from %s (%zu bytes, error=%d)",
             path.c_str(), data.size(), mResTable->getError());
    }
    if(name.compare("cdroid")==0){
        //setTheme("cdroid:style/Theme");
        setTheme("cdroid:style/Theme.Material");
    }

    while (!pending.colors.empty()) {
        bool resolved = false;
        for (auto it = pending.colors.begin(); it != pending.colors.end(); ) {
            auto found = mColors.find(it->second);
            if (found != mColors.end()) {
                mColors.insert({it->first, found->second});
                it = pending.colors.erase(it);
                resolved = true;
            } else {
                ++it;
            }
        }
        if (!resolved) break;
    }

    while (!pending.dimens.empty()) {
        bool resolved = false;
        for (auto it = pending.dimens.begin(); it != pending.dimens.end(); ) {
            auto found = mDimensions.find(it->second);
            if (found != mDimensions.end()) {
                mDimensions.insert({it->first, found->second});
                it = pending.dimens.erase(it);
                resolved = true;
            } else {
                ++it;
            }
        }
        if (!resolved) break;
    }

    for (auto& c : pending.colors) {
        LOGD("color %s-->%s unresolved", c.first.c_str(), c.second.c_str());
    }
    for (auto& d : pending.dimens) {
        LOGD("dimen %s-->%s unresolved", d.first.c_str(), d.second.c_str());
    }
    while (!pending.colorStateList.empty()) {
        bool resolved = false;
        for (auto it = pending.colorStateList.begin(); it != pending.colorStateList.end(); ) {
            // Resolve each pending color-state-list by name through the apk
            // id-path (Assets::getColorStateList(name) → Resources::loadComplexColor(id),
            // cached). Forward references that can't resolve yet stay pending for
            // the next pass.
            if (getColorStateList(it->first)) {
                it = pending.colorStateList.erase(it);
                resolved = true;
            } else {
                LOGD("%s tobe done", it->first.c_str());
                ++it;
            }
        }
        if (!resolved) break;
    }
    for(auto c:pending.colorStateList){
        LOGD("colorStateList %s unresolved", c.first.c_str());
    }
    const size_t preloadCount = mColors.size()+mDimensions.size()+mStateColors.size()+mArraies.size()+mStyles.size()+mStrings.size();
    LOGI("[%s] load %d assets from %d files [%d id,%d colors,%d stateColors, %d array,%d style,%d string,%d dimens] mTheme.size=%d used %dms",
         package.c_str(),preloadCount,count, mIDS.size(),mColors.size(),mStateColors.size(),mArraies.size(), mStyles.size(),
         mStrings.size(),mDimensions.size(),mTheme.getAttributeCount(),int(SystemClock::uptimeMillis()-sttm));
    return pak?0:-1;
}

static bool guessExtension(ZIPArchive*pak,std::string&ioname) {
    static const char* exts[]={".xml",".9.png",".png",".jpg",".gif",".apng",".webp",nullptr};
    if(ioname.find('.')!=std::string::npos)
        return true;
    for(int i=0;exts[i];i++){
        if(pak->hasEntry(ioname+exts[i],false)){
            ioname += exts[i];
            return true;
        }
    }
    return false;
}

//"@[package:][+]id/filname"
const std::string Assets::parseResource(const std::string&fullResId,std::string*res,std::string*ns)const {
    std::string pkg = mName;
    std::string relname= fullResId;
    std::string fullid = fullResId;

    size_t pos = fullid.find_last_of("@+");
    if(pos!=std::string::npos)fullid =fullid.erase(0,pos+1);

    pos= fullid.find(":");
    if(pos != std::string::npos) {
        pkg = fullid.substr(0,pos);
        relname = fullid.substr(pos+1);
    } else { //id/xxx
        pos = mName.find_last_of('/');
        if(pos != std::string::npos)
            pkg = mName.substr(pos+1);
        relname = fullid;
    }
    if(pkg =="android") pkg="cdroid";
    if( ns) *ns = pkg;
    if(res)*res = relname;
    return pkg+":"+relname;
}

ZIPArchive*Assets::getResource(const std::string&fullResId,std::string*relativeResID,std::string*outPackage)const {
    std::string package,resname;
    parseResource(fullResId,&resname,&package);
    auto it = mResources.find(package);
    ZIPArchive* pak = nullptr;
    if(outPackage) *outPackage = package;
    if(it != mResources.end()) { //convert noextname ->extname.
        pak = it->second;
        guessExtension(pak,resname);
        if(relativeResID) *relativeResID = resname;
    }
    LOGV_IF(pak==nullptr && resname.size(),"resource for [%s] is%s found",fullResId.c_str(),(pak?"":" not"));
    return pak;
}

std::unique_ptr<std::istream> Assets::getInputStream(const std::string&fullresid,std::string*outpkg) {
    // A theme-attribute reference isn't a streamable resource — resolve it
    // first. If it resolves to a color/literal (not a file), fall through to
    // the not-found path rather than leaking "?..." to zip lookup.
    std::string rid = (!fullresid.empty() && fullresid[0] == '?') ? resolveThemeRef(fullresid) : fullresid;
    if (rid != fullresid && (rid.empty() || rid[0] == '#' || rid.compare(0,2,"0x")==0))
        return nullptr;  // resolved to a non-stream value (color/int)
    const std::string& effective = (rid != fullresid) ? rid : fullresid;
    std::string resname,package;
    ZIPArchive*pak = getResource(effective,&resname,&package);
    if(outpkg)*outpkg = package;
    std::istream*stream = pak ? pak->getInputStream(resname) : nullptr;
    // Fallback: a "@drawable/..." reference names a resource, not a file —
    // resolve it through the arsc to the qualified PNG path (e.g.
    // drawable-hdpi-v4/foo.9.png), like getDrawable does. Needed for 9-patch
    // src and other image loads that go through getInputStream.
    if(!stream && mResTable && effective.find("drawable/") != std::string::npos){
        std::string rawName;
        parseResource(effective, &rawName, &package);
        uint32_t id = arscGetIdentifier(rawName, "drawable", package);
        if(id != 0){
            Res_value v;
            if(mResTable->getResource(id, &v) >= 0 && v.dataType == Res_value::TYPE_STRING){
                size_t len = 0;
                const char16_t* s = mResTable->getResourceString(id, &len);
                if(s && len > 0){
                    std::string path = u16toUtf8(s, len);
                    if(path.substr(0, 4) == "res/") path = path.substr(4);
                    if(!path.empty()){
                        ZIPArchive* pak2 = getResource(package + ":" + path, &resname, &package);
                        if(pak2) stream = pak2->getInputStream(resname);
                    }
                }
            }
        }
    }
    if(stream)return std::unique_ptr<std::istream>(stream);
    if( fullresid.empty() || resname.empty() || (access(fullresid.c_str(),F_OK)<0)){
        LOGD("resoure:\"%s\" not found",fullresid.c_str());
        return nullptr;
    }
    return std::make_unique<std::ifstream>(fullresid);
}

// Set the arsc request locale so getResource/getResourceString pick the matching
// locale variant (mirrors the test config construction: packLanguage/Region +
// localeDataComputeScript). Binary apps read localized strings from arsc; text
// apps still use loadStrings for their app-local values.
void Assets::applyLocale(const std::string& lan) {
    if (!mResTable || lan.empty()) return;
    std::string lang = lan, region;
    size_t sep = lan.find_first_of("_-");
    if (sep != std::string::npos) { lang = lan.substr(0, sep); region = lan.substr(sep + 1); }
    ResTable_config cfg = {};
    if (lang.size() >= 2) cfg.packLanguage(lang.substr(0, 2).c_str());
    if (region.size() >= 2) cfg.packRegion(region.substr(0, 2).c_str());
    char script[4] = {0, 0, 0, 0};
    localeDataComputeScript(script, cfg.language, cfg.country);
    memcpy(cfg.localeScript, script, 4);
    cfg.localeScriptWasComputed = true;
    mResTable->setParameters(&cfg);
}

void Assets::loadStrings(const std::string&lan) {
    const std::string suffix = "/strings-"+lan+".xml";
    for(auto& a:mResources) {
        std::vector<std::string>files;
        a.second->getEntries(files);
        for(auto& fileName:files){
            if( (TextUtils::endWith(fileName,".xml") && TextUtils::endWith(fileName,suffix) )==false)continue;
            loadKeyValues(a.first,fileName,nullptr);
            LOGD("load %s for '%s'",fileName.c_str(),lan.c_str());
        }
    }
}

Cairo::RefPtr<Cairo::ImageSurface> Assets::loadImage(std::istream&stream,int width,int height){
   return ImageDecoder::loadImage(stream,width,height); 
}

Cairo::RefPtr<Cairo::ImageSurface> Assets::loadImage(const std::string&resname,int width,int height){
    if(!resname.empty()&&resname.compare("null")){
        std::unique_ptr<std::istream> stm = getInputStream(resname);
        if(stm) return loadImage(*stm,width,height);
    }
    return nullptr;
}

int Assets::getId(const std::string&resname)const {
    std::string resid,pkg;
    std::string key = resname;
    if(key.empty())return -1;
    if(key.length()&&(key.find('/')==std::string::npos)) {
        // Bare value (no '/'): a pure numeric id ("42") or a bare name ("cs_prev", "parent").
        // strtol resolves numerics; a bare name is resolved as an id reference against the id table
        // (Android Resources.getIdentifier(name,"id",pkg)). Falling back to strtol (==0) when the
        // name is unregistered preserves the legacy "parent" -> PARENT_ID(0) convention — otherwise
        // a bare name strtol()d to 0 and every MotionScene ConstraintSet id collided at 0.
        char* endP = nullptr;
        const long v = std::strtol(key.c_str(), &endP, 10);
        if (*endP == '\0') return (int)v;            // whole string consumed → pure numeric
        const int rid = getId("@id/" + key);         // bare name → id table via the prefixed path
        return (rid != -1) ? rid : (int)v;           // unregistered → strtol fallback (parent -> 0)
    }
    auto pos = key.find('+');
    if(pos != std::string::npos)
        key.erase(pos,1);
    parseResource(key,&resid,&pkg);

    // arsc is the single id source for binary apps: R.h is dumped from the same
    // arsc (aapt2_gen_rh), so the resolved id matches View::getId(). mIDS (idgen's
    // values/ID.xml) is the text-fallback for apps whose aapt2 link failed.
    if (mResTable) {
        uint32_t id = arscGetIdentifier(resid, "id", pkg);
        if (id != 0) return (int)id;
    }
    auto it = mIDS.find(pkg+":"+resid);
    if(it != mIDS.end()) return it->second;
    return -1;
}

int Assets::getNextAutofillId(){
    return mNextAutofillViewId++;
}

const std::string Assets::getString(const std::string& resid,const std::string&lan) {
    // Theme-attribute reference "?type/key" → resolve through arsc Theme.
    if (!resid.empty() && resid[0] == '?') {
        std::string r = resolveThemeRef(resid);
        if (r != resid) return getString(r, lan);
    }
    // Binary AXML hex reference "@0xPPtteeee" → resolve the resource ID to a
    // string via arsc. (getResourceString takes a resId and does getResource
    // internally; do NOT pass the resolved Res_value.data, which for strings is
    // a string-pool index, not a resId.)
    if (mResTable) {
        size_t at = resid.rfind('@');
        std::string hex = (at != std::string::npos) ? resid.substr(at + 1) : resid;
        if (hex.compare(0, 2, "0x") == 0 || hex.compare(0, 2, "0X") == 0) {
            char* end = nullptr;
            unsigned long id = strtoul(hex.c_str() + 2, &end, 16);
            if (end != hex.c_str() + 2 && id != 0 && id != 0xFFFFFFFF) {
                size_t len = 0;
                const char16_t* s = mResTable->getResourceString((uint32_t)id, &len);
                if (s && len > 0) return u16toUtf8(s, len);
            }
        }
    }
    if((!lan.empty())&&(mLanguage!=lan)) {
        applyLocale(lan);     // arsc locale (setParameters) — binary apps
        loadStrings(lan);     // text fallback (no-op for apps with no text values)
        mLanguage = lan;      // track current locale (was never assigned before)
    }
    std::string str = resid;
    std::string pkg,name = resid;
    parseResource(resid,&name,&pkg);
    std::string rawName = name; // save before normalize for arsc lookup
    name = AttributeSet::normalize(pkg,resid);
    // arsc is the single string source for binary apps (mStrings text is fallback).
    bool resolved = false;
    if (mResTable && mResTable->getError() == 0) {
        uint32_t id = arscGetIdentifier(rawName, "string", pkg);
        if (id != 0) {
            size_t len = 0;
            const char16_t* s = mResTable->getResourceString(id, &len);
            if (s && len > 0) { str = u16toUtf8(s, len); resolved = true; }
        }
    }
    if (!resolved) {
        auto itr = mStrings.find(name);
        if(itr != mStrings.end()) str = itr->second;
    }
    TextUtils::replace(str,"\\n","\n");
    return str;
}

size_t Assets::getArray(const std::string&resid,std::vector<int>&out) {
    std::string pkg,name = resid;
    std::string fullname = parseResource(resid,&name,&pkg);
    // arsc-first: read integer-array bag from resources.arsc.
    if (mResTable) {
        uint32_t id = arscGetIdentifier(name, "array", pkg);
        if (id != 0) {
            size_t count = 0; ssize_t block = -1;
            const ResTable_map* map = mResTable->getBag(id, &count, nullptr, &block);
            if (map && count) {
                for (size_t i = 0; i < count; i++) {
                    const Res_value& v = map[i].value;
                    if (v.dataType == Res_value::TYPE_INT_DEC || v.dataType == Res_value::TYPE_INT_HEX)
                        out.emplace_back((int)v.data);
                }
                return count;
            }
        }
    }
    auto it = mArraies.find(fullname);
    if(it != mArraies.end()) {
        for(auto itm:it->second)
           out.emplace_back(std::stoi(itm));
        return it->second.size();
    }
    return  0;
}

size_t Assets::getArray(const std::string&resid,std::vector<std::string>&out) {
    std::string pkg,name = resid;
    std::string fullname = parseResource(resid,&name,&pkg);
    // arsc-first: read string-array bag from resources.arsc.
    if (mResTable) {
        uint32_t id = arscGetIdentifier(name, "array", pkg);
        if (id != 0) {
            size_t count = 0; ssize_t block = -1;
            const ResTable_map* map = mResTable->getBag(id, &count, nullptr, &block);
            if (map && count) {
                for (size_t i = 0; i < count; i++) {
                    const Res_value& v = map[i].value;
                    if (v.dataType == Res_value::TYPE_STRING) {
                        size_t len = 0;
                        const char16_t* s = mResTable->stringAtBlock(block, v.data, &len);
                        if (s && len) out.emplace_back(u16toUtf8(s, len));
                    } else {
                        out.emplace_back(renderResValue(this, v));
                    }
                }
                return count;
            }
        }
    }
    auto it = mArraies.find(fullname);
    if(it != mArraies.end()) {
        for(auto itm:it->second){
            itm = AttributeSet::normalize(pkg,itm);
            out.emplace_back(itm);
        }
        return it->second.size();
    }
    ZIPArchive * pak = getResource(resid,&name,nullptr);
    if(pak)pak->forEachEntry([&out,pkg](const std::string&res){
        if(TextUtils::startWith(res,"font")){
            std::string fullres = AttributeSet::normalize(pkg,res);
            out.emplace_back(fullres);
        }
        return out.size();
    });
    return 0;
}


Drawable* Assets::getDrawable(const std::string&resid) {
    // Theme-attribute reference "?type/key" → resolve through arsc Theme.
    if (!resid.empty() && resid[0] == '?') {
        std::string r = resolveThemeRef(resid);
        if (r != resid) return getDrawable(r);
    }
    Drawable* d = nullptr;
    // Binary AXML hex reference → resolve path via arsc.
    {
        Res_value rv;
        if (arscResolveHexRef(resid, &rv) && rv.dataType == Res_value::TYPE_STRING) {
            size_t len = 0;
            const char16_t* s = mResTable->getResourceString(rv.data, &len);
            if (s && len > 0) {
                std::string path = u16toUtf8(s, len);
                if (path.substr(0, 4) == "res/") path = path.substr(4);
                return getDrawable(path);
            }
        }
    }
    std::string resname,package,ext,fullresid;
    if(resid.empty()||(resid.compare("null")==0)) {
        return nullptr;
    }
    fullresid = parseResource(resid,&resname,&package);
    // arsc: if the resource resolves to a string (file path), use that path.
    if (mResTable && resname.find("drawable/") != std::string::npos) {
        std::string rawName;
        parseResource(resid, &rawName, &package);
        uint32_t id = arscGetIdentifier(rawName, "drawable", package);
        if (id != 0) {
            Res_value v;
            if (mResTable->getResource(id, &v) >= 0 && v.dataType == Res_value::TYPE_STRING) {
                size_t len = 0;
                const char16_t* s = mResTable->getResourceString(id, &len);
                if (s && len > 0) {
                    std::string path = u16toUtf8(s, len);
                    // arsc paths have "res/" prefix; pak stores without it.
                    if (path.substr(0, 4) == "res/") path = path.substr(4);
                    if (!path.empty()) {
                        resname = path;
                        fullresid = package + ":" + path;
                    }
                }
            }
        }
    }
    ZIPArchive* pak = getResource(fullresid,&resname,nullptr);
    {
        auto it = mDrawables.find(fullresid);
        if( it != mDrawables.end() ) {
            if(it->second.expired()==false) {
                auto cs=it->second.lock();
                d= cs->newDrawable();
                LOGV("%s:%p use_count=%d",fullresid.c_str(),d,it->second.use_count());
                return d;
            }
            mDrawables.erase(it);
        }
    }
    auto extpos = resname.rfind(".");
    if(extpos!=std::string::npos)
        ext = resname.substr(extpos+1);
    //wrap png to drawable,make app develop simply
    if((resname[0]=='#')||(resname[1]=='x')||(resname[1]=='X')){
        LOGV("color %s",fullresid.c_str());
        d = new ColorDrawable(Color::parseColor(resname));
        mDrawables.insert(std::pair<std::string,std::weak_ptr<Drawable::ConstantState>>(fullresid,d->getConstantState()));
        return d;
    }
    if(resname.find("color/")!=std::string::npos){
        auto itc = mColors.find(fullresid);
        auto its = mStateColors.find(fullresid);
        if( itc != mColors.end() ){
            const uint32_t cc = (uint32_t)getColor(fullresid);
            LOGV("%s use colors as drawable",fullresid.c_str());
            d = new ColorDrawable(cc);
            mDrawables.insert(std::pair<std::string,std::weak_ptr<Drawable::ConstantState>>(fullresid,d->getConstantState()));
            return d;
        } else if(its != mStateColors.end()){
            LOGV("%s use colorstatelist as drawable",fullresid.c_str());
            d = new StateListDrawable(*its->second);
            mDrawables.insert(std::pair<std::string,std::weak_ptr<Drawable::ConstantState>>(fullresid,d->getConstantState()));
            return d;
        }
    }

    if(resname.find("attr/")!=std::string::npos) {//for reference resource
        resname = themeString(resname.substr(5), package);
        d = getDrawable(resname);
    } else if(resname.find("color/")!=std::string::npos) {
        const uint32_t cc = (uint32_t)getColor(fullresid);
        return new ColorDrawable(cc);
    } else if(ext.compare("xml")){
        if(resname.find(":")==std::string::npos){
            struct stat st;
            if(stat(resname.c_str(),&st))
                resname = package+":"+resname;
        }
        d = ImageDecoder::createAsDrawable(this,resname);
    }
    if( (d == nullptr) && (ext.compare("xml")==0) ) {
        d = DrawableInflater::loadDrawable(this,fullresid);//fromStream(this,zs,resname,package);
    }
    if(d) {
        mDrawables.insert({fullresid,std::weak_ptr<Drawable::ConstantState>(d->getConstantState())});
    }
    return d;
}

int Assets::getDimension(const std::string&refid)const{
    if (!refid.empty() && refid[0] == '?') {
        std::string r = resolveThemeRef(refid);
        if (r != refid) return getDimension(r);
    }
    std::string pkg,name = refid;
    parseResource(name,nullptr,&pkg);
    // arsc is the single dimen source for binary apps (mDimensions text is fallback).
    if (mResTable) {
        std::string rawName;
        parseResource(refid, &rawName, nullptr);
        uint32_t id = arscGetIdentifier(rawName, "dimen", pkg);
        if (id != 0) {
            Res_value v;
            if (mResTable->getResource(id, &v) >= 0) {
                if (v.dataType == Res_value::TYPE_DIMENSION) {
                    float mag = complexToFloat(v.data);
                    int unit = (v.data >> Res_value::COMPLEX_UNIT_SHIFT) & Res_value::COMPLEX_UNIT_MASK;
                    const auto& dm = getDisplayMetrics();
                    if (unit == Res_value::COMPLEX_UNIT_DIP) return (int)(dm.density * mag);
                    if (unit == Res_value::COMPLEX_UNIT_SP)  return (int)(dm.scaledDensity * mag);
                    return (int)mag;
                }
                if (v.dataType == Res_value::TYPE_INT_DEC || v.dataType == Res_value::TYPE_INT_HEX)
                    return (int)v.data;
            }
        }
    }
    name = resolveAttrValue(refid);
    auto it = mDimensions.find(name);
    if(it != mDimensions.end())
        return GET_VARIANT(it->second,int);
    LOGW("Resource not found:%s",refid.c_str());
    return 0;
}

int Assets::getDimensionPixelSize(const std::string&refid,int def)const{
    std::string pkg,name = refid;
    parseResource(name,nullptr,&pkg);
    // arsc is the single dimen source for binary apps (mDimensions text is fallback).
    if (mResTable) {
        std::string rawName;
        parseResource(refid, &rawName, nullptr);
        uint32_t id = arscGetIdentifier(rawName, "dimen", pkg);
        if (id != 0) {
            Res_value v;
            if (mResTable->getResource(id, &v) >= 0) {
                if (v.dataType == Res_value::TYPE_DIMENSION) {
                    float mag = complexToFloat(v.data);
                    int unit = (v.data >> Res_value::COMPLEX_UNIT_SHIFT) & Res_value::COMPLEX_UNIT_MASK;
                    const auto& dm = getDisplayMetrics();
                    if (unit == Res_value::COMPLEX_UNIT_DIP) return (int)(dm.density * mag + 0.5f);
                    if (unit == Res_value::COMPLEX_UNIT_SP)  return (int)(dm.scaledDensity * mag + 0.5f);
                    return (int)(mag + 0.5f);
                }
                if (v.dataType == Res_value::TYPE_INT_DEC || v.dataType == Res_value::TYPE_INT_HEX)
                    return (int)v.data;
            }
        }
    }
    name = AttributeSet::normalize(pkg,name);
    auto it = mDimensions.find(name);
    if(it != mDimensions.end()){
        return GET_VARIANT(it->second,int);
    }
    return def;
}

bool Assets::getBoolean(const std::string&refid)const{
    return getDimension(refid);
}

float Assets::getFloat(const std::string&refid,float def)const{
    std::string pkg,name = refid;
    parseResource(name,nullptr,&pkg);
    // arsc is the single dimen/float source for binary apps (mDimensions text is fallback).
    if (mResTable) {
        std::string rawName;
        parseResource(refid, &rawName, nullptr);
        uint32_t id = arscGetIdentifier(rawName, "dimen", pkg);
        if (id != 0) {
            Res_value v;
            if (mResTable->getResource(id, &v) >= 0) {
                if (v.dataType == Res_value::TYPE_FLOAT) {
                    float f; memcpy(&f, &v.data, sizeof(f));
                    return f;
                }
                if (v.dataType == Res_value::TYPE_INT_DEC || v.dataType == Res_value::TYPE_INT_HEX)
                    return (float)(int)v.data;
                if (v.dataType == Res_value::TYPE_DIMENSION)
                    return complexToFloat(v.data);
            }
        }
    }
    name = AttributeSet::normalize(pkg,name);
    auto it = mDimensions.find(name);
    if(it != mDimensions.end()){
        return GET_VARIANT(it->second,float);
    }
    return def;
}

#pragma GCC push_options
#pragma GCC optimize("O0")
//codes between pragma will crashed in ubuntu GCC V8.x,bus GCC V7 wroked well.
int Assets::getColor(const std::string&refid) {
    // Theme-attribute reference "?type/key" → resolve through arsc Theme.
    if (!refid.empty() && refid[0] == '?') {
        std::string r = resolveThemeRef(refid);
        if (r != refid) return getColor(r);
    }
    // Binary AXML hex reference: "@0x01060373" → resolve via arsc.
    {
        Res_value rv;
        if (arscResolveHexRef(refid, &rv) &&
            rv.dataType >= Res_value::TYPE_FIRST_COLOR_INT &&
            rv.dataType <= Res_value::TYPE_LAST_COLOR_INT)
            return rv.data;
    }
    std::string pkg,relname,name = refid;
    parseResource(name,&relname,&pkg);
    // arsc is the single color source for binary apps (mColors text is fallback).
    if (mResTable) {
        uint32_t id = arscGetIdentifier(relname, "color", pkg);
        if (id != 0) {
            Res_value v;
            if (mResTable->getResource(id, &v) >= 0 &&
                v.dataType >= Res_value::TYPE_FIRST_COLOR_INT &&
                v.dataType <= Res_value::TYPE_LAST_COLOR_INT) {
                return v.data;
            }
        }
    }
    name = AttributeSet::normalize(pkg,name);
    auto it = mColors.find(name);
    if(it != mColors.end()) {
        return it->second;
    }
    if(relname.compare(0,4,"attr")==0){
        relname=relname.substr(5);
        name =  themeString(relname, pkg);
        return getColor(name);
    }else if(refid.find("?")!=std::string::npos){
        std::string clrRef = name;//mTheme.getString(name.substr(6));
        TextUtils::replace(clrRef,"attr","color");
        it = mColors.find(clrRef);
        if(it != mColors.end())
            return it->second;
        name = name.substr(name.find_last_of(":?/")+1);
        clrRef = themeString(name, pkg);
        return getColor(clrRef);
    }else if((refid[0]=='#')||refid.find(':')==std::string::npos) {
        return Color::parseColor(refid);
    } else if(refid.find("color/")==std::string::npos) { //refid is defined as an color reference
        parseResource(refid,&name,nullptr);
        name = themeString(name, pkg);
        return getColor(name);
    }
    throw std::runtime_error("Resource not found:" + refid);
}

cdroid::RefPtr<ColorStateList> Assets::getColorStateList(const std::string&fullresid) {
    std::string pkg,name = fullresid,relname;
    parseResource(name,&relname,&pkg);
    name = AttributeSet::normalize(pkg,name);
    auto itc = mColors.find(name);
    auto its = mStateColors.find(name);
    if( its != mStateColors.end())
        return its->second;
    else if(itc != mColors.end()){
        auto cls = ColorStateList::valueOf(itc->second);
        mStateColors.insert(std::pair<const std::string,RefPtr<ColorStateList>>(name,cls));
        return cls;
    }
    // Fallback: resolve from resources.arsc via ResTable.
    if (mResTable) {
        uint32_t id = arscGetIdentifier(relname, "color", pkg);
        if (id != 0) {
            Res_value v;
            if (mResTable->getResource(id, &v) >= 0 &&
                v.dataType >= Res_value::TYPE_FIRST_COLOR_INT &&
                v.dataType <= Res_value::TYPE_LAST_COLOR_INT) {
                auto cls = ColorStateList::valueOf(v.data);
                mStateColors.insert(std::pair<const std::string,RefPtr<ColorStateList>>(name,cls));
                return cls;
            }
        }
    }
    if( name.size()&&(fullresid.find("attr")==std::string::npos) ) {
        const size_t slashpos = fullresid.find("/");
        try{
            cdroid::RefPtr<ColorStateList>cls;
            if(fullresid.size()&&(fullresid[0]=='#')){
                const int color = Color::parseColor(fullresid);
                cls = ColorStateList::valueOf(color);
            }else{
                // Apk id-path (AOSP loadComplexColor): resolve the resource id and
                // load through the cached Resources::loadComplexColor; fall back to
                // parsing the XML by name for resources absent from the arsc.
                Resources& r = getResources();
                const uint32_t id = arscGetIdentifier(relname, "color", pkg);
                if (id != 0) {
                    cls = std::dynamic_pointer_cast<ColorStateList>(r.loadComplexColor((int)id));
                }
                if (!cls) {
                    XmlPullParser parser(this, fullresid);
                    cls = ColorStateList::createFromXml(r, parser);
                }
            }
            mStateColors.insert(std::pair<const std::string,RefPtr<ColorStateList>>(name,cls));
            return cls;
        }catch(std::exception&e){
            std::string realName;
            parseResource(fullresid,&realName,nullptr);
            if(realName.find("?")!=std::string::npos)
            realName = themeString(realName, pkg);
            itc = mColors.find(realName);
            if(itc != mColors.end()){
                auto cls = ColorStateList::valueOf(itc->second);
                mStateColors.insert(std::pair<const std::string,RefPtr<ColorStateList>>(name,cls));
                return cls;
            }
        }
    } else if(fullresid.find("attr")!=std::string::npos) {
        const size_t slashpos = fullresid.find("/");
        std::string name = fullresid.substr(slashpos+1);
        name = themeString(name, pkg);
        if(!name.empty())return getColorStateList(name);
    }
    LOGD_IF(!fullresid.empty(),"%s not found",fullresid.c_str());
    return nullptr;
}

#pragma GCC pop_options

void Assets::clearStyles() {
    mStyles.clear();
}

std::string Assets::resolveAttrValue(const std::string&attrResId)const{
    std::string name = attrResId;
    AttributeSet atts;
    size_t pos = name.find("attr/");
    if(pos!=std::string::npos){
        do {
            std::string key;
            if((pos=name.find('?'))!=std::string::npos)
                name.erase(pos,1);
            if((pos =name.find('/'))!=std::string::npos)
                name=name.substr(pos+1);
            key = name;
            name= themeString(key, "");
            atts.add(key,name);
            if((pos=name.find('@'))!=std::string::npos)
                name.erase(pos,1);
            pos = name.find("attr");
        }while(pos!=std::string::npos);
        name = parseResource(name,nullptr,nullptr);
    }
    if((pos=name.find("@"))!=std::string::npos)
        name.erase(pos,1);
    return name;
}

AttributeSet Assets::obtainStyledAttributes(const std::string&resname) {
    AttributeSet atts;
    std::string pkg,name = resname;
    // Package prefix of the reference (e.g. "android" in "android:attr/..."). In
    // SDK/binary mode values live only in resources.arsc, so the text mTheme is
    // empty and theme-attribute lookups must go through the arsc Theme below.
    size_t colonAt = resname.find(':');
    std::string resPkg = (colonAt != std::string::npos) ? resname.substr(0, colonAt) : "";
    size_t pos = name.find("attr/");
    if(pos!=std::string::npos){
        do {
            std::string key;
            if((pos=name.find('?'))!=std::string::npos)
                name.erase(pos,1);
            if((pos =name.find('/'))!=std::string::npos)
                name=name.substr(pos+1);
            key = name;
            name= themeString(key, resPkg);
            atts.add(key,name);
            // A theme attr that resolves to a style reference: capture the style
            // resId directly (the themeString -> name -> arscGetIdentifier round-
            // trip below can fail to recover it). Lets obtainStyledAttributes
            // re-resolve the style through the arsc theme resolver.
            if (mResTable) {
                uint32_t attrId = arscGetIdentifier(key, "attr", resPkg);
                Res_value tv;
                if (attrId && arscThemeAttribute(attrId, &tv) &&
                    (tv.dataType == Res_value::TYPE_REFERENCE ||
                     tv.dataType == Res_value::TYPE_DYNAMIC_REFERENCE)) {
                    atts.setStyleResourceId((int)tv.data);
                }
            }
            if((pos=name.find('@'))!=std::string::npos)
                name.erase(pos,1);
            pos = name.find("attr");
        }while(pos!=std::string::npos);
    }else{
        if((pos=name.find('?'))!=std::string::npos)
            name.erase(pos,1);
    }
    name = parseResource(name,nullptr,&pkg);
    // arsc-first: resolve the style from resources.arsc (binary apps). Falls back
    // to the text mStyles table below when no arsc / style not found.
    if (mResTable) {
        uint32_t styleId = arscGetIdentifier(name, "style", pkg);
        if (styleId != 0) {
            size_t count = 0; ssize_t block = -1;
            const ResTable_map* map = mResTable->getBag(styleId, &count, nullptr, &block);
            if (map) {
                for (size_t i = 0; i < count; i++) {
                    std::string attrName;
                    mResTable->getResourceName(map[i].name.ident, nullptr, nullptr, &attrName);
                    if (attrName.empty() || attrName == "parent") continue;
                    const Res_value& v = map[i].value;
                    std::string valStr;
                    if (v.dataType == Res_value::TYPE_STRING) {
                        size_t len = 0;
                        const char16_t* s = mResTable->stringAtBlock(block, v.data, &len);
                        if (s && len) valStr = u16toUtf8(s, len);
                    } else {
                        valStr = renderResValue(this, v);
                    }
                    atts.add(attrName, valStr);
                    atts.setAttributeResourceId(attrName, (int)map[i].name.ident);
                }
                atts.setStyleResourceId((int)styleId);   // mark as a resolved style
                atts.setContext(this, pkg);
                uint32_t parentId = mResTable->getBagParent(styleId);
                if (parentId != 0) {
                    std::string pp, pn;
                    if (mResTable->getResourceName(parentId, &pp, nullptr, &pn) && !pn.empty()) {
                        AttributeSet parentAtts = obtainStyledAttributes(pp + ":style/" + pn);
                        atts.inherit(parentAtts);
                    }
                }
                return atts;
            }
        }
    }
    auto it = mStyles.find(name);
    if(it != mStyles.end()){
        atts = it->second;
    }
    atts.setContext(this,pkg);
    std::string parent = atts.getString("parent");
    if(parent.length()) {
        if(parent.find('/')==std::string::npos)
            parent = std::string("style/")+parent;
        if(parent.find(':')==std::string::npos)
            parent = pkg+":"+parent;
        AttributeSet parentAtts = obtainStyledAttributes(parent);
        atts.inherit(parentAtts);
    }
    return atts;
}

// AOSP Context.obtainStyledAttributes(AttributeSet, int[], defStyleAttr,
// defStyleRes) — delegates to Resources.obtainStyledAttributes (the AOSP
// Resources surface; the resolver logic lives there now). AttributeSet is
// nullable (AOSP @Nullable).
std::unique_ptr<TypedArray> Assets::obtainStyledAttributes(
    const AttributeSet* attrs, const uint32_t* styleable,
    int32_t defStyleAttr, int32_t defStyleRes)
{
    return getResources().obtainStyledAttributes(attrs, styleable, defStyleAttr, defStyleRes);
}

}//namespace

