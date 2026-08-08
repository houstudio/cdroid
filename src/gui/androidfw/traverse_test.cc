// The single canonical ResTable test: load an arsc, traverse EVERY resource
// (listAllResources) and output its full info — id, type/key, and either the
// resolved simple value or the bag entries — then demonstrate config selection
// (locale) on a variant resource. Inline asserts pin the fidelity facts.
//
// Fixture arsc_verify_fixture.h (kARSV) was built by aapt2 with:
//   array/names   = [Alice, Bob, Carol]                       (bag)
//   string/chain  = @string/greeting -> @string/hello          (reference chain)
//   string/greeting = @string/hello
//   string/hello  = Default | zh=你好 | es=Hola | en=English | en-rUS=American | en-rGB=British | fil=Filipino
//   style/AppStyle = parent 0x01030012 + {0x01010095=16sp, 0x01010098=#ffff0000}  (bag)
#include "resourcetypes.h"
#include "arsc_verify_fixture.h"
#include "LocaleData.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using cdroid::ResTable;
using cdroid::ResTable_config;
using cdroid::Res_value;
using cdroid::ResTable_map;
using cdroid::dtohl;

static int g_failures = 0, g_checks = 0;
static void CHECK(bool cond, const char* expr, int line) {
    ++g_checks;
    if (!cond) { ++g_failures; std::fprintf(stderr, "FAIL %d: %s\n", line, expr); }
}
#define C(x) CHECK((x), #x, __LINE__)

// Render a Res_value as a short diagnostic string.
static std::string valStr(const ResTable& t, const Res_value& v) {
    char buf[64];
    switch (v.dataType) {
        case Res_value::TYPE_STRING: {
            size_t len = 0; const char16_t* s = t.getStringPool().stringAt(v.data, &len);
            std::string out; for (size_t i = 0; s && i < len; i++) out += (char)s[i];
            return "\"" + out + "\"";
        }
        case Res_value::TYPE_REFERENCE: std::snprintf(buf, sizeof(buf), "@0x%08x", v.data); return buf;
        case Res_value::TYPE_INT_DEC:   std::snprintf(buf, sizeof(buf), "%d", (int)v.data); return buf;
        case Res_value::TYPE_INT_HEX:   std::snprintf(buf, sizeof(buf), "0x%x", v.data); return buf;
        case Res_value::TYPE_INT_BOOLEAN: return v.data ? "true" : "false";
        case Res_value::TYPE_INT_COLOR_RGB8: std::snprintf(buf, sizeof(buf), "#%06x", v.data); return buf;
        case Res_value::TYPE_DIMENSION: {
            uint32_t unit = (v.data >> Res_value::COMPLEX_UNIT_SHIFT) & Res_value::COMPLEX_UNIT_MASK;
            const char* u = unit==Res_value::COMPLEX_UNIT_SP?"sp":unit==Res_value::COMPLEX_UNIT_DIP?"dp":"?";
            std::snprintf(buf, sizeof(buf), "%d%s",
                          (int)((v.data >> Res_value::COMPLEX_MANTISSA_SHIFT) & Res_value::COMPLEX_MANTISSA_MASK), u);
            return buf;
        }
        default: std::snprintf(buf, sizeof(buf), "(type 0x%02x data 0x%x)", v.dataType, v.data); return buf;
    }
}

// Build a request locale config with computed script (AssetManager-style).
static ResTable_config locCfg(const char* lang, const char* region = nullptr) {
    ResTable_config c; memset(&c, 0, sizeof(c)); c.size = sizeof(c);
    if (lang && lang[0]) {
        c.packLanguage(lang);
        if (region && region[0]) c.packRegion(region);
        char sc[4] = {0,0,0,0}; android::localeDataComputeScript(sc, c.language, c.country);
        memcpy(c.localeScript, sc, 4); c.localeScriptWasComputed = true;
    }
    return c;
}

static std::string helloUnder(ResTable& t, const ResTable_config& cfg) {
    t.setParameters(&cfg);
    size_t len = 0; const char16_t* s = t.getResourceString(0x7f020002, &len);
    std::string out; for (size_t i = 0; s && i < len; i++) out += (char)s[i];
    return out;
}

int main() {
    ResTable t;
    C(t.add(kARSV, kARSVLen) == cdroid::NO_ERROR);

    // ----- Traverse every resource and output its info -----
    std::vector<ResTable::ResourceRef> all;
    size_t n = t.listAllResources(&all);
    C(n == 5);
    std::printf("=== %zu resources ===\n", n);

    ResTable_config dflt; memset(&dflt, 0, sizeof(dflt)); dflt.size = sizeof(dflt);
    t.setParameters(&dflt);

    for (const auto& r : all) {
        std::printf("0x%08x  %s/%s\n", r.resId, r.type.c_str(), r.key.c_str());
        // Try as a simple value first.
        Res_value v; uint32_t specFlags = 0; ResTable_config matched;
        ssize_t blk = t.getResource(r.resId, &v, true, 0, &specFlags, &matched);  // mayBeBag: silence log
        if (blk >= 0) {
            std::printf("    value = %s   (specFlags=0x%x)\n", valStr(t, v).c_str(), specFlags);
        } else {
            // Complex (bag): array / style / plurals.
            size_t cnt = 0; const ResTable_map* map = t.getBag(r.resId, &cnt, &matched);
            std::printf("    bag: %zu entries\n", cnt);
            for (size_t i = 0; map && i < cnt; i++) {
                Res_value mv; mv.copyFrom_dtoh(map[i].value);
                std::printf("      [0x%08x] = %s\n", dtohl(map[i].name.ident), valStr(t, mv).c_str());
            }
        }
    }

    // ----- Fidelity assertions (pinned from aapt2 ground truth) -----
    // Resource set + ids.
    C(all[0].resId == 0x7f010000 && all[0].type == "array" && all[0].key == "names");
    C(all[2].resId == 0x7f020001 && all[2].key == "greeting");
    C(all[4].resId == 0x7f030000 && all[4].type == "style");

    // Locale selection on string/hello (variants: default/fil/en/en-rUS/en-rGB).
    C(helloUnder(t, locCfg(nullptr)) == "Default");
    C(helloUnder(t, locCfg("en")) == "English");
    C(helloUnder(t, locCfg("en", "US")) == "American");
    C(helloUnder(t, locCfg("en", "GB")) == "British");
    C(helloUnder(t, locCfg("fil", nullptr)) == "Filipino");
    C(helloUnder(t, locCfg("fr")) == "Default");                          // fallback

    // Reference chain resolves through the current locale.
    ResTable_config enReq = locCfg("en");
    t.setParameters(&enReq);
    size_t gl = 0; const char16_t* g = t.getResourceString(0x7f020000, &gl); // chain->greeting->hello
    std::string gs; for (size_t i = 0; g && i < gl; i++) gs += (char)g[i];
    C(gs == "English");

    // Style bag: parent + 2 items sorted by name resid.
    t.setParameters(&dflt);
    size_t sc = 0; const ResTable_map* smap = t.getBag(0x7f030000, &sc);
    C(sc == 2);
    C(dtohl(smap[0].name.ident) == 0x01010095u);   // textSize
    C(dtohl(smap[1].name.ident) == 0x01010098u);   // textColor
    Res_value tc; tc.copyFrom_dtoh(smap[1].value);
    C(tc.dataType == Res_value::TYPE_INT_COLOR_RGB8 && tc.data == 0xFFFF0000u);

    // String-array bag: 3 string values.
    size_t nc = 0; const ResTable_map* nmap = t.getBag(0x7f010000, &nc);
    C(nc == 3);

    // density override parameter on getResource (AOSP parity).
    Res_value v; uint16_t savedDensity = 0; (void)savedDensity;
    C(t.getResource(0x7f020002, &v) >= 0);

    std::printf("\ntraverse: %d checks, %d failures\n", g_checks, g_failures);
    return g_failures ? 1 : 0;
}
