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
namespace cdroid{ class Asset; }  // content/asset.h (finishAssetTypeface param, fwd-only here)
#include <unordered_map>
#include <vector>
#include <memory>
#include <cairomm/scaledfont.h>
#include <cairomm/surface.h>
namespace minikin{
    class FontFamily;
    class FontCollection;
    class MinikinFont;
    class MinikinPaint;
}
namespace cdroid{
class Context;
class FontData;  // core/typeface.cc (blob holder: Asset-backed mmap font bytes)
class FontFamily{};
class Typeface{
public:
    static constexpr bool ENABLE_LAZY_TYPEFACE_INITIALIZATION=true;
    static constexpr int RESOLVE_BY_FONT_TABLE = -1;
    static constexpr const char*DEFAULT_FAMILY = "sans-serif";
    enum Style{
        NORMAL = 0,
        BOLD   = 1,
        ITALIC = 2,
        BOLD_ITALIC = 3
    };
    /** The default NORMAL typeface object */
    static Typeface* DEFAULT;

    static Typeface* DEFAULT_BOLD;
    /** The NORMAL style of the default sans serif typeface. */
    static Typeface* SANS_SERIF;
    /** The NORMAL style of the default serif typeface. */
    static Typeface* SERIF;
    /** The NORMAL style of the default monospace typeface. */
    static Typeface* MONOSPACE;
    // AOSP Typeface.getDefault(): public @NonNull — "the default NORMAL
    // typeface object" (returns sDefaults[NORMAL]). Paint's null-face
    // fallback resolves through here.
    static Typeface* getDefault();
private:
    static constexpr int STYLE_MASK  = 0x03;
    static std::string mFallbackFamilyName;
    static std::string sFontConfigXml;  // optional Android fonts.xml/font_fallback.xml path
    std::string mFamily;
    std::string mFileName;
    int mFaceIndex = 0;
    int mStyle;
    int mWeight;
    int mItalic;
    Cairo::RefPtr<Cairo::FontFace>mFontFace;
    mutable std::shared_ptr<minikin::FontCollection>mFontCollection;
    std::shared_ptr<minikin::MinikinFont> mMinikinFont;
    // The font bytes backing every FT_Face of this Typeface (file mmap or pak
    // window). FT memory faces read lazily, so the blob must outlive them —
    // it is held here and pinned process-lifetime inside FontData.
    std::shared_ptr<FontData> mFontData;
    static cdroid::Context*mContext;
    static std::string mSystemLang;
    static Typeface* sDefaultTypeface;
    static Typeface* sDefaults[4];
    static std::vector<std::shared_ptr<Typeface>> sSystemFontFaces;
    static std::unordered_map<std::string,std::shared_ptr<Typeface>> sSystemFontMap;
    static std::unordered_map<std::string,std::vector<FontFamily>>systemFallbackMap;
    static std::unordered_map<void*,Typeface*>sStyledTypefaceCache;
private:
    struct Deleter;
    static void setDefault(Typeface* t);
    static bool hasFontFamily(const std::string&familyName);
    static Typeface* createWeightStyle(Typeface* base,int weight, bool italic);
    static Typeface* getSystemDefaultTypeface(const std::string& familyName);
    //Typeface(Cairo::RefPtr<Cairo::FontFace>face);
    // Build a Typeface directly from font fields (Android fonts.xml path) — no fontconfig.
    // If `family` is empty (a fallback <family lang=...>), the real family is read from the
    // font file's family_name so buildSystemFallback/buildFamily can group it correctly.
    Typeface(const std::string& family, int weight, bool italic, const std::string& fileName, int faceIndex);
    // Memory-backed Typeface (e.g. PAK @font): font bytes live in the FontData
    // mapping held by this instance (AOSP Font.createBuffer mmap model).
    Typeface(const std::string& family, int weight, bool italic,
             std::shared_ptr<FontData> fontData, int faceIndex);
    static int loadFromFontsXml(const std::string& fontDir, const std::string& xmlPath);
    ~Typeface()=default;
    static std::shared_ptr<minikin::FontFamily>buildFamily(const std::string&family,const std::vector<std::shared_ptr<Typeface>>&faces);
public:
    int getWeight()const;
    int getStyle() const;
    bool isBold() const;
    bool isItalic() const;
    std::string getFamily()const;
    Cairo::RefPtr<Cairo::FontFace>getFontFace()const;
    std::shared_ptr<minikin::MinikinFont> getMinikinFont() const;
    std::shared_ptr<Cairo::ScaledFont> getScaledFont(const minikin::MinikinPaint&,
            const minikin::MinikinFont* minikinFont = nullptr) const;
    std::shared_ptr<minikin::FontCollection> getFontCollection() const;
    // ScaledFont cache statistics
    static void setContext(cdroid::Context*);
    static void setFallback(const std::string&);
    // Optional: set an Android fonts.xml / font_fallback.xml path. If set and the file
    // exists, loadPreinstalledSystemFontMap() uses it (curated named families + fallback
    // chain) instead of fontconfig. Unset/missing => fontconfig (no behavior change).
    static void setFontConfigXml(const std::string& path);
    //static Typeface* createFromResources(cdroid::Context*context,const std::string& path);
    static void buildSystemFallback();
    //static Typeface* findFromCache(AssetManager mgr, const std::string& path);
    static Typeface* create(const std::string& familyName,int style);
    static Typeface* create(Typeface* family,int style);
    static Typeface* create(Typeface* family,int weight, bool italic);
    static Typeface* defaultFromStyle(int style);
    static Typeface* createFromAsset(const std::string path);
    // AOSP Typeface.createFromResources: the R.font/<name> resource route.
    // The arsc value for a raw ttf font resource is the pak-relative file
    // path; loaded via AssetManager.openNonAsset (zip root path, no assets/
    // prefix) and cached per path like createFromAsset.
    static Typeface* createFromResourcePath(const std::string path);
    static void loadPreinstalledSystemFontMap();
    void initFace(FT_Face ftFace, const std::string& family);
private:
    // Shared tail of the two pak-font factories: adopt the Asset's mapping as
    // the FontData blob, build the memory-backed face, store it per path.
    static Typeface* finishAssetTypeface(const std::string& path, Asset* asset, const char* tag);
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

// CBDT/sbix color-glyph data (impl in typeface.cc). Pure font-resource
// access — the glyph's color bitmap as an ARGB32 surface plus its placement
// relative to the pen position at `textSize` — mirroring how MinikinFont
// serves GetBounds/GetFontExtent. Painting it is the caller's (Paint's) job.
struct ColorGlyph {
    Cairo::RefPtr<Cairo::ImageSurface> surface;  // premultiplied ARGB32, strike resolution
    double left;    // bitmap_left already scaled to textSize
    double top;     // -bitmap_top already scaled to textSize
    double scale;   // textSize / strike ppem — paint scales the surface by this
};
// Returns nullptr when the glyph is not a color bitmap glyph and must take
// the normal alpha-mask path.
const ColorGlyph* getColorGlyph(const minikin::MinikinFont* font,
        uint32_t glyphIndex, double textSize);

}

#endif
