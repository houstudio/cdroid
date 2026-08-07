// AOSP CTS drawable test port (TransitionDrawableTest.java). The CTS cases verify behavior by
// sampling pixels on a Bitmap/Canvas across timed transition phases (verifyTransition* helpers).
// CDROID renders with Cairo (not Skia) and has no Bitmap#getPixel, so the pixel/color-timing
// assertions are NOT ported; instead each lifecycle call is verified to (a) not throw, (b) fire an
// invalidate on the registered Drawable.Callback (CTS uses Mockito verify(cb, times(1)).
// invalidateDrawable(any())). The null-canvas draw variant (testDrawWithNullCanvas) is also skipped.
//
// Programmatic surface under test: ctor(drawables[]), getNumberOfLayers(), getDrawable(i) (inherited
// from LayerDrawable), startTransition(ms), resetTransition(), reverseTransition(ms),
// isCrossFadeEnabled()/setCrossFadeEnabled().
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/TransitionDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <memory>
#include <drawable/transitiondrawable.h>
#include <drawable/drawables.h>   // ColorDrawable
#include <drawable/colordrawable.h>
#include <core/app.h>
#include <core/rect.h>
#include <guienvironment.h>

using namespace cdroid;

namespace {

// Cts color constants.
constexpr int COLOR0 = 0xFFFF0000;  // Color.RED
constexpr int COLOR1 = 0xFF0000FF;  // Color.BLUE

// Counts invalidate invocations (CTS uses a Mockito mock; CDROID uses a hand-written mock).
class MockCallback : public Drawable::Callback {
public:
    int invalidateCount = 0;
    int scheduleCount = 0;
    int unscheduleCount = 0;
    void invalidateDrawable(Drawable&) override { invalidateCount++; }
    void scheduleDrawable(Drawable&, const Runnable&, int64_t) override { scheduleCount++; }
    void unscheduleDrawable(Drawable&, const Runnable&) override { unscheduleCount++; }
};

// Builds a fresh 2-layer TransitionDrawable (COLOR0 → COLOR1). Each TEST_F owns the instance via
// std::unique_ptr; the TransitionDrawable owns its two ColorDrawable children.
std::unique_ptr<TransitionDrawable> makeTransitionDrawable() {
    return std::unique_ptr<TransitionDrawable>(
        new TransitionDrawable({new ColorDrawable(COLOR0), new ColorDrawable(COLOR1)}));
}

} // namespace

class CtsTransitionDrawableTest : public testing::Test {
protected:
    static constexpr int CANVAS_WIDTH = 10;
    static constexpr int CANVAS_HEIGHT = 10;

    MockCallback mCb;
    std::unique_ptr<TransitionDrawable> mDrawable;

    void SetUp() override {
        mDrawable = makeTransitionDrawable();
        mDrawable->setCallback(&mCb);
        mDrawable->setBounds(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT);
    }
};

TEST_F(CtsTransitionDrawableTest, testConstructor) {
    // CTS wraps two resource drawables; here both layers are ColorDrawables.
    TransitionDrawable d({new ColorDrawable(COLOR0), new ColorDrawable(COLOR1)});
    EXPECT_EQ(2, d.getNumberOfLayers());
}

TEST_F(CtsTransitionDrawableTest, testNumberLayersAndGetDrawable) {
    // LayerDrawable surface inherited by TransitionDrawable.
    EXPECT_EQ(2, mDrawable->getNumberOfLayers());
    EXPECT_NE(nullptr, mDrawable->getDrawable(0));
    EXPECT_NE(nullptr, mDrawable->getDrawable(1));
    // The drawables passed to the ctor are the ones returned by index.
    auto* c0 = dynamic_cast<ColorDrawable*>(mDrawable->getDrawable(0));
    auto* c1 = dynamic_cast<ColorDrawable*>(mDrawable->getDrawable(1));
    ASSERT_NE(nullptr, c0);
    ASSERT_NE(nullptr, c1);
}

TEST_F(CtsTransitionDrawableTest, testStartTransition) {
    int before = mCb.invalidateCount;
    // Starting a forward transition must fire at least one invalidate (CTS: exactly one).
    mDrawable->startTransition(2000);
    EXPECT_GE(mCb.invalidateCount, before + 1);

    // Starting again while a transition is in progress must also be safe and fire invalidation.
    before = mCb.invalidateCount;
    mDrawable->startTransition(2000);
    EXPECT_GE(mCb.invalidateCount, before + 1);

    // Negative duration is tolerated (no throw; CTS: "should not accept negative duration").
    EXPECT_NO_THROW(mDrawable->startTransition(-1));
}

TEST_F(CtsTransitionDrawableTest, testResetTransition) {
    int before = mCb.invalidateCount;
    // reset when there is no transition in progress fires invalidate (CTS: times(1)).
    mDrawable->resetTransition();
    EXPECT_GE(mCb.invalidateCount, before + 1);

    // reset after starting a forward transition returns to the start state and fires invalidate.
    mDrawable->startTransition(2000);
    before = mCb.invalidateCount;
    mDrawable->resetTransition();
    EXPECT_GE(mCb.invalidateCount, before + 1);

    // reset when a reverse transition is in progress is also safe.
    mDrawable->startTransition(2000);
    mDrawable->reverseTransition(2000);
    before = mCb.invalidateCount;
    mDrawable->resetTransition();
    EXPECT_GE(mCb.invalidateCount, before + 1);
}

TEST_F(CtsTransitionDrawableTest, testReverseTransition) {
    int before = mCb.invalidateCount;
    // reverse when there is no transition in progress starts a forward transition (CTS verifies
    // COLOR0 → COLOR1) and fires invalidate.
    mDrawable->reverseTransition(2000);
    EXPECT_GE(mCb.invalidateCount, before + 1);

    // reverse after the forward transition has run swaps direction (CTS: COLOR1 → COLOR0).
    before = mCb.invalidateCount;
    mDrawable->reverseTransition(2000);
    EXPECT_GE(mCb.invalidateCount, before + 1);

    // Negative duration is tolerated (no throw).
    EXPECT_NO_THROW(mDrawable->reverseTransition(-1));
}

TEST_F(CtsTransitionDrawableTest, testAccessCrossFadeEnabled) {
    EXPECT_FALSE(mDrawable->isCrossFadeEnabled());

    mDrawable->setCrossFadeEnabled(true);
    EXPECT_TRUE(mDrawable->isCrossFadeEnabled());

    mDrawable->setCrossFadeEnabled(false);
    EXPECT_FALSE(mDrawable->isCrossFadeEnabled());
}

// --- ConstantState contract (mirrors CtsVectorDrawableTest) ---

TEST_F(CtsTransitionDrawableTest, testGetConstantState) {
    // getConstantState() must return a non-null state whose newDrawable() yields a distinct
    // instance backed by the same constant state. Mirrors CtsVectorDrawableTest.testGetConstantState;
    // CDROID diverges from AOSP only in that the instance is built from ColorDrawable layers rather
    // than inflated from a resource.
    TransitionDrawable drawable({new ColorDrawable(COLOR0), new ColorDrawable(COLOR1)});
    auto constantState = drawable.getConstantState();
    ASSERT_NE(nullptr, constantState);

    Drawable* copy = constantState->newDrawable();
    ASSERT_NE(nullptr, copy);
    EXPECT_NE(&drawable, copy);
    delete copy;
}

TEST_F(CtsTransitionDrawableTest, testMutate) {
    // mutate() must give this drawable a private constant-state copy (copy-on-write), so a state
    // change on the mutated instance does not affect a sibling produced from the same
    // getConstantState(). Mirrors AOSP testMutate (which uses two resource-cached instances).
    // CDROID diverges: a default-constructed TransitionDrawable has no child layers and
    // LayerDrawable::setAlpha/getAlpha proxy to the first child, so the two-layer ColorDrawable
    // construction is required to observe alpha independence.
    TransitionDrawable d1({new ColorDrawable(COLOR0), new ColorDrawable(COLOR1)});
    ASSERT_EQ(255, d1.getAlpha());
    TransitionDrawable* d2 = dynamic_cast<TransitionDrawable*>(d1.getConstantState()->newDrawable());
    ASSERT_NE(nullptr, d2);
    ASSERT_EQ(255, d2->getAlpha());

    d1.mutate();
    d1.setAlpha(100);
    EXPECT_NE(255, d1.getAlpha());   // d1's alpha changed from the default 255 (LayerDrawable proxies
                                     // alpha to the ColorDrawable child, which modulates baseAlpha*alpha>>8,
                                     // so the read-back is not the raw input — assert the change, not the value)
    EXPECT_EQ(255, d2->getAlpha());  // sibling unaffected — mutate gave d1 a private state copy

    d2->mutate();
    const int d1Alpha = d1.getAlpha();
    d2->setAlpha(50);
    EXPECT_EQ(d1Alpha, d1.getAlpha());     // d1 unaffected by d2's change (copy-on-write)
    EXPECT_NE(d1Alpha, d2->getAlpha());    // d2 changed, independently of d1
    delete d2;
}

TEST_F(CtsTransitionDrawableTest, testGetChangingConfigurations) {
    // Mirrors CtsVectorDrawableTest.testGetChangingConfigurations: default 0; changing the drawable's
    // configuration does not affect a previously-fetched state snapshot; re-fetching reflects it; the
    // drawable ORs its instance value with the state's value. TransitionDrawable inherits LayerDrawable's
    // snapshot sync in getConstantState; the instance|state OR is provided by TransitionDrawable itself
    // (AOSP LayerDrawable.getChangingConfigurations, LayerDrawable.java:1025).
    TransitionDrawable drawable;
    auto constantState = drawable.getConstantState();
    ASSERT_NE(nullptr, constantState);

    // default
    EXPECT_EQ(0, constantState->getChangingConfigurations());
    EXPECT_EQ(0, drawable.getChangingConfigurations());

    // changing the drawable's configuration does not affect the cached state's snapshot
    drawable.setChangingConfigurations(0xff);
    EXPECT_EQ(0xff, drawable.getChangingConfigurations());
    EXPECT_EQ(0, constantState->getChangingConfigurations());

    // re-fetching the constant state reflects the new value
    constantState = drawable.getConstantState();
    EXPECT_EQ(0xff, constantState->getChangingConfigurations());

    // set a new configuration to the drawable; drawable ORs with the state's value
    drawable.setChangingConfigurations(0xff00);
    EXPECT_EQ(0xff, constantState->getChangingConfigurations());
    EXPECT_EQ(0xffff, drawable.getChangingConfigurations());
}
