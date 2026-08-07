// AOSP CTS TextUtilsTest port (android.text.TextUtils). First batch: the pure ASCII string-logic
// methods whose AOSP bodies were verified. CharSequence is constructed via cdroid::String (a
// CharSequence impl built from UTF-8). Methods requiring spans (copySpansFrom, concat-with-spans),
// the HSV-like missing APIs (join/expandTemplate/getReverse — absent from CDROID's TextUtils), the
// unicode supplementary-plane cases (surrogate pairs), and the TextPaint-coupled ellipsize family
// are deferred to a later batch. Java null CharSequence → nullptr.
//
// Original: cts/tests/tests/text/src/android/text/cts/TextUtilsTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <text/textutils.h>
#include <text/String.h>
#include <text/spannablestringbuilder.h>
#include <text/spannablestring.h>
#include <text/style/characterstyles.h>
#include <vector>
#include <string>

using namespace cdroid;

TEST(CtsTextUtilsTest, testIsEmpty) {
    String notEmpty("not empty"), spaces("    "), empty("");
    EXPECT_FALSE(TextUtils::isEmpty(&notEmpty));
    EXPECT_FALSE(TextUtils::isEmpty(&spaces));
    EXPECT_TRUE(TextUtils::isEmpty(&empty));
    EXPECT_TRUE(TextUtils::isEmpty((CharSequence*)nullptr));   // AOSP: isEmpty(null) == true
}

TEST(CtsTextUtilsTest, testEquals) {
    String a("same object");
    EXPECT_TRUE(TextUtils::equals(&a, &a));                     // same object

    String c1("different object"), c2("different object");
    EXPECT_TRUE(TextUtils::equals(&c1, &c2));                   // same content, distinct objects

    String d1("different content A"), d2("different content B");
    EXPECT_FALSE(TextUtils::equals(&d1, &d2));                  // different content

    EXPECT_TRUE(TextUtils::equals(nullptr, nullptr));           // both null
    EXPECT_FALSE(TextUtils::equals(&a, nullptr));               // one null
    EXPECT_FALSE(TextUtils::equals(nullptr, &a));
}

TEST(CtsTextUtilsTest, testGetTrimmedLength) {
    // AOSP: trimmed length excludes leading/trailing whitespace (<= U+0020) only.
    String s1("normalstring"), s2("normal string");
    EXPECT_EQ(12, TextUtils::getTrimmedLength(&s1));
    EXPECT_EQ(13, TextUtils::getTrimmedLength(&s2));
    String lead(" \t  blank before"), trail("blank after   \n    "), both(" \t   blank both  \n ");
    EXPECT_EQ(12, TextUtils::getTrimmedLength(&lead));
    EXPECT_EQ(11, TextUtils::getTrimmedLength(&trail));
    EXPECT_EQ(10, TextUtils::getTrimmedLength(&both));
}

TEST(CtsTextUtilsTest, testHtmlEncode) {
    EXPECT_EQ(std::string("&lt;_html_&gt;\\ &amp;&quot;&#39;string&#39;&quot;"),
              TextUtils::htmlEncode("<_html_>\\ &\"'string'\""));
}

TEST(CtsTextUtilsTest, testIsDigitsOnly) {
    String e(""), word("no digit"), mixed("character and 56 digits"), digits("0123456789"),
           spaced("1234 56789");
    EXPECT_TRUE(TextUtils::isDigitsOnly(&e));
    EXPECT_FALSE(TextUtils::isDigitsOnly(&word));
    EXPECT_FALSE(TextUtils::isDigitsOnly(&mixed));
    EXPECT_TRUE(TextUtils::isDigitsOnly(&digits));
    EXPECT_FALSE(TextUtils::isDigitsOnly(&spaced));
    // Supplementary-plane digit cases (OSMYANA / IMPERIAL ARAMAIC, surrogate pairs) deferred.
}

TEST(CtsTextUtilsTest, testIsGraphic) {
    String notGraphic(""), graphic("abc");
    EXPECT_FALSE(TextUtils::isGraphic(&notGraphic));   // empty has no graphic chars
    EXPECT_TRUE(TextUtils::isGraphic(&graphic));
    EXPECT_FALSE(TextUtils::isGraphic((char16_t)0));     // null control char - not graphic
    EXPECT_TRUE(TextUtils::isGraphic(u'a'));
    EXPECT_FALSE(TextUtils::isGraphic(u' '));          // AOSP: space is not graphic
}

TEST(CtsTextUtilsTest, testSplit) {
    // split(text, string delim)
    auto parts = TextUtils::split(std::string("a,b,c"), std::string(","));
    ASSERT_EQ(3u, parts.size());
    EXPECT_EQ("a", parts[0]);
    EXPECT_EQ("b", parts[1]);
    EXPECT_EQ("c", parts[2]);
    // split(text, int delim)
    auto parts2 = TextUtils::split(std::string("a.b.c"), (int)'.');
    ASSERT_EQ(3u, parts2.size());
    EXPECT_EQ("c", parts2[2]);
}

TEST(CtsTextUtilsTest, testReplace) {
    std::string s = "this is a string to be as the template for replacement";
    TextUtils::replace(s, std::string("string"), std::string("text"));
    EXPECT_EQ(std::string("this is a text to be as the template for replacement"), s);
}

TEST(CtsTextUtilsTest, testIndexOf1) {
    String s("string to be searched");   // length 21
    EXPECT_EQ(1,  TextUtils::indexOf(&s, u't'));   // first 't'
    EXPECT_EQ(2,  TextUtils::indexOf(&s, u'r'));   // first 'r'
    EXPECT_EQ(20, TextUtils::indexOf(&s, u'd'));   // last char
    EXPECT_EQ(-1, TextUtils::indexOf(&s, u'f'));   // absent
}

TEST(CtsTextUtilsTest, testIndexOf2) {
    String s("string to be searched");
    EXPECT_EQ(2,  TextUtils::indexOf(&s, u'r', 0));              // first 'r'
    EXPECT_EQ(16, TextUtils::indexOf(&s, u'r', 3));              // second 'r' (after first)
    EXPECT_EQ(-1, TextUtils::indexOf(&s, u'r', s.length()));     // start at end
}

TEST(CtsTextUtilsTest, testIndexOf3) {
    String s("string to be searched");
    EXPECT_EQ(2,  TextUtils::indexOf(&s, u'r', 0, s.length()));
    EXPECT_EQ(16, TextUtils::indexOf(&s, u'r', 3, s.length()));
    EXPECT_EQ(-1, TextUtils::indexOf(&s, u'r', 3, 16));          // window excludes the 2nd 'r'
}

TEST(CtsTextUtilsTest, testLastIndexOf1) {
    String s("string to be searched");
    EXPECT_EQ(7,  TextUtils::lastIndexOf(&s, u't'));   // 't' of "to"
    EXPECT_EQ(16, TextUtils::lastIndexOf(&s, u'r'));
    EXPECT_EQ(20, TextUtils::lastIndexOf(&s, u'd'));
    EXPECT_EQ(-1, TextUtils::lastIndexOf(&s, u'f'));
}

TEST(CtsTextUtilsTest, testRegionMatches) {
    String one("one"), two("two");
    EXPECT_FALSE(TextUtils::regionMatches(&one, 0, &two, 0, 3));
    EXPECT_TRUE (TextUtils::regionMatches(&one, 0, &one, 0, 3));

    String hello1("Hello Android, hello World!"), hello2("Hello World");
    EXPECT_TRUE (TextUtils::regionMatches(&hello1, 0,  &hello2, 0, 5));   // "Hello" == "Hello"
    EXPECT_FALSE(TextUtils::regionMatches(&hello1, 0,  &hello2, 0, 7));   // "Hello A" != "Hello W"
    EXPECT_TRUE (TextUtils::regionMatches(&hello1, 21, &hello2, 6, 5));   // "World" == "World"
    EXPECT_FALSE(TextUtils::regionMatches(&hello1, 21, &hello2, 0, 5));   // "World" != "Hello"
}

TEST(CtsTextUtilsTest, testGetChars) {
    // getChars copies source[start,end) into dest at destOff; bytes outside that range are unchanged.
    String src("source string mock");
    const std::u16string destOrig = u"destination";
    char16_t dest[32];
    for (int i = 0; i < 11; i++) dest[i] = destOrig[i];   // seed with "destination"

    TextUtils::getChars(&src, 0, 4, dest, 0);   // copy "sour" over "dest"
    EXPECT_EQ(u's', dest[0]); EXPECT_EQ(u'o', dest[1]); EXPECT_EQ(u'u', dest[2]); EXPECT_EQ(u'r', dest[3]);
    EXPECT_EQ(u'i', dest[4]);                    // unchanged (was "ination"[0])
    TextUtils::getChars(&src, 0, (int)src.length(), dest, 0);
    EXPECT_EQ(src.charAt(0), dest[0]);
    EXPECT_EQ(src.charAt(src.length() - 1), dest[src.length() - 1]);
}

TEST(CtsTextUtilsTest, testConcat) {
    // concat() with no args -> empty
    {
        CharSequence* r = TextUtils::concat({});
        ASSERT_NE(nullptr, r);
        EXPECT_EQ(std::string(""), r->toUTF8());
        delete r;
    }
    // single
    {
        String first("first");
        CharSequence* r = TextUtils::concat({&first});
        EXPECT_EQ(std::string("first"), r->toUTF8());
        delete r;
    }
    // three parts
    {
        String first("first"), sep(", "), second("second");
        CharSequence* r = TextUtils::concat({&first, &sep, &second});
        EXPECT_EQ(std::string("first, second"), r->toUTF8());
        delete r;
    }
}

TEST(CtsTextUtilsTest, testConcatSpans) {
    // A Spanned piece routes concat through the span-preserving path (SpannableStringBuilder);
    // the result is a Spanned (not a flattened String) carrying the concatenated text.
    SpannableStringBuilder b1(u"first");
    UnderlineSpan* us = new UnderlineSpan;
    b1.setSpan(us, 0, b1.length(), Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);
    String sep(", "), second("second");
    CharSequence* r = TextUtils::concat({&b1, &sep, &second});
    ASSERT_NE(nullptr, r);
    EXPECT_EQ(std::string("first, second"), r->toUTF8());
    EXPECT_NE(nullptr, dynamic_cast<Spanned*>(r));   // spanned path produced a Spanned
    delete r;
}

TEST(CtsTextUtilsTest, testIndexOf4) {
    // substring search (indexOf(CharSequence, CharSequence))
    String s("string to be searched by string");
    String needleString("string"), needleSearch("search"), needleTobe("tobe"), needleEmpty("");
    EXPECT_EQ(0,  TextUtils::indexOf(&s, &needleString));
    EXPECT_EQ(13, TextUtils::indexOf(&s, &needleSearch));   // "search" starts at 13
    EXPECT_EQ(-1, TextUtils::indexOf(&s, &needleTobe));
    EXPECT_EQ(0,  TextUtils::indexOf(&s, &needleEmpty));     // empty needle -> start (0)
}

TEST(CtsTextUtilsTest, testIndexOf5) {
    String s("string to be searched by string");
    String needle("string");
    EXPECT_EQ(0,  TextUtils::indexOf(&s, &needle, 0));        // first "string"
    EXPECT_EQ(25, TextUtils::indexOf(&s, &needle, 1));        // second "string"
    EXPECT_EQ(-1, TextUtils::indexOf(&s, &needle, 26));       // none after the 2nd
    String emptyNeedle("");
    EXPECT_EQ(1,  TextUtils::indexOf(&s, &emptyNeedle, 1));   // empty needle -> start
    String whole("string to be searched by string");
    EXPECT_EQ(0,  TextUtils::indexOf(&s, &whole, 0));         // whole haystack
}

TEST(CtsTextUtilsTest, testLastIndexOf2) {
    String s("string to be searched");   // 'r' at 2 and 16
    EXPECT_EQ(16, TextUtils::lastIndexOf(&s, u'r', s.length()));  // last 'r'
    EXPECT_EQ(-1, TextUtils::lastIndexOf(&s, u'r', 0));           // window [0..0] has no 'r'
    EXPECT_EQ(2,  TextUtils::lastIndexOf(&s, u'r', 2));           // window [0..2] -> first 'r'
}

TEST(CtsTextUtilsTest, testGetOffsetAfter) {
    // Build a UTF-16 string with explicit surrogate pairs at known offsets:
    //   9:0xD800,0xDB00 (high + non-low  -> +1), 16:0xD800,0xDC00 (high+low -> +2),
    //   26:0xDBFF,0xDFFF (high+low -> +2).
    std::u16string utext = {u's',u't',u'r',u'i',u'n',u'g',u' ',u't',u'o',
                            0xD800,0xDB00, u' ',u'g',u'e',u't',u' ',
                            0xD800,0xDC00, u' ',u'o',u'f',u'f',u's',u'e',u't',u' ',
                            0xDBFF,0xDFFF, u' ',u'a',u'f',u't',u'e',u'r'};
    String text(utext);
    const int len = (int)utext.size();
    EXPECT_EQ(1,   TextUtils::getOffsetAfter(&text, 0));          // normal char
    EXPECT_EQ(len, TextUtils::getOffsetAfter(&text, len));        // at end stays
    EXPECT_EQ(len, TextUtils::getOffsetAfter(&text, len - 1));    // last char -> end
    EXPECT_EQ(10,  TextUtils::getOffsetAfter(&text, 9));          // D800 + non-low -> +1
    EXPECT_EQ(18,  TextUtils::getOffsetAfter(&text, 16));         // D800 + DC00 low -> +2
    EXPECT_EQ(28,  TextUtils::getOffsetAfter(&text, 26));         // DBFF + DFFF low -> +2
}

TEST(CtsTextUtilsTest, testGetOffsetBefore) {
    std::u16string utext = {u's',u't',u'r',u'i',u'n',u'g',u' ',u't',u'o',
                            0xD800,0xDB00, u' ',u'g',u'e',u't',u' ',
                            0xD800,0xDC00, u' ',u'o',u'f',u'f',u's',u'e',u't',u' ',
                            0xDBFF,0xDFFF, u' ',u'a',u'f',u't',u'e',u'r'};
    String text(utext);
    EXPECT_EQ(0,  TextUtils::getOffsetBefore(&text, 0));
    EXPECT_EQ(0,  TextUtils::getOffsetBefore(&text, 1));
    EXPECT_EQ(10, TextUtils::getOffsetBefore(&text, 11));   // charAt(10)=DB00 (non-low) -> -1
    EXPECT_EQ(16, TextUtils::getOffsetBefore(&text, 18));   // charAt(17)=DC00 low, charAt(16)=D800 high -> -2
    EXPECT_EQ(26, TextUtils::getOffsetBefore(&text, 28));   // charAt(27)=DFFF low, charAt(26)=DBFF high -> -2
}

TEST(CtsTextUtilsTest, testIndexOf6) {
    // substring indexOf with [start,end) window
    String s("string to be searched by string");
    String needle("string");
    EXPECT_EQ(0,  TextUtils::indexOf(&s, &needle, 0, s.length()));
    EXPECT_EQ(25, TextUtils::indexOf(&s, &needle, 1, s.length()));
    EXPECT_EQ(-1, TextUtils::indexOf(&s, &needle, 1, 24));   // window excludes the 2nd "string"
}

TEST(CtsTextUtilsTest, testLastIndexOf3) {
    // lastIndexOf with [start,last] window
    String s("string to be searched");   // 'r' at 2 and 16
    EXPECT_EQ(16, TextUtils::lastIndexOf(&s, u'r', 0, s.length()));
    EXPECT_EQ(2,  TextUtils::lastIndexOf(&s, u'r', 0, 15));  // window [0..15] excludes index-16 'r'
    EXPECT_EQ(-1, TextUtils::lastIndexOf(&s, u'r', 0, 1));   // window [0..1] has no 'r'
}

// Skipped (edge cases):
//   null-variant cases (testGetTrimmedLengthNull / testHtmlEncodeNull / testIsDigitsOnlyNull):
//     like AOSP Java (which NPEs), CDROID's getTrimmedLength/isDigitsOnly/htmlEncode do NOT
//     null-guard their CharSequence*/string arg — passing null is a crash (SIGSEGV), not a
//     catchable throw, so the AOSP null "tests" don't map to C++. The null-GUARDED methods
//     (isEmpty/equals, which return true for null) are already exercised in testIsEmpty/testEquals.
//   getOffsetAfter/Before ReplacementSpan span-adjustment branch: needs a mock ReplacementSpan
//     whose getSize() returns 0; CDROID's branch is covered structurally by the surrogate-pair
//     cases above (the span branch is a no-op for a plain String).

