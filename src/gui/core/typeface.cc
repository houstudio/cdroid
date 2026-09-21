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

// ---------------------------------------------------------------------------
// FontData — the AOSP Font.createBuffer analog: the font bytes as a read-only
// mapping held for the lifetime of every face reading them.
//   - pak fonts: the opened Asset IS the mapping. STORED entries hand
//     getBuffer() a view straight into the archive window (the fc.map
//     success path); a compressed entry keeps its one-time inflated copy
//     inside the Asset (the stream-read ByteBuffer fallback).
//   - file fonts (fonts.xml): a whole-file mmap window (AOSP Builder(File)).
// getBuffer(false) everywhere: FreeType's memory-face reads are byte-wise, so
// _MappedAsset's 4-byte-aligned heap-copy fallback is never taken.
// The bytes back FT_New_Memory_Face faces, which read LAZILY during
// rasterization — the holder must outlive them, hence the process-lifetime
// pin (the same retention class as sSystemFontFaces itself).
// ---------------------------------------------------------------------------
class FontData {
public:
    // Whole-file mmap (AOSP Builder(File) fc.map). openWindow maps the fd but
    // does not take it over; the mapping outlives the closed descriptor.
    // One FontData per file path (AOSP SkTypeface cache semantics): fonts.xml
    // lists the same file under several families/weights, and they share the
    // mapping instead of opening a window each.
    static std::shared_ptr<FontData> fromFile(const std::string& path) {
        static std::unordered_map<std::string, std::shared_ptr<FontData>> byPath;
        auto it = byPath.find(path);
        if (it != byPath.end()) return it->second;
        const int fd = ::open(path.c_str(), O_RDONLY);
        if (fd < 0) return nullptr;
        struct stat st;
        if (fstat(fd, &st) != 0 || st.st_size <= 0) {
            ::close(fd);
            return nullptr;
        }
        std::unique_ptr<Asset> asset(Asset::createFromMappedWindow(
                fd, path.c_str(), 0, (size_t)st.st_size, Asset::ACCESS_BUFFER));
        ::close(fd);
        auto out = fromAsset(std::move(asset));
        if (out) byPath.emplace(path, out);
        return out;
    }
    // Adopt an opened Asset (pak window / directory provider window /
    // inflated heap). Takes ownership: the Asset lives as long as the blob.
    static std::shared_ptr<FontData> fromAsset(std::unique_ptr<Asset> asset) {
        if (asset == nullptr) return nullptr;
        const void* data = asset->getBuffer(false);
        const size_t size = (size_t)asset->getLength();
        if (data == nullptr || size == 0) return nullptr;
        auto out = std::shared_ptr<FontData>(new FontData);
        out->mAsset = std::move(asset);
        out->mData = data;
        out->mSize = size;
        keepAlive().push_back(out);
        return out;
    }
    const void* data() const { return mData; }
    size_t size() const { return mSize; }
private:
    static std::vector<std::shared_ptr<FontData>>& keepAlive() {
        static std::vector<std::shared_ptr<FontData>> pinned;
        return pinned;
    }
    FontData() = default;
    std::unique_ptr<Asset> mAsset;   // owns the mmap window / inflated buffer
    const void* mData = nullptr;
    size_t mSize = 0;
};

class FullMinikinFont : public minikin::MinikinFont {
public:
    // One variant for every source: the bytes always come from `fontData`
    // (the Typeface-held FontData — pak window / file mmap / inflated heap);
    // the shared_ptr keeps the mapping alive for the faces' lazy reads.
    // `filePath` is informational (registry grouping, GetFontPath); asset
    // fonts pass it empty, matching the old memory-variant shape.
    FullMinikinFont(const Cairo::RefPtr<Cairo::FtFontFace>& fontFace,
                    std::shared_ptr<FontData> fontData,
                    const std::string& filePath = {}, int faceIndex = 0)
        : mFontFace(fontFace), mFilePath(filePath), mFaceIndex(faceIndex),
          mFontData(std::move(fontData)) {
        mSourceId = mFontId++;
        mData = mFontData ? mFontData->data() : nullptr;
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



    // Global LRU cache for ScaledFonts, shared by all FullMinikinFont instances
    // Reduced from 64 to 32 for better cache locality
    static constexpr size_t MAX_SCALED_FONT_CACHE = 32;
    // LRU shape (android::LruCache-style): the list OWNS the {key, font} nodes and
    // defines the recency order; the map only holds iterators into the list. One
    // source of truth — the previous "map of values + parallel key list" could
    // desync (map erase no-ops / duplicate keys), leaving freed nodes linked in
    // either container (valgrind: invalid read/write of size 8 in _Hashtable /
    // _M_hook during heavy cache churn, e.g. IME typing through the candidate bar).
    struct ScaledFontLruEntry {
        ScaledFontKey key;
        Cairo::RefPtr<Cairo::FtScaledFont> font;
    };
    using ScaledFontLru = std::list<ScaledFontLruEntry>;
    static ScaledFontLru& getGlobalScaledFontLru() {
        static ScaledFontLru lru;
        return lru;
    }
    static std::unordered_map<ScaledFontKey, ScaledFontLru::iterator, ScaledFontKeyHash>& getGlobalScaledFontCache() {
        static std::unordered_map<ScaledFontKey, ScaledFontLru::iterator, ScaledFontKeyHash> cache;
        return cache;
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
        auto& lru = getGlobalScaledFontLru();
        auto it = cache.find(key);
        if (it != cache.end()) {
            // Move to front (most recently used); splice keeps node identity, so the
            // stored iterator stays valid.
            lru.splice(lru.begin(), lru, it->second);
            return it->second->font;
        }
        auto newFont = createScaledFont(paint.size, paint.scaleX, paint.skewX);
        // Evict least-recently-used when full (the old code froze the cache once full,
        // so every new (font,paint) combo after 32 entries was never cached).
        if (cache.size() >= MAX_SCALED_FONT_CACHE) {
            cache.erase(lru.back().key);
            lru.pop_back();
        }
        lru.push_front({key, newFont});
        cache.emplace(key, lru.begin());
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
    std::shared_ptr<FontData> mFontData;  // the blob (mData/mSize) owner
    const void* mData = nullptr;  // mFontData->data()
    size_t mSize = 0;
};
int FullMinikinFont::mFontId=0;

static FT_Library ftLibrary = nullptr;  // shared by the FT_New_Memory_Face paths (file + pak)

// The shared fallback chain: one FontFamily per unique font file, built once
// by buildSystemFallback() and reused by every Typeface's lazily-built
// FontCollection. This is the Android fallback model: a Typeface renders with
// [its primary family] followed by the shared chain, which supplies glyphs
// the primary lacks. fonts.xml carries clean family names and explicit
// weight/italic per <font>, so family identity is the font FILE — the
// fontconfig-era fuzzy name matching (isSameFamily/normalizeFamilyPart) is
// retired.
struct FallbackFamilyEntry {
    std::string name;
    std::string file;
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
    // Build [this family's primary FontFamily] + [shared fallback chain].
    const auto& chain = fallbackFamilies();
    int primary = -1;
    for (size_t i = 0; i < chain.size(); ++i) {
        if (chain[i].name == family) { primary = (int)i; break; }
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

cdroid::Context* Typeface::mContext;
std::vector<std::shared_ptr<Typeface>> Typeface::sSystemFontFaces;
std::unordered_map<std::string, std::shared_ptr<Typeface> > Typeface::sSystemFontMap;

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
// Shared face bootstrap for both ctors: cairo's for-ft-face wrapper does NOT
// take a reference, so the face is referenced once before the ctor's own Done
// drops it — registry typefaces stay alive for the process. The family falls
// back to the face's own name when the caller has none.
void Typeface::initFace(FT_Face ftFace, const std::string& family) {
    mFamily = family.empty()
            ? std::string(ftFace->family_name ? ftFace->family_name : "") : family;
    FT_Reference_Face(ftFace);
    mFontFace = Cairo::FtFontFace::create(ftFace, 0);
}

Typeface::Typeface(const std::string& family, int weight, bool italic,
                   const std::string& fileName, int faceIndex) {
    mFileName = fileName;
    mFaceIndex = faceIndex;
    mWeight = weight;
    mStyle = (weight >= 600 ? BOLD : 0) | (italic ? ITALIC : 0);
    if (ftLibrary == nullptr) FT_Init_FreeType(&ftLibrary);
    // AOSP Builder(File) fc.map: the whole file maps once (FontData) and the
    // bytes back the memory face — no FT stdio stream, no heap copy; only the
    // tables/glyph pages actually read get paged in.
    mFontData = FontData::fromFile(fileName);
    FT_Face ftFace = nullptr;
    // Font file missing/unreadable: keep the provided family, leave mFontFace null.
    if (!mFontData
            || FT_New_Memory_Face(ftLibrary, (const FT_Byte*)mFontData->data(),
                                  (FT_Long)mFontData->size(), faceIndex, &ftFace) || !ftFace) {
        mFontData.reset();  // don't pin a mapping nothing reads
        mFamily = family;
        return;
    }
    initFace(ftFace, family);
    FT_Done_Face(ftFace);
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
        // Negative cache: cache.emplace with nullptr keeps the miss, so a bad
        // path logs and pays the openNonAsset roundtrip exactly once instead
        // of once per TextView that carries the value.
        LOGW("%s: not found: %s", tag, path.c_str());
        cache.emplace(path, nullptr);
        return nullptr;
    }
    // AOSP Font.createBuffer: the STORED pak entry maps straight through (the
    // openFd + fc.map path); only a compressed entry carries the one-time
    // inflated copy inside the Asset. No bytes are copied here — the Typeface
    // (process cache) holds the window for the faces' lazy reads.
    auto fontData = FontData::fromAsset(std::unique_ptr<Asset>(asset));
    if (!fontData) {
        LOGW("%s: empty: %s", tag, path.c_str());
        return nullptr;
    }
    auto tf = std::shared_ptr<Typeface>(new Typeface("", 400 /* normal */, false, fontData, 0),
                                        Deleter{});
    cache[path] = tf;
    return tf.get();
}

// Memory-backed Typeface (PAK @font / R.font): the bytes live in the FontData
// mapping (pak window, or the Asset's inflated copy for a compressed entry).
// FT_New_Memory_Face reads lazily — mFontData keeps the window mapped for the
// face's (process) lifetime. The MinikinFont serves the same blob, so
// GetFontData() obeys the standard contract without needing a file path.
Typeface::Typeface(const std::string& family, int weight, bool italic,
                   std::shared_ptr<FontData> fontData, int faceIndex) {
    mWeight = weight;
    mStyle = (weight >= 600 ? BOLD : 0) | (italic ? ITALIC : 0);
    mFaceIndex = faceIndex;
    if (ftLibrary == nullptr) FT_Init_FreeType(&ftLibrary);
    if (!fontData || fontData->size() == 0) { mFamily = family; return; }
    mFontData = fontData;
    FT_Face ftFace = nullptr;
    if (FT_New_Memory_Face(ftLibrary, (const FT_Byte*)fontData->data(),
                           (FT_Long)fontData->size(), faceIndex, &ftFace) || !ftFace) {
        mFontData.reset();  // don't pin a mapping nothing reads
        mFamily = family;
        return;
    }
    initFace(ftFace, family);
    FT_Done_Face(ftFace);
    // Eagerly build the blob-backed MinikinFont so getMinikinFont() returns it
    // directly (the lazy path groups by file path, which asset fonts lack).
    auto ftFaceRef = std::dynamic_pointer_cast<Cairo::FtFontFace>(mFontFace);
    if (ftFaceRef) {
        mMinikinFont = std::make_shared<FullMinikinFont>(ftFaceRef, fontData, std::string(), faceIndex);
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

Cairo::RefPtr<Cairo::FontFace>Typeface::getFontFace()const {
    return mFontFace;
}

std::shared_ptr<minikin::MinikinFont> Typeface::getMinikinFont() const {
    if (!mMinikinFont && mFontFace) {
        auto ftFace = std::dynamic_pointer_cast<Cairo::FtFontFace>(mFontFace);
        if (ftFace) {
            const_cast<Typeface*>(this)->mMinikinFont =
                std::make_shared<FullMinikinFont>(ftFace, mFontData, mFileName, mFaceIndex);
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
    LOGV("typeface=%p family=%p name=%s style=[%x/%x] fontfile=%s",typeface,family,
       typeface->mFamily.c_str(),style,typeface->mStyle,typeface->mFileName.c_str());
    return typeface;
}

Typeface* Typeface::getSystemDefaultTypeface(const std::string& familyName) {
    // fonts.xml registry keys: file names, fonts.xml family names/aliases and
    // the faces' own family names — an exact lookup covers them all. (The
    // fontconfig-era scoring loop and the @font string mangling are retired;
    // app fonts load through Context::getFont / R.font instead.)
    auto it = sSystemFontMap.find(familyName);
    if (it != sSystemFontMap.end()) return it->second.get();
    for (auto& tf : sSystemFontFaces) {  // face family names may list "A;B"
        for (auto& name : TextUtils::split(tf->getFamily(), ";")) {
            if (name == familyName) return tf.get();
        }
    }
    return Typeface::DEFAULT;
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

std::shared_ptr<minikin::FontFamily>Typeface::buildFamily(const std::string&family,const std::vector<std::shared_ptr<Typeface>>&faces){
    std::vector<std::shared_ptr<minikin::Font>> fonts;
    for(auto f:faces){
        if(f->mFamily == family){
            auto ft = std::dynamic_pointer_cast<Cairo::FtFontFace>(f->getFontFace());
            auto minikinFont = std::make_shared<FullMinikinFont>(ft, f->mFontData, f->mFileName, f->mFaceIndex);
            auto font = minikin::Font::Builder(minikinFont).build();
            fonts.push_back(font);
            LOGD("    ->[%d](%s): %s",fonts.size()-1,f->getFamily().c_str(),f->mFileName.c_str());
        }
    }
    return minikin::FontFamily::create(std::move(fonts));
}
void Typeface::buildSystemFallback() {
    // Build the shared fallback chain: one FontFamily per unique font FILE, in
    // fonts.xml order. These FontFamily objects — and their FullMinikinFont /
    // mmap — are built once and shared by every Typeface's lazily-built
    // FontCollection (see getFontCollection). No per-Typeface FontCollection
    // is built here. (A file listed under several <family> names used to
    // enter the chain once per name; file identity dedups it to one entry
    // and one mmap.)
    auto& chain = fallbackFamilies();
    chain.clear();
    for (auto& tf : sSystemFontFaces) {
        bool dup = false;
        for (auto& e : chain) {
            if (e.file == tf->mFileName) { dup = true; break; }
        }
        if (dup) continue;
        auto fam = buildFamily(tf->mFamily, sSystemFontFaces);
        if (fam && fam->getNumFonts() > 0) {
            chain.push_back({tf->mFamily, tf->mFileName, fam});
            LOGD("fallback family[%zu]: %s (%s)", chain.size() - 1,
                 tf->mFamily.c_str(), tf->mFileName.c_str());
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
    // Walk up until the filesystem root (the old <4 hops missed executables
    // nested deeper in the out tree, e.g. src/gui/app/espresso — 5 levels).
    for (; !dir.empty(); ) {
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
    // fallback chain). Try the explicitly-configured path, the build-tree snapshot
    // next to the executable, then common system locations.
    // There is NO fontconfig fallback anymore (enumeration retired): when no
    // candidate hits, the system font map stays empty and the warning below
    // points at build.sh/genfontsxml.sh, which snapshots the host font set
    // into <out>/fonts.xml at build time.
    bool loadedFromXml = false;
    std::vector<std::string> candidates;
    if (!sFontConfigXml.empty()) candidates.push_back(sFontConfigXml);
    /* $CDROID_FONTS_XML: explicit fonts.xml location, ahead of the search. */
    if (const char* env = getenv("CDROID_FONTS_XML")) candidates.push_back(env);
    candidates.push_back(findFontsXmlNearExecutable());
    /* System install locations, aligned with the shared-pak probe paths. */
    candidates.push_back("/usr/share/cdroid/fonts.xml");
    candidates.push_back("/opt/cdroid/fonts.xml");
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

}
