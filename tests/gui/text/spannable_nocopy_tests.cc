// Ported from AOSP coretests android.text NoCopySpan copy semantics:
//   SpannableStringNoCopyTest.java / SpannedStringNoCopyTest.java
// Originals: frameworks/base/core/tests/coretests/src/android/text/ (Apache 2.0)
//
// CDROID adaptation:
//  - NoCopySpan.Concrete -> local struct NoCopyConcreteSpan : NoCopySpan
//    (stack-allocated: NoCopySpans are BORROWED by the container, never freed
//    by it — a new'd one would leak).
//  - QuoteSpan is not ported to CDROID — UnderlineSpan stand-ins keep the
//    spans identity-distinct (that is all the assertions need).
//  - SpannedString has no (CharSequence, bool ignoreNoCopySpan) ctor in CDROID
//    (API gap, recorded here); the doesNotCopy variants substitute
//    SpannableString(source, true) — the copy semantics live in the shared
//    SpannableStringInternal base, so the observation stays meaningful.
//  - instanceof NoCopySpan -> dynamic_cast<const NoCopySpan*>.
#include <gtest/gtest.h>
#include <text/spannablestring.h>
#include <text/parcelablespan.h>
#include <text/style/characterstyles.h>

using namespace cdroid;

namespace {

struct NoCopyConcreteSpan : public NoCopySpan {};

// The three spans AOSP sets on the source (shared by all cases): an owned
// paragraph span, a borrowed NoCopySpan, an owned full-range span.
struct SourceSpans {
    SpannableString first{u"t\nest data"};
    NoCopyConcreteSpan noCopy; // borrowed: must outlive the copies

    SourceSpans() {
        first.setSpan(new UnderlineSpan, 0, 2, Spanned::SPAN_PARAGRAPH); // QuoteSpan stand-in
        first.setSpan(&noCopy, 2, 4, Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);
        first.setSpan(new UnderlineSpan, 0, (int)first.length(), Spanned::SPAN_PRIORITY);
    }
};

// A custom implementation of Spannable delegating everything (AOSP
// CustomSpannable): exercises the constructor path for a foreign Spannable.
class CustomSpannable : public Spannable {
private:
    Spannable& mText;
public:
    explicit CustomSpannable(Spannable& text) : mText(text) {}

    void setSpan(const ParcelableSpan* what, int start, int end, int flags) override {
        mText.setSpan(what, start, end, flags);
    }
    void removeSpan(const ParcelableSpan* what) override { mText.removeSpan(what); }
    std::vector<const ParcelableSpan*> getSpans(int start, int end,
            const SpanFilter& filter) const override { return mText.getSpans(start, end, filter); }
    int getSpanStart(const ParcelableSpan* what) const override { return mText.getSpanStart(what); }
    int getSpanEnd(const ParcelableSpan* what) const override { return mText.getSpanEnd(what); }
    int getSpanFlags(const ParcelableSpan* what) const override { return mText.getSpanFlags(what); }
    int nextSpanTransition(int start, int limit, const SpanFilter& kind) const override {
        return mText.nextSpanTransition(start, limit, kind);
    }
    size_t length() const override { return mText.length(); }
    int charAt(int idx) const override { return mText.charAt(idx); }
    String* toString() const override { return mText.toString(); }
    std::string toUTF8() const override { return mText.toUTF8(); }
    std::u16string toUTF16() const override { return mText.toUTF16(); }
    void getChars(int start, int end, char16_t* dest, int destPos) const override {
        mText.getChars(start, end, dest, destPos);
    }
    CharSequence* subSequence(int start, int end) const override {
        return mText.subSequence(start, end);
    }
};

void assertNoCopyAbsent(const Spanned& copied, size_t expectedSpans) {
    auto spans = copied.getSpans(0, copied.length(), make_span_filter<ParcelableSpan>());
    ASSERT_EQ(expectedSpans, spans.size());
    for (auto* span : spans) {
        EXPECT_FALSE(dynamic_cast<const NoCopySpan*>(span) != nullptr);
    }
}

void copyNoCopySpansInternalImpl() {
    SourceSpans s;
    SpannedString copied(&s.first);
    EXPECT_EQ(3u, copied.getSpans(0, copied.length(), make_span_filter<ParcelableSpan>()).size());
    EXPECT_EQ(1u, copied.getSpans(0, copied.length(), make_span_filter<NoCopySpan>()).size());
}

void doesNotCopyNoCopySpansInternalImpl() {
    SourceSpans s;
    SpannableString copied(&s.first, true /* ignoreNoCopySpan */);
    assertNoCopyAbsent(copied, 2);
}

void copyNoCopySpansOtherImpl(Spannable& custom) {
    SpannedString copied(&custom);
    EXPECT_EQ(3u, copied.getSpans(0, copied.length(), make_span_filter<ParcelableSpan>()).size());
    EXPECT_EQ(1u, copied.getSpans(0, copied.length(), make_span_filter<NoCopySpan>()).size());
}

void doesNotCopyNoCopySpansOtherImpl(Spannable& custom) {
    SpannableString copied(&custom, true /* ignoreNoCopySpan */);
    assertNoCopyAbsent(copied, 2);
}

} // namespace

// --- SpannableStringNoCopyTest ---

TEST(SpannableStringNoCopyTest, testCopyConstructor_copyNoCopySpans_SpannableStringInternalImpl) {
    copyNoCopySpansInternalImpl();
}

TEST(SpannableStringNoCopyTest, testCopyConstructor_doesNotCopyNoCopySpans_SpannableStringInternalImpl) {
    doesNotCopyNoCopySpansInternalImpl();
}

TEST(SpannableStringNoCopyTest, testCopyConstructor_copyNoCopySpans_OtherSpannableImpl) {
    SourceSpans s;
    CustomSpannable custom(s.first);
    copyNoCopySpansOtherImpl(custom);
}

TEST(SpannableStringNoCopyTest, testCopyConstructor_doesNotCopyNoCopySpans_OtherSpannableImpl) {
    SourceSpans s;
    CustomSpannable custom(s.first);
    doesNotCopyNoCopySpansOtherImpl(custom);
}

// --- SpannedStringNoCopyTest ---
// AOSP duplicates the four bodies with a read-only CustomSpanned wrapper
// (implements Spanned instead of Spannable). CDROID's Spannable-only wrapper
// above already covers the constructor path for a foreign implementation; the
// InternalImpl pair is byte-identical to SpannableStringNoCopyTest's, so both
// fixtures share the helpers.

TEST(SpannedStringNoCopyTest, testCopyConstructor_copyNoCopySpans_SpannableStringInternalImpl) {
    copyNoCopySpansInternalImpl();
}

TEST(SpannedStringNoCopyTest, testCopyConstructor_doesNotCopyNoCopySpans_SpannableStringInternalImpl) {
    doesNotCopyNoCopySpansInternalImpl();
}

TEST(SpannedStringNoCopyTest, testCopyConstructor_copyNoCopySpans_OtherSpannedImpl) {
    SourceSpans s;
    CustomSpannable custom(s.first);
    copyNoCopySpansOtherImpl(custom);
}

TEST(SpannedStringNoCopyTest, testCopyConstructor_doesNotCopyNoCopySpans_OtherSpannedImpl) {
    SourceSpans s;
    CustomSpannable custom(s.first);
    doesNotCopyNoCopySpansOtherImpl(custom);
}
