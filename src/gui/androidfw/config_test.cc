// Standalone unit test for ResTable_config — the multi-axis configuration
// matching engine (stage 3a). Verifies the selection logic that lets one
// resource id resolve to different values across locale (multi-language) and
// density (multi-resolution) variants, with no aapt2 dependency: configs are
// built by hand and the ResTable selection algorithm (filter by match(), pick
// the best by isBetterThan()) is replicated in bestOf().
#include "resourcetypes.h"
#include "LocaleData.h"

#include <cstdio>
#include <cstring>
#include <vector>

namespace {

int g_failures = 0;
int g_checks = 0;

#define CHECK(cond)                                                       \
    do {                                                                  \
        ++g_checks;                                                       \
        if (!(cond)) {                                                    \
            ++g_failures;                                                 \
            std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__,  \
                         #cond);                                          \
        }                                                                 \
    } while (0)

using cdroid::ResTable_config;

// A zeroed config with the correct struct size (all axes "any").
ResTable_config anyConfig() {
    ResTable_config c;
    memset(&c, 0, sizeof(c));
    c.size = sizeof(ResTable_config);
    return c;
}

// Build a config for a density bucket (multi-resolution).
ResTable_config densityConfig(uint16_t d) {
    ResTable_config c = anyConfig();
    c.density = d;
    return c;
}

// Set language[/region] on a config and compute its likely script, mirroring
// how AssetManager builds a request from a BCP-47 locale. Resources may leave
// the script empty (match() computes it on the fly).
void setLocale(ResTable_config& c, const char* lang, const char* region = nullptr) {
    c.packLanguage(lang);
    if (region && region[0]) c.packRegion(region);
    char script[4] = {0, 0, 0, 0};
    android::localeDataComputeScript(script, c.language, c.country);
    memcpy(c.localeScript, script, 4);
    c.localeScriptWasComputed = true;
}

ResTable_config localeConfig(const char* lang, const char* region = nullptr) {
    ResTable_config c = anyConfig();
    c.packLanguage(lang);
    if (region && region[0]) c.packRegion(region);
    return c; // resource: empty script, computed at match() time
}

// Replicate ResTable::getResource's selection: keep configs that match() the
// request, then choose the one that isBetterThan all others. Returns nullptr if
// none match.
const ResTable_config* bestOf(std::vector<ResTable_config>& configs,
                              const ResTable_config& req) {
    const ResTable_config* best = nullptr;
    for (ResTable_config& c : configs) {
        if (!c.match(req)) continue;
        if (best == nullptr || c.isBetterThan(*best, &req)) best = &c;
    }
    return best;
}

// ---- Multi-resolution (density) ----
void test_density() {
    std::vector<ResTable_config> buckets = {
        densityConfig(ResTable_config::DENSITY_LOW),    // 120 mdpi
        densityConfig(ResTable_config::DENSITY_HIGH),   // 240 hdpi
        densityConfig(ResTable_config::DENSITY_XHIGH),  // 320 xhdpi
    };

    // Exact bucket requested -> that bucket.
    CHECK(bestOf(buckets, densityConfig(ResTable_config::DENSITY_HIGH))->density
          == ResTable_config::DENSITY_HIGH);

    // Request higher than any available -> highest available (scale up).
    CHECK(bestOf(buckets, densityConfig(ResTable_config::DENSITY_XXHIGH))->density
          == ResTable_config::DENSITY_XHIGH);

    // Request lower than any available -> lowest available (scale down).
    {
        std::vector<ResTable_config> hi = {
            densityConfig(ResTable_config::DENSITY_HIGH),
            densityConfig(ResTable_config::DENSITY_XHIGH),
        };
        CHECK(bestOf(hi, densityConfig(ResTable_config::DENSITY_LOW))->density
              == ResTable_config::DENSITY_HIGH);
    }

    // Between two buckets with none exact (request hdpi, have mdpi + xhdpi):
    // scaling down from xhdpi is preferred over scaling up from mdpi.
    {
        std::vector<ResTable_config> two = {
            densityConfig(ResTable_config::DENSITY_LOW),   // 120
            densityConfig(ResTable_config::DENSITY_XHIGH), // 320
        };
        CHECK(bestOf(two, densityConfig(ResTable_config::DENSITY_HIGH))->density
              == ResTable_config::DENSITY_XHIGH);
    }
}

// ---- Multi-language (locale) ----
void test_locale() {
    std::vector<ResTable_config> langs = {
        anyConfig(),                 // default (no locale)
        localeConfig("en"),
        localeConfig("es"),
        localeConfig("zh", "CN"),    // zh-Hans-CN
        localeConfig("zh", "TW"),    // zh-Hant-TW
    };

    // Exact language+region.
    {
        ResTable_config req = anyConfig(); setLocale(req, "zh", "CN");
        const ResTable_config* b = bestOf(langs, req);
        CHECK(b != nullptr);
        char lang[4] = {0}, region[4] = {0};
        b->unpackLanguage(lang); b->unpackRegion(region);
        CHECK(lang[0] == 'z' && lang[1] == 'h' && region[0] == 'C' && region[1] == 'N');
    }

    // Language-only request "zh" computes the Hans script, so zh-CN (Hans)
    // matches and zh-TW (Hant) does not. Best is zh-CN.
    {
        ResTable_config req = anyConfig(); setLocale(req, "zh");
        const ResTable_config* b = bestOf(langs, req);
        CHECK(b != nullptr);
        char region[4] = {0}; b->unpackRegion(region);
        CHECK(region[0] == 'C' && region[1] == 'N'); // not TW
    }

    // "zh-TW" request (Hant) matches zh-TW, not zh-CN.
    {
        ResTable_config req = anyConfig(); setLocale(req, "zh", "TW");
        const ResTable_config* b = bestOf(langs, req);
        char region[4] = {0}; b->unpackRegion(region);
        CHECK(region[0] == 'T' && region[1] == 'W');
    }

    // Language with no matching resource falls back to the default (no locale).
    {
        ResTable_config req = anyConfig(); setLocale(req, "fr");
        const ResTable_config* b = bestOf(langs, req);
        CHECK(b != nullptr);
        CHECK(b->locale == 0); // default config
    }

    // A specific language beats the default for a matching request.
    {
        ResTable_config req = anyConfig(); setLocale(req, "en");
        const ResTable_config* b = bestOf(langs, req);
        char lang[4] = {0}; b->unpackLanguage(lang);
        CHECK(lang[0] == 'e' && lang[1] == 'n');
    }
}

// ---- match() filtering on a non-locale axis (orientation) ----
void test_match_filtering() {
    ResTable_config port = anyConfig(); port.orientation = ResTable_config::ORIENTATION_PORT;
    ResTable_config landReq = anyConfig(); landReq.orientation = ResTable_config::ORIENTATION_LAND;
    ResTable_config portReq = anyConfig(); portReq.orientation = ResTable_config::ORIENTATION_PORT;
    CHECK(!port.match(landReq)); // portrait resource doesn't match landscape request
    CHECK(port.match(portReq));
}

// ---- compare / diff sanity ----
void test_compare_diff() {
    ResTable_config a = densityConfig(ResTable_config::DENSITY_HIGH);
    ResTable_config b = densityConfig(ResTable_config::DENSITY_XHIGH);
    CHECK(a.diff(b) == ResTable_config::CONFIG_DENSITY);
    CHECK(a.compare(a) == 0);

    ResTable_config en = localeConfig("en");
    ResTable_config zh = localeConfig("zh");
    CHECK(en.diff(zh) == ResTable_config::CONFIG_LOCALE);
}

} // namespace

int main() {
    test_density();
    test_locale();
    test_match_filtering();
    test_compare_diff();

    std::printf("ResTable_config: %d checks, %d failures\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
