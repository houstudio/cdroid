// Legacy AssetManager closed-loop test: addAssetPath + open/openNonAsset/openDir
// + ResTable wiring against the real aapt2 apks in testdata/_gen. Exercises the
// Asset/AssetDir/AssetManager port end to end (zip backend via libzip).

#include "assetmanager.h"

#include <memory>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>

using namespace cdroid;
using cdroid::ResTable;
using cdroid::ResTable_config;
using cdroid::ResXMLTree;
using cdroid::ResXMLParser;
using cdroid::NO_ERROR;

static std::string tpath(const std::string& s) {
    return std::string(ANDROIDFW_TESTDATA) + "/" + s;
}

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { \
    std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

int main() {
    // --- addAssetPath + path iteration + ResTable wiring (arsc apk) ---
    AssetManager am;
    int32_t cookie = 0;
    const std::string arscApk = tpath("_gen/arsc/out.apk");
    CHECK(am.addAssetPath(arscApk, &cookie));
    CHECK(cookie > 0);
    CHECK(am.nextAssetPath(0) == cookie);
    CHECK(am.getAssetPath(cookie) == arscApk);
    CHECK(am.nextAssetPath(cookie) == -1);

    const ResTable& rt = am.getResources();
    const uint32_t helloId = rt.getIdentifier("hello", "string", "com.example.restbl");
    CHECK(helloId != 0);   // resources.arsc parsed + queryable through the wrapper

    // --- openNonAsset("resources.arsc") ---
    std::unique_ptr<Asset> arscAsset(am.openNonAsset("resources.arsc", Asset::ACCESS_BUFFER));
    CHECK(arscAsset != nullptr);
    CHECK(arscAsset->getLength() > 0);
    CHECK(arscAsset->getBuffer(false) != nullptr);
    arscAsset.reset();   // free before the global-accounting check at end of main

    // --- openNonAsset("AndroidManifest.xml") -> ResXMLTree root <manifest> ---
    {
        std::unique_ptr<Asset> man(am.openNonAsset("AndroidManifest.xml", Asset::ACCESS_RANDOM));
        CHECK(man != nullptr);
        ResXMLTree tree;
        CHECK(tree.setTo(man->getBuffer(false), (size_t)man->getLength()) == NO_ERROR);
        bool foundManifest = false;
        ResXMLParser::event_code_t ev;
        while ((ev = tree.next()) != ResXMLParser::END_DOCUMENT) {
            if (ev == ResXMLParser::START_TAG) {
                size_t len = 0;
                const char16_t* name = tree.getElementName(&len);
                if (name) foundManifest = (std::u16string(name, len) == u"manifest");
                break;
            }
            if (ev == ResXMLParser::BAD_DOCUMENT) break;
        }
        CHECK(foundManifest);
    }

    // --- a compressed-flavored read: read()/seek() over an asset ---
    {
        std::unique_ptr<Asset> arsc2(am.openNonAsset("resources.arsc", Asset::ACCESS_STREAMING));
        CHECK(arsc2 != nullptr);
        char head[4] = {};
        ssize_t n = arsc2->read(head, 4);
        CHECK(n == 4);
        // resources.arsc begins with a RES_TABLE_TYPE chunk: bytes 0..1 = 0x0002 (LE).
        CHECK((uint8_t)head[0] == 0x02 && (uint8_t)head[1] == 0x00);
        off64_t pos = arsc2->seek(0, SEEK_SET);
        CHECK(pos == 0);
    }

    // --- axml apk: non-asset directory listing + a layout file ---
    {
        AssetManager am2;
        int32_t c2 = 0;
        const std::string axmlApk = tpath("_gen/axml/out.apk");
        CHECK(am2.addAssetPath(axmlApk, &c2));
        std::unique_ptr<AssetDir> dir(am2.openNonAssetDir(c2, "res/layout"));
        CHECK(dir != nullptr);
        bool hasXml = false;
        for (size_t i = 0; i < dir->getFileCount(); i++) {
            if (dir->getFileName(i).find(".xml") != std::string::npos) hasXml = true;
        }
        CHECK(hasXml);

        std::unique_ptr<Asset> lay(am2.openNonAsset("res/layout/test.xml", Asset::ACCESS_BUFFER));
        CHECK(lay != nullptr);
        CHECK(lay->getLength() > 0);
        // Binary AXML begins with RES_XML_TYPE chunk: bytes 0..1 = 0x0003 (LE).
        const uint8_t* b = (const uint8_t*)lay->getBuffer(false);
        CHECK(b != nullptr);
        CHECK(b[0] == 0x03 && b[1] == 0x00);
    }

    // --- setResTable injection: share a host-owned table so getResources()
    // returns it verbatim (no re-parse of resources.arsc) and the AssetManager
    // does NOT take ownership — the borrowed table outlives the manager. This is
    // the cdroid::Assets path (Assets::mResTable shared with its AssetManager). ---
    {
        ResTable prebuilt;
        {
            std::unique_ptr<Asset> arsc(am.openNonAsset("resources.arsc", Asset::ACCESS_BUFFER));
            CHECK(arsc != nullptr);
            CHECK(prebuilt.add(arsc->getBuffer(false), (size_t)arsc->getLength(),
                               /*cookie*/-1, /*copyData*/true) == NO_ERROR);
        }
        const uint32_t helloDirect = prebuilt.getIdentifier("hello", "string", "com.example.restbl");
        CHECK(helloDirect != 0);
        CHECK(helloDirect == helloId);   // equivalent to the manager-built table

        {
            AssetManager am3;
            int32_t c3 = 0;
            CHECK(am3.addAssetPath(arscApk, &c3));
            am3.setResTable(&prebuilt);               // borrowed
            // getResources() returns the SAME table object — no second parse.
            CHECK(&am3.getResources() == &prebuilt);
            CHECK(am3.getResources().getIdentifier("hello", "string",
                                                   "com.example.restbl") == helloDirect);
            // Asset paths are still registered for file opening.
            std::unique_ptr<Asset> again(am3.openNonAsset("resources.arsc", Asset::ACCESS_BUFFER));
            CHECK(again != nullptr);
        }   // ~am3 must NOT delete the borrowed table

        // The host table survived the AssetManager (no double-free / UAF).
        CHECK(prebuilt.getIdentifier("hello", "string", "com.example.restbl") == helloDirect);
    }

    // --- setConfiguration / getConfiguration round-trip + getLocales ---
    {
        ResTable_config cfg;
        memset(&cfg, 0, sizeof(cfg));
        cfg.size = sizeof(cfg);
        am.setConfiguration(cfg);
        ResTable_config out;
        memset(&out, 0, sizeof(out));
        am.getConfiguration(&out);
        CHECK(out.size == cfg.size);

        std::vector<std::string> locales;
        am.getLocales(&locales);
        CHECK(!locales.empty());   // arsc declares default / zh / es
    }

    // --- Asset global accounting ---
    CHECK(Asset::getGlobalCount() == 0);   // all assets above went out of scope

    if (failures == 0) {
        std::printf("OK assetmanager\n");
        return 0;
    }
    std::fprintf(stderr, "%d check(s) failed\n", failures);
    return 1;
}
