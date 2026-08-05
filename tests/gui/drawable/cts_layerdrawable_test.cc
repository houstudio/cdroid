// AOSP CTS drawable test port (LayerDrawableTest.java). Pure-logic cases only — pixel/render,
// density (testPreloadDensity*/_tvdpi), Mockito-spy (intrinsic size from spied ColorDrawable),
// tint-driven opacity/statefulness (testOpacityChange/testStatefulnessChange), and the XML inflate
// path (testInflate, needs the CTS layerdrawable.xml asset with png refs) are NOT ported. A small
// IntrinsicSizeDrawable helper replaces CTS's spied ColorDrawable where a controlled intrinsic size
// is required; a MockCallback replaces Mockito for the callback-relay cases.
//
// CDROID↔Android differences (see layerdrawable.h):
//  - LayerDrawable(const std::vector<Drawable*>&) takes a vector, not a Java array; the ctor null
//    case (testConstructorNull) is not expressible and is skipped.
//  - Several index accessors diverge from CTS's IndexOutOfBoundsException:
//      * getId(out-of-range) returns -1 (CTS throws)              — ported as EXPECT_EQ(-1, ...)
//      * getDrawable(out-of-range) returns nullptr (CTS throws)   — ported as EXPECT_EQ(nullptr, ...)
//      * setId/setLayerInset*/getLayerInset*(out-of-range) use unchecked operator[] (undefined
//        behavior, NOT an exception)                              — the OOB sub-cases are skipped.
//      * setLayerSize/Width/Height/Gravity use at() (throws std::out_of_range) — OOB sub-cases ported.
//  - findDrawableByLayerId/findIndexByLayerId return the FIRST matching child; CTS relies on the
//    last match for duplicate IDs, so the duplicate-id sub-case is skipped.
//  - LayerDrawable does not override getChangingConfigurations (only LayerState does), so child
//    configs are NOT aggregated in the drawable-level getter; testGetChangingConfigurations is
//    skipped (would assert a behavior CDROID does not implement).
//  - Rect is {left,top,width,height}; setHotspotBounds(l,t,w,h) takes width/height (not right/bottom).
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/LayerDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <memory>
#include <limits.h>
#include <vector>
#include <drawable/layerdrawable.h>
#include <drawable/drawables.h>
#include <drawable/statelistdrawable.h>
#include <core/app.h>
#include <core/rect.h>
#include <view/view.h>
#include <view/gravity.h>
#include <guienvironment.h>

using namespace cdroid;

namespace {
// Configurable-opacity / configurable-isStateful drawable standing in for CTS's spied ColorDrawable.
// The base Drawable returns nullptr from getConstantState() (used by some mutate/clone reasoning).
class MockDrawable : public Drawable {
public:
    int mOpacity;
    bool mIsStateful;
    explicit MockDrawable(int opacity = PixelFormat::OPAQUE, bool stateful = false)
        : mOpacity(opacity), mIsStateful(stateful) {}
    void draw(Canvas&) override {}
    int getOpacity() const override { return mOpacity; }
    bool isStateful() const override { return mIsStateful; }
};

// Drawable with a controllable intrinsic width/height (CTS's spied ColorDrawable with doReturn for
// getIntrinsicWidth/Height). Used by the inset/intrinsic-size cases.
class IntrinsicSizeDrawable : public Drawable {
public:
    int mW;
    int mH;
    explicit IntrinsicSizeDrawable(int w, int h) : mW(w), mH(h) {}
    void draw(Canvas&) override {}
    int getOpacity() const override { return PixelFormat::OPAQUE; }
    int getIntrinsicWidth() override { return mW; }
    int getIntrinsicHeight() override { return mH; }
};

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

class CtsLayerDrawableTest : public testing::Test {};

TEST_F(CtsLayerDrawableTest, testConstructor) {
    ColorDrawable* a = new ColorDrawable(0xFF0000FF);
    ColorDrawable* b = new ColorDrawable(0xFFFF0000);
    LayerDrawable ld(std::vector<Drawable*>{a, b});
    EXPECT_EQ(2, ld.getNumberOfLayers());
    EXPECT_EQ(a, ld.getDrawable(0));
    EXPECT_EQ(b, ld.getDrawable(1));
    // Unset layer IDs default to NO_ID (-1).
    EXPECT_EQ((int)View::NO_ID, ld.getId(0));
    EXPECT_EQ((int)View::NO_ID, ld.getId(1));

    LayerDrawable empty(std::vector<Drawable*>{});
    EXPECT_EQ(0, empty.getNumberOfLayers());
}

TEST_F(CtsLayerDrawableTest, testGetNumberOfLayers) {
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFF111111), new ColorDrawable(0xFF222222)});
    EXPECT_EQ(2, ld.getNumberOfLayers());

    std::vector<Drawable*> five;
    for (int i = 0; i < 5; i++) five.push_back(new ColorDrawable(0xFF000000 | i));
    LayerDrawable ld2(five);
    EXPECT_EQ(5, ld2.getNumberOfLayers());

    LayerDrawable ld3(std::vector<Drawable*>{});
    EXPECT_EQ(0, ld3.getNumberOfLayers());
}

TEST_F(CtsLayerDrawableTest, testAccessId) {
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFF111111), new ColorDrawable(0xFF222222)});
    ld.setId(0, 10);
    ld.setId(1, 20);
    EXPECT_EQ(10, ld.getId(0));
    EXPECT_EQ(20, ld.getId(1));

    ld.setId(0, INT_MIN);
    ld.setId(1, INT_MAX);
    EXPECT_EQ(INT_MIN, ld.getId(0));
    EXPECT_EQ(INT_MAX, ld.getId(1));
}

// CTS expects IndexOutOfBoundsException for getId(out-of-range). CDROID's getId returns -1 instead
// (the codebase comment explicitly documents this divergence at layerdrawable.cc getId). The
// setId/getId OOB throw cases are therefore ported as -1 / not ported (setId uses unchecked []).
TEST_F(CtsLayerDrawableTest, testGetIdOutOfBoundsReturnsSentinel) {
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFF111111), new ColorDrawable(0xFF222222)});
    EXPECT_EQ(-1, ld.getId(-1));
    EXPECT_EQ(-1, ld.getId(ld.getNumberOfLayers()));
}

TEST_F(CtsLayerDrawableTest, testFindDrawableByLayerId) {
    ColorDrawable* a = new ColorDrawable(0xFF111111);
    ColorDrawable* b = new ColorDrawable(0xFF222222);
    LayerDrawable ld(std::vector<Drawable*>{a, b});

    ld.setId(0, 10);
    ld.setId(1, 20);
    EXPECT_EQ(a, ld.findDrawableByLayerId(10));
    EXPECT_EQ(b, ld.findDrawableByLayerId(20));
    EXPECT_EQ(nullptr, ld.findDrawableByLayerId(30));

    ld.setId(0, INT_MIN);
    ld.setId(1, INT_MAX);
    EXPECT_EQ(a, ld.findDrawableByLayerId(INT_MIN));
    EXPECT_EQ(b, ld.findDrawableByLayerId(INT_MAX));
    // The duplicate-id sub-case is omitted: CDROID returns the FIRST match whereas CTS expects the
    // last-set layer to win.
}

TEST_F(CtsLayerDrawableTest, testFindIndexByLayerId) {
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFF111111), new ColorDrawable(0xFF222222)});
    ld.setId(0, 10);
    ld.setId(1, 20);
    EXPECT_EQ(0, ld.findIndexByLayerId(10));
    EXPECT_EQ(1, ld.findIndexByLayerId(20));
    EXPECT_EQ(-1, ld.findIndexByLayerId(30));
}

TEST_F(CtsLayerDrawableTest, testAccessDrawable) {
    ColorDrawable* a = new ColorDrawable(0xFF111111);
    ColorDrawable* b = new ColorDrawable(0xFF222222);
    LayerDrawable ld(std::vector<Drawable*>{a, b});
    EXPECT_EQ(a, ld.getDrawable(0));
    EXPECT_EQ(b, ld.getDrawable(1));

    ld.setId(0, 10);
    ld.setId(1, 20);
    ColorDrawable* d1 = new ColorDrawable(0xFF00FF00);
    ColorDrawable* d2 = new ColorDrawable(0xFF0000FF);
    EXPECT_TRUE(ld.setDrawableByLayerId(10, d1));
    EXPECT_TRUE(ld.setDrawableByLayerId(20, d2));
    EXPECT_EQ(d1, ld.getDrawable(0));
    EXPECT_EQ(d2, ld.getDrawable(1));

    // Unknown layer id ⇒ setDrawableByLayerId returns false.
    EXPECT_FALSE(ld.setDrawableByLayerId(30, d1));

    // CDROID getDrawable(out-of-range) returns nullptr (CTS throws IndexOutOfBoundsException).
    EXPECT_EQ(nullptr, ld.getDrawable(ld.getNumberOfLayers()));
    EXPECT_EQ(nullptr, ld.getDrawable(-1));
}

TEST_F(CtsLayerDrawableTest, testSetDrawableByLayerId) {
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFFFF0000), new ColorDrawable(0xFF0000FF)});
    ld.setId(0, 10);
    ld.setId(1, 20);

    ColorDrawable* layer1B = new ColorDrawable(0xFF00FF00);
    layer1B->setLevel(10000);
    ColorDrawable* layer2B = new ColorDrawable(0xFFFFFF00);
    layer2B->setLevel(5000);
    ld.setDrawableByLayerId(10, layer1B);
    ld.setDrawableByLayerId(20, layer2B);

    // Level is preserved on the replacement drawable.
    EXPECT_EQ(10000, ld.findDrawableByLayerId(10)->getLevel());
    EXPECT_EQ(5000, ld.findDrawableByLayerId(20)->getLevel());
}

TEST_F(CtsLayerDrawableTest, testGetDrawable) {
    ColorDrawable* a = new ColorDrawable(0xFF111111);
    ColorDrawable* b = new ColorDrawable(0xFF222222);
    LayerDrawable ld(std::vector<Drawable*>{a, b});
    EXPECT_EQ(a, ld.getDrawable(0));
    EXPECT_EQ(b, ld.getDrawable(1));
    // CDROID returns nullptr for out-of-range (CTS throws).
    EXPECT_EQ(nullptr, ld.getDrawable(2));
}

TEST_F(CtsLayerDrawableTest, testSetDrawable) {
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFF111111), new ColorDrawable(0xFF222222)});
    ColorDrawable* newColor = new ColorDrawable(0xFF00FF00);
    ColorDrawable* otherColor = new ColorDrawable(0xFF0000FF);
    ld.setDrawable(0, newColor);
    ld.setDrawable(1, otherColor);

    EXPECT_EQ(2, ld.getNumberOfLayers());
    EXPECT_EQ(newColor, ld.getDrawable(0));
    EXPECT_EQ(otherColor, ld.getDrawable(1));
    EXPECT_EQ(nullptr, ld.getDrawable(2));
}

TEST_F(CtsLayerDrawableTest, testAddLayer) {
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFF111111), new ColorDrawable(0xFF222222)});
    ColorDrawable* added = new ColorDrawable(0xFF00FF00);
    const int index = ld.addLayer(added);

    EXPECT_EQ(index, ld.getNumberOfLayers() - 1);
    EXPECT_EQ(added, ld.getDrawable(index));
}

TEST_F(CtsLayerDrawableTest, testSetLayerInset) {
    // Layer 0 has intrinsic 10x10; layer 1 has none (-1) so it does not contribute.
    LayerDrawable ld(std::vector<Drawable*>{new IntrinsicSizeDrawable(10, 10), new MockDrawable()});
    const int left = 10, top = 20, right = 30, bottom = 40;
    ld.setLayerInset(0, left, top, right, bottom);
    EXPECT_EQ(ld.getDrawable(0)->getIntrinsicWidth() + left + right, ld.getIntrinsicWidth());
    EXPECT_EQ(ld.getDrawable(0)->getIntrinsicHeight() + top + bottom, ld.getIntrinsicHeight());

    // A child with no intrinsic size does not expand the aggregate.
    ld.setLayerInset(1, 10, 10, 10, 10);
    EXPECT_EQ(ld.getDrawable(0)->getIntrinsicWidth() + left + right, ld.getIntrinsicWidth());
    EXPECT_EQ(ld.getDrawable(0)->getIntrinsicHeight() + top + bottom, ld.getIntrinsicHeight());
}

TEST_F(CtsLayerDrawableTest, testSetLayerInsetRelative) {
    LayerDrawable ld(std::vector<Drawable*>{new IntrinsicSizeDrawable(10, 10), new MockDrawable()});
    const int start = 10, top = 20, end = 30, bottom = 40;
    ld.setLayerInsetRelative(0, start, top, end, bottom);
    EXPECT_EQ(ld.getDrawable(0)->getIntrinsicWidth() + start + end, ld.getIntrinsicWidth());
    EXPECT_EQ(ld.getDrawable(0)->getIntrinsicHeight() + top + bottom, ld.getIntrinsicHeight());
    EXPECT_EQ(start, ld.getLayerInsetStart(0));
    EXPECT_EQ(top, ld.getLayerInsetTop(0));
    EXPECT_EQ(end, ld.getLayerInsetEnd(0));
    EXPECT_EQ(bottom, ld.getLayerInsetBottom(0));
    // Absolute left/right stay at 0 when only relative insets are set.
    EXPECT_EQ(0, ld.getLayerInsetLeft(0));
    EXPECT_EQ(0, ld.getLayerInsetRight(0));

    ld.setLayerInsetRelative(1, 10, 10, 10, 10);
    EXPECT_EQ(ld.getDrawable(0)->getIntrinsicWidth() + start + end, ld.getIntrinsicWidth());
}

TEST_F(CtsLayerDrawableTest, testAccessLayerInsets) {
    // Round-trip each per-side inset setter/getter pair via setLayerInsetLeft/Top/Right/Bottom and
    // setLayerInsetStart/End (the OOB try/catch clauses are skipped — CDROID uses unchecked []).
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFF111111)});
    ld.setLayerInset(0, 10, 20, 30, 40);
    EXPECT_EQ(10, ld.getLayerInsetLeft(0));
    EXPECT_EQ(20, ld.getLayerInsetTop(0));
    EXPECT_EQ(30, ld.getLayerInsetRight(0));
    EXPECT_EQ(40, ld.getLayerInsetBottom(0));

    ld.setLayerInsetLeft(0, 15);
    EXPECT_EQ(15, ld.getLayerInsetLeft(0));
    ld.setLayerInsetTop(0, 25);
    EXPECT_EQ(25, ld.getLayerInsetTop(0));
    ld.setLayerInsetRight(0, 35);
    EXPECT_EQ(35, ld.getLayerInsetRight(0));
    ld.setLayerInsetBottom(0, 45);
    EXPECT_EQ(45, ld.getLayerInsetBottom(0));

    ld.setLayerInsetRelative(0, 10, 20, 30, 40);
    EXPECT_EQ(10, ld.getLayerInsetStart(0));
    EXPECT_EQ(30, ld.getLayerInsetEnd(0));
    ld.setLayerInsetStart(0, 50);
    EXPECT_EQ(50, ld.getLayerInsetStart(0));
    ld.setLayerInsetEnd(0, 60);
    EXPECT_EQ(60, ld.getLayerInsetEnd(0));
}

TEST_F(CtsLayerDrawableTest, testSetLayerGravity) {
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFF111111), new ColorDrawable(0xFF222222)});
    ld.setLayerGravity(0, Gravity::CENTER);
    ld.setLayerGravity(1, Gravity::NO_GRAVITY);
    EXPECT_EQ((int)Gravity::CENTER, ld.getLayerGravity(0));
    EXPECT_EQ((int)Gravity::NO_GRAVITY, ld.getLayerGravity(1));
    // setLayerGravity/getLayerGravity use at() → throw std::out_of_range on out-of-range index.
    EXPECT_ANY_THROW(ld.setLayerGravity(2, Gravity::TOP));
    EXPECT_ANY_THROW(ld.getLayerGravity(2));
}

TEST_F(CtsLayerDrawableTest, testAccessLayerWidthHeightSize) {
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFF111111), new ColorDrawable(0xFF222222)});

    ld.setLayerWidth(0, 100);
    ld.setLayerWidth(1, 200);
    EXPECT_EQ(100, ld.getLayerWidth(0));
    EXPECT_EQ(200, ld.getLayerWidth(1));
    EXPECT_ANY_THROW(ld.setLayerWidth(2, 300));
    EXPECT_ANY_THROW(ld.getLayerWidth(2));

    ld.setLayerHeight(0, 100);
    ld.setLayerHeight(1, 200);
    EXPECT_EQ(100, ld.getLayerHeight(0));
    EXPECT_EQ(200, ld.getLayerHeight(1));
    EXPECT_ANY_THROW(ld.setLayerHeight(2, 300));
    EXPECT_ANY_THROW(ld.getLayerHeight(2));

    ld.setLayerSize(0, 11, 22);
    ld.setLayerSize(1, 33, 44);
    EXPECT_EQ(11, ld.getLayerWidth(0));
    EXPECT_EQ(22, ld.getLayerHeight(0));
    EXPECT_EQ(33, ld.getLayerWidth(1));
    EXPECT_EQ(44, ld.getLayerHeight(1));
    EXPECT_ANY_THROW(ld.setLayerSize(2, 5, 6));
}

TEST_F(CtsLayerDrawableTest, testSetPadding) {
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFF111111)});
    ld.setPadding(10, 11, 20, 21);
    EXPECT_EQ(10, ld.getLeftPadding());
    EXPECT_EQ(11, ld.getTopPadding());
    EXPECT_EQ(20, ld.getRightPadding());
    EXPECT_EQ(21, ld.getBottomPadding());
    // Absolute padding leaves the relative (start/end) paddings unset (-1).
    EXPECT_EQ(-1, ld.getStartPadding());
    EXPECT_EQ(-1, ld.getEndPadding());
}

TEST_F(CtsLayerDrawableTest, testSetPaddingRelative) {
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFF111111)});
    ld.setPaddingRelative(10, 11, 20, 21);
    EXPECT_EQ(10, ld.getStartPadding());
    EXPECT_EQ(11, ld.getTopPadding());
    EXPECT_EQ(20, ld.getEndPadding());
    EXPECT_EQ(21, ld.getBottomPadding());
    EXPECT_EQ(-1, ld.getLeftPadding());
    EXPECT_EQ(-1, ld.getRightPadding());
}

// LayerDrawable::invalidateDrawable/scheduleDrawable/unscheduleDrawable take Drawable& (a reference),
// so CTS's null-argument sub-cases cannot be expressed; only the non-null branches are ported. Unlike
// DrawableContainer, LayerDrawable forwards every child callback to the host callback unconditionally.
TEST_F(CtsLayerDrawableTest, testInvalidateDrawable) {
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFF111111), new ColorDrawable(0xFF222222)});
    MockCallback cb;
    ld.setCallback(&cb);

    ColorDrawable other(0xFF00FF00);
    ld.invalidateDrawable(other);
    EXPECT_EQ(1, cb.invalidateCount);

    ld.invalidateDrawable(*ld.getDrawable(0));
    EXPECT_EQ(2, cb.invalidateCount);

    // No callback: must not throw.
    ld.setCallback(nullptr);
    ld.invalidateDrawable(other);
}

TEST_F(CtsLayerDrawableTest, testScheduleDrawable) {
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFF111111), new ColorDrawable(0xFF222222)});
    MockCallback cb;
    ld.setCallback(&cb);
    Runnable r = [] {};

    ColorDrawable other(0xFF00FF00);
    ld.scheduleDrawable(other, r, 1000);
    EXPECT_EQ(1, cb.scheduleCount);

    ld.setCallback(nullptr);
    ld.scheduleDrawable(other, r, 0);
    EXPECT_EQ(1, cb.scheduleCount);
}

TEST_F(CtsLayerDrawableTest, testUnscheduleDrawable) {
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFF111111), new ColorDrawable(0xFF222222)});
    MockCallback cb;
    ld.setCallback(&cb);
    Runnable r = [] {};

    ColorDrawable other(0xFF00FF00);
    ld.unscheduleDrawable(other, r);
    EXPECT_EQ(1, cb.unscheduleCount);

    ld.setCallback(nullptr);
    ld.unscheduleDrawable(other, r);
    EXPECT_EQ(1, cb.unscheduleCount);
}

TEST_F(CtsLayerDrawableTest, testSetVisible) {
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFF111111), new ColorDrawable(0xFF222222)});

    EXPECT_TRUE(ld.setVisible(false, true));
    EXPECT_FALSE(ld.isVisible());
    EXPECT_FALSE(ld.getDrawable(0)->isVisible());
    EXPECT_FALSE(ld.getDrawable(1)->isVisible());

    EXPECT_FALSE(ld.setVisible(false, false));

    EXPECT_TRUE(ld.setVisible(true, false));
    EXPECT_TRUE(ld.isVisible());
    EXPECT_TRUE(ld.getDrawable(0)->isVisible());
    EXPECT_TRUE(ld.getDrawable(1)->isVisible());
}

TEST_F(CtsLayerDrawableTest, testSetHotspotBounds) {
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFF111111), new ColorDrawable(0xFF222222)});
    // CDROID setHotspotBounds(l,t,w,h) (CTS passes l,t,r,b — the four ints round-trip identically).
    ld.setHotspotBounds(10, 15, 100, 150);
    Rect out;
    ld.getHotspotBounds(out);
    EXPECT_EQ(10, out.left);
    EXPECT_EQ(15, out.top);
    EXPECT_EQ(100, out.width);
    EXPECT_EQ(150, out.height);
}

TEST_F(CtsLayerDrawableTest, testGetHotspotBounds) {
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFF111111), new ColorDrawable(0xFF222222)});
    ld.setHotspotBounds(10, 15, 100, 150);
    Rect out;
    ld.getHotspotBounds(out);
    EXPECT_EQ(10, out.left);
    EXPECT_EQ(15, out.top);
    EXPECT_EQ(100, out.width);
    EXPECT_EQ(150, out.height);
}

TEST_F(CtsLayerDrawableTest, testAccessOpacity) {
    // No children ⇒ transparent.
    LayerDrawable empty(std::vector<Drawable*>{});
    EXPECT_EQ((int)PixelFormat::TRANSPARENT, empty.getOpacity());

    // All-opaque children ⇒ opaque.
    LayerDrawable ld(std::vector<Drawable*>{new MockDrawable(PixelFormat::OPAQUE), new MockDrawable(PixelFormat::OPAQUE)});
    EXPECT_EQ((int)PixelFormat::OPAQUE, ld.getOpacity());

    // resolveOpacity(OPAQUE, TRANSPARENT) ⇒ TRANSPARENT.
    LayerDrawable ld2(std::vector<Drawable*>{new MockDrawable(PixelFormat::OPAQUE), new MockDrawable(PixelFormat::TRANSPARENT)});
    EXPECT_EQ((int)PixelFormat::TRANSPARENT, ld2.getOpacity());

    // resolveOpacity(TRANSLUCENT, TRANSPARENT) ⇒ TRANSLUCENT.
    LayerDrawable ld3(std::vector<Drawable*>{new MockDrawable(PixelFormat::TRANSLUCENT), new MockDrawable(PixelFormat::TRANSPARENT)});
    EXPECT_EQ((int)PixelFormat::TRANSLUCENT, ld3.getOpacity());

    // resolveOpacity(TRANSLUCENT, UNKNOWN) ⇒ UNKNOWN.
    LayerDrawable ld4(std::vector<Drawable*>{new MockDrawable(PixelFormat::TRANSLUCENT), new MockDrawable(PixelFormat::UNKNOWN)});
    EXPECT_EQ((int)PixelFormat::UNKNOWN, ld4.getOpacity());

    // setOpacity override wins regardless of children.
    LayerDrawable ld5(std::vector<Drawable*>{new MockDrawable(PixelFormat::TRANSLUCENT), new MockDrawable(PixelFormat::UNKNOWN)});
    ld5.setOpacity(PixelFormat::OPAQUE);
    EXPECT_EQ((int)PixelFormat::OPAQUE, ld5.getOpacity());
}

TEST_F(CtsLayerDrawableTest, testIsStateful) {
    LayerDrawable empty(std::vector<Drawable*>{});
    EXPECT_FALSE(empty.isStateful());

    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFF111111), new MockDrawable(PixelFormat::OPAQUE, /*stateful=*/false)});
    EXPECT_FALSE(ld.isStateful());

    LayerDrawable ld2(std::vector<Drawable*>{new ColorDrawable(0xFF111111), new StateListDrawable()});
    EXPECT_TRUE(ld2.isStateful());
}

TEST_F(CtsLayerDrawableTest, testGetConstantState) {
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFF111111), new ColorDrawable(0xFF222222)});
    auto cs = ld.getConstantState();
    EXPECT_NE(nullptr, cs);
    EXPECT_EQ(0, cs->getChangingConfigurations());

    ld.setChangingConfigurations(1);
    cs = ld.getConstantState();
    EXPECT_NE(nullptr, cs);
    EXPECT_EQ(1, cs->getChangingConfigurations());
}

TEST_F(CtsLayerDrawableTest, testChildIntrinsicSize) {
    // A child with no intrinsic size (ColorDrawable reports -1/-1 in CDROID), even with insets,
    // leaves the LayerDrawable's intrinsic size at -1.
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFFFF0000)});
    ld.setLayerInset(0, 10, 10, 10, 10);
    EXPECT_EQ(-1, ld.getIntrinsicWidth());
    EXPECT_EQ(-1, ld.getIntrinsicHeight());
}

TEST_F(CtsLayerDrawableTest, testIsProjectedWithNullLayer) {
    // A null child layer must not crash isProjected (CDROID, like Android, tolerates null children).
    LayerDrawable ld(std::vector<Drawable*>{nullptr});
    ld.isProjected();
    SUCCEED();
}

TEST_F(CtsLayerDrawableTest, testMutate) {
    LayerDrawable ld(std::vector<Drawable*>{new ColorDrawable(0xFF111111), new ColorDrawable(0xFF222222)});
    // mutate() returns the drawable itself and must not throw.
    EXPECT_EQ(&ld, ld.mutate());
}
