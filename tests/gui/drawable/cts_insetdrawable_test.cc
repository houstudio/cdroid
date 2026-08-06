// AOSP CTS drawable test port (InsetDrawableTest.java). Pure-logic / int-inset cases only.
//
// Skipped (per the port rules):
//  - Float-inset cases (testConstructor's float variants, testGetPadding_fraction,
//    testOnBoundsChange_fraction, testIsBoundsAndIntrinsicSizeInverse): CDROID's InsetDrawable
//    exposes ONLY int-inset ctors InsetDrawable(Drawable*, int) and (Drawable*, int,int,int,int);
//    there is no float-inset constructor.
//  - Resource cases (testInflate, testGetIntrinsicWidth/Height, testMutate, testPreloadDensity*):
//    depend on Android R.drawable.* (scenery/inset_color/inset_mutate/inset_density) and density
//    scaling CDROID does not reproduce.
//  - Pixel/render (testDraw, testDrawNull) and null-argument variants (testInflateNull,
//    testGetPaddingNull, testOnBoundsChangeNull).
//
// CDROID Rect = {left, top, width, height} (NOT Android's l/t/r/b). CDROID's InsetDrawable stores
// the right inset directly into the padding Rect's `width` field and the bottom inset into its
// `height` field (see InsetDrawable::getPadding/getInsets), and onBoundsChange subtracts the
// right/bottom insets directly into the child bounds' width/height. So CTS assertions on Rect
// `.right`/`.bottom` translate to CDROID Rect `.width`/`.height` here (noted inline). CDROID
// `mPassDrawable` (CTS R.drawable.pass) is represented by a ColorDrawable.
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/InsetDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <memory>
#include <drawable/drawable.h>
#include <drawable/drawables.h>
#include <drawable/insetdrawable.h>
#include <drawable/colorfilters.h>
#include <drawable/stateset.h>
#include <core/porterduff.h>
#include <core/rect.h>
#include <core/insets.h>
#include <guienvironment.h>

using namespace cdroid;

namespace {
// CTS MockInsetDrawable re-exposes the protected onStateChange/onBoundsChange so the test can
// invoke them directly. CDROID's InsetDrawable inherits both (protected overrides) from
// DrawableWrapper; `using` lifts them into public scope.
class MockInsetDrawable : public InsetDrawable {
public:
    MockInsetDrawable(Drawable* drawable, int inset) : InsetDrawable(drawable, inset) {}
    using InsetDrawable::onStateChange;
    using InsetDrawable::onBoundsChange;
};
} // namespace

// CTS @Before builds mInsetDrawable = InsetDrawable(R.drawable.pass, 0). CDROID uses an opaque
// ColorDrawable as the wrap child (the specific child is irrelevant to the cases ported; opacity
// cases need an opaque child). The InsetDrawable OWNS the child (DrawableWrapper deletes it).
class CtsInsetDrawableTest : public testing::Test {
protected:
    std::unique_ptr<InsetDrawable> mInsetDrawable;
    void SetUp() override {
        mInsetDrawable.reset(new InsetDrawable(new ColorDrawable(0xFF000000), 0));
    }
};

TEST_F(CtsInsetDrawableTest, testConstructor) {
    // int-inset ctors only. Each InsetDrawable owns its child, so a fresh ColorDrawable per
    // instance (CTS shares a borrowed resource drawable; CDROID's mPassDrawable substitute is owned).
    { InsetDrawable d(new ColorDrawable(0xFF000000), 1); }
    { InsetDrawable d(new ColorDrawable(0xFF000000), 1, 1, 1, 1); }
    { InsetDrawable d(nullptr, -1); }
    { InsetDrawable d(nullptr, -1, -1, -1, -1); }
    SUCCEED();
    // float-inset ctors (new InsetDrawable(d, .1f) / 4-arg float) are NOT ported: CDROID's
    // InsetDrawable has no float-inset constructor.
}

TEST_F(CtsInsetDrawableTest, testInvalidateDrawable) {
    // InsetDrawable inherits DrawableWrapper::invalidateDrawable, which forwards to this drawable's
    // Callback (none set here). Must not throw.
    Drawable* child = mInsetDrawable->getDrawable();
    mInsetDrawable->invalidateDrawable(*child);
}

TEST_F(CtsInsetDrawableTest, testScheduleDrawable) {
    Drawable* child = mInsetDrawable->getDrawable();
    Runnable r = [] {};
    mInsetDrawable->scheduleDrawable(*child, r, 10);

    // null params — CDROID forwards to its Callback (none); must not throw.
    mInsetDrawable->scheduleDrawable(*child, Runnable{}, -1);
}

TEST_F(CtsInsetDrawableTest, testUnscheduleDrawable) {
    Drawable* child = mInsetDrawable->getDrawable();
    Runnable r = [] {};
    mInsetDrawable->unscheduleDrawable(*child, r);

    mInsetDrawable->unscheduleDrawable(*child, Runnable{});
}

TEST_F(CtsInsetDrawableTest, testGetChangingConfigurations) {
    mInsetDrawable->setChangingConfigurations(11);
    EXPECT_EQ(11, mInsetDrawable->getChangingConfigurations());

    mInsetDrawable->setChangingConfigurations(-21);
    EXPECT_EQ(-21, mInsetDrawable->getChangingConfigurations());
}

TEST_F(CtsInsetDrawableTest, testGetPadding_dimension) {
    // InsetDrawable(child, 1, 2, 3, 4): left=1, top=2, right=3, bottom=4.
    std::unique_ptr<InsetDrawable> insetDrawable(new InsetDrawable(new ColorDrawable(0xFF000000), 1, 2, 3, 4));
    Rect r;
    EXPECT_EQ(0, r.left);
    EXPECT_EQ(0, r.top);
    EXPECT_EQ(0, r.width);
    EXPECT_EQ(0, r.height);

    EXPECT_TRUE(insetDrawable->getPadding(r));

    EXPECT_EQ(1, r.left);
    EXPECT_EQ(2, r.top);
    // CDROID stores the right inset in Rect::width and the bottom inset in Rect::height
    // (CTS asserts on r.right==3 / r.bottom==4).
    EXPECT_EQ(3, r.width);
    EXPECT_EQ(4, r.height);

    // padding of 0 → getPadding returns false.
    insetDrawable.reset(new InsetDrawable(new ColorDrawable(0xFF000000), 0));
    r = Rect{};
    EXPECT_FALSE(insetDrawable->getPadding(r));
    EXPECT_EQ(0, r.left);
    EXPECT_EQ(0, r.top);
    EXPECT_EQ(0, r.width);
    EXPECT_EQ(0, r.height);
    // testGetPadding_fraction (float-inset ctor) NOT ported — no float-inset constructor.
}

TEST_F(CtsInsetDrawableTest, testSetVisible) {
    EXPECT_FALSE(mInsetDrawable->setVisible(true, true));   // unchanged (already visible)
    EXPECT_TRUE(mInsetDrawable->setVisible(false, true));   // changed
    EXPECT_FALSE(mInsetDrawable->setVisible(false, true));  // unchanged
}

TEST_F(CtsInsetDrawableTest, testSetAlpha) {
    // Forwarded to the wrapped child; just verify no throw across edge values.
    mInsetDrawable->setAlpha(1);
    mInsetDrawable->setAlpha(0);
    mInsetDrawable->setAlpha(INT_MAX);
    mInsetDrawable->setAlpha(INT_MIN);
    SUCCEED();
}

TEST_F(CtsInsetDrawableTest, testSetColorFilter) {
    RefPtr<ColorFilter> cf(new PorterDuffColorFilter(0xFF000000, PorterDuff::SRC_OVER));
    mInsetDrawable->setColorFilter(cf);

    // null must not throw.
    mInsetDrawable->setColorFilter(nullptr);
    SUCCEED();
}

TEST_F(CtsInsetDrawableTest, testGetOpacity) {
    // Wrap child is opaque (0xFF000000); inset is 0.
    mInsetDrawable->setAlpha(255);
    EXPECT_EQ((int)PixelFormat::OPAQUE, mInsetDrawable->getOpacity());

    mInsetDrawable->setAlpha(100);
    EXPECT_EQ((int)PixelFormat::TRANSLUCENT, mInsetDrawable->getOpacity());
}

TEST_F(CtsInsetDrawableTest, testIsStateful) {
    EXPECT_FALSE(mInsetDrawable->isStateful());
}

TEST_F(CtsInsetDrawableTest, testOnStateChange) {
    // CTS's first phase: child is non-stateful (ColorDrawable), so onStateChange is a no-op and
    // the child state stays StateSet::WILD_CARD. The stateful-child phase (statelistdrawable
    // resource) is resource-dependent and NOT ported.
    MockInsetDrawable insetDrawable(new ColorDrawable(0xFF000000), 10);
    Drawable* child = insetDrawable.getDrawable();
    EXPECT_EQ(StateSet::WILD_CARD, child->getState());

    const std::vector<int> state = {1, 2, 3};
    EXPECT_FALSE(insetDrawable.onStateChange(state));
    EXPECT_EQ(StateSet::WILD_CARD, child->getState());
}

TEST_F(CtsInsetDrawableTest, testOnBoundsChange_dimension) {
    MockInsetDrawable insetDrawable(new ColorDrawable(0xFF000000), 5);
    Drawable* child = insetDrawable.getDrawable();

    const Rect& initial = child->getBounds();
    EXPECT_EQ(0, initial.left);
    EXPECT_EQ(0, initial.top);
    EXPECT_EQ(0, initial.width);
    EXPECT_EQ(0, initial.height);

    // Trigger the bounds change with an empty bounds rect (CTS passes new Rect()).
    insetDrawable.onBoundsChange(Rect{});

    const Rect& bounds = child->getBounds();
    EXPECT_EQ(5, bounds.left);
    EXPECT_EQ(5, bounds.top);
    // CDROID subtracts the right/bottom insets into the child bounds' width/height fields
    // (CTS asserts bounds.right==-5 / bounds.bottom==-5).
    EXPECT_EQ(-5, bounds.width);
    EXPECT_EQ(-5, bounds.height);
    // testOnBoundsChange_fraction (float-inset ctor) NOT ported.
}

TEST_F(CtsInsetDrawableTest, testGetConstantState) {
    EXPECT_NE(nullptr, mInsetDrawable->getConstantState());
}

TEST_F(CtsInsetDrawableTest, testOpticalInsets) {
    // InsetDrawable(child, 1, 2, 3, 4) yields optical insets (1, 2, 3, 4) on top of the child's
    // (none for ColorDrawable).
    std::unique_ptr<InsetDrawable> drawable(new InsetDrawable(new ColorDrawable(0xFF000000), 1, 2, 3, 4));
    EXPECT_EQ(Insets::of(1, 2, 3, 4), drawable->getOpticalInsets());
}
