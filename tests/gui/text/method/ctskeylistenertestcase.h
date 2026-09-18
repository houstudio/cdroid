/*********************************************************************************
 * Ported from AOSP CTS KeyListenerTestCase / CtsKeyEventUtil / TextMethodUtils
 * (Apache 2.0) — the shared harness for the android.text.method KeyListener CTS
 * test ports in this directory (digits/dialer/date/time/datetime/qwerty/text/
 * multitap/meta/base).
 *
 * AOSP runs every case against a focused EditText in KeyListenerCtsActivity and
 * injects keys with instrumentation (CtsKeyEventUtil.sendKeys). gui_test has no
 * per-case focused activity, so the delivery is reduced to its unit: an
 * EditText host on the App context (the same host shape backspace_tests.cc
 * uses), with a press delivered as the TextView::onKeyDown/onKeyUp pair the
 * injected event would have produced (insertion happens on DOWN in
 * NumberKeyListener.onKeyDown, matching AOSP). The host stays unattached:
 * Editor's cursor bookkeeping is null-layout guarded, so text-content
 * assertions are unaffected.
 *
 * The multi-tap cases attach the host to the GUIEnvironment stage instead: the
 * 2s MultiTapKeyListener timeout is armed via View::postDelayed, which needs an
 * attached view (unattached posts sit in the run queue and never fire), and
 * those cases pump the shared looper while waiting (pumpFor instead of sleep).
 *
 * KeyEvents are built with deviceId = -1 (the VIRTUAL_KEYBOARD device), exactly
 * like AOSP's `new KeyEvent(action, code)`; KeyEvent::getKeyCharacterMap then
 * resolves through KeyCharacterMap::getDefault() (Generic.kcm).
 *********************************************************************************/
#ifndef TEXT_METHOD_TESTS_CTSKEYLISTENERTESTCASE_H
#define TEXT_METHOD_TESTS_CTSKEYLISTENERTESTCASE_H

#include <cdroid.h>
#include <widget/edittext.h>
#include <text/method/keylistener.h>
#include <view/keyevent.h>
#include <gtest/gtest.h>
#include <string>

namespace cdroid {

class KeyListenerTestCase : public testing::Test {
protected:
    void SetUp() override {
        mTextView = new EditText(&App::getInstance());
    }

    void TearDown() override {
        // An attached host is removed first so the stage teardown between cases
        // never sees a view whose owner is this test.
        if (mTextView->getParent() != nullptr) {
            ((ViewGroup*) mTextView->getParent())->removeView(mTextView);
        }
        delete mTextView;
        mTextView = nullptr;
    }

    // KeyListenerTestCase.setKeyListenerSync: AOSP hops to the UI thread; the
    // test body already runs there, so this is the plain setter.
    void setKeyListenerSync(KeyListener* keyListener) {
        mTextView->setKeyListener(keyListener);
    }

    // CtsKeyEventUtil.sendKey: one logical press (ACTION_DOWN + ACTION_UP
    // sharing a downTime), dispatched to the view like a focused delivery.
    void sendKey(int keyCode, int metaState = 0) {
        const nsecs_t downTime = SystemClock::uptimeMillis();
        KeyEvent* down = KeyEvent::obtain(downTime, downTime, KeyEvent::ACTION_DOWN,
                keyCode, 0, metaState, -1, 0, 0, 0);
        mTextView->onKeyDown(keyCode, *down);
        delete down;

        KeyEvent* up = KeyEvent::obtain(downTime, SystemClock::uptimeMillis(),
                KeyEvent::ACTION_UP, keyCode, 0, metaState, -1, 0, 0, 0);
        mTextView->onKeyUp(keyCode, *up);
        delete up;
    }

    // CtsKeyEventUtil.sendKeys(instrumentation, view, keyCode)
    void sendKeys(int keyCode) { sendKey(keyCode); }

    // CtsKeyEventUtil.sendKeyDownUp — the DOWN/UP pair without the string-
    // character machinery around it.
    void sendKeyDownUp(int keyCode) { sendKey(keyCode); }

public:
    // KeyListenerTestCase.getKey: fresh ACTION_DOWN event with a meta state.
    // Public so free helpers (kcmMatch, getUnacceptedKeyCode callers) reach it.
    static KeyEvent* getKey(int keycode, int metaState) {
        const nsecs_t now = SystemClock::uptimeMillis();
        return KeyEvent::obtain(now, now, KeyEvent::ACTION_DOWN, keycode, 0,
                metaState, -1, 0, 0, 0);
    }

    // A bare ACTION_DOWN event, AOSP `new KeyEvent(KeyEvent.ACTION_DOWN, code)`.
    static KeyEvent* getDownKey(int keycode) { return getKey(keycode, 0); }

    std::u16string text() const { return mTextView->getText().toUTF16(); }

    EditText* mTextView = nullptr;
};

// TextMethodUtils.getUnacceptedKeyCode: first A..Z keycode whose character is
// not in the accepted set, or -1.
inline int getUnacceptedKeyCode(const std::u16string& acceptedChars) {
    for (int keyCode = KeyEvent::KEYCODE_A; keyCode <= KeyEvent::KEYCODE_Z; keyCode++) {
        const nsecs_t now = SystemClock::uptimeMillis();
        KeyEvent* event = KeyEvent::obtain(now, now, KeyEvent::ACTION_DOWN,
                keyCode, 0, 0, -1, 0, 0, 0);
        const char16_t match = event->getMatch(acceptedChars.data(),
                (int)acceptedChars.size(), 0);
        delete event;
        if (match == u'\0') {
            return keyCode;
        }
    }
    return -1;
}

} // namespace cdroid

#endif // TEXT_METHOD_TESTS_CTSKEYLISTENERTESTCASE_H
