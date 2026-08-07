// AOSP CTS AdaptiveIconDrawableTest port (android.graphics.drawable.AdaptiveIconDrawable).
// Portable cases only: the inflate/draw/invalidate-schedule-unschedule/intrinsic-size cases need
// an XML resource, pixel comparison, a Callback mock, or layer drawables with intrinsic size —
// none reachable headless. Constructor / changing-config / alpha / opacity / stateful /
// constant-state / mutate / foreground-background cases are ported (construction uses ColorDrawable
// layers, like AOSP). mutate is ported default-instance contract style (AOSP uses a resource cache).
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/AdaptiveIconDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <drawable/adaptiveicondrawable.h>
#include <drawable/colordrawable.h>
#include <drawable/statelistdrawable.h>
#include <drawable/drawables.h>

using namespace cdroid;

class CtsAdaptiveIconDrawableTest : public testing::Test {};

TEST_F(CtsAdaptiveIconDrawableTest, testConstructor) {
    AdaptiveIconDrawable d(nullptr, nullptr);
}

TEST_F(CtsAdaptiveIconDrawableTest, testGetChangingConfigurations) {
    AdaptiveIconDrawable d(new ColorDrawable(0xFFFF0000), new ColorDrawable(0xFF0000FF));
    d.setChangingConfigurations(11);
    EXPECT_EQ(11, d.getChangingConfigurations());
    d.setChangingConfigurations(-21);
    EXPECT_EQ(-21, d.getChangingConfigurations());
}

TEST_F(CtsAdaptiveIconDrawableTest, testGetAlpha) {
    AdaptiveIconDrawable d(nullptr, new ColorDrawable(0xFFFF0000));
    d.setAlpha(128);
    EXPECT_EQ(128, d.getAlpha());
}

TEST_F(CtsAdaptiveIconDrawableTest, testGetOpacity) {
    AdaptiveIconDrawable d(new ColorDrawable(0xFFFF0000), new ColorDrawable(0xFF0000FF));
    // AdaptiveIconDrawable always reports TRANSLUCENT regardless of the set opacity.
    d.setOpacity((int)PixelFormat::OPAQUE);
    EXPECT_EQ((int)PixelFormat::TRANSLUCENT, d.getOpacity());
    d.setOpacity((int)PixelFormat::TRANSPARENT);
    EXPECT_EQ((int)PixelFormat::TRANSLUCENT, d.getOpacity());
}

TEST_F(CtsAdaptiveIconDrawableTest, testIsStateful) {
    {
        AdaptiveIconDrawable d(new ColorDrawable(0xFFFF0000), new ColorDrawable(0xFF0000FF));
        EXPECT_FALSE(d.isStateful());   // two non-stateful ColorDrawables
    }
    {
        AdaptiveIconDrawable d(new StateListDrawable(), new ColorDrawable(0xFFFF0000));
        EXPECT_TRUE(d.isStateful());    // a stateful layer (StateListDrawable) makes it stateful
    }
}

TEST_F(CtsAdaptiveIconDrawableTest, testGetConstantState) {
    AdaptiveIconDrawable d(new ColorDrawable(0xFFFF0000), new ColorDrawable(0xFF0000FF));
    auto constantState = d.getConstantState();
    ASSERT_NE(nullptr, constantState);
}

TEST_F(CtsAdaptiveIconDrawableTest, testGetForegroundBackground) {
    AdaptiveIconDrawable d(new ColorDrawable(0xFFFF0000), new ColorDrawable(0xFF0000FF));
    EXPECT_NE(nullptr, dynamic_cast<ColorDrawable*>(d.getForeground()));
    EXPECT_NE(nullptr, dynamic_cast<ColorDrawable*>(d.getBackground()));
}

TEST_F(CtsAdaptiveIconDrawableTest, testMutate) {
    // Copy-on-write contract (AOSP uses two resource-cached instances). A real-layer construction
    // is required: a default-constructed AdaptiveIconDrawable has null child layers, so its
    // canConstantState() is false and getConstantState() returns null.
    AdaptiveIconDrawable d1(new ColorDrawable(0xFFFF0000), new ColorDrawable(0xFF0000FF));
    auto cs = d1.getConstantState();
    ASSERT_NE(nullptr, cs);
    AdaptiveIconDrawable* sib = dynamic_cast<AdaptiveIconDrawable*>(cs->newDrawable());
    ASSERT_NE(nullptr, sib);
    const int initial = d1.getAlpha();
    d1.mutate();
    d1.setAlpha(100);
    EXPECT_NE(initial, d1.getAlpha());
    EXPECT_EQ(initial, sib->getAlpha());   // sibling unaffected — mutate copied the state
    sib->mutate();
    const int d1Alpha = d1.getAlpha();
    sib->setAlpha(50);
    EXPECT_EQ(d1Alpha, d1.getAlpha());
    EXPECT_NE(d1Alpha, sib->getAlpha());
    delete sib;
}
