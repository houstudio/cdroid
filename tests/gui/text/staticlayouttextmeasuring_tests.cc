/*********************************************************************************
 * Ported from AOSP coretests android.text.StaticLayoutTextMeasuringTest
 * (Apache 2.0). Tests for text measuring methods of StaticLayout.
 *
 * KNOWN DEVIATION candidate: testGetPrimaryHorizontal_flagEmoji asserts the
 * grapheme/emoji cluster pairing of getPrimaryHorizontal — same Emoji-cluster
 * gap family as StaticLayoutTest.testEmojiOffset (getOffsetToRightOf skips one
 * code unit instead of the flag pair).
 *********************************************************************************/
#include <text/staticlayout.h>
#include <text/String.h>
#include <text/textpaint.h>
#include <gtest/gtest.h>
#include <string>

using namespace cdroid;

namespace {

constexpr float SPACE_MULTI = 1.0f;
constexpr float SPACE_ADD = 0.0f;
constexpr int DEFAULT_OUTER_WIDTH = 150;
const Layout::Alignment DEFAULT_ALIGN = Layout::Alignment::ALIGN_LEFT;

TEST(StaticLayoutTextMeasuringTest, testGetPrimaryHorizontal_zwnbsp) {
    TextPaint defaultPaint;
    // a, ZERO WIDTH NO-BREAK SPACE (U+FEFF)
    const std::u16string testString = { u'a', char16_t(0xFEFF) };
    StaticLayout layout(new String(testString), &defaultPaint,
            DEFAULT_OUTER_WIDTH, DEFAULT_ALIGN, SPACE_MULTI, SPACE_ADD, true);

    EXPECT_EQ(0.0f, layout.getPrimaryHorizontal(0));
    EXPECT_EQ(layout.getPrimaryHorizontal(2), layout.getPrimaryHorizontal(1));
}

TEST(StaticLayoutTextMeasuringTest, testGetPrimaryHorizontal_devanagari) {
    TextPaint defaultPaint;
    // DEVANAGARI LETTER KA (U+0915), DEVANAGARI VOWEL SIGN AA (U+093E)
    const std::u16string testString = { char16_t(0x0915), char16_t(0x093E) };
    StaticLayout layout(new String(testString), &defaultPaint,
            DEFAULT_OUTER_WIDTH, DEFAULT_ALIGN, SPACE_MULTI, SPACE_ADD, true);

    EXPECT_EQ(0.0f, layout.getPrimaryHorizontal(0));
    EXPECT_EQ(layout.getPrimaryHorizontal(2), layout.getPrimaryHorizontal(1));
}

TEST(StaticLayoutTextMeasuringTest, testGetPrimaryHorizontal_flagEmoji) {
    TextPaint defaultPaint;
    // REGIONAL INDICATOR SYMBOL LETTER U, LETTER S, LETTER Z.
    // First two code points (U and S) forms a US flag.
    const std::u16string testString = { char16_t(0xD83C), char16_t(0xDDFA),
                                        char16_t(0xD83C), char16_t(0xDDF8),
                                        char16_t(0xD83C), char16_t(0xDDFF) };
    StaticLayout layout(new String(testString), &defaultPaint,
            DEFAULT_OUTER_WIDTH, DEFAULT_ALIGN, SPACE_MULTI, SPACE_ADD, true);

    EXPECT_EQ(0.0f, layout.getPrimaryHorizontal(0));
    EXPECT_EQ(layout.getPrimaryHorizontal(4), layout.getPrimaryHorizontal(1));
    EXPECT_EQ(layout.getPrimaryHorizontal(4), layout.getPrimaryHorizontal(2));
    EXPECT_EQ(layout.getPrimaryHorizontal(4), layout.getPrimaryHorizontal(3));

    EXPECT_GT(layout.getPrimaryHorizontal(6), layout.getPrimaryHorizontal(4));
    EXPECT_EQ(layout.getPrimaryHorizontal(6), layout.getPrimaryHorizontal(5));
}

} // namespace
