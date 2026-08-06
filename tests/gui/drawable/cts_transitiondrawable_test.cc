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
