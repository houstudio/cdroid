// ResourcesImpl isolated-port test: ID-based value/meta resolution over an
// AssetManager loaded with the arsc testdata apk (package com.example.restbl;
// string/hello = "Hello", integer/grid ∈ {8,12,16} across densities). Exercises
// every fully-implemented getter; GUI factories are asserted stubbed.

#include <core/resourcesimpl.h>
#include <core/asset.h>
#include <core/assetmanager.h>
#include <androidfw/resourcetypes.h>  // Res_value
#include <core/typedvalue.h>     // TypedValue, applyDimension

#include <memory>
#include <string>
#include <cstdio>

using namespace cdroid;
using cdroid::Res_value;

static std::string tpath(const std::string& s) {
    return std::string(ANDROIDFW_TESTDATA) + "/" + s;
}

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { \
    std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

int main() {
    AssetManager am;
    int32_t cookie = 0;
    CHECK(am.addAssetPath(tpath("_gen/arsc/out.apk"), &cookie));

    DisplayMetrics metrics;   // default density=1
    ResourcesImpl res(&am, nullptr, &metrics);

    static const char* kPkg = "com.example.restbl";

    // --- identifier + naming + string ---
    const int helloId = res.getIdentifier("hello", "string", kPkg);
    CHECK(helloId != 0);
    CHECK(res.getIdentifier("nope", "string", kPkg) == 0);

    std::string name;
    CHECK(res.getResourceName(helloId, &name) && name == std::string(kPkg) + ":string/hello");
    CHECK(res.getResourceTypeName(helloId, &name) && name == "string");
    CHECK(res.getResourceEntryName(helloId, &name) && name == "hello");
    CHECK(res.getResourcePackageName(helloId, &name) && name == kPkg);

    TypedValue tv;
    CHECK(res.getValue(helloId, &tv, true) && tv.type == Res_value::TYPE_STRING);
    CHECK(res.getString(helloId) == "Hello");
    CHECK(res.getText(helloId) == u"Hello");

    // --- integer across density variants (default config picks the base 8) ---
    const int gridId = res.getIdentifier("grid", "integer", kPkg);
    CHECK(gridId != 0);
    CHECK(res.getValue(gridId, &tv, true) && tv.type == Res_value::TYPE_INT_DEC);
    const int grid = res.getInteger(gridId);
    CHECK(grid == 8 || grid == 12 || grid == 16);   // any declared density variant

    // --- boolean (no bool resource in this apk; wrong-type returns default) ---
    // getBoolean on a string resource returns false (no crash, no throw).
    CHECK(res.getBoolean(helloId) == false);

    // --- color: no color resource; getColor returns 0 ---
    CHECK(res.getColor(helloId) == 0);

    // --- GUI stubs: forward-declared return types, bodies return nullptr ---
    CHECK(res.getDrawable(helloId) == nullptr);
    CHECK(res.getColorStateList(helloId) == nullptr);
    CHECK(res.getFont(helloId) == nullptr);

    // --- getXml on a non-file (string) resource returns nullptr gracefully ---
    {
        std::unique_ptr<Asset> xml(res.getXml(helloId));
        CHECK(xml == nullptr);
    }

    // --- applyDimension sanity: 10dip at density 2.0 -> 20px ---
    DisplayMetrics m2; m2.density = 2.0f;
    CHECK(applyDimension(Res_value::COMPLEX_UNIT_DIP, 10.0f, m2) == 20.0f);

    if (failures == 0) {
        std::printf("OK resources\n");
        return 0;
    }
    std::fprintf(stderr, "%d check(s) failed\n", failures);
    return 1;
}
