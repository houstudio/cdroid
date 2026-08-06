// Ported from AOSP CTS span style tests (android.text.style.*SpanTest).
// Each CTS span test is tiny (constructor + value getter; updateDrawState/parcel paths are
// paint/Parcelable-coupled and skipped). Covers the span "value" half of the text suite.
//
// Color literals stand in for android.graphics.Color.* (GREEN=0xFF00FF00 etc.).
//
// Sources: cts/tests/tests/text/src/android/text/style/cts/{Foreground,Background}ColorSpanTest,
// AbsoluteSizeSpanTest, StyleSpanTest, QuoteSpanTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <core/canvas.h>
#include <text/style/characterstyles.h>
#include <text/style/metricaffectingspan.h>
#include <text/style/leadingmarginspan.h>

using namespace cdroid;

namespace {
constexpr int COLOR_RED   = 0xFFFF0000;
constexpr int COLOR_BLUE  = 0xFF0000FF;
constexpr int COLOR_BLACK = 0xFF000000;
constexpr int COLOR_CYAN  = 0xFF00FFFF;
constexpr int COLOR_GRAY  = 0xFF888888;
}

// --- ForegroundColorSpanTest ---
TEST(CtsSpanStyleTest, ForegroundColorSpanGetColor) {
    ForegroundColorSpan a(COLOR_BLUE);
    EXPECT_EQ(COLOR_BLUE, a.getForegroundColor());
    ForegroundColorSpan b(COLOR_BLACK);
    EXPECT_EQ(COLOR_BLACK, b.getForegroundColor());
}

// --- BackgroundColorSpanTest ---
TEST(CtsSpanStyleTest, BackgroundColorSpanGetColor) {
    BackgroundColorSpan a(COLOR_CYAN);
    EXPECT_EQ(COLOR_CYAN, a.getBackgroundColor());
    BackgroundColorSpan b(COLOR_GRAY);
    EXPECT_EQ(COLOR_GRAY, b.getBackgroundColor());
}

// --- AbsoluteSizeSpanTest ---
TEST(CtsSpanStyleTest, AbsoluteSizeSpanGetSize) {
    AbsoluteSizeSpan a(5);
    EXPECT_EQ(5, a.getSize());
    AbsoluteSizeSpan b(-5);
    EXPECT_EQ(-5, b.getSize());
}

// --- StyleSpanTest ---
TEST(CtsSpanStyleTest, StyleSpanGetStyle) {
    StyleSpan a(2);
    EXPECT_EQ(2, a.getStyle());
    StyleSpan b(-2);
    EXPECT_EQ(-2, b.getStyle());
}

// --- QuoteSpanTest ---
TEST(CtsSpanStyleTest, QuoteSpanDefaultColor) {
    QuoteSpan q;
    EXPECT_NE(0, q.getColor());
}
TEST(CtsSpanStyleTest, QuoteSpanFromColor) {
    QuoteSpan q(COLOR_RED);
    EXPECT_EQ(COLOR_RED, q.getColor());
}
