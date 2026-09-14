/*********************************************************************************
 * Ported from AOSP coretests android.text.LayoutBidiCursorPathTest (Apache 2.0).
 *
 * Asserts the cursor Path Layout.getCursorPath draws for an LTR+RTL line under
 * the special 1em bidi test font (tests/gui/assets/assets/font/
 * 1em_bidi_font.ttf, 1em advance, sTypoLineGap = Height/5), plain / with the
 * SHIFT meta triangle / with the ALT meta triangle.
 *
 * AOSP compares Path.approximate(0f) arrays; CDROID's Path has the same
 * approximate(std::vector<float>&, float) flattening, compared element-wise.
 * The SpannableStringBuilder must outlive the layout (borrowed base text).
 *********************************************************************************/
#include <text/staticlayout.h>
#include <text/spannablestringbuilder.h>
#include <text/textpaint.h>
#include <core/typeface.h>
#include <text/method/metakeylistener.h>
#include <core/path.h>
#include <view/keyevent.h>
#include <cdroid.h>
#include <gtest/gtest.h>
#include <cmath>
#include <string>
#include <vector>

using namespace cdroid;

namespace {

constexpr float BIDI_TEXT_SIZE = 12.f;
// "hello" + Arabic "marhaba" (U+0645 U+0631 U+062D U+0628 U+0627).
const std::u16string LTR_TEXT = u"hello";
const std::u16string RTL_TEXT = { char16_t(0x0645), char16_t(0x0631), char16_t(0x062D),
                                  char16_t(0x0628), char16_t(0x0627) };

// MetaKeyKeyListener leaves KeyListener::getInputType pure (AOSP instantiates
// it as an anonymous subclass).
struct TestMetaKeyKeyListener : public MetaKeyKeyListener {
    int getInputType() const override { return 0; }
};

class LayoutBidiCursorPathTest : public testing::Test {
protected:
    void SetUp() override {
        mTextPaint.setTypeface(Typeface::createFromAsset("font/1em_bidi_font.ttf"));
        mTextPaint.setTextSize(BIDI_TEXT_SIZE);
    }

    void setupLayoutAndGetCursorPath(Path& path) {
        StaticLayout::Builder* builder = StaticLayout::Builder::obtain(
                &mBidiText, 0, (int) mBidiText.length(), &mTextPaint, INT_MAX);
        builder->setIncludePad(false);
        StaticLayout* layout = builder->build();
        layout->getCursorPath((int) LTR_TEXT.length(), path, &mBidiText);
        // build() recycles the Builder into its pool (do NOT delete the
        // builder); the returned layout is owned and spent once the path is
        // filled — free it here.
        delete layout;
    }

    // AOSP assertArrayEquals(expected.approximate(0f), actual.approximate(0f), 0f)
    static void assertSamePath(Path& expected, Path& actual) {
        std::vector<float> e, a;
        expected.approximate(e, 0.f);
        actual.approximate(a, 0.f);
        ASSERT_EQ(e.size(), a.size());
        for (size_t i = 0; i < e.size(); i++) {
            EXPECT_EQ(e[i], a[i]) << "approximate[" << i << "]";
        }
    }

    SpannableStringBuilder mBidiText { LTR_TEXT + RTL_TEXT };
    TextPaint mTextPaint;
};

TEST_F(LayoutBidiCursorPathTest, testGetCursorPathSegments) {
    // Setup layout and Act.
    Path actualPath;
    setupLayoutAndGetCursorPath(actualPath);

    // Expected path.
    const float h1 = BIDI_TEXT_SIZE * LTR_TEXT.length() - 0.5f;
    const int top = 0;
    // sTypoLineGap is set to 1/5 of the Height in font metrics of the font file used here.
    const int bottom = (int) std::lround(BIDI_TEXT_SIZE + BIDI_TEXT_SIZE / 5.f);

    Path expectedPath;
    expectedPath.moveTo(h1, top);
    expectedPath.lineTo(h1, bottom);

    // Assert.
    assertSamePath(expectedPath, actualPath);
}

TEST_F(LayoutBidiCursorPathTest, testGetCursorPath_whenShiftIsPressed) {
    // When shift is pressed a triangle is drawn at the bottom quarter of the cursor.
    // Set up key.
    TestMetaKeyKeyListener metaKeyKeyListener;
    EditText host(&App::getInstance());   // AOSP passes null view/event; only the
    const nsecs_t now = SystemClock::uptimeMillis();   // keycode is consumed
    KeyEvent* keyEvent = KeyEvent::obtain(now, now, KeyEvent::ACTION_DOWN,
            KeyEvent::KEYCODE_SHIFT_RIGHT, 0, 0, 0, 0, 0, 0);
    metaKeyKeyListener.onKeyDown(host, mBidiText, KeyEvent::KEYCODE_SHIFT_RIGHT, *keyEvent);
    delete keyEvent;

    // Setup layout and Act.
    Path actualPath;
    setupLayoutAndGetCursorPath(actualPath);

    // Expected path.
    const float h1 = BIDI_TEXT_SIZE * LTR_TEXT.length() - 0.5f;
    const int top = 0;
    int bottom = (int) std::lround(BIDI_TEXT_SIZE + BIDI_TEXT_SIZE / 5.f);
    // Draw a triangle at the bottom quarter of the cursor, thus cut the cursor to its 3/4
    // length.
    const int dist = (bottom - top) / 4;
    bottom -= dist;

    Path expectedPath;
    expectedPath.moveTo(h1, top);
    expectedPath.lineTo(h1, bottom);

    expectedPath.moveTo(h1, bottom);
    expectedPath.lineTo(h1 - dist, bottom + dist);

    expectedPath.moveTo(h1 - dist, bottom + dist - 0.5f);
    expectedPath.lineTo(h1 + dist, bottom + dist - 0.5f);

    expectedPath.moveTo(h1 + dist, bottom + dist);
    expectedPath.lineTo(h1, bottom);

    // Assert.
    assertSamePath(expectedPath, actualPath);
}

TEST_F(LayoutBidiCursorPathTest, testGetCursorPath_whenAltIsPressed) {
    // When alt is pressed a triangle is drawn at the top quarter of the cursor.
    TestMetaKeyKeyListener metaKeyKeyListener;
    EditText host(&App::getInstance());
    const nsecs_t now = SystemClock::uptimeMillis();
    KeyEvent* keyEvent = KeyEvent::obtain(now, now, KeyEvent::ACTION_DOWN,
            KeyEvent::KEYCODE_ALT_RIGHT, 0, 0, 0, 0, 0, 0);
    metaKeyKeyListener.onKeyDown(host, mBidiText, KeyEvent::KEYCODE_ALT_RIGHT, *keyEvent);
    delete keyEvent;

    // Setup layout and Act.
    Path actualPath;
    setupLayoutAndGetCursorPath(actualPath);

    // Expected path.
    const float h1 = BIDI_TEXT_SIZE * LTR_TEXT.length() - 0.5f;
    int top = 0;
    const int bottom = (int) std::lround(BIDI_TEXT_SIZE + BIDI_TEXT_SIZE / 5.f);
    // Draw a triangle at the top quarter of the cursor, thus cut the cursor to its 3/4 length.
    const int dist = (bottom - top) / 4;
    top += dist;

    Path expectedPath;
    expectedPath.moveTo(h1, top);
    expectedPath.lineTo(h1, bottom);

    expectedPath.moveTo(h1, top);
    expectedPath.lineTo(h1 - dist, top - dist);

    expectedPath.moveTo(h1 - dist, top - dist + 0.5f);
    expectedPath.lineTo(h1 + dist, top - dist + 0.5f);

    expectedPath.moveTo(h1 + dist, top - dist);
    expectedPath.lineTo(h1, top);

    // Assert.
    assertSamePath(expectedPath, actualPath);
}

} // namespace
