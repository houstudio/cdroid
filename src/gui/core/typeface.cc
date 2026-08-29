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
#include <climits>
#include <memory>
#include <list>
#include <set>
#include <unordered_set>
#include <regex>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iterator>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <cdlog.h>
#include <core/app.h>
#include <content/asset.h>
#include <content/assetmanager.h>
#include <core/typeface.h>
#include <core/fontlistparser.h>
#include <functional>
#include <text/textutils.h>
#include <cairomm/matrix.h>
#include <core/context.h>
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/tttables.h>
#include <hb.h>
#include <hb-ft.h>
#include <minikin/GraphemeBreak.h>
#include <minikin/LocaleList.h>
#include <minikin/Measurement.h>
#include <minikin/MeasuredText.h>
#include <minikin/SystemFonts.h>
#include <core/canvas.h>
#include <unordered_map>
#include <cstdlib>
namespace cdroid {

class FullMinikinFont : public minikin::MinikinFont {
public:
    FullMinikinFont(const Cairo::RefPtr<Cairo::FtFontFace>& fontFace,
                    const std::string& filePath = {}, int faceIndex = 0)
        : mFontFace(fontFace), mFilePath(filePath), mFaceIndex(faceIndex) {
        mSourceId = mFontId++;
        // mmap the font file so GetFontData/GetFontSize expose a lazy, page-backed blob:
        // only the cmap/gsub/gpos pages harfbuzz touches get paged in, and the kernel can
        // reclaim them under memory pressure. Avoids copying the whole file into RAM
        // (important for embedded) — same approach upstream minikin/android uses.
        if (!mFilePath.empty()) {
            int fd = open(mFilePath.c_str(), O_RDONLY);
            if (fd >= 0) {
                struct stat st;
                if (fstat(fd, &st) == 0 && st.st_size > 0) {
                    void* p = mmap(nullptr, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
                    if (p != MAP_FAILED) {
                        mMappedData = p;
                        mMappedSize = st.st_size;
                        mData = mMappedData;
                        mSize = mMappedSize;
                    }
                }
                close(fd);
            }
        }
    }
    ~FullMinikinFont() override {
        if (mMappedData != nullptr) {
            munmap(mMappedData, mMappedSize);
        }
    }
    // Memory-backed variant (e.g. PAK @font): the font bytes live in `fontData` (kept alive
    // by the shared_ptr). Same GetFontData/GetFontSize contract as the mmap file variant.
    FullMinikinFont(const Cairo::RefPtr<Cairo::FtFontFace>& fontFace,
                    std::shared_ptr<std::vector<uint8_t>> fontData, int faceIndex = 0)
        : mFontFace(fontFace), mFaceIndex(faceIndex), mFontData(std::move(fontData)) {
        mSourceId = mFontId++;
        mData = mFontData ? (const void*)mFontData->data() : nullptr;
        mSize = mFontData ? mFontData->size() : 0;
    }
    int32_t GetSourceId() const override{
        return mSourceId;
    }
    float GetHorizontalAdvance(uint32_t glyph_id, const minikin::MinikinPaint& paint, const minikin::FontFakery&) const override {
        Cairo::RefPtr<Cairo::FtScaledFont> scaledFont = getScaledFont(paint);
        std::vector<Cairo::Glyph> glyphs{{glyph_id,0,0}};
        Cairo::TextExtents extents;
        scaledFont->get_glyph_extents(glyphs, extents);
        return extents.x_advance;
    }
    void GetBounds(minikin::MinikinRect* bounds, uint32_t glyph_id, const minikin::MinikinPaint& paint, const minikin::FontFakery&) const override {
        Cairo::RefPtr<Cairo::FtScaledFont> scaledFont = getScaledFont(paint);

        std::vector<Cairo::Glyph> glyphs{{glyph_id,0,0}};
        Cairo::TextExtents extents;
        scaledFont->get_glyph_extents(glyphs, extents);
        bounds->mLeft = extents.x_bearing;
        bounds->mTop  = extents.y_bearing;
        bounds->mRight = extents.x_bearing + extents.width;
        bounds->mBottom= extents.y_bearing + extents.height;
    }
    void GetFontExtent(minikin::MinikinExtent* extent, const minikin::MinikinPaint& paint, const minikin::FontFakery&) const override {
        Cairo::RefPtr<Cairo::FtScaledFont> scaledFont = getScaledFont(paint);
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
    const void* GetFontData() const override { return mData; }
    size_t GetFontSize() const override { return mSize; }
    int GetFontIndex() const override { return mFaceIndex; }
    const std::string& GetFontPath() const override { return mFilePath; }
    // LRU Cache key for ScaledFont: fontId + MinikinPaint hash
    struct ScaledFontKey {
        int fontId;
        uint32_t paintHash;

        bool operator==(const ScaledFontKey& other) const {
            return fontId == other.fontId && paintHash == other.paintHash;
        }
    };

    struct ScaledFontKeyHash {
        size_t operator()(const ScaledFontKey& k) const {
            size_t h = std::hash<int>()(k.fontId);
            h ^= std::hash<uint32_t>()(k.paintHash) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };

    struct ScaledFontEntry {
        Cairo::RefPtr<Cairo::FtScaledFont> font;
        std::list<ScaledFontKey>::iterator lruIter;
    };

    // Global LRU cache for ScaledFonts, shared by all FullMinikinFont instances
    // Reduced from 64 to 32 for better cache locality
    static constexpr size_t MAX_SCALED_FONT_CACHE = 32;
    static std::unordered_map<ScaledFontKey, ScaledFontEntry, ScaledFontKeyHash>& getGlobalScaledFontCache() {
        static std::unordered_map<ScaledFontKey, ScaledFontEntry, ScaledFontKeyHash> cache;
        return cache;
    }
    static std::list<ScaledFontKey>& getGlobalLruList() {
        static std::list<ScaledFontKey> lru;
        return lru;
    }
    // Cache statistics
    static std::atomic<uint64_t>& getCacheHits() {
        static std::atomic<uint64_t> hits(0);
        return hits;
    }
    static std::atomic<uint64_t>& getCacheMisses() {
        static std::atomic<uint64_t> misses(0);
        return misses;
    }

    Cairo::RefPtr<Cairo::FtScaledFont> getScaledFont(const minikin::MinikinPaint& paint) const {
        // ScaledFont depends only on (font, size, scaleX, skewX); font feature settings
        // shape differently but don't change the scaled font, so skip the cache for them
        // to avoid junking it.
        if (paint.skipCache()) {
            return createScaledFont(paint.size, paint.scaleX, paint.skewX);
        }
        ScaledFontKey key{mSourceId, paint.hash()};
        auto& cache = getGlobalScaledFontCache();
        auto& lru = getGlobalLruList();
        auto it = cache.find(key);
        if (it != cache.end()) {
            getCacheHits()++;
            // Move to front (most recently used); splice keeps iterators valid.
            if (it->second.lruIter != lru.begin()) {
                lru.splice(lru.begin(), lru, it->second.lruIter);
                it->second.lruIter = lru.begin();
            }
            return it->second.font;
        }
        getCacheMisses()++;
        auto newFont = createScaledFont(paint.size, paint.scaleX, paint.skewX);
        // Evict least-recently-used when full (the old code froze the cache once full,
        // so every new (font,paint) combo after 32 entries was never cached).
        if (lru.size() >= MAX_SCALED_FONT_CACHE) {
            cache.erase(lru.back());
            lru.pop_back();
        }
        lru.push_front(key);
        cache[key] = {newFont, lru.begin()};
        return newFont;
    }
private:
    Cairo::RefPtr<Cairo::FtScaledFont> createScaledFont(float size, float scaleX = 1.0f, float skewX = 0.0f) const {
        // Apply scaleX and skewX to the font matrix
        float scaledSize = size * scaleX;
        Cairo::Matrix font_mtx(scaledSize, skewX, 0.0f, size, 0.0f, 0.0f);
        Cairo::FontOptions options;
        Cairo::Matrix ctm = Cairo::identity_matrix();
        options.set_hint_style(Cairo::FontOptions::HintStyle::MEDIUM);
        options.set_hint_metrics(Cairo::FontOptions::HintMetrics::OFF);
        return Cairo::FtScaledFont::create(mFontFace, font_mtx, ctm, options);
    }
    static int mFontId;
    int mSourceId;
    Cairo::RefPtr<Cairo::FtFontFace> mFontFace;
    std::string mFilePath;
    int mFaceIndex;
    void* mMappedData = nullptr;
    size_t mMappedSize = 0;
    std::shared_ptr<std::vector<uint8_t>> mFontData;  // memory-backed (PAK) bytes
    const void* mData = nullptr;  // unified blob ptr: mMappedData or mFontData->data()
    size_t mSize = 0;
};
int FullMinikinFont::mFontId=0;

static FT_Library ftLibrary = nullptr;  // shared by FT_New_Face paths (file + fonts.xml)

// Forward decl: defined further down, but getFontCollection() (above its definition) uses it.
static bool isSameFamily(const std::string& fm1, const std::string& fm2);

// The shared fallback chain: one FontFamily per unique family, built once by
// buildSystemFallback() and reused by every Typeface's lazily-built FontCollection.
// This is the Android fallback model: a Typeface renders with [its primary family]
// followed by the shared chain, which supplies glyphs the primary lacks.
struct FallbackFamilyEntry {
    std::string name;
    std::shared_ptr<minikin::FontFamily> family;
};
static std::vector<FallbackFamilyEntry>& fallbackFamilies() {
    static std::vector<FallbackFamilyEntry> chain;
    return chain;
}

// One FontCollection per UNIQUE family that is actually used, cached & shared by all
// Typefaces of that family (e.g. regular/bold/italic of "DejaVu Sans" share one).
static std::shared_ptr<minikin::FontCollection> collectionForFamily(const std::string& family) {
    static std::unordered_map<std::string, std::shared_ptr<minikin::FontCollection>> cache;
    auto exact = cache.find(family);
    if (exact != cache.end()) return exact->second;
    for (auto& kv : cache) {  // fuzzy isSameFamily hit
        if (isSameFamily(kv.first, family)) { cache[family] = kv.second; return kv.second; }
    }
    // Build [this family's primary FontFamily] + [shared fallback chain].
    const auto& chain = fallbackFamilies();
    int primary = -1;
    for (size_t i = 0; i < chain.size(); ++i) {
        if (isSameFamily(chain[i].name, family)) { primary = (int)i; break; }
    }
    std::vector<std::shared_ptr<minikin::FontFamily>> ordered;
    if (primary >= 0) ordered.push_back(chain[primary].family);
    for (size_t i = 0; i < chain.size(); ++i) {
        if ((int)i != primary) ordered.push_back(chain[i].family);
    }
    auto col = minikin::FontCollection::create(std::move(ordered));
    cache[family] = col;
    return col;
}

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
std::string Typeface::sFontConfigXml;

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

void Typeface::setContext(cdroid::Context*ctx){
    mContext = ctx;
}

void Typeface::setFallback(const std::string&family){
    mFallbackFamilyName = family;
}

void Typeface::setFontConfigXml(const std::string& path) {
    sFontConfigXml = path;
}

// Typeface built directly from Android fonts.xml font fields (no fontconfig).
Typeface::Typeface(const std::string& family, int weight, bool italic,
                   const std::string& fileName, int faceIndex) {
    mFileName = fileName;
    mFaceIndex = faceIndex;
    mWeight = weight;
    mStyle = (weight >= 600 ? BOLD : 0) | (italic ? ITALIC : 0);
    if (ftLibrary == nullptr) FT_Init_FreeType(&ftLibrary);
    FT_Face ftFace = nullptr;
    if (!FT_New_Face(ftLibrary, fileName.c_str(), faceIndex, &ftFace) && ftFace != nullptr) {
        mFamily = family.empty() ? std::string(ftFace->family_name ? ftFace->family_name : "")
                                 : family;
        mStyleName = ftFace->style_name ? ftFace->style_name : "";
        // cairo's for-ft-face wrapper does NOT take a reference: keep the face
        // alive for the process (registry typefaces are cached globally) by
        // bumping the refcount before our own Done drops it.
        FT_Reference_Face(ftFace);
        mFontFace = Cairo::FtFontFace::create(ftFace, 0);
        FT_Done_Face(ftFace);
    } else {
        // Font file missing/unreadable: keep the provided family, leave mFontFace null.
        mFamily = family;
    }
}

namespace {
// Faces loaded from pak entries (asset or R.font resource), cached per resolved
// path for the process lifetime: repeated calls return the SAME instance (AOSP
// parity) and one FT_Face/FontCollection is shared across every paint using it.
std::unordered_map<std::string, std::shared_ptr<Typeface>>& assetTypefaceCache() {
    static std::unordered_map<std::string, std::shared_ptr<Typeface>> cache;
    return cache;
}
} // namespace

// Typeface.createFromAsset (AOSP returns an app-held object backed by GC; here
// the face is cached per asset path and owned by the cache for the process
// lifetime, so repeated calls share one FreeType face and callers never free).
// Reads through AssetManager.open like the AOSP original — not getInputStream.
Typeface* Typeface::createFromAsset(const std::string path) {
    auto& cache = assetTypefaceCache();
    auto it = cache.find(path);
    if (it != cache.end()) return it->second.get();

    Asset* asset = App::getInstance().getAssets().open(path.c_str(), Asset::ACCESS_BUFFER);
    return finishAssetTypeface(path, asset, "createFromAsset");
}

// AOSP Typeface.createFromResources: R.font resource route. Same cache and
// loading, but the path comes from the arsc and is opened via openNonAsset
// (zip root path, no assets/ prefix). aapt2 records apk-style "res/<type>/<f>"
// paths while cdroid paks store entries with the "res/" prefix stripped (the
// pak naming convention) — normalize, like openPakPath does for -v4 suffixes.
Typeface* Typeface::createFromResourcePath(const std::string path) {
    std::string entry = (path.rfind("res/", 0) == 0) ? path.substr(4) : path;

    auto& cache = assetTypefaceCache();
    auto it = cache.find(entry);
    if (it != cache.end()) return it->second.get();

    Asset* asset = App::getInstance().getAssets().openNonAsset(entry.c_str(), Asset::ACCESS_BUFFER);
    return finishAssetTypeface(entry, asset, "createFromResourcePath");
}

Typeface* Typeface::finishAssetTypeface(const std::string& path, Asset* asset, const char* tag) {
    auto& cache = assetTypefaceCache();
    if (asset == nullptr) {
        LOGW("%s: not found: %s", tag, path.c_str());
        return nullptr;
    }
    const ssize_t size = (ssize_t)asset->getLength();
    auto fontData = std::make_shared<std::vector<uint8_t>>();
    if (size > 0) {
        const uint8_t* buffer = static_cast<const uint8_t*>(asset->getBuffer(true));
        if (buffer != nullptr) fontData->assign(buffer, buffer + size);
    }
    delete asset; // AOSP closes the Asset after reading the bytes
    if (fontData->empty()) {
        LOGW("%s: empty: %s", tag, path.c_str());
        return nullptr;
    }
    auto tf = std::shared_ptr<Typeface>(new Typeface("", 400 /* normal */, false, fontData, 0),
                                        Deleter{});
    cache[path] = tf;
    return tf.get();
}

// Memory-backed Typeface (PAK @font): font bytes live in `fontData`. FT_Face via
// FT_New_Memory_Face; the MinikinFont is the memory variant (GetFontData returns the
// buffer) so it obeys the standard blob contract without needing a file path.
Typeface::Typeface(const std::string& family, int weight, bool italic,
                   std::shared_ptr<std::vector<uint8_t>> fontData, int faceIndex) {
    mWeight = weight;
    mStyle = (weight >= 600 ? BOLD : 0) | (italic ? ITALIC : 0);
    mFaceIndex = faceIndex;
    if (ftLibrary == nullptr) FT_Init_FreeType(&ftLibrary);
    if (!fontData || fontData->empty()) { mFamily = family; return; }
    FT_Face ftFace = nullptr;
    if (FT_New_Memory_Face(ftLibrary, fontData->data(), fontData->size(), faceIndex, &ftFace) || !ftFace) {
        mFamily = family;
        return;
    }
    mFamily = family.empty() ? std::string(ftFace->family_name ? ftFace->family_name : "") : family;
    mStyleName = ftFace->style_name ? ftFace->style_name : "";
    FT_Reference_Face(ftFace);  // cairo's for-ft-face wrapper holds no reference
    mFontFace = Cairo::FtFontFace::create(ftFace, 0);
    FT_Done_Face(ftFace);
    // Eagerly build the memory-backed MinikinFont so getMinikinFont() returns it directly
    // (rather than the file/mmap path, which needs a real file path).
    auto ftFaceRef = std::dynamic_pointer_cast<Cairo::FtFontFace>(mFontFace);
    if (ftFaceRef) {
        mMinikinFont = std::make_shared<FullMinikinFont>(ftFaceRef, fontData, faceIndex);
        // And a DEDICATED single-font collection: getFontCollection()'s lazy
        // path resolves by family name against the system registry, which
        // knows nothing about asset/resource fonts (they would silently
        // measure with the default face). Pre-setting skips that lookup.
        mFontCollection = minikin::FontCollection::create(
                {minikin::FontFamily::create({minikin::Font::Builder(mMinikinFont).build()})});
    }
}

// Parse a fonts.xml / font_fallback.xml and populate the registry from it.
// Returns the number of fonts that were actually loaded (existing files only).
int Typeface::loadFromFontsXml(const std::string& fontDir, const std::string& xmlPath) {
    FontConfig cfg = parseFontConfig(xmlPath, fontDir);
    int loaded = 0;
    for (const auto& fam : cfg.families) {
        for (const auto& fnt : fam.fonts) {
            std::ifstream test(fnt.fileName);
            if (!test.good()) { LOGW("font file missing, skip: %s", fnt.fileName.c_str()); continue; }
            auto tf = std::shared_ptr<Typeface>(
                    new Typeface(fam.name, fnt.weight, fnt.italic, fnt.fileName, fnt.index),
                    Deleter{});  // ~Typeface is private; Deleter (a member struct) can call it
            sSystemFontFaces.push_back(tf);
            mFontFaces.push_back(tf->getFontFace());
            sSystemFontMap.insert({fnt.fileName, tf});               // unique key per font
            const std::string famKey = !fam.name.empty() ? fam.name : tf->getFamily();
            if (!famKey.empty()) sSystemFontMap.insert({famKey, tf}); // family-name key
            ++loaded;
        }
    }
    for (const auto& al : cfg.aliases) {
        auto it = sSystemFontMap.find(al.to);
        if (it != sSystemFontMap.end()) sSystemFontMap.insert({al.name, it->second});
    }
    LOGI("loadFromFontsXml: %d fonts from %s", loaded, xmlPath.c_str());
    return loaded;
}

/*Typeface::Typeface(Cairo::RefPtr<Cairo::FontFace>face) {
    mFontFace = face;
}*/

std::vector<Cairo::RefPtr<Cairo::FontFace>>Typeface::getFontFaces(){
    return mFontFaces;
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
            const_cast<Typeface*>(this)->mMinikinFont = std::make_shared<FullMinikinFont>(ftFace, mFileName, mFaceIndex);
        }
    }
    return mMinikinFont;
}

std::shared_ptr<Cairo::ScaledFont> Typeface::getScaledFont(const minikin::MinikinPaint& paint,
        const minikin::MinikinFont* minikinFont) const {
    // 如果提供了 minikinFont，优先使用它；否则使用 mMinikinFont
    const minikin::MinikinFont* fontToUse = minikinFont;
    if (fontToUse == nullptr) {
        fontToUse = mMinikinFont.get();
    }

    if (fontToUse != nullptr) {
        // 将 MinikinFont 转换为 FullMinikinFont
        const FullMinikinFont* fullFont = dynamic_cast<const FullMinikinFont*>(fontToUse);
        if (fullFont != nullptr) {
            // 使用 paint 获取 ScaledFont（会使用 LRU 缓存）
            return fullFont->getScaledFont(paint);
        }
    }
    return nullptr;
}

std::shared_ptr<minikin::FontCollection> Typeface::getFontCollection() const {
    if (!mFontCollection) {
        // Lazily fetch (or build+cache) this family's collection. Same-family Typefaces
        // share one; only families actually used pay for a FontCollection.
        mFontCollection = collectionForFamily(mFamily);
    }
    return mFontCollection;
}

void Typeface::getScaledFontCacheStats(uint64_t& hits, uint64_t& misses) {
    hits = FullMinikinFont::getCacheHits().load();
    misses = FullMinikinFont::getCacheMisses().load();
}

void Typeface::resetScaledFontCacheStats() {
    FullMinikinFont::getCacheHits().store(0);
    FullMinikinFont::getCacheMisses().store(0);
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
            auto minikinFont = std::make_shared<FullMinikinFont>(ft, f->mFileName, f->mFaceIndex);
            auto font = minikin::Font::Builder(minikinFont).build();
            fonts.push_back(font);
            LOGD("    ->[%d](%s): %s",fonts.size()-1,f->getFamily().c_str(),f->mFileName.c_str());
        }
    }
    return minikin::FontFamily::create(std::move(fonts));
}
void Typeface::buildSystemFallback() {
    // Build the shared fallback chain: one FontFamily per unique family (dedup'd via
    // isSameFamily), in fontconfig load order. These FontFamily objects — and their
    // FullMinikinFont / mmap — are built once and shared by every Typeface's lazily-built
    // FontCollection (see getFontCollection). No per-Typeface FontCollection is built here.
    auto& chain = fallbackFamilies();
    chain.clear();
    for (auto& tf : sSystemFontFaces) {
        bool dup = false;
        for (auto& e : chain) {
            if (isSameFamily(e.name, tf->mFamily)) { dup = true; break; }
        }
        if (dup) continue;
        auto fam = buildFamily(tf->mFamily, sSystemFontFaces);
        if (fam && fam->getNumFonts() > 0) {
            chain.push_back({tf->mFamily, fam});
            LOGD("fallback family[%zu]: %s", chain.size() - 1, tf->mFamily.c_str());
        }
    }
}

// Walk up from the executable looking for a fonts.xml beside the build
// artifacts (build.sh snapshots the host fontconfig into <out>/fonts.xml,
// the same level as cdroid.pak), mirroring App::findSharedPak's probe.
static std::string findFontsXmlNearExecutable() {
#if defined(__linux__)
    char rp[PATH_MAX] = {0};
    if (!realpath("/proc/self/exe", rp)) return std::string();
    std::string dir = rp;
    const size_t slash = dir.find_last_of('/');
    if (slash == std::string::npos) return std::string();
    dir = dir.substr(0, slash);
    for (int up = 0; up < 4 && !dir.empty(); up++) {
        std::string candidate = dir + "/fonts.xml";
        std::ifstream test(candidate);
        if (test.good()) return candidate;
        const size_t s = dir.find_last_of('/');
        if (s == std::string::npos || s == 0) break;
        dir = dir.substr(0, s);
    }
#endif
    return std::string();
}

void Typeface::loadPreinstalledSystemFontMap() {
    static bool sLoadAttempted = false;  // empty registry must not retry per create()
    if(sSystemFontMap.size() || sLoadAttempted) return;
    sLoadAttempted = true;

    // Prefer an Android fonts.xml / font_fallback.xml (curated named families + ordered
    // fallback chain). Try the explicitly-configured path, an env override, the
    // build-tree snapshot next to the executable, then common system locations.
    // Fontconfig enumeration stays as the last resort only — a full desktop
    // font set makes startup crawl.
    bool loadedFromXml = false;
    std::vector<std::string> candidates;
    if (!sFontConfigXml.empty()) candidates.push_back(sFontConfigXml);
    if (const char* env = getenv("CDROID_FONTS_XML")) if (*env) candidates.push_back(env);
    candidates.push_back(findFontsXmlNearExecutable());
    candidates.push_back("/system/etc/font_fallback.xml");
    candidates.push_back("/system/etc/fonts.xml");
    candidates.push_back("/etc/fonts/fonts.xml");
    for (const auto& path : candidates) {
        if (path.empty()) continue;
        std::ifstream test(path);
        if (!test.good()) continue;
        test.close();
        std::string fontDir;
        const size_t slash = path.find_last_of('/');
        fontDir = (slash != std::string::npos) ? path.substr(0, slash + 1) : "./";
        if (loadFromFontsXml(fontDir, path) > 0) {
            loadedFromXml = true;
            break;
        }
    }
    if (!loadedFromXml) {
        LOGW("no fonts.xml found (env CDROID_FONTS_XML / app-bundled / out-root snapshot"
             " / /system/etc); fontconfig enumeration is retired — run build.sh so"
             " scripts/genfontsxml.sh can snapshot the host fonts");
    }
    //loadFromPath("");
    //loadFaceFromResource(mContext);  // optional: also load fonts from the app's pak resources
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

    // 验证 DEFAULT 是否有 FontCollection（getFontCollection 会触发懒构造）
    LOGD("DEFAULT=%p [%s] FontCollection=%p",
         DEFAULT, DEFAULT->mFamily.c_str(), DEFAULT->getFontCollection().get());
    LOGD("DEFAULT_BOLD=%p [%s] FontCollection=%p",
         DEFAULT_BOLD, DEFAULT_BOLD->mFamily.c_str(), DEFAULT_BOLD->getFontCollection().get());

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



// Pull (family, weight, italic) straight from the FreeType face (no fontconfig):
// family from the name table, weight from OS/2 usWeightClass (bold style flag as
// fallback), italic from the style flags. Shared by the in-memory (small font →
// vector blob) and file-backed (large font → tmp + mmap) PAK paths.
static void extractMetaFromFace(FT_Face ftFace, std::string& family, int& weight, bool& italic) {
    family = ftFace->family_name ? ftFace->family_name : "";
    italic = (ftFace->style_flags & FT_STYLE_FLAG_ITALIC) != 0;
    const TT_OS2* os2 = (const TT_OS2*)FT_Get_Sfnt_Table(ftFace, FT_SFNT_OS2);
    if (os2 != nullptr && os2->usWeightClass >= 100) {
        weight = os2->usWeightClass;
    } else {
        weight = (ftFace->style_flags & FT_STYLE_FLAG_BOLD) ? 700 : 400;
    }
}

int Typeface::loadFaceFromResource(cdroid::Context* context) {
    std::vector<std::string> fonts;
    // The font lists are string-array resources ("fonts"/"font"); resolve them
    // by identifier (the string-keyed Context::getArray is retired).
    const std::string pkg = context->getPackageName();
    for (const char* name : { "fonts", "font" }) {
        const int id = context->getResources().getIdentifier(name, "array", pkg);
        if (id != 0) {
            const auto arr = context->getResources().getStringArray(id);
            fonts.insert(fonts.end(), arr.begin(), arr.end());
        }
    }
    if (ftLibrary == nullptr) FT_Init_FreeType(&ftLibrary);
    // Small fonts (below the threshold) are read fully into an in-memory blob; large ones are
    // streamed to a tmp file and mmap'd (lazy paging, low RAM). Non-intrusive: only uses
    // context->getInputStream — no Context/Assets/ziparchive changes.
    constexpr size_t LARGE_FONT_THRESHOLD = 256 * 1024;
    const std::string tmpDir = "/tmp/" + context->getPackageName();
    mkdir(tmpDir.c_str(), 0700);  // ignore EEXIST
    int loaded = 0;

    // Build + register per-face Typefaces from in-memory font bytes (small-font path).
    auto processMemory = [&](const uint8_t* data, size_t size, const std::string& key,
                             const std::function<std::shared_ptr<Typeface>(
                                     const std::string&, int, bool, int)>& makeFace) {
        FT_Face probe = nullptr;
        if (FT_New_Memory_Face(ftLibrary, data, size, 0, &probe) || !probe) return 0;
        const int numFaces = probe->num_faces;
        FT_Done_Face(probe);
        int n = 0;
        for (int fi = 0; fi < numFaces; ++fi) {
            FT_Face ft = nullptr;
            if (FT_New_Memory_Face(ftLibrary, data, size, fi, &ft) || !ft) continue;
            std::string family; int weight = 400; bool italic = false;
            extractMetaFromFace(ft, family, weight, italic);
            FT_Done_Face(ft);
            auto tf = makeFace(family, weight, italic, fi);
            if (!tf) continue;
            sSystemFontFaces.push_back(tf);
            mFontFaces.push_back(tf->getFontFace());
            sSystemFontMap.insert({key + (numFaces > 1 ? ("@" + std::to_string(fi)) : ""), tf});
            if (!family.empty()) sSystemFontMap.insert({family, tf});
            LOGD("@%s [%s] face[%d/%d] (blob)", key.c_str(), family.c_str(), fi, numFaces);
            ++n;
        }
        return n;
    };

    // Build + register per-face Typefaces from a file path (large-font path: file ctor mmaps it).
    auto processFile = [&](const std::string& path, const std::string& key) {
        FT_Face probe = nullptr;
        if (FT_New_Face(ftLibrary, path.c_str(), 0, &probe) || !probe) return 0;
        const int numFaces = probe->num_faces;
        FT_Done_Face(probe);
        int n = 0;
        for (int fi = 0; fi < numFaces; ++fi) {
            FT_Face ft = nullptr;
            if (FT_New_Face(ftLibrary, path.c_str(), fi, &ft) || !ft) continue;
            std::string family; int weight = 400; bool italic = false;
            extractMetaFromFace(ft, family, weight, italic);
            FT_Done_Face(ft);
            auto tf = std::shared_ptr<Typeface>(new Typeface(family, weight, italic, path, fi), Deleter{});
            sSystemFontFaces.push_back(tf);
            mFontFaces.push_back(tf->getFontFace());
            sSystemFontMap.insert({key + (numFaces > 1 ? ("@" + std::to_string(fi)) : ""), tf});
            if (!family.empty()) sSystemFontMap.insert({family, tf});
            LOGD("@%s [%s] face[%d/%d] (mmap %s)", key.c_str(), family.c_str(), fi, numFaces, path.c_str());
            ++n;
        }
        return n;
    };

    for (const auto& fontUrl : fonts) {
        std::string key = fontUrl;
        size_t d = key.find_last_of('.');
        if (d != std::string::npos) key = key.substr(0, d);
        d = key.find("fonts");
        if (d != std::string::npos) key.replace(d, 5, "font");
        std::string baseName = fontUrl;
        d = baseName.find_last_of('/');
        if (d != std::string::npos) baseName = baseName.substr(d + 1);
        d = baseName.find_last_of('.');
        if (d != std::string::npos) baseName = baseName.substr(0, d);

        std::unique_ptr<std::istream> istream = context->getInputStream(fontUrl);
        if (!istream) continue;

        // Size from the stream (seekable); non-seekable streams fall through to the tmp path.
        istream->seekg(0, std::ios::end);
        std::streampos sz = istream->tellg();
        const bool seekable = (sz != std::streampos(-1));
        const size_t fontSize = seekable ? (size_t)sz : 0;
        istream->seekg(0, std::ios::beg);

        if (seekable && fontSize < LARGE_FONT_THRESHOLD) {
            // Small: read into a vector blob.
            auto fontData = std::make_shared<std::vector<uint8_t>>(fontSize);
            if (fontSize > 0) istream->read(reinterpret_cast<char*>(fontData->data()), fontSize);
            if (static_cast<size_t>(istream->gcount()) != fontSize) continue;
            loaded += processMemory(fontData->data(), fontData->size(), key,
                    [&](const std::string& fam, int w, bool it, int fi) {
                        return std::shared_ptr<Typeface>(new Typeface(fam, w, it, fontData, fi), Deleter{});
                    });
        } else {
            // Large (or non-seekable): stream to a tmp file (cached), then mmap via the file ctor.
            const std::string tmpPath = tmpDir + "/" + baseName + ".bin";
            struct stat st;
            if (stat(tmpPath.c_str(), &st) != 0) {  // not cached yet: extract
                std::ofstream out(tmpPath, std::ios::binary | std::ios::trunc);
                if (!out) continue;
                char buf[65536];
                while (istream->read(buf, sizeof(buf)) || istream->gcount() > 0)
                    out.write(buf, istream->gcount());
                out.close();
            }
            loaded += processFile(tmpPath, key);
        }
    }
    LOGI("%d font loaded from resource (blob/tmp+mmap)", loaded);
    return loaded;
}

// ---------------------------------------------------------------------------
// CBDT/sbix color glyphs — the Skia-equivalent layer. cairo only renders
// glyphs as A8 alpha masks, so a CBDT font through show_glyphs comes out
// monochrome (or blank: bitmap-only faces reject cairo's scalable size
// request and FT_Load_Glyph fails with Unimplemented_Feature). Like Android's
// SkScalerContext we load the glyph with FT_LOAD_COLOR on a dedicated face
// and blit the BGRA bitmap as a scaled image; non-color glyphs stay on the
// normal mask path. The FreeType calls live here (not in paint.cc) so the
// cdtext static library keeps its symbol surface unchanged.
// ---------------------------------------------------------------------------

// Converted color-glyph surfaces, cached per (font, glyph): the strike pixels
// never change, so the BGRA->premultiplied-ARGB32 conversion runs once.
// Crude full eviction past the cap keeps memory bounded (136px glyphs ~74KB).
static std::unordered_map<uint64_t, Cairo::RefPtr<Cairo::ImageSurface>>& colorGlyphCache() {
    static std::unordered_map<uint64_t, Cairo::RefPtr<Cairo::ImageSurface>> cache;
    return cache;
}
static constexpr size_t MAX_COLOR_GLYPH_CACHE = 256;

// Dedicated FT_Face for color-bitmap loading, opened per font and cached for
// the process. cairo's locked face carries request state cairo owns (and in
// this build is additionally touched by a second FreeType instance pulled in
// through the debug vcpkg deps), which reliably leaves CBDT loading broken.
// A face of our own — bound to a strike right before each load — is immune.
static std::unordered_map<int32_t, FT_Face>& colorFaceCache() {
    static std::unordered_map<int32_t, FT_Face> cache;
    return cache;
}

// Memory-backed fonts (PAK @font) hand us a blob; keep our own copy so the
// FT_Face outlives the MinikinFont that provided it.
static std::unordered_map<int32_t, std::vector<uint8_t>>& colorFaceBlobs() {
    static std::unordered_map<int32_t, std::vector<uint8_t>> blobs;
    return blobs;
}

static FT_Face colorFaceFor(const minikin::MinikinFont* font) {
    if (font == nullptr) return nullptr;
    const int32_t id = font->GetSourceId();
    auto& cache = colorFaceCache();
    auto it = cache.find(id);
    if (it != cache.end()) return it->second;

    static FT_Library lib = nullptr;  // process-lifetime, never freed
    if (lib == nullptr) FT_Init_FreeType(&lib);

    FT_Face face = nullptr;
    if (!font->GetFontPath().empty()) {
        FT_New_Face(lib, font->GetFontPath().c_str(), font->GetFontIndex(), &face);
    }
    if (face == nullptr && font->GetFontData() != nullptr && font->GetFontSize() > 0) {
        auto blobIt = colorFaceBlobs().emplace(id,
                std::vector<uint8_t>((const uint8_t*)font->GetFontData(),
                                     (const uint8_t*)font->GetFontData() + font->GetFontSize()));
        FT_New_Memory_Face(lib, blobIt.first->second.data(),
                           (FT_Long)blobIt.first->second.size(), font->GetFontIndex(), &face);
    }
    if (face == nullptr || !FT_HAS_COLOR(face)) {
        if (face != nullptr) FT_Done_Face(face);
        cache.emplace(id, nullptr);  // negative entry: not a color font
        return nullptr;
    }
    cache.emplace(id, face);
    return face;
}

static Cairo::RefPtr<Cairo::ImageSurface> colorGlyphSurface(int32_t fontId, FT_UInt glyphIndex,
        FT_Face face) {
    const uint64_t key = ((uint64_t)(uint32_t)fontId << 32) | glyphIndex;
    auto& cache = colorGlyphCache();
    auto it = cache.find(key);
    if (it != cache.end()) return it->second;

    const FT_Bitmap& bmp = face->glyph->bitmap;
    auto surf = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,
                                            bmp.width, bmp.rows);
    const uint8_t* src = bmp.buffer;  // BGRA, straight (non-premultiplied) alpha
    uint8_t* dst = surf->get_data();
    for (unsigned row = 0; row < bmp.rows; row++) {
        for (unsigned col = 0; col < bmp.width; col++) {
            const uint8_t b = src[0], g = src[1], r = src[2], a = src[3];
            *dst++ = (uint8_t)((b * a + 127) / 255);  // premultiplied B
            *dst++ = (uint8_t)((g * a + 127) / 255);
            *dst++ = (uint8_t)((r * a + 127) / 255);
            *dst++ = a;
            src += 4;
        }
        src += bmp.pitch - (int)(bmp.width * 4);
    }
    surf->mark_dirty();
    if (cache.size() >= MAX_COLOR_GLYPH_CACHE) cache.clear();
    cache.emplace(key, surf);
    return surf;
}

const ColorGlyph* getColorGlyph(const minikin::MinikinFont* font,
        uint32_t glyphIndex, double textSize) {
    FT_Face face = colorFaceFor(font);
    if (face == nullptr) return nullptr;
    // Bitmap-only (CBDT) faces cannot honor a scalable size request: FreeType
    // keeps the strike metrics but stays in scalable load mode, and
    // FT_Load_Glyph then fails with Unimplemented_Feature (it refuses to
    // scale bitmap strikes). Bind the size to the strike closest to the text
    // size before loading.
    if (!(face->face_flags & FT_FACE_FLAG_SCALABLE) && face->num_fixed_sizes > 0) {
        int best = 0;
        long bestDiff = labs((long)face->available_sizes[0].height - (long)textSize);
        for (int i = 1; i < face->num_fixed_sizes; i++) {
            const long diff = labs((long)face->available_sizes[i].height - (long)textSize);
            if (diff < bestDiff) {
                bestDiff = diff;
                best = i;
            }
        }
        FT_Select_Size(face, best);
    }
    if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_COLOR) != 0) return nullptr;
    const FT_GlyphSlot slot = face->glyph;
    if (slot->format != FT_GLYPH_FORMAT_BITMAP
            || slot->bitmap.pixel_mode != FT_PIXEL_MODE_BGRA) {
        return nullptr;
    }
    // The bitmap is at the strike's pixel size; report placement pre-scaled
    // to the requested text size (the surface itself stays at strike
    // resolution and the caller scales it when painting).
    const double strike = face->size->metrics.y_ppem;
    const double scale = (strike > 0) ? textSize / strike : 1.0;
    auto surf = colorGlyphSurface(font->GetSourceId(), glyphIndex, face);
    static thread_local ColorGlyph out;
    out.surface = surf;
    out.left = slot->bitmap_left * scale;
    out.top = -(double)slot->bitmap_top * scale;
    out.scale = scale;
    return &out;
}

}
