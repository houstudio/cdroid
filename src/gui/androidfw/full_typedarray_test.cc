// Full TypedArray test (phase 3): the applyStyle priority merge — element's own
// attribute wins; attributes the element omits fall back to its style=.
//
// Fixtures:
//   arsc_typedarray_fixture.h (kARSTA): style/BtnStyle=0x7f020000
//       android:textSize=12sp, android:textColor=#ffffaa00
//   axml_typedarray_fixture.h (kAXMLTA): <Button style="@style/BtnStyle" android:textSize="20sp"/>
//
// So: textSize <- element (20sp, overrides style's 12sp);
//     textColor <- style (element omits it) -> #ffffaa00.
#include "resourcetypes.h"
#include "arsc_typedarray_fixture.h"
#include "axml_typedarray_fixture.h"

#include <cstdio>
#include <cstring>

namespace {
int g_failures = 0, g_checks = 0;
#define C(x) do { ++g_checks; if(!(x)){ ++g_failures; \
    std::fprintf(stderr,"FAIL %s:%d: %s\n",__FILE__,__LINE__,#x);} } while(0)
} // namespace

using cdroid::ResTable;
using cdroid::ResXMLTree;
using cdroid::ResXMLParser;
using cdroid::Res_value;
using cdroid::StyledAttr;
using cdroid::TypedArray;

static const uint32_t ATTR_TEXT_COLOR = 0x01010098;
static const uint32_t ATTR_TEXT_SIZE  = 0x01010095;

int main() {
    // Resource table holds the style.
    ResTable table;
    C(table.add(kARSTA, kARSTALen) == cdroid::NO_ERROR);
    cdroid::ResTable_config dflt; memset(&dflt, 0, sizeof(dflt)); dflt.size = sizeof(dflt);
    table.setParameters(&dflt);

    // AXML holds the element.
    ResXMLTree xml;
    C(xml.setTo(kAXMLTA, kAXMLTALen) == cdroid::NO_ERROR);
    while (xml.next() != ResXMLParser::START_TAG) {  // walk to <Button>
        if (xml.getEventType() == ResXMLParser::END_DOCUMENT) return 1;
    }

    // attrs[] (a styleable set): [textColor, textSize].
    const uint32_t attrs[] = { ATTR_TEXT_COLOR, ATTR_TEXT_SIZE };
    StyledAttr vals[2];
    cdroid::obtainStyledAttributes(xml, table, /*theme*/nullptr, attrs, 2,
                                   /*defStyleAttr*/0, /*defStyleRes*/0, vals);

    TypedArray ta(table, vals, 2, &xml, /*density*/2.0f);
    C(ta.hasValue(0));  // textColor <- style
    C(ta.hasValue(1));  // textSize <- element

    // textColor from the style (element omits it): #ffffaa00.
    C(ta.getColor(0, 0) == 0xFFFFAA00u);

    // textSize from the ELEMENT (20sp), overriding the style's 12sp.
    C(ta.getDimension(1, 0) == 20.0f);
    C(ta.getDimensionPixelSize(1, 0) == 40);  // 20 * density(2.0)

    // An attr nobody provides -> not set; getter returns default.
    const uint32_t attrs2[] = { 0x01010099 /* some other attr */ };
    StyledAttr vals2[1];
    cdroid::obtainStyledAttributes(xml, table, nullptr, attrs2, 1, 0, 0, vals2);
    TypedArray ta2(table, vals2, 1, &xml);
    C(!ta2.hasValue(0));
    C(ta2.getInt(0, 7) == 7);

    std::printf("full_typedarray: %d checks, %d failures\n", g_checks, g_failures);
    return g_failures ? 1 : 0;
}
