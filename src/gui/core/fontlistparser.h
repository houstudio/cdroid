#ifndef __FONT_LIST_PARSER_H__
#define __FONT_LIST_PARSER_H__
#include <string>
#include <vector>
namespace cdroid{

// Minimal C++ port of android.graphics.FontListParser (android-36).
// Parses the <familyset> schema used by BOTH Android's legacy fonts.xml and the
// modern font_fallback.xml (Android 15+) — the schema is identical; only the file
// name differs. v1 omits OEM customization, updatable fonts, <family-list>, variant
// and fallbackFor; it keeps named families, fallback families (lang), per-font
// weight/style/index/axis, and aliases.

struct FontVariationAxis {
    std::string tag;          // 4-char OpenType axis tag, e.g. "wght"
    float styleValue = 0.f;
};

struct FontConfig {
    struct Font {
        std::string fontName;                 // filename text inside <font>, trimmed
        std::string fileName;                 // resolved: fontDir + fontName
        int weight = 400;                      // <font weight="...">
        bool italic = false;                  // <font style="italic">
        int index = 0;                         // TTC face index
        std::vector<FontVariationAxis> axes;  // <axis> children
    };
    struct Family {
        std::string name;   // non-empty => named family (sans-serif/serif/...); empty => fallback
        std::string lang;   // BCP47 lang tag, mainly for fallback families
        std::vector<Font> fonts;
    };
    struct Alias {
        std::string name;   // alias name, e.g. "sans-serif-medium"
        std::string to;     // target family name, e.g. "sans-serif"
        int weight = 0;
    };
    std::vector<Family> families;  // document order: named + fallback interleaved
    std::vector<Alias> aliases;
};

// Parse a fonts.xml / font_fallback.xml file. `fontDir` is prepended to each <font>
// filename to build an absolute path (mirrors Android's systemFontDir).
// Returns an empty FontConfig (no families) if the file can't be opened, so the caller
// can fall back to fontconfig discovery.
FontConfig parseFontConfig(const std::string& xmlPath, const std::string& fontDir);

} // namespace cdroid
#endif // __FONT_LIST_PARSER_H__
