/*********************************************************************************
 * Ported from AOSP CTS android.text.method.MultiTapKeyListenerTest (Apache 2.0).
 *
 * The multi-tap session is bounded by a 2s inactivity timeout armed through
 * View::postDelayed, which only fires on an attached view — so this suite's
 * host is attached to the GUIEnvironment stage and each key group is followed
 * by pumpFor(TIME_OUT) (AOSP: Thread.sleep(TIME_OUT)) to both let the wall
 * clock pass and fire the timer on the shared looper. AOSP's @LargeTest
 * sizing (each group costs the timeout) applies here too.
 *
 * AOSP's enableAutoCapSettings/resetAutoCapSettings toggle TEXT_AUTO_CAPS
 * around the capitalize cases; CDROID's TextKeyListener::getPrefs() returns
 * the stock-device default (AUTO_CAP on), which is the state AOSP sets up.
 *
 * KNOWN DEVIATIONS (red, framework fix needs separate authorization):
 *  - testOnKeyDown_capitalizeSentences / testOnKeyDown_capitalizeWords expect
 *    WORDS/SENTENCES auto-capitalization; CDROID's TextUtils::getCapsMode is
 *    a stub (`#if 0` body, textutils.cc) that always returns 0, so the
 *    multi-tap cycle types the lowercase char. Capitalize::CHARACTERS works.
 *
 * NOT PORTED (APIs absent in CDROID):
 *  - testOnSpanAdded (needs a Mockito spy to observe the watcher call).
 *  - The null-CharSequence arm of testOnSpanChanged (a null Spannable&
 *    cannot be expressed; UB, not a NullPointerException).
 *********************************************************************************/
#include "ctskeylistenertestcase.h"
#include "guienvironment.h"
#include <text/String.h>
#include <text/method/multitapkeylistener.h>
#include <text/method/textkeylistener.h>
#include <text/spannablestringbuilder.h>
#include <text/selection.h>
#include <text/inputtype.h>

using namespace cdroid;

namespace {
/**
 * time out of MultiTapKeyListener. longer than 2000ms in case the system is sluggish.
 */
constexpr int TIME_OUT = 3000;

using Capitalize = TextKeyListener::Capitalize;

class MultiTapKeyListenerTest : public KeyListenerTestCase {
protected:
    void SetUp() override {
        KeyListenerTestCase::SetUp();
        // Attached host: the 2s MultiTap timeout posts through the attach
        // handler (unattached posts sit in the run queue and never fire).
        GUIEnvironment::content()->addView(mTextView,
                new ViewGroup::LayoutParams(-2, -2));
        pumpFor(50);
    }

    void prepareEmptyTextView() {
        mTextView->setText(new String(u""), TextView::BufferType::EDITABLE);
        Selection::setSelection(mTextView->getEditableText(), 0, 0);
        EXPECT_EQ(u"", text());
    }

    void callOnKeyDown(MultiTapKeyListener* keyListener, int keyCode, int numTimes) {
        for (int i = 0; i < numTimes; i++) {
            KeyEvent* event = KeyListenerTestCase::getDownKey(keyCode);
            keyListener->onKeyDown(*mTextView, *mTextView->getEditableText(), keyCode, *event);
            delete event;
        }

        // Wait a bit in order to distinguish this character and the next one
        // (and let the posted timeout runnable fire on the shared looper).
        pumpFor(TIME_OUT);
    }

    void addSpace() {
        mTextView->append(String(u" "));
    }
};
} // namespace

// MultiTapKeyListenerTest.testConstructor
TEST_F(MultiTapKeyListenerTest, Constructor) {
    MultiTapKeyListener none(Capitalize::NONE, true);
    MultiTapKeyListener words(Capitalize::WORDS, false);
}

// MultiTapKeyListenerTest.testOnSpanChanged (non-null arm; the watcher body
// only reacts to the ACTIVE dead-key span)
TEST_F(MultiTapKeyListenerTest, OnSpanChanged) {
    MultiTapKeyListener* multiTapKeyListener =
            MultiTapKeyListener::getInstance(true, Capitalize::CHARACTERS);
    SpannableStringBuilder text(u"123456");
    multiTapKeyListener->onSpanChanged(text, Selection::SELECTION_END, 0, 0, 0, 0);
}

// MultiTapKeyListenerTest.testOnKeyDown_capitalizeNone
TEST_F(MultiTapKeyListenerTest, OnKeyDown_capitalizeNone) {
    MultiTapKeyListener* keyListener = MultiTapKeyListener::getInstance(false, Capitalize::NONE);

    prepareEmptyTextView();

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_4, 2);
    EXPECT_EQ(u"h", text());

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_3, 2);
    EXPECT_EQ(u"he", text());

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_5, 3);
    EXPECT_EQ(u"hel", text());

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_5, 3);
    EXPECT_EQ(u"hell", text());

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_6, 3);
    EXPECT_EQ(u"hello", text());
}

// MultiTapKeyListenerTest.testOnKeyDown_capitalizeCharacters
TEST_F(MultiTapKeyListenerTest, OnKeyDown_capitalizeCharacters) {
    MultiTapKeyListener* keyListener = MultiTapKeyListener::getInstance(false,
            Capitalize::CHARACTERS);

    prepareEmptyTextView();

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_4, 2);
    EXPECT_EQ(u"H", text());

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_3, 2);
    EXPECT_EQ(u"HE", text());

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_5, 3);
    EXPECT_EQ(u"HEL", text());

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_5, 3);
    EXPECT_EQ(u"HELL", text());

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_6, 3);
    EXPECT_EQ(u"HELLO", text());
}

// MultiTapKeyListenerTest.testOnKeyDown_capitalizeSentences
TEST_F(MultiTapKeyListenerTest, OnKeyDown_capitalizeSentences) {
    MultiTapKeyListener* keyListener = MultiTapKeyListener::getInstance(false,
            Capitalize::SENTENCES);

    prepareEmptyTextView();

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_4, 2);
    EXPECT_EQ(u"H", text());

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_4, 3);
    EXPECT_EQ(u"Hi", text());

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_1, 1);
    EXPECT_EQ(u"Hi.", text());

    addSpace();

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_2, 2);
    EXPECT_EQ(u"Hi. B", text());

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_9, 3);
    EXPECT_EQ(u"Hi. By", text());

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_3, 2);
    EXPECT_EQ(u"Hi. Bye", text());
}

// MultiTapKeyListenerTest.testOnKeyDown_capitalizeWords
TEST_F(MultiTapKeyListenerTest, OnKeyDown_capitalizeWords) {
    MultiTapKeyListener* keyListener = MultiTapKeyListener::getInstance(false,
            Capitalize::WORDS);

    prepareEmptyTextView();

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_4, 2);
    EXPECT_EQ(u"H", text());

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_4, 3);
    EXPECT_EQ(u"Hi", text());

    addSpace();

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_2, 2);
    EXPECT_EQ(u"Hi B", text());

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_9, 3);
    EXPECT_EQ(u"Hi By", text());

    callOnKeyDown(keyListener, KeyEvent::KEYCODE_3, 2);
    EXPECT_EQ(u"Hi Bye", text());
}

// MultiTapKeyListenerTest.testGetInstance
TEST_F(MultiTapKeyListenerTest, GetInstance) {
    MultiTapKeyListener* listener1 = MultiTapKeyListener::getInstance(false, Capitalize::NONE);
    MultiTapKeyListener* listener2 = MultiTapKeyListener::getInstance(false, Capitalize::NONE);
    MultiTapKeyListener* listener3 = MultiTapKeyListener::getInstance(false, Capitalize::WORDS);
    MultiTapKeyListener* listener4 = MultiTapKeyListener::getInstance(true, Capitalize::NONE);

    EXPECT_NE(nullptr, listener1);
    EXPECT_NE(nullptr, listener2);
    EXPECT_EQ(listener1, listener2);

    EXPECT_NE(listener1, listener3);
    EXPECT_NE(listener4, listener3);
    EXPECT_NE(listener4, listener1);
}

// MultiTapKeyListenerTest.testOnSpanRemoved (watcher body is a no-op; the
// call just must not crash)
TEST_F(MultiTapKeyListenerTest, OnSpanRemoved) {
    MultiTapKeyListener multiTapKeyListener(Capitalize::CHARACTERS, true);
    SpannableStringBuilder text(u"123456");
    NoCopySpan span;
    multiTapKeyListener.onSpanRemoved(text, &span, 0, 0);
}

// MultiTapKeyListenerTest.testGetInputType
TEST_F(MultiTapKeyListenerTest, GetInputType) {
    MultiTapKeyListener* listener = MultiTapKeyListener::getInstance(false, Capitalize::NONE);
    const int plain = InputType::TYPE_CLASS_TEXT;
    EXPECT_EQ(plain, listener->getInputType());

    listener = MultiTapKeyListener::getInstance(true, Capitalize::CHARACTERS);
    const int full = InputType::TYPE_CLASS_TEXT | InputType::TYPE_TEXT_FLAG_CAP_CHARACTERS
            | InputType::TYPE_TEXT_FLAG_AUTO_CORRECT;
    EXPECT_EQ(full, listener->getInputType());
}
