// AOSP CTS drawable test port (CustomAnimationScaleListDrawableTest.java).
//
// CTS has two cases — testNonZeroDurationScale and testZeroDurationScale — and both inflate the
// real resource R.drawable.custom_animation_scale_list_drawable, then assert that the current child
// is the animatable drawable when ValueAnimator.getDurationScale() != 0 and the static drawable when
// the scale is 0. CDROID's test pak doesn't ship that XML, so the two cases are reproduced here by
// constructing an AnimationScaleListDrawable programmatically and adding one static + one animatable
// child. The selection unit under test (AnimationScaleListState::getCurrentDrawableIndexBasedOnScale)
// is exercised identically.
//
// CDROID notes:
//  - AnimationScaleListDrawable does NOT expose addDrawable publicly; it lives on the protected
//    nested AnimationScaleListState (which sets the static/animatable indices the scale selection
//    reads). The mock re-exposes onStateChange (the scale-based selectDrawable trigger, which
//    inflate() calls at its end) AND reaches the state via the protected mDrawableContainerState
//    (inherited from DrawableContainer) downcast to AnimationScaleListState — both are nameable
//    from a derived class.
//  - "current is Animatable" uses dynamic_cast<Animatable*>(getCurrent()), matching CTS's
//    `instanceof Animatable`. ColorDrawable is NOT Animatable (static child); AnimationDrawable IS
//    (animatable child).
//  - ValueAnimator::setDurationScale/getDurationScale are static and mutate global state; saved and
//    restored around each case exactly like CTS's @Before/@After.
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/CustomAnimationScaleListDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <memory>
#include <drawable/animationscalelistdrawable.h>
#include <drawable/drawables.h>
#include <drawable/animationdrawable.h>
#include <animation/valueanimator.h>
#include <core/app.h>
#include <guienvironment.h>

using namespace cdroid;

namespace {
// Re-exposes the protected onStateChange (the scale-based selection trigger) and addDrawable (via
// the protected nested state) so children can be added and selected programmatically.
class MockAnimationScaleListDrawable : public AnimationScaleListDrawable {
public:
    using AnimationScaleListDrawable::onStateChange;
    void addDrawableForTest(Drawable* d) {
        // mDrawableContainerState is protected-inherited from DrawableContainer; the nested
        // AnimationScaleListState is a protected nested type of AnimationScaleListDrawable, so both
        // are nameable from this derived class.
        auto state = std::dynamic_pointer_cast<AnimationScaleListDrawable::AnimationScaleListState>(
                mDrawableContainerState);
        ASSERT_TRUE(state != nullptr);
        state->addDrawable(d);
    }
};
} // namespace

class CtsAnimationScaleListDrawableTest : public testing::Test {
protected:
    float mOriginalScale = 1.f;
    void SetUp() override {
        // Match CTS @Before: capture the live duration scale so TearDown can restore it.
        mOriginalScale = ValueAnimator::getDurationScale();
    }
    void TearDown() override {
        ValueAnimator::setDurationScale(mOriginalScale);
    }
};

TEST_F(CtsAnimationScaleListDrawableTest, testNonZeroDurationScale) {
    // A non-zero animation scale selects the animatable child.
    ValueAnimator::setDurationScale(2.0f);

    MockAnimationScaleListDrawable dr;
    ASSERT_TRUE(dynamic_cast<DrawableContainer*>(&dr) != nullptr);

    dr.addDrawableForTest(new ColorDrawable(0xFFFF0000));      // NOT Animatable (static child)
    dr.addDrawableForTest(new AnimationDrawable());            // IS  Animatable (animatable child)

    // Trigger selection (the constructor already ran onStateChange before children existed).
    dr.onStateChange(dr.getState());

    EXPECT_TRUE(dynamic_cast<Animatable*>(dr.getCurrent()) != nullptr);
}

TEST_F(CtsAnimationScaleListDrawableTest, testZeroDurationScale) {
    // A zero animation scale selects the static child.
    ValueAnimator::setDurationScale(0.0f);

    MockAnimationScaleListDrawable dr;
    ASSERT_TRUE(dynamic_cast<DrawableContainer*>(&dr) != nullptr);

    dr.addDrawableForTest(new ColorDrawable(0xFFFF0000));
    dr.addDrawableForTest(new AnimationDrawable());

    dr.onStateChange(dr.getState());

    EXPECT_FALSE(dynamic_cast<Animatable*>(dr.getCurrent()) != nullptr);
}

// --- ConstantState contract (default-instance logic; DrawableContainer inherits the correct
// contract, so no source fixes are needed). Mirrors CtsStateListDrawableTest. ---

TEST_F(CtsAnimationScaleListDrawableTest, testGetConstantState) {
    AnimationScaleListDrawable drawable;
    auto constantState = drawable.getConstantState();
    ASSERT_NE(nullptr, constantState);
    Drawable* copy = constantState->newDrawable();
    ASSERT_NE(nullptr, copy);
    EXPECT_NE(&drawable, copy);
    delete copy;
}

TEST_F(CtsAnimationScaleListDrawableTest, testGetChangingConfigurations) {
    AnimationScaleListDrawable drawable;
    auto constantState = drawable.getConstantState();
    ASSERT_NE(nullptr, constantState);
    EXPECT_EQ(0, constantState->getChangingConfigurations());
    EXPECT_EQ(0, drawable.getChangingConfigurations());
    drawable.setChangingConfigurations(0xff);
    EXPECT_EQ(0xff, drawable.getChangingConfigurations());
    EXPECT_EQ(0, constantState->getChangingConfigurations());
    constantState = drawable.getConstantState();
    EXPECT_EQ(0xff, constantState->getChangingConfigurations());
    drawable.setChangingConfigurations(0xff00);
    EXPECT_EQ(0xff, constantState->getChangingConfigurations());
    EXPECT_EQ(0xffff, drawable.getChangingConfigurations());
}

TEST_F(CtsAnimationScaleListDrawableTest, testMutate) {
    // copy-on-write: sibling (from newDrawable) shares state; mutate gives a private copy.
    AnimationScaleListDrawable d1;
    AnimationScaleListDrawable* sibling = dynamic_cast<AnimationScaleListDrawable*>(d1.getConstantState()->newDrawable());
    ASSERT_NE(nullptr, sibling);
    const int initial = d1.getAlpha();
    d1.mutate();
    d1.setAlpha(100);
    EXPECT_NE(initial, d1.getAlpha());
    EXPECT_EQ(initial, sibling->getAlpha());
    sibling->mutate();
    const int d1Alpha = d1.getAlpha();
    sibling->setAlpha(50);
    EXPECT_EQ(d1Alpha, d1.getAlpha());
    EXPECT_NE(d1Alpha, sibling->getAlpha());
    delete sibling;
}
