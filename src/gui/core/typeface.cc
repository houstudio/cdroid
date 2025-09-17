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
#include <set>
#include <regex>
#include <dirent.h>
#include <cdlog.h>
#include <core/typeface.h>
#include <core/textutils.h>
#include <cairomm/matrix.h>
#include <core/context.h>
#include <core/atexit.h>
#include <fontconfig/fcfreetype.h>

namespace cdroid {

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
std::unordered_map<std::string, Typeface*>*Typeface::sSystemFontMap = new std::unordered_map<std::string, Typeface*>;

void Typeface::setContext(cdroid::Context*ctx){
    mContext = ctx;
}

void Typeface::setFallback(const std::string&family){
    mFallbackFamilyName = family;
}

Typeface::Typeface(Cairo::RefPtr<Cairo::FtScaledFont>face) {
    mFontFace = face;
}

Typeface::Typeface(FcPattern & font) {
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

    Cairo::Matrix matrix = Cairo::identity_matrix();
    Cairo::Matrix ctm = Cairo::identity_matrix();
    Cairo::RefPtr<Cairo::FtFontFace> face = Cairo::FtFontFace::create(&font);
    mFontFace = Cairo::FtScaledFont::create(face,matrix,ctm);
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

Cairo::RefPtr<Cairo::FtScaledFont>Typeface::getFontFace()const {
    return mFontFace;
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
    for(auto it= sSystemFontMap->begin(); it!= sSystemFontMap->end(); it++) {
        Typeface* face = it->second;
        const int match= (face->getStyle()==style)+(face->getFamily().compare(family->getFamily())==0);
        if((face->getStyle()==style)&&(match>bestMactched)) {
            typeface = face;
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
        auto it =sSystemFontMap->find(wantFamily);
        if(it!=sSystemFontMap->end())
            return it->second;
    }
    for(auto i= sSystemFontMap->begin(); i!= sSystemFontMap->end(); i++) {
        Typeface*tf = i->second;
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
            bestFace = face;
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
    if(sSystemFontMap->empty())
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
    for(auto it= sSystemFontMap->begin(); it!= sSystemFontMap->end(); it++) {
        Typeface* face = it->second;
        const int match= base->getFamily()==it->first;
        if( (it->second->isItalic()==italic) && (match>bestMactched) ) {
            typeface = face;
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

void Typeface::buildSystemFallback(const std::string xmlPath,const std::string& fontDir,
                                   std::unordered_map<std::string, Typeface*>& fontMap,
                                   std::unordered_map<std::string, std::vector<FontFamily>>& fallbackMap) {
}

void Typeface::loadPreinstalledSystemFontMap() {
    if(sSystemFontMap->size()) return;
    loadFromFontConfig();
    //loadFromPath("");
    //loadFaceFromResource(mContext);
    
    DEFAULT      = create("", NORMAL);
    DEFAULT_BOLD = create("", BOLD);
    SANS_SERIF   = create("sans-serif",NORMAL);
    SERIF        = create("serif",NORMAL);
    MONOSPACE    = create("monospace",NORMAL);

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

    AtExit::registerCallback([](){
        printf("Typeface::sSystemFontMap.size=%d\r\n",int(sSystemFontMap->size()));
        std::set<Typeface*>faces;
        for(auto it= sSystemFontMap->begin(); it!= sSystemFontMap->end(); it++) {
            faces.insert(it->second);
            LOGD("%p %s",it->second,it->first.c_str());
        }
        for(auto it2=faces.begin();it2!=faces.end();){
            delete *it2;
            it2 = faces.erase(it2);
        }
        sSystemFontMap->clear();
        delete sSystemFontMap;
    });
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

                Typeface* typeface = new Typeface(*pat);
                const std::string family = typeface->getFamily();
                const std::string style = typeface->getStyleName();
                std::vector<std::string>families=TextUtils::split(family,";");
				loadedFont += int(families.size());
                for(std::string fm:families)
                    sSystemFontMap->insert({fm,typeface});
                LOGV("[%s] style=%s/%x %d glyphs",family.c_str(),style.c_str(),typeface->getStyle(),ftFace->num_glyphs);
            }
            FT_Done_Face(ftFace);
        }
        if(dir)closedir(dir);
        LOGV("path=%s",ps);
    }
    FcStrListDone(dirs);
    LOGD("loaded %d font sSystemFontMap.size=%d",loadedFont,sSystemFontMap->size());
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
    const int loadedFont = fs->nfont;
    if(pos != std::string::npos)
        lang = lang.substr(0,pos);
    pos = lang.find('_');
    if(pos != std::string::npos)
        lang = lang.substr(0,pos);
    mSystemLang = lang;
    for (int i=0; fs && i < fs->nfont; i++) {
        FcPattern *pat = fs->fonts[i];//FcPatternDuplicate(fs->fonts[i]);
        Typeface* tf = new Typeface(*pat);
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
        sSystemFontMap->insert({fontKey,tf});
        std::vector<std::string>families = TextUtils::split(family,";");
        for(std::string fm:families)
            sSystemFontMap->insert({fm,tf});
        LOGV("font %s %p",family.c_str(),tf);
        if(std::regex_search(family,patSans)) {
            std::string ms = std::regex_search(family,patMono)?"mono":"serif";
            ms = "sans-"+ms;
            auto it = sSystemFontMap->find(ms);
            if( it == sSystemFontMap->end()) {
                sSystemFontMap->insert({std::string(ms),tf});
                LOGV("family:[%s] is marked as [%s]",family.c_str(),ms.c_str());
            }
            if(ms.find("mono")!=std::string::npos) {
                it = sSystemFontMap->find("monospace");
                if(it == sSystemFontMap->end()) {
                    sSystemFontMap->insert({std::string("monospace"),tf});
                    LOGV("family [%s] is marked as [monospace]",family.c_str());
                }
            }
        } else if(std::regex_search(family,patMono)) {
            auto it = sSystemFontMap->find("monospace");
            if( it == sSystemFontMap->end()) {
                sSystemFontMap->insert({std::string("monospace"),tf});
                LOGV("family [%s] is marked as [monospace]",family.c_str());
            }
        }
        if(std::regex_search(family,patSerif)) {
            if(false == std::regex_search(family,patSans)&& false==std::regex_search(family,patMono)) {
                auto it = sSystemFontMap->find("serif");
                if(it == sSystemFontMap->end()) {
                    sSystemFontMap->insert({std::string("serif"),tf});
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
    LOGI("load %d font sSystemFontMap.size=%d",loadedFont,sSystemFontMap->size());
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

            Typeface* typeface = new Typeface(*pat);
            LOGD("@%s [%s] face[%d]=%p/%p=%d",fontUrl.c_str(),
                 typeface->getFamily().c_str(),face_index,face,font_face,err);
            sSystemFontMap->insert({fontUrl,typeface});
            sSystemFontMap->insert({typeface->getFamily(),typeface});
            FcPatternDestroy(pat);
        }
        FT_Done_Face(face);
#endif
    }
    LOGI("%d font loaded from resource",fonts.size());
    return fonts.size();
}

}
