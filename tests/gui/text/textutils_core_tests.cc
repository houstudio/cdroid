// Ported from AOSP coretests TextUtilsTest (android.text.TextUtils).
// Original: frameworks/base/core/tests/coretests/src/android/text/TextUtilsTest.java (Apache 2.0)
//
// CDROID adaptation (ported subset — the rest are API gaps recorded here):
//  - testBasic: concat(std::vector<CharSequence*>); Java String/SpannedString
//    instanceof -> dynamic_cast on the returned CharSequence.
//  - testEllipsize / testEllipsize_multiCodepoint: portable as-is (TextPaint +
//    ellipsize overloads exist); assertions are consistency-based, except the
//    multi-codepoint exact strings which are metric-relative.
//  - testToUpperCase: cdroid toUpperCase(source, copySpans) has NO locale
//    parameter (v1 is locale-insensitive) — the Turkish/Greek cases are not
//    expressible; the locale-free cases are ported.
//  - NOT PORTED (API missing in cdroid): expandTemplate (template string/span),
//    SimpleStringSplitter (5 splitter tests), delimitedStringContains,
//    writeToParcel/CHAR_SEQUENCE_CREATOR (4 parcel tests), trimToSize,
//    length(), trimToLengthWithEllipsis, formatSimple (6 tests),
//    Rfc822Tokenizer (android.text.util not ported).
#include <gtest/gtest.h>
#include <text/textutils.h>
#include <text/spannablestring.h>
#include <text/spannablestringbuilder.h>
#include <text/String.h>
#include <text/parcelablespan.h>
#include <text/style/characterstyles.h>
#include <text/textpaint.h>
#include <content/Locale.h>
#include <view/view.h>

using namespace cdroid;

namespace {
// Identity-only span stand-in for AOSP `new Object()`: NoCopySpan-derived so
// the container treats stack instances as borrowed (never freed) and the
// identity survives the concat copy.
struct MarkSpan : public NoCopySpan {};
} // namespace

TEST(CoreTextUtilsTest, testBasic) {
    EXPECT_EQ("", TextUtils::concat({})->toUTF8());
    EXPECT_EQ("foo", TextUtils::concat({new String(u"foo")})->toUTF8());
    EXPECT_EQ("foobar", TextUtils::concat({new String(u"foo"), new String(u"bar")})->toUTF8());
    EXPECT_EQ("foobarbaz", TextUtils::concat({new String(u"foo"), new String(u"bar"),
                                              new String(u"baz")})->toUTF8());

    /*AOSP asserts the SAME span objects land in the concat result (Java
      references, GC-kept). Under the raw-pointer model concat propagates only
      clone()-able spans (a NoCopySpan borrowed across would dangle once the
      source piece dies), so offsets are asserted through an owned UnderlineSpan
      clone, and the NoCopy markers are asserted to stay behind. Same split as
      spannable_core_tests' testAppend.*/
    SpannableString foo(u"foo");
    MarkSpan fooSpan;
    foo.setSpan(&fooSpan, 1, 2, Spannable::SPAN_EXCLUSIVE_INCLUSIVE);
    foo.setSpan(new UnderlineSpan, 1, 2, Spannable::SPAN_EXCLUSIVE_INCLUSIVE);

    SpannableString bar(u"bar");
    MarkSpan barSpan;
    bar.setSpan(&barSpan, 1, 2, Spannable::SPAN_EXCLUSIVE_INCLUSIVE);
    bar.setSpan(new UnderlineSpan, 1, 2, Spannable::SPAN_EXCLUSIVE_INCLUSIVE);

    SpannableString baz(u"baz");
    MarkSpan bazSpan;
    baz.setSpan(&bazSpan, 1, 2, Spannable::SPAN_EXCLUSIVE_INCLUSIVE);
    baz.setSpan(new UnderlineSpan, 1, 2, Spannable::SPAN_EXCLUSIVE_INCLUSIVE);

    EXPECT_EQ("foo", TextUtils::concat({&foo})->toUTF8());
    EXPECT_EQ("foobar", TextUtils::concat({&foo, &bar})->toUTF8());
    EXPECT_EQ("foobarbaz", TextUtils::concat({&foo, &bar, &baz})->toUTF8());

    CharSequence* c1 = TextUtils::concat({&foo});
    Spanned* spanned1 = dynamic_cast<Spanned*>(c1);
    ASSERT_NE(nullptr, spanned1) << "concat of a single Spannable must keep the Spanned type";
    EXPECT_EQ(-1, spanned1->getSpanStart(&fooSpan));   // NoCopy never travels
    ASSERT_EQ(1u, spanned1->getSpans(0, 3, make_span_filter<UnderlineSpan>()).size());
    EXPECT_EQ(1, spanned1->getSpanStart(
            spanned1->getSpans(0, 3, make_span_filter<UnderlineSpan>())[0]));

    CharSequence* c2 = TextUtils::concat({&foo, &bar});
    Spanned* spanned2 = dynamic_cast<Spanned*>(c2);
    ASSERT_NE(nullptr, spanned2) << "concat of Spannables must keep the Spanned type";
    EXPECT_EQ(-1, spanned2->getSpanStart(&fooSpan));
    EXPECT_EQ(-1, spanned2->getSpanStart(&barSpan));
    auto us2 = spanned2->getSpans(0, 6, make_span_filter<UnderlineSpan>());
    ASSERT_EQ(2u, us2.size());
    EXPECT_EQ(1, spanned2->getSpanStart(us2[0]));
    EXPECT_EQ(4, spanned2->getSpanStart(us2[1]));

    CharSequence* c3 = TextUtils::concat({&foo, &bar, &baz});
    Spanned* spanned3 = dynamic_cast<Spanned*>(c3);
    ASSERT_NE(nullptr, spanned3) << "concat of Spannables must keep the Spanned type";
    EXPECT_EQ(-1, spanned3->getSpanStart(&bazSpan));
    auto us3 = spanned3->getSpans(0, 9, make_span_filter<UnderlineSpan>());
    ASSERT_EQ(3u, us3.size());
    EXPECT_EQ(1, spanned3->getSpanStart(us3[0]));
    EXPECT_EQ(4, spanned3->getSpanStart(us3[1]));
    EXPECT_EQ(7, spanned3->getSpanStart(us3[2]));

    // plain-text concat yields a String; spanned concat yields a SpannedString
    EXPECT_TRUE(dynamic_cast<String*>(TextUtils::concat({new String(u"foo"),
                                                         new String(u"bar")})) != nullptr);
    EXPECT_TRUE(dynamic_cast<SpannedString*>(TextUtils::concat({&foo, &bar})) != nullptr);
}

TEST(CoreTextUtilsTest, testTrim) {
    const char* strings[] = { "abc", " abc", "  abc", "abc ", "abc  ",
                              " abc ", "  abc  ", "\nabc\n", "\nabc", "abc\n" };
    for (const char* s : strings) {
        // Java String.trim() strips chars <= ' ' from both ends.
        std::string str(s);
        size_t b = str.find_first_not_of(" \t\n\r\f\v");
        size_t e = str.find_last_not_of(" \t\n\r\f\v");
        size_t trimmed = (b == std::string::npos) ? 0 : e - b + 1;
        String cs(TextUtils::utf8_utf16(str));
        EXPECT_EQ(trimmed, (size_t)TextUtils::getTrimmedLength(&cs)) << s;
    }
}

// CharSequence wrapper for the ellipsize copy path (text via getChars).
namespace {
class Wrapper : public CharSequence {
private:
    CharSequence* mString;
public:
    explicit Wrapper(CharSequence* s) : mString(s) {}
    size_t length() const override { return mString->length(); }
    int charAt(int off) const override { return mString->charAt(off); }
    String* toString() const override { return mString->toString(); }
    std::string toUTF8() const override { return mString->toUTF8(); }
    std::u16string toUTF16() const override { return mString->toUTF16(); }
    CharSequence* subSequence(int start, int end) const override {
        return new Wrapper(mString->subSequence(start, end));
    }
    void getChars(int start, int end, char16_t* dest, int destPos) const override {
        mString->getChars(start, end, dest, destPos);
    }
};
} // namespace

TEST(CoreTextUtilsTest, testEllipsize) {
    String s1(TextUtils::utf8_utf16("The quick brown fox jumps over þhe lazy dog."));
    Wrapper s2(&s1);
    SpannableString s3(s1);
    ParcelableSpan* style = new UnderlineSpan; // StyleSpan stand-in (heap: owned by s3)
    s3.setSpan(style, 5, 10, Spannable::SPAN_EXCLUSIVE_EXCLUSIVE);
    TextPaint p;

    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 3; j++) {
            TextUtils::TruncateAt kind;
            switch (j) {
            case 0: kind = TextUtils::TruncateAt::START;  break;
            case 1: kind = TextUtils::TruncateAt::END;    break;
            default: kind = TextUtils::TruncateAt::MIDDLE; break;
            }

            std::string out1 = TextUtils::ellipsize(&s1, p, i, kind)->toUTF8();
            std::string out2 = TextUtils::ellipsize(&s2, p, i, kind)->toUTF8();
            std::string out3 = TextUtils::ellipsize(&s3, p, i, kind)->toUTF8();

            std::string keep1 = TextUtils::ellipsize(&s1, p, i, kind, true, nullptr)->toUTF8();
            std::string keep2 = TextUtils::ellipsize(&s2, p, i, kind, true, nullptr)->toUTF8();
            std::string keep3 = TextUtils::ellipsize(&s3, p, i, kind, true, nullptr)->toUTF8();

            // TextUtils::replace(std::string&, ...) rewrites its argument in
            // place (a C++-local helper — android.text.TextUtils has no such
            // method), unlike Java's pure String.replace. Strip the BOM padding
            // on a COPY so keep1 stays intact for the assertions below.
            std::string trim1 = keep1;
            TextUtils::replace(trim1, "\xEF\xBB\xBF", "");

            // Are all normal output strings identical?
            EXPECT_EQ(out1, out2) << "wid " << i << " pass " << j;
            EXPECT_EQ(out2, out3) << "wid " << i << " pass " << j;

            // Are preserved output strings identical?
            EXPECT_EQ(keep1, keep2) << "wid " << i << " pass " << j;
            EXPECT_EQ(keep2, keep3) << "wid " << i << " pass " << j;

            // Does trimming padding from preserved yield normal?
            EXPECT_EQ(out1, trim1) << "wid " << i << " pass " << j;

            // Did preserved output strings preserve length? (Java's length() is
            // UTF-16 code units; keep1 is the UTF-8 rendering, so convert back
            // before comparing — each BOM filler is 3 bytes but 1 unit.)
            EXPECT_EQ(s1.length(), TextUtils::utf8_utf16(keep1).size()) << "wid " << i << " pass " << j;

            // Does the output string actually fit in the space? (+0.5f: the
            // cairo/pixman advance accumulation can overshoot by a fraction of
            // a pixel where AOSP's float measurement fits exactly — the same
            // rounding-tolerance treatment as the chain spread tests.)
            EXPECT_LE(p.measureText(out1), (float)i + 0.5f)
                    << "wid " << i << " pass " << j;
        }
    }
}

TEST(CoreTextUtilsTest, testRemoveEmptySpans) {
    // MockSpanned: spans report end=1 only when in the nonEmpty set.
    struct MockSpanned : public Spanned {
        std::vector<const ParcelableSpan*> allSpans;
        std::vector<const ParcelableSpan*> nonEmptySpans;

        MockSpanned* addSpan() {
            UnderlineSpan* o = new UnderlineSpan; // owned leak-free: test scope
            allSpans.push_back(o);
            nonEmptySpans.push_back(o);
            return this;
        }
        MockSpanned* addEmptySpan() {
            UnderlineSpan* o = new UnderlineSpan;
            allSpans.push_back(o);
            return this;
        }
        void clearSpans() {
            for (auto* s : allSpans) delete const_cast<ParcelableSpan*>(s);
            allSpans.clear();
            nonEmptySpans.clear();
        }
        void test() {
            auto spans = allSpans;
            TextUtils::removeEmptySpans(spans, this, make_span_filter<ParcelableSpan>());
            ASSERT_EQ(nonEmptySpans.size(), spans.size());
            for (size_t i = 0; i < spans.size(); i++) {
                EXPECT_EQ(nonEmptySpans[i], spans[i]);
            }
        }

        size_t length() const override { return 0; }
        int charAt(int) const override { return 0; }
        void getChars(int, int, char16_t*, int) const override {}
        String* toString() const override { return nullptr; }
        std::string toUTF8() const override { return ""; }
        std::u16string toUTF16() const override { return u""; }
        CharSequence* subSequence(int, int) const override { return nullptr; }
        std::vector<const ParcelableSpan*> getSpans(int, int,
                const SpanFilter&) const override { return {}; }
        int getSpanStart(const ParcelableSpan*) const override { return 0; }
        int getSpanEnd(const ParcelableSpan* tag) const override {
            return std::find(nonEmptySpans.begin(), nonEmptySpans.end(), tag)
                   != nonEmptySpans.end() ? 1 : 0;
        }
        int getSpanFlags(const ParcelableSpan*) const override { return 0; }
        int nextSpanTransition(int, int, const SpanFilter&) const override { return 0; }
    } spanned;

    spanned.test();
    for (int i = 0; i < 3; i++) spanned.addSpan()->test();
    spanned.addEmptySpan()->test();
    spanned.addSpan()->test();
    spanned.addEmptySpan()->test();
    spanned.addEmptySpan()->test();
    spanned.addSpan()->test();

    spanned.clearSpans();
    for (int i = 0; i < 3; i++) spanned.addEmptySpan()->test();
    spanned.addSpan()->test();
    spanned.addEmptySpan()->test();
    spanned.addSpan()->test();

    spanned.clearSpans();
    spanned.addSpan()->test();
    spanned.addEmptySpan()->test();
    spanned.addSpan()->test();
    spanned.addEmptySpan()->test();
    spanned.addSpan()->test();
    spanned.addSpan()->test();
    spanned.clearSpans(); // free the news
}

TEST(CoreTextUtilsTest, testGetLayoutDirectionFromLocale) {
    // LayoutDirection lives in view/gravity.h; LTR=0, RTL=1 (View exposes
    // them as LAYOUT_DIRECTION_* too).
    Locale nullLocale;
    EXPECT_EQ((int)LayoutDirection::LTR, TextUtils::getLayoutDirectionFromLocale(nullLocale));
    EXPECT_EQ((int)LayoutDirection::LTR, TextUtils::getLayoutDirectionFromLocale(Locale::ROOT));
    EXPECT_EQ((int)LayoutDirection::LTR, TextUtils::getLayoutDirectionFromLocale(Locale::forLanguageTag("en")));
    EXPECT_EQ((int)LayoutDirection::LTR, TextUtils::getLayoutDirectionFromLocale(Locale::forLanguageTag("en-US")));
    EXPECT_EQ((int)LayoutDirection::LTR, TextUtils::getLayoutDirectionFromLocale(Locale::forLanguageTag("az")));
    EXPECT_EQ((int)LayoutDirection::LTR, TextUtils::getLayoutDirectionFromLocale(Locale::forLanguageTag("az-AZ")));
    EXPECT_EQ((int)LayoutDirection::LTR, TextUtils::getLayoutDirectionFromLocale(Locale::forLanguageTag("az-Latn")));
    EXPECT_EQ((int)LayoutDirection::LTR, TextUtils::getLayoutDirectionFromLocale(Locale::forLanguageTag("en-EG")));
    EXPECT_EQ((int)LayoutDirection::LTR, TextUtils::getLayoutDirectionFromLocale(Locale::forLanguageTag("ar-Latn")));

    EXPECT_EQ((int)LayoutDirection::RTL, TextUtils::getLayoutDirectionFromLocale(Locale::forLanguageTag("ar")));
    EXPECT_EQ((int)LayoutDirection::RTL, TextUtils::getLayoutDirectionFromLocale(Locale::forLanguageTag("fa")));
    EXPECT_EQ((int)LayoutDirection::RTL, TextUtils::getLayoutDirectionFromLocale(Locale::forLanguageTag("he")));
    EXPECT_EQ((int)LayoutDirection::RTL, TextUtils::getLayoutDirectionFromLocale(Locale::forLanguageTag("iw")));
    EXPECT_EQ((int)LayoutDirection::RTL, TextUtils::getLayoutDirectionFromLocale(Locale::forLanguageTag("ur")));
    EXPECT_EQ((int)LayoutDirection::RTL, TextUtils::getLayoutDirectionFromLocale(Locale::forLanguageTag("dv")));
    EXPECT_EQ((int)LayoutDirection::RTL, TextUtils::getLayoutDirectionFromLocale(Locale::forLanguageTag("az-Arab")));
    EXPECT_EQ((int)LayoutDirection::RTL, TextUtils::getLayoutDirectionFromLocale(Locale::forLanguageTag("az-IR")));
    EXPECT_EQ((int)LayoutDirection::RTL, TextUtils::getLayoutDirectionFromLocale(Locale::forLanguageTag("fa-US")));
    EXPECT_EQ((int)LayoutDirection::RTL, TextUtils::getLayoutDirectionFromLocale(Locale::forLanguageTag("tr-Arab")));
}

// toUpperCase: locale-free subset (cdroid has no locale parameter).
TEST(CoreTextUtilsTest, testToUpperCase) {
    {
        String source(u"abc");
        CharSequence* result = TextUtils::toUpperCase(&source, false);
        EXPECT_EQ("ABC", result->toUTF8());
    }
    {
        SpannableString str(u"abc");
        ParcelableSpan* heapSpan = new UnderlineSpan;
        str.setSpan(heapSpan, 1, 2, Spanned::SPAN_INCLUSIVE_INCLUSIVE);

        CharSequence* result = TextUtils::toUpperCase(&str, true /* copySpans */);
        EXPECT_EQ("ABC", result->toUTF8());
        Spanned* spanned = dynamic_cast<Spanned*>(result);
        ASSERT_NE(nullptr, spanned);
        auto resultSpans = spanned->getSpans(0, result->length(), make_span_filter<ParcelableSpan>());
        ASSERT_EQ(1u, resultSpans.size());
        // AOSP asserts span IDENTITY (assertSame); cdroid clones owned spans
        // into the result by design (never shares an owned span), so only the
        // placement and flags are asserted here.
        EXPECT_EQ(1, spanned->getSpanStart(resultSpans[0]));
        EXPECT_EQ(2, spanned->getSpanEnd(resultSpans[0]));
        EXPECT_EQ(Spanned::SPAN_INCLUSIVE_INCLUSIVE, spanned->getSpanFlags(resultSpans[0]));
    }
    {
        // Already-uppercase text: AOSP can return the same instance (GC-safe
        // identity); under the raw-pointer owned-return contract callers like
        // InputFilter::AllCaps delete the input and keep the result, so the
        // no-change path must also return a FRESH object with equal content —
        // never the borrowed source (that handed AllCaps freed memory).
        String str(u"ABC");
        CharSequence* upper = TextUtils::toUpperCase(&str, false);
        EXPECT_NE(&str, upper);
        ASSERT_EQ(str.length(), upper->length());
        for (int i = 0; i < str.length(); i++) {
            EXPECT_EQ(str.charAt(i), upper->charAt(i));
        }
        delete upper;
    }
}

TEST(CoreTextUtilsTest, DISABLED_testToUpperCase_Turkish) {
    // Not expressible: cdroid toUpperCase(source, copySpans) has no locale
    // parameter (v1 is locale-insensitive).
}

TEST(CoreTextUtilsTest, DISABLED_testToUpperCase_SpansArePreserved_Greek) {
    // Not expressible: no locale parameter (Greek iota length-changing case).
}
