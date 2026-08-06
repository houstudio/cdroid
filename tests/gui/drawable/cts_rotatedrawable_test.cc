// AOSP CTS drawable test port (RotateDrawableTest.java). Pure-logic cases only.
//
// Skipped (not ported):
//  - testDraw: renders to a Canvas (CDROID pixels differ from Skia).
//  - testInflate: needs R.drawable.rotatedrawable_rel / rotatedrawable_abs pak assets (not present
//    here). Also CDROID's isPivotXRelative() is declared-but-undefined (link error — see below) and
//    the pivotXRelative parsing diverges from AOSP, so the relative/absolute assertions could not be
//    ported faithfully even with the assets.
//  - testGetIntrinsicWidthAndHeight: compares against R.drawable.testimage (a pak bitmap asset).
//  - testInflateNull: null-arg NPE is a Java-ism.
//  - testMutate: loads R.drawable.rotatedrawable (pak asset) and asserts BitmapDrawable paint alpha
//    propagation (CDROID has no Bitmap/paint-alpha mirror).
//
// CDROID divergences/gaps noted inline:
//  - RotateDrawable.isPivotXRelative() is declared in rotatedrawable.h but NOT defined in
//    rotatedrawable.cc — calling it would be a link error. Every CTS assertion that reads
//    isPivotXRelative() is therefore skipped (testSetPivot, testInflate). isPivotYRelative() is
//    defined and asserted normally.
//  - DrawableWrapper does NOT override getOpacity (CDROID gap vs AOSP, where DrawableWrapper
//    forwards to the inner drawable). RotateDrawable therefore inherits Drawable::getOpacity,
//    which does not reflect the inner drawable — CTS testGetOpacity is not ported.
//  - CTS testSetAlpha/testSetColorFilter inspect BitmapDrawable.getPaint().getAlpha()/getColorFilter
//    via the wrapped drawable; CDROID has no BitmapDrawable, so setAlpha is asserted via the wrapped
//    ColorDrawable's getAlpha, and setColorFilter as a no-throw.
//  - testGetChangingConfigurations uses AOSP Configuration.KEYBOARD_* constants; substituted with
//    arbitrary unique ints (the set/get logic under test is value-agnostic).
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/RotateDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <drawable/rotatedrawable.h>
#include <drawable/drawables.h>
#include <core/app.h>
#include <core/porterduff.h>
#include <core/rect.h>
#include <view/view.h>
#include <view/gravity.h>
#include <guienvironment.h>

using namespace cdroid;

namespace {
// Minimal Drawable that records the exact alpha passed to setAlpha. CTS reads
// BitmapDrawable.getPaint().getAlpha(); CDROID has no BitmapDrawable, and its ColorDrawable floors
// the modulated alpha (setAlpha(255) on opaque → 254), so this tracking mock isolates the
// forwarding logic under test.
class AlphaTrackingDrawable : public Drawable {
public:
    int lastAlpha = -1;
    void draw(Canvas&) override {}
    void setAlpha(int alpha) override { lastAlpha = alpha; }
    int getAlpha() const override { return lastAlpha; }
};

// Records Callback invocations (CTS uses Mockito).
class MockCallback : public Drawable::Callback {
public:
    int invalidateCount = 0;
    int scheduleCount = 0;
    int unscheduleCount = 0;
    Drawable* lastInvalidate = nullptr;
    void invalidateDrawable(Drawable& d) override { invalidateCount++; lastInvalidate = &d; }
    void scheduleDrawable(Drawable&, const Runnable&, int64_t) override { scheduleCount++; }
    void unscheduleDrawable(Drawable&, const Runnable&) override { unscheduleCount++; }
};
} // namespace

class CtsRotateDrawableTest : public testing::Test {};

TEST_F(CtsRotateDrawableTest, testConstructor) {
    // default ctor (no inner drawable).
    RotateDrawable r1;
    EXPECT_EQ(nullptr, r1.getDrawable());

    // explicit null inner.
    RotateDrawable r2(nullptr);
    EXPECT_EQ(nullptr, r2.getDrawable());

    // a real wrapped drawable.
    ColorDrawable* inner = new ColorDrawable(0xFFFF0000);
    RotateDrawable r3(inner);
    EXPECT_EQ(inner, r3.getDrawable());
}

TEST_F(CtsRotateDrawableTest, testSetPivot) {
    RotateDrawable d;
    // defaults: pivot centred, relative.
    EXPECT_FLOAT_EQ(0.5f, d.getPivotX());
    EXPECT_FLOAT_EQ(0.5f, d.getPivotY());
    EXPECT_TRUE(d.isPivotYRelative());
    // NOTE: isPivotXRelative() is declared in rotatedrawable.h but not defined in rotatedrawable.cc
    // (CDROID gap — would be a link error); the matching default-true assertion is skipped.

    d.setPivotX(10.0f);
    EXPECT_FLOAT_EQ(10.0f, d.getPivotX());

    d.setPivotY(10.0f);
    EXPECT_FLOAT_EQ(10.0f, d.getPivotY());

    d.setPivotXRelative(false);
    // isPivotXRelative() not callable (undefined symbol) — not asserted.
    d.setPivotYRelative(false);
    EXPECT_FALSE(d.isPivotYRelative());
}

TEST_F(CtsRotateDrawableTest, testSetDegrees) {
    RotateDrawable d;
    // defaults: 0..360.
    EXPECT_FLOAT_EQ(0.0f, d.getFromDegrees());
    EXPECT_FLOAT_EQ(360.0f, d.getToDegrees());

    d.setFromDegrees(-10.0f);
    EXPECT_FLOAT_EQ(-10.0f, d.getFromDegrees());
    EXPECT_FLOAT_EQ(360.0f, d.getToDegrees());

    d.setToDegrees(10.0f);
    EXPECT_FLOAT_EQ(10.0f, d.getToDegrees());
    EXPECT_FLOAT_EQ(-10.0f, d.getFromDegrees());
}

TEST_F(CtsRotateDrawableTest, testGetChangingConfigurations) {
    RotateDrawable d(new ColorDrawable(0xFFFF0000));
    EXPECT_EQ(0, d.getChangingConfigurations());
    // CTS uses Configuration.KEYBOARD_NOKEYS / KEYBOARD_12KEY; the set/get logic is value-agnostic,
    // so arbitrary unique ints stand in for those constants.
    d.setChangingConfigurations(1);
    EXPECT_EQ(1, d.getChangingConfigurations());
    d.setChangingConfigurations(2);
    EXPECT_EQ(2, d.getChangingConfigurations());
}

TEST_F(CtsRotateDrawableTest, testSetAlpha) {
    AlphaTrackingDrawable* inner = new AlphaTrackingDrawable();
    RotateDrawable d(inner);
    // setAlpha forwards to the wrapped drawable verbatim. CTS reads BitmapDrawable.getPaint()
    // .getAlpha(); the tracking mock stands in (CDROID has no BitmapDrawable, and ColorDrawable
    // floors the modulated alpha).
    d.setAlpha(100);
    EXPECT_EQ(100, inner->getAlpha());
    d.setAlpha(255);
    EXPECT_EQ(255, inner->getAlpha());
}

TEST_F(CtsRotateDrawableTest, testSetColorFilter) {
    RotateDrawable d(new ColorDrawable(0xFFFF0000));
    // forwards to the wrapped drawable; just verify it does not throw. CTS reads the inner paint's
    // color filter (BitmapDrawable-specific); not mirrored in CDROID.
    d.Drawable::setColorFilter(5, PorterDuff::CLEAR);
    SUCCEED();
}

TEST_F(CtsRotateDrawableTest, testGetPadding) {
    RotateDrawable d(new ColorDrawable(0xFFFF0000));
    Rect rect{10, 10, 20, 20};
    // delegates to the wrapped ColorDrawable, which reports no padding.
    EXPECT_FALSE(d.getPadding(rect));
    EXPECT_EQ(0, rect.left);
    EXPECT_EQ(0, rect.top);
    EXPECT_EQ(0, rect.width);
    EXPECT_EQ(0, rect.height);
}

TEST_F(CtsRotateDrawableTest, testSetVisible) {
    RotateDrawable d(new ColorDrawable(0xFFFF0000));
    EXPECT_TRUE(d.isVisible());
    EXPECT_TRUE(d.setVisible(false, false));
    EXPECT_FALSE(d.isVisible());
    EXPECT_FALSE(d.setVisible(false, true));
    EXPECT_FALSE(d.isVisible());
    EXPECT_TRUE(d.setVisible(true, false));
    EXPECT_TRUE(d.isVisible());
}

TEST_F(CtsRotateDrawableTest, testIsStateful) {
    RotateDrawable d(new ColorDrawable(0xFFFF0000));
    // ColorDrawable.isStateful() is false → wrapper not stateful.
    EXPECT_FALSE(d.isStateful());
}

TEST_F(CtsRotateDrawableTest, testGetConstantState) {
    RotateDrawable d(new ColorDrawable(0xFFFF0000));
    EXPECT_NE(nullptr, d.getConstantState());
}

TEST_F(CtsRotateDrawableTest, testInvalidateDrawable) {
    RotateDrawable d(new ColorDrawable(0xFFFF0000));
    MockCallback cb;
    d.setCallback(&cb);
    // DrawableWrapper forwards invalidateDrawable(who) as invalidateDrawable(this) regardless of who.
    ColorDrawable other(0xFF00FF00);
    d.invalidateDrawable(other);
    EXPECT_EQ(1, cb.invalidateCount);
    EXPECT_EQ(&d, cb.lastInvalidate);

    // with no callback, invalidateDrawable must not throw.
    d.setCallback(nullptr);
    d.invalidateDrawable(other);
}

TEST_F(CtsRotateDrawableTest, testScheduleDrawable) {
    RotateDrawable d(new ColorDrawable(0xFFFF0000));
    MockCallback cb;
    d.setCallback(&cb);
    Runnable r = [] {};
    d.scheduleDrawable(*d.getDrawable(), r, 1000);
    EXPECT_EQ(1, cb.scheduleCount);

    d.setCallback(nullptr);
    d.scheduleDrawable(*d.getDrawable(), r, 0);
}

TEST_F(CtsRotateDrawableTest, testUnscheduleDrawable) {
    RotateDrawable d(new ColorDrawable(0xFFFF0000));
    MockCallback cb;
    d.setCallback(&cb);
    Runnable r = [] {};
    d.unscheduleDrawable(*d.getDrawable(), r);
    EXPECT_EQ(1, cb.unscheduleCount);

    d.setCallback(nullptr);
    d.unscheduleDrawable(*d.getDrawable(), r);
}
