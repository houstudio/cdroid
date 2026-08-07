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
