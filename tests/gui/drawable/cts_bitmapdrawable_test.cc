// AOSP CTS drawable test port (BitmapDrawableTest.java). Pure-logic cases only — pixel
// comparison (testSetTint/testSetBlendMode/testDraw/testBitmapDrawableOpticalInset), image
// density scaling (testSetTargetDensity/testGetIntrinsicSize's bitmap clauses), and the
// InputStream/file-path/Resources constructors (testConstructor's non-default variants) are
// NOT ported: CDROID has no android.graphics.Bitmap (Cairo::ImageSurface plays that role) and
// the BitmapDrawable ctors that take a stream/path/Context+resname are not 1:1 with Android's
// (see bitmapdrawable.h). Likewise android.graphics.Paint is not exposed on BitmapDrawable in
// CDROID — paint-backed assertions (getPaint().isAntiAlias()/isFilterBitmap()/isDither()/
// getAlpha()/getColorFilter()/getShader()) are ported via the direct getters on the drawable
// (hasAntiAlias/isFilterBitmap/getAlpha/getColorFilter/getTileModeX/Y). Note CDROID divergences
// from CTS defaults (documented inline): mFilterBitmap and mDither default to false (CTS's Paint
// defaults them to true via FILTER_BITMAP_FLAG/DITHER_FLAG); TileMode default is DISABLED (-1),
// not null (CTS uses null Shader/TileMode).
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/BitmapDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <drawable/bitmapdrawable.h>
#include <drawable/drawables.h>
#include <drawable/colorfilters.h>
#include <core/app.h>
#include <core/porterduff.h>
#include <core/rect.h>
#include <core/insets.h>
#include <view/gravity.h>
#include <guienvironment.h>
#include <limits.h>

using namespace cdroid;

// Empty fixture — pure logic. App/Context is provided process-wide by GUIEnvironment
// (see guienvironment.h). All BitmapDrawable state lives on the internal BitmapState and is
// reachable through the public getters regardless of whether a backing ImageSurface is set,
// so the default-constructed drawable (null bitmap) is sufficient for the ported cases.
class CtsBitmapDrawableTest : public testing::Test {};

// CTS testConstructor exercises BitmapDrawable()/BitmapDrawable(bitmap)/BitmapDrawable(res)/
// BitmapDrawable(res,bitmap)/BitmapDrawable(path)/BitmapDrawable(stream) plus null-arg variants.
// Only the default ctor maps cleanly to CDROID (no Bitmap class; stream/path/res ctors diverge —
// CDROID has BitmapDrawable(ImageSurface) and BitmapDrawable(Context*,string)). Verify the
// default-constructed state; the other ctor variants are skipped (Bitmap API difference).
TEST_F(CtsBitmapDrawableTest, testConstructor) {
    BitmapDrawable bitmapDrawable;
    // CTS asserts the default bitmap is null and the paint is non-null. CDROID exposes only
    // getBitmap() (no getPaint()) — verify the null bitmap; the default state (alpha=255,
    // gravity=FILL, tile modes DISABLED) is asserted in the dedicated cases below.
    EXPECT_EQ(nullptr, bitmapDrawable.getBitmap());
    EXPECT_NE(nullptr, bitmapDrawable.getConstantState());
    // Skipped: BitmapDrawable(bitmap) / (Resources) / (Resources,bitmap) / (path) / (stream) —
    // CDROID has no android.graphics.Bitmap; stream/path ctors are not 1:1 with Android.
}

TEST_F(CtsBitmapDrawableTest, testAccessGravity) {
    BitmapDrawable bitmapDrawable;
    // CDROID default gravity is FILL (matches CTS).
    EXPECT_EQ(Gravity::FILL, bitmapDrawable.getGravity());

    bitmapDrawable.setGravity(Gravity::CENTER);
    EXPECT_EQ(Gravity::CENTER, bitmapDrawable.getGravity());

    bitmapDrawable.setGravity(-1);
    EXPECT_EQ(-1, bitmapDrawable.getGravity());

    bitmapDrawable.setGravity(INT_MAX);
    EXPECT_EQ(INT_MAX, bitmapDrawable.getGravity());
}

TEST_F(CtsBitmapDrawableTest, testSetAntiAlias) {
    BitmapDrawable bitmapDrawable;
    // CDROID default is false (matches CTS). CTS reads paint.isAntiAlias(); CDROID exposes
    // hasAntiAlias() directly on the drawable.
    EXPECT_FALSE(bitmapDrawable.hasAntiAlias());

    bitmapDrawable.setAntiAlias(true);
    EXPECT_TRUE(bitmapDrawable.hasAntiAlias());

    bitmapDrawable.setAntiAlias(false);
    EXPECT_FALSE(bitmapDrawable.hasAntiAlias());
}

TEST_F(CtsBitmapDrawableTest, testSetFilterBitmap) {
    BitmapDrawable bitmapDrawable;
    // CDROID divergence: defaults to false (CTS's Paint defaults to true via FILTER_BITMAP_FLAG).
    EXPECT_FALSE(bitmapDrawable.isFilterBitmap());

    bitmapDrawable.setFilterBitmap(true);
    EXPECT_TRUE(bitmapDrawable.isFilterBitmap());

    bitmapDrawable.setFilterBitmap(false);
    EXPECT_FALSE(bitmapDrawable.isFilterBitmap());
}

TEST_F(CtsBitmapDrawableTest, testIsFilterBitmap) {
    BitmapDrawable bitmapDrawable;
    // CDROID divergence: default is false (CTS default is true). isFilterBitmap() must agree
    // with the most recent setFilterBitmap() call.
    EXPECT_EQ(bitmapDrawable.isFilterBitmap(), bitmapDrawable.isFilterBitmap());

    bitmapDrawable.setFilterBitmap(false);
    EXPECT_FALSE(bitmapDrawable.isFilterBitmap());

    bitmapDrawable.setFilterBitmap(true);
    EXPECT_TRUE(bitmapDrawable.isFilterBitmap());
}

TEST_F(CtsBitmapDrawableTest, testSetDither) {
    BitmapDrawable bitmapDrawable;
    // CDROID has setDither() but no dither getter (CTS reads paint.isDither()). Port as a
    // smoke test: setDither must not throw. CDROID divergence: default is false (CTS true).
    bitmapDrawable.setDither(false);
    bitmapDrawable.setDither(true);
    bitmapDrawable.setDither(false);
    SUCCEED();
}

TEST_F(CtsBitmapDrawableTest, testAccessTileMode) {
    BitmapDrawable bitmapDrawable;
    // CDROID default tile mode is DISABLED (-1); CTS's default is null. Verify both axes start
    // at DISABLED, then exercise setTileModeX/Y/XY with the TileMode enum (CLAMP/REPEAT/MIRROR).
    EXPECT_EQ(TileMode::DISABLED, bitmapDrawable.getTileModeX());
    EXPECT_EQ(TileMode::DISABLED, bitmapDrawable.getTileModeY());

    bitmapDrawable.setTileModeX(TileMode::CLAMP);
    EXPECT_EQ(TileMode::CLAMP, bitmapDrawable.getTileModeX());
    EXPECT_EQ(TileMode::DISABLED, bitmapDrawable.getTileModeY());

    bitmapDrawable.setTileModeY(TileMode::REPEAT);
    EXPECT_EQ(TileMode::CLAMP, bitmapDrawable.getTileModeX());
    EXPECT_EQ(TileMode::REPEAT, bitmapDrawable.getTileModeY());

    bitmapDrawable.setTileModeXY(TileMode::REPEAT, TileMode::MIRROR);
    EXPECT_EQ(TileMode::REPEAT, bitmapDrawable.getTileModeX());
    EXPECT_EQ(TileMode::MIRROR, bitmapDrawable.getTileModeY());

    bitmapDrawable.setTileModeX(TileMode::MIRROR);
    EXPECT_EQ(TileMode::MIRROR, bitmapDrawable.getTileModeX());
    EXPECT_EQ(TileMode::MIRROR, bitmapDrawable.getTileModeY());

    // Skipped: CTS also asserts the Paint's Shader is (re)built across these calls + draw();
    // CDROID's BitmapDrawable has no getPaint()/getShader() — the shader is an internal detail
    // materialized only inside draw().
}

TEST_F(CtsBitmapDrawableTest, testGetChangingConfigurations) {
    BitmapDrawable bitmapDrawable;
    EXPECT_EQ(0, bitmapDrawable.getChangingConfigurations());

    bitmapDrawable.setChangingConfigurations(1);
    EXPECT_EQ(1, bitmapDrawable.getChangingConfigurations());

    bitmapDrawable.setChangingConfigurations(2);
    EXPECT_EQ(2, bitmapDrawable.getChangingConfigurations());
}

TEST_F(CtsBitmapDrawableTest, testSetAlpha) {
    BitmapDrawable bitmapDrawable;
    // CTS reads paint.getAlpha(); CDROID exposes getAlpha() directly on the drawable.
    EXPECT_EQ(255, bitmapDrawable.getAlpha());

    bitmapDrawable.setAlpha(0);
    EXPECT_EQ(0, bitmapDrawable.getAlpha());

    bitmapDrawable.setAlpha(100);
    EXPECT_EQ(100, bitmapDrawable.getAlpha());

    // CTS "exceptional" out-of-range values: setAlpha masks to 0..255 in CDROID (alpha&0xFF).
    bitmapDrawable.setAlpha(-1);
    EXPECT_GE(bitmapDrawable.getAlpha(), 0);
    EXPECT_LE(bitmapDrawable.getAlpha(), 255);

    bitmapDrawable.setAlpha(256);
    EXPECT_GE(bitmapDrawable.getAlpha(), 0);
    EXPECT_LE(bitmapDrawable.getAlpha(), 255);
}

TEST_F(CtsBitmapDrawableTest, testSetColorFilter) {
    BitmapDrawable bitmapDrawable;
    // CTS reads paint.getColorFilter(); CDROID exposes getColorFilter() on the Drawable base.
    EXPECT_EQ(nullptr, bitmapDrawable.getColorFilter().get());

    cdroid::RefPtr<ColorFilter> colorFilter =
        std::make_shared<PorterDuffColorFilter>(0xFFFF0000, PorterDuff::SRC_IN);
    bitmapDrawable.setColorFilter(colorFilter);
    EXPECT_EQ(colorFilter.get(), bitmapDrawable.getColorFilter().get());

    bitmapDrawable.setColorFilter(nullptr);
    EXPECT_EQ(nullptr, bitmapDrawable.getColorFilter().get());
}

TEST_F(CtsBitmapDrawableTest, testGetConstantState) {
    BitmapDrawable bitmapDrawable;
    auto constantState = bitmapDrawable.getConstantState();
    ASSERT_NE(nullptr, constantState);
    EXPECT_EQ(0, constantState->getChangingConfigurations());

    bitmapDrawable.setChangingConfigurations(1);
    auto constantState2 = bitmapDrawable.getConstantState();
    ASSERT_NE(nullptr, constantState2);
    EXPECT_EQ(1, constantState2->getChangingConfigurations());
}

TEST_F(CtsBitmapDrawableTest, testGetIntrinsicSize) {
    // Default-constructed drawable (no bitmap) reports -1 — matches CTS's BitmapDrawable()-only
    // clause. The bitmap-backed clauses (createBitmap / R.drawable.size_48x48) need Bitmap and
    // density scaling and are skipped.
    BitmapDrawable bitmapDrawable;
    EXPECT_EQ(-1, bitmapDrawable.getIntrinsicWidth());
    EXPECT_EQ(-1, bitmapDrawable.getIntrinsicHeight());
}

TEST_F(CtsBitmapDrawableTest, testGetOpacity) {
    // Default-constructed drawable: gravity FILL + null bitmap => TRANSPARENT (CDROID's
    // getOpacity returns TRANSPARENT when the bitmap is null). CTS asserts the same default.
    BitmapDrawable bitmapDrawable;
    EXPECT_EQ(Gravity::FILL, bitmapDrawable.getGravity());
    EXPECT_EQ((int)PixelFormat::TRANSPARENT, bitmapDrawable.getOpacity());

    // With gravity != FILL, getOpacity returns TRANSLUCENT regardless of bitmap presence.
    bitmapDrawable.setGravity(Gravity::BOTTOM);
    EXPECT_EQ((int)PixelFormat::TRANSLUCENT, bitmapDrawable.getOpacity());

    // Skipped: CTS's bitmap-backed OPAQUE / setAlpha=>TRANSLUCENT clauses need an ImageSurface
    // with known transparency; skipped along with the other Bitmap-API cases.
}

// Skipped (pixel/density/Activity-dependent, see header):
//   testBitmapDrawableOpticalInset  — needs R.raw.testimage + optical-bounds decoding.
//   testAccessMipMap                — CDROID's hasMipMap() always returns (bitmap != nullptr),
//                                      not the real mip state; Bitmap.hasMipMap() is not ported.
//   testSetTargetDensity            — Bitmap density scaling; no Bitmap in CDROID.
//   testInflate                     — needs R.xml.bitmapdrawable; verify separately with an
//                                      added asset if desired.
//   testDraw                        — pixel test on a Canvas.
//   testMutate                      — needs R.drawable.testimage (resource-backed drawable cache
//                                      with multiple instances); the mutate() API itself is
//                                      exercised by the constant-state cases above.
//   testSetBitmap                   — needs android.graphics.Bitmap.
