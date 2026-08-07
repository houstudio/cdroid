// AOSP CTS drawable test port (ScaleDrawableTest.java). Pure-logic cases only.
//
// Skipped (not ported):
//  - testDraw: renders to a Canvas (CDROID pixels differ from Skia; null-Canvas NPE is a Java-ism).
//  - testInflate / testInitialLevel: CTS drives inflate through R.xml.scaledrawable + the custom
//    DrawableTestUtils.getAttributeSet helper (a <alias><scale_allattrs> wrapper, not a standalone
//    drawable XML). CDROID has no equivalent test-XML asset or AttributeSet extraction util.
//  - testMutate: loads R.drawable.scaledrawable (a pak asset not present here) and asserts
//    BitmapDrawable paint alpha propagation (CDROID has no Bitmap/paint-alpha mirror).
//  - testGetPaddingNull: null-arg NPE is a Java-ism.
//  - Mockito verify(...) counts (CDROID has no spy): replaced with actual-behaviour assertions
//    (inner alpha after setAlpha, inner level after setLevel, non-null getPadding, etc.).
//
// CDROID divergences noted inline:
//  - Rect is {left,top,width,height} (right()/bottom() derived); setBounds(x,y,w,h) takes
//    width/height, NOT right/bottom like Android. testOnBoundsChange uses Rect{2,2,24,30}
//    (CDROID w=24,h=30) in place of CTS's new Rect(2,2,26,32) (right=26,bottom=32 → w=24,h=30).
//  - DrawableWrapperState::getChangingConfigurations() returns a hard-coded 0 and neither
//    DrawableWrapper nor ScaleDrawable syncs the Drawable's mChangingConfigurations into the state
//    (unlike AOSP), so the second half of CTS testGetConstantState is not ported.
//  - Ownership: CDROID's DrawableWrapper::setDrawable DELETES the previous inner and takes
//    ownership of the new one (no GC). So each wrapped inner is `new`-allocated and the wrapper
//    owns it; a borrowed raw pointer is kept only for inspection, within the wrapper's lifetime.
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/ScaleDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <limits>
#include <memory>
#include <drawable/scaledrawable.h>
#include <drawable/drawables.h>
#include <drawable/statelistdrawable.h>
#include <drawable/stateset.h>
#include <core/app.h>
#include <core/porterduff.h>
#include <core/rect.h>
#include <view/view.h>
#include <view/gravity.h>
#include <guienvironment.h>

using namespace cdroid;

namespace {
// Hand-written equivalent of CTS's inner MockDrawable: a minimal concrete Drawable that records
// onLevelChange invocations. onLevelChange is protected in the base; the public override lets the
// test inspect it. Heap-allocated because DrawableWrapper takes ownership of its inner.
class MockLevelTrackingDrawable : public Drawable {
public:
    bool mCalledOnLevelChange = false;
    void draw(Canvas&) override {}
    bool hasCalledOnLevelChange() const { return mCalledOnLevelChange; }
    void reset() { mCalledOnLevelChange = false; }
    bool onLevelChange(int level) override {
        mCalledOnLevelChange = true;
        return Drawable::onLevelChange(level);
    }
};

// Minimal Drawable that records the exact alpha passed to setAlpha (CTS uses a Mockito spy on a
// ColorDrawable). CDROID's ColorDrawable floors the modulated alpha, so setAlpha(255) on an opaque
// base round-trips as 254; this mock isolates the forwarding logic under test. Also doubles as a
// non-stateful concrete drawable for the isStateful/onStateChange paths.
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

// Exposes the protected onStateChange/onLevelChange/onBoundsChange and tracks onBoundsChange
// invocations (CTS MockScaleDrawable). onBoundsChange marks the flag BEFORE delegating so the flag
// is set even when the delegate early-returns.
class MockScaleDrawable : public ScaleDrawable {
public:
    bool mCalledOnBoundsChange = false;
    MockScaleDrawable(Drawable* drawable, int gravity, float scaleWidth, float scaleHeight)
        : ScaleDrawable(drawable, gravity, scaleWidth, scaleHeight) {}
    bool hasCalledOnBoundsChange() const { return mCalledOnBoundsChange; }
    void reset() { mCalledOnBoundsChange = false; }
    bool onStateChange(const std::vector<int>& state) override {
        return ScaleDrawable::onStateChange(state);
    }
    bool onLevelChange(int level) override { return ScaleDrawable::onLevelChange(level); }
    void onBoundsChange(const Rect& bounds) override {
        mCalledOnBoundsChange = true;
        ScaleDrawable::onBoundsChange(bounds);
    }
};
} // namespace

class CtsScaleDrawableTest : public testing::Test {};

TEST_F(CtsScaleDrawableTest, testConstructor) {
    ColorDrawable* d = new ColorDrawable(0xFFFF0000);
    ScaleDrawable scaleDrawable(d, Gravity::CENTER, 100, 200);
    EXPECT_EQ(d, scaleDrawable.getDrawable());

    // null inner + extreme scale factors — must not throw.
    ScaleDrawable nullSd(nullptr, -1,
                         std::numeric_limits<float>::max(),
                         std::numeric_limits<float>::min());
    EXPECT_EQ(nullptr, nullSd.getDrawable());
}

TEST_F(CtsScaleDrawableTest, testInvalidateDrawable) {
    ScaleDrawable scaleDrawable(new ColorDrawable(0xFFFF0000), Gravity::CENTER, 100, 200);
    MockCallback cb;
    scaleDrawable.setCallback(&cb);
    // DrawableWrapper forwards invalidateDrawable(who) as invalidateDrawable(this) regardless of who.
    ColorDrawable other(0xFF00FF00);
    scaleDrawable.invalidateDrawable(other);
    EXPECT_EQ(1, cb.invalidateCount);
    EXPECT_EQ(&scaleDrawable, cb.lastInvalidate);

    // with no callback, invalidateDrawable must not throw.
    scaleDrawable.setCallback(nullptr);
    scaleDrawable.invalidateDrawable(other);
}

TEST_F(CtsScaleDrawableTest, testScheduleDrawable) {
    ScaleDrawable scaleDrawable(new ColorDrawable(0xFFFF0000), Gravity::CENTER, 100, 200);
    MockCallback cb;
    scaleDrawable.setCallback(&cb);
    Runnable r = [] {};
    scaleDrawable.scheduleDrawable(*scaleDrawable.getDrawable(), r, 1000);
    EXPECT_EQ(1, cb.scheduleCount);

    // with no callback, scheduleDrawable must not throw.
    scaleDrawable.setCallback(nullptr);
    scaleDrawable.scheduleDrawable(*scaleDrawable.getDrawable(), r, 0);
}

TEST_F(CtsScaleDrawableTest, testUnscheduleDrawable) {
    ScaleDrawable scaleDrawable(new ColorDrawable(0xFFFF0000), Gravity::CENTER, 100, 200);
    MockCallback cb;
    scaleDrawable.setCallback(&cb);
    Runnable r = [] {};
    scaleDrawable.unscheduleDrawable(*scaleDrawable.getDrawable(), r);
    EXPECT_EQ(1, cb.unscheduleCount);

    scaleDrawable.setCallback(nullptr);
    scaleDrawable.unscheduleDrawable(*scaleDrawable.getDrawable(), r);
}

TEST_F(CtsScaleDrawableTest, testGetChangingConfigurations) {
    const int SUPER_CONFIG = 1;
    const int CONTAINED_DRAWABLE_CONFIG = 2;

    ColorDrawable* inner = new ColorDrawable(0xFFFFFF00);
    ScaleDrawable scaleDrawable(inner, Gravity::CENTER, 100, 200);
    EXPECT_EQ(0, scaleDrawable.getChangingConfigurations());

    inner->setChangingConfigurations(CONTAINED_DRAWABLE_CONFIG);
    EXPECT_EQ(CONTAINED_DRAWABLE_CONFIG, scaleDrawable.getChangingConfigurations());

    scaleDrawable.setChangingConfigurations(SUPER_CONFIG);
    EXPECT_EQ(SUPER_CONFIG | CONTAINED_DRAWABLE_CONFIG, scaleDrawable.getChangingConfigurations());
}

TEST_F(CtsScaleDrawableTest, testGetPadding) {
    ScaleDrawable scaleDrawable(new ColorDrawable(0xFFFF0000), Gravity::CENTER, 100, 200);
    // delegates to the wrapped ColorDrawable, which reports no padding.
    Rect padding{10, 10, 20, 20};
    EXPECT_FALSE(scaleDrawable.getPadding(padding));
    EXPECT_EQ(0, padding.left);
    EXPECT_EQ(0, padding.top);
    EXPECT_EQ(0, padding.width);
    EXPECT_EQ(0, padding.height);
}

TEST_F(CtsScaleDrawableTest, testSetVisible) {
    ScaleDrawable scaleDrawable(new ColorDrawable(0xFFFF0000), Gravity::CENTER, 100, 200);
    EXPECT_TRUE(scaleDrawable.isVisible());
    EXPECT_TRUE(scaleDrawable.setVisible(false, false));
    EXPECT_FALSE(scaleDrawable.isVisible());
    EXPECT_FALSE(scaleDrawable.setVisible(false, false));
    EXPECT_FALSE(scaleDrawable.isVisible());
    EXPECT_TRUE(scaleDrawable.setVisible(true, false));
    EXPECT_TRUE(scaleDrawable.isVisible());
}

TEST_F(CtsScaleDrawableTest, testSetAlpha) {
    AlphaTrackingDrawable* inner = new AlphaTrackingDrawable();
    ScaleDrawable scaleDrawable(inner, Gravity::CENTER, 100, 200);
    // setAlpha forwards to the wrapped drawable verbatim. CTS uses Mockito verify (spy); the
    // tracking mock stands in for the spy and also avoids CDROID ColorDrawable's alpha floor.
    scaleDrawable.setAlpha(100);
    EXPECT_EQ(100, inner->getAlpha());
    scaleDrawable.setAlpha(255);
    EXPECT_EQ(255, inner->getAlpha());
    scaleDrawable.setAlpha(-1);
    EXPECT_EQ(-1, inner->getAlpha());
}

TEST_F(CtsScaleDrawableTest, testSetColorFilter) {
    ScaleDrawable scaleDrawable(new ColorDrawable(0xFFFF0000), Gravity::CENTER, 100, 200);
    // forwards to the wrapped drawable; just verify it does not throw.
    scaleDrawable.Drawable::setColorFilter(5, PorterDuff::CLEAR);
    SUCCEED();
}

TEST_F(CtsScaleDrawableTest, testGetOpacity) {
    // ScaleDrawable::getOpacity is level-driven: inner level 0 → TRANSPARENT; otherwise the inner's
    // opacity, downgraded to TRANSLUCENT when the inner is OPAQUE but not at full level.
    ScaleDrawable scaleDrawable(new ColorDrawable(0xFFFF0000), Gravity::CENTER, 100, 200);
    // default level 0 (propagated to inner) → TRANSPARENT
    EXPECT_EQ((int)PixelFormat::TRANSPARENT, scaleDrawable.getOpacity());

    scaleDrawable.setLevel(5000);
    EXPECT_EQ((int)PixelFormat::TRANSLUCENT, scaleDrawable.getOpacity());

    scaleDrawable.setLevel(10000);
    EXPECT_EQ((int)PixelFormat::OPAQUE, scaleDrawable.getOpacity());
}

TEST_F(CtsScaleDrawableTest, testIsStateful) {
    ScaleDrawable scaleDrawable(new ColorDrawable(0xFFFF0000), Gravity::CENTER, 100, 200);
    // ColorDrawable.isStateful() is false → wrapper not stateful.
    EXPECT_FALSE(scaleDrawable.isStateful());
}

TEST_F(CtsScaleDrawableTest, testOnStateChangeNonStatefulChild) {
    // A plain (non-stateful) wrapped drawable: onStateChange reports no change and the child's
    // state stays at the initial wild card. (CTS first half of testOnStateChange.)
    ColorDrawable* inner = new ColorDrawable(0xFFFF0000);
    MockScaleDrawable scaleDrawable(inner, Gravity::CENTER, 100, 200);
    EXPECT_EQ(StateSet::WILD_CARD, inner->getState());

    const std::vector<int> state = {1, 2, 3};
    EXPECT_FALSE(scaleDrawable.onStateChange(state));
    EXPECT_EQ(StateSet::WILD_CARD, inner->getState());
}

TEST_F(CtsScaleDrawableTest, testOnStateChangeStatefulChild) {
    // A stateful wrapped drawable (StateListDrawable built programmatically — CTS loads
    // R.drawable.statelistdrawable): onStateChange forwards the state to the child.
    StateListDrawable* inner = new StateListDrawable();
    inner->addState({1}, new ColorDrawable(0xFFFF0000));
    inner->addState(StateSet::WILD_CARD, new ColorDrawable(0xFFFFFF00));
    MockScaleDrawable scaleDrawable(inner, Gravity::CENTER, 100, 200);
    EXPECT_EQ(StateSet::WILD_CARD, inner->getState());

    const std::vector<int> state = {1, 2, 3};
    scaleDrawable.onStateChange(state);
    EXPECT_EQ(state, inner->getState());
}

TEST_F(CtsScaleDrawableTest, testOnLevelChange) {
    MockLevelTrackingDrawable* inner = new MockLevelTrackingDrawable();
    MockScaleDrawable scaleDrawable(inner, Gravity::CENTER, 100, 200);

    // ScaleDrawable.onLevelChange always returns true and triggers onBoundsChange.
    EXPECT_TRUE(scaleDrawable.onLevelChange(0));
    // inner's current level is already 0 → Drawable.setLevel does not fire inner.onLevelChange.
    EXPECT_FALSE(inner->hasCalledOnLevelChange());
    EXPECT_TRUE(scaleDrawable.hasCalledOnBoundsChange());

    inner->reset();
    scaleDrawable.reset();
    EXPECT_TRUE(scaleDrawable.onLevelChange(INT_MIN));
    // level actually changed (0 → INT_MIN) → inner.onLevelChange fired.
    EXPECT_TRUE(inner->hasCalledOnLevelChange());
    EXPECT_TRUE(scaleDrawable.hasCalledOnBoundsChange());
}

TEST_F(CtsScaleDrawableTest, testOnBoundsChange) {
    // CTS bounds = new Rect(2,2,26,32) → left=2,top=2,width=24,height=30. CDROID Rect{2,2,24,30}.
    const Rect bounds{2, 2, 24, 30};
    Rect expected;

    // Case 1: LEFT gravity, scale 0.3/0.3, level 0 (default). The scale formula reduces to
    // w -= (int)(w*scaleW), h -= (int)(h*scaleH); Gravity.apply places the result at LEFT.
    {
        ColorDrawable* inner = new ColorDrawable(0xFFFFFF00);
        MockScaleDrawable sd(inner, Gravity::LEFT, 0.3f, 0.3f);
        inner->setBounds(bounds);
        sd.onBoundsChange(bounds);
        Gravity::apply(Gravity::LEFT,
                       bounds.width  - (int)(bounds.width  * 0.3f),
                       bounds.height - (int)(bounds.height * 0.3f),
                       bounds, expected);
        EXPECT_EQ(expected.left, inner->getBounds().left);
        EXPECT_EQ(expected.top, inner->getBounds().top);
        EXPECT_EQ(expected.width, inner->getBounds().width);
        EXPECT_EQ(expected.height, inner->getBounds().height);
    }

    // Case 2: BOTTOM|RIGHT gravity, scale 0.6/0.7, level 4000 — the full level-weighted formula.
    {
        ColorDrawable* inner = new ColorDrawable(0xFFFFFF00);
        MockScaleDrawable sd(inner, Gravity::BOTTOM | Gravity::RIGHT, 0.6f, 0.7f);
        const int level = 4000;
        inner->setBounds(bounds);
        sd.setLevel(level);
        sd.onBoundsChange(bounds);
        Gravity::apply(Gravity::BOTTOM | Gravity::RIGHT,
                       bounds.width  - (int)(bounds.width  * 0.6f * (10000 - level) / 10000),
                       bounds.height - (int)(bounds.height * 0.7f * (10000 - level) / 10000),
                       bounds, expected);
        EXPECT_EQ(expected.left, inner->getBounds().left);
        EXPECT_EQ(expected.top, inner->getBounds().top);
        EXPECT_EQ(expected.width, inner->getBounds().width);
        EXPECT_EQ(expected.height, inner->getBounds().height);
    }

    // Case 3: scaleWidth=0 and scaleHeight<0 → neither axis scales; inner bounds stay = bounds.
    {
        ColorDrawable* inner = new ColorDrawable(0xFFFFFF00);
        MockScaleDrawable sd(inner, Gravity::BOTTOM | Gravity::RIGHT, 0.f, -0.3f);
        inner->setBounds(bounds);
        sd.onBoundsChange(bounds);
        EXPECT_EQ(bounds.left, inner->getBounds().left);
        EXPECT_EQ(bounds.top, inner->getBounds().top);
        EXPECT_EQ(bounds.width, inner->getBounds().width);
        EXPECT_EQ(bounds.height, inner->getBounds().height);
    }

    // Case 4: scaleWidth=1, scaleHeight=1.7 → computed w<=0, so ScaleDrawable skips setBounds and
    // the inner keeps its preset bounds (= bounds).
    {
        ColorDrawable* inner = new ColorDrawable(0xFFFFFF00);
        MockScaleDrawable sd(inner, Gravity::BOTTOM | Gravity::RIGHT, 1.f, 1.7f);
        inner->setBounds(bounds);
        sd.onBoundsChange(bounds);
        EXPECT_EQ(bounds.left, inner->getBounds().left);
        EXPECT_EQ(bounds.top, inner->getBounds().top);
        EXPECT_EQ(bounds.width, inner->getBounds().width);
        EXPECT_EQ(bounds.height, inner->getBounds().height);
    }
}

TEST_F(CtsScaleDrawableTest, testGetIntrinsicWidth) {
    ScaleDrawable scaleDrawable(new ColorDrawable(0xFFFF0000), Gravity::CENTER, 100, 200);
    // delegates to the wrapped ColorDrawable, which reports no intrinsic size.
    EXPECT_EQ(-1, scaleDrawable.getIntrinsicWidth());
}

TEST_F(CtsScaleDrawableTest, testGetIntrinsicHeight) {
    ScaleDrawable scaleDrawable(new ColorDrawable(0xFFFF0000), Gravity::CENTER, 100, 200);
    EXPECT_EQ(-1, scaleDrawable.getIntrinsicHeight());
}

TEST_F(CtsScaleDrawableTest, testGetConstantState) {
    ScaleDrawable scaleDrawable(new ColorDrawable(0xFFFF0000), Gravity::CENTER, 100, 200);
    auto state = scaleDrawable.getConstantState();
    EXPECT_NE(nullptr, state);
    EXPECT_EQ(0, state->getChangingConfigurations());
    // CTS then does setChangingConfigurations(1) and expects the state to report 1. CDROID's
    // DrawableWrapperState::getChangingConfigurations() returns a hard-coded 0 and ScaleDrawable
    // does not sync the Drawable's own configs into the state (unlike AOSP), so that part is not
    // ported.
}
