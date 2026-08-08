// Closed-loop demo for the resource table (stage 3): load a real aapt2
// resources.arsc and resolve a string across LOCALES and an integer across
// DENSITIES, printing the selected value and the matched configuration. This is
// the multi-language / multi-resolution end-to-end verification.
//
//   make -C outX64-Debug androidfw_resource_demo
//   ./outX64-Debug/src/gui/androidfw/androidfw_resource_demo
#include "resourcetypes.h"
#include "arsc_fixture.h"
#include "LocaleData.h"

#include <cstdio>
#include <cstring>

using cdroid::ResTable;
using cdroid::ResTable_config;
using cdroid::Res_value;

static const uint32_t kHello = 0x7f020000;  // string/hello
static const uint32_t kGrid  = 0x7f010000;  // integer/grid

static ResTable_config reqLocale(const char* lang, const char* region = nullptr) {
    ResTable_config c;
    memset(&c, 0, sizeof(c));
    c.size = sizeof(c);
    // packLanguage/packRegion read 3 chars unconditionally (AOSP behavior), so
    // only call them for non-empty input to avoid reading past the NUL.
    if (lang && lang[0]) c.packLanguage(lang);
    if (region && region[0]) c.packRegion(region);
    if (lang && lang[0]) {
        char script[4] = {0, 0, 0, 0};
        android::localeDataComputeScript(script, c.language, c.country);
        memcpy(c.localeScript, script, 4);
        c.localeScriptWasComputed = true;
    }
    return c;
}

static ResTable_config reqDensity(uint16_t d) {
    ResTable_config c;
    memset(&c, 0, sizeof(c));
    c.size = sizeof(c);
    c.density = d;
    return c;
}

// Render a config's locale + density compactly.
static void printConfig(const ResTable_config& c) {
    char loc[40];
    ResTable_config nc = c;
    nc.getBcp47Locale(loc);
    std::printf("locale=%-7s density=", loc[0] ? loc : "default");
    switch (c.density) {
        case ResTable_config::DENSITY_LOW:     std::printf("mdpi");  break;
        case ResTable_config::DENSITY_MEDIUM:  std::printf("mdpi(160)"); break;
        case ResTable_config::DENSITY_HIGH:    std::printf("hdpi");  break;
        case ResTable_config::DENSITY_XHIGH:   std::printf("xhdpi"); break;
        case ResTable_config::DENSITY_XXHIGH:  std::printf("xxhdpi"); break;
        default:                                std::printf("%u", c.density); break;
    }
}

static void resolveString(ResTable& t, const char* label, const ResTable_config& req) {
    t.setParameters(&req);
    ResTable_config matched;
    size_t len = 0;
    const char16_t* s = t.getResourceString(kHello, &len);
    std::printf("  [%s] request ", label);
    printConfig(req);
    std::printf("  ->  hello = \"");
    for (size_t i = 0; s && i < len; ++i) {
        uint32_t c = s[i];
        if (c >= 0xD800 && c <= 0xDBFF && i + 1 < len && s[i + 1] >= 0xDC00) c = 0x10000 + ((c - 0xD800) << 10) + (s[++i] - 0xDC00);
        if (c < 0x80) std::putchar((char)c);
        else if (c < 0x800) { std::putchar((char)(0xC0 | (c >> 6))); std::putchar((char)(0x80 | (c & 0x3F))); }
        else if (c < 0x10000) { std::putchar((char)(0xE0 | (c >> 12))); std::putchar((char)(0x80 | ((c >> 6) & 0x3F))); std::putchar((char)(0x80 | (c & 0x3F))); }
    }
    std::printf("\"");
    // Show which variant matched.
    Res_value v;
    if (t.getResource(kHello, &v, &matched) == cdroid::NO_ERROR) {
        std::printf("  (matched ");
        printConfig(matched);
        std::printf(")");
    }
    std::printf("\n");
}

static void resolveInt(ResTable& t, const char* label, const ResTable_config& req) {
    t.setParameters(&req);
    Res_value v; ResTable_config matched;
    std::printf("  [%s] request density=", label);
    switch (req.density) {
        case ResTable_config::DENSITY_LOW:    std::printf("mdpi"); break;
        case ResTable_config::DENSITY_MEDIUM: std::printf("mdpi(160)"); break;
        case ResTable_config::DENSITY_HIGH:   std::printf("hdpi"); break;
        case ResTable_config::DENSITY_XHIGH:  std::printf("xhdpi"); break;
        case ResTable_config::DENSITY_XXHIGH: std::printf("xxhdpi"); break;
        default: std::printf("%u", req.density); break;
    }
    if (t.getResource(kGrid, &v, &matched) == cdroid::NO_ERROR) {
        std::printf("  ->  grid = %d  (matched ", (int)v.data);
        printConfig(matched);
        std::printf(")\n");
    } else {
        std::printf("  ->  grid = NOT FOUND\n");
    }
}

int main() {
    ResTable table;
    if (table.add(kARS, kARSLen) != cdroid::NO_ERROR) {
        std::fprintf(stderr, "failed to load resources.arsc\n");
        return 1;
    }

    std::printf("=== Multi-language: @string/hello (default=Hello, zh=\xE4\xBD\xA0\xE5\xA5\xBD, es=Hola) ===\n");
    resolveString(table, "default", reqLocale(""));
    resolveString(table, "en",      reqLocale("en"));
    resolveString(table, "zh",      reqLocale("zh"));
    resolveString(table, "zh-CN",   reqLocale("zh", "CN"));
    resolveString(table, "zh-TW",   reqLocale("zh", "TW"));
    resolveString(table, "es",      reqLocale("es"));
    resolveString(table, "fr (no match -> default)", reqLocale("fr"));

    std::printf("\n=== Multi-resolution: @integer/grid (default=8, hdpi=12, xhdpi=16) ===\n");
    resolveInt(table, "mdpi",   reqDensity(ResTable_config::DENSITY_MEDIUM));
    resolveInt(table, "hdpi",   reqDensity(ResTable_config::DENSITY_HIGH));
    resolveInt(table, "xhdpi",  reqDensity(ResTable_config::DENSITY_XHIGH));
    resolveInt(table, "xxhdpi", reqDensity(ResTable_config::DENSITY_XXHIGH));

    std::printf("\nLocale + density combine independently: zh + xhdpi\n");
    ResTable_config both = reqLocale("zh");
    both.density = ResTable_config::DENSITY_XHIGH;
    resolveString(table, "zh+xhdpi", both);
    resolveInt(table, "zh+xhdpi", both);

    return 0;
}
