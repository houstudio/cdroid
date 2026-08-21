// androidfw_resdemo — integration sample: drives cdroid::ResourcesImpl (the androidfw
// ID-based facade) inside the cdroid runtime, and verifies cdroid::App exposes the
// AOSP Context resource face (app.getResources()).
//
// Two demos:
//   1. Standalone: cdroid::AssetManager + cdroid::ResourcesImpl over the testdata
//      arsc apk — value methods (getString/getInteger/getResourceName/getValue).
//   2. App integration: app.getResources() (cdroid::Resources, built lazily from
//      App's paks) — getAssets() wired, ID-based getters available.
//
// Usage: androidfw_resdemo [path/to/an.apk]
//   default apk = the androidfw arsc testdata apk (string/hello, integer/grid).

#include <cdroid.h>
#include <cdlog.h>

#include <androidfw/resourcesimpl.h>  // cdroid::ResourcesImpl
#include <androidfw/assetmanager.h>   // cdroid::AssetManager

#include <string>
#include <vector>
#include <cstdio>

int main(int argc, const char* argv[]) {
    App app(argc, argv);

    // Locate the apk: arg, else the androidfw arsc testdata apk.
    std::string apkPath = (argc > 1) ? argv[1]
        : "../src/gui/androidfw/testdata/_gen/arsc/out.apk";
    if (argc <= 1) {
        FILE* fp = fopen(apkPath.c_str(), "rb");
        if (!fp) apkPath = "src/gui/androidfw/testdata/_gen/arsc/out.apk";
        else fclose(fp);
    }

    Window* w = new Window(0, 0, 720, 480);
    LinearLayout* root = new LinearLayout(&App::getInstance());
    root->setOrientation(LinearLayout::VERTICAL);
    w->addView(root);

    auto addLine = [&](const std::string& s) {
        TextView* tv = new TextView(&App::getInstance()); tv->setText(s);
        root->addView(tv);
    };
    char buf[256];

    // --- Demo 1: standalone cdroid::ResourcesImpl over a known apk ---
    addLine("== cdroid::ResourcesImpl over: " + apkPath + " ==");
    static const char* kPkg = "com.example.restbl";
    cdroid::AssetManager am;
    int32_t cookie = 0;
    if (am.addAssetPath(apkPath, &cookie)) {
        cdroid::ResourcesImpl res(&am);
        const int helloId = res.getIdentifier("hello", "string", kPkg);
        const int gridId  = res.getIdentifier("grid", "integer", kPkg);
        snprintf(buf, sizeof(buf), "getIdentifier(hello) = 0x%08x", helloId); addLine(buf);
        std::string name;
        if (res.getResourceName(helloId, &name)) addLine("getResourceName = " + name);
        addLine("getString(hello) = " + res.getString(helloId));
        snprintf(buf, sizeof(buf), "getInteger(grid) = %d", res.getInteger(gridId)); addLine(buf);
        cdroid::TypedValue tv;
        if (res.getValue(helloId, &tv, true)) {
            snprintf(buf, sizeof(buf), "getValue(hello): type=0x%02x data=0x%08x", tv.type, tv.data);
            addLine(buf);
        }
    } else {
        addLine("addAssetPath failed for " + apkPath);
    }

    // --- Demo 2: App integration — cdroid::Context::getResources() ---
    addLine("== App.getResources() (cdroid::Context ID face) ==");
    cdroid::Resources& appRes = app.getResources();   // cdroid::Resources (GUI subclass), lazily built
    cdroid::AssetManager* appAm = appRes.getAssets();
    std::string pkgs;
    if (appAm != nullptr) {
        for (const auto& p : appAm->getResources().listPackageNames()) { pkgs += p; pkgs += " "; }
    }
    addLine("App Resources packages: " + (pkgs.empty() ? std::string("(none / text-mode pak)") : pkgs));
    addLine("App now exposes the AOSP Context face: getResources/getAssets/getString(int)/...");

    // --- Demo 3: Theme + obtainStyledAttributes (AOSP ID face) ---
    addLine("== App.getTheme() + obtainStyledAttributes ==");
    const int colorPrimaryAttr = app.getId("cdroid:attr/colorPrimary");
    snprintf(buf, sizeof(buf), "attr colorPrimary id = 0x%08x", (unsigned)colorPrimaryAttr);
    addLine(buf);
    if (colorPrimaryAttr) {
        // Single themed attribute via Theme.resolveAttribute (AOSP API).
        cdroid::ResTable::Theme& theme = app.getTheme();
        cdroid::Res_value rv;
        if (theme.resolveAttribute((uint32_t)colorPrimaryAttr, &rv, true)) {
            snprintf(buf, sizeof(buf), "theme.colorPrimary = 0x%08x (type 0x%02x)", rv.data, rv.dataType);
            addLine(buf);
        } else {
            addLine("colorPrimary not resolved in theme");
        }
        // Bulk resolve via the ID-based obtainStyledAttributes(vector<int>).
        auto ta = app.obtainStyledAttributes(std::vector<int>{ colorPrimaryAttr });
        if (ta && ta->hasValue(0)) {
            snprintf(buf, sizeof(buf), "obtainStyledAttributes[colorPrimary].getColor = 0x%08x", ta->getColor(0, 0));
            addLine(buf);
        } else {
            addLine("obtainStyledAttributes returned no value");
        }
    }
    // Resources.newTheme(): a fresh (empty) theme over the same table.
    auto freshTheme = appRes.newTheme();
    snprintf(buf, sizeof(buf), "Resources.newTheme() = %s", freshTheme ? "ok" : "null");
    addLine(buf);

    return app.exec();
}
