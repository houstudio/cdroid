/*********************************************************************************
 * Ported from AOSP CTS android.text.method.TimeKeyListenerTest (Apache 2.0).
 *
 * getAcceptedChars is public in CDROID (protected in AOSP), so no
 * MockTimeKeyListener is needed; the static CHARACTERS set is a std::u16string
 * value, so the assertSame/NotSame identity assertions reduce to content
 * comparisons. CtsKeyEventUtil.sendString("1") injects the key events that
 * produce the character; the port reduces that to the digit's DOWN/UP press.
 *
 * NOT PORTED (API absent in CDROID — compat (locale=null) port; no Locale
 * ctors / getInstance(Locale) / localized digit sets):
 *  - testConstructor's Locale variants, testGetInstance's Locale arms,
 *    testGetAcceptedChars' Locale arms, testGetInputType's English/Persian
 *    arms (the deprecated-constructor input type is asserted instead).
 *********************************************************************************/
#include "ctskeylistenertestcase.h"
#include <text/method/timekeylistener.h>
#include <text/inputtype.h>

using namespace cdroid;

namespace {
// KeyCharacterMap.getMatch(keyCode, accepted): the key's character when it is
// among the accepted set, else '\0' — resolved through the default
// (VIRTUAL_KEYBOARD) map like AOSP's KeyCharacterMap.load(VIRTUAL_KEYBOARD).
char16_t kcmMatch(int keyCode, const std::u16string& accepted) {
    KeyEvent* event = KeyListenerTestCase::getDownKey(keyCode);
    const char16_t match = event->getMatch(accepted.data(), (int)accepted.size(), 0);
    delete event;
    return match;
}
} // namespace

using TimeKeyListenerTest = KeyListenerTestCase;

// TimeKeyListenerTest.testConstructor (deprecated empty constructor)
TEST_F(TimeKeyListenerTest, Constructor) {
    TimeKeyListener listener;
}

// TimeKeyListenerTest.testGetInstance
TEST_F(TimeKeyListenerTest, GetInstance) {
    TimeKeyListener* emptyListener1 = TimeKeyListener::getInstance();
    TimeKeyListener* emptyListener2 = TimeKeyListener::getInstance();

    EXPECT_NE(nullptr, emptyListener1);
    EXPECT_NE(nullptr, emptyListener2);
    EXPECT_EQ(emptyListener1, emptyListener2);
}

// TimeKeyListenerTest.testGetAcceptedChars (compat set)
TEST_F(TimeKeyListenerTest, GetAcceptedChars) {
    EXPECT_FALSE(TimeKeyListener::CHARACTERS.empty());

    TimeKeyListener listener;
    EXPECT_EQ(TimeKeyListener::CHARACTERS, listener.getAcceptedChars());
}

// TimeKeyListenerTest.testGetInputType (deprecated-constructor behavior)
TEST_F(TimeKeyListenerTest, GetInputType) {
    // The "normal" input type that has been used consistently until Android O.
    const int dateTimeType = InputType::TYPE_CLASS_DATETIME
            | InputType::TYPE_DATETIME_VARIATION_TIME;

    TimeKeyListener* listener = TimeKeyListener::getInstance();
    EXPECT_EQ(dateTimeType, listener->getInputType());
}

// TimeKeyListenerTest.testTimeKeyListener
TEST_F(TimeKeyListenerTest, TimeKeyListener) {
    TimeKeyListener* timeKeyListener = TimeKeyListener::getInstance();
    std::u16string expectedText = u"";

    setKeyListenerSync(timeKeyListener);
    EXPECT_EQ(expectedText, text());

    // press '1' key.
    sendKeyDownUp(KeyEvent::KEYCODE_1);
    expectedText += u"1";
    EXPECT_EQ(expectedText, text());

    // press '2' key.
    sendKeyDownUp(KeyEvent::KEYCODE_2);
    expectedText += u"2";
    EXPECT_EQ(u"12", text());

    // press 'a' key if producible
    if (u'a' == kcmMatch(KeyEvent::KEYCODE_A, TimeKeyListener::CHARACTERS)) {
        expectedText += u"a";
        sendKeyDownUp(KeyEvent::KEYCODE_A);
        EXPECT_EQ(expectedText, text());
    }

    // press 'p' key if producible
    if (u'p' == kcmMatch(KeyEvent::KEYCODE_P, TimeKeyListener::CHARACTERS)) {
        expectedText += u"p";
        sendKeyDownUp(KeyEvent::KEYCODE_P);
        EXPECT_EQ(expectedText, text());
    }

    // press 'm' key if producible
    if (u'm' == kcmMatch(KeyEvent::KEYCODE_M, TimeKeyListener::CHARACTERS)) {
        expectedText += u"m";
        sendKeyDownUp(KeyEvent::KEYCODE_M);
        EXPECT_EQ(expectedText, text());
    }

    // press an unaccepted key if it exists.
    const int keyCode = getUnacceptedKeyCode(TimeKeyListener::CHARACTERS);
    if (keyCode != -1) {
        sendKeys(keyCode);
        EXPECT_EQ(expectedText, text());
    }

    setKeyListenerSync(nullptr);

    // press '1' key.
    sendKeyDownUp(KeyEvent::KEYCODE_1);
    EXPECT_EQ(expectedText, text());
}
