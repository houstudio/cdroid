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
#include <core/typedvalue.h>   // TypedValue (typed currency of this layer)
#include <androidfw/restable.h> // ResTable engine + Res_value (boundary lookups)
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
#include <sstream>
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

// androidfw glue (same seam as typedarray.cc): fill a TypedValue from the raw
// Res_value handed out by ResTable lookups. AOSP does this fill in the native
// layer; core code speaks TypedValue from here on.
static TypedValue tvOf(const Res_value& rv) {
    TypedValue tv; tv.type = rv.dataType; tv.data = rv.data; return tv;
}
// mArscTheme is stored opaque in the header (void*) to keep androidfw out of
// assets.h; cast at the engine boundary.
static ResTable::Theme* asTheme(void* t) { return (ResTable::Theme*)t; }

static std::string u16toUtf8(const char16_t* s, size_t len);  // fwd (def below)

// Resolve a resource ID through the loaded arsc.
bool Assets::arscResolveId(uint32_t resId, TypedValue* out) const {
    if (!mResTable || resId == 0 || resId == 0xFFFFFFFF) return false;
    Res_value rv;
    if (mResTable->getResource(resId, &rv) < 0) return false;
    *out = tvOf(rv);
    return true;
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
bool Assets::arscThemeAttribute(uint32_t attrId, TypedValue* out, ssize_t* outBlock) const {
    if (!mArscTheme || !out) return false;
    ResTable::Theme* theme = (ResTable::Theme*)mArscTheme;
    Res_value rv;
    ssize_t blk = theme->getAttribute(attrId, &rv);
    if (blk < 0) return false;
    // Flatten ?attr / @ref chains to a concrete value.
    blk = theme->resolveAttributeReference(&rv, blk);
    if (blk < 0) return false;
    *out = tvOf(rv);
    if (outBlock) *outBlock = blk;
    return true;
}

// Try to resolve a "@0xPPtteeee" hex resource ID string through the arsc.
bool Assets::arscResolveHexRef(const std::string& s, TypedValue* out) const {
    if (!mResTable || s.empty()) return false;
    // Accept "@0x...", "0x...", or a bare hex tail after the last '@'.
    size_t at = s.rfind('@');
    std::string hex = (at != std::string::npos) ? s.substr(at + 1) : s;
    if (hex.compare(0, 2, "0x") != 0 && hex.compare(0, 2, "0X") != 0) return false;
    char* end = nullptr;
    errno = 0;
    unsigned long id = strtoul(hex.c_str() + 2, &end, 16);
    if (errno || end == hex.c_str() + 2 || id == 0 || id == 0xFFFFFFFF) return false;
    Res_value rv;
    if (mResTable->getResource((uint32_t)id, &rv) < 0) return false;
    *out = tvOf(rv);
    return true;
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
    delete asTheme(mArscTheme);
    delete mResTable;

    for(auto it=mResources.begin(); it!=mResources.end(); it++) {
        delete it->second;
    }
    for(auto& d:mDrawables) {
        LOGV_IF(d.second.use_count(),"%s reference=%d",d.first.c_str(),d.second.use_count());
    }
    mDrawables.clear();
    mResources.clear();
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

// getDrawable/getColorStateList: Context's themed defaults (context.cc) call
// getResources()/getTheme(), both of which ensureCdroidResources() — no
// override needed here anymore (AOSP-final semantics).

const DisplayMetrics& Assets::getDisplayMetrics()const{
    return mDisplayMetrics;
}

const std::string Assets::getPackageName()const {
    return mName;
}

Resources::Theme Assets::getTheme() {
    // Lazily build an arsc theme if none has been applied yet, so the returned
    // view's engine is valid (AOSP getTheme() never returns a null theme). Binary
    // mode always has mResTable; the static fallback covers text-only paks.
    if (!mArscTheme && mResTable) {
        mArscTheme = new ResTable::Theme(*mResTable);
    }
    ResTable::Theme* engine = asTheme(mArscTheme);
    if (engine == nullptr) {
        static ResTable sEmptyTable;
        static ResTable::Theme sEmptyTheme(sEmptyTable);
        engine = &sEmptyTheme;
    }
    return Resources::Theme(getResources(), engine);
}

void Assets::setTheme(int resid) {
    // AOSP Context.setTheme(@StyleRes int): rebuild the arsc-backed theme from
    // the style resource id (applyStyle follows the style's parent chain).
    delete asTheme(mArscTheme);
    mArscTheme = nullptr;
    if (mResTable && resid) {
        mArscTheme = new ResTable::Theme(*mResTable);
        if (asTheme(mArscTheme)->applyStyle((uint32_t)resid) != 0) {
            LOGW("arsc Theme applyStyle(resId=0x%08x) failed", resid);
            delete asTheme(mArscTheme);
            mArscTheme = nullptr;
        } else {
            LOGD("arsc Theme built from %s (resId=0x%08x)",
                 getResourceName((uint32_t)resid).c_str(), resid);
        }
    }
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
        // id/dimen/integer/bool/color/string/item/array values now live in the
        // resources.arsc (binary) or are resolved lazily via the id-path; the
        // retired text caches (mIDS/mColors/mDimensions/mStrings/mArraies) no
        // longer store them. Only styles and color-state-list selectors still
        // need text-XML parsing here.
        if(tag.compare("selector")==0){//for colorstatelist
            std::string key = attrs.getAttributeValue(std::string(), "name");
            depth = parser.getDepth()+1;
            std::string resUri = resid.substr(0,resid.find(".xml"));
            std::unordered_map<std::string,std::vector<AttributeSet>>::iterator it;
            it = pending->colorStateList.end();
            while(((type=parser.next())!=XmlPullParser::END_DOCUMENT) && (parser.getDepth()>=depth) ){
                if(type!=XmlPullParser::START_TAG)continue;
                AttributeSet itemAtts(attrs);
                if(it==pending->colorStateList.end()){
                    it = pending->colorStateList.insert({resUri,{itemAtts}}).first;
                }else
                    it->second.emplace_back(itemAtts);
            }
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
    LOGD("Loaded %s",name.c_str());
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
    /*pak->forEachEntry([this,package,pak,&count,&pending](const std::string&res) {
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
    });*/
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
        if (!mResTable) {
            mResTable = new ResTable();
            // Seed the requested config from the device metrics (AOSP
            // ResourcesManager applies the device configuration to every
            // Resources). densityDpi drives config-variant selection (hdpi vs
            // default buckets); with no LCD_DENSITY override it is 160 and
            // selection behaves exactly as before.
            ResTable_config cfg = {};
            cfg.density = (uint16_t)mDisplayMetrics.densityDpi;
            mResTable->setParameters(&cfg);
        }
        // NOTE: the 4-arg form is required so `true` binds to copyData, not to
        // the int32_t cookie of the 3-arg overload — otherwise copyData defaults
        // to false, hdr->data aliases the local buffer, and freeing it on return
        // leaves every Package type/key pointer dangling (UAF).
        mResTable->add(data.data(), data.size(), /*cookie*/-1, /*copyData*/true);
        LOGD("Loaded resources.arsc from %s (%zu bytes, error=%d)",
             path.c_str(), data.size(), mResTable->getError());
    }
    // The default theme is applied by App's bootstrap (single AOSP-like point
    // after every pak is loaded), not here.
    LOGI("[%s] loaded %d files, %d theme attrs, used %dms",
         package.c_str(), count, mArscTheme!=nullptr,
         int(SystemClock::uptimeMillis()-sttm));
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

ZIPArchive* Assets::findPakForPath(const std::string&package,const std::string&arscPath,
                                   std::string*outResname)const{
    std::vector<std::string> cands;
    pakPathCandidates(arscPath, cands);
    for (const auto& c : cands) {
        std::string resname;
        ZIPArchive* pak = getResource(package.empty() ? c : package + ":" + c, &resname, nullptr);
        if (!pak) continue;
        // getResource resolves the PACKAGE only — it never checks the entry
        // exists. Probe-open to verify (zip_name_locate is unreliable on some
        // paks, so use getInputStream like the real open would).
        std::istream* probe = pak->getInputStream(resname);
        if (probe) { delete probe; if (outResname) *outResname = resname; return pak; }
    }
    return nullptr;
}

std::unique_ptr<std::istream> Assets::getInputStream(const std::string&fullresid,std::string*outpkg) {
    std::string resname,package;
    ZIPArchive*pak = getResource(fullresid,&resname,&package);
    if(outpkg)*outpkg = package;
    std::istream*stream = pak ? pak->getInputStream(resname) : nullptr;
    // Fallback: a "@drawable/..." reference names a resource, not a file —
    // resolve it through the arsc to the qualified PNG path (e.g.
    // drawable-hdpi-v4/foo.9.png), like getDrawable does. Needed for 9-patch
    // src and other image loads that go through getInputStream.
    if(!stream && mResTable && fullresid.find("drawable/") != std::string::npos){
        std::string rawName;
        parseResource(fullresid, &rawName, &package);
        uint32_t id = arscGetIdentifier(rawName, "drawable", package);
        if(id != 0){
            Res_value rv;
            if(mResTable->getResource(id, &rv) >= 0){
            TypedValue v = tvOf(rv);
            if(v.type == TypedValue::TYPE_STRING){
                size_t len = 0;
                const char16_t* s = mResTable->getResourceString(id, &len);
                if(s && len > 0){
                    std::string path = u16toUtf8(s, len);
                    if(!path.empty()){
                        // arsc path ("res/drawable-hdpi-v4/x.png") -> pak entry
                        // ("drawable-hdpi/x.png"): res/ strip + "-vN" strip,
                        // shared with ResourcesImpl::openPakPath.
                        ZIPArchive* pak2 = findPakForPath(package, path, &resname);
                        if(pak2) stream = pak2->getInputStream(resname);
                    }
                }
            }
            }
        }
    }
    // Path-form input fallback: callers may pass an already-resolved arsc path
    // ("kaidu_ms7:res/drawable-hdpi-v4/x.png") — the open above missed because
    // the pak stores the source dir name. Retry the pakPathCandidates variants
    // against the resolved pak (probes the zip, unlike getResource).
    if(!stream && pak && !resname.empty()){
        std::vector<std::string> cands;
        pakPathCandidates(resname, cands);
        for(const auto& c : cands){
            if(c == resname) continue;
            std::istream* s2 = pak->getInputStream(c);
            if(s2){ stream = s2; break; }
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
    // Read-modify-write: setParameters REPLACES mParams, so start from the
    // current config (device density etc.) instead of a zeroed one — otherwise
    // a locale switch would wipe the requested density back to unset.
    ResTable_config cfg = {};
    mResTable->getParameters(&cfg);
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

// Int face: openRawResource resolves the id to the packed file (res/ strip
// included) — the string face can't open "pkg:type/key" refs from a binary pak.
Cairo::RefPtr<Cairo::ImageSurface> Assets::loadImage(int id,int width,int height){
    if (id <= 0) return nullptr;
    ensureCdroidResources();
    if (mCdroidResources == nullptr) return nullptr;
    Asset* asset = mCdroidResources->openRawResource(id);
    if (asset == nullptr) return nullptr;
    const off64_t sz = asset->getLength();
    if (sz <= 0) { delete asset; return nullptr; }
    std::string buf((size_t)sz, '\0');
    asset->read(&buf[0], (size_t)sz);
    delete asset;
    std::istringstream stm(buf);
    return loadImage(stm, width, height);
}


int Assets::getNextAutofillId(){
    return mNextAutofillViewId++;
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

