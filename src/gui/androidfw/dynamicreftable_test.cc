// Standalone test for DynamicRefTable — build-time -> run-time package id
// translation. Mirrors the cases in AOSP DynamicRefTable_test.cpp.
#include "resourcetypes.h"

#include <cstdio>

namespace {
int g_failures = 0, g_checks = 0;
#define CHECK(cond) do { ++g_checks; if(!(cond)){ ++g_failures; \
    std::fprintf(stderr,"FAIL %s:%d: %s\n",__FILE__,__LINE__,#cond);} } while(0)
} // namespace

using cdroid::DynamicRefTable;
using cdroid::Res_value;
using cdroid::status_t;

int main() {
    // App package, not a library: app + framework ids are absolute (unchanged).
    {
        DynamicRefTable t(0x7f, false);
        uint32_t r = 0x7f010000;
        CHECK(t.lookupResourceId(&r) == cdroid::NO_ERROR);
        CHECK(r == 0x7f010000);                       // app id unchanged
        r = 0x01020003;
        CHECK(t.lookupResourceId(&r) == cdroid::NO_ERROR);
        CHECK(r == 0x01020003);                       // framework id unchanged
        r = 0;
        CHECK(t.lookupResourceId(&r) == cdroid::NO_ERROR);  // invalid -> no-op
        CHECK(r == 0);
    }

    // Shared library loaded as app-as-lib: its own @0x7f refs -> its assigned id.
    {
        DynamicRefTable t(0x02, true);                 // assigned runtime id 0x02
        uint32_t r = 0x7f010000;                       // self-reference (built as 0x7f)
        CHECK(t.lookupResourceId(&r) == cdroid::NO_ERROR);
        CHECK(r == 0x02010000);                        // remapped to 0x02
    }

    // Direct build-time -> run-time mapping via addMapping(buildId, runtimeId).
    {
        DynamicRefTable t(0x7f, false);
        t.addMapping(0x40, 0x50);                      // build 0x40 -> runtime 0x50
        uint32_t r = 0x40010002;
        CHECK(t.lookupResourceId(&r) == cdroid::NO_ERROR);
        CHECK(r == 0x50010002);
    }

    // No mapping registered for a package -> UNKNOWN_ERROR.
    {
        DynamicRefTable t(0x7f, false);
        uint32_t r = 0x40010002;                       // 0x40 has no mapping
        CHECK(t.lookupResourceId(&r) != cdroid::NO_ERROR);
    }

    // lookupResourceValue follows TYPE_REFERENCE / DYNAMIC_REFERENCE.
    {
        DynamicRefTable t(0x02, true);
        Res_value v;
        v.dataType = Res_value::TYPE_REFERENCE;
        v.data = 0x7f010000;
        CHECK(t.lookupResourceValue(&v) == cdroid::NO_ERROR);
        CHECK(v.dataType == Res_value::TYPE_REFERENCE);
        CHECK(v.data == 0x02010000);

        v.dataType = Res_value::TYPE_DYNAMIC_REFERENCE;
        v.data = 0x40010002;
        t.addMapping(0x40, 0x50);
        CHECK(t.lookupResourceValue(&v) == cdroid::NO_ERROR);
        CHECK(v.dataType == Res_value::TYPE_REFERENCE);  // DYNAMIC_REF -> REFERENCE
        CHECK(v.data == 0x50010002);

        // Non-reference value: no lookup.
        v.dataType = Res_value::TYPE_INT_DEC;
        v.data = 42;
        CHECK(t.lookupResourceValue(&v) == cdroid::NO_ERROR);
        CHECK(v.data == 42);
    }

    std::printf("dynamicreftable: %d checks, %d failures\n", g_checks, g_failures);
    return g_failures ? 1 : 0;
}
