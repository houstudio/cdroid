/*********************************************************************************
 * Ported from AOSP coretests android.text.TextLayoutTest (Apache 2.0) and
 * android.text.TextShaperTest.
 *
 * TextLayoutTest is a construction smoke test for both layout families. The
 * layouts' base text must outlive them (DynamicLayout's dtor reads it), so the
 * String is deliberately leaked.
 *
 * TextShaperTest: android.text.TextShaper is not ported (see layout.h's
 * TextShaper/advances gap note) — recorded as a skip.
 *********************************************************************************/
#include <text/staticlayout.h>
#include <text/dynamiclayout.h>
#include <text/String.h>
#include <text/textpaint.h>
#include <gtest/gtest.h>
#include <string>

using namespace cdroid;

namespace {

TEST(CoreTextLayoutTest, testStaticLayout) {
    String* text = new String(u"The quick brown fox");
    TextPaint paint;
    StaticLayout layout(text, &paint, 200,
            Layout::Alignment::ALIGN_NORMAL, 1, 0, true);
}

TEST(CoreTextLayoutTest, testDynamicLayoutTest) {
    String* text = new String(u"The quick brown fox");
    TextPaint paint;
    DynamicLayout layout(text, &paint, 200,
            Layout::Alignment::ALIGN_NORMAL, 1, 0, true);
}

TEST(CoreTextShaperTest, testFontWithPath) {
    GTEST_SKIP() << "android.text.TextShaper is not ported (layout.h documents "
                    "the TextShaper/advances gap); Paint::setFontFeatureSettings "
                    "exists but shapeText does not";
}

} // namespace
