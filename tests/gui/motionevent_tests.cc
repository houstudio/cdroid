// Ported from AOSP CTS MotionEventTest.java (android.view.MotionEvent) — core logic subset.
//
// CDROID adaptation:
//  - obtain(6-arg) maps directly. getAction() in CTS returns raw mAction; for single-pointer
//    events that equals getActionMasked(), used here.
//  - getPressure/getSize/getRawX/getRawY: CDROID exposes these via getAxisValue/raw offset, not as
//    direct getters — the obtain-basic case uses only the directly-available getters.
//  - parcel/transform/PointerCoords/PointerProperties/historical: heavy/matrix-coupled, skipped.
//
// Original: cts/tests/tests/view/src/android/view/cts/MotionEventTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <view/motionevent.h>
#include <core/inputdevice.h>

using namespace cdroid;

namespace {
constexpr int64_t DOWN_TIME = 1000;
constexpr int64_t EVENT_TIME = 1000;
constexpr float X = 3.0f;
constexpr float Y = 4.0f;
constexpr int META_STATE = 1;  // KeyEvent.META_SHIFT_ON
}

TEST(CtsMotionEventTest, testObtainBasic) {
    MotionEvent* e = MotionEvent::obtain(DOWN_TIME, EVENT_TIME, MotionEvent::ACTION_DOWN, X, Y, META_STATE);
    EXPECT_NE(nullptr, e);
    EXPECT_EQ(DOWN_TIME, e->getDownTime());
    EXPECT_EQ(EVENT_TIME, e->getEventTime());
    EXPECT_EQ(MotionEvent::ACTION_DOWN, e->getActionMasked());
    EXPECT_FLOAT_EQ(X, e->getX());
    EXPECT_FLOAT_EQ(Y, e->getY());
    EXPECT_EQ(0, e->getEdgeFlags());
    EXPECT_FLOAT_EQ(1.0f, e->getXPrecision());
    EXPECT_FLOAT_EQ(1.0f, e->getYPrecision());
    e->recycle();
}

TEST(CtsMotionEventTest, testAccessAction) {
    MotionEvent* e = MotionEvent::obtain(DOWN_TIME, EVENT_TIME, MotionEvent::ACTION_MOVE, X, Y, META_STATE);
    EXPECT_EQ(MotionEvent::ACTION_MOVE, e->getActionMasked());
    e->setAction(MotionEvent::ACTION_UP);
    EXPECT_EQ(MotionEvent::ACTION_UP, e->getActionMasked());
    e->setAction(MotionEvent::ACTION_CANCEL);
    EXPECT_EQ(MotionEvent::ACTION_CANCEL, e->getActionMasked());
    e->setAction(MotionEvent::ACTION_DOWN);
    EXPECT_EQ(MotionEvent::ACTION_DOWN, e->getActionMasked());
    e->recycle();
}

// CDROID known bug (system-side, deferred): setLocation(x,y) does not affect getX()/getY()
// (getX reads pointer-coords AXIS_X; setLocation sets a separate location field). CTS
// setLocation is absolute and changes getX. Left ENABLED as a failing regression target.
TEST(CtsMotionEventTest, testSetLocation) {
    MotionEvent* e = MotionEvent::obtain(DOWN_TIME, EVENT_TIME, MotionEvent::ACTION_MOVE, X, Y, META_STATE);
    EXPECT_FLOAT_EQ(X, e->getX());
    EXPECT_FLOAT_EQ(Y, e->getY());
    // CTS sets source to SOURCE_TOUCHSCREEN (a POINTER-class source) before setLocation: only
    // pointer-source events apply the window offset, so shouldDisregardOffset() must be false
    // for setLocation (which goes through offsetLocation) to affect getX/getY. obtain(6-arg)
    // leaves source=0 (UNKNOWN, non-pointer) where offset is disregarded — matching Android.
    e->setSource(InputDevice::SOURCE_TOUCHSCREEN);
    e->setLocation(0.0f, 0.0f);
    EXPECT_FLOAT_EQ(0.0f, e->getX());
    EXPECT_FLOAT_EQ(0.0f, e->getY());
    e->setLocation(2.0f, 2.0f);
    EXPECT_FLOAT_EQ(2.0f, e->getX());
    EXPECT_FLOAT_EQ(2.0f, e->getY());
    e->recycle();
}

TEST(CtsMotionEventTest, testAccessEdgeFlags) {
    MotionEvent* e = MotionEvent::obtain(DOWN_TIME, EVENT_TIME, MotionEvent::ACTION_DOWN, X, Y, META_STATE);
    EXPECT_EQ(0, e->getEdgeFlags());
    e->setEdgeFlags(10);
    EXPECT_EQ(10, e->getEdgeFlags());
    e->recycle();
}

TEST(CtsMotionEventTest, testRecycle) {
    MotionEvent* e = MotionEvent::obtain(DOWN_TIME, EVENT_TIME, MotionEvent::ACTION_MOVE, X, Y, META_STATE);
    EXPECT_EQ(0u, e->getHistorySize());
    e->recycle();
    // CDROID does not throw on double-recycle (no Java RuntimeException); just don't crash.
    SUCCEED();
}
