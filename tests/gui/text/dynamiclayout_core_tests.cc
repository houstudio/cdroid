// Ported from AOSP coretests DynamicLayoutTest (android.text.DynamicLayout).
// Original: frameworks/base/core/tests/coretests/src/android/text/DynamicLayoutTest.java (Apache 2.0)
//
// CDROID adaptation:
//  - getBlocksAlwaysNeedToBeRedrawn returns std::set<int> (AOSP: ArraySet or
//    null) — assertNull becomes EXPECT_TRUE(...empty()).
//  - testFallbackLineSpacing is SKIPPED: the FontFallbackSetup test helper
//    (synthetic font registry) is not ported.
//  - DynamicLayout.Builder::build returns DynamicLayout*; the builder is
//    pool-recycled by build() (do not delete).
#include <gtest/gtest.h>
#include <text/dynamiclayout.h>
#include <text/staticlayout.h>
#include <text/spannablestringbuilder.h>
#include <text/textpaint.h>
#include <text/String.h>
#include <text/textdirectionheuristics.h>
#include <text/style/replacementspan.h>

using namespace cdroid;

namespace {

constexpr int WIDTH = 10000;

class MockReplacementSpan : public ReplacementSpan {
public:
    int getSize(const Paint&, const CharSequence*, int, int, Paint::FontMetricsInt*) const override {
        return 10;
    }
    void draw(Canvas&, const CharSequence*, int, int, float, int, int, int, const Paint&) const override {}
};

} // namespace

TEST(CoreDynamicLayoutTest, testGetBlocksAlwaysNeedToBeRedrawn_en) {
    SpannableStringBuilder builder;
    TextPaint paint;
    DynamicLayout layout(&builder, &paint, WIDTH, Layout::Alignment::ALIGN_NORMAL, 0, 0, false);

    EXPECT_TRUE(layout.getBlocksAlwaysNeedToBeRedrawn().empty());

    builder.append(String(u"abcd efg\n"));
    builder.append(String(u"hijk lmn\n"));
    EXPECT_TRUE(layout.getBlocksAlwaysNeedToBeRedrawn().empty());

    builder.deleteText(0, (int)builder.length());
    EXPECT_TRUE(layout.getBlocksAlwaysNeedToBeRedrawn().empty());
}

TEST(CoreDynamicLayoutTest, testGetBlocksAlwaysNeedToBeRedrawn_replacementSpan) {
    SpannableStringBuilder builder;
    TextPaint paint;
    DynamicLayout layout(&builder, &paint, WIDTH, Layout::Alignment::ALIGN_NORMAL, 0, 0, false);

    EXPECT_TRUE(layout.getBlocksAlwaysNeedToBeRedrawn().empty());

    builder.append(String(u"abcd efg\n"));
    builder.append(String(u"hijk lmn\n"));
    EXPECT_TRUE(layout.getBlocksAlwaysNeedToBeRedrawn().empty());

    builder.setSpan(new MockReplacementSpan, 0, 4, Spannable::SPAN_EXCLUSIVE_EXCLUSIVE);
    EXPECT_FALSE(layout.getBlocksAlwaysNeedToBeRedrawn().empty());
    EXPECT_NE(0u, layout.getBlocksAlwaysNeedToBeRedrawn().count(0));

    builder.setSpan(new MockReplacementSpan, 9, 13, Spannable::SPAN_EXCLUSIVE_EXCLUSIVE);
    EXPECT_NE(0u, layout.getBlocksAlwaysNeedToBeRedrawn().count(0));
    EXPECT_NE(0u, layout.getBlocksAlwaysNeedToBeRedrawn().count(1));

    builder.deleteText(9, 13);
    EXPECT_NE(0u, layout.getBlocksAlwaysNeedToBeRedrawn().count(0));
    EXPECT_EQ(0u, layout.getBlocksAlwaysNeedToBeRedrawn().count(1));

    builder.deleteText(0, 4);
    EXPECT_EQ(0u, layout.getBlocksAlwaysNeedToBeRedrawn().count(0));
    EXPECT_TRUE(layout.getBlocksAlwaysNeedToBeRedrawn().empty());
}

TEST(CoreDynamicLayoutTest, testGetBlocksAlwaysNeedToBeRedrawn_thai) {
    SpannableStringBuilder builder;
    TextPaint paint;
    DynamicLayout layout(&builder, &paint, WIDTH, Layout::Alignment::ALIGN_NORMAL, 0, 0, false);

    EXPECT_TRUE(layout.getBlocksAlwaysNeedToBeRedrawn().empty());

    builder.append(String(u"ยินดีต้อนรับ"));
    builder.append(String(u"สู่"));
    EXPECT_TRUE(layout.getBlocksAlwaysNeedToBeRedrawn().empty());

    builder.append(String(u"่่่่่"));
    EXPECT_FALSE(layout.getBlocksAlwaysNeedToBeRedrawn().empty());
    EXPECT_NE(0u, layout.getBlocksAlwaysNeedToBeRedrawn().count(0));

    builder.deleteText((int)builder.length() - 5, (int)builder.length());
    EXPECT_EQ(0u, layout.getBlocksAlwaysNeedToBeRedrawn().count(0));
    EXPECT_TRUE(layout.getBlocksAlwaysNeedToBeRedrawn().empty());
}

namespace {

void checkLineExtraAgainstStatic(float spacingMultiplier, float spacingAdd, bool checkLastLine) {
    SpannableStringBuilder text(u"a\nb\nc");
    TextPaint textPaint;

    StaticLayout::Builder* sb = StaticLayout::Builder::obtain(
            &text, 0, (int)text.length(), &textPaint, WIDTH);
    sb->setAlignment(Layout::Alignment::ALIGN_NORMAL).setIncludePad(false)
      .setLineSpacing(spacingAdd, spacingMultiplier);
    StaticLayout* staticLayout = sb->build();

    DynamicLayout dynamicLayout(&text, &textPaint, WIDTH, Layout::Alignment::ALIGN_NORMAL,
                                spacingMultiplier, spacingAdd, false /*includepad*/);

    const int lineCount = staticLayout->getLineCount();
    ASSERT_EQ(lineCount, dynamicLayout.getLineCount());
    for (int i = 0; i < lineCount - (checkLastLine ? 0 : 1); i++) {
        EXPECT_EQ(staticLayout->getLineExtra(i), dynamicLayout.getLineExtra(i)) << "line " << i;
    }
    delete staticLayout;
}

} // namespace

TEST(CoreDynamicLayoutTest, testGetLineExtra_withoutLinespacing) {
    checkLineExtraAgainstStatic(1.0f, 0.0f, true);
}

TEST(CoreDynamicLayoutTest, testGetLineExtra_withLinespacing) {
    checkLineExtraAgainstStatic(2.0f, 4.0f, false);
}

TEST(CoreDynamicLayoutTest, DISABLED_testGetLineExtra_withNegativeValue) {
    // AOSP expects IndexOutOfBoundsException; not expressible without Java exceptions.
}

TEST(CoreDynamicLayoutTest, DISABLED_testGetLineExtra_withParamGreaterThanLineCount) {
    // AOSP expects IndexOutOfBoundsException; not expressible without Java exceptions.
}

TEST(CoreDynamicLayoutTest, testReflow_afterSpannableEdit) {
    const std::u16string text = u"a\nb:🌚 c \n🌚";
    const int length = (int)text.length();
    SpannableStringBuilder spannable(text);
    spannable.setSpan(new MockReplacementSpan, 4, 6, Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);
    spannable.setSpan(new MockReplacementSpan, 10, length, Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);

    TextPaint paint;
    DynamicLayout layout(&spannable, &paint, WIDTH, Layout::Alignment::ALIGN_NORMAL,
                         1.0f /*spacingMultiplier*/, 0.0f /*spacingAdd*/, false /*includepad*/);

    spannable.deleteText(8, 9);
    spannable.replace(7, 8, String(u"ch"));

    layout.reflow(&spannable, 0, length, length);
    for (int value : layout.getBlocksAlwaysNeedToBeRedrawn()) {
        EXPECT_GE(value, 0) << "Block index should not be negative";
    }
}

TEST(CoreDynamicLayoutTest, DISABLED_testFallbackLineSpacing) {
    // Needs the FontFallbackSetup test helper (synthetic font registry) —
    // not ported.
}

TEST(CoreDynamicLayoutTest, testBuilder_defaultTextDirection) {
    String text(u"");
    TextPaint paint;
    DynamicLayout::Builder* builder = DynamicLayout::Builder::obtain(&text, &paint, WIDTH);
    DynamicLayout* layout = builder->build();
    EXPECT_EQ(TextDirectionHeuristics::FIRSTSTRONG_LTR, layout->getTextDirectionHeuristic());
    delete layout;
}

TEST(CoreDynamicLayoutTest, testBuilder_setTextDirection) {
    String text(u"");
    TextPaint paint;
    DynamicLayout::Builder* builder = DynamicLayout::Builder::obtain(&text, &paint, WIDTH);
    builder->setTextDirection(TextDirectionHeuristics::ANYRTL_LTR);
    DynamicLayout* layout = builder->build();
    EXPECT_EQ(TextDirectionHeuristics::ANYRTL_LTR, layout->getTextDirectionHeuristic());
    delete layout;
}
