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
#include <memory>
#include <set>
#include <unordered_set>
#include <regex>
#include <dirent.h>
#include <cdlog.h>
#include <core/typeface.h>
#include <text/textutils.h>
#include <cairomm/matrix.h>
#include <core/context.h>
#include <fontconfig/fcfreetype.h>
#include <hb.h>
#include <hb-ft.h>
#include <minikin/GraphemeBreak.h>
#include <minikin/LocaleList.h>
#include <minikin/Measurement.h>
#include <minikin/MeasuredText.h>
#include <minikin/SystemFonts.h>
namespace cdroid {

class FullMinikinFont : public minikin::MinikinFont {
public:
    FullMinikinFont(const Cairo::RefPtr<Cairo::FtFontFace>& fontFace)
        : mFontFace(fontFace), mHbFace(nullptr),
          mCachedScaledFont(nullptr), mCachedFontSize(0.0f) {
          mSourceId = mFontId++;
    }
    ~FullMinikinFont() override {
        if (mHbFace != nullptr) {
            hb_face_destroy(mHbFace);
        }
    }
    int32_t GetSourceId() const override{
        return mSourceId;
    }
    float GetHorizontalAdvance(uint32_t glyph_id, const minikin::MinikinPaint& paint, const minikin::FontFakery&) const override {
        Cairo::RefPtr<Cairo::FtScaledFont> scaledFont = getScaledFont(paint.size);
        std::vector<Cairo::Glyph> glyphs{{glyph_id,0,0}};
        Cairo::TextExtents extents;
        scaledFont->get_glyph_extents(glyphs, extents);
        return extents.x_advance;
    }
    void GetBounds(minikin::MinikinRect* bounds, uint32_t glyph_id, const minikin::MinikinPaint& paint, const minikin::FontFakery&) const override {
        Cairo::RefPtr<Cairo::FtScaledFont> scaledFont = getScaledFont(paint.size);

        std::vector<Cairo::Glyph> glyphs{{glyph_id,0,0}};
        Cairo::TextExtents extents;
        scaledFont->get_glyph_extents(glyphs, extents);
        bounds->mLeft = extents.x_bearing;
        bounds->mTop  = extents.y_bearing;
        bounds->mRight = extents.x_bearing + extents.width;
        bounds->mBottom= extents.y_bearing + extents.height;
    }
    void GetFontExtent(minikin::MinikinExtent* extent, const minikin::MinikinPaint& paint, const minikin::FontFakery&) const override {
        Cairo::RefPtr<Cairo::FtScaledFont> scaledFont = getScaledFont(paint.size);
        Cairo::FontExtents fontExtents;
        scaledFont->get_extents(fontExtents);
        // cairo 使用正数表示所有距离，而 minikin 期望 ascent 为负数
        extent->ascent = -fontExtents.ascent;
        extent->descent = fontExtents.descent;
    }
    const std::vector<minikin::FontVariation>& GetAxes() const override {
        static const std::vector<minikin::FontVariation> emptyAxes;
        return emptyAxes;
    }
    const void* GetFontData() const override {
        if (mHbFace == nullptr) {
            Cairo::RefPtr<Cairo::FtScaledFont> scaledFont = createScaledFont(12.0f);
            FT_Face ftFace = scaledFont->lock_face();
            if (ftFace != nullptr) {
                mHbFace = hb_ft_face_create(ftFace, 0);
            }
            scaledFont->unlock_face();
        }
        return mHbFace;
    }
    size_t GetFontSize() const override { return 0;/*sizeof filedata*/}
    int GetFontIndex() const override { return 0; }
    const std::string& GetFontPath() const override {
        static const std::string empty;
        return empty;
    }
private:
    Cairo::RefPtr<Cairo::FtScaledFont> getScaledFont(float size) const {
        if (mCachedScaledFont == nullptr || mCachedFontSize != size) {
            mCachedScaledFont = createScaledFont(size);
            mCachedFontSize = size;
        }
        return mCachedScaledFont;
    }
    Cairo::RefPtr<Cairo::FtScaledFont> createScaledFont(float size) const {
        Cairo::Matrix font_mtx(size, 0.0, 0.0, size, 0.0, 0.0);
        Cairo::FontOptions options;
        Cairo::Matrix ctm = Cairo::identity_matrix();
        options.set_hint_style(Cairo::FontOptions::HintStyle::MEDIUM);
        options.set_hint_metrics(Cairo::FontOptions::HintMetrics::OFF);
        return Cairo::FtScaledFont::create(mFontFace, font_mtx, ctm, options);
    }
    static int mFontId;
    int mSourceId;
    Cairo::RefPtr<Cairo::FtFontFace> mFontFace;
    mutable hb_face_t* mHbFace = nullptr;
    mutable Cairo::RefPtr<Cairo::FtScaledFont> mCachedScaledFont;
    mutable float mCachedFontSize;
};
int FullMinikinFont::mFontId=0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
Typeface* Typeface::MONOSPACE;
Typeface* Typeface::SANS_SERIF;
Typeface* Typeface::SERIF;
Typeface* Typeface::DEFAULT;
Typeface* Typeface::DEFAULT_BOLD;
Typeface* Typeface::sDefaultTypeface;
Typeface* Typeface::sDefaults[4];

std::string Typeface::mSystemLang;
std::string Typeface::mFallbackFamilyName;

static constexpr int SYSLANG_MATCHED = 0x80000000;

cdroid::Context* Typeface::mContext;
std::vector<std::shared_ptr<Typeface>> Typeface::sSystemFontFaces;
std::unordered_map<std::string, std::shared_ptr<Typeface> > Typeface::sSystemFontMap;
std::vector<Cairo::RefPtr<Cairo::FontFace>> Typeface::mFontFaces;

struct Typeface::Deleter{
    void operator()(Typeface* p) const noexcept {
        delete p;
    }
};

std::shared_ptr<Typeface> Typeface::make(const FcPattern& pat) {
    return std::shared_ptr<Typeface>(new Typeface(pat), Deleter{});
}

void Typeface::setContext(cdroid::Context*ctx){
    mContext = ctx;
}

void Typeface::setFallback(const std::string&family){
    mFallbackFamilyName = family;
}

/*Typeface::Typeface(Cairo::RefPtr<Cairo::FontFace>face) {
    mFontFace = face;
}*/

std::vector<Cairo::RefPtr<Cairo::FontFace>>Typeface::getFontFaces(){
    return mFontFaces;
}

Typeface::Typeface(const FcPattern & font) {
    int i,ret, weight= 0;
    double pixelSize = 0.f;
    FcChar8* s = nullptr;
    if(FcPatternGetString(&font, FC_FILE, 0, &s) == FcResultMatch){
        mFileName = std::string((const char*)s);
    }
    std::ostringstream oss;
    for(int i = 0; FcPatternGetString(&font,FC_FAMILY,i,&s) == FcResultMatch;i++){
        if(!oss.str().empty())oss<<";";
        oss<<(const char*)s;
    }
    mFamily = oss.str();
    LOGV("family=%s",mFamily.c_str());
    oss.clear();
    for(i=0; FcPatternGetString(&font, FC_STYLE, i, &s) == FcResultMatch;i++){
        if(!oss.str().empty())oss<<",";
        oss<<(const char*)s;
    }
    mStyleName = oss.str();
    mStyle = parseStyle(oss.str(),mStyleName);
    LOGV("Style=%s/%d",mStyleName.c_str(),mStyle);

    ret = FcPatternGetString(&font,FC_SLANT,0,&s);
    LOGV_IF(ret == FcResultMatch,"Slant=%s",s);

    ret = FcPatternGetInteger(&font,FC_WEIGHT,0,&weight);
    LOGV_IF(ret == FcResultMatch,"weight =%d",weight);
    mWeight = weight;

    ret = FcPatternGetDouble(&font,FC_PIXEL_SIZE,0,&pixelSize);
    LOGV_IF(ret == FcResultMatch,"pixelSize =%f",pixelSize);

    ret = FcPatternGetDouble(&font,FC_DPI,0,&pixelSize);
    LOGV_IF(ret == FcResultMatch,"dpi =%f",pixelSize);

    FcLangSet *langset = nullptr;
    ret = FcPatternGetLangSet(&font, FC_LANG,0,&langset);
    LOGV_IF(ret,"FcPatternGetLangSet=%d",ret);
    //FcChar8 *lang = FcLangSetGetString(langset, 0);
    ret = FcLangSetHasLang(langset,(const FcChar8*)mSystemLang.c_str());
    if(ret == FcResultMatch) mStyle |= SYSLANG_MATCHED;
    LOGV_IF(ret,"FcLangSetHasLang %s=%d",mSystemLang.c_str(),ret);

    Cairo::RefPtr<Cairo::FtFontFace> face = Cairo::FtFontFace::create((FcPattern*)&font);
    mFontFace = face;
}

int Typeface::parseStyle(const std::string&styleName,std::string&normalizedName) {
    static const struct {
        const char*styleName;
        const char*name;
        int styleProp;
    } stlMAP[]= {
        {"(?=.*\\bregular\\b)", "Normal",NORMAL},
        {"(?=.*\\bnormal\\b)" , "Normal",NORMAL},
        {"(?=.*\\bstandard\\b)","Normal",NORMAL},
        {"(?=.*\\bitalic\\b)", "Italic" ,ITALIC},
        {"(?=.*\\boblique\\b)","Italic" ,ITALIC},
        {"(?=.*\\bbold\\b)", "Bold",BOLD},
        {NULL,0}
    };
    int style = NORMAL;
    normalizedName = std::string();
    for(int i=0; stlMAP[i].styleName; i++) {
        const std::regex pat(stlMAP[i].styleName, std::regex_constants::icase);
        if(std::regex_search(styleName,pat)){
            style |= stlMAP[i].styleProp;
            if(!normalizedName.empty())normalizedName.append("|");
            normalizedName.append(stlMAP[i].name);
        }
    }
    return style;
}

void Typeface::fetchProps(FT_Face face) {
    mStyle = parseStyle(face->style_name,mStyleName);
    mFamily= std::string(face->family_name);
}

int Typeface::getWeight()const {
    return mWeight;
}

int Typeface::getStyle() const {
    return mStyle&STYLE_MASK;
}

bool Typeface::isBold() const {
    return (mStyle & BOLD) != 0;
}

bool Typeface::isItalic() const {
    return (mStyle & ITALIC) != 0;
}

std::string Typeface::getFamily()const {
    return mFamily;
}

std::string Typeface::getStyleName()const{
    return mStyleName;
}

Cairo::RefPtr<Cairo::FontFace>Typeface::getFontFace()const {
    return mFontFace;
}

std::shared_ptr<minikin::MinikinFont> Typeface::getMinikinFont() const {
    if (!mMinikinFont && mFontFace) {
        auto ftFace = std::dynamic_pointer_cast<Cairo::FtFontFace>(mFontFace);
        if (ftFace) {
            const_cast<Typeface*>(this)->mMinikinFont = std::make_shared<FullMinikinFont>(ftFace);
        }
    }
    return mMinikinFont;
}

std::shared_ptr<minikin::FontCollection> Typeface::getFontCollection() const {
    return mFontCollection;
}

Typeface* Typeface::create(Typeface*family, int style) {
    if ((style & ~STYLE_MASK) != 0) {
        style = NORMAL;
    }
    if (family == nullptr) {
        family = getDefault();
    }

    // Return early if we're asked for the same face/style
    if (family && (family->mStyle == style)) {
        return family;
    }

    Typeface* typeface = family;
    int bestMactched=0;
    for(auto it= sSystemFontMap.begin(); it!= sSystemFontMap.end(); it++) {
        auto face = it->second;
        const int match= (face->getStyle()==style)+(face->getFamily().compare(family->getFamily())==0);
        if((face->getStyle()==style)&&(match>bestMactched)) {
            typeface = face.get();
            bestMactched=match;
        }
    }
    LOGV("typeface=%p family=%p name=%s style=[%x/%x]%s fontfile=%s",typeface,family,
       typeface->mFamily.c_str(),style,typeface->mStyle,typeface->mStyleName.c_str(),typeface->mFileName.c_str());
    return typeface;
}

Typeface* Typeface::getSystemDefaultTypeface(const std::string& familyName) {
    Typeface*face = Typeface::DEFAULT;
    Typeface*bestFace = Typeface::DEFAULT;
    std::string wantFamily = familyName;
    int bestMatched = 0;
    if(!wantFamily.empty()) {
        if(wantFamily[0]=='@')
            wantFamily = wantFamily.substr(1);
        if(wantFamily.find(":")==std::string::npos)
            wantFamily = mContext->getPackageName()+":"+wantFamily;
    }
    if(wantFamily.find(":")!=std::string::npos){
        //only for family @font/font-name
        auto it =sSystemFontMap.find(wantFamily);
        if(it!=sSystemFontMap.end())
            return it->second.get();
    }
    for(auto i= sSystemFontMap.begin(); i!= sSystemFontMap.end(); i++) {
        auto tf = i->second;
        int familyMatched = 0;
        const std::string fontKey= i->first;
        const std::string family = tf->getFamily();
        std::vector<std::string>families = TextUtils::split(family,";");
        auto it = std::find(families.begin(),families.end(),familyName);
        if( (it != families.end()) || (fontKey.compare(wantFamily) == 0) ) {
            familyMatched++;
        }
        if(tf->mStyle&SYSLANG_MATCHED) {
            familyMatched++;
        }
        if(familyMatched>bestMatched){
            bestMatched = familyMatched;
            bestFace = tf.get();  // 修复：应该设置为当前遍历到的字体
        }
    }
    face = bestFace;
    LOGV_IF(face,"want %s got %s/%s style=[%x]%s",familyName.c_str(),
         face->getFamily().c_str(),wantFamily.c_str(),face->mStyle,face->mStyleName.c_str());
    return bestFace;
}

Typeface* Typeface::create(cdroid::Typeface*family, int weight, bool italic) {
    if (family == nullptr) {
        loadPreinstalledSystemFontMap();
        family = getDefault();
    }
    return createWeightStyle(family, weight, italic);
}

Typeface* Typeface::create(const std::string& familyName,int style) {
    if(sSystemFontMap.empty())
        loadPreinstalledSystemFontMap();
    Typeface*tf = getSystemDefaultTypeface(familyName);
    if(familyName.find("/")!=std::string::npos)return tf;
    return create(tf, style);
}

Typeface* Typeface::defaultFromStyle(int style) {
    return sDefaults[(style&0x03)];
}

Typeface* Typeface::createWeightStyle(Typeface* base,int weight, bool italic) {
    //const int key = (weight << 1) | (italic ? 1 : 0);

    Typeface* typeface = base;
    int bestMactched = 0;
    for(auto it= sSystemFontMap.begin(); it!= sSystemFontMap.end(); it++) {
        auto face = it->second;
        const int match= base->getFamily()==it->first;
        if( (it->second->isItalic()==italic) && (match>bestMactched) ) {
            typeface = face.get();
            bestMactched=match;
        }
    }
    return typeface;
}

void Typeface::setDefault(Typeface* t) {
    sDefaultTypeface = t;
}

Typeface* Typeface::getDefault() {
    return sDefaultTypeface;
}

static std::string normalizeFamilyPart(const std::string& name) {
    static const std::regex stylePattern(
        R"(\b(?:Bold|Italic|Regular|Normal|Light|Thin|ExtraLight|UltraLight|
            Medium|SemiBold|DemiBold|ExtraBold|UltraBold|Black|Heavy|Oblique|
            Condensed|SemiCondensed|ExtraCondensed|Narrow|Wide|Extended|
            SemiExtended|ExtraExtended|SC|TC|HK|JP|KR|\d+)\b)",
        std::regex_constants::icase | std::regex_constants::optimize
    );
    
    std::string result = name;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    
    result = std::regex_replace(result, stylePattern, "");
    
    result.erase(std::remove_if(result.begin(), result.end(), 
        [](char c) { return !std::isalnum(c) && c != ' '; }), result.end());
    
    result.erase(std::unique(result.begin(), result.end(), 
        [](char a, char b) { return a == ' ' && b == ' '; }), result.end());
    
    size_t first = result.find_first_not_of(' ');
    size_t last = result.find_last_not_of(' ');
    return (first == std::string::npos) ? "" : result.substr(first, last - first + 1);
}

static bool isSameFamily(const std::string& fm1, const std::string& fm2) {
    std::vector<std::string> parts1 = TextUtils::split(fm1, ";");
    std::vector<std::string> parts2 = TextUtils::split(fm2, ";");
    
    std::unordered_set<std::string> normalizedParts1;
    for (const auto& part : parts1) {
        std::string normalized = normalizeFamilyPart(part);
        if (!normalized.empty()) {
            normalizedParts1.insert(normalized);
        }
    }
    
    for (const auto& part : parts2) {
        std::string normalized = normalizeFamilyPart(part);
        if (!normalized.empty() && normalizedParts1.count(normalized)) {
            LOGV("isSameFamily: '%s' == '%s' (normalized: '%s')", 
                 fm1.c_str(), fm2.c_str(), normalized.c_str());
            return true;
        }
    }
    
    return false;
}

std::shared_ptr<minikin::FontFamily>Typeface::buildFamily(const std::string&family,const std::vector<std::shared_ptr<Typeface>>&faces){
    std::vector<std::shared_ptr<minikin::Font>> fonts;
    for(auto f:faces){
        if(isSameFamily(family,f->getFamily())){
            auto ft = std::dynamic_pointer_cast<Cairo::FtFontFace>(f->getFontFace());
            auto minikinFont = std::make_shared<FullMinikinFont>(ft);
            auto font = minikin::Font::Builder(minikinFont).build();
            fonts.push_back(font);
            LOGD("    ->[%d](%s): %s",fonts.size()-1,f->getFamily().c_str(),f->mFileName.c_str());
        }
    }
    return minikin::FontFamily::create(std::move(fonts));
}
void Typeface::buildSystemFallback() {
    std::map<std::string, std::shared_ptr<minikin::FontCollection>> familyCollections;
    std::map<std::string, std::shared_ptr<minikin::FontFamily>> familyCache;  // 缓存 FontFamily
    
    // 双重循环：为每个唯一家族创建 FontCollection，同一家族共享
    for (auto tf : sSystemFontFaces) {
        // 查找是否已创建同一家族的 FontCollection
        bool found = false;
        for (auto& entry : familyCollections) {
            if (isSameFamily(tf->mFamily, entry.first)) {
                tf->mFontCollection = entry.second;
                found = true;
                break;
            }
        }
        
        // 未找到则创建新的 FontCollection
        if (!found) {
            LOGD("Build FontCollection for family: %s", tf->mFamily.c_str());
            std::vector<std::shared_ptr<minikin::FontFamily>> families;
            
            // 添加所有家族的 FontFamily（当前家族优先）
            for (auto tf2 : sSystemFontFaces) {
                // 检查缓存：遍历已缓存的家族，看是否与当前家族相同
                bool cached = false;
                std::shared_ptr<minikin::FontFamily> cachedFamily;
                for (auto& cacheEntry : familyCache) {
                    if (isSameFamily(cacheEntry.first, tf2->mFamily)) {
                        cachedFamily = cacheEntry.second;
                        cached = true;
                        break;
                    }
                }
                
                if (!cached) {
                    cachedFamily = buildFamily(tf2->mFamily, sSystemFontFaces);
                    familyCache[tf2->mFamily] = cachedFamily;
                }
                
                if (cachedFamily && cachedFamily->getNumFonts() > 0) {
                    if (isSameFamily(tf->mFamily, tf2->mFamily)) {
                        families.insert(families.begin(), cachedFamily);  // 主家族放前面
                    } else {
                        families.push_back(cachedFamily);
                    }
                }
            }
            
            auto collection = minikin::FontCollection::create(std::move(families));
            familyCollections[tf->mFamily] = collection;
            tf->mFontCollection = collection;
            LOGD("[%s] -> shared FontCollection (family: %s)", 
                 tf->mFamily.c_str(), tf->mFamily.c_str());
        }
    }
}

void Typeface::loadPreinstalledSystemFontMap() {
    if(sSystemFontMap.size()) return;
    loadFromFontConfig();
    //loadFromPath("");
    //loadFaceFromResource(mContext);
    buildSystemFallback();

    // 从 sSystemFontFaces 中选择默认字体（已经有 FontCollection）
    if (!sSystemFontFaces.empty()) {
        DEFAULT = sSystemFontFaces[0].get();
        sDefaultTypeface = DEFAULT;
        // 查找 Bold 版本
        for (auto& tf : sSystemFontFaces) {
            if (tf->isBold() && !tf->isItalic()) {
                DEFAULT_BOLD = tf.get();
                break;
            }
        }
        // 如果没找到 Bold，使用 DEFAULT
        if (!DEFAULT_BOLD) {
            DEFAULT_BOLD = DEFAULT;
        }
    } else {
        DEFAULT = create("", NORMAL);
        DEFAULT_BOLD = create("", BOLD);
    }
    
    SANS_SERIF   = create("sans-serif",NORMAL);
    SERIF        = create("serif",NORMAL);
    MONOSPACE    = create("monospace",NORMAL);

    // 验证 DEFAULT 是否有 FontCollection
    LOGD("DEFAULT=%p [%s] FontCollection=%p", 
         DEFAULT, DEFAULT->mFamily.c_str(), DEFAULT->mFontCollection.get());
    LOGD("DEFAULT_BOLD=%p [%s] FontCollection=%p", 
         DEFAULT_BOLD, DEFAULT_BOLD->mFamily.c_str(), DEFAULT_BOLD->mFontCollection.get());

    sDefaults[0]=DEFAULT;
    sDefaults[1]=DEFAULT_BOLD;
    sDefaults[2]=create(nullptr,ITALIC);
    sDefaults[3]=create(nullptr,BOLD_ITALIC);

    LOGD("sDefaults=%p,%p,%p,%p",sDefaults[0],sDefaults[1],sDefaults[2],sDefaults[3]);
    LOGD("DEFAULT=%p [%s] style:%d %s",DEFAULT,DEFAULT->mFamily.c_str(),DEFAULT->getStyle(),DEFAULT->mFileName.c_str());
    LOGD("DEFAULT_BOLD=%p [%s] style:%d %s",DEFAULT_BOLD,DEFAULT_BOLD->mFamily.c_str(),DEFAULT->getStyle(),DEFAULT_BOLD->mFileName.c_str());
    LOGD("SANS_SERIF=%p [%s] style:%d %s",SANS_SERIF,SANS_SERIF->mFamily.c_str(),DEFAULT->getStyle(),SANS_SERIF->mFileName.c_str());
    LOGD("SERIF=%p [%s]style:%d %s",SERIF,SERIF->mFamily.c_str(),DEFAULT->getStyle(),SERIF->mFileName.c_str());
    LOGD("MONOSPACE=%p [%s]style:%d %s",MONOSPACE,MONOSPACE->mFamily.c_str(),DEFAULT->getStyle(),MONOSPACE->mFileName.c_str());
}

static FT_Library ftLibrary = nullptr;
int Typeface::loadFromPath(const std::string&path) {
    if(ftLibrary==nullptr)
        FT_Init_FreeType(&ftLibrary);
    FcConfig *config= FcInitLoadConfigAndFonts ();
    FcStrList *dirs = FcConfigGetFontDirs (config);
	int loadedFont = 0;
    FcStrListFirst(dirs);
    while(FcChar8*ps = FcStrListNext(dirs)) {
        struct dirent*ent;
        DIR*dir = opendir((const char*)ps);
        while(dir && ( ent = readdir(dir) ) ) {
            FT_Face ftFace = nullptr,font_face = nullptr;
            std::string fullpath = std::string((char*)ps) + "/" + ent->d_name;
            FT_Error err = FT_New_Face(ftLibrary,fullpath.c_str(),0,&ftFace);
            if(ftFace == nullptr || err )continue;
            FcPattern*pat = FcFreeTypeQueryFace(ftFace,nullptr,0,nullptr);
            if(pat){
                err = FcPatternGetFTFace (pat, FC_FT_FACE, 0, &font_face);
                LOGE_IF(!ftFace->family_name,"%s missing familyname",fullpath.c_str());

                std::shared_ptr<Typeface> typeface = Typeface::make(*pat);
                const std::string family = typeface->getFamily();
                const std::string style = typeface->getStyleName();
                std::vector<std::string>families=TextUtils::split(family,";");
				loadedFont += int(families.size());
                for(std::string fm:families)
                    sSystemFontMap.insert({fm,typeface});
                LOGV("[%s] style=%s/%x %d glyphs",family.c_str(),style.c_str(),typeface->getStyle(),ftFace->num_glyphs);
            }
            FT_Done_Face(ftFace);
        }
        if(dir)closedir(dir);
        LOGV("path=%s",ps);
    }
    FcStrListDone(dirs);
    LOGD("loaded %d font sSystemFontMap.size=%d",loadedFont,sSystemFontMap.size());
    return loadedFont;
}

#ifdef _WIN32
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

int Typeface::loadFromFontConfig() {
    FcConfig *config = FcInitLoadConfigAndFonts ();
    if(!config) return 0;

    FcPattern  *pat= FcPatternCreate();
    FcObjectSet*os = FcObjectSetBuild (FC_FAMILY, FC_STYLE, FC_LANG, FC_FILE,FC_WEIGHT,NULL);
    FcFontSet  *fs = FcFontList(config, pat, os);
    //FcPatternDestroy(pat);
    const std::regex patSerif("(?=.*\\bserif\\b)", std::regex_constants::icase);
    const std::regex patSans( "(?=.*\\bsans\\b)", std::regex_constants::icase);
    const std::regex patMono( "(?=.*\\bmono\\b)", std::regex_constants::icase);
    const char*langenv=getenv("LANG");
    std::string lang = "en_US.UTF-8";
    if(langenv)lang = langenv;
    size_t pos = lang.find('.');
    const int loadedFont = fs?fs->nfont:0;
    if(pos != std::string::npos)
        lang = lang.substr(0,pos);
    pos = lang.find('_');
    if(pos != std::string::npos)
        lang = lang.substr(0,pos);
    mSystemLang = lang;
    for (int i=0; i < loadedFont; i++) {
        FcPattern *pat = fs->fonts[i];//FcPatternDuplicate(fs->fonts[i]);
        auto tf = Typeface::make(*pat);
        const std::string family = tf->getFamily();
        const std::string style = tf->getStyleName();
        std::string font = tf->mFileName;
        size_t pos = font.find_last_of(".");
        if(pos!=std::string::npos)
            font = font.substr(0,pos);
        pos = font.find_last_of(PATH_SEPARATOR);
        if(pos != std::string::npos)
            font = font.substr(pos+1);
        std::string fontKey = mContext->getPackageName()+":font/"+font;
        LOGI("%d [%s] <%s> @%s=%s",i,family.c_str(),style.c_str(),fontKey.c_str(),tf->mFileName.c_str());
        sSystemFontFaces.push_back(tf);
        sSystemFontMap.insert({fontKey,tf});
        std::vector<std::string>families = TextUtils::split(family,";");
        for(std::string fm:families) sSystemFontMap.insert({fm,tf});
        mFontFaces.push_back(tf->getFontFace());
        LOGV("font %s %p",family.c_str(),tf.get());
        if(std::regex_search(family,patSans)) {
            std::string ms = std::regex_search(family,patMono)?"mono":"serif";
            ms = "sans-"+ms;
            auto it = sSystemFontMap.find(ms);
            if( it == sSystemFontMap.end()) {
                sSystemFontMap.insert({std::string(ms),tf});
                LOGV("family:[%s] is marked as [%s]",family.c_str(),ms.c_str());
            }
            if(ms.find("mono")!=std::string::npos) {
                it = sSystemFontMap.find("monospace");
                if(it == sSystemFontMap.end()) {
                    sSystemFontMap.insert({std::string("monospace"),tf});
                    LOGV("family [%s] is marked as [monospace]",family.c_str());
                }
            }
        } else if(std::regex_search(family,patMono)) {
            auto it = sSystemFontMap.find("monospace");
            if( it == sSystemFontMap.end()) {
                sSystemFontMap.insert({std::string("monospace"),tf});
                LOGV("family [%s] is marked as [monospace]",family.c_str());
            }
        }
        if(std::regex_search(family,patSerif)) {
            if(false == std::regex_search(family,patSans)&& false==std::regex_search(family,patMono)) {
                auto it = sSystemFontMap.find("serif");
                if(it == sSystemFontMap.end()) {
                    sSystemFontMap.insert({std::string("serif"),tf});
                    LOGV("family [%s] is marked as [serif]",family.c_str());
                }
            }
        }
    }
    FcConfigSubstitute (0, pat, FcMatchPattern);
    FcDefaultSubstitute (pat);
    FcResult result;
    FcChar8*s = NULL;
    FcFontSet* fsdef = FcFontSetCreate ();
    FcPattern*match = FcFontMatch (0, pat, &result);
    FcPatternGetString(match, FC_FILE, 0, &s);
    LOGD("match=%p:%d nfonts=%d %s",match,result,fsdef->nfont,s);
    if(match){
        setDefault(new Typeface(*match));
        FcPatternDestroy(match);
    }
    FcPatternDestroy(pat);
    FcFontSetDestroy(fsdef);

    if(fs)FcFontSetDestroy(fs);
    FcObjectSetDestroy(os);
    FcConfigDestroy(config);
    FcFini();
    LOGI("load %d font sSystemFontMap.size=%d",loadedFont,sSystemFontMap.size());
    return loadedFont;
}

static unsigned long my_stream_io(FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count) {
    std::istream*istream = (std::istream*)stream->descriptor.pointer;
    istream->seekg(offset,std::ios::beg);
    istream->read((char*)buffer,count);
    const size_t len = istream->gcount();
    return len;
}

int Typeface::loadFaceFromResource(cdroid::Context*context) {
    std::vector<std::string> fonts;
    context->getArray("@fonts",fonts);
    context->getArray("@font",fonts);
    if(ftLibrary == nullptr)
        FT_Init_FreeType(&ftLibrary);
    for(auto fontUrl:fonts) {
        FT_Face face = nullptr;
        FT_Open_Args args;
        FT_StreamRec stream;
        std::unique_ptr<std::istream> istream =context->getInputStream(fontUrl);
        memset(&stream,0,sizeof(FT_StreamRec));
        stream.read  = my_stream_io;
        stream.close = nullptr;
        stream.descriptor.pointer = istream.get();
        /*In case of compressed streams where the size is unknown before
        *actually doing the decompression, the value is set to 0x7FFFFFFF.
        *(Note that this size value can occur for normal streams also; it is
        *thus just a hint.)*/
        istream->seekg(0,std::ios::end);
        stream.size = istream->tellg();
        istream->seekg(0,std::ios::beg);
        args.flags  = FT_OPEN_STREAM;
        args.stream = &stream;

        int err = FT_Open_Face(ftLibrary, &args, 0, &face);

        size_t dotPos = fontUrl.find_last_of(".");
        if(dotPos != std::string::npos)
            fontUrl = fontUrl.substr(0, dotPos);
        dotPos = fontUrl.find("fonts");
        if(dotPos != std::string::npos)
            fontUrl.replace(dotPos,5,"font");    
#ifdef CAIRO_HAS_FT_FONT
        FT_Face font_face;
        Cairo::RefPtr<Cairo::FtFontFace> ftface = Cairo::FtFontFace::create(face,0);//FT_LOAD_NO_SCALE:1 FT_LOAD_DEFAULT:0;
        LOGD_IF(face->num_faces>1,"Face.num_faces=%d",face->num_faces);
        for(int face_index=0;face_index<face->num_faces;face_index++){
            FcPattern* pat = FcFreeTypeQueryFace(face,nullptr,face_index,nullptr);
            FcPatternAddFTFace(pat, FC_FT_FACE,face);
            FcPatternAddString(pat, FC_FILE,(const FcChar8*)fontUrl.c_str());
            err = FcPatternGetFTFace (pat, FC_FT_FACE,0, &font_face);

            auto typeface = Typeface::make(*pat);
            LOGD("@%s [%s] face[%d]=%p/%p=%d",fontUrl.c_str(),
                 typeface->getFamily().c_str(),face_index,face,font_face,err);
            sSystemFontMap.insert({fontUrl,typeface});
            sSystemFontMap.insert({typeface->getFamily(),typeface});
            FcPatternDestroy(pat);
        }
        FT_Done_Face(face);
#endif
    }
    LOGI("%d font loaded from resource",fonts.size());
    return fonts.size();
}

}
