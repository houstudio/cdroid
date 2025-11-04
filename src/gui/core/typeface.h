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
#ifndef __TYPEFACE_H__
#define __TYPEFACE_H__
#include <string>
#include <unordered_map>
#include <cairomm/scaledfont.h>

namespace cdroid{
class Context;
class FontFamily{};
class Typeface{
public:
    static constexpr bool ENABLE_LAZY_TYPEFACE_INITIALIZATION=true;
    static constexpr int RESOLVE_BY_FONT_TABLE = -1;
    static constexpr const char*DEFAULT_FAMILY = "sans-serif";
    // Style
    static constexpr int NORMAL = 0;
    static constexpr int BOLD   = 1;
    static constexpr int ITALIC = 2;
    static constexpr int BOLD_ITALIC = 3;
    static constexpr int STYLE_MASK  = 0x03;
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
    static std::string mFallbackFamilyName;
    std::string mFamily;
    std::string mStyleName;
    std::string mFileName;
    int mStyle;
    int mWeight;
    int mItalic;
    Cairo::RefPtr<Cairo::FtScaledFont>mFontFace;
    static cdroid::Context*mContext;
    static std::string mSystemLang;
    static Typeface* sDefaultTypeface;
    static Typeface* sDefaults[4];
    static std::unordered_map<std::string,std::shared_ptr<Typeface>> sSystemFontMap;
    static std::unordered_map<std::string,std::vector<FontFamily>>systemFallbackMap;
    static std::unordered_map<void*,Typeface*>sStyledTypefaceCache;
private:
    struct Deleter;
    static void setDefault(Typeface* t);
    static Typeface* getDefault();
    static bool hasFontFamily(const std::string&familyName);
    static Typeface* createWeightStyle(Typeface* base,int weight, bool italic);
    static Typeface* getSystemDefaultTypeface(const std::string& familyName);
    Typeface(Cairo::RefPtr<Cairo::FtScaledFont>face);
    Typeface(const FcPattern&);
    ~Typeface()=default;
    static int parseStyle(const std::string&style,std::string&normalizedName);
    void fetchProps(FT_Face);
public:
    int getWeight()const;
    int getStyle() const;
    bool isBold() const;
    bool isItalic() const;
    std::string getFamily()const;
    std::string getStyleName()const;
    Cairo::RefPtr<Cairo::FtScaledFont>getFontFace()const;
    static void setContext(cdroid::Context*);
    static void setFallback(const std::string&);
    //static Typeface* createFromResources(cdroid::Context*context,const std::string& path);
    static void buildSystemFallback(const std::string xmlPath,const std::string& fontDir,
           std::unordered_map<std::string, Typeface*>& fontMap, 
	   std::unordered_map<std::string, std::vector<FontFamily>>& fallbackMap);
    //static Typeface* findFromCache(AssetManager mgr, const std::string& path);
    static std::shared_ptr<Typeface> make(const FcPattern& pat);
    static Typeface* create(const std::string& familyName,int style);
    static Typeface* create(Typeface* family,int style);
    static Typeface* create(Typeface* family,int weight, bool italic);
    static Typeface* defaultFromStyle(int style);
    static Typeface* createFromAsset(const std::string path);
    static void loadPreinstalledSystemFontMap();
    static int loadFromFontConfig();
    static int loadFromPath(const std::string&path);
    static int loadFaceFromResource(cdroid::Context*context);
};

class FontStyle {
public:
    static constexpr int FONT_WEIGHT_MIN  = 1;
    static constexpr int FONT_WEIGHT_THIN = 100;
    static constexpr int FONT_WEIGHT_EXTRA_LIGHT = 200;
    static constexpr int FONT_WEIGHT_LIGHT  = 300;
    static constexpr int FONT_WEIGHT_NORMAL = 400;
    static constexpr int FONT_WEIGHT_MEDIUM = 500;
    static constexpr int FONT_WEIGHT_SEMI_BOLD = 600;
    static constexpr int FONT_WEIGHT_BOLD = 700;
    static constexpr int FONT_WEIGHT_EXTRA_BOLD = 800;
    static constexpr int FONT_WEIGHT_BLACK = 900;
    static constexpr int FONT_WEIGHT_MAX   = 1000;
    static constexpr int FONT_SLANT_UPRIGHT= 0;
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
