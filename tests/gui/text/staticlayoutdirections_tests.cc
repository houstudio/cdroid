/*********************************************************************************
 * Ported from AOSP coretests android.text.StaticLayoutDirectionsTest (Apache 2.0).
 * Checks the Directions run encoding StaticLayout produces for pseudo-bidi
 * texts (A..D map to ALEF..DALET), plus trailing-whitespace run splitting and
 * the visual-order cursor walkers.
 *
 * Directions comparison is element-wise over mDirections (the packed
 * start/length|level int pairs); CDROID's Directions has no operator==.
 *********************************************************************************/
#include <text/staticlayout.h>
#include <text/layout.h>
#include <text/String.h>
#include <text/textpaint.h>
#include <text/textutils.h>
#include <gtest/gtest.h>
#include <cmath>
#include <string>
#include <vector>

using namespace cdroid;

namespace {

// Hebrew ALEF (U+05D0) — pseudo-bidi texts map 'A'..'D' onto ALEF..DALET.
const char16_t ALEF = char16_t(0x05D0);

// AOSP StaticLayoutTest.LayoutBuilder defaults (same as staticlayout_core_tests).
struct LayoutBuilder {
    std::u16string text = u"This is a test";
    TextPaint paint;                       // default
    int width = 100;
    Layout::Alignment align = Layout::Alignment::ALIGN_NORMAL;
    float spacingMult = 1.f;
    float spacingAdd = 0.f;
    bool includePad = false;

    StaticLayout* build() {
        return new StaticLayout(new String(text), &paint, width, align,
                                spacingMult, spacingAdd, includePad);
    }
};

// Constants from Layout that are package-protected in AOSP.
constexpr int RUN_LENGTH_MASK = Layout::RUN_LENGTH_MASK;
constexpr int RUN_LEVEL_SHIFT = Layout::RUN_LEVEL_SHIFT;
constexpr int RUN_LEVEL_MASK = Layout::RUN_LEVEL_MASK;
constexpr int RUN_RTL_FLAG = Layout::RUN_RTL_FLAG;

const std::vector<int> DIRS_ALL_LEFT_TO_RIGHT = { 0, RUN_LENGTH_MASK };
const std::vector<int> DIRS_ALL_RIGHT_TO_LEFT = { 0, RUN_LENGTH_MASK | RUN_RTL_FLAG };

constexpr int LVL1_1 = 1 | (1 << RUN_LEVEL_SHIFT);
constexpr int LVL2_1 = 1 | (2 << RUN_LEVEL_SHIFT);
constexpr int LVL2_2 = 2 | (2 << RUN_LEVEL_SHIFT);

const std::vector<std::u16string> texts = {
    u"", u" ", u"a", u"a1", u"aA", u"a1b", u"a1A", u"aA1", u"aAb", u"aA1B", u"aA1B2",
    // rtl
    u"A", u"A1", u"Aa", u"A1B", u"A1a", u"Aa1", u"AaB"
};

// Expected directions are an array of start/length+level pairs,
// in visual order from the leading margin.
const std::vector<std::vector<int>> expected = {
    DIRS_ALL_LEFT_TO_RIGHT,
    DIRS_ALL_LEFT_TO_RIGHT,
    DIRS_ALL_LEFT_TO_RIGHT,
    DIRS_ALL_LEFT_TO_RIGHT,
    { 0, 1, 1, LVL1_1 },
    DIRS_ALL_LEFT_TO_RIGHT,
    { 0, 2, 2, LVL1_1 },
    { 0, 1, 2, LVL2_1, 1, LVL1_1 },
    { 0, 1, 1, LVL1_1, 2, 1 },
    { 0, 1, 3, LVL1_1, 2, LVL2_1, 1, LVL1_1 },
    { 0, 1, 4, LVL2_1, 3, LVL1_1, 2, LVL2_1, 1, LVL1_1 },
    // rtl
    DIRS_ALL_RIGHT_TO_LEFT,
    { 0, LVL1_1, 1, LVL2_1 },
    { 0, LVL1_1, 1, LVL2_1 },
    { 0, LVL1_1, 1, LVL2_1, 2, LVL1_1 },
    { 0, LVL1_1, 1, LVL2_2 },
    { 0, LVL1_1, 1, LVL2_2 },
    { 0, LVL1_1, 1, LVL2_1, 2, LVL1_1 },
};

static std::u16string pseudoBidiToReal(const std::u16string& src) {
    std::u16string out = src;
    for (size_t j = 0; j < out.size(); ++j) {
        const char16_t c = out[j];
        if (c >= u'A' && c <= u'D') {
            out[j] = (char16_t)(ALEF + c - u'A');
        }
    }
    return out;
}

// utility for displaying arrays in hex
static std::string hexArray(const std::vector<int>& array) {
    std::string sb = "{";
    char buf[16];
    for (size_t i = 0; i < array.size(); i++) {
        if (sb.size() > 1) sb += ", ";
        snprintf(buf, sizeof(buf), "%x", array[i]);
        sb += buf;
    }
    return sb + "}";
}

static void expectDirections(const char* msg, const std::vector<int>& expectedDirs,
        const Directions* result) {
    EXPECT_TRUE(expectedDirs == result->mDirections) << msg << ": expected: "
        << hexArray(expectedDirs) << " got: " << hexArray(result->mDirections);
}

TEST(StaticLayoutDirectionsTest, testDirections) {
    LayoutBuilder b;
    for (size_t i = 0; i < texts.size(); ++i) {
        b.text = pseudoBidiToReal(texts[i]);
        Layout* l = b.build();
        const Directions* result = l->getLineDirections(0);
        ASSERT_TRUE(result != nullptr);
        // One assertion per text, tagged with the case index and text.
        EXPECT_TRUE(expected[i] == result->mDirections)
            << "[" << i << "] '" << TextUtils::utf16_utf8(texts[i]) << "', expected: "
            << hexArray(expected[i]) << " != got: " << hexArray(result->mDirections);
    }
}

TEST(StaticLayoutDirectionsTest, testTrailingWhitespace) {
    LayoutBuilder b;
    b.text = pseudoBidiToReal(u"Ab   c");
    const float width = b.paint.measureText(b.text.c_str(), 0, 5);  // exclude 'c'
    b.width = (int) std::lroundf(width);
    Layout* l = b.build();
    ASSERT_EQ(2, l->getLineCount()) << "expected 2 lines";
    const Directions* result = l->getLineDirections(0);
    const std::vector<int> expectedDirs = { 0, LVL1_1, 1, LVL2_1,
            2, 3 | (1 << Layout::RUN_LEVEL_SHIFT) };
    expectDirections("split line", expectedDirs, result);
}

TEST(StaticLayoutDirectionsTest, testNextToRightOf) {
    LayoutBuilder b;
    b.text = pseudoBidiToReal(u"aA1B2");
    // visual a2B1A positions 04321
    // 0: |a2B1A, strong is sol, after -> 0
    // 1: a|2B1A, strong is a, after ->, 1
    // 2: a2|B1A, strong is B, after -> 4
    // 3: a2B|1A, strong is B, before -> 3
    // 4: a2B1|A, strong is A, after -> 2
    // 5: a2B1A|, strong is eol, before -> 5
    const int expected[] = { 0, 1, 4, 3, 2, 5 };
    Layout* l = b.build();
    int n = 0;
    for (int i = 1; i < (int)(sizeof(expected) / sizeof(expected[0])); ++i) {
        const int t = l->getOffsetToRightOf(n);
        EXPECT_EQ(expected[i], t) << "offset[" << i << "] to right of: " << n;
        n = t;
    }
}

TEST(StaticLayoutDirectionsTest, testNextToLeftOf) {
    LayoutBuilder b;
    b.text = pseudoBidiToReal(u"aA1B2");
    const int expected[] = { 0, 1, 4, 3, 2, 5 };
    Layout* l = b.build();
    int n = 5;
    for (int i = (int)(sizeof(expected) / sizeof(expected[0])) - 1; --i >= 0;) {
        const int t = l->getOffsetToLeftOf(n);
        EXPECT_EQ(expected[i], t) << "offset[" << i << "] to left of: " << n;
        n = t;
    }
}

} // namespace
