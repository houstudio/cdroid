/*********************************************************************************
 * Ported from AOSP coretests android.text.style.UnderlineSpanTest (Apache 2.0).
 *
 * A width-only assertion: a colored-underline UnderlineSpan subclass must not
 * change the measured line width (MetricAffectingSpan vs draw-only span).
 *********************************************************************************/
#include <text/spannablestring.h>
#include <text/staticlayout.h>
#include <text/String.h>
#include <text/textpaint.h>
#include <text/style/characterstyles.h>
#include <gtest/gtest.h>
#include <string>

using namespace cdroid;

namespace {

class RedUnderlineSpan : public UnderlineSpan {
public:
    void updateDrawState(TextPaint& ds) const override {
        ds.setUnderlineText(0xFFFF0000, 1.0f);
    }
};

// Measures the width of some potentially-spanned text, assuming it's not too wide.
static float textWidth(const CharSequence& text) {
    TextPaint tp;
    tp.setTextSize(100.0f); // Large enough so that the difference in kerning is visible.
    const int largeWidth = 10000; // Enough width so the whole text fits in one line.
    StaticLayout* layout = StaticLayout::Builder::obtain(
            const_cast<CharSequence*>(&text), 0, (int) text.length(), &tp, largeWidth)->build();
    return layout->getLineWidth(0);
}

// Identical to the normal UnderlineSpan test, except that a subclass of UnderlineSpan is used
// that draws a red underline. This shouldn't affect width either.
TEST(CoreUnderlineSpanTest, testDoesntAffectWidth_colorUnderlineSubclass) {
    // Roboto kerns between "P" and "."
    SpannableString* text = new SpannableString(u"P.");
    const float origLineWidth = textWidth(*text);
    // Underline just the "P".
    text->setSpan(new RedUnderlineSpan(), 0, 1, Spanned::SPAN_INCLUSIVE_INCLUSIVE);
    const float underlinedLineWidth = textWidth(*text);
    EXPECT_FLOAT_EQ(origLineWidth, underlinedLineWidth);
}

} // namespace
