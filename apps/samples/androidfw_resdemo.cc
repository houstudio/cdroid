// androidfw_resdemo — integration sample: drives android::Resources (the androidfw
// ID-based facade) inside the cdroid runtime, and verifies cdroid::App exposes the
// AOSP Context resource face (app.getResources()).
//
// Two demos:
//   1. Standalone: android::AssetManager + android::Resources over the testdata
//      arsc apk — value methods (getString/getInteger/getResourceName/getValue).
//   2. App integration: app.getResources() (cdroid::Resources, built lazily from
//      App's paks) — getAssets() wired, ID-based getters available.
//
// Usage: androidfw_resdemo [path/to/an.apk]
//   default apk = the androidfw arsc testdata apk (string/hello, integer/grid).

#include <cdroid.h>
#include <cdlog.h>

#include "resources.h"      // android::Resources
#include "assetmanager.h"   // android::AssetManager

#include <string>
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
    LinearLayout* root = new LinearLayout(720, 480);
    root->setOrientation(LinearLayout::VERTICAL);
    w->addView(root);

    auto addLine = [&](const std::string& s) {
        TextView* tv = new TextView(720, 40);
        tv->setText(s);
        root->addView(tv);
    };
    char buf[256];

    // --- Demo 1: standalone android::Resources over a known apk ---
    addLine("== android::Resources over: " + apkPath + " ==");
    static const char* kPkg = "com.example.restbl";
    android::AssetManager am;
    int32_t cookie = 0;
    if (am.addAssetPath(apkPath, &cookie)) {
        android::Resources res(&am);
        const int helloId = res.getIdentifier("hello", "string", kPkg);
        const int gridId  = res.getIdentifier("grid", "integer", kPkg);
        snprintf(buf, sizeof(buf), "getIdentifier(hello) = 0x%08x", helloId); addLine(buf);
        std::string name;
        if (res.getResourceName(helloId, &name)) addLine("getResourceName = " + name);
        addLine("getString(hello) = " + res.getString(helloId));
        snprintf(buf, sizeof(buf), "getInteger(grid) = %d", res.getInteger(gridId)); addLine(buf);
        android::TypedValue tv;
        if (res.getValue(helloId, &tv, true)) {
            snprintf(buf, sizeof(buf), "getValue(hello): type=0x%02x data=0x%08x", tv.type, tv.data);
            addLine(buf);
        }
    } else {
        addLine("addAssetPath failed for " + apkPath);
    }

    // --- Demo 2: App integration — cdroid::Context::getResources() ---
    addLine("== App.getResources() (cdroid::Context ID face) ==");
    android::Resources& appRes = app.getResources();   // cdroid::Resources, lazily built
    android::AssetManager* appAm = appRes.getAssets();
    std::string pkgs;
    if (appAm != nullptr) {
        for (const auto& p : appAm->getResources().listPackageNames()) { pkgs += p; pkgs += " "; }
    }
    addLine("App Resources packages: " + (pkgs.empty() ? std::string("(none / text-mode pak)") : pkgs));
    addLine("App now exposes the AOSP Context face: getResources/getAssets/getString(int)/...");

    return app.exec();
}
