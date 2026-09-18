/*********************************************************************************
 * Ported from AOSP CTS android.text.method.QwertyKeyListenerTest (Apache 2.0).
 *
 * AOSP's enableAutoCapSettings/resetAutoCapSettings toggle the TEXT_AUTO_CAPS
 * system setting around the capitalize cases; CDROID's
 * TextKeyListener::getPrefs() takes no Context and returns the stock-device
 * default (AUTO_CAP on), which is the state AOSP sets up, so no toggling is
 * needed. The CDROID ctor takes the extra `fullKeyboard` flag
 * (AOSP sets it from the keyboard type); the tests pass false.
 *
 * KNOWN DEVIATIONS (red, framework fix needs separate authorization):
 *  - testOnKeyDown_capitalizeSentences / testOnKeyDown_capitalizeWords expect
 *    WORDS/SENTENCES auto-capitalization, but CDROID's
 *    TextUtils::getCapsMode is a stub (`#if 0` body, textutils.cc) that
 *    always returns 0, so TextKeyListener::shouldCap only knows NONE and
 *    CHARACTERS. Capitalize::CHARACTERS cases pass (the shortcut works).
 *
 * NOT PORTED (API absent in CDROID):
 *  - testMarkAsReplaced / testMarkAsReplacedNullContent /
 *    testMarkAsReplacedNullOriginal (QwertyKeyListener::markAsReplaced — the
 *    Replaced-span marker for autotext undo — is not ported; autotext itself
 *    is deferred).
 *  - testConstructor's direct-construction arms (the ctor is private,
 *    singleton-only in CDROID; the factory covers the same shapes) and its
 *    `new QwertyKeyListener(null, true)` arm (a null Capitalize has no C++
 *    enum equivalent).
 *********************************************************************************/
#include "ctskeylistenertestcase.h"
#include <text/String.h>
#include <text/method/qwertykeylistener.h>
#include <text/method/textkeylistener.h>
#include <text/spannablestringbuilder.h>
#include <text/selection.h>
#include <text/inputtype.h>

using namespace cdroid;

using QwertyKeyListenerTest = KeyListenerTestCase;
using Capitalize = TextKeyListener::Capitalize;

namespace {
void prepareEmptyTextView(EditText* textView) {
    textView->setText(new String(u""), TextView::BufferType::EDITABLE);
    Selection::setSelection(textView->getEditableText(), 0, 0);
    EXPECT_EQ(u"", textView->getText().toUTF16());
}

void callOnKeyDown(QwertyKeyListener* keyListener, EditText* textView, int keyCode) {
    KeyEvent* event = KeyListenerTestCase::getDownKey(keyCode);
    keyListener->onKeyDown(*textView, *textView->getEditableText(), keyCode, *event);
    delete event;
}
} // namespace

// QwertyKeyListenerTest.testConstructor — the CDROID ctor is private
// (singleton-only via getInstance), so the construction smoke test reduces to
// exercising the factory for the same (cap, autoText) shapes.
TEST_F(QwertyKeyListenerTest, Constructor) {
    QwertyKeyListener* none = QwertyKeyListener::getInstance(false, Capitalize::NONE);
    QwertyKeyListener* words = QwertyKeyListener::getInstance(true, Capitalize::WORDS);
    EXPECT_NE(nullptr, none);
    EXPECT_NE(nullptr, words);
}

// QwertyKeyListenerTest.testOnKeyDown_capitalizeNone
TEST_F(QwertyKeyListenerTest, OnKeyDown_capitalizeNone) {
    QwertyKeyListener* keyListener = QwertyKeyListener::getInstance(false, Capitalize::NONE);

    prepareEmptyTextView(mTextView);

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_H);
    EXPECT_EQ(u"h", text());

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_E);
    EXPECT_EQ(u"he", text());

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_L);
    EXPECT_EQ(u"hel", text());

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_L);
    EXPECT_EQ(u"hell", text());

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_O);
    EXPECT_EQ(u"hello", text());
}

// QwertyKeyListenerTest.testOnKeyDown_capitalizeCharacters
TEST_F(QwertyKeyListenerTest, OnKeyDown_capitalizeCharacters) {
    QwertyKeyListener* keyListener = QwertyKeyListener::getInstance(false,
            Capitalize::CHARACTERS);

    prepareEmptyTextView(mTextView);

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_H);
    EXPECT_EQ(u"H", text());

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_E);
    EXPECT_EQ(u"HE", text());

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_L);
    EXPECT_EQ(u"HEL", text());

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_L);
    EXPECT_EQ(u"HELL", text());

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_O);
    EXPECT_EQ(u"HELLO", text());
}

// QwertyKeyListenerTest.testOnKeyDown_capitalizeSentences
TEST_F(QwertyKeyListenerTest, OnKeyDown_capitalizeSentences) {
    QwertyKeyListener* keyListener = QwertyKeyListener::getInstance(false,
            Capitalize::SENTENCES);

    prepareEmptyTextView(mTextView);

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_H);
    EXPECT_EQ(u"H", text());

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_I);
    EXPECT_EQ(u"Hi", text());

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_PERIOD);
    EXPECT_EQ(u"Hi.", text());

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_SPACE);
    EXPECT_EQ(u"Hi. ", text());

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_B);
    EXPECT_EQ(u"Hi. B", text());

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_Y);
    EXPECT_EQ(u"Hi. By", text());

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_E);
    EXPECT_EQ(u"Hi. Bye", text());
}

// QwertyKeyListenerTest.testOnKeyDown_capitalizeWords
TEST_F(QwertyKeyListenerTest, OnKeyDown_capitalizeWords) {
    QwertyKeyListener* keyListener = QwertyKeyListener::getInstance(false,
            Capitalize::WORDS);

    prepareEmptyTextView(mTextView);

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_H);
    EXPECT_EQ(u"H", text());

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_I);
    EXPECT_EQ(u"Hi", text());

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_SPACE);
    EXPECT_EQ(u"Hi ", text());

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_B);
    EXPECT_EQ(u"Hi B", text());

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_Y);
    EXPECT_EQ(u"Hi By", text());

    callOnKeyDown(keyListener, mTextView, KeyEvent::KEYCODE_E);
    EXPECT_EQ(u"Hi Bye", text());
}

// QwertyKeyListenerTest.testGetInstance
TEST_F(QwertyKeyListenerTest, GetInstance) {
    QwertyKeyListener* listener1 = QwertyKeyListener::getInstance(true, Capitalize::WORDS);
    QwertyKeyListener* listener2 = QwertyKeyListener::getInstance(true, Capitalize::WORDS);
    QwertyKeyListener* listener3 = QwertyKeyListener::getInstance(false, Capitalize::WORDS);
    QwertyKeyListener* listener4 = QwertyKeyListener::getInstance(true, Capitalize::SENTENCES);

    EXPECT_NE(nullptr, listener1);
    EXPECT_NE(nullptr, listener2);
    EXPECT_EQ(listener1, listener2);

    EXPECT_NE(listener1, listener3);
    EXPECT_NE(listener1, listener4);
    EXPECT_NE(listener4, listener3);
}

// QwertyKeyListenerTest.testGetInstanceForFullKeyboard
TEST_F(QwertyKeyListenerTest, GetInstanceForFullKeyboard) {
    QwertyKeyListener* listener = QwertyKeyListener::getInstanceForFullKeyboard();

    EXPECT_NE(nullptr, listener);
    // auto correct and cap flags should not be set
    const int expected = InputType::TYPE_CLASS_TEXT;
    EXPECT_EQ(expected, listener->getInputType());
}

// QwertyKeyListenerTest.testGetInputType
TEST_F(QwertyKeyListenerTest, GetInputType) {
    QwertyKeyListener* listener = QwertyKeyListener::getInstance(false, Capitalize::NONE);
    const int plain = InputType::TYPE_CLASS_TEXT;
    EXPECT_EQ(plain, listener->getInputType());

    listener = QwertyKeyListener::getInstance(false, Capitalize::CHARACTERS);
    const int caps = InputType::TYPE_CLASS_TEXT | InputType::TYPE_TEXT_FLAG_CAP_CHARACTERS;
    EXPECT_EQ(caps, listener->getInputType());
}
