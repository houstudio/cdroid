// AOSP CTS drawable test port (DrawableTest.java). Pure-logic cases only — render/pixel/
// InputStream/Theme-dependent cases (createFromPath/Stream/ResourceStream, image density, themed
// XML, transparent region) are NOT ported (CDROID has no top-level createFromPath/Stream and Cairo
// pixels differ from Skia). CDROID's Rect is {left,top,width,height} (right()/bottom() are
// derived), and setBounds(x,y,w,h) takes width/height (NOT right/bottom like Android) — bounds
// assertions reflect CDROID's semantics, with a note where they diverge from CTS's right/bottom.
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/DrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <drawable/drawable.h>
#include <drawable/drawables.h>
#include <core/app.h>
#include <core/porterduff.h>
#include <core/rect.h>
#include <core/insets.h>
#include <view/view.h>
#include <guienvironment.h>
#include <limits.h>

using namespace cdroid;

namespace {
// Minimal concrete Drawable (CTS MockDrawable). Only draw() is pure-virtual in CDROID's Drawable;
// the rest use the base defaults. The protected onBoundsChange/onLevelChange/onStateChange are
// re-exposed so the testOn*Change cases can invoke them directly (CTS calls them on a subclass).
class MockDrawable : public Drawable {
public:
    void draw(Canvas&) override {}
    using Drawable::onBoundsChange;
    using Drawable::onLevelChange;
    using Drawable::onStateChange;
};

// Records Callback invocations (CTS uses Mockito; CDROID uses a hand-written mock).
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

// Empty fixture — pure logic, no per-case setup. App/Context is provided process-wide by
// GUIEnvironment (see guienvironment.h).
class CtsDrawableTest : public testing::Test {};

TEST_F(CtsDrawableTest, testClearColorFilter) {
    MockDrawable d;
    // initial color filter is null; clear is a no-op that must not throw
    EXPECT_EQ(nullptr, d.getColorFilter());
    d.clearColorFilter();
    EXPECT_EQ(nullptr, d.getColorFilter());
}

TEST_F(CtsDrawableTest, testCopyBounds) {
    MockDrawable d;
    Rect r;
    d.copyBounds(r);
    EXPECT_EQ(0, r.left);
    EXPECT_EQ(0, r.top);
    EXPECT_EQ(0, r.width);
    EXPECT_EQ(0, r.height);

    // CDROID setBounds(x,y,w,h): w,h are width/height (CTS passes right/bottom; divergence noted).
    d.setBounds(10, 10, 100, 100);
    d.copyBounds(r);
    EXPECT_EQ(10, r.left);
    EXPECT_EQ(10, r.top);
    EXPECT_EQ(100, r.width);
    EXPECT_EQ(100, r.height);
    EXPECT_EQ(110, r.right());   // left + width
    EXPECT_EQ(110, r.bottom());  // top + height

    d.setBounds(Rect{50, 50, 500, 500});
    d.copyBounds(r);
    EXPECT_EQ(50, r.left);
    EXPECT_EQ(50, r.top);
    EXPECT_EQ(500, r.width);
    EXPECT_EQ(500, r.height);
}

TEST_F(CtsDrawableTest, testAccessBounds) {
    MockDrawable d;
    d.setBounds(0, 0, 100, 100);  // CDROID: width=100, height=100
    const Rect& r = d.getBounds();
    EXPECT_EQ(0, r.left);
    EXPECT_EQ(0, r.top);
    EXPECT_EQ(100, r.width);
    EXPECT_EQ(100, r.height);
    EXPECT_EQ(100, r.right());
    EXPECT_EQ(100, r.bottom());

    d.setBounds(Rect{10, 10, 150, 150});
    EXPECT_EQ(10, d.getBounds().left);
    EXPECT_EQ(10, d.getBounds().top);
    EXPECT_EQ(150, d.getBounds().width);
    EXPECT_EQ(150, d.getBounds().height);
}

TEST_F(CtsDrawableTest, testAccessChangingConfigurations) {
    MockDrawable d;
    EXPECT_EQ(0, d.getChangingConfigurations());
    d.setChangingConfigurations(1);
    EXPECT_EQ(1, d.getChangingConfigurations());
    d.setChangingConfigurations(INT_MAX);
    EXPECT_EQ(INT_MAX, d.getChangingConfigurations());
    d.setChangingConfigurations(INT_MIN);
    EXPECT_EQ(INT_MIN, d.getChangingConfigurations());
}

TEST_F(CtsDrawableTest, testGetConstantState) {
    MockDrawable d;
    EXPECT_EQ(nullptr, d.getConstantState());
}

TEST_F(CtsDrawableTest, testGetCurrent) {
    MockDrawable d;
    EXPECT_EQ(&d, d.getCurrent());
}

TEST_F(CtsDrawableTest, testGetIntrinsicHeight) {
    MockDrawable d;
    EXPECT_EQ(-1, d.getIntrinsicHeight());
}

TEST_F(CtsDrawableTest, testGetIntrinsicWidth) {
    MockDrawable d;
    EXPECT_EQ(-1, d.getIntrinsicWidth());
}

TEST_F(CtsDrawableTest, testAccessLevel) {
    MockDrawable d;
    EXPECT_EQ(0, d.getLevel());
    EXPECT_FALSE(d.setLevel(10));
    EXPECT_EQ(10, d.getLevel());
    EXPECT_FALSE(d.setLevel(20));
    EXPECT_EQ(20, d.getLevel());
    EXPECT_FALSE(d.setLevel(0));
    EXPECT_EQ(0, d.getLevel());
    EXPECT_FALSE(d.setLevel(10000));
    EXPECT_EQ(10000, d.getLevel());
}

TEST_F(CtsDrawableTest, testGetMinimumHeight) {
    MockDrawable d;
    EXPECT_EQ(0, d.getMinimumHeight());
}

TEST_F(CtsDrawableTest, testGetMinimumWidth) {
    MockDrawable d;
    EXPECT_EQ(0, d.getMinimumWidth());
}

TEST_F(CtsDrawableTest, testGetPadding) {
    MockDrawable d;
    Rect r{10, 10, 20, 20};
    EXPECT_FALSE(d.getPadding(r));
    EXPECT_EQ(0, r.left);
    EXPECT_EQ(0, r.top);
    EXPECT_EQ(0, r.width);
    EXPECT_EQ(0, r.height);
}

TEST_F(CtsDrawableTest, testAccessState) {
    MockDrawable d;
    EXPECT_EQ(StateSet::WILD_CARD, d.getState());
    const std::vector<int> states = {1, 2, 3};
    EXPECT_FALSE(d.setState(states));
    EXPECT_EQ(states, d.getState());
}

TEST_F(CtsDrawableTest, testIsStateful) {
    MockDrawable d;
    EXPECT_FALSE(d.isStateful());
}

TEST_F(CtsDrawableTest, testVisible) {
    MockDrawable d;
    EXPECT_TRUE(d.isVisible());
    EXPECT_TRUE(d.setVisible(false, false));
    EXPECT_FALSE(d.isVisible());
    EXPECT_FALSE(d.setVisible(false, false));
    EXPECT_FALSE(d.isVisible());
    EXPECT_TRUE(d.setVisible(true, false));
    EXPECT_TRUE(d.isVisible());
}

TEST_F(CtsDrawableTest, testOnBoundsChange) {
    MockDrawable d;
    // No-op in the Drawable superclass.
    d.onBoundsChange(Rect{0, 0, 10, 10});
}

TEST_F(CtsDrawableTest, testOnLevelChange) {
    MockDrawable d;
    EXPECT_FALSE(d.onLevelChange(0));
}

TEST_F(CtsDrawableTest, testOnStateChange) {
    MockDrawable d;
    EXPECT_FALSE(d.onStateChange(std::vector<int>()));
}

TEST_F(CtsDrawableTest, testResolveOpacity) {
    EXPECT_EQ((int)PixelFormat::TRANSLUCENT,
              Drawable::resolveOpacity(PixelFormat::TRANSLUCENT, PixelFormat::TRANSLUCENT));
    EXPECT_EQ((int)PixelFormat::UNKNOWN,
              Drawable::resolveOpacity(PixelFormat::UNKNOWN, PixelFormat::TRANSLUCENT));
    EXPECT_EQ((int)PixelFormat::TRANSLUCENT,
              Drawable::resolveOpacity(PixelFormat::OPAQUE, PixelFormat::TRANSLUCENT));
    EXPECT_EQ((int)PixelFormat::TRANSPARENT,
              Drawable::resolveOpacity(PixelFormat::OPAQUE, PixelFormat::TRANSPARENT));
    // CTS also asserts resolveOpacity(RGB_888, RGB_565)==OPAQUE — CDROID's PixelFormat enum lacks
    // RGB_888/RGB_565, so that clause is not ported.
}

TEST_F(CtsDrawableTest, testAccessCallback) {
    MockDrawable d;
    MockCallback cb;
    d.setCallback(&cb);
    EXPECT_EQ(&cb, d.getCallback());
    d.setCallback(nullptr);
    EXPECT_EQ(nullptr, d.getCallback());
}

TEST_F(CtsDrawableTest, testInvalidateSelf) {
    MockDrawable d;
    // without a callback, invalidateSelf() must not throw
    d.invalidateSelf();
    MockCallback cb;
    d.setCallback(&cb);
    d.invalidateSelf();
    EXPECT_EQ(1, cb.invalidateCount);
}

TEST_F(CtsDrawableTest, testScheduleSelf) {
    MockDrawable d;
    // null runnable / no callback must not throw
    d.scheduleSelf(nullptr, 1000);
    Runnable r = [] {};
    d.scheduleSelf(r, 1000);
    MockCallback cb;
    d.setCallback(&cb);
    d.scheduleSelf(r, 1000);
    d.scheduleSelf(r, 0);
    d.scheduleSelf(r, -1000);
    EXPECT_EQ(3, cb.scheduleCount);
}

TEST_F(CtsDrawableTest, testUnscheduleSelf) {
    MockDrawable d;
    MockCallback cb;
    d.setCallback(&cb);
    d.unscheduleSelf(nullptr);
    EXPECT_EQ(1, cb.unscheduleCount);
}

TEST_F(CtsDrawableTest, testSetColorFilter) {
    MockDrawable d;
    // No-op in the Drawable superclass; just verify it does not throw.
    d.setColorFilter(5, PorterDuff::CLEAR);
    SUCCEED();
}

TEST_F(CtsDrawableTest, testSetDither) {
    MockDrawable d;
    d.setDither(false);
    SUCCEED();
}

TEST_F(CtsDrawableTest, testSetHotspotBounds) {
    MockDrawable d;
    d.setHotspotBounds(10, 15, 100, 150);
    SUCCEED();
}

TEST_F(CtsDrawableTest, testGetHotspotBounds) {
    MockDrawable d;
    Rect r;
    d.getHotspotBounds(r);
    SUCCEED();
}

TEST_F(CtsDrawableTest, testAccessLayoutDirection) {
    MockDrawable d;
    d.setLayoutDirection(View::LAYOUT_DIRECTION_LTR);
    EXPECT_EQ(View::LAYOUT_DIRECTION_LTR, d.getLayoutDirection());
    d.setLayoutDirection(View::LAYOUT_DIRECTION_RTL);
    EXPECT_EQ(View::LAYOUT_DIRECTION_RTL, d.getLayoutDirection());
}

TEST_F(CtsDrawableTest, testMutate) {
    MockDrawable d;
    EXPECT_EQ(&d, d.mutate());
}

TEST_F(CtsDrawableTest, testCreateFromXml) {
    // gradientdrawable.xml is a <shape> with <size width=42px height=63px> (ported from CTS,
    // res/drawable/gradientdrawable.xml). Exercises DrawableInflater parsing the android: namespace
    // and the shape's intrinsic size. getDrawable() returns a borrowed (cached) instance.
    Drawable* d = App::getInstance().getDrawable("@drawable/gradientdrawable");
    ASSERT_NE(nullptr, d);
    EXPECT_EQ(42, d->getIntrinsicWidth());
    EXPECT_EQ(63, d->getIntrinsicHeight());
}
