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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include "contextimpl.h"
#include <content/androidfw/restable.h>     // ResTable + pakPathCandidates + Res_value
#include <content/typedvalue.h>             // TypedValue (tvOf / TYPE_STRING)
#include <content/asset.h>                  // Asset (openRawResource path)
#include <private/ziparchive.h>             // ZIPArchive (pak registry streams)
#include <image-decoders/imagedecoder.h>    // ImageDecoder::loadImage
#include <text/textutils.h>                 // TextUtils::utf16_utf8
#include <porting/cdlog.h>                  // LOGD/LOGV_IF
#include <unistd.h>                         // access() (local-file fallback)
#include <fstream>
#include <sstream>

namespace cdroid{

// --- pak registry / string-key resource streams (the ContextImpl role) -----

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

static TypedValue tvOf(const Res_value& rv) {
    TypedValue tv; tv.type = rv.dataType; tv.data = rv.data; return tv;
}

//"@[package:][+]id/filname"
// Default package = this context's package name (App's is the exe basename —
// App::getPackageName already strips the dir part).
const std::string ContextImpl::parseResource(const std::string&fullResId,std::string*res,std::string*ns)const {
    std::string pkg = getPackageName();
    std::string relname= fullResId;
    std::string fullid = fullResId;

    size_t pos = fullid.find_last_of("@+");
    if(pos!=std::string::npos)fullid =fullid.erase(0,pos+1);

    pos= fullid.find(":");
    if(pos != std::string::npos) {
        pkg = fullid.substr(0,pos);
        relname = fullid.substr(pos+1);
    } else { //id/xxx
        pkg = getPackageName();
        relname = fullid;
    }
    if(pkg =="android") pkg="cdroid";
    if( ns) *ns = pkg;
    if(res)*res = relname;
    return pkg+":"+relname;
}

// arsc identifier lookup. aapt2 forces a dotted package name (e.g.
// "cdroid.axmlapp") that won't match CDROID's pak name ("axmlapp"), so after
// trying the requested package and the framework ("android"), fall back to a
// name-only search across ALL loaded packages (empty package = search all).
uint32_t ContextImpl::arscGetIdentifier(const std::string& name, const std::string& type, const std::string& pkg) const {
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

ZIPArchive*ContextImpl::getResource(const std::string&fullResId,std::string*relativeResID,std::string*outPackage)const {
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

ZIPArchive* ContextImpl::findPakForPath(const std::string&package,const std::string&arscPath,
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

std::unique_ptr<std::istream> ContextImpl::getInputStream(const std::string&fullresid) {
    std::string resname,package;
    ZIPArchive*pak = getResource(fullresid,&resname,&package);
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
                    std::string path = TextUtils::utf16_utf8((const uint16_t*)s, len);
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

Cairo::RefPtr<Cairo::ImageSurface> ContextImpl::loadImage(std::istream&stream,int width,int height){
   return ImageDecoder::loadImage(stream,width,height);
}

Cairo::RefPtr<Cairo::ImageSurface> ContextImpl::loadImage(const std::string&resname,int width,int height){
    if(!resname.empty()&&resname.compare("null")){
        std::unique_ptr<std::istream> stm = getInputStream(resname);
        if(stm) return loadImage(*stm,width,height);
    }
    return nullptr;
}

// Int face: openRawResource resolves the id to the packed file (res/ strip
// included) — the string face can't open "pkg:type/key" refs from a binary pak.
Cairo::RefPtr<Cairo::ImageSurface> ContextImpl::loadImage(int id,int width,int height){
    if (id <= 0) return nullptr;
    Asset* asset = openRawResource(id);
    if (asset == nullptr) return nullptr;
    const off64_t sz = asset->getLength();
    if (sz <= 0) { delete asset; return nullptr; }
    std::string buf((size_t)sz, '\0');
    asset->read(&buf[0], (size_t)sz);
    delete asset;
    std::istringstream stm(buf);
    return loadImage(stm, width, height);
}

} // namespace cdroid
