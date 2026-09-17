/*********************************************************************************
 * Ported from AOSP CTS android.text.method.DigitsKeyListenerTest (Apache 2.0).
 *
 * KNOWN DEVIATIONS (red, framework fix needs separate authorization):
 *  - The Spanned-source span-copy block in each testFilter* asserts that the
 *    accepted characters survive the filter WITH their spans (AOSP's filter
 *    builds `new SpannableStringBuilder(source, start, end)`, which copies
 *    spans). CDROID's NumberKeyListener::filter rebuilds the result by
 *    appending characters only, so no spans are carried and the
 *    getSpanFlags/getSpanStart/getSpanEnd assertions fail.
 *
 * NOT PORTED (API absent in CDROID — DigitsKeyListener is the compat
 * (locale=null) port; no Locale ctors / getInstance(Locale) / localized digit
 * sets):
 *  - testConstructor's Locale variants, testFilter4_internationalized,
 *    testGetInstance4/5, testGetAcceptedChars2, testGetInputType_English,
 *    testGetInputType_Persian.
 *********************************************************************************/
#include "ctskeylistenertestcase.h"
#include <text/String.h>
#include <text/method/digitskeylistener.h>
#include <text/spannablestring.h>
#include <text/spannablestringbuilder.h>
#include <text/parcelablespan.h>
#include <text/inputtype.h>

using namespace cdroid;

namespace {

// Identity-only span stand-in for AOSP `new Object()` (borrowed container span).
struct MarkSpan : public NoCopySpan {};

// The repeated `digitsKeyListener.filter(source, 0, source.length(), dest,
// dstart, dend)` shape. Returns the owned filter result (null when AOSP gets
// null), so the assertions read exactly like the original.
CharSequence* filterOf(DigitsKeyListener* listener, const std::u16string& source,
        Spanned* dest, int dstart, int dend) {
    String src(source);
    return listener->filter(&src, 0, (int)source.size(), dest, dstart, dend);
}

} // namespace

using DigitsKeyListenerTest = KeyListenerTestCase;

// DigitsKeyListenerTest.testConstructor (non-Locale variants)
TEST_F(DigitsKeyListenerTest, Constructor) {
    DigitsKeyListener plain;
    DigitsKeyListener signAndDecimal(true, true);
    DigitsKeyListener signOnly(true, false);
    DigitsKeyListener decimalOnly(false, true);
    DigitsKeyListener neither(false, false);
}

/*
 * Check point:
 * Current accepted characters are '0'..'9'.
 */
TEST_F(DigitsKeyListenerTest, Filter1) {
    std::u16string source = u"123456";
    const std::u16string destString = u"dest string";

    DigitsKeyListener* digitsKeyListener = DigitsKeyListener::getInstance();
    SpannableString* dest = new SpannableString(destString);
    EXPECT_EQ(nullptr, filterOf(digitsKeyListener, source, dest, 0, (int)dest->length()));
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"a1b2c3d";
    CharSequence* out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"123", out->toUTF16());
    delete out;
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"-a1.b2c3d";
    out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"123", out->toUTF16());
    delete out;
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"+a1.b2c3d";
    out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"123", out->toUTF16());
    delete out;
    EXPECT_EQ(destString, dest->toUTF16());

    MarkSpan what;
    source = u"+a1.b2c3d";
    SpannableString* spannableSource = new SpannableString(source);
    spannableSource->setSpan(&what, 0, (int)spannableSource->length(),
            Spanned::SPAN_POINT_POINT);
    Spanned* filtered = dynamic_cast<Spanned*>(digitsKeyListener->filter(
            spannableSource, 0, (int)spannableSource->length(), dest, 0, (int)dest->length()));
    delete spannableSource;
    ASSERT_NE(nullptr, filtered);
    EXPECT_EQ(u"123", filtered->toUTF16());
    // Red: cdroid's filter drops spans (see file header).
    EXPECT_EQ((int)Spanned::SPAN_POINT_POINT, filtered->getSpanFlags(&what));
    EXPECT_EQ(0, filtered->getSpanStart(&what));
    EXPECT_EQ(3, filtered->getSpanEnd(&what));
    delete filtered;

    EXPECT_EQ(nullptr, filterOf(digitsKeyListener, u"", dest, 0, (int)dest->length()));
    EXPECT_EQ(destString, dest->toUTF16());
    delete dest;
}

/*
 * Check point:
 * Current accepted characters are '0'..'9', '-', '+'.
 */
TEST_F(DigitsKeyListenerTest, Filter2) {
    std::u16string source = u"-123456";
    const std::u16string destString = u"dest string without sign and decimal";

    DigitsKeyListener* digitsKeyListener = DigitsKeyListener::getInstance(true, false);
    SpannableString* dest = new SpannableString(destString);
    EXPECT_EQ(nullptr, filterOf(digitsKeyListener, source, dest, 0, (int)dest->length()));
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"+123456";
    EXPECT_EQ(nullptr, filterOf(digitsKeyListener, source, dest, 0, (int)dest->length()));
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"-a1.b2c3d";
    CharSequence* out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"-123", out->toUTF16());
    delete out;
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"-a1-b2c3d";
    out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"-123", out->toUTF16());
    delete out;
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"+a1-b2c3d";
    out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"+123", out->toUTF16());
    delete out;
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"5-a1-b2c3d";
    out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"5123", out->toUTF16());
    delete out;
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"5-a1+b2c3d";
    out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"5123", out->toUTF16());
    delete out;
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"+5-a1+b2c3d";
    out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"+5123", out->toUTF16());
    delete out;
    EXPECT_EQ(destString, dest->toUTF16());

    MarkSpan what;
    source = u"5-a1+b2c3d";
    SpannableString* spannableSource = new SpannableString(source);
    spannableSource->setSpan(&what, 0, (int)spannableSource->length(),
            Spanned::SPAN_POINT_POINT);
    Spanned* filtered = dynamic_cast<Spanned*>(digitsKeyListener->filter(
            spannableSource, 0, (int)spannableSource->length(), dest, 0, (int)dest->length()));
    delete spannableSource;
    ASSERT_NE(nullptr, filtered);
    EXPECT_EQ(u"5123", filtered->toUTF16());
    // Red: cdroid's filter drops spans (see file header).
    EXPECT_EQ((int)Spanned::SPAN_POINT_POINT, filtered->getSpanFlags(&what));
    EXPECT_EQ(0, filtered->getSpanStart(&what));
    EXPECT_EQ(4, filtered->getSpanEnd(&what));
    delete filtered;

    EXPECT_EQ(nullptr, filterOf(digitsKeyListener, u"", dest, 0, (int)dest->length()));
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"-123456";
    std::u16string endSign = u"789-";
    dest = new SpannableString(endSign);
    out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length() - 1);
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"", out->toUTF16());
    delete out;
    EXPECT_EQ(endSign, dest->toUTF16());

    endSign = u"789+";
    dest = new SpannableString(endSign);
    out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length() - 1);
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"", out->toUTF16());
    delete out;
    EXPECT_EQ(endSign, dest->toUTF16());

    const std::u16string startSign = u"-789";
    dest = new SpannableString(startSign);
    out = filterOf(digitsKeyListener, source, dest, 1, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"123456", out->toUTF16());
    delete out;
    EXPECT_EQ(startSign, dest->toUTF16());

    source = u"+123456";
    dest = new SpannableString(startSign);
    out = filterOf(digitsKeyListener, source, dest, 1, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"123456", out->toUTF16());
    delete out;
    EXPECT_EQ(startSign, dest->toUTF16());
    delete dest;
}

/*
 * Check point:
 * Current accepted characters are '0'..'9', '.'.
 */
TEST_F(DigitsKeyListenerTest, Filter3) {
    std::u16string source = u"123.456";
    const std::u16string destString = u"dest string without sign and decimal";

    DigitsKeyListener* digitsKeyListener = DigitsKeyListener::getInstance(false, true);
    SpannableString* dest = new SpannableString(destString);
    EXPECT_EQ(nullptr, filterOf(digitsKeyListener, source, dest, 0, (int)dest->length()));
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"-a1.b2c3d";
    CharSequence* out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"1.23", out->toUTF16());
    delete out;
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"+a1.b2c3d";
    out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"1.23", out->toUTF16());
    delete out;
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"a1.b2c3d.";
    out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"123.", out->toUTF16());
    delete out;
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"5.a1.b2c3d";
    out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"51.23", out->toUTF16());
    delete out;
    EXPECT_EQ(destString, dest->toUTF16());

    MarkSpan what;
    SpannableString* spannableSource = new SpannableString(source);
    spannableSource->setSpan(&what, 0, (int)spannableSource->length(),
            Spanned::SPAN_POINT_POINT);
    Spanned* filtered = dynamic_cast<Spanned*>(digitsKeyListener->filter(
            spannableSource, 0, (int)spannableSource->length(), dest, 0, (int)dest->length()));
    delete spannableSource;
    ASSERT_NE(nullptr, filtered);
    EXPECT_EQ(u"51.23", filtered->toUTF16());
    // Red: cdroid's filter drops spans (see file header).
    EXPECT_EQ((int)Spanned::SPAN_POINT_POINT, filtered->getSpanFlags(&what));
    EXPECT_EQ(0, filtered->getSpanStart(&what));
    EXPECT_EQ(5, filtered->getSpanEnd(&what));
    delete filtered;

    EXPECT_EQ(nullptr, filterOf(digitsKeyListener, u"", dest, 0, (int)dest->length()));
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"123.456";
    std::u16string endDecimal = u"789.";
    dest = new SpannableString(endDecimal);
    out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length() - 1);
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"123456", out->toUTF16());
    delete out;
    EXPECT_EQ(endDecimal, dest->toUTF16());

    const std::u16string startDecimal = u".789";
    dest = new SpannableString(startDecimal);
    out = filterOf(digitsKeyListener, source, dest, 1, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"123456", out->toUTF16());
    delete out;
    EXPECT_EQ(startDecimal, dest->toUTF16());
    delete dest;
}

/*
 * Check point:
 * Current accepted characters are '0'..'9', '.', '-', '+'.
 */
TEST_F(DigitsKeyListenerTest, Filter4) {
    std::u16string source = u"-123.456";
    const std::u16string destString = u"dest string without sign and decimal";

    DigitsKeyListener* digitsKeyListener = DigitsKeyListener::getInstance(true, true);
    SpannableString* dest = new SpannableString(destString);
    EXPECT_EQ(nullptr, filterOf(digitsKeyListener, source, dest, 0, (int)dest->length()));
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"+123.456";
    EXPECT_EQ(nullptr, filterOf(digitsKeyListener, source, dest, 0, (int)dest->length()));
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"-a1.b2c3d";
    CharSequence* out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"-1.23", out->toUTF16());
    delete out;
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"a1.b-2c+3d.";
    out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"123.", out->toUTF16());
    delete out;
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"-5.a1.b2c+3d";
    out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"-51.23", out->toUTF16());
    delete out;
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"+5.a1.b2c-3d";
    out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"+51.23", out->toUTF16());
    delete out;
    EXPECT_EQ(destString, dest->toUTF16());

    MarkSpan what;
    source = u"-5.a1.b2c+3d";
    SpannableString* spannableSource = new SpannableString(source);
    spannableSource->setSpan(&what, 0, (int)spannableSource->length(),
            Spanned::SPAN_POINT_POINT);
    Spanned* filtered = dynamic_cast<Spanned*>(digitsKeyListener->filter(
            spannableSource, 0, (int)spannableSource->length(), dest, 0, (int)dest->length()));
    delete spannableSource;
    ASSERT_NE(nullptr, filtered);
    EXPECT_EQ(u"-51.23", filtered->toUTF16());
    // Red: cdroid's filter drops spans (see file header).
    EXPECT_EQ((int)Spanned::SPAN_POINT_POINT, filtered->getSpanFlags(&what));
    EXPECT_EQ(0, filtered->getSpanStart(&what));
    EXPECT_EQ(6, filtered->getSpanEnd(&what));
    delete filtered;

    EXPECT_EQ(nullptr, filterOf(digitsKeyListener, u"", dest, 0, (int)dest->length()));
    EXPECT_EQ(destString, dest->toUTF16());

    source = u"-123.456";
    std::u16string endDecimal = u"789.";
    dest = new SpannableString(endDecimal);
    out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length() - 1);
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"-123456", out->toUTF16());
    delete out;
    EXPECT_EQ(endDecimal, dest->toUTF16());

    std::u16string startDecimal = u".789";
    dest = new SpannableString(startDecimal);
    out = filterOf(digitsKeyListener, source, dest, 1, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"123456", out->toUTF16());
    delete out;
    EXPECT_EQ(startDecimal, dest->toUTF16());

    source = u"+123.456";
    endDecimal = u"789.";
    dest = new SpannableString(endDecimal);
    out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length() - 1);
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"+123456", out->toUTF16());
    delete out;
    EXPECT_EQ(endDecimal, dest->toUTF16());

    startDecimal = u".789";
    dest = new SpannableString(startDecimal);
    out = filterOf(digitsKeyListener, source, dest, 1, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"123456", out->toUTF16());
    delete out;
    EXPECT_EQ(startDecimal, dest->toUTF16());

    source = u"-123.456";
    std::u16string endSign = u"789-";
    dest = new SpannableString(endSign);
    out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length() - 1);
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"", out->toUTF16());
    delete out;
    EXPECT_EQ(endSign, dest->toUTF16());

    endSign = u"789+";
    dest = new SpannableString(endSign);
    out = filterOf(digitsKeyListener, source, dest, 0, (int)dest->length() - 1);
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"", out->toUTF16());
    delete out;
    EXPECT_EQ(endSign, dest->toUTF16());

    const std::u16string startSign = u"-789";
    dest = new SpannableString(startSign);
    out = filterOf(digitsKeyListener, source, dest, 1, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"123.456", out->toUTF16());
    delete out;
    EXPECT_EQ(startSign, dest->toUTF16());

    source = u"+123.456";
    dest = new SpannableString(startSign);
    out = filterOf(digitsKeyListener, source, dest, 1, (int)dest->length());
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(u"123.456", out->toUTF16());
    delete out;
    EXPECT_EQ(startSign, dest->toUTF16());
    delete dest;
}

/*
 * Scenario description:
 * Current accepted characters are '0'..'9'.
 */
TEST_F(DigitsKeyListenerTest, DigitsKeyListener1) {
    DigitsKeyListener* digitsKeyListener = DigitsKeyListener::getInstance();

    setKeyListenerSync(digitsKeyListener);
    EXPECT_EQ(u"", text());

    // press '-' key.
    sendKeys(KeyEvent::KEYCODE_MINUS);
    EXPECT_EQ(u"", text());

    // press '1' key.
    sendKeys(KeyEvent::KEYCODE_1);
    EXPECT_EQ(u"1", text());

    // press '.' key.
    sendKeys(KeyEvent::KEYCODE_PERIOD);
    EXPECT_EQ(u"1", text());

    // press '2' key.
    sendKeys(KeyEvent::KEYCODE_2);
    EXPECT_EQ(u"12", text());
}

/*
 * Scenario description:
 * Current accepted characters are '0'..'9', '-', '+'.
 */
TEST_F(DigitsKeyListenerTest, DigitsKeyListener2) {
    DigitsKeyListener* digitsKeyListener = DigitsKeyListener::getInstance(true, false);

    setKeyListenerSync(digitsKeyListener);
    EXPECT_EQ(u"", text());

    // press '-' key.
    sendKeys(KeyEvent::KEYCODE_MINUS);
    EXPECT_EQ(u"-", text());

    // press '1' key.
    sendKeys(KeyEvent::KEYCODE_1);
    EXPECT_EQ(u"-1", text());

    // press '.' key.
    sendKeys(KeyEvent::KEYCODE_PERIOD);
    EXPECT_EQ(u"-1", text());

    // press '+' key.
    sendKeys(KeyEvent::KEYCODE_PLUS);
    EXPECT_EQ(u"-1", text());

    // press '2' key.
    sendKeys(KeyEvent::KEYCODE_2);
    EXPECT_EQ(u"-12", text());

    // press '-' key.
    sendKeys(KeyEvent::KEYCODE_MINUS);
    EXPECT_EQ(u"-12", text());
}

/*
 * Scenario description:
 * Current accepted characters are '0'..'9', '.'.
 */
TEST_F(DigitsKeyListenerTest, DigitsKeyListener3) {
    DigitsKeyListener* digitsKeyListener = DigitsKeyListener::getInstance(false, true);

    setKeyListenerSync(digitsKeyListener);
    EXPECT_EQ(u"", text());

    // press '-' key.
    sendKeys(KeyEvent::KEYCODE_MINUS);
    EXPECT_EQ(u"", text());

    // press '+' key.
    sendKeys(KeyEvent::KEYCODE_PLUS);
    EXPECT_EQ(u"", text());

    // press '1' key.
    sendKeys(KeyEvent::KEYCODE_1);
    EXPECT_EQ(u"1", text());

    // press '.' key.
    sendKeys(KeyEvent::KEYCODE_PERIOD);
    EXPECT_EQ(u"1.", text());

    // press '2' key.
    sendKeys(KeyEvent::KEYCODE_2);
    EXPECT_EQ(u"1.2", text());

    // press '.' key.
    sendKeys(KeyEvent::KEYCODE_PERIOD);
    EXPECT_EQ(u"1.2", text());
}

/*
 * Scenario description:
 * Current accepted characters are '0'..'9', '-', '+', '.'.
 */
TEST_F(DigitsKeyListenerTest, DigitsKeyListener4) {
    DigitsKeyListener* digitsKeyListener = DigitsKeyListener::getInstance(true, true);

    setKeyListenerSync(digitsKeyListener);
    EXPECT_EQ(u"", text());

    // press '+' key.
    sendKeys(KeyEvent::KEYCODE_PLUS);
    EXPECT_EQ(u"+", text());

    // press '1' key.
    sendKeys(KeyEvent::KEYCODE_1);
    EXPECT_EQ(u"+1", text());

    // press '.' key.
    sendKeys(KeyEvent::KEYCODE_PERIOD);
    EXPECT_EQ(u"+1.", text());

    // press '2' key.
    sendKeys(KeyEvent::KEYCODE_2);
    EXPECT_EQ(u"+1.2", text());

    // press '-' key.
    sendKeys(KeyEvent::KEYCODE_MINUS);
    EXPECT_EQ(u"+1.2", text());

    // press '.' key.
    sendKeys(KeyEvent::KEYCODE_PERIOD);
    EXPECT_EQ(u"+1.2", text());
}

/*
 * Scenario description:
 * Current accepted characters are '5', '6', '7', '8', '9'.
 */
TEST_F(DigitsKeyListenerTest, DigitsKeyListener5) {
    const std::u16string accepted = u"56789";
    DigitsKeyListener* digitsKeyListener = DigitsKeyListener::getInstance(accepted);

    setKeyListenerSync(digitsKeyListener);
    EXPECT_EQ(u"", text());

    // press '1' key.
    sendKeys(KeyEvent::KEYCODE_1);
    EXPECT_EQ(u"", text());

    // press '5' key.
    sendKeys(KeyEvent::KEYCODE_5);
    EXPECT_EQ(u"5", text());

    // press '.' key.
    sendKeys(KeyEvent::KEYCODE_PERIOD);
    EXPECT_EQ(u"5", text());

    // press '-' key.
    sendKeys(KeyEvent::KEYCODE_MINUS);
    EXPECT_EQ(u"5", text());

    // remove DigitsKeyListener.
    setKeyListenerSync(nullptr);
    EXPECT_EQ(u"5", text());

    // press '5' key.
    sendKeys(KeyEvent::KEYCODE_5);
    EXPECT_EQ(u"5", text());
}

// DigitsKeyListenerTest.testGetInstance1
TEST_F(DigitsKeyListenerTest, GetInstance1) {
    DigitsKeyListener* listener1 = DigitsKeyListener::getInstance();
    DigitsKeyListener* listener2 = DigitsKeyListener::getInstance();

    EXPECT_NE(nullptr, listener1);
    EXPECT_NE(nullptr, listener2);
    EXPECT_EQ(listener1, listener2);
}

// DigitsKeyListenerTest.testGetInstance2
TEST_F(DigitsKeyListenerTest, GetInstance2) {
    DigitsKeyListener* listener1 = DigitsKeyListener::getInstance(true, true);
    DigitsKeyListener* listener2 = DigitsKeyListener::getInstance(true, true);

    EXPECT_NE(nullptr, listener1);
    EXPECT_NE(nullptr, listener2);
    EXPECT_EQ(listener1, listener2);

    DigitsKeyListener* listener3 = DigitsKeyListener::getInstance(true, false);
    DigitsKeyListener* listener4 = DigitsKeyListener::getInstance(true, false);

    EXPECT_NE(nullptr, listener3);
    EXPECT_NE(nullptr, listener4);
    EXPECT_EQ(listener3, listener4);

    EXPECT_NE(listener1, listener3);
}

// DigitsKeyListenerTest.testGetInstance3
TEST_F(DigitsKeyListenerTest, GetInstance3) {
    DigitsKeyListener* digitsKeyListener = DigitsKeyListener::getInstance(u"abcdefg");
    EXPECT_NE(nullptr, digitsKeyListener);

    digitsKeyListener = DigitsKeyListener::getInstance(u"Android Test");
    EXPECT_NE(nullptr, digitsKeyListener);
}

// DigitsKeyListenerTest.testGetAcceptedChars1 — cdroid exposes
// getAcceptedChars() publicly, so no Mock subclass is needed.
TEST_F(DigitsKeyListenerTest, GetAcceptedChars1) {
    DigitsKeyListener listener;

    EXPECT_EQ(u"0123456789", listener.getAcceptedChars());

    DigitsKeyListener signListener(true, false);
    EXPECT_EQ(u"0123456789-+", signListener.getAcceptedChars());

    DigitsKeyListener decimalListener(false, true);
    EXPECT_EQ(u"0123456789.", decimalListener.getAcceptedChars());

    DigitsKeyListener signDecimalListener(true, true);
    EXPECT_EQ(u"0123456789-+.", signDecimalListener.getAcceptedChars());
}

// DigitsKeyListenerTest.testGetInputType_deprecatedConstructors
TEST_F(DigitsKeyListenerTest, GetInputType_deprecatedConstructors) {
    DigitsKeyListener* digitsKeyListener = DigitsKeyListener::getInstance(false, false);
    int expected = InputType::TYPE_CLASS_NUMBER;
    EXPECT_EQ(expected, digitsKeyListener->getInputType());

    digitsKeyListener = DigitsKeyListener::getInstance(true, false);
    expected = InputType::TYPE_CLASS_NUMBER | InputType::TYPE_NUMBER_FLAG_SIGNED;
    EXPECT_EQ(expected, digitsKeyListener->getInputType());

    digitsKeyListener = DigitsKeyListener::getInstance(false, true);
    expected = InputType::TYPE_CLASS_NUMBER | InputType::TYPE_NUMBER_FLAG_DECIMAL;
    EXPECT_EQ(expected, digitsKeyListener->getInputType());

    digitsKeyListener = DigitsKeyListener::getInstance(true, true);
    expected = InputType::TYPE_CLASS_NUMBER | InputType::TYPE_NUMBER_FLAG_SIGNED
            | InputType::TYPE_NUMBER_FLAG_DECIMAL;
    EXPECT_EQ(expected, digitsKeyListener->getInputType());
}
