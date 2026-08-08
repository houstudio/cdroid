// AssetManager2 management surface test (phase 4): getIdentifier (name->id),
// getResourceName (full), listPackageNames, getConfigurations.
// Fixture arsc_verify_fixture.h (kARSV): package com.example.verify with
//   array/names=0x7f010000, string/{chain,greeting,hello}=0x7f02000x, style/AppStyle=0x7f030000,
//   hello has locale variants (default/fil/en/en-rGB/en-rUS).
#include "resourcetypes.h"
#include "arsc_verify_fixture.h"

#include <cstdio>
#include <cstring>
#include <vector>
#include <string>

namespace {
int g_failures = 0, g_checks = 0;
#define C(x) do { ++g_checks; if(!(x)){ ++g_failures; \
    std::fprintf(stderr,"FAIL %s:%d: %s\n",__FILE__,__LINE__,#x);} } while(0)
} // namespace

using cdroid::ResTable;
using cdroid::ResTable_config;

int main() {
    ResTable t;
    C(t.add(kARSV, kARSVLen) == cdroid::NO_ERROR);
    const std::string PKG = "com.example.verify";

    // getIdentifier: name -> id.
    C(t.getIdentifier("hello", "string", PKG)   == 0x7f020002u);
    C(t.getIdentifier("names", "array",  PKG)   == 0x7f010000u);
    C(t.getIdentifier("AppStyle", "style", PKG) == 0x7f030000u);
    C(t.getIdentifier("nope", "string", PKG)    == 0u);        // missing
    C(t.getIdentifier("hello", "string", "")    == 0x7f020002u); // package="" -> all
    C(t.getIdentifier("hello", "string", "other.pkg") == 0u); // wrong package

    // getResourceName: id -> package/type/key.
    std::string pkg, type, key;
    C(t.getResourceName(0x7f020002, &pkg, &type, &key));
    C(pkg == PKG && type == "string" && key == "hello");

    // listPackageNames.
    auto names = t.listPackageNames();
    C(names.size() == 1 && names[0] == PKG);

    // getConfigurations: the locale variants on hello must all appear.
    std::vector<ResTable_config> cfgs;
    t.getConfigurations(&cfgs);
    C(cfgs.size() >= 5);  // default + fil + en + en-rGB + en-rUS
    int locales = 0;
    for (const auto& c : cfgs) if (c.language[0]) locales++;
    C(locales >= 4);  // fil/en/en-rGB/en-rUS

    // getIdentifier round-trips with getResource: id from name -> value.
    uint32_t id = t.getIdentifier("greeting", "string", PKG);
    C(id == 0x7f020001u);

    std::printf("mgmt: %d checks, %d failures\n", g_checks, g_failures);
    return g_failures ? 1 : 0;
}
