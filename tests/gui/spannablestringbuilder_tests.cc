// Ported from AOSP CTS SpannableStringBuilderTest.java (android.text.SpannableStringBuilder).
//
// CDROID adaptation:
//  - Java String -> cdroid::String(u"...") (u16string-backed CharSequence) for ctor/append/
//    insert/replace; toString() -> toUTF8() (std::string) for assertions.
//  - CTS spans (UnderlineSpan/StrikethroughSpan) are default-constructed; CDROID setSpan owns
//    non-NoCopySpan instances (container deletes them), so each test news its own span.
//  - @Test(expected=NPE/IndexOutOfBounds) cases: CDROID has no Java exceptions -> skipped.
//  - testConstructorStartEnd: CDROID SpannableStringBuilder has no (text, start, end) ctor -> skipped.
//  - getSpans order/priority + equals: deferred (need SpanFilter/typeId plumbing).
//
// Original: cts/tests/tests/text/src/android/text/cts/SpannableStringBuilderTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <core/canvas.h>
#include <text/spannablestringbuilder.h>
#include <text/spannablestring.h>
#include <text/String.h>
#include <text/style/characterstyles.h>
#include <text/style/metricaffectingspan.h>
#include <text/style/alignmentspan.h>
#include <text/style/leadingmarginspan.h>
#include <text/parcelablespan.h>

using namespace cdroid;

TEST(CtsSpannableStringBuilderTest, testConstructor) {
    SpannableStringBuilder a;
    SpannableStringBuilder b(u"test");
    EXPECT_EQ("test", b.toUTF8());
}

TEST(CtsSpannableStringBuilderTest, testConstructorStartEnd) {
    // CDROID SpannableStringBuilder has no (text, start, end) ctor; skipped.
    SUCCEED();
}

TEST(CtsSpannableStringBuilderTest, testAppend1) {
    SpannableStringBuilder b(u"hello");
    b.append(String(u",world"));
    EXPECT_EQ("hello,world", b.toUTF8());
}

TEST(CtsSpannableStringBuilderTest, testAppend3) {
    SpannableStringBuilder b(u"hello");
    b.append(u'a');
    b.append(u'b');
    b.append(u'c');
    EXPECT_EQ("helloabc", b.toUTF8());
}

TEST(CtsSpannableStringBuilderTest, testCharAt) {
    SpannableStringBuilder b(u"hello");
    EXPECT_EQ(u'h', b.charAt(0));
    EXPECT_EQ(u'e', b.charAt(1));
}

TEST(CtsSpannableStringBuilderTest, testToString) {
    SpannableStringBuilder b(u"hello");
    EXPECT_EQ("hello", b.toUTF8());
    SpannableStringBuilder e;
    EXPECT_EQ("", e.toUTF8());
}

TEST(CtsSpannableStringBuilderTest, testInsert1) {
    SpannableStringBuilder b(u"hello");
    b.insert(1, String(u"abcd"), 1, 3);
    EXPECT_EQ("hbcello", b.toUTF8());
}

TEST(CtsSpannableStringBuilderTest, testReplace1) {
    SpannableStringBuilder b(u"hello, world!");
    b.replace(0, 5, String(u"hi"));
    EXPECT_EQ("hi, world!", b.toUTF8());

    SpannableStringBuilder b2(u"hello, world!");
    b2.replace(7, 12, String(u"google"));
    EXPECT_EQ("hello, google!", b2.toUTF8());
}

TEST(CtsSpannableStringBuilderTest, testSubSequence) {
    SpannableStringBuilder b(u"hello, world");
    CharSequence* text = b.subSequence(0, 2);
    EXPECT_NE(nullptr, text);
    EXPECT_EQ("he", text->toUTF8());
    delete text;
}

TEST(CtsSpannableStringBuilderTest, testGetChars) {
    SpannableStringBuilder b(u"hello");
    char16_t buf[4] = {u'x', 0, 0, 0};
    b.getChars(0, 3, buf, 1);
    EXPECT_EQ(u'x', buf[0]);
    EXPECT_EQ(u'h', buf[1]);
    EXPECT_EQ(u'e', buf[2]);
    EXPECT_EQ(u'l', buf[3]);
}

TEST(CtsSpannableStringBuilderTest, testSetSpan) {
    SpannableStringBuilder b(u"hello, world");
    UnderlineSpan* us = new UnderlineSpan;
    b.setSpan(us, 0, 2, Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);
    EXPECT_EQ(0, b.getSpanStart(us));
    EXPECT_EQ(2, b.getSpanEnd(us));
    EXPECT_EQ(Spanned::SPAN_EXCLUSIVE_EXCLUSIVE, b.getSpanFlags(us));
}

TEST(CtsSpannableStringBuilderTest, testGetSpanStart) {
    SpannableStringBuilder b(u"hello");
    UnderlineSpan* us = new UnderlineSpan;
    b.setSpan(us, 1, 3, 0);
    EXPECT_EQ(1, b.getSpanStart(us));
    StrikethroughSpan* ss = new StrikethroughSpan;
    EXPECT_EQ(-1, b.getSpanStart(ss));
    delete ss;  // never added -> caller owns
}

TEST(CtsSpannableStringBuilderTest, testGetSpanEnd) {
    SpannableStringBuilder b(u"hello");
    UnderlineSpan* us = new UnderlineSpan;
    b.setSpan(us, 1, 3, 0);
    EXPECT_EQ(3, b.getSpanEnd(us));
}

TEST(CtsSpannableStringBuilderTest, testGetSpanFlags) {
    SpannableStringBuilder b(u"spannable string");
    UnderlineSpan* us = new UnderlineSpan;
    EXPECT_EQ(0, b.getSpanFlags(us));  // not set yet
    b.setSpan(us, 2, 4, Spanned::SPAN_INCLUSIVE_INCLUSIVE);
    b.setSpan(us, 0, 1, Spanned::SPAN_EXCLUSIVE_INCLUSIVE);  // re-set updates flags
    EXPECT_EQ(Spanned::SPAN_EXCLUSIVE_INCLUSIVE, b.getSpanFlags(us));
}

TEST(CtsSpannableStringBuilderTest, testRemoveSpan) {
    SpannableStringBuilder b(u"hello, world");
    UnderlineSpan* us = new UnderlineSpan;
    b.setSpan(us, 0, 2, 2);
    EXPECT_EQ(0, b.getSpanStart(us));
    b.removeSpan(us);  // container deletes us
    // After removal the span is gone; getSpanStart returns -1 for a fresh instance.
    UnderlineSpan* us2 = new UnderlineSpan;
    EXPECT_EQ(-1, b.getSpanStart(us2));
    delete us2;
}

TEST(CtsSpannableStringBuilderTest, testClearSpans) {
    SpannableStringBuilder b(u"hello, world");
    UnderlineSpan* us = new UnderlineSpan;
    b.setSpan(us, 0, 2, 2);
    EXPECT_EQ(0, b.getSpanStart(us));
    b.clearSpans();  // deletes all owned spans
    UnderlineSpan* us2 = new UnderlineSpan;
    EXPECT_EQ(-1, b.getSpanStart(us2));
    delete us2;
}

TEST(CtsSpannableStringBuilderTest, testClear) {
    SpannableStringBuilder b(u"hello, world");
    b.clear();
    EXPECT_EQ(0, b.length());
}

TEST(CtsSpannableStringBuilderTest, testInsert2) {
    SpannableStringBuilder b(u"hello");
    b.insert(1, String(u"abcd"));
    EXPECT_EQ("habcdello", b.toUTF8());

    SpannableStringBuilder b2(u"hello");
    b2.insert(5, String(u"abcd"));
    EXPECT_EQ("helloabcd", b2.toUTF8());
}

TEST(CtsSpannableStringBuilderTest, testDelete) {
    SpannableStringBuilder b(u"hello, world");
    b.deleteText(0, 5);
    EXPECT_EQ(", world", b.toUTF8());

    SpannableStringBuilder b2(u"hello, world");
    b2.deleteText(7, 12);
    EXPECT_EQ("hello, ", b2.toUTF8());
}

TEST(CtsSpannableStringBuilderTest, testAppend2) {
    SpannableStringBuilder b(u"hello");
    b.append(String(u",world"), 1, 3);
    EXPECT_EQ("hellowo", b.toUTF8());

    SpannableStringBuilder b2(u"hello");
    b2.append(String(u",world"), 0, 4);
    EXPECT_EQ("hello,wor", b2.toUTF8());
}

TEST(CtsSpannableStringBuilderTest, testReplace2) {
    SpannableStringBuilder b(u"hello, world");
    b.replace(0, 5, String(u"ahiabc"), 3, 6);
    EXPECT_EQ("abc, world", b.toUTF8());

    SpannableStringBuilder b2(u"hello, world");
    b2.replace(3, 5, String(u"ahiabc"), 3, 6);
    EXPECT_EQ("helabc, world", b2.toUTF8());

    // Replacing by an empty range deletes
    SpannableStringBuilder b3(u"hello, world");
    b3.replace(4, 6, String(u""), 0, 0);
    EXPECT_EQ("hell world", b3.toUTF8());

    // Insert at (7,7)
    SpannableStringBuilder b4(u"hello, world");
    b4.replace(7, 7, String(u"nice "), 0, 5);
    EXPECT_EQ("hello, nice world", b4.toUTF8());
}

TEST(CtsSpannableStringBuilderTest, testAppend_textWithSpan) {
    QuoteSpan* span = new QuoteSpan;
    SpannableStringBuilder b(u"hello ");
    int spanStart = b.length();
    b.append(String(u"planet"), span, Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);
    int spanEnd = b.length();
    b.append(String(u" earth"));

    EXPECT_EQ("hello planet earth", b.toUTF8());

    auto spans = b.getSpans(0, b.length(), make_span_filter<ParcelableSpan>());
    EXPECT_EQ(1u, spans.size());
    EXPECT_EQ(span, spans[0]);
    EXPECT_EQ(spanStart, b.getSpanStart(spans[0]));
    EXPECT_EQ(spanEnd, b.getSpanEnd(spans[0]));
}

TEST(CtsSpannableStringBuilderTest, testGetSpans) {
    SpannableStringBuilder b(u"hello, world");
    UnderlineSpan* span1 = new UnderlineSpan;
    UnderlineSpan* span2 = new UnderlineSpan;
    b.setSpan(span1, 1, 2, Spanned::SPAN_POINT_POINT);
    b.setSpan(span2, 4, 8, Spanned::SPAN_MARK_POINT);

    // Filter for UnderlineSpan -> both
    auto underlineSpans = b.getSpans(0, 10, make_span_filter<UnderlineSpan>());
    EXPECT_EQ(2u, underlineSpans.size());
    EXPECT_EQ(span1, underlineSpans[0]);
    EXPECT_EQ(span2, underlineSpans[1]);

    // Filter for StrikethroughSpan -> none
    auto strikeSpans = b.getSpans(0, 10, make_span_filter<StrikethroughSpan>());
    EXPECT_EQ(0u, strikeSpans.size());
}

TEST(CtsSpannableStringBuilderTest, testGetSpans_returnsEmptyIfSetSpanIsNotCalled) {
    SpannableStringBuilder b(u"hello, world");
    auto spans = b.getSpans(0, 10, make_span_filter<UnderlineSpan>());
    EXPECT_EQ(0u, spans.size());
}

// --- getSpans ordering + nextSpanTransition (CDROID supports priority-sort + type filter) ---

TEST(CtsSpannableStringBuilderTest, testGetSpans_returnsInInsertionOrder) {
    // AOSP testGetSpans_returnsInInsertionOrder_regular: spans of equal priority come back in
    // insertion order.
    SpannableStringBuilder b;
    std::vector<SubscriptSpan*> expected;
    for (int i = 0; i < 5; i++) {
        int cur = b.length();
        b.append(String(u"12\n"));
        SubscriptSpan* s = new SubscriptSpan;
        b.setSpan(s, cur + 1, cur + 2, Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);
        expected.push_back(s);
    }
    auto spans = b.getSpans(0, (int)b.length(), make_span_filter<SubscriptSpan>());
    ASSERT_EQ(expected.size(), spans.size());
    for (size_t i = 0; i < expected.size(); i++) {
        EXPECT_EQ(expected[i], spans[i]) << "span " << i << " not in insertion order";
    }
}

TEST(CtsSpannableStringBuilderTest, testGetSpans_returnsSpansSortedFirstByPriorityThenByInsertionOrder) {
    // AOSP testGetSpans_returnsSpansSortedFirstByPriorityThenByInsertionOrder: priority spans
    // come first, then non-priority in insertion order.
    SpannableStringBuilder b(u"p_in_s");
    SubscriptSpan* first  = new SubscriptSpan;
    SubscriptSpan* second = new SubscriptSpan;
    SubscriptSpan* third  = new SubscriptSpan;
    SubscriptSpan* fourth = new SubscriptSpan;
    int flags        = Spanned::SPAN_EXCLUSIVE_EXCLUSIVE;
    int flagsPriority= Spanned::SPAN_EXCLUSIVE_EXCLUSIVE | Spanned::SPAN_PRIORITY;
    b.setSpan(first,  2, 4, flags);
    b.setSpan(second, 2, 4, flagsPriority);
    b.setSpan(third,  0, (int)b.length(), flags);
    b.setSpan(fourth, 0, (int)b.length(), flagsPriority);
    auto spans = b.getSpans(0, (int)b.length(), make_span_filter<ParcelableSpan>());
    ASSERT_EQ(4u, spans.size());
    EXPECT_EQ(second, spans[0]);
    EXPECT_EQ(fourth, spans[1]);
    EXPECT_EQ(first,  spans[2]);
    EXPECT_EQ(third,  spans[3]);
}

TEST(CtsSpannableStringBuilderTest, testNextSpanTransition) {
    // AOSP testNextSpanTransition. Re-setting the SAME span object moves it (underline ends at
    // 3-4, strikethrough at 8-9). TabStopSpan is absent in CDROID; BackgroundColorSpan stands in
    // as a query type that is not present in the builder. "null" (all spans) ->
    // make_span_filter<ParcelableSpan>().
    SpannableStringBuilder b(u"spannable string");
    UnderlineSpan* ul = new UnderlineSpan;
    StrikethroughSpan* st = new StrikethroughSpan;
    b.setSpan(ul, 1, 2, Spanned::SPAN_INCLUSIVE_INCLUSIVE);
    b.setSpan(ul, 3, 4, Spanned::SPAN_INCLUSIVE_INCLUSIVE);
    b.setSpan(st, 5, 6, Spanned::SPAN_INCLUSIVE_INCLUSIVE);
    b.setSpan(st, 8, 9, Spanned::SPAN_INCLUSIVE_INCLUSIVE);
    EXPECT_EQ(8,   b.nextSpanTransition(0, 10, make_span_filter<StrikethroughSpan>()));
    EXPECT_EQ(10,  b.nextSpanTransition(0, 10, make_span_filter<BackgroundColorSpan>()));  // absent
    EXPECT_EQ(3,   b.nextSpanTransition(0, 5,  make_span_filter<ParcelableSpan>()));       // all
    EXPECT_EQ(100, b.nextSpanTransition(-5, 100, make_span_filter<BackgroundColorSpan>()));
    EXPECT_EQ(1,   b.nextSpanTransition(3, 1,  make_span_filter<UnderlineSpan>()));
}

// --- SpannableStringBuilderSpanTest: paragraph-span retention on replace (AlignmentSpan is a
//     ParagraphStyle; SPAN_PARAGRAPH spans are kept only when their end lands on a paragraph
//     boundary — a '\n' or end-of-text). Source is built manually (AlignmentSpan + SPAN_PARAGRAPH)
//     instead of AOSP's Html.fromHtml("<blockquote>"), which CDROID's fromHtml does not map. ---

static SpannableStringBuilder paragraphSource(const std::u16string& text) {
    SpannableStringBuilder s(text);
    s.setSpan(new AlignmentSpan::Standard(Layout::ALIGN_NORMAL), 0, (int)s.length(),
              Spanned::SPAN_PARAGRAPH);
    return s;
}

TEST(CtsSpannableStringBuilderTest, testReplace_retainsParagraphSpanIfNewLineBefore) {
    SpannableStringBuilder dest(u"1\nselection_to_replace");
    dest.replace(2, (int)dest.length(), paragraphSource(u"new text"));
    auto spans = dest.getSpans(0, (int)dest.length(), make_span_filter<AlignmentSpan>());
    EXPECT_EQ(1u, spans.size());
}

TEST(CtsSpannableStringBuilderTest, testReplace_retainsParagraphSpanIfStartIsZero) {
    SpannableStringBuilder dest(u"selection_to_replace");
    dest.replace(0, (int)dest.length(), paragraphSource(u"new text"));
    auto spans = dest.getSpans(0, (int)dest.length(), make_span_filter<AlignmentSpan>());
    EXPECT_EQ(1u, spans.size());
}

TEST(CtsSpannableStringBuilderTest, testReplace_discardsParagraphSpanIfNoNewLineAfter) {
    SpannableStringBuilder source(u"a");
    source.setSpan(new AlignmentSpan::Standard(Layout::ALIGN_NORMAL), 0, 1, Spanned::SPAN_PARAGRAPH);
    SpannableStringBuilder dest(u"r remaining\n");
    dest.replace(0, 1, source);
    auto spans = dest.getSpans(0, (int)dest.length(), make_span_filter<AlignmentSpan>());
    EXPECT_EQ(0u, spans.size());
}
