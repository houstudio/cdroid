/*********************************************************************************
 * Ported from AOSP coretests android.text.util.LinkifyTest (Apache 2.0).
 *
 * CDROID adaptation notes:
 *  - AOSP forces an en-US context via createConfigurationContext; CDROID has
 *    no configuration contexts, so the TextView cases run on the harness' App
 *    context and default locale (en in the test environment).
 *  - TextView.getUrls() is not ported; its AOSP body is exactly
 *    mText.getSpans(0, length, URLSpan), which is what the helper below does.
 *  - The four addLinks(Spannable, Pattern, scheme, schemes, ...) cases are
 *    skips: the pattern/schemes overloads and android.util.Patterns are not
 *    ported (Linkify has the 2-arg mask forms only).
 *********************************************************************************/
#include <text/linkify.h>
#include <text/spannablestring.h>
#include <text/spannablestringbuilder.h>
#include <text/style/clickablespan.h>
#include <text/method/linkmovementmethod.h>
#include <widget/textview.h>
#include <cdroid.h>
#include <gtest/gtest.h>

using namespace cdroid;

namespace {

// AOSP TextView.getUrls(): all URLSpans over the text.
static std::vector<const URLSpan*> getUrls(TextView& tv) {
    std::vector<const URLSpan*> urls;
    CharSequence& text = tv.getText();
    Spanned* spanned = dynamic_cast<Spanned*>(&text);
    if (spanned == nullptr) return urls;   // plain (non-spanned) buffer: no urls
    for (const ParcelableSpan* s : spanned->getSpans(0, (int) text.length(),
            make_span_filter<URLSpan>())) {
        // dynamic_cast is required: ParcelableSpan is a virtual base.
        urls.push_back(dynamic_cast<const URLSpan*>(s));
    }
    return urls;
}

TEST(CoreLinkifyTest, testNothing) {
    TextView tv(&App::getInstance());
    tv.setText("Hey, foo@google.com, call 415-555-1212.");

    EXPECT_FALSE(dynamic_cast<LinkMovementMethod*>(tv.getMovementMethod()) != nullptr);
    EXPECT_TRUE(getUrls(tv).empty());
}

TEST(CoreLinkifyTest, testNormal) {
    TextView tv(&App::getInstance());
    tv.setAutoLinkMask(Linkify::ALL);
    tv.setText("Hey, foo@google.com, call +1-415-555-1212.");

    EXPECT_TRUE(dynamic_cast<LinkMovementMethod*>(tv.getMovementMethod()) != nullptr);
    EXPECT_EQ(2u, getUrls(tv).size());
}

TEST(CoreLinkifyTest, testUnclickable) {
    TextView tv(&App::getInstance());
    tv.setAutoLinkMask(Linkify::ALL);
    tv.setLinksClickable(false);
    tv.setText("Hey, foo@google.com, call +1-415-555-1212.");

    EXPECT_FALSE(dynamic_cast<LinkMovementMethod*>(tv.getMovementMethod()) != nullptr);
    EXPECT_EQ(2u, getUrls(tv).size());
}

TEST(CoreLinkifyTest, testAddLinks_addsLinksWhenDefaultSchemeIsNull) {
    GTEST_SKIP() << "Linkify.addLinks(Spannable, Pattern, ...) and android.util.Patterns "
                    "are not ported (2-arg mask forms only)";
}

TEST(CoreLinkifyTest, testAddLinks_addsLinksWhenSchemesArrayIsNull) {
    GTEST_SKIP() << "Linkify.addLinks pattern/schemes overloads not ported";
}

TEST(CoreLinkifyTest, testAddLinks_prependsDefaultSchemeToBeginingOfLink) {
    GTEST_SKIP() << "Linkify.addLinks pattern/schemes overloads not ported";
}

TEST(CoreLinkifyTest, testAddLinks_doesNotPrependSchemeIfSchemeExists) {
    GTEST_SKIP() << "Linkify.addLinks pattern/schemes overloads not ported";
}

} // namespace
