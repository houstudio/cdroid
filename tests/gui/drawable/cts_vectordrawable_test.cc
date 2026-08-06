// AOSP CTS drawable test port (VectorDrawableTest.java + VectorDrawableSizeTest.java). Pure-logic
// cases only — all the golden-image pixel comparisons (testBasicVectorDrawables /
// testLMVectorDrawables / testNVectorDrawables / testVectorDrawableGradient / testColorStateList)
// and the density preload cases (testPreloadDensity / testPreloadDensity_tvdpi) are NOT ported:
// CDROID renders through Cairo (pixels diverge from Skia) and the test's Theme/DisplayMetrics
// density flipping is not 1:1 reachable here. Mutate / constant-state / changing-configurations /
// color-filter / opacity cases are ported verbatim using the cts_vector_icon asset; the
// VectorDrawableSizeTest parameterized cases (round() vs floor() at the host density) are merged
// into this file as testVectorDrawableSize_*.
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/VectorDrawableTest.java (Apache 2.0)
//           cts/tests/tests/graphics/src/android/graphics/drawable/cts/VectorDrawableSizeTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <drawable/vectordrawable.h>
#include <drawable/drawables.h>
#include <drawable/colorfilters.h>
#include <core/app.h>
#include <core/porterduff.h>
#include <core/displaymetrics.h>
#include <guienvironment.h>
#include <cmath>

using namespace cdroid;

namespace {
// Resolves the host's densityDpi (set by App from the porting backend; defaults to
// DisplayMetrics::DENSITY_DEFAULT == 160 in the headless test environment). Used by the
// VectorDrawableSizeTest cases to mirror CTS's Math.round(dp * densityDpi / 160f).
int hostDensityDpi() {
    return App::getInstance().getDisplayMetrics().densityDpi;
}

// Casts the cached, resource-loaded drawable to a VectorDrawable. getDrawable() returns a
// borrowed (cached) instance — do not delete.
VectorDrawable* loadVectorDrawable(const std::string& ref) {
    Drawable* d = App::getInstance().getDrawable(ref);
    return dynamic_cast<VectorDrawable*>(d);
}
} // namespace

class CtsVectorDrawableTest : public testing::Test {};

TEST_F(CtsVectorDrawableTest, testGetChangingConfigurations) {
    VectorDrawable vectorDrawable;
    auto constantState = vectorDrawable.getConstantState();

    // default
    ASSERT_NE(nullptr, constantState);
    EXPECT_EQ(0, constantState->getChangingConfigurations());
    EXPECT_EQ(0, vectorDrawable.getChangingConfigurations());

    // changing the drawable's configuration does not affect the cached state's snapshot
    vectorDrawable.setChangingConfigurations(0xff);
    EXPECT_EQ(0xff, vectorDrawable.getChangingConfigurations());
    EXPECT_EQ(0, constantState->getChangingConfigurations());

    // re-fetching the constant state reflects the new value
    constantState = vectorDrawable.getConstantState();
    EXPECT_EQ(0xff, constantState->getChangingConfigurations());

    // set a new configuration to the drawable; drawable ORs with the state's value
    vectorDrawable.setChangingConfigurations(0xff00);
    EXPECT_EQ(0xff, constantState->getChangingConfigurations());
    EXPECT_EQ(0xffff, vectorDrawable.getChangingConfigurations());
}

TEST_F(CtsVectorDrawableTest, testGetConstantState) {
    VectorDrawable vectorDrawable;
    auto constantState = vectorDrawable.getConstantState();
    ASSERT_NE(nullptr, constantState);
    EXPECT_EQ(0, constantState->getChangingConfigurations());

    vectorDrawable.setChangingConfigurations(1);
    constantState = vectorDrawable.getConstantState();
    ASSERT_NE(nullptr, constantState);
    EXPECT_EQ(1, constantState->getChangingConfigurations());
}

TEST_F(CtsVectorDrawableTest, testMutate) {
    // CTS loads vector_icon_create three times from the resource cache so d1/d2/d3 share
    // constant state, then mutates d1/d2 to verify the shared-state copy-on-write semantics.
    // CDROID's getDrawable() returns cached, shared-constant-state drawables the same way.
    VectorDrawable* d1 = loadVectorDrawable("@drawable/cts_vector_icon");
    VectorDrawable* d2 = loadVectorDrawable("@drawable/cts_vector_icon");
    VectorDrawable* d3 = loadVectorDrawable("@drawable/cts_vector_icon");
    ASSERT_NE(nullptr, d1);
    ASSERT_NE(nullptr, d2);
    ASSERT_NE(nullptr, d3);
    const int initialAlpha = d1->getAlpha();

    d1->mutate();
    d1->setAlpha(0x40);
    EXPECT_EQ(0x40, d1->getAlpha());
    EXPECT_EQ(initialAlpha, d2->getAlpha());
    EXPECT_EQ(initialAlpha, d3->getAlpha());

    d2->mutate();
    d2->setAlpha(0x20);
    EXPECT_EQ(0x40, d1->getAlpha());
    EXPECT_EQ(0x20, d2->getAlpha());
    EXPECT_EQ(initialAlpha, d3->getAlpha());
}

TEST_F(CtsVectorDrawableTest, testMutatePreservesState) {
    // CTS asserts that alpha set BEFORE mutate() survives the mutate (state is copied, not
    // reset). The cached drawable's alpha is left untouched by the resource cache restoration
    // since cts_vector_icon has no explicit alpha.
    VectorDrawable* d = loadVectorDrawable("@drawable/cts_vector_icon");
    ASSERT_NE(nullptr, d);
    EXPECT_NE(0x00, d->getAlpha());

    d->setAlpha(0x00);
    d->mutate();
    EXPECT_EQ(0x00, d->getAlpha());
}

TEST_F(CtsVectorDrawableTest, testColorFilter) {
    cdroid::RefPtr<ColorFilter> filter =
        std::make_shared<PorterDuffColorFilter>(0xFFFF0000, PorterDuff::SRC_IN);
    VectorDrawable vectorDrawable;
    vectorDrawable.setColorFilter(filter);
    EXPECT_EQ(filter.get(), vectorDrawable.getColorFilter().get());
}

TEST_F(CtsVectorDrawableTest, testGetOpacity) {
    VectorDrawable vectorDrawable;
    EXPECT_EQ(255, vectorDrawable.getAlpha());
    EXPECT_EQ((int)PixelFormat::TRANSLUCENT, vectorDrawable.getOpacity());

    vectorDrawable.setAlpha(0);
    EXPECT_EQ(0, vectorDrawable.getAlpha());
    EXPECT_EQ((int)PixelFormat::TRANSPARENT, vectorDrawable.getOpacity());
}

TEST_F(CtsVectorDrawableTest, testOpticalInsets) {
    // CTS loads vector_icon_create (whose XML declares opticalInset 1/2/3/4) and asserts
    // Insets.of(1,2,3,4) is read back. cts_vector_icon.xml carries the same four px insets;
    // at the test env's default density (source == target == 160) no scaling is applied.
    VectorDrawable* d = loadVectorDrawable("@drawable/cts_vector_icon");
    ASSERT_NE(nullptr, d);
    EXPECT_EQ(Insets::of(1, 2, 3, 4), d->getOpticalInsets());
}

TEST_F(CtsVectorDrawableTest, testIntrinsicSizeFromResource) {
    // cts_vector_icon declares width=24dp / height=24dp. At the host density the intrinsic
    // size is round(24 * densityDpi / 160) — verify both axes report that.
    VectorDrawable* d = loadVectorDrawable("@drawable/cts_vector_icon");
    ASSERT_NE(nullptr, d);
    const int expected = (int)std::lround(24.0f * hostDensityDpi() / 160.0f);
    EXPECT_EQ(expected, d->getIntrinsicWidth());
    EXPECT_EQ(expected, d->getIntrinsicHeight());
}

// --- VectorDrawableSizeTest (parameterized in CTS) merged as plain TEST_F cases ---

// CTS vector_icon_size_1: width=7dp. Verifies VectorDrawable uses round(), not floor(), when
// scaling dp to pixels at the host density.
TEST_F(CtsVectorDrawableTest, testVectorDrawableSize_size_1) {
    VectorDrawable* d = loadVectorDrawable("@drawable/cts_vector_size_1");
    ASSERT_NE(nullptr, d);
    const int expected = (int)std::lround(7 * hostDensityDpi() / 160.0f);
    EXPECT_EQ(expected, d->getIntrinsicWidth());
    EXPECT_EQ(expected, d->getIntrinsicHeight());
}

// CTS vector_icon_size_2: width=9dp.
TEST_F(CtsVectorDrawableTest, testVectorDrawableSize_size_2) {
    VectorDrawable* d = loadVectorDrawable("@drawable/cts_vector_size_2");
    ASSERT_NE(nullptr, d);
    const int expected = (int)std::lround(9 * hostDensityDpi() / 160.0f);
    EXPECT_EQ(expected, d->getIntrinsicWidth());
    EXPECT_EQ(expected, d->getIntrinsicHeight());
}

// Skipped (pixel/density/Theme-dependent, see header):
//   testBasicVectorDrawables / testLMVectorDrawables / testNVectorDrawables
//     — golden-image pixel comparison; Cairo != Skia.
//   testVectorDrawableGradient            — golden-image pixel comparison.
//   testColorStateList                    — golden-image pixel comparison across state sets.
//   testPreloadDensity / testPreloadDensity_tvdpi
//     — needs density flipping on Resources + Theme.applyTheme density re-resolution.
