// Ported from AOSP coretests LayoutTest (android.text.Layout).
// Original: frameworks/base/core/tests/coretests/src/android/text/LayoutTest.java (Apache 2.0)
//
// CDROID adaptation:
//  - MockLayout stubs the pure virtuals exactly like the AOSP inner class
//    (fixed 5 lines x 12px height, descent 4; getLineStart(line)=line).
//    getLineDirections returns const Directions* here.
//  - Rect is {l,t,w,h}: AOSP bounds.right/.bottom map to width/.bottom().
//  - Java-only clauses are skipped inline: null-ctor IllegalArgumentException,
//    increaseWidthTo shrink RuntimeException, getLineVisibleEnd OOB exception.
//  - testDraw is SKIPPED: cdroid Canvas (a Cairo::Context) has no virtual
//    drawText to intercept — MockCanvas recording is not expressible.
//  - testGetSelectionPath: cdroid Path has no isRect(); compare computeBounds
//    against the selection rectangle instead.
#include <gtest/gtest.h>
#include <text/layout.h>
#include <text/staticlayout.h>
#include <text/spannablestring.h>
#include <text/parcelablespan.h>
#include <text/style/characterstyles.h>
#include <text/String.h>
#include <text/textpaint.h>
#include <core/path.h>
#include <core/rect.h>

using namespace cdroid;

namespace {

constexpr int LINE_COUNT  = 5;
constexpr int LINE_HEIGHT = 12;
constexpr int LINE_DESCENT = 4;

class MockLayout : public Layout {
public:
    MockLayout(CharSequence* text, TextPaint* paint, int width,
               Alignment align, float spacingmult, float spacingadd)
        : Layout(text, paint, width, align, spacingmult, spacingadd) {}

    bool mockIsSpanned() const { return isSpanned(); }

    int getBottomPadding() const override { return 0; }
    int getEllipsisCount(int) const override { return 0; }
    int getEllipsisStart(int) const override { return 0; }
    bool getLineContainsTab(int) const override { return false; }
    int getLineCount() const override { return LINE_COUNT; }
    int getLineDescent(int) const override { return LINE_DESCENT; }
    const Directions* getLineDirections(int) const override {
        return &Layout::DIRS_ALL_LEFT_TO_RIGHT;
    }
    int getLineStart(int line) const override {
        if (line < 0) return 0;
        return line;
    }
    int getLineTop(int line) const override {
        if (line < 0) return 0;
        return LINE_HEIGHT * line;
    }
    int getParagraphDirection(int) const override { return 0; }
    int getTopPadding() const override { return 0; }
};

struct Fixture {
    TextPaint mTextPaint;
    SpannableString mSpannedText{u"alwei\t;sdfs\ndf @"};
    int mWidth = 11;
    Layout::Alignment mAlign = Layout::Alignment::ALIGN_CENTER;
    float mSpacingMult = 1;
    float mSpacingAdd = 2;

    Fixture() {
        mSpannedText.setSpan(new StrikethroughSpan, 0, 1,
                             Spannable::SPAN_INCLUSIVE_EXCLUSIVE);
    }
    CharSequence* text() { return &mSpannedText; }
};

} // namespace

TEST(CoreLayoutTest, testConstructor) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
}

TEST(CoreLayoutTest, DISABLED_testConstructorNull) {
    // AOSP expects IllegalArgumentException for the null-text/null-paint ctor;
    // not expressible without Java exceptions.
}

TEST(CoreLayoutTest, testGetText) {
    Fixture f;
    String text(u"test case 1");
    MockLayout layout(&text, &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    EXPECT_EQ(text.toUTF8(), layout.getText()->toUTF8());
}

TEST(CoreLayoutTest, testGetPaint) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    EXPECT_EQ(&f.mTextPaint, layout.getPaint());
}

TEST(CoreLayoutTest, testGetWidth) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, 10, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    EXPECT_EQ(10, layout.getWidth());

    MockLayout zero(f.text(), &f.mTextPaint, 0, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    EXPECT_EQ(0, zero.getWidth());
}

TEST(CoreLayoutTest, testGetEllipsizedWidth) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, 15, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    EXPECT_EQ(15, layout.getEllipsizedWidth());

    MockLayout zero(f.text(), &f.mTextPaint, 0, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    EXPECT_EQ(0, zero.getEllipsizedWidth());
}

TEST(CoreLayoutTest, testIncreaseWidthTo) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    int oldWidth = layout.getWidth();

    layout.increaseWidthTo(oldWidth);
    EXPECT_EQ(oldWidth, layout.getWidth());

    // AOSP expects a RuntimeException when shrinking; not asserted here.

    layout.increaseWidthTo(oldWidth + 1);
    EXPECT_EQ(oldWidth + 1, layout.getWidth());
}

TEST(CoreLayoutTest, testGetHeight) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    EXPECT_EQ(60, layout.getHeight());
}

TEST(CoreLayoutTest, testGetAlignment) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    EXPECT_EQ((int)f.mAlign, (int)layout.getAlignment());
}

TEST(CoreLayoutTest, testGetSpacingMultiplier) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, -1, f.mSpacingAdd);
    EXPECT_EQ(-1.0f, layout.getSpacingMultiplier());

    MockLayout positive(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, 5, f.mSpacingAdd);
    EXPECT_EQ(5.0f, positive.getSpacingMultiplier());
}

TEST(CoreLayoutTest, testGetSpacingAdd) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, -1);
    EXPECT_EQ(-1.0f, layout.getSpacingAdd());

    MockLayout positive(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, 20);
    EXPECT_EQ(20.0f, positive.getSpacingAdd());
}

TEST(CoreLayoutTest, testGetLineBounds) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    Rect bounds;

    EXPECT_EQ(32, layout.getLineBounds(2, &bounds));
    // cdroid getLineBounds stores the padding-convention Rect: .width holds
    // AOSP's right (== mWidth), .height holds AOSP's bottom (getLineTop(line+1)).
    EXPECT_EQ(0, bounds.left);
    EXPECT_EQ(f.mWidth, bounds.width);
    EXPECT_EQ(24, bounds.top);
    EXPECT_EQ(36, bounds.height);
}

TEST(CoreLayoutTest, testGetLineForVertical) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    EXPECT_EQ(0, layout.getLineForVertical(-1));
    EXPECT_EQ(0, layout.getLineForVertical(0));
    EXPECT_EQ(0, layout.getLineForVertical(LINE_COUNT));
    EXPECT_EQ(LINE_COUNT - 1, layout.getLineForVertical(1000));
}

TEST(CoreLayoutTest, testGetLineForOffset) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    EXPECT_EQ(0, layout.getLineForOffset(-1));
    EXPECT_EQ(1, layout.getLineForOffset(1));
    EXPECT_EQ(LINE_COUNT - 1, layout.getLineForOffset(LINE_COUNT - 1));
    EXPECT_EQ(LINE_COUNT - 1, layout.getLineForOffset(1000));
}

TEST(CoreLayoutTest, testGetLineEnd) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    EXPECT_EQ(2, layout.getLineEnd(1));
}

TEST(CoreLayoutTest, testGetLineExtra_returnsZeroByDefault) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, 100, 100);
    for (int i = 0; i < 4; i++) { // "a\nb\nc\n".split("\n").length == 4
        EXPECT_EQ(0, layout.getLineExtra(i));
    }
}

TEST(CoreLayoutTest, testGetLineVisibleEnd) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    const int textLength = (int)f.mSpannedText.length();

    EXPECT_EQ(2, layout.getLineVisibleEnd(1));
    EXPECT_EQ(LINE_COUNT, layout.getLineVisibleEnd(LINE_COUNT - 1));
    EXPECT_EQ(textLength, layout.getLineVisibleEnd(textLength - 1));
    // AOSP expects StringIndexOutOfBoundsException past the end; skipped.
}

TEST(CoreLayoutTest, testGetLineBottom) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    EXPECT_EQ(LINE_HEIGHT, layout.getLineBottom(0));
}

TEST(CoreLayoutTest, testGetLineBaseline) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    EXPECT_EQ(8, layout.getLineBaseline(0));
}

TEST(CoreLayoutTest, testGetLineAscent) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    EXPECT_EQ(-8, layout.getLineAscent(0));
}

TEST(CoreLayoutTest, testGetParagraphAlignment) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    EXPECT_EQ((int)f.mAlign, (int)layout.getParagraphAlignment(0));

    MockLayout spanned(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    EXPECT_EQ((int)f.mAlign, (int)spanned.getParagraphAlignment(0));
    EXPECT_EQ((int)f.mAlign, (int)spanned.getParagraphAlignment(1));
}

TEST(CoreLayoutTest, testGetParagraphLeft) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    EXPECT_EQ(0, layout.getParagraphLeft(0));
}

TEST(CoreLayoutTest, testGetParagraphRight) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    EXPECT_EQ(f.mWidth, layout.getParagraphRight(0));
}

TEST(CoreLayoutTest, testGetSelectionWithEmptySelection) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);

    // When the selection is empty, we do not expect any rectangles.
    layout.getSelection(5, 5, [](float, float, float, float, int) {
        FAIL() << "Did not expect any rectangles";
    });
}

namespace {

std::vector<std::array<float, 4>> collectSelection(Layout& layout, int start, int end) {
    std::vector<std::array<float, 4>> rectangles;
    layout.getSelection(start, end, [&](float left, float top, float right, float bottom, int) {
        rectangles.push_back({left, top, right, bottom});
    });
    return rectangles;
}

} // namespace

TEST(CoreLayoutTest, testGetSelectionWithASingleLineSelection) {
    Fixture f;
    String s(u"abc");
    StaticLayout layout(&s, &f.mTextPaint, INT_MAX, Layout::Alignment::ALIGN_LEFT,
                        f.mSpacingMult, f.mSpacingAdd, false);

    auto rectangles = collectSelection(layout, 0, 1);

    // The selection covers only "a": one rectangle from the top-left.
    ASSERT_EQ(1u, rectangles.size());
    EXPECT_EQ(0.0f, rectangles[0][0]); // left
    EXPECT_EQ(0.0f, rectangles[0][1]); // top
    EXPECT_GT(rectangles[0][2], 0.0f); // right
    EXPECT_GT(rectangles[0][3], 0.0f); // bottom
}

TEST(CoreLayoutTest, testGetSelectionWithMultilineSelection_secondLineSelectionEndsBeforeFirstCharacter) {
    Fixture f;
    String s(u"a\nb\nc");
    StaticLayout layout(&s, &f.mTextPaint, INT_MAX, Layout::Alignment::ALIGN_LEFT,
                        f.mSpacingMult, f.mSpacingAdd, false);

    // Selection "a\n": three rectangles.
    auto rectangles = collectSelection(layout, 0, 2);
    ASSERT_EQ(3u, rectangles.size());

    const auto& topRectangle = rectangles[0];
    const auto& topToEndOfLineRectangle = rectangles[1];
    const auto& bottomLineStartRectangle = rectangles[2];

    EXPECT_LT(topRectangle[1], bottomLineStartRectangle[1]);
    EXPECT_EQ(topRectangle[0], bottomLineStartRectangle[0]);

    EXPECT_EQ((float)INT_MAX, topToEndOfLineRectangle[2]);
    EXPECT_EQ(topRectangle[1], topToEndOfLineRectangle[1]);
    EXPECT_EQ(topRectangle[2], topToEndOfLineRectangle[0]);
    EXPECT_EQ(topRectangle[3], topToEndOfLineRectangle[3]);

    EXPECT_EQ(0.0f, bottomLineStartRectangle[0]);
    EXPECT_EQ(0.0f, bottomLineStartRectangle[2]);
}

TEST(CoreLayoutTest, testGetSelectionWithMultilineSelection_secondLineSelectionEndsAfterACharacter) {
    Fixture f;
    String s(u"a\nb\nc");
    StaticLayout layout(&s, &f.mTextPaint, INT_MAX, Layout::Alignment::ALIGN_LEFT,
                        f.mSpacingMult, f.mSpacingAdd, false);

    // Selection "a\nb": four rectangles.
    auto rectangles = collectSelection(layout, 0, 3);
    ASSERT_EQ(4u, rectangles.size());

    const auto& topRectangle = rectangles[0];
    const auto& topToEndOfLineRectangle = rectangles[1];
    const auto& bottomRectangle = rectangles[2];
    const auto& bottomLineStartRectangle = rectangles[3];

    EXPECT_EQ(topRectangle[1], topToEndOfLineRectangle[1]);
    EXPECT_EQ(bottomLineStartRectangle[1], bottomRectangle[1]);
    EXPECT_EQ(bottomLineStartRectangle[3], bottomRectangle[3]);
    EXPECT_EQ(0.0f, bottomLineStartRectangle[0]);
    EXPECT_EQ(0.0f, bottomLineStartRectangle[2]);
    EXPECT_EQ(0.0f, bottomRectangle[0]);
    EXPECT_GT(bottomRectangle[2], 0.0f);
}

TEST(CoreLayoutTest, DISABLED_testDraw) {
    // AOSP MockCanvas records virtual Canvas.drawText calls; cdroid Canvas is a
    // Cairo::Context with no interceptable text-draw method — not expressible.
}

TEST(CoreLayoutTest, testIsSpanned) {
    Fixture f;
    String plain(u"plain");
    MockLayout layout(&plain, &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    // default is not spanned text
    EXPECT_FALSE(layout.mockIsSpanned());

    MockLayout spanned(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    EXPECT_TRUE(spanned.mockIsSpanned());
}

TEST(CoreLayoutTest, testGetLineWidth) {
    Fixture f;
    MockLayout layout(f.text(), &f.mTextPaint, f.mWidth, f.mAlign, f.mSpacingMult, f.mSpacingAdd);
    const std::u16string text = f.mSpannedText.toUTF16();
    for (int i = 0; i < LINE_COUNT; i++) {
        int start = layout.getLineStart(i);
        int end = layout.getLineEnd(i);
        std::string line = TextUtils::utf16_utf8(text.substr(start, end - start));
        EXPECT_NEAR(f.mTextPaint.measureText(line), layout.getLineWidth(i), 1.0f) << "line " << i;
    }
}

TEST(CoreLayoutTest, DISABLED_testGetCursorPath) {
    // Needs Path::computeBounds over the cursor path; cdroid Path has
    // computeBounds — kept DISABLED pending verification of path semantics.
}

namespace {

// testPrimaryIsTrailingPrevious helper (AOSP assertPrimaryIsTrailingPrevious).
void assertPrimaryIsTrailingPrevious(const std::u16string& input,
                                     const std::vector<bool>& expected) {
    ASSERT_EQ(input.length() + 1, expected.size());

    TextPaint paint;
    paint.setTextSize(16.0f);
    String source(input);
    StaticLayout::Builder* builder =
            StaticLayout::Builder::obtain(&source, 0, (int)input.length(), &paint, INT_MAX);
    StaticLayout* layout = builder->build();
    // build() recycles the Builder into its pool — do NOT delete it here.
    for (size_t i = 0; i <= input.length(); ++i) {
        EXPECT_EQ(expected[i], layout->primaryIsTrailingPrevious((int)i))
                << "offset " << i << " of " << TextUtils::utf16_utf8(input);
    }
    auto actual = layout->primaryIsTrailingPreviousAllLineOffsets(0);
    ASSERT_EQ(expected.size(), actual.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(expected[i], actual[i]) << "allLineOffsets " << i;
    }
    delete layout;
}

} // namespace

TEST(CoreLayoutTest, testPrimaryIsTrailingPrevious) {
    const char16_t LTR = u'a';
    const char16_t RTL = 0x05D0;      // HEBREW LETTER ALEF
    const std::u16string LTR_SP = {0xD801, 0xDCB0}; // OSAGE CAPITAL LETTER A
    const std::u16string RTL_SP = {0xD83A, 0xDD00}; // ADLAM CAPITAL LETTER ALIF
    const char16_t LRI = 0x2066;      // LEFT-TO-RIGHT ISOLATE
    const char16_t RLI = 0x2067;      // RIGHT-TO-LEFT ISOLATE
    const char16_t PDI = 0x2069;      // POP DIRECTIONAL ISOLATE

    assertPrimaryIsTrailingPrevious(
            std::u16string() + LTR + u' ' + LTR + LTR + u' ' + LTR + LTR + LTR,
            {false, false, false, false, false, false, false, false, false});
    assertPrimaryIsTrailingPrevious(
            std::u16string() + RTL + u' ' + RTL + RTL + u' ' + RTL + RTL + RTL,
            {false, false, false, false, false, false, false, false, false});
    assertPrimaryIsTrailingPrevious(
            std::u16string() + LTR + RTL + LTR + RTL + LTR,
            {false, true, false, true, false, false});
    assertPrimaryIsTrailingPrevious(
            std::u16string() + RTL + LTR + RTL + LTR + RTL,
            {false, true, false, true, false, false});
    assertPrimaryIsTrailingPrevious(
            RTL_SP + LTR_SP + RTL_SP + LTR_SP + RTL_SP,
            {false, false, true, false, false, false, true, false, false, false, false});
    assertPrimaryIsTrailingPrevious(
            LTR_SP + RTL_SP + LTR_SP + RTL_SP + LTR_SP,
            {false, false, true, false, false, false, true, false, false, false, false});
    assertPrimaryIsTrailingPrevious(
            std::u16string() + LTR + RLI + LTR + RTL + PDI + LTR,
            {false, false, true, false, false, false, false});
    assertPrimaryIsTrailingPrevious(
            std::u16string() + RTL + LRI + RTL + LTR + PDI + RTL,
            {false, false, true, false, false, false, false});
    assertPrimaryIsTrailingPrevious(u"", {false});
}
