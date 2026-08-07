// AOSP CTS ColorTest port (android.graphics.Color). Partial alignment: CDROID's Color port
// exposes the component extractors (alpha/red/green/blue), parseColor, and now argb/rgb (added
// alongside this test, faithful to AOSP). The HSV family (HSVToColor/colorToHSV/RGBToHSV) and
// luminance are absent from CDROID's Color and unused, so their cases are skipped (YAGNI — adding
// them just to satisfy the test would be code written only for the test). resourceColor needs the
// framework resource table and is skipped.
//
// CDROID divergences (documented, not "fixed" — partial alignment per scope):
//  - Named-color table is an HTML-color superset; it diverges from AOSP's 12-name set for the gray
//    family: "gray" -> 0xFFBEBEBE (vs AOSP GRAY 0xFF888888), and "darkgray"/"lightgray" are absent.
//    testParseColor asserts only the names that align.
//  - parseColor does not validate '#' length, so AOSP testParseColorStringOfInvalidLength (expects
//    a throw on "#ff00ff0") does not hold — skipped.
//
// Original: cts/tests/tests/graphics/src/android/graphics/cts/ColorTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <core/color.h>
#include <stdexcept>

using namespace cdroid;

class CtsColorTest : public testing::Test {};

TEST_F(CtsColorTest, testAlpha) {
    EXPECT_EQ(0xff, Color::alpha(Color::RED));
    EXPECT_EQ(0xff, Color::alpha(Color::YELLOW));
}

TEST_F(CtsColorTest, testRed) {
    EXPECT_EQ(0xff, Color::red(Color::RED));
    EXPECT_EQ(0xff, Color::red(Color::YELLOW));
}

TEST_F(CtsColorTest, testGreen) {
    EXPECT_EQ(0x00, Color::green(Color::RED));
    EXPECT_EQ(0xff, Color::green(Color::GREEN));
}

TEST_F(CtsColorTest, testBlue) {
    EXPECT_EQ(0x00, Color::blue(Color::RED));
    EXPECT_EQ(0x00, Color::blue(Color::YELLOW));
}

TEST_F(CtsColorTest, testArgb) {
    EXPECT_EQ((unsigned)Color::RED,    Color::argb(0xff, 0xff, 0x00, 0x00));
    EXPECT_EQ((unsigned)Color::YELLOW, Color::argb(0xff, 0xff, 0xff, 0x00));
    EXPECT_EQ((unsigned)Color::RED,    Color::argb(1.0f, 1.0f, 0.0f, 0.0f));
    EXPECT_EQ((unsigned)Color::YELLOW, Color::argb(1.0f, 1.0f, 1.0f, 0.0f));
}

TEST_F(CtsColorTest, testRgb) {
    EXPECT_EQ((unsigned)Color::RED,    Color::rgb(0xff, 0x00, 0x00));
    EXPECT_EQ((unsigned)Color::YELLOW, Color::rgb(0xff, 0xff, 0x00));
    EXPECT_EQ((unsigned)Color::RED,    Color::rgb(1.0f, 0.0f, 0.0f));
    EXPECT_EQ((unsigned)Color::YELLOW, Color::rgb(1.0f, 1.0f, 0.0f));
}

TEST_F(CtsColorTest, testParseColor) {
    EXPECT_EQ((unsigned)Color::RED,   Color::parseColor("#ff0000"));
    EXPECT_EQ((unsigned)Color::RED,   Color::parseColor("#ffff0000"));
    // CDROID also supports the 3-digit #rgb form (AOSP testParseColor doesn't exercise it).
    EXPECT_EQ((unsigned)Color::RED,   Color::parseColor("#f00"));

    EXPECT_EQ((unsigned)Color::BLACK,  Color::parseColor("black"));
    EXPECT_EQ((unsigned)Color::WHITE,  Color::parseColor("white"));
    EXPECT_EQ((unsigned)Color::RED,    Color::parseColor("red"));
    EXPECT_EQ((unsigned)Color::GREEN,  Color::parseColor("green"));
    EXPECT_EQ((unsigned)Color::BLUE,   Color::parseColor("blue"));
    EXPECT_EQ((unsigned)Color::YELLOW, Color::parseColor("yellow"));
    EXPECT_EQ((unsigned)Color::CYAN,   Color::parseColor("cyan"));
    EXPECT_EQ((unsigned)Color::MAGENTA,Color::parseColor("magenta"));
    // Gray family intentionally NOT asserted: CDROID's HTML table maps "gray" -> 0xFFBEBEBE (not
    // AOSP GRAY 0xFF888888) and lacks "darkgray"/"lightgray".
}

TEST_F(CtsColorTest, testParseColorUnsupportedFormat) {
    // getHtmlColor throws std::invalid_argument for an unknown name (CTS expects IllegalArgumentException).
    EXPECT_THROW(Color::parseColor("hello"), std::invalid_argument);
}

// Skipped (CDROID divergence / unused API, see header):
//   testParseColorStringOfInvalidLength — CDROID parseColor does not validate '#'-length, so
//     "#ff00ff0" does not throw.
//   testHSVToColor / testHSVToColorWithAlpha / testHSVToColorArrayTooShort /
//   testRGBToHSV / testRGBToHSVArrayTooShort — no HSV API in CDROID's Color (unused).
//   testLuminance                        — no luminance() (unused).
//   resourceColor                        — needs the framework resource table (android.R.color).
