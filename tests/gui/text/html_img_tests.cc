// Html <img> source round-trip regression (AOSP Html.startImg / toHtml).
//
// AOSP startImg builds ImageSpan(d, src) so the src attribute survives the
// parse; toHtml emits <img src="getSource()"> — a fromHtml/toHtml round trip
// must return the same src string. The CDROID port once kept the source as an
// int resource id ( getSource() returned 0 for getter-built spans), so the
// round trip emitted src="0" (before that, nothing at all).
#include <gtest/gtest.h>
#include <text/html.h>
#include <text/spannablestring.h>
#include <text/style/replacementspan.h>
#include <drawable/drawable.h>

using namespace cdroid;

namespace {
// Minimal drawable for the ImageGetter (toHtml never measures it).
struct StubDrawable : public Drawable {
    void draw(Canvas&) override {}
    int getIntrinsicWidth() override { return 8; }
    int getIntrinsicHeight() override { return 8; }
    int getOpacity() const override { return TRANSLUCENT; }
};
} // namespace

TEST(CoreHtmlTest, testImgSourceRoundTrip) {
    StubDrawable stub;
    const std::string src = "cdroid://icons/sample.png";
    int getterCalls = 0;
    Html::ImageGetter getter = [&](const std::string& want) -> Drawable* {
        getterCalls++;
        EXPECT_EQ(src, want);   // the getter sees the parsed src attribute
        return &stub;
    };

    Spanned* spanned = Html::fromHtml("<img src=\"" + src + "\">", getter, nullptr);
    ASSERT_NE(nullptr, spanned);
    EXPECT_EQ(1, getterCalls);

    // ImageSpan carries the source string (AOSP ImageSpan.getSource()).
    const ImageSpan* img = nullptr;
    auto spans = spanned->getSpans(0, spanned->length(),
            make_span_filter<DynamicDrawableSpan>());
    for (auto* s : spans) {
        if (auto* is = dynamic_cast<const ImageSpan*>(s)) img = is;
    }
    ASSERT_NE(nullptr, img);
    EXPECT_EQ(src, img->getSource());

    // toHtml emits the same src attribute back.
    const std::string out = Html::toHtml(*spanned);
    EXPECT_NE(std::string::npos, out.find("<img src=\"" + src + "\">")) << out;

    delete spanned;   // fromHtml returns an owned Spanned
}

TEST(CoreHtmlTest, testImgWithoutGetterStillRoundTripsSource) {
    // No ImageGetter: the placeholder path (or the bare U+FFFC fallback)
    // keeps the src attribute on the span, so toHtml still carries it.
    Spanned* spanned = Html::fromHtml("<b>x</b><img src=\"res/foo.webp\">", nullptr, nullptr);
    ASSERT_NE(nullptr, spanned);
    const std::string out = Html::toHtml(*spanned);
    EXPECT_NE(std::string::npos, out.find("<img src=\"res/foo.webp\">")) << out;
    delete spanned;
}
