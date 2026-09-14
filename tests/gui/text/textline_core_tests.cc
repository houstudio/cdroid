// Ported from AOSP coretests TextLineTest (android.text.TextLine).
// Original: frameworks/base/core/tests/coretests/src/android/text/TextLineTest.java (Apache 2.0)
//
// CDROID adaptation:
//  - Exact-width font from assets/font/ via the new Typeface::createFromAsset
//    (same StaticLayoutLineBreakingTestFont; U+0049 I=1em, U+05D0 alef=1em,
//    U+05D1 bet=5em, U+0020=10em ...).
//  - TextLine::justify takes a justificationMode here — pass
//    JUSTIFICATION_MODE_INTER_WORD (the only mode AOSP-12 justify() had).
//  - TabStopSpan.Standard is not ported — local StandardTabStopSpan provides
//    the same getTabStop()=100.
//  - TextLine::set takes one extra trailing bool (useFallbackLineSpacing) —
//    false, matching the AOSP-12 call shape.
#include <gtest/gtest.h>
#include <text/textline.h>
#include <text/layout.h>
#include <text/staticlayout.h>
#include <text/textpaint.h>
#include <text/String.h>
#include <text/spannablestringbuilder.h>
#include <text/textdirectionheuristics.h>
#include <core/typeface.h>
#include <text/style/tabstopspan.h>
#include <text/style/replacementspan.h>

using namespace cdroid;

namespace {

Typeface* testTypeface() {
    static Typeface* face = Typeface::createFromAsset("font/StaticLayoutLineBreakingTestFont.ttf");
    return face;
}

struct StandardTabStopSpan : public TabStopSpan {
    int tab;
    explicit StandardTabStopSpan(int t) : tab(t) {}
    int getTabStop() const override { return tab; }
};

// getTextLine's TextLine borrows a String and a Directions pointer into a
// StaticLayout's line storage — the {layout, source} pair is registered here
// and released by releaseTextLine (TextLine::recycle clears mText/mDirections,
// so nothing references the pair after the recycle).
std::vector<std::pair<StaticLayout*, String*>>& textLineBacking() {
    static std::vector<std::pair<StaticLayout*, String*>> backing;
    return backing;
}

TextLine* getTextLine(const std::u16string& str, TextPaint& paint, TabStops* tabStops = nullptr) {
    // TextLine::set BORROWS the CharSequence AND the Directions (a pointer into
    // the layout's line storage): both must outlive the line. AOSP leans on GC;
    // heap-allocate both and hand the pair to releaseTextLine instead of
    // letting stack objects dangle after this helper returns.
    String* source = new String(str);
    StaticLayout::Builder* builder =
            StaticLayout::Builder::obtain(source, 0, (int)str.length(), &paint, INT_MAX);
    StaticLayout* layout = builder->build(); // recycles the builder
    TextLine* tl = TextLine::obtain();
    tl->set(&paint, source, 0, (int)str.length(),
            TextDirectionHeuristics::FIRSTSTRONG_LTR->isRtl(source, 0, (int)str.length()) ? -1 : 1,
            layout->getLineDirections(0), tabStops != nullptr, tabStops,
            0, 0 /* no ellipsis */, false /* useFallbackLineSpacing */);
    textLineBacking().push_back({layout, source});
    return tl;
}

void releaseTextLine(TextLine* tl) {
    TextLine::recycle(tl);   // clears mText/mDirections — the backing pair dies below
    auto& backing = textLineBacking();
    for (auto& entry : backing) {
        delete entry.first;
        delete entry.second;
    }
    backing.clear();
}

void assertMeasurements(TextLine* tl, int length, bool trailing, const std::vector<float>& expected) {
    ASSERT_EQ((size_t)length + 1, expected.size());
    for (int offset = 0; offset <= length; ++offset) {
        EXPECT_FLOAT_EQ(expected[offset], tl->measure(offset, trailing, nullptr))
                << "offset " << offset << (trailing ? " trailing" : " leading");
    }
    std::vector<bool> trailings(length + 1, trailing);
    auto allMeasurements = tl->measureAllOffsets(trailings, nullptr);
    ASSERT_EQ(expected.size(), allMeasurements.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_FLOAT_EQ(expected[i], allMeasurements[i]) << "allOffsets " << i;
    }
}

bool stretchesToFullWidth(const std::u16string& line) {
    TextPaint paint;
    String text(line);
    TextLine* tl = TextLine::obtain();
    tl->set(&paint, &text, 0, (int)text.length(), Layout::DIR_LEFT_TO_RIGHT,
            &Layout::DIRS_ALL_LEFT_TO_RIGHT, false /* hasTabs */, nullptr /* tabStops */,
            0, 0 /* no ellipsis */, false);
    const float originalWidth = tl->metrics(nullptr);
    const float expandedWidth = 2 * originalWidth;

    tl->justify(Layout::JUSTIFICATION_MODE_INTER_WORD, expandedWidth);
    const float newWidth = tl->metrics(nullptr);
    releaseTextLine(tl);
    return std::abs(newWidth - expandedWidth) < 0.5;
}

class TestReplacementSpan : public ReplacementSpan {
public:
    mutable bool mIsUsed = false;
    int getSize(const Paint&, const CharSequence*, int, int, Paint::FontMetricsInt*) const override {
        mIsUsed = true;
        return 0;
    }
    void draw(Canvas&, const CharSequence*, int, int, float, int, int, int, const Paint&) const override {
        mIsUsed = true;
    }
};

} // namespace

TEST(CoreTextLineTest, testJustify_spaces) {
    // There are no spaces to stretch.
    EXPECT_FALSE(stretchesToFullWidth(u"text"));

    EXPECT_TRUE(stretchesToFullWidth(u"one space"));
    EXPECT_TRUE(stretchesToFullWidth(u"exactly two spaces"));
    EXPECT_TRUE(stretchesToFullWidth(u"up to three spaces"));
}

TEST(CoreTextLineTest, DISABLED_testJustify_NBSP) {
    // AOSP @Suppress (b/68204709): NBSP stretching unsupported upstream too.
}

TEST(CoreTextLineTest, testMeasure_LTR) {
    TextPaint paint;
    paint.setTypeface(testTypeface());
    paint.setTextSize(10.0f);  // make 1em = 10px

    TextLine* tl = getTextLine(u"IIIIIV", paint);
    assertMeasurements(tl, 6, false,
                       {0.0f, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 100.0f});
    assertMeasurements(tl, 6, true,
                       {0.0f, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 100.0f});
    releaseTextLine(tl);
}

TEST(CoreTextLineTest, testMeasure_RTL) {
    TextPaint paint;
    paint.setTypeface(testTypeface());
    paint.setTextSize(10.0f);

    TextLine* tl = getTextLine(u"אאאאאב", paint);
    assertMeasurements(tl, 6, false,
                       {0.0f, -10.0f, -20.0f, -30.0f, -40.0f, -50.0f, -100.0f});
    assertMeasurements(tl, 6, true,
                       {0.0f, -10.0f, -20.0f, -30.0f, -40.0f, -50.0f, -100.0f});
    releaseTextLine(tl);
}

TEST(CoreTextLineTest, testMeasure_BiDi) {
    TextPaint paint;
    paint.setTypeface(testTypeface());
    paint.setTextSize(10.0f);

    TextLine* tl = getTextLine(u"IIאאII", paint);
    assertMeasurements(tl, 6, false,
                       {0.0f, 10.0f, 40.0f, 30.0f, 40.0f, 50.0f, 60.0f});
    assertMeasurements(tl, 6, true,
                       {0.0f, 10.0f, 20.0f, 30.0f, 20.0f, 50.0f, 60.0f});
    releaseTextLine(tl);
}

TEST(CoreTextLineTest, testMeasure_BiDi2) {
    TextPaint paint;
    paint.setTypeface(testTypeface());
    paint.setTextSize(10.0f);

    TextLine* tl = getTextLine(u"I⁧Iאא⁩I", paint);
    assertMeasurements(tl, 7, false,
                       {0.0f, 10.0f, 30.0f, 30.0f, 20.0f, 40.0f, 40.0f, 50.0f});
    assertMeasurements(tl, 7, true,
                       {0.0f, 10.0f, 10.0f, 40.0f, 20.0f, 10.0f, 40.0f, 50.0f});
    releaseTextLine(tl);
}

TEST(CoreTextLineTest, testMeasure_BiDi3) {
    TextPaint paint;
    paint.setTypeface(testTypeface());
    paint.setTextSize(10.0f);

    TextLine* tl = getTextLine(u"א⁦אII⁩א", paint);
    assertMeasurements(tl, 7, false,
                       {0.0f, -10.0f, -30.0f, -30.0f, -20.0f, -40.0f, -40.0f, -50.0f});
    assertMeasurements(tl, 7, true,
                       {0.0f, -10.0f, -10.0f, -40.0f, -20.0f, -10.0f, -40.0f, -50.0f});
    releaseTextLine(tl);
}

TEST(CoreTextLineTest, testMeasure_Tab_LTR) {
    StandardTabStopSpan tabSpan(100);
    TabStops stops(100, {&tabSpan});
    TextPaint paint;
    paint.setTypeface(testTypeface());
    paint.setTextSize(10.0f);

    TextLine* tl = getTextLine(u"II\tII", paint, &stops);
    assertMeasurements(tl, 5, false,
                       {0.0f, 10.0f, 20.0f, 100.0f, 110.0f, 120.0f});
    assertMeasurements(tl, 5, true,
                       {0.0f, 10.0f, 20.0f, 100.0f, 110.0f, 120.0f});
    releaseTextLine(tl);
}

TEST(CoreTextLineTest, testMeasure_Tab_RTL) {
    StandardTabStopSpan tabSpan(100);
    TabStops stops(100, {&tabSpan});
    TextPaint paint;
    paint.setTypeface(testTypeface());
    paint.setTextSize(10.0f);

    TextLine* tl = getTextLine(u"אא\tאא", paint, &stops);
    assertMeasurements(tl, 5, false,
                       {0.0f, -10.0f, -20.0f, -100.0f, -110.0f, -120.0f});
    assertMeasurements(tl, 5, true,
                       {0.0f, -10.0f, -20.0f, -100.0f, -110.0f, -120.0f});
    releaseTextLine(tl);
}

TEST(CoreTextLineTest, testMeasure_Tab_BiDi) {
    StandardTabStopSpan tabSpan(100);
    TabStops stops(100, {&tabSpan});
    TextPaint paint;
    paint.setTypeface(testTypeface());
    paint.setTextSize(10.0f);

    TextLine* tl = getTextLine(u"Iא\tIא", paint, &stops);
    assertMeasurements(tl, 5, false,
                       {0.0f, 20.0f, 20.0f, 100.0f, 120.0f, 120.0f});
    assertMeasurements(tl, 5, true,
                       {0.0f, 10.0f, 10.0f, 100.0f, 110.0f, 110.0f});
    releaseTextLine(tl);
}

TEST(CoreTextLineTest, testMeasure_Tab_BiDi2) {
    StandardTabStopSpan tabSpan(100);
    TabStops stops(100, {&tabSpan});
    TextPaint paint;
    paint.setTypeface(testTypeface());
    paint.setTextSize(10.0f);

    TextLine* tl = getTextLine(u"אI\tאI", paint, &stops);
    assertMeasurements(tl, 5, false,
                       {0.0f, -20.0f, -20.0f, -100.0f, -120.0f, -120.0f});
    assertMeasurements(tl, 5, true,
                       {0.0f, -10.0f, -10.0f, -100.0f, -110.0f, -110.0f});
    releaseTextLine(tl);
}

TEST(CoreTextLineTest, testMeasure_wordSpacing) {
    TextPaint paint;
    paint.setTypeface(testTypeface());
    paint.setTextSize(10.0f);
    paint.setWordSpacing(10.0f);

    TextLine* tl = getTextLine(u"I I", paint);
    assertMeasurements(tl, 3, false, {0.0f, 10.0f, 120.0f, 130.0f});
    releaseTextLine(tl);
}

TEST(CoreTextLineTest, testHandleRun_ellipsizedReplacementSpan_isSkipped) {
    SpannableStringBuilder text(u"This is a... text");

    // Setup a replacement span that the measurement should not interact with.
    TestReplacementSpan* span = new TestReplacementSpan;
    text.setSpan(span, 9, 12, Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);

    TextLine* tl = TextLine::obtain();
    TextPaint paint;
    tl->set(&paint, &text, 0, (int)text.length(), 1, &Layout::DIRS_ALL_LEFT_TO_RIGHT,
            false /* hasTabs */, nullptr /* tabStops */, 9, 12, false);
    tl->measure((int)text.length(), false /* trailing */, nullptr);

    EXPECT_FALSE(span->mIsUsed);
    releaseTextLine(tl);
}

TEST(CoreTextLineTest, testHandleRun_notEllipsizedReplacementSpan_isNotSkipped) {
    SpannableStringBuilder text(u"This is a... text");

    TestReplacementSpan* span = new TestReplacementSpan;
    text.setSpan(span, 1, 5, Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);

    TextLine* tl = TextLine::obtain();
    TextPaint paint;
    tl->set(&paint, &text, 0, (int)text.length(), 1, &Layout::DIRS_ALL_LEFT_TO_RIGHT,
            false /* hasTabs */, nullptr /* tabStops */, 9, 12, false);
    tl->measure((int)text.length(), false /* trailing */, nullptr);

    EXPECT_TRUE(span->mIsUsed);
    releaseTextLine(tl);
}

TEST(CoreTextLineTest, testHandleRun_halfEllipsizedReplacementSpan_isNotSkipped) {
    SpannableStringBuilder text(u"This is a... text");

    TestReplacementSpan* span = new TestReplacementSpan;
    text.setSpan(span, 7, 11, Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);

    TextLine* tl = TextLine::obtain();
    TextPaint paint;
    tl->set(&paint, &text, 0, (int)text.length(), 1, &Layout::DIRS_ALL_LEFT_TO_RIGHT,
            false /* hasTabs */, nullptr /* tabStops */, 9, 12, false);
    tl->measure((int)text.length(), false /* trailing */, nullptr);

    EXPECT_TRUE(span->mIsUsed);
    releaseTextLine(tl);
}
