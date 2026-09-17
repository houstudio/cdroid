/*********************************************************************************
 * Ported from AOSP CTS android.text.method.MetaKeyKeyListenerTest (Apache 2.0).
 *
 * The AOSP tests exercise two API shapes: the span-based one (state stored as
 * CAP/ALT/SYM/SELECTING marker spans in the Editable) and the pure long-
 * bitmask one (state passed and returned by value). CDROID ports both; the
 * protected resetLockedMeta(Spannable) / clearMetaKeyState(Editable, int)
 * statics are reached through a subclass, exactly like the AOSP
 * MockMetaKeyKeyListener.
 *
 * MockSpannable replicates the AOSP inner class: it only records the flags
 * from the last setSpan and counts removeSpan calls, and reports 0/0/0 for
 * every span query — good enough to observe which of the four meta markers
 * resetLock decided to remove (it gates on getSpanFlags == LOCKED).
 *
 * SKIPPED (C++ has no NullPointerException; a null CharSequence&/Spannable&
 * cannot be expressed):
 *  - the null-text arms of testGetMetaState_withCharSequenceAndKeyEvent /
 *    testGetMetaState_withCharSequenceAndMetaAndKeyEvent /
 *    testResetLockedMeta's third arm.
 *********************************************************************************/
#include "ctskeylistenertestcase.h"
#include <text/method/metakeylistener.h>
#include <text/method/datekeylistener.h>
#include <text/String.h>
#include <text/spannablestringbuilder.h>
#include <text/selection.h>
#include <text/parcelablespan.h>
#include <widget/imageview.h>

using namespace cdroid;

namespace {

// AOSP `new MetaKeyKeyListener() {}` — the anonymous concrete subclass.
class MockMetaKeyKeyListener : public MetaKeyKeyListener {
public:
    int getInputType() const override { return 0; }

    void callResetLockedMeta(Spannable& content) {
        MetaKeyKeyListener::resetLockedMeta(content);
    }

    // AOSP's public static clearMetaKeyState(Spannable, int) — protected in
    // the CDROID port (Editable& form), reached like the AOSP mock reaches
    // the protected member.
    static void callClearMetaKeyState(Editable& text, int states) {
        MetaKeyKeyListener::clearMetaKeyState(text, states);
    }
};

// AOSP MockSpannable: flags from the last setSpan, removeSpan call count, and
// all-zero span queries.
class MockSpannable : public Spannable {
public:
    void setSpan(const ParcelableSpan* what, int start, int end, int flags) override {
        (void)what; (void)start; (void)end;
        mFlags = flags;
    }

    void removeSpan(const ParcelableSpan* what) override {
        (void)what;
        mRemoveSpanCount++;
    }

    std::vector<const ParcelableSpan*> getSpans(int start, int end,
            const SpanFilter& filter) const override {
        (void)start; (void)end; (void)filter;
        return {};
    }

    int getSpanStart(const ParcelableSpan* what) const override { (void)what; return 0; }
    int getSpanEnd(const ParcelableSpan* what) const override { (void)what; return 0; }
    int getSpanFlags(const ParcelableSpan* what) const override { (void)what; return mFlags; }
    int nextSpanTransition(int start, int limit, const SpanFilter& kind) const override {
        (void)start; (void)limit; (void)kind;
        return 0;
    }

    size_t length() const override { return 0; }
    int charAt(int index) const override { (void)index; return 0; }
    void getChars(int start, int end, char16_t* dest, int destPos) const override {
        (void)start; (void)end; (void)dest; (void)destPos;
    }
    String* toString() const override { return new String(u""); }
    std::string toUTF8() const override { return std::string(); }
    std::u16string toUTF16() const override { return std::u16string(); }

    int removeSpanCount() const { return mRemoveSpanCount; }

private:
    int mFlags = 0;
    int mRemoveSpanCount = 0;
};

KeyEvent* metaDownKey(int keycode, int metaState) {
    return KeyEvent::obtain(0, 0, KeyEvent::ACTION_DOWN, keycode, 0, metaState,
            -1, 0, 0, 0);
}

} // namespace

using MetaKeyKeyListenerTest = KeyListenerTestCase;

// MetaKeyKeyListenerTest.testPressKey
TEST_F(MetaKeyKeyListenerTest, PressKey) {
    const std::u16string str = u"123456";
    DateKeyListener numberKeyListener;
    ImageView view(&App::getInstance());
    SpannableStringBuilder* content = new SpannableStringBuilder(str);

    content->setSpan(Selection::SELECTION_START, 0, 0, Spanned::SPAN_POINT_POINT);
    content->setSpan(Selection::SELECTION_END, 0, 0, Spanned::SPAN_POINT_POINT);
    KeyEvent* key = KeyListenerTestCase::getDownKey(KeyEvent::KEYCODE_0);
    numberKeyListener.onKeyDown(view, *content, KeyEvent::KEYCODE_0, *key);
    delete key;
    EXPECT_EQ(u'0', (char16_t)content->charAt(0));

    content->setSpan(Selection::SELECTION_START, 1, 1, Spanned::SPAN_POINT_POINT);
    content->setSpan(Selection::SELECTION_END, 1, 1, Spanned::SPAN_POINT_POINT);
    key = KeyListenerTestCase::getDownKey(KeyEvent::KEYCODE_2);
    numberKeyListener.onKeyDown(view, *content, KeyEvent::KEYCODE_2, *key);
    delete key;
    EXPECT_EQ(u'2', (char16_t)content->charAt(1));

    content->setSpan(Selection::SELECTION_START, 3, 3, Spanned::SPAN_POINT_POINT);
    content->setSpan(Selection::SELECTION_END, 3, 3, Spanned::SPAN_POINT_POINT);
    key = KeyListenerTestCase::getDownKey(KeyEvent::KEYCODE_3);
    numberKeyListener.onKeyDown(view, *content, KeyEvent::KEYCODE_3, *key);
    delete key;
    EXPECT_EQ(u'3', (char16_t)content->charAt(3));
    delete content;
}

// MetaKeyKeyListenerTest.testReleaseKey
TEST_F(MetaKeyKeyListenerTest, ReleaseKey) {
    const std::u16string str = u"123456";
    DateKeyListener numberKeyListener;
    ImageView view(&App::getInstance());
    SpannableStringBuilder* content = new SpannableStringBuilder(str);

    content->setSpan(Selection::SELECTION_START, 0, 0, Spanned::SPAN_POINT_POINT);
    content->setSpan(Selection::SELECTION_END, 0, 0, Spanned::SPAN_POINT_POINT);
    KeyEvent* key = KeyListenerTestCase::getDownKey(KeyEvent::KEYCODE_0);
    numberKeyListener.onKeyUp(view, *content, KeyEvent::KEYCODE_0, *key);
    delete key;
    EXPECT_EQ(str[0], (char16_t)content->charAt(0));

    content->setSpan(Selection::SELECTION_START, 1, 1, Spanned::SPAN_POINT_POINT);
    content->setSpan(Selection::SELECTION_END, 1, 1, Spanned::SPAN_POINT_POINT);
    key = KeyListenerTestCase::getDownKey(KeyEvent::KEYCODE_2);
    numberKeyListener.onKeyUp(view, *content, KeyEvent::KEYCODE_2, *key);
    delete key;
    EXPECT_EQ(str[1], (char16_t)content->charAt(1));

    content->setSpan(Selection::SELECTION_START, 3, 3, Spanned::SPAN_POINT_POINT);
    content->setSpan(Selection::SELECTION_END, 3, 3, Spanned::SPAN_POINT_POINT);
    key = KeyListenerTestCase::getDownKey(KeyEvent::KEYCODE_3);
    numberKeyListener.onKeyUp(view, *content, KeyEvent::KEYCODE_3, *key);
    delete key;
    EXPECT_EQ(str[3], (char16_t)content->charAt(3));
    delete content;
}

// MetaKeyKeyListenerTest.testAdjustMetaAfterKeypress
TEST_F(MetaKeyKeyListenerTest, AdjustMetaAfterKeypress) {
    const char16_t* strings[] = { u"123456", u"abc", u"#@%#$^%^" };
    for (const char16_t* str : strings) {
        SpannableStringBuilder content(str);
        content.setSpan(Selection::SELECTION_START, 0, 0, Spanned::SPAN_POINT_POINT);
        const int len = (int)content.length(); // for one line less than 100
        content.setSpan(Selection::SELECTION_END, len, len, Spanned::SPAN_POINT_POINT);
        MetaKeyKeyListener::adjustMetaAfterKeypress(content);
        EXPECT_EQ((int)Spanned::SPAN_POINT_POINT,
                content.getSpanFlags(Selection::SELECTION_START));
        EXPECT_EQ((int)Spanned::SPAN_POINT_POINT,
                content.getSpanFlags(Selection::SELECTION_END));
    }
}

// MetaKeyKeyListenerTest.testAdjustMetaAfterKeypress2
TEST_F(MetaKeyKeyListenerTest, AdjustMetaAfterKeypress2) {
    int64_t state = MetaKeyKeyListener::adjustMetaAfterKeypress(
            (int64_t)MetaKeyKeyListener::META_SHIFT_ON);
    EXPECT_EQ((int64_t)MetaKeyKeyListener::META_SHIFT_ON, state);

    state = MetaKeyKeyListener::adjustMetaAfterKeypress(
            (int64_t)MetaKeyKeyListener::META_ALT_ON);
    EXPECT_EQ((int64_t)MetaKeyKeyListener::META_ALT_ON, state);

    state = MetaKeyKeyListener::adjustMetaAfterKeypress(
            (int64_t)MetaKeyKeyListener::META_SYM_ON);
    EXPECT_EQ((int64_t)MetaKeyKeyListener::META_SYM_ON, state);

    state = MetaKeyKeyListener::adjustMetaAfterKeypress(0);
    EXPECT_EQ(0, state);
}

// MetaKeyKeyListenerTest.testResetMetaState
TEST_F(MetaKeyKeyListenerTest, ResetMetaState) {
    const char16_t* strings[] = { u"123456", u"abc", u"#@%#$^%^" };
    for (const char16_t* str : strings) {
        SpannableStringBuilder text(str);
        text.setSpan(Selection::SELECTION_START, 0, 0, Spanned::SPAN_POINT_POINT);
        const int len = (int)text.length();
        text.setSpan(Selection::SELECTION_END, len, len, Spanned::SPAN_POINT_POINT);
        MetaKeyKeyListener::resetMetaState(text);
        EXPECT_EQ((int)Spanned::SPAN_POINT_POINT,
                text.getSpanFlags(Selection::SELECTION_START));
        EXPECT_EQ((int)Spanned::SPAN_POINT_POINT,
                text.getSpanFlags(Selection::SELECTION_END));
    }
}

// MetaKeyKeyListenerTest.testGetMetaState
TEST_F(MetaKeyKeyListenerTest, GetMetaState) {
    String digits(u"123456");
    String abc(u"abc");
    String symbols(u"@#$$#^$^");

    EXPECT_EQ(0, MetaKeyKeyListener::getMetaState(digits));
    EXPECT_EQ(0, MetaKeyKeyListener::getMetaState(abc));
    EXPECT_EQ(0, MetaKeyKeyListener::getMetaState(symbols));

    EXPECT_EQ(0, MetaKeyKeyListener::getMetaState(digits,
            MetaKeyKeyListener::META_SHIFT_ON));
    EXPECT_EQ(0, MetaKeyKeyListener::getMetaState(abc,
            MetaKeyKeyListener::META_ALT_ON));
    EXPECT_EQ(0, MetaKeyKeyListener::getMetaState(symbols,
            MetaKeyKeyListener::META_SYM_ON));

    // AOSP's getMetaState(text, 0/-1/MAX_VALUE) arms — the (CharSequence, int)
    // overload resolves by argument type.
    EXPECT_EQ(0, MetaKeyKeyListener::getMetaState(digits, 0));
    EXPECT_EQ(0, MetaKeyKeyListener::getMetaState(abc, -1));
    EXPECT_EQ(0, MetaKeyKeyListener::getMetaState(symbols,
            std::numeric_limits<int>::max()));

    KeyEvent* event = metaDownKey(KeyEvent::KEYCODE_0, 0);
    EXPECT_EQ(0, MetaKeyKeyListener::getMetaState(digits, *event));
    delete event;
}

// MetaKeyKeyListenerTest.testGetMetaState2 (long state forms)
TEST_F(MetaKeyKeyListenerTest, GetMetaState2) {
    EXPECT_EQ(0, MetaKeyKeyListener::getMetaState((int64_t)0));
    EXPECT_EQ(MetaKeyKeyListener::META_SHIFT_ON,
            MetaKeyKeyListener::getMetaState((int64_t)MetaKeyKeyListener::META_SHIFT_ON));
    EXPECT_EQ(MetaKeyKeyListener::META_CAP_LOCKED,
            MetaKeyKeyListener::getMetaState((int64_t)MetaKeyKeyListener::META_CAP_LOCKED));

    EXPECT_EQ(0, MetaKeyKeyListener::getMetaState((int64_t)0,
            MetaKeyKeyListener::META_SYM_ON));
    EXPECT_EQ(1, MetaKeyKeyListener::getMetaState((int64_t)MetaKeyKeyListener::META_SYM_ON,
            MetaKeyKeyListener::META_SYM_ON));
    EXPECT_EQ(2, MetaKeyKeyListener::getMetaState(
            (int64_t)MetaKeyKeyListener::META_SYM_LOCKED, MetaKeyKeyListener::META_SYM_ON));
}

// MetaKeyKeyListenerTest.testGetMetaState_withCharSequenceAndKeyEvent
TEST_F(MetaKeyKeyListenerTest, GetMetaState_withCharSequenceAndKeyEvent) {
    String empty(u"");
    KeyEvent* event = metaDownKey(KeyEvent::KEYCODE_0, KeyEvent::META_SHIFT_MASK);

    EXPECT_EQ(KeyEvent::META_SHIFT_MASK, MetaKeyKeyListener::getMetaState(empty, *event));
    delete event;
}

// MetaKeyKeyListenerTest.testGetMetaState_withCharSequenceAndMetaAndKeyEvent
TEST_F(MetaKeyKeyListenerTest, GetMetaState_withCharSequenceAndMetaAndKeyEvent) {
    String empty(u"");
    KeyEvent* event = metaDownKey(KeyEvent::KEYCODE_0, KeyEvent::META_CTRL_ON);
    EXPECT_EQ(0, MetaKeyKeyListener::getMetaState(empty,
            MetaKeyKeyListener::META_SHIFT_ON, *event));
    delete event;

    event = metaDownKey(KeyEvent::KEYCODE_0, KeyEvent::META_SHIFT_ON);
    EXPECT_EQ(1, MetaKeyKeyListener::getMetaState(empty,
            MetaKeyKeyListener::META_SHIFT_ON, *event));
    delete event;

    event = metaDownKey(KeyEvent::KEYCODE_0, (int)MetaKeyKeyListener::META_SYM_LOCKED);
    EXPECT_EQ(2, MetaKeyKeyListener::getMetaState(empty,
            MetaKeyKeyListener::META_SYM_ON, *event));
    delete event;
}

// MetaKeyKeyListenerTest.testIsMetaTracker
TEST_F(MetaKeyKeyListenerTest, IsMetaTracker) {
    String digits(u"123456");
    String abc(u"abc");
    String symbols(u"@#$$#^$^");
    NoCopySpan span;
    EXPECT_FALSE(MetaKeyKeyListener::isMetaTracker(digits, &span));
    EXPECT_FALSE(MetaKeyKeyListener::isMetaTracker(abc, &span));
    EXPECT_FALSE(MetaKeyKeyListener::isMetaTracker(symbols, &span));
}

// MetaKeyKeyListenerTest.testIsSelectingMetaTracker
TEST_F(MetaKeyKeyListenerTest, IsSelectingMetaTracker) {
    String digits(u"123456");
    String abc(u"abc");
    String symbols(u"@#$$#^$^");
    NoCopySpan span;
    EXPECT_FALSE(MetaKeyKeyListener::isSelectingMetaTracker(digits, &span));
    EXPECT_FALSE(MetaKeyKeyListener::isSelectingMetaTracker(abc, &span));
    EXPECT_FALSE(MetaKeyKeyListener::isSelectingMetaTracker(symbols, &span));
}

// MetaKeyKeyListenerTest.testResetLockedMeta
TEST_F(MetaKeyKeyListenerTest, ResetLockedMeta) {
    MockMetaKeyKeyListener mockMetaKeyKeyListener;

    // LOCKED == SPAN_MARK_MARK | (4 << SPAN_USER_SHIFT): resetLock removes it.
    MockSpannable str;
    NoCopySpan lockedSpan;
    str.setSpan(&lockedSpan, 0, 0, Spanned::SPAN_MARK_MARK
            | (4 << Spanned::SPAN_USER_SHIFT));
    EXPECT_EQ(0, str.removeSpanCount());
    mockMetaKeyKeyListener.callResetLockedMeta(str);
    EXPECT_GE(str.removeSpanCount(), 1);

    // Not LOCKED: resetLock leaves it alone.
    MockSpannable plain;
    NoCopySpan plainSpan;
    plain.setSpan(&plainSpan, 0, 0, Spanned::SPAN_MARK_POINT);
    mockMetaKeyKeyListener.callResetLockedMeta(plain);
    EXPECT_EQ(0, plain.removeSpanCount());
}

// MetaKeyKeyListenerTest.testResetLockedMeta2 (long state form)
TEST_F(MetaKeyKeyListenerTest, ResetLockedMeta2) {
    int64_t state = MetaKeyKeyListener::resetLockedMeta(
            (int64_t)MetaKeyKeyListener::META_CAP_LOCKED);
    EXPECT_EQ(0, state);

    state = MetaKeyKeyListener::resetLockedMeta(
            (int64_t)MetaKeyKeyListener::META_SHIFT_ON);
    EXPECT_EQ((int64_t)MetaKeyKeyListener::META_SHIFT_ON, state);

    state = MetaKeyKeyListener::resetLockedMeta(
            (int64_t)MetaKeyKeyListener::META_ALT_LOCKED);
    EXPECT_EQ(0, state);

    state = MetaKeyKeyListener::resetLockedMeta(
            (int64_t)MetaKeyKeyListener::META_ALT_ON);
    EXPECT_EQ((int64_t)MetaKeyKeyListener::META_ALT_ON, state);

    state = MetaKeyKeyListener::resetLockedMeta(
            (int64_t)MetaKeyKeyListener::META_SYM_LOCKED);
    EXPECT_EQ(0, state);

    state = MetaKeyKeyListener::resetLockedMeta(
            (int64_t)MetaKeyKeyListener::META_SYM_ON);
    EXPECT_EQ((int64_t)MetaKeyKeyListener::META_SYM_ON, state);
}

// MetaKeyKeyListenerTest.testClearMetaKeyState (listener instance form)
TEST_F(MetaKeyKeyListenerTest, ClearMetaKeyState) {
    DateKeyListener numberKeyListener;
    ImageView view(&App::getInstance());
    const char16_t* strings[] = { u"123456", u"abc", u"#@%#$^%^" };
    const int states[] = { MetaKeyKeyListener::META_SHIFT_ON, MetaKeyKeyListener::META_ALT_ON,
            MetaKeyKeyListener::META_SYM_ON };
    for (size_t i = 0; i < 3; i++) {
        SpannableStringBuilder text(strings[i]);
        text.setSpan(Selection::SELECTION_START, 0, 0, Spanned::SPAN_POINT_POINT);
        const int len = (int)text.length();
        text.setSpan(Selection::SELECTION_END, len, len, Spanned::SPAN_POINT_POINT);
        // AOSP passes a null View (unused by the span path); the CDROID
        // override takes View&, so hand it an idle host.
        numberKeyListener.clearMetaKeyState(view, text, states[i]);
        EXPECT_EQ((int)Spanned::SPAN_POINT_POINT,
                text.getSpanFlags(Selection::SELECTION_START));
        EXPECT_EQ((int)Spanned::SPAN_POINT_POINT,
                text.getSpanFlags(Selection::SELECTION_END));
    }
}

// MetaKeyKeyListenerTest.testClearMetaKeyState2 (static Spannable form)
TEST_F(MetaKeyKeyListenerTest, ClearMetaKeyState2) {
    const char16_t* strings[] = { u"123456", u"abc", u"#@%#$^%^" };
    const int states[] = { MetaKeyKeyListener::META_SHIFT_ON, MetaKeyKeyListener::META_ALT_ON,
            MetaKeyKeyListener::META_SYM_ON };
    for (size_t i = 0; i < 3; i++) {
        SpannableStringBuilder text(strings[i]);
        text.setSpan(Selection::SELECTION_START, 0, 0, Spanned::SPAN_POINT_POINT);
        const int len = (int)text.length();
        text.setSpan(Selection::SELECTION_END, len, len, Spanned::SPAN_POINT_POINT);
        MockMetaKeyKeyListener::callClearMetaKeyState(text, states[i]);
        EXPECT_EQ((int)Spanned::SPAN_POINT_POINT,
                text.getSpanFlags(Selection::SELECTION_START));
        EXPECT_EQ((int)Spanned::SPAN_POINT_POINT,
                text.getSpanFlags(Selection::SELECTION_END));
    }
}

// MetaKeyKeyListenerTest.testClearMetaKeyState3 (long state form)
TEST_F(MetaKeyKeyListenerTest, ClearMetaKeyState3) {
    MockMetaKeyKeyListener metaKeyKeyListener;
    int64_t state = metaKeyKeyListener.clearMetaKeyState(
            (int64_t)MetaKeyKeyListener::META_CAP_LOCKED, MetaKeyKeyListener::META_SHIFT_ON);
    EXPECT_EQ(0, state);

    state = metaKeyKeyListener.clearMetaKeyState(
            (int64_t)MetaKeyKeyListener::META_SHIFT_ON, MetaKeyKeyListener::META_SHIFT_ON);
    EXPECT_EQ((int64_t)MetaKeyKeyListener::META_SHIFT_ON, state);

    state = metaKeyKeyListener.clearMetaKeyState(
            (int64_t)MetaKeyKeyListener::META_ALT_LOCKED, MetaKeyKeyListener::META_ALT_ON);
    EXPECT_EQ(0, state);

    state = metaKeyKeyListener.clearMetaKeyState(
            (int64_t)MetaKeyKeyListener::META_ALT_ON, MetaKeyKeyListener::META_ALT_ON);
    EXPECT_EQ((int64_t)MetaKeyKeyListener::META_ALT_ON, state);

    state = metaKeyKeyListener.clearMetaKeyState(
            (int64_t)MetaKeyKeyListener::META_SYM_LOCKED, MetaKeyKeyListener::META_SYM_ON);
    EXPECT_EQ(0, state);

    state = metaKeyKeyListener.clearMetaKeyState(
            (int64_t)MetaKeyKeyListener::META_SYM_ON, MetaKeyKeyListener::META_SYM_ON);
    EXPECT_EQ((int64_t)MetaKeyKeyListener::META_SYM_ON, state);
}

// MetaKeyKeyListenerTest.testHandleKeyDown
TEST_F(MetaKeyKeyListenerTest, HandleKeyDown) {
    // AOSP: new KeyEvent(0, 0, ACTION_DOWN, KEYCODE_SHIFT_LEFT, 0, 0,
    //                    KeyCharacterMap.VIRTUAL_KEYBOARD, 0)
    KeyEvent* fullEvent = metaDownKey(KeyEvent::KEYCODE_SHIFT_LEFT, 0);
    const int64_t state = MetaKeyKeyListener::handleKeyDown(
            (int64_t)MetaKeyKeyListener::META_CAP_LOCKED,
            KeyEvent::KEYCODE_SHIFT_LEFT, *fullEvent);
    delete fullEvent;
    EXPECT_EQ(0, state);
}

// MetaKeyKeyListenerTest.testHandleKeyUp
TEST_F(MetaKeyKeyListenerTest, HandleKeyUp) {
    KeyEvent* fullEvent = KeyEvent::obtain(0, 0, KeyEvent::ACTION_UP,
            KeyEvent::KEYCODE_SHIFT_LEFT, 0, 0, -1, 0, 0, 0);
    const int64_t state = MetaKeyKeyListener::handleKeyUp(
            (int64_t)MetaKeyKeyListener::META_CAP_LOCKED,
            KeyEvent::KEYCODE_SHIFT_LEFT, *fullEvent);
    delete fullEvent;
    EXPECT_EQ(0, state);
}
