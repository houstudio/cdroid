/*********************************************************************************
 * Ported from AOSP CTS android.text.method.TextKeyListenerTest (Apache 2.0).
 *
 * KNOWN DEVIATIONS (red, framework fix needs separate authorization):
 *  - testShouldCap's WORDS/SENTENCES arms expect Context Start / Word Start
 *    detection, but CDROID's TextUtils::getCapsMode is a stub (`#if 0` body,
 *    textutils.cc) that always returns 0. NONE and CHARACTERS arms pass.
 *
 * NOT PORTED (APIs absent in CDROID):
 *  - testOnSpanAdded / testOnSpanChanged / testOnSpanRemoved and their null
 *    arms — CDROID's TextKeyListener does not implement SpanWatcher (Phase-1
 *    deferral, see textkeylistener.h), and there is no Mockito equivalent to
 *    spy the calls.
 *  - TextKeyListener::release() — not ported (the singleton cache is
 *    process-lifetime); the getInstance identity assertions are kept.
 *  - testOnKeyOther — needs the ACTION_MULTIPLE/KEYCODE_UNKNOWN string event
 *    constructor (KeyEvent(uptimeMillis, text, deviceId, flags)), which
 *    CDROID's KeyEvent does not expose. (Its only text assertion is commented
 *    out in AOSP too — issue 1731439.)
 *  - testShouldCapNull — a null CharSequence dereference is UB in C++, not a
 *    catchable NullPointerException.
 *********************************************************************************/
#include "ctskeylistenertestcase.h"
#include <text/method/textkeylistener.h>
#include <text/String.h>
#include <text/spannablestringbuilder.h>
#include <text/selection.h>
#include <text/inputtype.h>
#include <private/keycharactermap.h>

using namespace cdroid;

using TextKeyListenerTest = KeyListenerTestCase;
using Capitalize = TextKeyListener::Capitalize;

// TextKeyListenerTest.testConstructor
TEST_F(TextKeyListenerTest, Constructor) {
    TextKeyListener none(Capitalize::NONE, true);
}

// TextKeyListenerTest.testShouldCap
TEST_F(TextKeyListenerTest, ShouldCap) {
    String str(u"hello world! man");

    // Index of the characters(start with 0):
    // 'h' = 0; 'w' = 6; 'm' = 13; 'a' = 14
    EXPECT_FALSE(TextKeyListener::shouldCap(Capitalize::NONE, str, 0));
    EXPECT_TRUE(TextKeyListener::shouldCap(Capitalize::SENTENCES, str, 0));
    EXPECT_TRUE(TextKeyListener::shouldCap(Capitalize::WORDS, str, 0));
    EXPECT_TRUE(TextKeyListener::shouldCap(Capitalize::CHARACTERS, str, 0));

    EXPECT_FALSE(TextKeyListener::shouldCap(Capitalize::NONE, str, 6));
    EXPECT_FALSE(TextKeyListener::shouldCap(Capitalize::SENTENCES, str, 6));
    EXPECT_TRUE(TextKeyListener::shouldCap(Capitalize::WORDS, str, 6));
    EXPECT_TRUE(TextKeyListener::shouldCap(Capitalize::CHARACTERS, str, 6));

    EXPECT_FALSE(TextKeyListener::shouldCap(Capitalize::NONE, str, 13));
    EXPECT_TRUE(TextKeyListener::shouldCap(Capitalize::SENTENCES, str, 13));
    EXPECT_TRUE(TextKeyListener::shouldCap(Capitalize::WORDS, str, 13));
    EXPECT_TRUE(TextKeyListener::shouldCap(Capitalize::CHARACTERS, str, 13));

    EXPECT_FALSE(TextKeyListener::shouldCap(Capitalize::NONE, str, 14));
    EXPECT_FALSE(TextKeyListener::shouldCap(Capitalize::SENTENCES, str, 14));
    EXPECT_FALSE(TextKeyListener::shouldCap(Capitalize::WORDS, str, 14));
    EXPECT_TRUE(TextKeyListener::shouldCap(Capitalize::CHARACTERS, str, 14));
}

// TextKeyListenerTest.testGetInstance1
TEST_F(TextKeyListenerTest, GetInstance1) {
    TextKeyListener* listener1 = TextKeyListener::getInstance(true, Capitalize::WORDS);
    TextKeyListener* listener2 = TextKeyListener::getInstance(true, Capitalize::WORDS);
    TextKeyListener* listener3 = TextKeyListener::getInstance(false, Capitalize::WORDS);
    TextKeyListener* listener4 = TextKeyListener::getInstance(true, Capitalize::CHARACTERS);

    EXPECT_NE(nullptr, listener1);
    EXPECT_NE(nullptr, listener2);
    EXPECT_EQ(listener1, listener2);

    EXPECT_NE(listener1, listener3);
    EXPECT_NE(listener1, listener4);
    EXPECT_NE(listener4, listener3);
}

// TextKeyListenerTest.testGetInstance2
TEST_F(TextKeyListenerTest, GetInstance2) {
    TextKeyListener* listener1 = TextKeyListener::getInstance();
    TextKeyListener* listener2 = TextKeyListener::getInstance();

    EXPECT_NE(nullptr, listener1);
    EXPECT_NE(nullptr, listener2);
    EXPECT_EQ(listener1, listener2);
}

// TextKeyListenerTest.testClear
TEST_F(TextKeyListenerTest, Clear) {
    const std::u16string text = u"123456";
    mTextView->setText(new String(text), TextView::BufferType::EDITABLE);

    Editable* content = mTextView->getEditableText();
    ASSERT_NE(nullptr, content);
    EXPECT_EQ(text, content->toUTF16());

    TextKeyListener::clear(*content);
    EXPECT_EQ(u"", content->toUTF16());
}

/**
 * Wait for TIME_OUT, or listener will accept key event as multi tap rather than a new key.
 */
// TextKeyListenerTest.testPressKey
TEST_F(TextKeyListenerTest, PressKey) {
    TextKeyListener* textKeyListener = TextKeyListener::getInstance(false, Capitalize::NONE);

    mTextView->setText(new String(u""), TextView::BufferType::EDITABLE);
    Selection::setSelection(mTextView->getEditableText(), 0, 0);
    mTextView->setKeyListener(textKeyListener);
    EXPECT_EQ(u"", text());

    sendKeys(KeyEvent::KEYCODE_4);
    // Multi-tap timeout: KEYCODE_4 pressed once types the digit itself on an
    // ALPHA/FULL keyboard; the harness is single-shot (no repeat), so no wait
    // for the 2s MultiTap window is needed — TextKeyListener is not a
    // MultiTapKeyListener.
    const std::u16string text = this->text();
    const int keyType = KeyCharacterMap::getDefault()->getKeyboardType();
    if (keyType == KeyCharacterMap::KEYBOARD_TYPE_ALPHA
            || keyType == KeyCharacterMap::KEYBOARD_TYPE_FULL) {
        EXPECT_EQ(u"4", text);
    } else if (keyType == KeyCharacterMap::KEYBOARD_TYPE_NUMERIC) {
        EXPECT_EQ(u"g", text);
    } else {
        EXPECT_EQ(u"", text);
    }
}

// TextKeyListenerTest.testGetInputType
TEST_F(TextKeyListenerTest, GetInputType) {
    TextKeyListener* listener = TextKeyListener::getInstance(false, Capitalize::NONE);
    const int plain = InputType::TYPE_CLASS_TEXT;
    EXPECT_EQ(plain, listener->getInputType());

    listener = TextKeyListener::getInstance(false, Capitalize::CHARACTERS);
    const int caps = InputType::TYPE_CLASS_TEXT | InputType::TYPE_TEXT_FLAG_CAP_CHARACTERS;
    EXPECT_EQ(caps, listener->getInputType());
}
