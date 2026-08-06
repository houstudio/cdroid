// Ported from AOSP CTS VelocityTrackerTest.java (android.view.VelocityTracker).
//
// CDROID adaptation:
//  - MotionEvent.obtain(6-arg) maps directly (motionevent.h:257).
//  - Time base: CTS uses SystemClock.uptimeMillis() (ms); CDROID MotionEvent takes nsecs_t but the
//    input layer's base unit is ms (see [[inputdevice-time-units]] — VelocityTracker converts
//    internally), so the CTS ms values are passed through unchanged.
//  - VelocityTracker API: obtain/addMovement/computeCurrentVelocity/getXVelocity/getYVelocity/recycle.
//
// Original: cts/tests/tests/view/src/android/view/cts/VelocityTrackerTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <cmath>
#include <view/motionevent.h>
#include <view/velocitytracker.h>

using namespace cdroid;

namespace {
constexpr float TOLERANCE_EXACT     = 0.01f;
constexpr float TOLERANCE_TIGHT     = 0.05f;
constexpr float TOLERANCE_WEAK      = 0.1f;
constexpr float TOLERANCE_VERY_WEAK = 0.2f;

float velError(float expected, float actual) {
    float absError = std::fabs(actual - expected);
    if (absError < 0.001f) return 0;
    if (std::fabs(expected) < 0.001f) return 1;
    return absError / std::fabs(expected);
}

class VelocityTrackerMoveTest : public testing::Test {
protected:
    VelocityTracker* mVt;
    int64_t mTime;       // ms
    int64_t mLastTime;
    float mPx, mPy;
    float mVx, mVy;
    float mAx, mAy;

    void SetUp() override {
        mVt = VelocityTracker::obtain();
        mTime = 1000; mLastTime = 0;
        mPx = 300;   mPy = 600;
        mVx = 0;     mVy = 0;
        mAx = 0;     mAy = 0;
    }
    void TearDown() override { mVt->recycle(); }

    void addMovement() {
        if (mTime > mLastTime) {
            MotionEvent* ev = MotionEvent::obtain(0, mTime, MotionEvent::ACTION_MOVE, mPx, mPy, 0);
            mVt->addMovement(*ev);
            ev->recycle();
            mLastTime = mTime;
            mVt->computeCurrentVelocity(1);
        }
    }
    void move(long duration, long step) {
        addMovement();
        while (duration > 0) {
            duration -= step;
            mTime += step;
            mPx += (mAx / 2 * step + mVx) * step;
            mPy += (mAy / 2 * step + mVy) * step;
            mVx += mAx * step;
            mVy += mAy * step;
            addMovement();
        }
    }
    void pause(long duration) { mTime += duration; }
    void assertVelocity(float tolerance) {
        mVt->computeCurrentVelocity(1);
        float evx = mVt->getXVelocity();
        float evy = mVt->getYVelocity();
        EXPECT_LE(velError(mVx, evx), tolerance) << "vx expected=" << mVx << " actual=" << evx;
        EXPECT_LE(velError(mVy, evy), tolerance) << "vy expected=" << mVy << " actual=" << evy;
    }
};
} // namespace

TEST_F(VelocityTrackerMoveTest, testNoMovement) {
    move(100, 10);
    assertVelocity(TOLERANCE_EXACT);
}

TEST_F(VelocityTrackerMoveTest, testLinearMovement) {
    mVx = 2.0f; mVy = -4.0f;
    move(100, 10);
    assertVelocity(TOLERANCE_TIGHT);
}

TEST_F(VelocityTrackerMoveTest, testAcceleratingMovement) {
    mVx = 2.0f; mVy = -4.0f;
    mAx = 1.0f; mAy = -0.5f;
    move(200, 10);
    assertVelocity(TOLERANCE_WEAK);
}

TEST_F(VelocityTrackerMoveTest, testDeceleratingMovement) {
    mVx = 2.0f; mVy = -4.0f;
    mAx = -1.0f; mAy = 0.2f;
    move(200, 10);
    assertVelocity(TOLERANCE_WEAK);
}

TEST_F(VelocityTrackerMoveTest, testLinearSharpDirectionChange) {
    mVx = 2.0f; mVy = -4.0f;
    move(100, 10);
    assertVelocity(TOLERANCE_TIGHT);
    mVx = -1.0f; mVy = -3.0f;
    move(100, 10);
    assertVelocity(TOLERANCE_WEAK);
    move(100, 10);
    assertVelocity(TOLERANCE_TIGHT);
}

TEST_F(VelocityTrackerMoveTest, testLinearSharpDirectionChangeAfterALongPause) {
    mVx = 2.0f; mVy = -4.0f;
    move(100, 10);
    assertVelocity(TOLERANCE_TIGHT);
    pause(100);
    mVx = -1.0f; mVy = -3.0f;
    move(100, 10);
    assertVelocity(TOLERANCE_TIGHT);
}

TEST_F(VelocityTrackerMoveTest, testChangingAcceleration) {
    mVx = 2.0f; mVy = -4.0f;
    for (float change : {1.f, -2.f, -3.f, -1.f, 1.f}) {
        mAx += 1.0f * change;
        mAy += -0.5f * change;
        move(30, 10);
    }
    assertVelocity(TOLERANCE_VERY_WEAK);
}

TEST_F(VelocityTrackerMoveTest, testUsesRawCoordinates) {
    VelocityTracker* vt = VelocityTracker::obtain();
    const int numevents = 5;
    const int64_t downTime = 1000;
    for (int i = 0; i < numevents; i++) {
        int64_t eventTime = downTime + i * 10;
        int action = (i == 0) ? MotionEvent::ACTION_DOWN : MotionEvent::ACTION_MOVE;
        MotionEvent* ev = MotionEvent::obtain(downTime, eventTime, action, 0.f, 0.f, 0);
        ev->setSource(0x1002);  // CTS InputDevice.SOURCE_TOUCHSCREEN (CDROID has no constant)
        ev->offsetLocation(i * 10, i * 10);
        vt->addMovement(*ev);
        ev->recycle();
    }
    vt->computeCurrentVelocity(1000);
    EXPECT_NE(0.f, vt->getXVelocity());
    EXPECT_NE(0.f, vt->getYVelocity());
    vt->recycle();
}
