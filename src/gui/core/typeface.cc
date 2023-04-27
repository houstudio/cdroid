#include <core/typeface.h>

namespace cdroid{

Typeface* Typeface::MONOSPACE;
Typeface* Typeface::SANS_SERIF;
Typeface* Typeface::SERIF;
Typeface* Typeface::DEFAULT;
Typeface* Typeface::DEFAULT_BOLD;
Typeface* Typeface::sDefaultTypeface;
std::unordered_map<std::string, Typeface*>Typeface::sSystemFontMap;

Typeface::Typeface(Cairo::RefPtr<Cairo::FtFontFace>face){
    mFontFace = face;
}

Typeface* Typeface::create(Typeface*family, int style){
    if ((style & ~STYLE_MASK) != 0) {
        style = NORMAL;
    }
    if (family == nullptr) {
        family = getDefault();
    }

    // Return early if we're asked for the same face/style
    if (family->mStyle == style) {
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
    return typeface;
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
    //synchronized (SYSTEM_FONT_MAP_LOCK) {
    sDefaultTypeface = t;
    //    nativeSetDefault(t.native_instance);
    //}
}

Typeface* Typeface::getDefault(){
    return sDefaultTypeface;
}

void Typeface::buildSystemFallback(const std::string xmlPath,const std::string& fontDir,
        std::unordered_map<std::string, Typeface*>& fontMap,
	std::unordered_map<std::string, std::vector<FontFamily>>& fallbackMap){
}

void Typeface::setSystemFontMap(const std::unordered_map<std::string, Typeface*>&systemFontMap){
    sSystemFontMap = systemFontMap;
    auto it=sSystemFontMap.find(DEFAULT_FAMILY);
    if (it!=sSystemFontMap.end()) {
        setDefault(it->second);
    }
}

void Typeface::loadPreinstalledSystemFontMap(){
    const char*families[]={"serif","sans-serif","monospace"};
    std::unordered_map<std::string, Typeface*>fonts;
    for(int i=0;i<3;i++){
	FcPattern* fontPattern = FcPatternCreate();
        FcResult fontResult = FcResultMatch;
        int fontSize = 74;
        FcPatternAddString(fontPattern, FC_FAMILY,(const FcChar8*)families[i]);
	Typeface*tf = new Typeface(Cairo::FtFontFace::create(fontPattern));
	fonts.insert({families[i],tf});
	FcPatternDestroy(fontPattern);
	LOGD("typeface %p=%s",tf,families[i]);
        //FcPatternAddDouble(fontPattern, FC_SIZE, fontSize);
        //FcPatternAddString(fontPattern, FC_SPACING,(const FcChar8*) fontSpacing);
        //FcPatternAddString(fontPattern, FC_STYLE,(const FcChar8*) fontStyle);
    }
    setSystemFontMap(fonts);
}

void Typeface::init(){
    DEFAULT      = create("", 0);
    DEFAULT_BOLD = create("", Typeface::BOLD);
    SANS_SERIF   = create("sans-serif", 0);
    SERIF        = create("serif", 0);
    MONOSPACE    = create("monospace", 0);
}

}
