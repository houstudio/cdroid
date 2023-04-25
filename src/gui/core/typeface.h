#ifndef __TYPEFACE_H__
#define __TYPEFACE_H__
#include <string>
#include <core/callbackbase.h>

namespace cdroid{

class Typeface{
public:
    static constexpr bool ENABLE_LAZY_TYPEFACE_INITIALIZATION=true;
    static constexpr int RESOLVE_BY_FONT_TABLE = -1;
    static constexpr const char*DEFAULT_FAMILY = "sans-serif";
    // Style
    static constexpr int NORMAL = 0;
    static constexpr int BOLD = 1;
    static constexpr int ITALIC = 2;
    static constexpr int BOLD_ITALIC = 3;
    /** The default NORMAL typeface object */
    static Typeface* DEFAULT;

    static Typeface* DEFAULT_BOLD;
    /** The NORMAL style of the default sans serif typeface. */
    static Typeface* SANS_SERIF;
    /** The NORMAL style of the default serif typeface. */
    static Typeface* SERIF;
    /** The NORMAL style of the default monospace typeface. */
    static Typeface* MONOSPACE;
private:
    Runnable mCleaner;
    std::string mFallbackFamilyName;
    int mStyle;
    int mWeight;
    int mItalic;
private:
    static void setDefault(Typeface* t);
    static Typeface* getDefault();
    static bool hasFontFamily(const std::string&familyName);
    static Typeface* createWeightStyle(Typeface* base,int weight, bool italic);
public:
    int getWeight()const{
        return mWeight;
    }
    int getStyle() const{
        return mStyle;
    } 
    bool isBold() const{
        return (mStyle & BOLD) != 0;
    }
    bool isItalic() const{
        return (mStyle & ITALIC) != 0;
    }
    //static Typeface* createFromResources(FamilyResourceEntry entry, AssetManager mgr,const std::string& path)
    static Typeface* findFromCache(/*AssetManager mgr,*/const std::string& path);
    static Typeface* create(const std::string& familyName,int style);
    static Typeface* create(Typeface* family,int style);
    static Typeface* create(Typeface* family,int weight, bool italic);
    static Typeface* defaultFromStyle(int style);
    static Typeface* createFromAsset(const std::string path);
};

class FontStyle {
public:
    static constexpr int FONT_WEIGHT_MIN = 1;
    static constexpr int FONT_WEIGHT_THIN = 100;
    static constexpr int FONT_WEIGHT_EXTRA_LIGHT = 200;
    static constexpr int FONT_WEIGHT_LIGHT = 300;
    static constexpr int FONT_WEIGHT_NORMAL = 400;
    static constexpr int FONT_WEIGHT_MEDIUM = 500;
    static constexpr int FONT_WEIGHT_SEMI_BOLD = 600;
    static constexpr int FONT_WEIGHT_BOLD = 700;
    static constexpr int FONT_WEIGHT_EXTRA_BOLD = 800;
    static constexpr int FONT_WEIGHT_BLACK = 900;
    static constexpr int FONT_WEIGHT_MAX = 1000;
    static constexpr int FONT_SLANT_UPRIGHT = 0;
    static constexpr int FONT_SLANT_ITALIC = 1;
private:
    int mWeight;
    int mSlant;
public:
    FontStyle() {
        mWeight = FONT_WEIGHT_NORMAL;
        mSlant = FONT_SLANT_UPRIGHT;
    }
    int getWeight()const{
        return mWeight;
    }
    int getSlant()const {
        return mSlant;
    }
};

}

#endif
