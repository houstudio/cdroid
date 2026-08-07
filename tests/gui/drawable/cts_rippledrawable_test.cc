// AOSP CTS drawable test port (RippleDrawableTest.java). All cases except testPreloadDensity are
// ported. testPreloadDensity needs Resources density flipping + Theme density re-resolution, which
// is not 1:1 reachable in this env (same carve-out as CtsVectorDrawableTest). Constructor / radius
// get-set / radius-from-XML / setColor→invalidate / effectColor get-set / effectColor-from-XML are
// ported verbatim. The CTS Mockito Drawable.Callback is replaced by a hand-written callback that
// counts invalidateDrawable (same idiom as the other cts_*_drawable tests).
//
// Note: RippleDrawable.RADIUS_AUTO is private in CDROID's port (AOSP exposes it publicly); the
// sentinel value (-1) is redeclared locally. getEffectColor/setEffectColor were added to CDROID's
// RippleDrawable as part of porting this test (DEFAULT_EFFECT_COLOR == 0x8dffffff, per AOSP).
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/RippleDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <drawable/rippledrawable.h>
#include <drawable/colorstatelist.h>
#include <drawable/colordrawable.h>
#include <core/app.h>
#include <core/color.h>
#include <memory>

using namespace cdroid;

namespace {
constexpr int COLOR_RED   = 0xFFFF0000;
constexpr int COLOR_BLUE  = 0xFF0000FF;
constexpr int COLOR_YELLOW = 0xFFFFFF00;
constexpr int COLOR_BLACK = 0xFF000000;
// RippleDrawable.RADIUS_AUTO is private in CDROID; reuse the literal sentinel.
constexpr int RADIUS_AUTO = -1;
// RippleDrawable.DEFAULT_EFFECT_COLOR (AOSP: 0x8dffffff) — the default effect color returned by
// getEffectColor() when none was set/inflated.
constexpr int DEFAULT_EFFECT_COLOR = 0x8dffffff;

// Hand-written Drawable.Callback that counts invalidateDrawable (CTS uses Mockito.verify with
// times(1)). Same pattern as CtsDrawableTest::MockCallback / CtsColorStateListDrawableTest.
class CountingCallback : public Drawable::Callback {
public:
    int invalidateCount = 0;
    void invalidateDrawable(Drawable&) override { invalidateCount++; }
    void scheduleDrawable(Drawable&, const Runnable&, int64_t) override {}
    void unscheduleDrawable(Drawable&, const Runnable&) override {}
};

// Loads a cached, resource-inflated RippleDrawable. getDrawable() returns a borrowed (cached)
// instance — do not delete.
RippleDrawable* loadRippleDrawable(const std::string& ref) {
    return dynamic_cast<RippleDrawable*>(App::getInstance().getDrawable(ref));
}
} // namespace

class CtsRippleDrawableTest : public testing::Test {};

TEST_F(CtsRippleDrawableTest, testConstructor) {
    RippleDrawable drawable(ColorStateList::valueOf(COLOR_RED), nullptr, nullptr);
}

TEST_F(CtsRippleDrawableTest, testAccessRadius) {
    RippleDrawable drawable(ColorStateList::valueOf(COLOR_RED), nullptr, nullptr);
    EXPECT_EQ(RADIUS_AUTO, drawable.getRadius());
    drawable.setRadius(10);
    EXPECT_EQ(10, drawable.getRadius());
}

TEST_F(CtsRippleDrawableTest, testRadiusAttr) {
    RippleDrawable* drawable = loadRippleDrawable("@drawable/cts_ripple_radius");
    ASSERT_NE(nullptr, drawable);
    EXPECT_EQ(10, drawable->getRadius());
}

TEST_F(CtsRippleDrawableTest, testSetColor) {
    CountingCallback cb;
    RippleDrawable dr(ColorStateList::valueOf(COLOR_RED), nullptr, nullptr);
    dr.setCallback(&cb);

    dr.setColor(ColorStateList::valueOf(COLOR_BLACK));
    EXPECT_EQ(1, cb.invalidateCount);
}

TEST_F(CtsRippleDrawableTest, testEffectColor) {
    RippleDrawable drawable(ColorStateList::valueOf(COLOR_RED), nullptr, nullptr);
    // Default effect color is DEFAULT_EFFECT_COLOR (0x8dffffff), not the constructor ripple color.
    EXPECT_EQ(DEFAULT_EFFECT_COLOR, drawable.getEffectColor()->getDefaultColor());
    drawable.setEffectColor(ColorStateList::valueOf(COLOR_BLUE));
    EXPECT_EQ(COLOR_BLUE, drawable.getEffectColor()->getDefaultColor());
}

TEST_F(CtsRippleDrawableTest, testEffectColorInflation) {
    RippleDrawable* drawable = loadRippleDrawable("@drawable/cts_ripple_effect");
    ASSERT_NE(nullptr, drawable);
    EXPECT_EQ(COLOR_YELLOW, drawable->getEffectColor()->getDefaultColor());
}

// Skipped (see header):
//   testPreloadDensity          — needs Resources density flipping + Theme density re-resolution
//                                  (not 1:1 reachable; same carve-out as CtsVectorDrawableTest).
