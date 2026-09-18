/*********************************************************************************
 * Ported from AOSP CTS android.text.method.DialerKeyListenerTest (Apache 2.0).
 *
 * getAcceptedChars/lookup are public in CDROID (protected in AOSP), so the
 * MockDialerKeyListener is not needed.
 *
 * SKIPPED (C++ has no NullPointerException; a null KeyEvent* dereference is
 * UB, not a catchable exception):
 *  - testLookupNull.
 *********************************************************************************/
#include "ctskeylistenertestcase.h"
#include <text/method/dialerkeylistener.h>
#include <text/spannablestring.h>
#include <text/inputtype.h>

using namespace cdroid;

namespace {
// AOSP MockDialerKeyListener: re-exposes the protected lookup().
class MockDialerKeyListener : public DialerKeyListener {
public:
    int lookup(const KeyEvent& event, Spannable& content) {
        return DialerKeyListener::lookup(event, content);
    }
};
} // namespace

using DialerKeyListenerTest = KeyListenerTestCase;

// DialerKeyListenerTest.testConstructor
TEST_F(DialerKeyListenerTest, Constructor) {
    MockDialerKeyListener listener;
}

// DialerKeyListenerTest.testLookup
TEST_F(DialerKeyListenerTest, Lookup) {
    MockDialerKeyListener mockDialerKeyListener;
    const int events[] = { KeyEvent::KEYCODE_0, KeyEvent::KEYCODE_N, KeyEvent::KEYCODE_A };
    SpannableString span(u""); // no meta spans
    for (int event : events) {
        KeyEvent* keyEvent = KeyListenerTestCase::getDownKey(event);
        const int keyChar = keyEvent->getNumber();
        if (keyChar != 0) {
            EXPECT_EQ(keyChar, mockDialerKeyListener.lookup(*keyEvent, span));
        } else {
            // cannot make any assumptions how the key code gets translated
        }
        delete keyEvent;
    }
}

// DialerKeyListenerTest.testGetInstance
TEST_F(DialerKeyListenerTest, GetInstance) {
    EXPECT_NE(nullptr, DialerKeyListener::getInstance());

    DialerKeyListener* listener1 = DialerKeyListener::getInstance();
    DialerKeyListener* listener2 = DialerKeyListener::getInstance();

    EXPECT_EQ(listener1, listener2);
}

// DialerKeyListenerTest.testGetAcceptedChars
TEST_F(DialerKeyListenerTest, GetAcceptedChars) {
    DialerKeyListener listener;

    EXPECT_EQ(DialerKeyListener::CHARACTERS, listener.getAcceptedChars());
}

// DialerKeyListenerTest.testGetInputType
TEST_F(DialerKeyListenerTest, GetInputType) {
    DialerKeyListener* listener = DialerKeyListener::getInstance();

    const int expected = InputType::TYPE_CLASS_PHONE;
    EXPECT_EQ(expected, listener->getInputType());
}
