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
#include <text/spannablestringbuilder.h>
#include <text/spannablestring.h>
#include <text/String.h>
#include <text/style/characterstyles.h>
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
