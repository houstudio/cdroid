#include <core/typeface.h>
#include <regex>
#include <freetype/freetype.h>
#include <cairomm/matrix.h>
#include <dirent.h>
namespace cdroid{

Typeface* Typeface::MONOSPACE;
Typeface* Typeface::SANS_SERIF;
Typeface* Typeface::SERIF;
Typeface* Typeface::DEFAULT;
Typeface* Typeface::DEFAULT_BOLD;
Typeface* Typeface::sDefaultTypeface;

std::unordered_map<std::string, Typeface*>Typeface::sSystemFontMap;

Typeface::Typeface(Cairo::RefPtr<Cairo::FtScaledFont>face){
    mFontFace = face;
}

Typeface::Typeface(FcPattern & font){
    int weight=0;
    double pixelSize=.0f;
    FcChar8* s = FcNameUnparse(&font);
    LOGV("Font= %s",s);
    //free(s); 
    s = nullptr; 

    if(FcPatternGetString(&font, FC_FAMILY, 0, &s)==FcResultMatch){
        LOGD("Family=%s",s);
        mFamily = (const char*)s;
        //free(s); 
	s= nullptr;
    }

    if(FcPatternGetString(&font, FC_STYLE, 0, &s)==FcResultMatch){
        LOGD("Style=%s",s);
        //free(s); 
	s = nullptr;
    }

    if(FcPatternGetString(&font,FC_SLANT,0,&s)==FcResultMatch){
        LOGD("Slant=%s",s);
        //free(s); 
	s = nullptr;
	mStyle|=ITALIC;
    }
    if(FcPatternGetInteger(&font,FC_WEIGHT,0,&weight)==FcResultMatch){
	LOGD("weight =%d",weight);
	mWeight = weight;
    }
    if(FcPatternGetDouble(&font,FC_PIXEL_SIZE,0,&pixelSize)==FcResultMatch){
	 LOGD("pixelSize =%f",pixelSize);
    }
    if(FcPatternGetDouble(&font,FC_DPI,0,&pixelSize)==FcResultMatch){
	 LOGD("dpi =%f",pixelSize);
    }
    Cairo::Matrix matrix = Cairo::identity_matrix();
    Cairo::Matrix ctm = Cairo::identity_matrix();
    Cairo::RefPtr<Cairo::FtFontFace> face = Cairo::FtFontFace::create(&font);
    mFontFace = Cairo::FtScaledFont::create(face,matrix,ctm);
}

Typeface* Typeface::create(Typeface*family, int style){
    if ((style & ~STYLE_MASK) != 0) {
        style = NORMAL;
    }
    if (family == nullptr) {
        family = getDefault();
        if(family == nullptr)
            family = sSystemFontMap.begin()->second;
    }

    // Return early if we're asked for the same face/style
    if (family && family->mStyle == style) {
       return family;
    }

    Typeface* typeface = nullptr;
    /*long ni = family->native_instance;
    SparseArray<Typeface> styles = sStyledTypefaceCache.get(ni);

    if (styles == null) {
        styles = new SparseArray<Typeface>(4);
        sStyledTypefaceCache.put(ni, styles);
    } else {
        typeface = styles.get(style);
        if (typeface != null) {
            return typeface;
        }
    }

    typeface = new Typeface(nativeCreateFromTypeface(ni, style));
    styles.put(style, typeface);
    }*/
    LOGV("typeface=%p family=%p",typeface,family);
    return typeface?typeface:family;
}

Typeface* Typeface::getSystemDefaultTypeface(const std::string& familyName){
    auto it=sSystemFontMap.find(familyName);
    return (it==sSystemFontMap.end())? Typeface::DEFAULT : it->second;
}

Typeface* Typeface::create(cdroid::Typeface*family, int weight, bool italic){
    if (family == nullptr) {
	loadPreinstalledSystemFontMap();
        family = getDefault();
    }
    return createWeightStyle(family, weight, italic);
}

Typeface* Typeface::create(const std::string& familyName,int style){
    if(sSystemFontMap.empty())
        loadPreinstalledSystemFontMap();
    return create(getSystemDefaultTypeface(familyName), style);
}

Typeface* Typeface::defaultFromStyle(int style){
    return getDefault();
}

Typeface* Typeface::createWeightStyle(Typeface* base,int weight, bool italic){
    int key = (weight << 1) | (italic ? 1 : 0);

    Typeface* typeface = nullptr;
    /*synchronized(sWeightCacheLock) {
    SparseArray<Typeface> innerCache = sWeightTypefaceCache.get(base.native_instance);
    if (innerCache == null) {
        innerCache = new SparseArray<>(4);
        sWeightTypefaceCache.put(base.native_instance, innerCache);
    } else {
        typeface = innerCache.get(key);
        if (typeface != null) {
            return typeface;
        }
    }

    typeface = new Typeface(
            nativeCreateFromTypefaceWithExactStyle(base.native_instance, weight, italic));
    innerCache.put(key, typeface);
    }*/
    return typeface;
}

void Typeface::setDefault(Typeface* t) {
    sDefaultTypeface = t;
}

Typeface* Typeface::getDefault(){
    return sDefaultTypeface;
}

void Typeface::buildSystemFallback(const std::string xmlPath,const std::string& fontDir,
        std::unordered_map<std::string, Typeface*>& fontMap,
	std::unordered_map<std::string, std::vector<FontFamily>>& fallbackMap){
}

void Typeface::loadPreinstalledSystemFontMap(){
    if(sSystemFontMap.size())
	return;
    loadFromFontConfig();
    auto it=sSystemFontMap.find(DEFAULT_FAMILY);
    if (it!=sSystemFontMap.end()) {
        setDefault(it->second);
    }

    DEFAULT      = create("", 0);
    DEFAULT_BOLD = create("", Typeface::BOLD);
    SANS_SERIF   = create("sans-serif", 0);
    SERIF        = create("serif", 0);
    MONOSPACE    = create("monospace", 0);
    LOGD("DEFAULT=%p [%s]",DEFAULT,DEFAULT?DEFAULT->mFamily.c_str():"");
    LOGD("DEFAULT_BOLD=%p [%s]",DEFAULT_BOLD,DEFAULT_BOLD?DEFAULT_BOLD->mFamily.c_str():"");
    LOGD("SANS_SERIF=%p [%s]",SANS_SERIF,SANS_SERIF?SANS_SERIF->mFamily.c_str():"");
    LOGD("SERIF=%p [%s]",SERIF,SERIF?SERIF->mFamily.c_str():"");
    LOGD("MONOSPACE=%p [%s]",MONOSPACE,MONOSPACE?MONOSPACE->mFamily.c_str():"");
}

static FT_Library ftLibrary = nullptr;
int Typeface::loadFromPath(const std::string&path){
    FT_Init_FreeType(&ftLibrary);
    FcConfig *config= FcInitLoadConfigAndFonts ();
    FcStrList *dirs = FcConfigGetFontDirs (config);
    FcStrListFirst(dirs);
    FcChar8* ps;
    while(ps=FcStrListNext(dirs)){
        struct dirent*ent;
        DIR*dir=opendir((const char*)ps);
        while(dir&&(ent=readdir(dir))){
            FT_Face ftFace = nullptr;
            std::string fullpath=std::string((char*)ps)+"/"+ent->d_name;
            FT_Error err = FT_New_Face(ftLibrary,fullpath.c_str(),0,&ftFace);
	    if(ftFace==nullptr||err)continue;
            LOGE_IF(ftFace->family_name==nullptr,"%s missing familyname",fullpath.c_str());
            if(ftFace->family_name){
		const double scale = (double)ftFace->max_advance_height/ftFace->units_per_EM;//ftFace->height;//>max_advance_width
		Cairo::RefPtr<Cairo::FtFontFace> face = Cairo::FtFontFace::create(ftFace,FT_LOAD_DEFAULT);
		Cairo::Matrix matrix = Cairo::scaling_matrix(scale,scale);
                Cairo::Matrix ctm = Cairo::identity_matrix();
                auto autoft= Cairo::FtScaledFont::create(face,matrix,ctm);
                Typeface *typeface = new Typeface(autoft);
		typeface->mScale = scale;
                sSystemFontMap.insert({std::string(ftFace->family_name),typeface});
                LOGD("family=%s style=%s %d glyphs face.height=%x units_per_EM=%x scale=%f",ftFace->family_name,
			ftFace->style_name,ftFace->num_glyphs, ftFace->max_advance_height,ftFace->units_per_EM,scale);
            }else{
                FT_Done_Face(ftFace);
            }
	}
        if(dir)closedir(dir);
	LOGD("path=%s",ps);
    }
    FcStrListDone(dirs);
    return sSystemFontMap.size();
}

int Typeface::loadFromFontConfig(){
    FcConfig *config = FcInitLoadConfigAndFonts ();
    if(!config) return false;
 
    FcPattern  *p  = FcPatternCreate();
    FcObjectSet*os = FcObjectSetBuild (FC_FAMILY, FC_STYLE, FC_LANG, FC_FILE,NULL);
    FcFontSet  *fs = FcFontList(config, p, os);
    FcPatternDestroy(p);
    return loadFromPath("");
    LOGI("Total fonts: %d", fs->nfont);
    const std::regex patSerif("(?=.*\\bserif\\b)" , std::regex_constants::icase);
    const std::regex patSans( "(?=.*\\bsans\\b)" , std::regex_constants::icase);
    const std::regex patMono( "(?=.*\\bmono\\b)" , std::regex_constants::icase);
    for (int i=0; fs && i < fs->nfont; i++) {
	FcPattern *pat = fs->fonts[i];//FcPatternDuplicate(fs->fonts[i]);
	Typeface   *tf = new Typeface(*pat);
	const std::string family = tf->getFamily();
	sSystemFontMap.insert({family,tf});
	//FT_Face ftFace ftFace = tf->mFontFace->;
	//const double scale = (double)ftFace->height/ftFace->units_per_EM;//
	tf->mScale = 1.f;//scale;
        if(std::regex_search(family,patSans)){
            std::string ms=std::regex_search(family,patMono)?"mono":"serif";
            ms ="sans-"+ms;
            auto it = sSystemFontMap.find(ms);
            if( it == sSystemFontMap.end()){
		sSystemFontMap.insert({std::string(ms),tf});
                LOGD("family:[%s] is marked as [%s]",family.c_str(),ms.c_str());
	    }
	    if(ms.find("mono")!=std::string::npos){
		it = sSystemFontMap.find("monospace");
		if(it == sSystemFontMap.end()){
	            sSystemFontMap.insert({std::string("monospace"),tf});
		    LOGD("family [%s] is marked as [monospace]",family.c_str());
		}
	    }
        }else if(std::regex_search(family,patMono)){
            auto it = sSystemFontMap.find("monospace");
            if( it == sSystemFontMap.end()){
		sSystemFontMap.insert({std::string("monospace"),tf});
		LOGD("family [%s] is marked as [monospace]",family.c_str());
	    }
        }
	if(std::regex_search(family,patSerif)){
	    if(false == std::regex_search(family,patSans)&& false==std::regex_search(family,patMono)){
		auto it = sSystemFontMap.find("serif");
		if(it == sSystemFontMap.end()){
		    sSystemFontMap.insert({std::string("serif"),tf});
		     LOGD("family [%s] is marked as [serif]",family.c_str());
		}
	    }
	}
    }
    if(fs)FcFontSetDestroy(fs);
    FcConfigDestroy(config);
    return 0;
}

}
