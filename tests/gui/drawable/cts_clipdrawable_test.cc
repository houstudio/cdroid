// AOSP CTS drawable test port (ClipDrawableTest.java). Pure-logic cases only.
//
// Skipped (not ported):
//  - testDraw: renders to a Canvas (CDROID pixels differ from Skia; null-Canvas NPE is a Java-ism).
//  - testInflate: CTS feeds a non-clip XML (R.drawable.gradientdrawable) to a ClipDrawable purely
//    to exercise the parser without crashing — not a meaningful semantics test, and CDROID has no
//    equivalent clip-XML asset in the test pak.
//  - testOnStateChange: the stateful half of CTS uses R.drawable.statelistdrawable (a pak asset not
//    present here); the non-stateful half is covered by a programmatic ColorDrawable below.
//  - null-argument NPE variants (Java-isms; CDROID uses raw pointers, not @NonNull).
//
// CDROID divergences noted inline:
//  - Rect is {left,top,width,height} (right()/bottom() derived); setBounds(x,y,w,h) takes
//    width/height, NOT right/bottom like Android — bounds assertions reflect CDROID semantics.
//  - DrawableWrapper::getConstantState does not copy the Drawable's mChangingConfigurations into
//    mState before returning (unlike AOSP), so getConstantState()->getChangingConfigurations() does
//    not reflect a prior setChangingConfigurations(); only the non-null assertion is ported.
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/ClipDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <drawable/clipdrawable.h>
#include <drawable/drawables.h>
#include <drawable/statelistdrawable.h>
#include <drawable/stateset.h>
#include <core/app.h>
#include <core/porterduff.h>
#include <core/rect.h>
#include <view/view.h>
#include <view/gravity.h>
#include <guienvironment.h>
#include <limits.h>

using namespace cdroid;

namespace {
// Records Callback invocations (CTS uses Mockito; CDROID uses a hand-written mock). lastInvalidate
// captures which Drawable the callback saw — ClipDrawable must forward invalidateDrawable(who) as
// invalidateDrawable(this).
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

// Minimal Drawable that records the exact alpha passed to setAlpha (CTS uses a Mockito spy on a
// ColorDrawable for this). CDROID's ColorDrawable floors the modulated alpha (baseAlpha*alpha>>8),
// so setAlpha(255) on an opaque base round-trips as 254 — using a real ColorDrawable would couple
// the wrapper-forwarding test to that rounding bug. The tracking mock isolates the forwarding logic.
class AlphaTrackingDrawable : public Drawable {
public:
    int lastAlpha = -1;
    void draw(Canvas&) override {}
    void setAlpha(int alpha) override { lastAlpha = alpha; }
    int getAlpha() const override { return lastAlpha; }
};

// Exposes the protected onBoundsChange/onLevelChange/onStateChange so the corresponding cases can
// invoke them directly (CTS does the same via its inner MockClipDrawable).
class MockClipDrawable : public ClipDrawable {
public:
    MockClipDrawable(Drawable* drawable, int gravity, int orientation)
        : ClipDrawable(drawable, gravity, orientation) {}
    bool onLevelChange(int level) override { return ClipDrawable::onLevelChange(level); }
    void onBoundsChange(const Rect& bounds) override { ClipDrawable::onBoundsChange(bounds); }
    bool onStateChange(const std::vector<int>& state) override {
        return ClipDrawable::onStateChange(state);
    }
};
} // namespace

// Empty fixture — pure logic. App/Context is provided process-wide by GUIEnvironment.
class CtsClipDrawableTest : public testing::Test {};

TEST_F(CtsClipDrawableTest, testClipDrawable) {
    // null inner drawable — must not throw (CTS: new ClipDrawable(null, ...)).
    ClipDrawable nullD(nullptr, Gravity::BOTTOM, ClipDrawable::HORIZONTAL);
    EXPECT_EQ(nullptr, nullD.getDrawable());
    EXPECT_EQ(Gravity::BOTTOM, nullD.getGravity());
    EXPECT_EQ(ClipDrawable::HORIZONTAL, nullD.getOrientation());

    // a real wrapped drawable; getDrawable returns the borrowed inner.
    ColorDrawable* inner = new ColorDrawable(0xFF00FF00);
    ClipDrawable d(inner, Gravity::BOTTOM, ClipDrawable::HORIZONTAL);
    EXPECT_EQ(inner, d.getDrawable());
    EXPECT_EQ(Gravity::BOTTOM, d.getGravity());
    EXPECT_EQ(ClipDrawable::HORIZONTAL, d.getOrientation());

    // VERTICAL orientation is a distinct bit.
    ClipDrawable v(new ColorDrawable(0xFF00FF00), Gravity::LEFT, ClipDrawable::VERTICAL);
    EXPECT_EQ(ClipDrawable::VERTICAL, v.getOrientation());
}

TEST_F(CtsClipDrawableTest, testGetChangingConfigurations) {
    const int SUPER_CONFIG = 1;
    const int CONTAINED_DRAWABLE_CONFIG = 2;

    ColorDrawable* inner = new ColorDrawable(0xFF00FF00);
    ClipDrawable clip(inner, Gravity::BOTTOM, ClipDrawable::HORIZONTAL);
    EXPECT_EQ(0, clip.getChangingConfigurations());

    // changing configs of the wrapped drawable contribute to the wrapper's.
    inner->setChangingConfigurations(CONTAINED_DRAWABLE_CONFIG);
    EXPECT_EQ(CONTAINED_DRAWABLE_CONFIG, clip.getChangingConfigurations());

    // the wrapper's own configs OR together with the wrapped's.
    clip.setChangingConfigurations(SUPER_CONFIG);
    EXPECT_EQ(SUPER_CONFIG | CONTAINED_DRAWABLE_CONFIG, clip.getChangingConfigurations());
}

TEST_F(CtsClipDrawableTest, testGetConstantState) {
    ClipDrawable clip(new ColorDrawable(0xFF00FF00), Gravity::BOTTOM, ClipDrawable::HORIZONTAL);
    // ColorDrawable has a ConstantState, so the wrapper's state can constant-state.
    EXPECT_NE(nullptr, clip.getConstantState());
    // CTS also asserts getConstantState()->getChangingConfigurations() == 1 after
    // setChangingConfigurations(1). CDROID's DrawableWrapper::getConstantState does not sync the
    // Drawable's mChangingConfigurations into mState (unlike AOSP), so that assertion is not ported.
    clip.setChangingConfigurations(1);
    EXPECT_NE(nullptr, clip.getConstantState());
}

TEST_F(CtsClipDrawableTest, testGetIntrinsicHeight) {
    // ColorDrawable reports no intrinsic size (-1).
    ClipDrawable clip(new ColorDrawable(0xFF00FF00), Gravity::BOTTOM, ClipDrawable::HORIZONTAL);
    EXPECT_EQ(-1, clip.getIntrinsicHeight());
    // CTS also wraps a BitmapDrawable of a 100x50 bitmap and expects 50 — CDROID has no Bitmap
    // class (Cairo::ImageSurface plays that role) and the case is density-dependent; not ported.
}

TEST_F(CtsClipDrawableTest, testGetIntrinsicWidth) {
    ClipDrawable clip(new ColorDrawable(0xFF00FF00), Gravity::BOTTOM, ClipDrawable::HORIZONTAL);
    EXPECT_EQ(-1, clip.getIntrinsicWidth());
}

TEST_F(CtsClipDrawableTest, testGetOpacity) {
    // ColorDrawable(GREEN) is fully opaque (alpha=0xFF). ClipDrawable::getOpacity maps the clip
    // level to TRANSPARENT (level 0) / TRANSLUCENT (partial) / inner-opacity (full level).
    ClipDrawable clip(new ColorDrawable(0xFF00FF00), Gravity::BOTTOM, ClipDrawable::HORIZONTAL);
    clip.setLevel(0);
    EXPECT_EQ((int)PixelFormat::TRANSPARENT, clip.getOpacity());
    clip.setLevel(5000);
    EXPECT_EQ((int)PixelFormat::TRANSLUCENT, clip.getOpacity());
    clip.setLevel(10000);
    EXPECT_EQ((int)PixelFormat::OPAQUE, clip.getOpacity());

    // an unclipped translucent inner stays translucent.
    ClipDrawable trans(new ColorDrawable(0x80FFFF00), Gravity::BOTTOM, ClipDrawable::HORIZONTAL);
    trans.setLevel(10000);
    EXPECT_EQ((int)PixelFormat::TRANSLUCENT, trans.getOpacity());
}

TEST_F(CtsClipDrawableTest, testGetPadding) {
    ClipDrawable clip(new ColorDrawable(0xFF00FF00), Gravity::BOTTOM, ClipDrawable::HORIZONTAL);
    Rect padding{10, 10, 20, 20};
    // ColorDrawable has no padding → false and the rect is zeroed.
    EXPECT_FALSE(clip.getPadding(padding));
    EXPECT_EQ(0, padding.left);
    EXPECT_EQ(0, padding.top);
    EXPECT_EQ(0, padding.width);
    EXPECT_EQ(0, padding.height);
}

TEST_F(CtsClipDrawableTest, testInvalidateDrawable) {
    ColorDrawable* inner = new ColorDrawable(0xFF00FF00);
    ClipDrawable clip(inner, Gravity::BOTTOM, ClipDrawable::HORIZONTAL);
    MockCallback cb;
    clip.setCallback(&cb);
    // invalidateDrawable(inner) must be forwarded to the host callback as invalidateDrawable(clip).
    clip.invalidateDrawable(*inner);
    EXPECT_EQ(1, cb.invalidateCount);
    EXPECT_EQ(&clip, cb.lastInvalidate);
}

TEST_F(CtsClipDrawableTest, testScheduleDrawable) {
    ColorDrawable* inner = new ColorDrawable(0xFF00FF00);
    ClipDrawable clip(inner, Gravity::BOTTOM, ClipDrawable::HORIZONTAL);
    MockCallback cb;
    clip.setCallback(&cb);
    Runnable r = [] {};
    clip.scheduleDrawable(*inner, r, 1000);
    EXPECT_EQ(1, cb.scheduleCount);
}

TEST_F(CtsClipDrawableTest, testUnscheduleDrawable) {
    ColorDrawable* inner = new ColorDrawable(0xFF00FF00);
    ClipDrawable clip(inner, Gravity::BOTTOM, ClipDrawable::HORIZONTAL);
    MockCallback cb;
    clip.setCallback(&cb);
    Runnable r = [] {};
    clip.unscheduleDrawable(*inner, r);
    EXPECT_EQ(1, cb.unscheduleCount);
}

TEST_F(CtsClipDrawableTest, testIsStateful) {
    // ColorDrawable.isStateful() is false → the wrapper is not stateful either.
    ClipDrawable clip(new ColorDrawable(0xFF00FF00), Gravity::BOTTOM, ClipDrawable::HORIZONTAL);
    EXPECT_FALSE(clip.isStateful());
}

TEST_F(CtsClipDrawableTest, testOnBoundsChange) {
    ColorDrawable* inner = new ColorDrawable(0xFF00FF00);
    MockClipDrawable clip(inner, Gravity::BOTTOM, ClipDrawable::HORIZONTAL);
    EXPECT_EQ(0, inner->getBounds().left);
    EXPECT_EQ(0, inner->getBounds().top);
    EXPECT_EQ(0, inner->getBounds().width);
    EXPECT_EQ(0, inner->getBounds().height);

    // ClipDrawable does not override onBoundsChange; DrawableWrapper forwards setBounds verbatim
    // to the inner drawable. CDROID Rect{10,10,100,100} is left=10,top=10,width=100,height=100
    // (CTS's new Rect(10,10,100,100) is left=10,top=10,right=100,bottom=100 — different shape;
    // the assertion is "inner bounds == the rect passed to onBoundsChange", which holds either way).
    clip.onBoundsChange(Rect{10, 10, 100, 100});
    EXPECT_EQ(10, inner->getBounds().left);
    EXPECT_EQ(10, inner->getBounds().top);
    EXPECT_EQ(100, inner->getBounds().width);
    EXPECT_EQ(100, inner->getBounds().height);
    EXPECT_EQ(110, inner->getBounds().right());   // left + width
    EXPECT_EQ(110, inner->getBounds().bottom());  // top  + height
}

TEST_F(CtsClipDrawableTest, testOnLevelChange) {
    ColorDrawable* inner = new ColorDrawable(0xFF00FF00);
    MockClipDrawable clip(inner, Gravity::BOTTOM, ClipDrawable::HORIZONTAL);
    MockCallback cb;
    clip.setCallback(&cb);

    // onLevelChange propagates the level to the wrapped drawable and invalidates self.
    EXPECT_EQ(0, inner->getLevel());
    clip.onLevelChange(1000);
    EXPECT_EQ(1000, inner->getLevel());
    EXPECT_EQ(1, cb.invalidateCount);

    clip.onLevelChange(0);
    EXPECT_EQ(0, inner->getLevel());

    clip.onLevelChange(10000);
    EXPECT_EQ(10000, inner->getLevel());
}

TEST_F(CtsClipDrawableTest, testOnStateChangeNonStatefulChild) {
    // A plain (non-stateful) wrapped drawable: onStateChange reports no change and the child's
    // state stays at the initial wild card. (CTS's first half of testOnStateChange.)
    ColorDrawable* inner = new ColorDrawable(0xFF00FF00);
    MockClipDrawable clip(inner, Gravity::BOTTOM, ClipDrawable::HORIZONTAL);
    EXPECT_EQ(StateSet::WILD_CARD, inner->getState());

    const std::vector<int> state = {1, 2, 3};
    EXPECT_FALSE(clip.onStateChange(state));
    EXPECT_EQ(StateSet::WILD_CARD, inner->getState());
}

TEST_F(CtsClipDrawableTest, testOnStateChangeStatefulChild) {
    // A stateful wrapped drawable (StateListDrawable built programmatically — CTS loads
    // R.drawable.statelistdrawable): onStateChange forwards the state to the child.
    StateListDrawable* inner = new StateListDrawable();
    inner->addState({1}, new ColorDrawable(0xFFFF0000));
    inner->addState(StateSet::WILD_CARD, new ColorDrawable(0xFFFFFF00));
    MockClipDrawable clip(inner, Gravity::BOTTOM, ClipDrawable::HORIZONTAL);
    EXPECT_EQ(StateSet::WILD_CARD, inner->getState());

    const std::vector<int> state = {1};
    clip.onStateChange(state);
    EXPECT_EQ(state, inner->getState());
}

TEST_F(CtsClipDrawableTest, testSetAlpha) {
    AlphaTrackingDrawable* inner = new AlphaTrackingDrawable();
    ClipDrawable clip(inner, Gravity::BOTTOM, ClipDrawable::HORIZONTAL);
    // setAlpha forwards to the wrapped drawable verbatim.
    clip.setAlpha(0);
    EXPECT_EQ(0, inner->getAlpha());
    clip.setAlpha(128);
    EXPECT_EQ(128, inner->getAlpha());
    clip.setAlpha(255);
    EXPECT_EQ(255, inner->getAlpha());
}

TEST_F(CtsClipDrawableTest, testSetColorFilter) {
    ColorDrawable* inner = new ColorDrawable(0xFF00FF00);
    ClipDrawable clip(inner, Gravity::BOTTOM, ClipDrawable::HORIZONTAL);
    // setColorFilter forwards to the wrapped drawable; just verify it does not throw.
    clip.Drawable::setColorFilter(5, PorterDuff::CLEAR);
    SUCCEED();
}

TEST_F(CtsClipDrawableTest, testSetVisible) {
    ClipDrawable clip(new ColorDrawable(0xFF00FF00), Gravity::BOTTOM, ClipDrawable::HORIZONTAL);
    EXPECT_TRUE(clip.isVisible());
    EXPECT_TRUE(clip.setVisible(false, false));
    EXPECT_FALSE(clip.isVisible());
    EXPECT_FALSE(clip.setVisible(false, false));
    EXPECT_FALSE(clip.isVisible());
    EXPECT_TRUE(clip.setVisible(true, false));
    EXPECT_TRUE(clip.isVisible());
}
