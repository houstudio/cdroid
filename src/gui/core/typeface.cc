#include <core/typeface.h>
#include <core/textutils.h>
#include <regex>
#include <freetype/freetype.h>
#include <freetype/ftsnames.h>
#include <cairomm/matrix.h>
#include <dirent.h>
namespace cdroid{

Typeface* Typeface::MONOSPACE;
Typeface* Typeface::SANS_SERIF;
Typeface* Typeface::SERIF;
Typeface* Typeface::DEFAULT;
Typeface* Typeface::DEFAULT_BOLD;
Typeface* Typeface::sDefaultTypeface;
std::string Typeface::mSystemLang;
static constexpr int SYSLANG_MATCHED = 0x80000000;

std::unordered_map<std::string, Typeface*>Typeface::sSystemFontMap;

Typeface::Typeface(Cairo::RefPtr<Cairo::FtScaledFont>face){
    mFontFace = face;
}

Typeface::Typeface(FcPattern & font,const std::string&family){
    int i,ret, weight=0;
    double pixelSize=.0f;
    FcChar8* s = FcNameUnparse(&font);
    LOGV("Font= %s",s);
    s = nullptr; 

    if(family.empty()){
	ret = FcPatternGetString(&font,FC_FAMILY,0,&s);
        if(ret == FcResultMatch){
	    mFamily = std::string((const char*)s);
	    s = nullptr;
	}
    }
    for(int i=0;FcPatternGetString(&font, FC_FAMILY,i, &s)==FcResultMatch;i++){
        mFamily +=std::string((const char*)s);
	mFamily +=";";
	s = nullptr;
    }
    LOGD("family=%s",mFamily.c_str());
    mStyle = 0 ;
    ret = FcPatternGetString(&font, FC_STYLE, 0, &s);
    if(ret==FcResultMatch)
        mStyle = parseStyle(std::string((const char*)s));
    LOGD_IF(ret==FcResultMatch,"Style=%s/%d",s,mStyle);
    s = nullptr;

    ret = FcPatternGetString(&font,FC_SLANT,0,&s);
    LOGD_IF(ret == FcResultMatch,"Slant=%s",s);
    s = nullptr;

    ret = FcPatternGetInteger(&font,FC_WEIGHT,0,&weight);
    LOGD_IF(ret == FcResultMatch,"weight =%d",weight);
    mWeight = weight;

    ret = FcPatternGetDouble(&font,FC_PIXEL_SIZE,0,&pixelSize);
    LOGD_IF(ret == FcResultMatch,"pixelSize =%f",pixelSize);

    ret = FcPatternGetDouble(&font,FC_DPI,0,&pixelSize);
    LOGD_IF(ret == FcResultMatch,"dpi =%f",pixelSize);

    FcLangSet *langset=nullptr;
    ret = FcPatternGetLangSet(&font, FC_LANG,0,&langset);
    //FcChar8 *lang = FcLangSetGetString(langset, 0);
    ret = FcLangSetHasLang(langset,(const FcChar8*)mSystemLang.c_str());
    if(ret==0)
	mStyle|=SYSLANG_MATCHED;
    //FcLangSetDestroy(langset);
    //it seems langset is destroied by FcPattern iteself,destroied  here willc aused crash
    LOGD("has %s=%d",mSystemLang.c_str(),ret);
    s = nullptr;

    Cairo::Matrix matrix = Cairo::identity_matrix();
    Cairo::Matrix ctm = Cairo::identity_matrix();
    Cairo::RefPtr<Cairo::FtFontFace> face = Cairo::FtFontFace::create(&font);
    mFontFace = Cairo::FtScaledFont::create(face,matrix,ctm);
}

int Typeface::parseStyle(const std::string&styleName){
    static const struct{
	const char*styleName;
	int styleProp;
    }stlMAP[]={
	{"(?=.*\\bregular\\b)", NORMAL},
	{"(?=.*\\bnormal\\b)" , NORMAL},
	{"(?=.*\\bitalic\\b)" , ITALIC},
	{"(?=.*\\bbold\\b)", BOLD},
	{NULL,0}
    };
    int style = NORMAL;
    for(int i=0; stlMAP[i].styleName;i++){
       const std::regex pat(stlMAP[i].styleName , std::regex_constants::icase);
       if(std::regex_search(styleName,pat))
	   style|=stlMAP[i].styleProp;
    }
    return style;
}

int Typeface::getWeight()const{
    return mWeight;
}

int Typeface::getStyle() const{
    return mStyle;
}

bool Typeface::isBold() const{
    return (mStyle & BOLD) != 0;
}

bool Typeface::isItalic() const{
    return (mStyle & ITALIC) != 0;
}

std::string Typeface::getFamily()const{
    return mFamily;
}

Cairo::RefPtr<Cairo::FtScaledFont>Typeface::getFontFace()const{
    return mFontFace;
}

double Typeface::getScale()const{
    return mScale;
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

    Typeface* typeface = family;
    int bestMactched=0;
    for(auto it= sSystemFontMap.begin();it!= sSystemFontMap.end();it++){
        Typeface* face = it->second;
        const int match= it->second->getStyle()==family->getStyle();
        if((it->second->getStyle()==style)&&(match>bestMactched)){
           typeface = face;
           bestMactched=match;
        }
    }    
    LOGV("typeface=%p family=%p",typeface,family);
    return typeface?typeface:family;
}

Typeface* Typeface::getSystemDefaultTypeface(const std::string& familyName){
    Typeface*face = Typeface::DEFAULT;
    int familyMatched = 0;
    int supportLangs  = 0;
    for(auto i= sSystemFontMap.begin();i!= sSystemFontMap.end();i++){
	 Typeface*tf = i->second;
         const std::string family=tf->getFamily();
	 std::vector<std::string>families = TextUtils::split(family,";");
	 auto it = std::find(families.begin(),families.end(),familyName);
	 const int ttfLangs = families.size();
	 if(it!=families.end()){
	     face = tf;
	     familyMatched++;
	     break;
	 }else if((familyMatched==0)&&(tf->mStyle&SYSLANG_MATCHED)){// && (ttfLangs>supportLangs)){
             face = tf;
	 }
    }
    LOGV_IF(face&&familyName.size(),"want %s got %s style=%x",familyName.c_str(),face->getFamily().c_str(),face->mStyle);
    return face;
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

    Typeface* typeface = base;
    int bestMactched=0;
    for(auto it= sSystemFontMap.begin();it!= sSystemFontMap.end();it++){
	Typeface* face = it->second;
	const int match= base->getFamily()==it->first;
	if((it->second->isItalic()==italic)&&(match>bestMactched)){
	   typeface = face;
	   bestMactched=match;
	}
    }
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
		double scale = (double)ftFace->max_advance_height/ftFace->units_per_EM;
		Cairo::RefPtr<Cairo::FtFontFace> face = Cairo::FtFontFace::create(ftFace,FT_LOAD_DEFAULT);
		Cairo::Matrix matrix;
		if(scale!=.0f)
		    matrix = Cairo::scaling_matrix(scale,scale);
		else{
		    matrix = Cairo::identity_matrix();
		    scale  = 1.f;
		}
                Cairo::Matrix ctm = Cairo::identity_matrix();
                auto autoft= Cairo::FtScaledFont::create(face,matrix,ctm);
                Typeface *typeface = new Typeface(autoft);
		typeface->mScale = scale;
                sSystemFontMap.insert({std::string(ftFace->family_name),typeface});
		typeface->mStyle = parseStyle(ftFace->style_name);
                LOGI("[%s] style=%s/%x %d glyphs face.height=%x units_per_EM=%x scale=%f",
			ftFace->family_name,ftFace->style_name,ftFace->style_flags,ftFace->num_glyphs,
			ftFace->max_advance_height,ftFace->units_per_EM,scale);
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
    //return loadFromPath("");
    LOGI("Total fonts: %d", fs->nfont);
    const std::regex patSerif("(?=.*\\bserif\\b)" , std::regex_constants::icase);
    const std::regex patSans( "(?=.*\\bsans\\b)" , std::regex_constants::icase);
    const std::regex patMono( "(?=.*\\bmono\\b)" , std::regex_constants::icase);
    const char*langenv=getenv("LANG");
    std::string lang = "en_US.UTF-8";
    if(langenv)lang = langenv;
    size_t pos = lang.find('.');
    if(pos!=std::string::npos)
        lang=lang.substr(0,pos);
    pos = lang.find('_');
    if(pos!=std::string::npos)
        lang[pos]='-';
    mSystemLang = lang;
    for (int i=0; fs && i < fs->nfont; i++) {
	FcPattern *pat = fs->fonts[i];//FcPatternDuplicate(fs->fonts[i]);
	Typeface   *tf = new Typeface(*pat,"");
	const std::string family = tf->getFamily();

	sSystemFontMap.insert({family,tf});
	LOGD("font %s %p",family.c_str(),tf);
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
