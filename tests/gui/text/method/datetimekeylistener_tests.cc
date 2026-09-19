/*********************************************************************************
 * Ported from AOSP CTS android.text.method.DateTimeKeyListenerTest (Apache 2.0).
 *
 * getAcceptedChars is public in CDROID (protected in AOSP), so no
 * MockDateTimeKeyListener is needed; the static CHARACTERS set is a
 * std::u16string value, so the assertSame/NotSame identity assertions reduce
 * to content comparisons. CtsKeyEventUtil.sendString("1") injects the key
 * events that produce the character; the port reduces that to the digit's
 * DOWN/UP press.
 *
 * NOT PORTED (API absent in CDROID — compat (locale=null) port; no Locale
 * ctors / getInstance(Locale) / localized digit sets):
 *  - testConstructor's Locale variants, testGetInstance's Locale arms,
 *    testGetAcceptedChars' Locale arms, testGetInputType's English/Persian
 *    arms (the deprecated-constructor input type is asserted instead).
 *********************************************************************************/
#include "ctskeylistenertestcase.h"
#include <text/method/datetimekeylistener.h>
#include <text/inputtype.h>

using namespace cdroid;

namespace {
char16_t kcmMatch(int keyCode, const std::u16string& accepted) {
    KeyEvent* event = KeyListenerTestCase::getDownKey(keyCode);
    const char16_t match = event->getMatch(accepted.data(), (int)accepted.size(), 0);
    delete event;
    return match;
}
} // namespace

using DateTimeKeyListenerTest = KeyListenerTestCase;

// DateTimeKeyListenerTest.testConstructor (deprecated empty constructor)
TEST_F(DateTimeKeyListenerTest, Constructor) {
    DateTimeKeyListener listener;
}

// DateTimeKeyListenerTest.testGetInstance
TEST_F(DateTimeKeyListenerTest, GetInstance) {
    DateTimeKeyListener* emptyListener1 = DateTimeKeyListener::getInstance();
    DateTimeKeyListener* emptyListener2 = DateTimeKeyListener::getInstance();

    EXPECT_NE(nullptr, emptyListener1);
    EXPECT_NE(nullptr, emptyListener2);
    EXPECT_EQ(emptyListener1, emptyListener2);
}

// DateTimeKeyListenerTest.testGetAcceptedChars (compat set)
TEST_F(DateTimeKeyListenerTest, GetAcceptedChars) {
    EXPECT_FALSE(DateTimeKeyListener::CHARACTERS.empty());

    DateTimeKeyListener listener;
    EXPECT_EQ(DateTimeKeyListener::CHARACTERS, listener.getAcceptedChars());
}

// DateTimeKeyListenerTest.testGetInputType (deprecated-constructor behavior)
TEST_F(DateTimeKeyListenerTest, GetInputType) {
    // The "normal" input type that has been used consistently until Android O.
    const int dateTimeType = InputType::TYPE_CLASS_DATETIME
            | InputType::TYPE_DATETIME_VARIATION_NORMAL;

    DateTimeKeyListener* listener = DateTimeKeyListener::getInstance();
    EXPECT_EQ(dateTimeType, listener->getInputType());
}

// DateTimeKeyListenerTest.testDateTimeKeyListener
TEST_F(DateTimeKeyListenerTest, DateTimeKeyListener) {
    DateTimeKeyListener* dateTimeKeyListener = DateTimeKeyListener::getInstance();
    setKeyListenerSync(dateTimeKeyListener);
    std::u16string expectedText = u"";
    EXPECT_EQ(expectedText, text());

    // press '1' key.
    sendKeyDownUp(KeyEvent::KEYCODE_1);
    expectedText += u"1";
    EXPECT_EQ(expectedText, text());

    // press '2' key.
    sendKeyDownUp(KeyEvent::KEYCODE_2);
    expectedText += u"2";
    EXPECT_EQ(expectedText, text());

    // press 'a' key if producible
    if (u'a' == kcmMatch(KeyEvent::KEYCODE_A, DateTimeKeyListener::CHARACTERS)) {
        expectedText += u"a";
        sendKeyDownUp(KeyEvent::KEYCODE_A);
        EXPECT_EQ(expectedText, text());
    }

    // press 'p' key if producible
    if (u'p' == kcmMatch(KeyEvent::KEYCODE_P, DateTimeKeyListener::CHARACTERS)) {
        expectedText += u"p";
        sendKeyDownUp(KeyEvent::KEYCODE_P);
        EXPECT_EQ(expectedText, text());
    }

    // press 'm' key if producible
    if (u'm' == kcmMatch(KeyEvent::KEYCODE_M, DateTimeKeyListener::CHARACTERS)) {
        expectedText += u"m";
        sendKeyDownUp(KeyEvent::KEYCODE_M);
        EXPECT_EQ(expectedText, text());
    }

    // press an unaccepted key if it exists.
    const int keyCode = getUnacceptedKeyCode(DateTimeKeyListener::CHARACTERS);
    if (keyCode != -1) {
        sendKeys(keyCode);
        EXPECT_EQ(expectedText, text());
    }

    // remove DateTimeKeyListener
    setKeyListenerSync(nullptr);
    EXPECT_EQ(expectedText, text());

    sendKeyDownUp(KeyEvent::KEYCODE_1);
    EXPECT_EQ(expectedText, text());
}
