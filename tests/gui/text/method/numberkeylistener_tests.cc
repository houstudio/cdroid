/*********************************************************************************
 * Ported from AOSP CTS android.text.method.NumberKeyListenerTest (Apache 2.0).
 *
 * KNOWN DEVIATIONS (red, framework fix needs separate authorization):
 *  - The Spanned-source span-copy block of testFilter asserts the accepted
 *    characters survive the filter WITH their spans (AOSP builds the filtered
 *    result with `new SpannableStringBuilder(source, start, end)`, which copies
 *    spans). CDROID's NumberKeyListener::filter rebuilds the result by
 *    appending characters only, so no spans are carried and the
 *    getSpanFlags/getSpanStart/getSpanEnd assertions fail. Same root cause
 *    would apply to DigitsKeyListenerTest.testFilter1..4.
 * SKIPPED (C++ has no NullPointerException; a null CharSequence or KeyEvent
 * dereference is UB, not a catchable exception):
 *  - testFilter's / testLookup's / testOk's null-argument NPE expectations.
 *********************************************************************************/
#include "ctskeylistenertestcase.h"
#include <text/String.h>
#include <text/method/numberkeylistener.h>
#include <text/spannablestring.h>
#include <text/spannablestringbuilder.h>
#include <text/parcelablespan.h>
#include <text/selection.h>
#include <text/inputtype.h>

using namespace cdroid;

namespace {

// AOSP MockNumberKeyListener: supplies the accepted chars + a null input type,
// and re-exposes the protected lookup()/ok() like the AOSP mock does.
class MockNumberKeyListener : public NumberKeyListener {
public:
    static const std::u16string DIGITS;
    explicit MockNumberKeyListener(const std::u16string& acceptedChars)
        : mAcceptedChars(acceptedChars) {}

    std::u16string getAcceptedChars() const override { return mAcceptedChars; }
    int getInputType() const override { return 0; }

    int lookup(const KeyEvent& event, Spannable& content) {
        return NumberKeyListener::lookup(event, content);
    }

    static bool callOk(const std::u16string& accept, char16_t c) {
        return NumberKeyListener::ok(accept, c);
    }

private:
    std::u16string mAcceptedChars;
};

const std::u16string MockNumberKeyListener::DIGITS = u"0123456789";

// Identity-only span stand-in for AOSP `new Object()` (borrowed container span).
struct MarkSpan : public NoCopySpan {};

} // namespace

using NumberKeyListenerTest = KeyListenerTestCase;

// NumberKeyListenerTest.testFilter
TEST_F(NumberKeyListenerTest, Filter) {
    MockNumberKeyListener listener(MockNumberKeyListener::DIGITS);
    std::u16string source = u"Android test";
    String src(source);
    SpannableString* dest = new SpannableString(u"012345");
    CharSequence* out = listener.filter(&src, 0, (int)source.size(),
            dest, 0, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"", out->toUTF16());
    delete out;
    delete dest;

    source = u"12345";
    src = String(source);
    dest = new SpannableString(u"012345");
    EXPECT_EQ(nullptr, listener.filter(&src, 0, (int)source.size(),
            dest, 0, (int)dest->length()));
    delete dest;

    source = u"";
    src = String(source);
    dest = new SpannableString(u"012345");
    EXPECT_EQ(nullptr, listener.filter(&src, 0, (int)source.size(),
            dest, 0, (int)dest->length()));
    delete dest;

    source = u"12345 Android";
    src = String(source);
    dest = new SpannableString(u"012345 Android-test");
    out = listener.filter(&src, 0, (int)source.size(),
            dest, 0, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"12345", out->toUTF16());
    delete out;

    MarkSpan what;
    SpannableString* spannableSource = new SpannableString(u"12345 Android");
    spannableSource->setSpan(&what, 0, (int)spannableSource->length(),
            Spanned::SPAN_POINT_POINT);
    Spanned* filtered = dynamic_cast<Spanned*>(listener.filter(spannableSource,
            0, (int)spannableSource->length(), dest, 0, (int)dest->length()));
    delete spannableSource;
    delete dest;
    ASSERT_NE(nullptr, filtered);
    EXPECT_EQ(u"12345", filtered->toUTF16());
    // Red: cdroid's filter drops spans (see file header).
    EXPECT_EQ((int)Spanned::SPAN_POINT_POINT, filtered->getSpanFlags(&what));
    EXPECT_EQ(0, filtered->getSpanStart(&what));
    EXPECT_EQ(5, filtered->getSpanEnd(&what));
    delete filtered;
}

// NumberKeyListenerTest.testLookup
TEST_F(NumberKeyListenerTest, Lookup) {
    MockNumberKeyListener listener(MockNumberKeyListener::DIGITS);
    KeyEvent* event1 = KeyListenerTestCase::getDownKey(KeyEvent::KEYCODE_0);
    SpannableString* str = new SpannableString(u"012345");
    EXPECT_EQ(u'0', (char16_t)listener.lookup(*event1, *str));
    delete event1;

    MockNumberKeyListener nothingListener(u"");
    KeyEvent* event2 = KeyListenerTestCase::getDownKey(KeyEvent::KEYCODE_A);
    str = new SpannableString(u"ABCD");
    EXPECT_EQ(u'\0', (char16_t)nothingListener.lookup(*event2, *str));
    delete event2;
}

// NumberKeyListenerTest.testOk
TEST_F(NumberKeyListenerTest, Ok) {
    MockNumberKeyListener listener(MockNumberKeyListener::DIGITS);

    EXPECT_TRUE(MockNumberKeyListener::callOk(listener.getAcceptedChars(), u'3'));
    EXPECT_FALSE(MockNumberKeyListener::callOk(listener.getAcceptedChars(), u'e'));
}

// NumberKeyListenerTest.testPressKey
TEST_F(NumberKeyListenerTest, PressKey) {
    const std::u16string text = u"123456";
    MockNumberKeyListener* listener = new MockNumberKeyListener(MockNumberKeyListener::DIGITS);

    mTextView->setText(new String(text), TextView::BufferType::EDITABLE);
    mTextView->setKeyListener(listener);
    Selection::setSelection(dynamic_cast<Spannable*>(mTextView->getEditableText()), 0, 0);
    EXPECT_EQ(text, this->text());

    // press '0' key.
    sendKeys(KeyEvent::KEYCODE_0);
    EXPECT_EQ(u"0123456", this->text());

    // an unaccepted key if it exists.
    const int keyCode = getUnacceptedKeyCode(MockNumberKeyListener::DIGITS);
    if (keyCode != -1) {
        sendKeys(keyCode);
        // text of TextView will not be changed.
        EXPECT_EQ(u"0123456", this->text());
    }

    // remove NumberKeyListener.
    mTextView->setKeyListener(nullptr);
    // press '0' key.
    sendKeys(KeyEvent::KEYCODE_0);
    EXPECT_EQ(u"0123456", this->text());
}
