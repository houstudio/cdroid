/*********************************************************************************
 * Ported from AOSP CTS android.text.method.DateKeyListenerTest (Apache 2.0).
 *
 * getAcceptedChars is public in CDROID (protected in AOSP), so no
 * MockDateKeyListener is needed; the static CHARACTERS set is a std::u16string
 * value (not a shared char[] identity), so the assertSame/NotSame identity
 * assertions reduce to content comparisons.
 *
 * NOT PORTED (API absent in CDROID — DateKeyListener is the compat
 * (locale=null) port; no Locale ctors / getInstance(Locale) / localized digit
 * sets):
 *  - testConstructor's Locale variants, testGetInstance's Locale arms,
 *    testGetAcceptedChars' Locale arms, testGetInputType's English/Persian
 *    arms (the deprecated-constructor input type is asserted instead).
 *********************************************************************************/
#include "ctskeylistenertestcase.h"
#include <text/method/datekeylistener.h>
#include <text/inputtype.h>

using namespace cdroid;

using DateKeyListenerTest = KeyListenerTestCase;

// DateKeyListenerTest.testConstructor (deprecated empty constructor)
TEST_F(DateKeyListenerTest, Constructor) {
    DateKeyListener listener;
}

// DateKeyListenerTest.testGetInstance
TEST_F(DateKeyListenerTest, GetInstance) {
    DateKeyListener* emptyListener1 = DateKeyListener::getInstance();
    DateKeyListener* emptyListener2 = DateKeyListener::getInstance();

    EXPECT_NE(nullptr, emptyListener1);
    EXPECT_NE(nullptr, emptyListener2);
    EXPECT_EQ(emptyListener1, emptyListener2);
}

// DateKeyListenerTest.testGetAcceptedChars (compat set)
TEST_F(DateKeyListenerTest, GetAcceptedChars) {
    EXPECT_FALSE(DateKeyListener::CHARACTERS.empty());

    DateKeyListener listener;
    EXPECT_EQ(DateKeyListener::CHARACTERS, listener.getAcceptedChars());
}

// DateKeyListenerTest.testGetInputType (deprecated-constructor behavior)
TEST_F(DateKeyListenerTest, GetInputType) {
    // The "normal" input type that has been used consistently until Android O.
    const int dateTimeType = InputType::TYPE_CLASS_DATETIME
            | InputType::TYPE_DATETIME_VARIATION_DATE;

    DateKeyListener* listener = DateKeyListener::getInstance();
    EXPECT_EQ(dateTimeType, listener->getInputType());
}

/*
 * Scenario description:
 * 1. Press '1' key and check if the content of TextView becomes "1"
 * 2. Press '2' key and check if the content of TextView becomes "12"
 * 3. Press an unaccepted key if it exists and this key will not be accepted.
 * 4. Press '-' key and check if the content of TextView becomes "12-"
 * 5. Press '/' key and check if the content of TextView becomes "12-/"
 * 6. remove DateKeyListener and Press '/' key, this key will not be accepted
 */
TEST_F(DateKeyListenerTest, DateTimeKeyListener) {
    DateKeyListener* dateKeyListener = DateKeyListener::getInstance();

    setKeyListenerSync(dateKeyListener);
    EXPECT_EQ(u"", text());

    // press '1' key.
    sendKeys(KeyEvent::KEYCODE_1);
    EXPECT_EQ(u"1", text());

    // press '2' key.
    sendKeys(KeyEvent::KEYCODE_2);
    EXPECT_EQ(u"12", text());

    // press an unaccepted key if it exists.
    const int keyCode = getUnacceptedKeyCode(DateKeyListener::CHARACTERS);
    if (keyCode != -1) {
        sendKeys(keyCode);
        EXPECT_EQ(u"12", text());
    }

    // press '-' key.
    sendKeys(KeyEvent::KEYCODE_MINUS);
    EXPECT_EQ(u"12-", text());

    // press '/' key.
    sendKeys(KeyEvent::KEYCODE_SLASH);
    EXPECT_EQ(u"12-/", text());

    // remove DateKeyListener.
    setKeyListenerSync(nullptr);
    EXPECT_EQ(u"12-/", text());

    // press '/' key, it will not be accepted.
    sendKeys(KeyEvent::KEYCODE_SLASH);
    EXPECT_EQ(u"12-/", text());
}
