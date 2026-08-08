// AOSP CTS drawable test port (AnimatedVectorDrawableTest.java). Pure-logic cases only.
// NOT ported:
//   testInflate                         — golden-image pixel comparison (sun/earth colors); Cairo
//                                          renders through a different pipeline than Skia.
//   testGetOpticalInsets                — needs a specific complex animated-vector resource
//                                          (animation_vector_drawable_grouping_1) carrying 10/20/30/40
//                                          optical insets; the opticalInsets-from-XML path is already
//                                          covered by CtsVectorDrawableTest.testOpticalInsets.
//   testMutate                          — relies on Resources.getDrawable returning independent AVD
//                                          deep copies + the grouping_1 asset; mutate() copy-on-write
//                                          logic is already covered by CtsVectorDrawableTest.testMutate.
//   testReset / testStop / testAddCallbackBeforeStart / testAddCallbackAfterTrigger /
//   testAddCallbackAfterStart / testRemoveCallback / testClearCallback
//                                       — drive the AVD on the UI thread inside a DrawableStubActivity
//                                          with an ImageView, await start/end via Animatable2 callbacks,
//                                          and are @FlakyTest against render-thread timing. None of that
//                                          (Activity/ImageView/AVD-extraction/render-thread) is reachable
//                                          in the headless test environment.
// The changing-configurations / constant-state / color-filter / opacity cases are ported verbatim
// (default-constructed AVD, mirroring CtsVectorDrawableTest).
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/AnimatedVectorDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <drawable/animatedvectordrawable.h>
#include <drawable/colorfilters.h>
#include <core/app.h>
#include <core/porterduff.h>

using namespace cdroid;

class CtsAnimatedVectorDrawableTest : public testing::Test {};

TEST_F(CtsAnimatedVectorDrawableTest, testGetChangingConfigurations) {
    AnimatedVectorDrawable avd;
    auto constantState = avd.getConstantState();

    // default
    ASSERT_NE(nullptr, constantState);
    EXPECT_EQ(0, constantState->getChangingConfigurations());
    EXPECT_EQ(0, avd.getChangingConfigurations());

    // changing the drawable's configuration does not affect the cached state's snapshot
    avd.setChangingConfigurations(0xff);
    EXPECT_EQ(0xff, avd.getChangingConfigurations());
    EXPECT_EQ(0, constantState->getChangingConfigurations());

    // re-fetching the constant state reflects the new value
    constantState = avd.getConstantState();
    EXPECT_EQ(0xff, constantState->getChangingConfigurations());

    // set a new configuration to the drawable; drawable ORs with the state's value
    avd.setChangingConfigurations(0xff00);
    EXPECT_EQ(0xff, constantState->getChangingConfigurations());
    EXPECT_EQ(0xffff, avd.getChangingConfigurations());
}

TEST_F(CtsAnimatedVectorDrawableTest, testGetConstantState) {
    AnimatedVectorDrawable avd;
    auto constantState = avd.getConstantState();
    ASSERT_NE(nullptr, constantState);
    EXPECT_EQ(0, constantState->getChangingConfigurations());

    avd.setChangingConfigurations(1);
    constantState = avd.getConstantState();
    ASSERT_NE(nullptr, constantState);
    EXPECT_EQ(1, constantState->getChangingConfigurations());
}

TEST_F(CtsAnimatedVectorDrawableTest, testColorFilter) {
    cdroid::RefPtr<ColorFilter> filter =
        std::make_shared<PorterDuffColorFilter>(0xFFFF0000, PorterDuff::SRC_IN);
    AnimatedVectorDrawable avd;
    avd.setColorFilter(filter);
    EXPECT_EQ(filter.get(), avd.getColorFilter().get());
}

TEST_F(CtsAnimatedVectorDrawableTest, testGetOpacity) {
    AnimatedVectorDrawable avd;
    EXPECT_EQ(255, avd.getAlpha());  // default is translucent
    EXPECT_EQ((int)PixelFormat::TRANSLUCENT, avd.getOpacity());
    avd.setAlpha(0);
    EXPECT_EQ(0, avd.getAlpha());
    EXPECT_EQ((int)PixelFormat::TRANSLUCENT, avd.getOpacity());  // still translucent
}
