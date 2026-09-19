// Ported from AOSP coretests MeasuredParagraphTest (android.text).
// Original: frameworks/base/core/tests/coretests/src/android/text/MeasuredParagraphTest.java (Apache 2.0)
//
// CDROID adaptation:
//  - The exact-width test font (U+0058 X=10em, U+0056 V=5em, U+005F _=0em...)
//    is copied from coretests assets into tests/gui/assets/font/ and packed
//    into gui_test.pak; Typeface::createFromAsset loads it (per-path cache).
//  - buildForStaticLayout takes (paint, lineBreakConfig, text, start, end,
//    dir, computeHyphenation, computeLayout, computeBounds, hint, recycle)
//    here; AOSP's two false flags map to hyphenation/bounds off, layout on
//    (the test expects a non-null MeasuredText).
#include <gtest/gtest.h>
#include <text/measuredparagraph.h>
#include <text/layout.h>
#include <text/textpaint.h>
#include <text/String.h>
#include <text/textdirectionheuristics.h>
#include <core/typeface.h>

using namespace cdroid;

namespace {

const TextDirectionHeuristic* LTR = TextDirectionHeuristics::LTR;
const TextDirectionHeuristic* RTL = TextDirectionHeuristics::RTL;

std::u16string charsToString(const std::vector<char16_t>& chars) {
    return std::u16string(chars.begin(), chars.end());
}

TextPaint makeTestPaint() {
    TextPaint paint;
    paint.setTypeface(Typeface::createFromAsset("font/StaticLayoutLineBreakingTestFont.ttf"));
    paint.setTextSize(1.0f);  // Make 1em == 1px.
    return paint;
}

} // namespace

TEST(CoreMeasuredParagraphTest, buildForBidi) {
    MeasuredParagraph* mt = nullptr;

    String xxx(u"XXX");
    mt = MeasuredParagraph::buildForBidi(&xxx, 0, 3, LTR, nullptr);
    ASSERT_NE(nullptr, mt);
    EXPECT_EQ(u"XXX", charsToString(mt->getChars()));
    EXPECT_EQ(Layout::DIR_LEFT_TO_RIGHT, mt->getParagraphDir());
    EXPECT_NE(nullptr, mt->getDirections(0, 3));
    EXPECT_EQ(0.0f, mt->getWholeWidth());
    EXPECT_EQ(0u, mt->getWidths().size());
    EXPECT_EQ(0u, mt->getSpanEndCache().size());
    EXPECT_EQ(0u, mt->getFontMetrics().size());
    EXPECT_EQ(nullptr, mt->getMeasuredText());

    // Recycle it
    String vvv5(u"_VVV_");
    MeasuredParagraph* mt2 = MeasuredParagraph::buildForBidi(&vvv5, 1, 4, RTL, mt);
    EXPECT_EQ(mt2, mt);
    EXPECT_EQ(u"VVV", charsToString(mt2->getChars()));
    EXPECT_NE(nullptr, mt2->getDirections(0, 3));
    EXPECT_EQ(0.0f, mt2->getWholeWidth());
    EXPECT_EQ(0u, mt2->getWidths().size());
    EXPECT_EQ(0u, mt2->getSpanEndCache().size());
    EXPECT_EQ(0u, mt2->getFontMetrics().size());
    EXPECT_EQ(nullptr, mt->getMeasuredText());

    mt2->recycle();
}

TEST(CoreMeasuredParagraphTest, buildForMeasurement) {
    TextPaint paint = makeTestPaint();

    String xxx(u"XXX");
    MeasuredParagraph* mt = MeasuredParagraph::buildForMeasurement(&paint, &xxx, 0, 3, LTR, nullptr);
    ASSERT_NE(nullptr, mt);
    EXPECT_EQ(u"XXX", charsToString(mt->getChars()));
    EXPECT_EQ(Layout::DIR_LEFT_TO_RIGHT, mt->getParagraphDir());
    EXPECT_NE(nullptr, mt->getDirections(0, 3));
    EXPECT_EQ(30.0f, mt->getWholeWidth());
    ASSERT_EQ(3u, mt->getWidths().size());
    EXPECT_EQ(10.0f, mt->getWidths()[0]);
    EXPECT_EQ(10.0f, mt->getWidths()[1]);
    EXPECT_EQ(10.0f, mt->getWidths()[2]);
    EXPECT_EQ(0u, mt->getSpanEndCache().size());
    EXPECT_EQ(0u, mt->getFontMetrics().size());
    EXPECT_EQ(nullptr, mt->getMeasuredText());

    // Recycle it
    String vvv5(u"_VVV_");
    MeasuredParagraph* mt2 = MeasuredParagraph::buildForMeasurement(&paint, &vvv5, 1, 4, RTL, mt);
    EXPECT_EQ(mt2, mt);
    EXPECT_EQ(u"VVV", charsToString(mt2->getChars()));
    EXPECT_EQ(Layout::DIR_RIGHT_TO_LEFT, mt2->getParagraphDir());
    EXPECT_NE(nullptr, mt2->getDirections(0, 3));
    EXPECT_EQ(15.0f, mt2->getWholeWidth());
    ASSERT_EQ(3u, mt2->getWidths().size());
    EXPECT_EQ(5.0f, mt2->getWidths()[0]);
    EXPECT_EQ(5.0f, mt2->getWidths()[1]);
    EXPECT_EQ(5.0f, mt2->getWidths()[2]);
    EXPECT_EQ(0u, mt2->getSpanEndCache().size());
    EXPECT_EQ(0u, mt2->getFontMetrics().size());
    EXPECT_EQ(nullptr, mt->getMeasuredText());

    mt2->recycle();
}

TEST(CoreMeasuredParagraphTest, buildForStaticLayout) {
    TextPaint paint = makeTestPaint();

    String xxx(u"XXX");
    MeasuredParagraph* mt = MeasuredParagraph::buildForStaticLayout(
            &paint, nullptr /* no line break config */, &xxx, 0, 3, LTR,
            false, true, false, nullptr /* no hint */, nullptr);
    ASSERT_NE(nullptr, mt);
    EXPECT_EQ(u"XXX", charsToString(mt->getChars()));
    EXPECT_EQ(Layout::DIR_LEFT_TO_RIGHT, mt->getParagraphDir());
    EXPECT_NE(nullptr, mt->getDirections(0, 3));
    EXPECT_EQ(0.0f, mt->getWholeWidth());
    EXPECT_EQ(0u, mt->getWidths().size());
    ASSERT_EQ(1u, mt->getSpanEndCache().size());
    EXPECT_EQ(3, mt->getSpanEndCache()[0]);
    EXPECT_NE(0u, mt->getFontMetrics().size());
    EXPECT_NE(nullptr, mt->getMeasuredText());

    // Recycle it
    String vvv5(u"_VVV_");
    MeasuredParagraph* mt2 = MeasuredParagraph::buildForStaticLayout(
            &paint, nullptr, &vvv5, 1, 4, RTL,
            false, true, false, nullptr /* no hint */, mt);
    EXPECT_EQ(mt2, mt);
    EXPECT_EQ(u"VVV", charsToString(mt2->getChars()));
    EXPECT_EQ(Layout::DIR_RIGHT_TO_LEFT, mt2->getParagraphDir());
    EXPECT_NE(nullptr, mt2->getDirections(0, 3));
    EXPECT_EQ(0.0f, mt2->getWholeWidth());
    EXPECT_EQ(0u, mt2->getWidths().size());
    ASSERT_EQ(1u, mt2->getSpanEndCache().size());
    EXPECT_EQ(4, mt2->getSpanEndCache()[0]);
    EXPECT_NE(0u, mt2->getFontMetrics().size());
    EXPECT_NE(nullptr, mt->getMeasuredText());

    mt2->recycle();
}

TEST(CoreMeasuredParagraphTest, testFor70146381) {
    TextPaint paint = makeTestPaint();
    String xEllipsis(u"X…");
    MeasuredParagraph* mt = MeasuredParagraph::buildForMeasurement(
            &paint, &xEllipsis, 0, 2, RTL, nullptr);
    mt->recycle();
}
