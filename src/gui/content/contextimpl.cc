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
#include <content/androidfw/resourcetypes.h>  // pakPathCandidates + ResStringPool
#include <content/androidfw/assetmanager2.h>  // AM2 (arscGetIdentifier engine)
#include <content/assetmanager.h>           // the wrapper (getIdentifier seam)
#include <content/typedvalue.h>             // TypedValue (tvOf / TYPE_STRING)
#include <content/asset.h>                  // Asset / _FileAsset (buffer-backed)
#include <core/iostreams.h>                 // AssetInputStream
#include <image-decoders/imagedecoder.h>    // ImageDecoder::loadImage
#include <text/textutils.h>                 // TextUtils::utf16_utf8
#include <porting/cdlog.h>                  // LOGD/LOGV_IF
#include <zip.h>                            // libzip (pak registry handles)
#include <unistd.h>                         // access() (local-file fallback)
#include <fcntl.h>                          // ::open (file-backed Asset)
#include <sys/stat.h>                       // fstat (file-backed Asset)
#include <cstring>                          // memcpy
#include <fstream>
#include <sstream>

namespace cdroid{

// --- pak registry / string-key resource access (the ContextImpl role) ------
// The registry holds libzip handles (zip_t) directly — the former ZIPArchive
// wrapper retired; the resolution semantics below mirror it exactly.

// ZIPArchive::hasEntry semantics (zip_name_locate with the UTF-8 flag).
static bool zipHasEntry(struct zip*pak,const std::string&name) {
    return zip_name_locate(pak,name.c_str(),ZIP_FL_ENC_UTF_8) >= 0;
}

static bool guessExtension(struct zip*pak,std::string&ioname) {
    static const char* exts[]={".xml",".9.png",".png",".jpg",".gif",".apng",".webp",nullptr};
    if(ioname.find('.')!=std::string::npos)
        return true;
    for(int i=0;exts[i];i++){
        if(zipHasEntry(pak,ioname+exts[i])){
            ioname += exts[i];
            return true;
        }
    }
    return false;
}

// ZIPArchive::getInputStream semantics: entry opens go through zip_fopen —
// on paks with duplicate entries (cdroid.pak's doubled color/ set) zip's
// name-locate can fail to resolve names zip_fopen still opens.
static zip_file_t* zipOpenEntry(struct zip*pak,const std::string&name) {
    return zip_fopen(pak,name.c_str(),ZIP_RDONLY);
}

// Slurp an open zip entry into a buffer-backed, self-owning Asset. Zip entries
// pass through libzip decompression, so a full read is unavoidable — the same
// trade AssetManager::openAssetFromZip makes for deflated entries. The
// central-directory size lets us read straight into the final buffer (one
// allocation, Asset::inflateToBuffer shape).
static Asset* assetFromEntryHandle(zip_file_t*zf, zip_uint64_t size) {
    char* owned = new char[size];
    zip_uint64_t got = 0;
    while(got < size){
        zip_int64_t n = zip_fread(zf, owned + got, size - got);
        if(n <= 0) break;
        got += (zip_uint64_t)n;
    }
    zip_fclose(zf);
    if(got != size){ delete[] owned; return nullptr; }
    _FileAsset* asset = new _FileAsset();
    if(asset->openChunk(owned,(size_t)size,/*owned*/true)!=0){
        delete asset;
        return nullptr;
    }
    return asset;
}

// Fallback for paks whose central-directory name lookup cannot resolve an
// entry zip_fopen still opens (duplicate-entry paks): accumulate, then hand
// over one buffer.
static Asset* assetFromEntryHandleStreaming(zip_file_t*zf) {
    std::string data;
    char buf[65536];
    zip_int64_t n;
    while((n=zip_fread(zf,buf,sizeof(buf)))>0)
        data.append(buf,(size_t)n);
    zip_fclose(zf);
    char* owned = new char[data.size()];
    memcpy(owned,data.data(),data.size());
    _FileAsset* asset = new _FileAsset();
    if(asset->openChunk(owned,data.size(),/*owned*/true)!=0){
        delete asset;
        return nullptr;
    }
    return asset;
}

static Asset* assetFromOpenEntry(struct zip*pak,const std::string&name,zip_file_t*zf) {
    zip_stat_t st;
    zip_stat_init(&st);
    if(zip_stat(pak,name.c_str(),0,&st)==0 && (st.valid & ZIP_STAT_SIZE))
        return assetFromEntryHandle(zf,st.size);
    return assetFromEntryHandleStreaming(zf);
}

static Asset* assetFromZipEntry(struct zip*pak,const std::string&name) {
    zip_file_t*zf = zipOpenEntry(pak,name);
    return zf ? assetFromOpenEntry(pak,name,zf) : nullptr;
}

// File-backed Asset for on-disk paths (the former ifstream fallback).
static Asset* assetFromFile(const std::string&path) {
    int fd = ::open(path.c_str(),O_RDONLY
#ifdef O_BINARY
        |O_BINARY
#endif
    );
    if(fd<0) return nullptr;
    struct stat st;
    if(fstat(fd,&st)!=0 || !S_ISREG(st.st_mode)){ ::close(fd); return nullptr; }
    _FileAsset* asset = new _FileAsset();
    if(asset->openChunk(path.c_str(),fd,0,(size_t)st.st_size)!=0){
        delete asset;   // fd not yet fdopen-owned on this path
        ::close(fd);
        return nullptr;
    }
    return asset;
}

ContextImpl::~ContextImpl() {
    for(auto&kv:mResources)
        if(kv.second) zip_close(kv.second);
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
    if (arscEngine() == nullptr) return 0;   // no arsc loaded yet
    // Single seam: the AssetManager wrapper's ladder (requested package → its
    // "cdroid."-prefixed form → "android" → any registered package) — this
    // used to be a verbatim copy of it. (const_cast: Context::getAssets is a
    // non-const virtual; the lookup itself is const.)
    return const_cast<ContextImpl*>(this)->getAssets().getIdentifier(name, type, pkg);
}

struct zip*ContextImpl::getResource(const std::string&fullResId,std::string*relativeResID,std::string*outPackage)const {
    std::string package,resname;
    parseResource(fullResId,&resname,&package);
    auto it = mResources.find(package);
    struct zip* pak = nullptr;
    if(outPackage) *outPackage = package;
    if(it != mResources.end()) { //convert noextname ->extname.
        pak = it->second;
        if(pak) guessExtension(pak,resname);
        if(relativeResID) *relativeResID = resname;
    }
    LOGV_IF(pak==nullptr && resname.size(),"resource for [%s] is%s found",fullResId.c_str(),(pak?"":" not"));
    return pak;
}

struct zip* ContextImpl::findPakForPath(const std::string&package,const std::string&arscPath,
                                   std::string*outResname)const{
    std::vector<std::string> cands;
    pakPathCandidates(arscPath, cands);
    for (const auto& c : cands) {
        std::string resname;
        struct zip* pak = getResource(package.empty() ? c : package + ":" + c, &resname, nullptr);
        if (!pak) continue;
        // getResource resolves the PACKAGE only — it never checks the entry
        // exists. Probe-open to verify (zip_name_locate is unreliable on some
        // paks, so open via zip_fopen like the real open would).
        zip_file_t* probe = zipOpenEntry(pak,resname);
        if (probe) { zip_fclose(probe); if (outResname) *outResname = resname; return pak; }
    }
    return nullptr;
}

Asset* ContextImpl::openAsset(const std::string&fullresid) {
    std::string resname,package;
    struct zip*pak = getResource(fullresid,&resname,&package);
    Asset*asset = pak ? assetFromZipEntry(pak,resname) : nullptr;
    // Fallback: a "@drawable/..." reference names a resource, not a file —
    // resolve it through the arsc to the qualified PNG path (e.g.
    // drawable-hdpi-v4/foo.9.png), like getDrawable does. Needed for 9-patch
    // src and other image loads that go through openAsset.
    if(!asset && fullresid.find("drawable/") != std::string::npos){
        std::string rawName;
        parseResource(fullresid, &rawName, &package);
        uint32_t id = arscGetIdentifier(rawName, "drawable", package);
        AssetManager2* am2 = arscEngine();
        if(id != 0 && am2 != nullptr){
            auto value = am2->GetResource(id);
            if(value.has_value() && value->type == TypedValue::TYPE_STRING){
                size_t len = 0;
                const ResStringPool* pool = am2->GetStringPoolForCookie(value->cookie);
                const char16_t* s = pool ? pool->stringAt(value->data, &len) : nullptr;
                if(s && len > 0){
                    std::string path = TextUtils::utf16_utf8((const uint16_t*)s, len);
                    if(!path.empty()){
                        // arsc path ("res/drawable-hdpi-v4/x.png") -> pak entry
                        // ("drawable-hdpi/x.png"): res/ strip + "-vN" strip,
                        // shared with ResourcesImpl::openPakPath.
                        struct zip* pak2 = findPakForPath(package, path, &resname);
                        if(pak2) asset = assetFromZipEntry(pak2,resname);
                    }
                }
            }
        }
    }
    // Path-form input fallback: callers may pass an already-resolved arsc path
    // ("kaidu_ms7:res/drawable-hdpi-v4/x.png") — the open above missed because
    // the pak stores the source dir name. Retry the pakPathCandidates variants
    // against the resolved pak (probes the zip, unlike getResource).
    if(!asset && pak && !resname.empty()){
        std::vector<std::string> cands;
        pakPathCandidates(resname, cands);
        for(const auto& c : cands){
            if(c == resname) continue;
            zip_file_t* s2 = zipOpenEntry(pak,c);
            if(s2){ asset = assetFromOpenEntry(pak,c,s2); break; }
        }
    }
    if(asset) return asset;
    if( fullresid.empty() || resname.empty() || (access(fullresid.c_str(),F_OK)<0)){
        LOGD("resoure:\"%s\" not found",fullresid.c_str());
        return nullptr;
    }
    return assetFromFile(fullresid);
}

Cairo::RefPtr<Cairo::ImageSurface> ContextImpl::loadImage(std::istream&stream,int width,int height){
   return ImageDecoder::loadImage(stream,width,height);
}

Cairo::RefPtr<Cairo::ImageSurface> ContextImpl::loadImage(const std::string&resname,int width,int height){
    if(!resname.empty()&&resname.compare("null")){
        Asset*asset = openAsset(resname);
        if(asset){
            AssetInputStream stm(asset);   // owns and deletes the Asset
            return loadImage(stm,width,height);
        }
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
