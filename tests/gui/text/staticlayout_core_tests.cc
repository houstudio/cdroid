// Ported from AOSP coretests StaticLayoutTest (android.text).
// Original: frameworks/base/core/tests/coretests/src/android/text/
//           StaticLayoutTest.java (Apache 2.0)
//
// CDROID adaptation:
//  - StaticLayout.Builder::obtain returns a pooled builder whose build()
//    recycles it — builders are intentionally never deleted here.
//  - getLineBounds Rect is {l,t,w,h}; AOSP right/bottom map to width/bottom().
//  - Skipped (API gaps, recorded): testLocaleSpanAffectsHyphenation (no
//    LocaleSpan / Paint.setTextLocale / hyphenation engine),
//    testFallbackLineSpacing (no FontFallbackSetup harness), and the
//    Normalizer-form variants of testRtlOffset (no java.text.Normalizer).
//    testLineMetrics_withLargeText skips on leading==0 (AOSP does the same;
//    the registry default font has zero line gap).
//  - Accessibility never ported (system policy).
#include <gtest/gtest.h>
#include <text/staticlayout.h>
#include <text/layout.h>
#include <text/textpaint.h>
#include <text/textdirectionheuristics.h>
#include <text/String.h>
#include <text/textutils.h>
#include <core/rect.h>
#include <core/canvas.h>

using namespace cdroid;

namespace {

constexpr float SPACE_MULTI = 1.0f;
constexpr float SPACE_ADD = 0.0f;
constexpr int DEFAULT_OUTER_WIDTH = 150;

const std::u16string LAYOUT_TEXT = u"CharSe\tq\nChar"
        u"Sequence\nCharSequence\nHelllo\n, world\nLongLongLong";
const std::u16string LAYOUT_TEXT_SINGLE_LINE = u"CharSequence";

constexpr Layout::Alignment DEFAULT_ALIGN = Layout::Alignment::ALIGN_CENTER;
constexpr int ELLIPSIZE_WIDTH = 8;

// AOSP StaticLayoutTest.LayoutBuilder (defaults from :326-344).
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

// AOSP StaticLayoutTest.Scaler (:411-423) — note the ctor subtracts 1 from
// the multiplier: the expected extra is (below-above)*(mult-1)+add, exactly
// the formula StaticLayout.out() writes into the EXTRA column.
struct Scaler {
    const float mult, add;
    Scaler(float m, float a) : mult(m - 1), add(a) {}
    int scale(float x) const { return (int) std::lroundf(x * mult) + (int) add; }
};

// AOSP assertLinesMetrics/assertLineMetrics (:390-452).
void assertLineMetrics(const Layout& l, int line,
        int top, int ascent, int descent, int height, int extra) {
    EXPECT_EQ(top, l.getLineTop(line)) << "line " << line;
    EXPECT_EQ(ascent, l.getLineAscent(line)) << "line " << line;
    EXPECT_EQ(descent, l.getLineDescent(line)) << "line " << line;
    EXPECT_EQ(height, l.getLineBottom(line) - top) << "line " << line;
    EXPECT_EQ(extra, l.getLineExtra(line)) << "line " << line;
}

void assertVertMetrics(const Layout& l, int topPad, int botPad,
        const std::vector<std::array<int, 3>>& values) {
    EXPECT_EQ(topPad, l.getTopPadding());
    EXPECT_EQ(botPad, l.getBottomPadding());
    int t = 0;
    const int lines = l.getLineCount();
    ASSERT_EQ((size_t) lines, values.size());
    for (int i = 0; i < lines; ++i) {
        const int a = values[i][0];
        const int d = values[i][1];
        const int extra = values[i][2];
        const int h = -a + d;
        assertLineMetrics(l, i, t, a, d, h, extra);
        t += h;
    }
    EXPECT_EQ(t, l.getHeight());
}

StaticLayout* createEllipsizeStaticLayout(TextPaint& paint,
        const std::u16string& text, TextUtils::TruncateAt ellipsize, int maxLines) {
    return new StaticLayout(new String(text), 0, (int) text.length(), &paint,
            DEFAULT_OUTER_WIDTH, DEFAULT_ALIGN,
            TextDirectionHeuristics::FIRSTSTRONG_LTR,
            SPACE_MULTI, SPACE_ADD, true /* include pad */,
            ellipsize, ELLIPSIZE_WIDTH, maxLines);
}

// AOSP android.text.EditorState helper: "U+XXXX ... | ..." cursor notation.
struct EditorState {
    std::u16string mText;
    int mSelectionStart = -1;
    int mSelectionEnd = -1;

    void setByString(const std::string& spec) {
        std::u16string text;
        int cursor = -1;
        size_t i = 0;
        while (i < spec.size()) {
            if (spec.compare(i, 2, "U+") == 0) {
                const size_t sp = spec.find(' ', i);
                const std::string hex = spec.substr(i + 2,
                        (sp == std::string::npos ? spec.size() : sp) - i - 2);
                // emit surrogate pairs like AOSP's appendCodePoint — casting a
                // supplementary code point straight to char16_t truncates it
                const uint32_t cp = std::stoul(hex, nullptr, 16);
                if (cp >= 0x10000) {
                    const uint32_t v = cp - 0x10000;
                    text.push_back((char16_t)(0xD800 + (v >> 10)));
                    text.push_back((char16_t)(0xDC00 + (v & 0x3FF)));
                } else {
                    text.push_back((char16_t)cp);
                }
                i = (sp == std::string::npos) ? spec.size() : sp;
            } else if (spec[i] == '|') {
                cursor = (int) text.size();
                ++i;
            } else {
                ++i; // separators
            }
        }
        mText = text;
        mSelectionStart = mSelectionEnd = cursor;
    }
};

void moveCursorToRightCursorableOffset(EditorState& state, TextPaint& paint) {
    ASSERT_EQ(state.mSelectionStart, state.mSelectionEnd);
    LayoutBuilder b;
    b.text = state.mText;
    StaticLayout* layout = b.build();
    state.mSelectionStart = state.mSelectionEnd =
            layout->getOffsetToRightOf(state.mSelectionStart);
    delete layout;
}

void moveCursorToLeftCursorableOffset(EditorState& state, TextPaint& paint) {
    ASSERT_EQ(state.mSelectionStart, state.mSelectionEnd);
    LayoutBuilder b;
    b.text = state.mText;
    StaticLayout* layout = b.build();
    state.mSelectionStart = state.mSelectionEnd =
            layout->getOffsetToLeftOf(state.mSelectionStart);
    delete layout;
}

} // namespace

TEST(StaticLayoutTest, testBuilder_textDirection) {
    TextPaint paint;
    {
        StaticLayout::Builder* builder = StaticLayout::Builder::obtain(
                new String(LAYOUT_TEXT), 0, (int) LAYOUT_TEXT.length(),
                &paint, DEFAULT_OUTER_WIDTH);
        StaticLayout* layout = builder->build();
        // AOSP asserts the heuristic POINTER identity; FIRSTSTRONG_LTR is the
        // documented default.
        EXPECT_EQ(TextDirectionHeuristics::FIRSTSTRONG_LTR,
                  layout->getTextDirectionHeuristic());
    }
    {
        StaticLayout::Builder* builder = StaticLayout::Builder::obtain(
                new String(LAYOUT_TEXT), 0, (int) LAYOUT_TEXT.length(),
                &paint, DEFAULT_OUTER_WIDTH);
        builder->setTextDirection(TextDirectionHeuristics::RTL);
        StaticLayout* layout = builder->build();
        EXPECT_EQ(TextDirectionHeuristics::RTL,
                  layout->getTextDirectionHeuristic());
    }
}

TEST(StaticLayoutTest, testGetters1) {
    TextPaint paint;
    LayoutBuilder b;
    Paint::FontMetricsInt fmi = paint.getFontMetricsInt();
    Layout* l = b.build();
    EXPECT_EQ(1, l->getLineCount());
    // Directions has no operator==; compare run count + all-LTR level 0
    // semantics via the vector payload (DIRS_ALL_LEFT_TO_RIGHT = {0,1}).
    const Directions* dirs = l->getLineDirections(0);
    ASSERT_NE(nullptr, dirs);
    EXPECT_EQ(1, dirs->getRunCount());
    EXPECT_EQ(0, dirs->getRunStart(0));
    // AOSP DIRS_ALL_LEFT_TO_RIGHT = {0, RUN_LENGTH_MASK}: the single run
    // extends to end-of-line (encoded as the full mask, not the line length).
    EXPECT_EQ(Layout::RUN_LENGTH_MASK, dirs->getRunLength(0));
    EXPECT_FALSE(dirs->isRunRtl(0));
    EXPECT_EQ(0, l->getEllipsisCount(0));
    EXPECT_EQ(0, l->getEllipsisStart(0));
    EXPECT_EQ(b.width, l->getEllipsizedWidth());
    delete l;
}

TEST(StaticLayoutTest, testLineMetrics_withPadding) {
    LayoutBuilder b;
    b.includePad = true;
    Paint::FontMetricsInt fmi = b.paint.getFontMetricsInt();

    Layout* l = b.build();
    assertVertMetrics(*l, fmi.top - fmi.ascent, fmi.bottom - fmi.descent,
            { { fmi.top, fmi.bottom, 0 } });
    delete l;
}

TEST(StaticLayoutTest, testLineMetrics_withPaddingAndWidth) {
    LayoutBuilder b;
    b.includePad = true;
    b.width = 50;
    Paint::FontMetricsInt fmi = b.paint.getFontMetricsInt();

    Layout* l = b.build();
    assertVertMetrics(*l, fmi.top - fmi.ascent, fmi.bottom - fmi.descent,
            { { fmi.top, fmi.descent, 0 },
              { fmi.ascent, fmi.bottom, 0 } });
    delete l;
}

TEST(StaticLayoutTest, testLineMetrics_withThreeLines) {
    LayoutBuilder b;
    b.text = u"This is a longer test";
    b.includePad = true;
    b.width = 50;
    Paint::FontMetricsInt fmi = b.paint.getFontMetricsInt();

    Layout* l = b.build();
    assertVertMetrics(*l, fmi.top - fmi.ascent, fmi.bottom - fmi.descent,
            { { fmi.top, fmi.descent, 0 },
              { fmi.ascent, fmi.descent, 0 },
              { fmi.ascent, fmi.bottom, 0 } });
    delete l;
}

TEST(StaticLayoutTest, testLineMetrics_withLargeText) {
    LayoutBuilder b;
    b.text = u"This is a longer test";
    b.includePad = true;
    b.width = 150;
    b.paint.setTextSize(36);
    Paint::FontMetricsInt fmi = b.paint.getFontMetricsInt();

    if (fmi.leading == 0) { // nothing to test
        GTEST_SKIP() << "leading is 0, skipping (AOSP does the same)";
    }

    Layout* l = b.build();
    assertVertMetrics(*l, fmi.top - fmi.ascent, fmi.bottom - fmi.descent,
            { { fmi.top, fmi.descent, 0 },
              { fmi.ascent, fmi.descent, 0 },
              { fmi.ascent, fmi.bottom, 0 } });
    delete l;
}

TEST(StaticLayoutTest, testLineMetrics_withSpacingAdd) {
    const int spacingAdd = 2; // int so expressions return int
    LayoutBuilder b;
    b.text = u"This is a longer test";
    b.includePad = true;
    b.width = 50;
    b.spacingAdd = spacingAdd;
    Paint::FontMetricsInt fmi = b.paint.getFontMetricsInt();

    Layout* l = b.build();
    assertVertMetrics(*l, fmi.top - fmi.ascent, fmi.bottom - fmi.descent,
            { { fmi.top, fmi.descent + spacingAdd, spacingAdd },
              { fmi.ascent, fmi.descent + spacingAdd, spacingAdd },
              { fmi.ascent, fmi.bottom, 0 } });
    delete l;
}

TEST(StaticLayoutTest, testLineMetrics_withSpacingMult) {
    LayoutBuilder b;
    b.text = u"This is a longer test";
    b.includePad = true;
    b.width = 50;
    b.spacingAdd = 2;
    b.spacingMult = 1.5f;
    Paint::FontMetricsInt fmi = b.paint.getFontMetricsInt();
    const Scaler s(b.spacingMult, b.spacingAdd);

    Layout* l = b.build();
    assertVertMetrics(*l, fmi.top - fmi.ascent, fmi.bottom - fmi.descent,
            { { fmi.top, fmi.descent + s.scale(fmi.descent - fmi.top),
                        s.scale(fmi.descent - fmi.top) },
              { fmi.ascent, fmi.descent + s.scale(fmi.descent - fmi.ascent),
                        s.scale(fmi.descent - fmi.ascent) },
              { fmi.ascent, fmi.bottom, 0 } });
    delete l;
}

TEST(StaticLayoutTest, testLineMetrics_withUnitIntervalSpacingMult) {
    LayoutBuilder b;
    b.text = u"This is a longer test";
    b.includePad = true;
    b.width = 50;
    b.spacingAdd = 2;
    b.spacingMult = .8f;
    Paint::FontMetricsInt fmi = b.paint.getFontMetricsInt();
    const Scaler s(b.spacingMult, b.spacingAdd);

    Layout* l = b.build();
    assertVertMetrics(*l, fmi.top - fmi.ascent, fmi.bottom - fmi.descent,
            { { fmi.top, fmi.descent + s.scale(fmi.descent - fmi.top),
                        s.scale(fmi.descent - fmi.top) },
              { fmi.ascent, fmi.descent + s.scale(fmi.descent - fmi.ascent),
                        s.scale(fmi.descent - fmi.ascent) },
              { fmi.ascent, fmi.bottom, 0 } });
    delete l;
}

TEST(StaticLayoutTest, testGetLineExtra_withNegativeValue) {
    LayoutBuilder b;
    Layout* layout = b.build();
    delete layout;
    // AOSP expects IndexOutOfBoundsException, but android-36 StaticLayout
    // (:1446) has no explicit check — the throw comes from the JVM's array
    // bounds enforcement. CDROID ports the same expression onto std::vector,
    // where operator[] out of range is UB, not an exception. Language-level
    // semantics difference, not a framework divergence; nothing assertable.
    GTEST_SKIP() << "JVM array-bounds exception has no C++ operator[] analogue";
}

TEST(StaticLayoutTest, testGetLineExtra_withParamGreaterThanLineCount) {
    LayoutBuilder b;
    Layout* layout = b.build();
    const int lineCount = layout->getLineCount();
    delete layout;
    // Same as testGetLineExtra_withNegativeValue: the AOSP throw is JVM array
    // bounds semantics; android-36 has no explicit check in getLineExtra.
    GTEST_SKIP() << "JVM array-bounds exception has no C++ operator[] analogue";
}

TEST(StaticLayoutTest, testDefaultGetLineExtra) {
    LayoutBuilder b;
    Layout* layout = b.build();
    const int lineCount = layout->getLineCount();
    for (int i = 0; i < lineCount; ++i) {
        EXPECT_EQ(0, layout->getLineExtra(i));
    }
    delete layout;
}

TEST(StaticLayoutTest, testEllipsis_singleLine) {
    TextPaint paint;
    {
        StaticLayout* layout = createEllipsizeStaticLayout(paint,
                LAYOUT_TEXT_SINGLE_LINE, TextUtils::TruncateAt::END, 1);
        EXPECT_GT(layout->getEllipsisCount(0), 0);
        delete layout;
    }
    {
        StaticLayout* layout = createEllipsizeStaticLayout(paint,
                LAYOUT_TEXT_SINGLE_LINE, TextUtils::TruncateAt::MIDDLE, 1);
        EXPECT_GT(layout->getEllipsisCount(0), 0);
        delete layout;
    }
    {
        StaticLayout* layout = createEllipsizeStaticLayout(paint,
                LAYOUT_TEXT_SINGLE_LINE, TextUtils::TruncateAt::MARQUEE, 1);
        EXPECT_EQ(0, layout->getEllipsisCount(0));
        delete layout;
    }
}

TEST(StaticLayoutTest, testEllipsis_startAndEnd_halfWidth) {
    TextPaint paint;
    const std::u16string text = u"あ" // HIRAGANA LETTER A
            u"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
    const float textWidth = paint.measureText(TextUtils::utf16_utf8(text));
    const int halfWidth = (int) (textWidth / 2.0f);
    {
        StaticLayout* layout = new StaticLayout(new String(text), 0,
                (int) text.length(), &paint, halfWidth, DEFAULT_ALIGN,
                TextDirectionHeuristics::FIRSTSTRONG_LTR,
                SPACE_MULTI, SPACE_ADD, false, TextUtils::TruncateAt::END,
                halfWidth, 1);
        EXPECT_GT(layout->getEllipsisCount(0), 0);
        EXPECT_GT(layout->getEllipsisStart(0), 0);
        delete layout;
    }
    {
        StaticLayout* layout = new StaticLayout(new String(text), 0,
                (int) text.length(), &paint, halfWidth, DEFAULT_ALIGN,
                TextDirectionHeuristics::FIRSTSTRONG_LTR,
                SPACE_MULTI, SPACE_ADD, false, TextUtils::TruncateAt::START,
                halfWidth, 1);
        EXPECT_GT(layout->getEllipsisCount(0), 0);
        delete layout;
    }
}

TEST(StaticLayoutTest, testRtlOffset_plainStringOnly) {
    // Normalizer-form variants are not portable (no java.text.Normalizer);
    // the plain-string pass mirrors the AOSP expectations.
    TextPaint paint;
    const std::u16string testString = u"מסעדה"; // Hebrew
    StaticLayout::Builder* b = StaticLayout::Builder::obtain(
            new String(testString), 0, (int) testString.length(), &paint,
            DEFAULT_OUTER_WIDTH);
    b->setAlignment(DEFAULT_ALIGN)
     .setTextDirection(TextDirectionHeuristics::RTL)
     .setLineSpacing(SPACE_ADD, SPACE_MULTI)
     .setIncludePad(true);
    StaticLayout* layout = b->build();

    EXPECT_EQ(1, layout->getOffsetToLeftOf(0));
    EXPECT_EQ(2, layout->getOffsetToLeftOf(1));
    EXPECT_EQ(3, layout->getOffsetToLeftOf(2));
    EXPECT_EQ(4, layout->getOffsetToLeftOf(3));
    EXPECT_EQ(5, layout->getOffsetToLeftOf(4));
    EXPECT_EQ(5, layout->getOffsetToLeftOf(5));

    EXPECT_EQ(0, layout->getOffsetToRightOf(0));
    EXPECT_EQ(0, layout->getOffsetToRightOf(1));
    EXPECT_EQ(1, layout->getOffsetToRightOf(2));
    EXPECT_EQ(2, layout->getOffsetToRightOf(3));
    EXPECT_EQ(3, layout->getOffsetToRightOf(4));
    EXPECT_EQ(4, layout->getOffsetToRightOf(5));
}

TEST(StaticLayoutTest, testEmojiOffset) {
    EditorState state, expected;
    TextPaint paint;
    auto expectRight = [&](const std::string& spec) {
        moveCursorToRightCursorableOffset(state, paint);
        expected.setByString(spec);
        EXPECT_EQ(expected.mSelectionStart, state.mSelectionStart) << spec;
    };
    auto expectLeft = [&](const std::string& spec) {
        moveCursorToLeftCursorableOffset(state, paint);
        expected.setByString(spec);
        EXPECT_EQ(expected.mSelectionStart, state.mSelectionStart) << spec;
    };

    // Odd numbered regional indicator symbols (flag pairs + leftover).
    state.setByString("| U+1F1E6 U+1F1E8 U+1F1E6 U+1F1E8 U+1F1E6");
    expectRight("U+1F1E6 U+1F1E8 | U+1F1E6 U+1F1E8 U+1F1E6");
    expectRight("U+1F1E6 U+1F1E8 U+1F1E6 U+1F1E8 | U+1F1E6");
    expectRight("U+1F1E6 U+1F1E8 U+1F1E6 U+1F1E8 U+1F1E6 |");
    expectLeft("U+1F1E6 U+1F1E8 U+1F1E6 U+1F1E8 | U+1F1E6");
    expectLeft("U+1F1E6 U+1F1E8 | U+1F1E6 U+1F1E8 U+1F1E6");
    expectLeft("| U+1F1E6 U+1F1E8 U+1F1E6 U+1F1E8 U+1F1E6");

    // Zero width joiner sequence (family emoji).
    const std::string zwj = "U+1F468 U+200D U+2764 U+FE0F U+200D U+1F468";
    state.setByString("| " + zwj + " " + zwj + " " + zwj);
    expectRight(zwj + " | " + zwj + " " + zwj);
    expectRight(zwj + " " + zwj + " | " + zwj);
    expectRight(zwj + " " + zwj + " " + zwj + " |");
    expectLeft(zwj + " " + zwj + " | " + zwj);
    expectLeft(zwj + " | " + zwj + " " + zwj);
    expectLeft("| " + zwj + " " + zwj + " " + zwj);

    // Emoji modifiers (index + skin tone).
    state.setByString("| U+261D U+1F3FB U+261D U+1F3FB U+261D U+1F3FB");
    expectRight("U+261D U+1F3FB | U+261D U+1F3FB U+261D U+1F3FB");
    expectRight("U+261D U+1F3FB U+261D U+1F3FB | U+261D U+1F3FB");
    expectRight("U+261D U+1F3FB U+261D U+1F3FB U+261D U+1F3FB |");
    expectLeft("U+261D U+1F3FB U+261D U+1F3FB | U+261D U+1F3FB");
    expectLeft("U+261D U+1F3FB | U+261D U+1F3FB U+261D U+1F3FB");
    expectLeft("| U+261D U+1F3FB U+261D U+1F3FB U+261D U+1F3FB");
}

// --- Skipped: API gaps recorded, not fixable from the test side -----------

TEST(StaticLayoutTest, SKIPPED_testLocaleSpanAffectsHyphenation) {
    GTEST_SKIP() << "needs LocaleSpan + Paint.setTextLocale + hyphenation "
                    "engine (not ported)";
}

TEST(StaticLayoutTest, testLayoutDoesntModifyPaint) {
    TextPaint paint;
    paint.setStartHyphenEdit(Paint::START_HYPHEN_EDIT_INSERT_HYPHEN);
    paint.setEndHyphenEdit(Paint::END_HYPHEN_EDIT_INSERT_HYPHEN);
    StaticLayout::Builder* b = StaticLayout::Builder::obtain(new String(u""), 0, 0, &paint, 100);
    StaticLayout* layout = b->build();  // build() recycles the builder
    Canvas canvas(100, 100);
    layout->drawText(canvas, 0, 0);
    EXPECT_EQ((int) Paint::START_HYPHEN_EDIT_INSERT_HYPHEN, paint.getStartHyphenEdit());
    EXPECT_EQ((int) Paint::END_HYPHEN_EDIT_INSERT_HYPHEN, paint.getEndHyphenEdit());
    delete layout;
}

TEST(StaticLayoutTest, SKIPPED_testFallbackLineSpacing) {
    GTEST_SKIP() << "needs the FontFallbackSetup harness + synthetic fonts "
                    "(not ported)";
}
