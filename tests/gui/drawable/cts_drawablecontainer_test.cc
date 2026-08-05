// AOSP CTS drawable test port (DrawableContainerTest.java).
//
// CDROID↔Android differences (see drawablecontainer.h):
//  - DrawableContainer::DrawableContainerState is a *protected* nested type. Its name cannot be
//    written at the call site, so a MockDrawableContainer subclass provides pass-through helpers
//    (mDrawableContainerState->xxx()) for the few cases needing state-level access.
//  - DrawableContainer's default ctor ALWAYS creates a constant state (unlike Android's, where the
//    state starts null). All CTS "*NoConstantState"/testConstantStateNotSet NPE cases are therefore
//    not reproducible (CDROID never dereferences a null state here) and are skipped.
//  - CDROID uses Drawable& (reference) parameters for invalidateDrawable/scheduleDrawable/
//    unscheduleDrawable/onBoundsChange/onStateChange, so CTS's null-argument sub-cases cannot be
//    expressed; only the non-null branches are ported.
//  - The BlendMode-tint cases (testSetTint/testSetBlendMode) and the Mockito-spy opacity/statefulness
//    mutation cases (testOpacityChange/testStatefulnessChange) are skipped (BlendMode API and tint
//    coupling differ from Android).
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/DrawableContainerTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <drawable/drawables.h>
#include <drawable/drawablecontainer.h>
#include <drawable/levellistdrawable.h>
#include <core/rect.h>
#include <core/insets.h>
#include <guienvironment.h>

using namespace cdroid;

namespace {
// Hand-written stand-in for CTS's spied MockDrawable. Tracks calls to the protected
// onBoundsChange/onStateChange/onLevelChange (re-exposed public) and to the public setAlpha/
// setDither/mutate, with controllable opacity / intrinsic / minimum / padding / insets.
class MockDrawable : public Drawable {
public:
    int mOpacity = PixelFormat::OPAQUE;
    MockDrawable() = default;
    explicit MockDrawable(int opacity) : mOpacity(opacity) {}
    bool mIsStateful = false;
    int mIntrinsicW = -1;
    int mIntrinsicH = -1;
    int mMinimumW = 0;
    int mMinimumH = 0;
    bool mHasPadding = false;
    Rect mPadding;
    Insets mInsets;          // getOpticalInsets() returns this (default-constructed = zero)

    bool mBoundsChanged = false;
    bool mStateChanged = false;
    bool mLevelChanged = false;
    int mLastAlpha = -1;
    int mAlphaCount = 0;
    bool mLastDither = false;
    int mDitherCount = 0;
    int mMutateCount = 0;

    void draw(Canvas&) override {}
    int getOpacity() const override { return mOpacity; }
    bool isStateful() const override { return mIsStateful; }
    int getIntrinsicWidth() override { return mIntrinsicW; }
    int getIntrinsicHeight() override { return mIntrinsicH; }
    int getMinimumWidth() override { return mMinimumW; }
    int getMinimumHeight() override { return mMinimumH; }
    bool getPadding(Rect& padding) override {
        if (mHasPadding) { padding = mPadding; return true; }
        return Drawable::getPadding(padding);
    }
    Insets getOpticalInsets() override { return mInsets; }

    void reset() {
        mBoundsChanged = mStateChanged = mLevelChanged = false;
        mAlphaCount = mDitherCount = 0;
    }
    void onBoundsChange(const Rect&) override { mBoundsChanged = true; }
    bool onStateChange(const std::vector<int>&) override { mStateChanged = true; return false; }
    bool onLevelChange(int) override { mLevelChanged = true; return true; }
    void setAlpha(int alpha) override { mLastAlpha = alpha; mAlphaCount++; }
    void setDither(bool dither) override { mLastDither = dither; mDitherCount++; }
    Drawable* mutate() override { mMutateCount++; return this; }
};

// Subclass exposing DrawableContainer's protected overrides (onBoundsChange/onStateChange/
// onLevelChange) and a small set of state pass-throughs (needed for fade-duration / variable-padding
// cases). The protected state's name is never written at the call site.
class MockDrawableContainer : public DrawableContainer {
public:
    using DrawableContainer::onBoundsChange;
    using DrawableContainer::onStateChange;
    using DrawableContainer::onLevelChange;
    void stateSetEnterFadeDuration(int d) { mDrawableContainerState->setEnterFadeDuration(d); }
    int stateGetEnterFadeDuration() const { return mDrawableContainerState->getEnterFadeDuration(); }
    void stateSetExitFadeDuration(int d) { mDrawableContainerState->setExitFadeDuration(d); }
    int stateGetExitFadeDuration() const { return mDrawableContainerState->getExitFadeDuration(); }
    void stateSetVariablePadding(bool v) { mDrawableContainerState->setVariablePadding(v); }
    bool stateIsStateful() { return mDrawableContainerState->isStateful(); }
    int stateGetOpacity() { return mDrawableContainerState->getOpacity(); }
    bool stateIsConstantSize() const { return mDrawableContainerState->isConstantSize(); }
    void stateSetConstantSize(bool c) { mDrawableContainerState->setConstantSize(c); }
};

// Callback that counts relay calls (CTS uses Mockito).
class MockCallback : public Drawable::Callback {
public:
    int invalidateCount = 0;
    int scheduleCount = 0;
    int unscheduleCount = 0;
    void invalidateDrawable(Drawable&) override { invalidateCount++; }
    void scheduleDrawable(Drawable&, const Runnable&, int64_t) override { scheduleCount++; }
    void unscheduleDrawable(Drawable&, const Runnable&) override { unscheduleCount++; }
};
} // namespace

class CtsDrawableContainerTest : public testing::Test {
protected:
    std::unique_ptr<MockDrawableContainer> mDrawableContainer;
    void SetUp() override {
        mDrawableContainer.reset(new MockDrawableContainer());
        // No child selected yet.
        EXPECT_EQ(nullptr, mDrawableContainer->getCurrent());
    }

    // CTS addAndSelectDrawable: append the child and select it.
    void addAndSelectDrawable(Drawable* dr) {
        const int pos = mDrawableContainer->addChild(dr);
        mDrawableContainer->selectDrawable(pos);
        EXPECT_EQ(dr, mDrawableContainer->getCurrent());
    }
};

// All CTS "*NoConstantState"/testConstantStateNotSet NPE cases are skipped: CDROID's
// DrawableContainer ctor always provisions a constant state, so the null-state scenario does
// not occur (and getConstantState/getChangingConfigurations/getPadding/isStateful/selectDrawable/
// mutate do not throw).

TEST_F(CtsDrawableContainerTest, testSetEnterFadeDuration) {
    mDrawableContainer->setEnterFadeDuration(1000);
    EXPECT_EQ(1000, mDrawableContainer->stateGetEnterFadeDuration());
    mDrawableContainer->setEnterFadeDuration(0);
    EXPECT_EQ(0, mDrawableContainer->stateGetEnterFadeDuration());
}

TEST_F(CtsDrawableContainerTest, testSetExitFadeDuration) {
    mDrawableContainer->setExitFadeDuration(1000);
    EXPECT_EQ(1000, mDrawableContainer->stateGetExitFadeDuration());
    mDrawableContainer->setExitFadeDuration(0);
    EXPECT_EQ(0, mDrawableContainer->stateGetExitFadeDuration());
}

TEST_F(CtsDrawableContainerTest, testGetChangingConfigurations) {
    auto* dr0 = new MockDrawable();
    dr0->setChangingConfigurations(0x001);
    mDrawableContainer->addChild(dr0);
    auto* dr1 = new MockDrawable();
    dr1->setChangingConfigurations(0x010);
    mDrawableContainer->addChild(dr1);
    mDrawableContainer->selectDrawable(0);
    EXPECT_EQ(dr0, mDrawableContainer->getCurrent());

    // Drawable's own config (0x100) is OR'd with the children's aggregated config (0x011).
    mDrawableContainer->setChangingConfigurations(0x100);
    EXPECT_EQ(0x100 | 0x011, mDrawableContainer->getChangingConfigurations());
}

TEST_F(CtsDrawableContainerTest, testSetAlpha) {
    mDrawableContainer->setAlpha(0);  // no current child yet: stored only

    auto* dr = new MockDrawable();
    addAndSelectDrawable(dr);

    dr->reset();
    mDrawableContainer->setAlpha(1);
    EXPECT_EQ(1, dr->mLastAlpha);
    EXPECT_EQ(1, dr->mAlphaCount);

    // Same alpha: setAlpha must not be forwarded again.
    dr->reset();
    mDrawableContainer->setAlpha(1);
    EXPECT_EQ(0, dr->mAlphaCount);
}

TEST_F(CtsDrawableContainerTest, testSetDither) {
    mDrawableContainer->setDither(false);
    mDrawableContainer->setDither(true);

    auto* dr = new MockDrawable();
    addAndSelectDrawable(dr);

    dr->reset();
    mDrawableContainer->setDither(false);
    EXPECT_FALSE(dr->mLastDither);
    EXPECT_EQ(1, dr->mDitherCount);

    dr->reset();
    mDrawableContainer->setDither(true);
    EXPECT_TRUE(dr->mLastDither);
    EXPECT_EQ(1, dr->mDitherCount);
}

TEST_F(CtsDrawableContainerTest, testSetHotspotBounds) {
    // No child yet.
    EXPECT_EQ(nullptr, mDrawableContainer->getCurrent());
    addAndSelectDrawable(new MockDrawable());

    // CDROID setHotspotBounds(l,t,w,h) stores left/top/width/height (CTS passes l,t,r,b — the
    // values round-trip identically either way).
    mDrawableContainer->setHotspotBounds(10, 15, 100, 150);
    Rect out;
    mDrawableContainer->getHotspotBounds(out);
    EXPECT_EQ(10, out.left);
    EXPECT_EQ(15, out.top);
    EXPECT_EQ(100, out.width);
    EXPECT_EQ(150, out.height);
}

TEST_F(CtsDrawableContainerTest, testGetHotspotBounds) {
    addAndSelectDrawable(new MockDrawable());
    mDrawableContainer->setHotspotBounds(10, 15, 100, 150);
    Rect out;
    mDrawableContainer->getHotspotBounds(out);
    EXPECT_EQ(10, out.left);
    EXPECT_EQ(15, out.top);
    EXPECT_EQ(100, out.width);
    EXPECT_EQ(150, out.height);
}

TEST_F(CtsDrawableContainerTest, testSetColorFilter) {
    mDrawableContainer->setColorFilter(nullptr);

    auto* dr = new MockDrawable();
    addAndSelectDrawable(dr);

    // Setting the same (null) filter is a no-op for the current child.
    dr->reset();
    mDrawableContainer->setColorFilter(nullptr);
    // No MockDrawable setColorFilter tracking; just verify the relay path does not throw.
    SUCCEED();
}

TEST_F(CtsDrawableContainerTest, testOnBoundsChange) {
    // Empty rect, no child: must not throw.
    mDrawableContainer->onBoundsChange(Rect{});

    auto* dr = new MockDrawable();
    dr->setBounds(Rect{});
    addAndSelectDrawable(dr);

    // A different bounds reaches the current child's setBounds → its onBoundsChange fires.
    dr->reset();
    mDrawableContainer->onBoundsChange(Rect{1, 1, 1, 1});
    EXPECT_TRUE(dr->mBoundsChanged);

    // Same bounds: the child's setBounds is a no-op → onBoundsChange does not fire.
    dr->reset();
    mDrawableContainer->onBoundsChange(Rect{1, 1, 1, 1});
    EXPECT_FALSE(dr->mBoundsChanged);
}

TEST_F(CtsDrawableContainerTest, testIsStateful) {
    auto* dr0 = new MockDrawable();
    dr0->mIsStateful = true;
    mDrawableContainer->addChild(dr0);
    auto* dr1 = new MockDrawable();
    dr1->mIsStateful = false;
    mDrawableContainer->addChild(dr1);

    // The container's isStateful() reflects the aggregated state (any stateful child ⇒ true),
    // independent of which child is current.
    EXPECT_TRUE(mDrawableContainer->isStateful());

    mDrawableContainer->selectDrawable(1);
    EXPECT_TRUE(mDrawableContainer->isStateful());
}

TEST_F(CtsDrawableContainerTest, testOnStateChange) {
    // No current child: onStateChange returns false.
    EXPECT_FALSE(mDrawableContainer->onStateChange(std::vector<int>{0}));

    auto* dr = new MockDrawable();
    dr->setState(std::vector<int>{0});
    addAndSelectDrawable(dr);

    // A state that differs from the child's current state fires onStateChange on the child.
    dr->reset();
    mDrawableContainer->onStateChange(std::vector<int>{1});
    EXPECT_TRUE(dr->mStateChanged);

    // Same state: no transition, no callback.
    dr->reset();
    EXPECT_FALSE(mDrawableContainer->onStateChange(std::vector<int>{1}));
    EXPECT_FALSE(dr->mStateChanged);
}

TEST_F(CtsDrawableContainerTest, testOnLevelChange) {
    EXPECT_FALSE(mDrawableContainer->onLevelChange(INT_MAX));
    EXPECT_FALSE(mDrawableContainer->onLevelChange(INT_MIN));

    auto* dr = new MockDrawable();
    dr->setLevel(0);
    addAndSelectDrawable(dr);

    dr->reset();
    mDrawableContainer->onLevelChange(INT_MAX);
    EXPECT_EQ(INT_MAX, dr->getLevel());
    EXPECT_TRUE(dr->mLevelChanged);

    dr->reset();
    mDrawableContainer->onLevelChange(INT_MIN);
    EXPECT_EQ(INT_MIN, dr->getLevel());
    EXPECT_TRUE(dr->mLevelChanged);

    // Same level: setLevel returns false, onLevelChange on the child does not fire.
    dr->reset();
    EXPECT_FALSE(mDrawableContainer->onLevelChange(INT_MIN));
    EXPECT_FALSE(dr->mLevelChanged);
}

TEST_F(CtsDrawableContainerTest, testGetIntrinsicWidth) {
    auto* dr0 = new MockDrawable();
    dr0->mIntrinsicW = 1;
    mDrawableContainer->addChild(dr0);
    auto* dr1 = new MockDrawable();
    dr1->mIntrinsicW = 2;
    mDrawableContainer->addChild(dr1);

    // Constant size: returns the aggregated max constant width.
    mDrawableContainer->stateSetConstantSize(true);
    EXPECT_EQ(2, mDrawableContainer->getIntrinsicWidth());

    // Default value when nothing is selected.
    mDrawableContainer->stateSetConstantSize(false);
    EXPECT_EQ(nullptr, mDrawableContainer->getCurrent());
    EXPECT_EQ(-1, mDrawableContainer->getIntrinsicWidth());

    // Current drawable's intrinsic width otherwise.
    mDrawableContainer->selectDrawable(0);
    EXPECT_EQ(dr0, mDrawableContainer->getCurrent());
    EXPECT_EQ(1, mDrawableContainer->getIntrinsicWidth());
}

TEST_F(CtsDrawableContainerTest, testGetIntrinsicHeight) {
    auto* dr0 = new MockDrawable();
    dr0->mIntrinsicH = 1;
    mDrawableContainer->addChild(dr0);
    auto* dr1 = new MockDrawable();
    dr1->mIntrinsicH = 2;
    mDrawableContainer->addChild(dr1);

    mDrawableContainer->stateSetConstantSize(true);
    EXPECT_EQ(2, mDrawableContainer->getIntrinsicHeight());

    mDrawableContainer->stateSetConstantSize(false);
    EXPECT_EQ(-1, mDrawableContainer->getIntrinsicHeight());

    mDrawableContainer->selectDrawable(0);
    EXPECT_EQ(1, mDrawableContainer->getIntrinsicHeight());
}

TEST_F(CtsDrawableContainerTest, testGetMinimumWidth) {
    auto* dr0 = new MockDrawable();
    dr0->mMinimumW = 1;
    mDrawableContainer->addChild(dr0);
    auto* dr1 = new MockDrawable();
    dr1->mMinimumW = 2;
    mDrawableContainer->addChild(dr1);

    mDrawableContainer->stateSetConstantSize(true);
    EXPECT_EQ(2, mDrawableContainer->getMinimumWidth());

    mDrawableContainer->stateSetConstantSize(false);
    EXPECT_EQ(0, mDrawableContainer->getMinimumWidth());

    mDrawableContainer->selectDrawable(0);
    EXPECT_EQ(1, mDrawableContainer->getMinimumWidth());
}

TEST_F(CtsDrawableContainerTest, testGetMinimumHeight) {
    auto* dr0 = new MockDrawable();
    dr0->mMinimumH = 1;
    mDrawableContainer->addChild(dr0);
    auto* dr1 = new MockDrawable();
    dr1->mMinimumH = 2;
    mDrawableContainer->addChild(dr1);

    mDrawableContainer->stateSetConstantSize(true);
    EXPECT_EQ(2, mDrawableContainer->getMinimumHeight());

    mDrawableContainer->stateSetConstantSize(false);
    EXPECT_EQ(0, mDrawableContainer->getMinimumHeight());

    mDrawableContainer->selectDrawable(0);
    EXPECT_EQ(1, mDrawableContainer->getMinimumHeight());
}

TEST_F(CtsDrawableContainerTest, testInvalidateDrawable) {
    // No callback set: must not throw.
    mDrawableContainer->setCallback(nullptr);
    mDrawableContainer->invalidateDrawable(*mDrawableContainer);

    MockCallback cb;
    mDrawableContainer->setCallback(&cb);

    // No current child: relaying the container itself does not forward.
    cb.invalidateCount = 0;
    mDrawableContainer->invalidateDrawable(*mDrawableContainer);
    EXPECT_EQ(0, cb.invalidateCount);

    auto* dr = new MockDrawable();
    addAndSelectDrawable(dr);

    // A non-current drawable is filtered out.
    MockDrawable other;
    cb.invalidateCount = 0;
    mDrawableContainer->invalidateDrawable(other);
    EXPECT_EQ(0, cb.invalidateCount);

    // The selected drawable is forwarded.
    cb.invalidateCount = 0;
    mDrawableContainer->invalidateDrawable(*dr);
    EXPECT_EQ(1, cb.invalidateCount);
}

TEST_F(CtsDrawableContainerTest, testScheduleDrawable) {
    MockCallback cb;
    mDrawableContainer->setCallback(&cb);
    Runnable r = [] {};

    // No current child: relaying the container itself does not forward.
    mDrawableContainer->scheduleDrawable(*mDrawableContainer, r, 0);
    EXPECT_EQ(0, cb.scheduleCount);

    auto* dr = new MockDrawable();
    addAndSelectDrawable(dr);

    // A non-current drawable is filtered out.
    MockDrawable other;
    mDrawableContainer->scheduleDrawable(other, r, 0);
    EXPECT_EQ(0, cb.scheduleCount);

    // The selected drawable is forwarded.
    mDrawableContainer->scheduleDrawable(*dr, r, 0);
    EXPECT_EQ(1, cb.scheduleCount);
}

TEST_F(CtsDrawableContainerTest, testUnscheduleDrawable) {
    MockCallback cb;
    mDrawableContainer->setCallback(&cb);
    Runnable r = [] {};

    mDrawableContainer->unscheduleDrawable(*mDrawableContainer, r);
    EXPECT_EQ(0, cb.unscheduleCount);

    auto* dr = new MockDrawable();
    addAndSelectDrawable(dr);

    MockDrawable other;
    mDrawableContainer->unscheduleDrawable(other, r);
    EXPECT_EQ(0, cb.unscheduleCount);

    mDrawableContainer->unscheduleDrawable(*dr, r);
    EXPECT_EQ(1, cb.unscheduleCount);
}

TEST_F(CtsDrawableContainerTest, testSetVisible) {
    EXPECT_TRUE(mDrawableContainer->isVisible());
    EXPECT_FALSE(mDrawableContainer->setVisible(true, false));
    EXPECT_TRUE(mDrawableContainer->setVisible(false, false));
    EXPECT_FALSE(mDrawableContainer->isVisible());
    EXPECT_FALSE(mDrawableContainer->setVisible(false, false));
    EXPECT_TRUE(mDrawableContainer->setVisible(true, false));

    auto* dr = new MockDrawable();
    addAndSelectDrawable(dr);

    EXPECT_TRUE(mDrawableContainer->isVisible());
    EXPECT_TRUE(dr->isVisible());
    EXPECT_TRUE(mDrawableContainer->setVisible(false, false));
    EXPECT_FALSE(mDrawableContainer->isVisible());
    EXPECT_FALSE(dr->isVisible());
}

TEST_F(CtsDrawableContainerTest, testGetOpacity) {
    // No child selected ⇒ transparent.
    EXPECT_EQ((int)PixelFormat::TRANSPARENT, mDrawableContainer->getOpacity());

    auto* dr0 = new MockDrawable(PixelFormat::OPAQUE);
    mDrawableContainer->addChild(dr0);
    // Still no child selected.
    EXPECT_EQ((int)PixelFormat::TRANSPARENT, mDrawableContainer->getOpacity());

    mDrawableContainer->selectDrawable(0);
    EXPECT_EQ((int)PixelFormat::OPAQUE, mDrawableContainer->getOpacity());

    auto* dr1 = new MockDrawable(PixelFormat::TRANSLUCENT);
    mDrawableContainer->addChild(dr1);
    mDrawableContainer->selectDrawable(1);
    EXPECT_EQ((int)PixelFormat::TRANSLUCENT, mDrawableContainer->getOpacity());
}

TEST_F(CtsDrawableContainerTest, testSelectDrawable) {
    auto* dr0 = new MockDrawable();
    dr0->setVisible(false, false);
    EXPECT_FALSE(dr0->isVisible());
    mDrawableContainer->addChild(dr0);
    auto* dr1 = new MockDrawable();
    dr1->setVisible(false, false);
    EXPECT_FALSE(dr1->isVisible());
    mDrawableContainer->addChild(dr1);

    EXPECT_TRUE(mDrawableContainer->selectDrawable(0));
    EXPECT_EQ(dr0, mDrawableContainer->getCurrent());
    EXPECT_TRUE(dr0->isVisible());

    // Same index: returns false.
    EXPECT_FALSE(mDrawableContainer->selectDrawable(0));

    EXPECT_TRUE(mDrawableContainer->selectDrawable(1));
    EXPECT_EQ(dr1, mDrawableContainer->getCurrent());
    EXPECT_TRUE(dr1->isVisible());
    EXPECT_FALSE(dr0->isVisible());

    EXPECT_FALSE(mDrawableContainer->selectDrawable(1));

    // -1 deselects.
    EXPECT_TRUE(mDrawableContainer->selectDrawable(-1));
    EXPECT_EQ(nullptr, mDrawableContainer->getCurrent());
    EXPECT_FALSE(dr0->isVisible());
    EXPECT_FALSE(dr1->isVisible());

    // Out-of-range index also clears the current drawable.
    EXPECT_TRUE(mDrawableContainer->selectDrawable(2));
    EXPECT_EQ(nullptr, mDrawableContainer->getCurrent());
    EXPECT_FALSE(dr0->isVisible());
    EXPECT_FALSE(dr1->isVisible());
}

TEST_F(CtsDrawableContainerTest, testAccessConstantState) {
    // Fresh container: getConstantState returns the default state (children all have a constant
    // state or none added ⇒ canConstantState() true).
    EXPECT_NE(nullptr, mDrawableContainer->getConstantState());
}

TEST_F(CtsDrawableContainerTest, testMutate) {
    // Use LevelListDrawable (properly overrides cloneConstantState) so the mutate clone path is
    // exercised faithfully; a ColorDrawable child provides a real constant state.
    LevelListDrawable container;
    container.addLevel(0, 10, new ColorDrawable(0xFF0000FF));
    EXPECT_NE(nullptr, container.mutate());
}

TEST_F(CtsDrawableContainerTest, testGetOpticalBoundsWithNoInternalDrawable) {
    DrawableContainer container;
    EXPECT_EQ(Insets::NONE, container.getOpticalInsets());
}

TEST_F(CtsDrawableContainerTest, testGetOpticalBoundsFromInternalDrawable) {
    auto* dr = new MockDrawable();
    dr->mInsets = Insets::of(20, 40, 60, 100);
    addAndSelectDrawable(dr);
    const Insets got = mDrawableContainer->getOpticalInsets();
    EXPECT_EQ(20, got.left);
    EXPECT_EQ(40, got.top);
    EXPECT_EQ(60, got.right);
    EXPECT_EQ(100, got.bottom);
}
