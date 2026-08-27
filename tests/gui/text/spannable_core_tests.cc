// Ported from AOSP coretests android.text span contracts:
//   SpannableTest.java (abstract; run per implementation)
//   SpannableStringBuilderTest.java (extends SpannableTest)
//   SpannableStringTest.java (extends SpannableTest)
//   SpannedTest.java
// Originals: frameworks/base/core/tests/coretests/src/android/text/ (Apache 2.0)
//
// CDROID adaptation:
//  - Java String -> std::u16string; Java `new Object()` span stand-ins are
//    news of concrete spans (UnderlineSpan/SubscriptSpan): setSpan OWNS
//    non-NoCopySpan instances (container deletes), never freed by the test.
//  - getSpans(query, type, sort) has no sort parameter here — getSpans(filter)
//    always returns priority order, which is what the AOSP test asserts, so
//    the sort=false arg is dropped (observable order is still asserted).
//  - SpannableTest.testRemoveSpanWithIntermediateFlag: SKIPPED — CDROID has no
//    removeSpan(Object, int flags) overload (API gap, recorded here).
//  - SpannedTest.testWrapParcel: SKIPPED — no TextUtils.writeToParcel /
//    CHAR_SEQUENCE_CREATOR Parcel bridge.
//  - Watcher must multiply inherit SpanWatcher + TextWatcher: both already
//    virtual-inherit NoCopySpan, so the shared subobject is single (the
//    container's owned/borrowed classification stays correct).
#include <gtest/gtest.h>
#include <text/spannablestringbuilder.h>
#include <text/spannablestring.h>
#include <text/String.h>
#include <text/spanwatcher.h>
#include <text/textwatcher.h>
#include <text/parcelablespan.h>
#include <text/style/characterstyles.h>

using namespace cdroid;

namespace {

// Identity-only span stand-in for AOSP `new Object()`. Derives NoCopySpan so
// the container treats it as BORROWED (stack instances are never freed by the
// container, and the identity survives copies/concat).
struct MarkSpan : public NoCopySpan {};

// SpannableTest.testGetSpans — abutment semantics, parameterized over the
// implementation like the abstract AOSP base class.
void spannableTestGetSpans(Spannable& spannable) {
    ParcelableSpan* emptySpan = new UnderlineSpan;
    spannable.setSpan(emptySpan, 1, 1, 0);
    ParcelableSpan* unemptySpan = new UnderlineSpan;
    spannable.setSpan(unemptySpan, 1, 2, 0);

    // Empty spans are included when they merely abut the query region
    // but other spans are not, unless the query region is empty, in
    // in which case any abutting spans are returned.
    auto spans = spannable.getSpans(0, 1, make_span_filter<ParcelableSpan>());
    ASSERT_EQ(1u, spans.size());
    EXPECT_EQ(emptySpan, spans[0]);

    spans = spannable.getSpans(0, 2, make_span_filter<ParcelableSpan>());
    ASSERT_EQ(2u, spans.size());
    EXPECT_EQ(emptySpan, spans[0]);
    EXPECT_EQ(unemptySpan, spans[1]);

    spans = spannable.getSpans(1, 2, make_span_filter<ParcelableSpan>());
    ASSERT_EQ(2u, spans.size());
    EXPECT_EQ(emptySpan, spans[0]);
    EXPECT_EQ(unemptySpan, spans[1]);

    spans = spannable.getSpans(2, 2, make_span_filter<ParcelableSpan>());
    ASSERT_EQ(1u, spans.size());
    EXPECT_EQ(unemptySpan, spans[0]);
}

} // namespace

TEST(CoreSpannableStringBuilderTest, testGetSpans) {
    SpannableStringBuilder builder(u"abcdef");
    spannableTestGetSpans(builder);
}

TEST(CoreSpannableStringTest, testGetSpans) {
    SpannableString spannable(u"abcdef");
    spannableTestGetSpans(spannable);
}

TEST(CoreSpannableTest, testRemoveSpanWithIntermediateFlag) {
    GTEST_SKIP() << "CDROID has no removeSpan(Object, int flags) overload — "
                    "SPAN_INTERMEDIATE removal is not expressible (API gap)";
}

// SpannableStringBuilderTest.testGetSpans_sortsByPriorityEvenWhenSortParamIsFalse
TEST(CoreSpannableStringBuilderTest, testGetSpans_sortsByPriorityEvenWhenSortParamIsFalse) {
    std::u16string text = u"p_in_s";
    SpannableStringBuilder builder(text);
    ParcelableSpan* first = new SubscriptSpan;
    ParcelableSpan* second = new UnderlineSpan;
    ParcelableSpan* third = new UnderlineSpan;   // AOSP BulletSpan stand-in
    ParcelableSpan* fourth = new UnderlineSpan;  // AOSP QuoteSpan stand-in

    builder.setSpan(first, 2, 4, Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);
    builder.setSpan(second, 1, (int)text.length(), Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);
    builder.setSpan(third, 2, (int)text.length(), 1 << Spanned::SPAN_PRIORITY_SHIFT);
    builder.setSpan(fourth, 0, (int)text.length(), 2 << Spanned::SPAN_PRIORITY_SHIFT);

    auto spans = builder.getSpans(0, (int)text.length(), make_span_filter<ParcelableSpan>());

    ASSERT_EQ(4u, spans.size());
    // priority spans are first
    EXPECT_EQ(fourth, spans[0]);
    EXPECT_EQ(third, spans[1]);
    // other spans should be there
    EXPECT_EQ(second, spans[2]);
    EXPECT_EQ(first, spans[3]);
}

// --- SpannedTest ---

namespace {

// SpannedTest.Watcher — implements both watcher interfaces; asserts the
// callback notification order matches the span priority order.
class Watcher : public SpanWatcher, public TextWatcher {
private:
    int mSequence;
    int& mExpect;
public:
    explicit Watcher(int sequence, int& expect) : mSequence(sequence), mExpect(expect) {
        TextWatcher::onTextChanged = [this](CharSequence&, int, int, int) {
            if (mExpect != 0) {
                EXPECT_EQ(mSequence, mExpect);
                mExpect = mSequence - 1;
            }
        };
    }

    void onSpanAdded(Spannable&, const ParcelableSpan*, int, int) override {
        if (mExpect != 0) {
            EXPECT_EQ(mSequence, mExpect);
            mExpect = mSequence - 1;
        }
    }
    void onSpanRemoved(Spannable&, const ParcelableSpan*, int, int) override {}
    void onSpanChanged(Spannable&, const ParcelableSpan*, int, int, int, int) override {}
};

void checkPriority(Spannable& s, int& expect) {
    for (int priority : {5, 10, 0, 15, 3, 6, 0}) {
        s.setSpan(new UnderlineSpan, 0, 1, Spanned::SPAN_EXCLUSIVE_EXCLUSIVE |
                (priority << Spanned::SPAN_PRIORITY_SHIFT));
    }

    auto spans = s.getSpans(0, s.length(), make_span_filter<ParcelableSpan>());

    for (size_t i = 0; i + 1 < spans.size(); i++) {
        EXPECT_TRUE((s.getSpanFlags(spans[i]) & Spanned::SPAN_PRIORITY) >=
                    (s.getSpanFlags(spans[i + 1]) & Spanned::SPAN_PRIORITY));
    }

    expect = 0;

    // AOSP news the Watchers and lets GC keep them alive for the container.
    // Watchers are NoCopySpans (borrowed, never freed by the container), so
    // they must outlive every later query on the container — heap-allocate
    // and deliberately leak, exactly like the GC-backed original. A stack
    // instance would dangle in the span table after this helper returns.
    Watcher* watcher2 = new Watcher(2, expect);
    Watcher* watcher4 = new Watcher(4, expect);
    Watcher* watcher1 = new Watcher(1, expect);
    Watcher* watcher3 = new Watcher(3, expect);
    s.setSpan(watcher2, 0, s.length(),
              Spanned::SPAN_INCLUSIVE_INCLUSIVE | (2 << Spanned::SPAN_PRIORITY_SHIFT));
    s.setSpan(watcher4, 0, s.length(),
              Spanned::SPAN_INCLUSIVE_INCLUSIVE | (4 << Spanned::SPAN_PRIORITY_SHIFT));
    s.setSpan(watcher1, 0, s.length(),
              Spanned::SPAN_INCLUSIVE_INCLUSIVE | (1 << Spanned::SPAN_PRIORITY_SHIFT));
    s.setSpan(watcher3, 0, s.length(),
              Spanned::SPAN_INCLUSIVE_INCLUSIVE | (3 << Spanned::SPAN_PRIORITY_SHIFT));

    expect = 4;
    s.setSpan(new UnderlineSpan, 0, 1, Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);
    EXPECT_EQ(0, expect);
}

} // namespace

TEST(CoreSpannedTest, testSpannableString) {
    int expect = 0;
    SpannableString s(u"the quick brown fox");
    checkPriority(s, expect);
}

TEST(CoreSpannedTest, testSpannableStringBuilder) {
    int expect = 0;
    SpannableStringBuilder ssb(u"the quick brown fox");
    checkPriority(ssb, expect);

    // checkPriority2: inserting text notifies the TextWatchers in priority
    // order too.
    expect = 4;
    ssb.insert(3, String(u"something"));
    EXPECT_EQ(0, expect);
}

TEST(CoreSpannedTest, testAppend) {
    UnderlineSpan* o = new UnderlineSpan;
    SpannableString ss(u"Test");
    ss.setSpan(o, 0, (int)ss.length(), Spannable::SPAN_EXCLUSIVE_EXCLUSIVE);

    SpannableStringBuilder ssb;
    ssb.append(ss);
    EXPECT_EQ(0, ssb.getSpanStart(o));
    EXPECT_EQ(4, ssb.getSpanEnd(o));
    EXPECT_EQ(1u, ssb.getSpans(0, 4, make_span_filter<ParcelableSpan>()).size());

    ssb.insert(0, ss);
    EXPECT_EQ(4, ssb.getSpanStart(o));
    EXPECT_EQ(8, ssb.getSpanEnd(o));
    EXPECT_EQ(0u, ssb.getSpans(0, 4, make_span_filter<ParcelableSpan>()).size());
    EXPECT_EQ(1u, ssb.getSpans(4, 8, make_span_filter<ParcelableSpan>()).size());
}

TEST(CoreSpannedTest, testWrapParcel) {
    GTEST_SKIP() << "no TextUtils.writeToParcel / CHAR_SEQUENCE_CREATOR Parcel bridge";
}
