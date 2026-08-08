// Theme test (phase 2): applyStyle with parent inheritance + override semantics.
// Fixture arsc_theme_fixture.h (kARST):
//   style/Parent (0x7f010001): textColor=#ffff0000, textSize=10sp
//   style/Child  (0x7f010000, parent=Parent): textColor=#ff00ff00
// After applyStyle(Child): textColor <- child (#ff00ff00, override),
// textSize <- parent (10sp, inherited).
#include "resourcetypes.h"
#include "arsc_theme_fixture.h"

#include <cstdio>
#include <cstring>

namespace {
int g_failures = 0, g_checks = 0;
#define C(x) do { ++g_checks; if(!(x)){ ++g_failures; \
    std::fprintf(stderr,"FAIL %s:%d: %s\n",__FILE__,__LINE__,#x);} } while(0)
} // namespace

using cdroid::ResTable;
using cdroid::ResTable_config;
using cdroid::Res_value;

static const uint32_t kChild       = 0x7f010000;
static const uint32_t kParent      = 0x7f010001;
static const uint32_t ATTR_TEXT_COLOR = 0x01010098;  // android:textColor
static const uint32_t ATTR_TEXT_SIZE  = 0x01010095;  // android:textSize

// 10sp dimension = SP unit | radix 23p0 | mantissa 10.
static const uint32_t k10sp = (Res_value::COMPLEX_UNIT_SP)
                            | (Res_value::COMPLEX_RADIX_23p0 << Res_value::COMPLEX_RADIX_SHIFT)
                            | (10u << Res_value::COMPLEX_MANTISSA_SHIFT);

int main() {
    ResTable t;
    C(t.add(kARST, kARSTLen) == cdroid::NO_ERROR);

    ResTable_config dflt; memset(&dflt, 0, sizeof(dflt)); dflt.size = sizeof(dflt);
    t.setParameters(&dflt);

    // Apply Child; its parent (Parent) must be applied first by the chain.
    ResTable::Theme theme(t);
    C(theme.applyStyle(kChild) == cdroid::NO_ERROR);

    // textColor: child overrides parent.
    {
        Res_value v; uint32_t flags = 0;
        C(theme.getAttribute(ATTR_TEXT_COLOR, &v, &flags) >= 0);
        C(v.dataType == Res_value::TYPE_INT_COLOR_ARGB8);
        C(v.data == 0xFF00FF00u);   // child's #ff00ff00, NOT parent's #ffff0000
    }
    // textSize: inherited from parent (child doesn't set it).
    {
        Res_value v; uint32_t flags = 0;
        C(theme.getAttribute(ATTR_TEXT_SIZE, &v, &flags) >= 0);
        C(v.dataType == Res_value::TYPE_DIMENSION);
        C(v.data == k10sp);
    }
    // An attr neither style defines -> not found.
    {
        Res_value v;
        C(theme.getAttribute(0x01010099, &v) < 0);
    }

    // Force semantics: applying Parent with force=true overrides child's textColor.
    C(theme.applyStyle(kParent, /*force*/true) == cdroid::NO_ERROR);
    {
        Res_value v;
        C(theme.getAttribute(ATTR_TEXT_COLOR, &v) >= 0);
        C(v.data == 0xFFFF0000u);   // parent's, force-overrode child's
    }

    // clear() empties the theme.
    C(theme.clear() == cdroid::NO_ERROR);
    {
        Res_value v;
        C(theme.getAttribute(ATTR_TEXT_COLOR, &v) < 0);
    }

    // setTo copies another theme.
    {
        ResTable::Theme src(t);
        C(src.applyStyle(kChild) == cdroid::NO_ERROR);
        C(theme.setTo(src) == cdroid::NO_ERROR);
        Res_value v;
        C(theme.getAttribute(ATTR_TEXT_COLOR, &v) >= 0);
        C(v.data == 0xFF00FF00u);   // restored from src via setTo
    }

    // getChangingConfigurations: these styles are default-config, so no axes.
    C(theme.getChangingConfigurations() == 0);

    std::printf("theme: %d checks, %d failures\n", g_checks, g_failures);
    return g_failures ? 1 : 0;
}
