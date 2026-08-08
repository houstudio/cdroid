// Multi-package test (phase 1 capstone): load TWO arsc blobs that were BOTH
// compiled as the app package (0x7f). A is loaded normally (keeps 0x7f); B is
// loaded as a shared library (appAsLib=true) so it is reassigned runtime id 0x02
// and its internal @string/... self-references (build-time 0x7f) are translated
// by its DynamicRefTable to 0x02, where they then resolve. This is the
// soft-keyboard / IME add-on package scenario.
//
//   A: string/greeting=@string/hello, string/hello="Hi"
//   B: string/kb=@string/kb_val,      string/kb_val="Keyboard"
#include "resourcetypes.h"
#include "arsc_multipkg_A_fixture.h"
#include "arsc_multipkg_B_fixture.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace {
int g_failures = 0, g_checks = 0;
#define C(x) do { ++g_checks; if(!(x)){ ++g_failures; \
    std::fprintf(stderr,"FAIL %s:%d: %s\n",__FILE__,__LINE__,#x);} } while(0)
} // namespace

using cdroid::ResTable;
using cdroid::ResTable_config;

static std::string u8(const char16_t* s, size_t len) {
    std::string out;
    for (size_t i = 0; s && i < len; i++) {
        uint32_t c = s[i];
        if (c < 0x80) out += (char)c;
        else if (c < 0x800) { out += (char)(0xC0|(c>>6)); out += (char)(0x80|(c&0x3F)); }
        else { out += (char)(0xE0|(c>>12)); out += (char)(0x80|((c>>6)&0x3F)); out += (char)(0x80|(c&0x3F)); }
    }
    return out;
}

int main() {
    ResTable t;
    C(t.add(kARSA, kARSALen, /*cookie*/1, /*copyData*/false) == cdroid::NO_ERROR);
    // B loaded as a shared library -> its 0x7f is reassigned to 0x02.
    C(t.add(kARSB, kARSBLen, /*appAsLib*/true, /*cookie*/2, /*copyData*/false) == cdroid::NO_ERROR);

    ResTable_config dflt; memset(&dflt, 0, sizeof(dflt)); dflt.size = sizeof(dflt);
    t.setParameters(&dflt);

    // Enumerate — both packages must be present, B at the remapped runtime id 0x02.
    std::vector<ResTable::ResourceRef> all;
    t.listAllResources(&all);
    C(all.size() == (size_t)4);
    bool sawApp = false, sawLib = false;
    for (const auto& r : all) {
        if (r.packageId == 0x7f) sawApp = true;
        if (r.packageId == 0x02) sawLib = true;
    }
    C(sawApp);          // A kept its 0x7f
    C(sawLib);          // B was remapped to 0x02

    // A's reference resolves within package 0x7f (greeting -> hello -> "Hi").
    size_t l = 0;
    const char16_t* a = t.getResourceString(0x7f010000, &l);  // A greeting
    C(a && u8(a, l) == "Hi");

    // B's reference: build-time it is @0x7f010001 (kb_val), but B's
    // DynamicRefTable(0x02, appAsLib) rewrites it to 0x02010001, which resolves
    // to B's kb_val -> "Keyboard". This is the DynamicRefTable wiring proof.
    const char16_t* b = t.getResourceString(0x02010000, &l);  // B kb (remapped runtime id)
    C(b && u8(b, l) == "Keyboard");

    // B's raw kb value is a REFERENCE whose data was translated build->runtime;
    // getResource returns B's owning-asset blockIndex (1: A=0, B=1).
    cdroid::Res_value v;
    C(t.getResource(0x02010000, &v) == 1);              // blockIndex = B's header
    C(v.dataType == cdroid::Res_value::TYPE_REFERENCE);
    C(v.data == 0x02010001u);                            // translated to runtime 0x02

    std::printf("multipackage: %d checks, %d failures\n", g_checks, g_failures);
    return g_failures ? 1 : 0;
}
