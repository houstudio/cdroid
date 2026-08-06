// AOSP CTS drawable test port (PictureDrawableTest.java). Pure-logic cases only.
//
// Skipped (per the port rules):
//  - testDraw: pixel-based (Bitmap.getPixel before/after PictureDrawable.draw); CDROID has no
//    Bitmap.getPixel and pixels differ from Skia.
//  - testGetIntrinsicSize's non-null sub-case: CTS does `new Picture()` then
//    `picture.beginRecording(99, 101)` and expects intrinsic size 99x101 from the recording
//    dimensions. CDROID's Picture is a Cairo RecordingSurface and PictureDrawable derives intrinsic
//    size from `ink_extents()` (the *drawn* ink bounds), which is 0x0 for an empty recording — there
//    is no equivalent of Picture.beginRecording(w,h). Only the null-picture case (intrinsic -1/-1)
//    is ported.
//
// CDROID's Picture is `Cairo::RefPtr<Cairo::RecordingSurface>` (== std::shared_ptr<RecordingSurface>).
// A default/null RefPtr corresponds to CTS's `null` Picture; `RecordingSurface::create()` builds a
// non-null empty recording surface corresponding to CTS's `new Picture()`.
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/PictureDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <drawable/drawable.h>
#include <drawable/picturedrawable.h>
#include <drawable/colorfilters.h>
#include <core/porterduff.h>
#include <cairomm/surface.h>
#include <guienvironment.h>

using namespace cdroid;

namespace {
// Convenience aliases: Picture == RefPtr<RecordingSurface> (== shared_ptr<RecordingSurface>).
using Picture = Cairo::RefPtr<Cairo::RecordingSurface>;

// Builds a non-null empty recording surface — the CDROID analog of CTS's `new Picture()`.
Picture newEmptyPicture() {
    return Cairo::RecordingSurface::create();
}
} // namespace

// Empty fixture — pure logic, no per-case setup.
class CtsPictureDrawableTest : public testing::Test {};

TEST_F(CtsPictureDrawableTest, testConstructor) {
    // CTS: new PictureDrawable(null).getPicture() == null.
    EXPECT_EQ(nullptr, PictureDrawable(Picture{}).getPicture());
    // CTS: new PictureDrawable(new Picture()).getPicture() != null.
    EXPECT_NE(nullptr, PictureDrawable(newEmptyPicture()).getPicture());
}

TEST_F(CtsPictureDrawableTest, testGetIntrinsicSize) {
    // null Picture → -1 / -1.
    PictureDrawable pictureDrawable(Picture{});
    EXPECT_EQ(-1, pictureDrawable.getIntrinsicWidth());
    EXPECT_EQ(-1, pictureDrawable.getIntrinsicHeight());
    // The non-null case (CTS records 99x101 and expects those as intrinsic size) is NOT ported:
    // CDROID uses RecordingSurface::ink_extents (drawn bounds), not a recording dimension.
}

TEST_F(CtsPictureDrawableTest, testGetOpacity) {
    PictureDrawable pictureDrawable(Picture{});
    EXPECT_EQ((int)PixelFormat::TRANSLUCENT, pictureDrawable.getOpacity());
}

TEST_F(CtsPictureDrawableTest, testSetAlpha) {
    // PictureDrawable::setAlpha is a no-op override; must not throw.
    PictureDrawable pictureDrawable(Picture{});
    pictureDrawable.setAlpha(0);
    SUCCEED();
}

TEST_F(CtsPictureDrawableTest, testSetColorFilter) {
    // PictureDrawable::setColorFilter is a no-op override; must not throw for both a real filter
    // and a null argument (CTS uses `new ColorFilter()`; CDROID's ColorFilter is abstract, so use a
    // concrete PorterDuffColorFilter).
    PictureDrawable pictureDrawable(Picture{});
    RefPtr<ColorFilter> colorFilter(new PorterDuffColorFilter(0x000000, PorterDuff::SRC_OVER));
    pictureDrawable.setColorFilter(colorFilter);
    SUCCEED();
}

TEST_F(CtsPictureDrawableTest, testSetDither) {
    // Inherited Drawable::setDither — no-op in CDROID; must not throw.
    PictureDrawable pictureDrawable(Picture{});
    pictureDrawable.setDither(true);
    SUCCEED();
}

TEST_F(CtsPictureDrawableTest, testSetFilterBitmap) {
    // Inherited Drawable::setFilterBitmap; must not throw.
    PictureDrawable pictureDrawable(Picture{});
    pictureDrawable.setFilterBitmap(true);
    SUCCEED();
}

TEST_F(CtsPictureDrawableTest, testAccessPicture) {
    PictureDrawable pictureDrawable(Picture{});
    EXPECT_EQ(nullptr, pictureDrawable.getPicture());

    // Test with a real Picture object.
    Picture picture = newEmptyPicture();
    pictureDrawable.setPicture(picture);
    // RefPtr (shared_ptr) compares by stored pointer.
    EXPECT_EQ(picture.get(), pictureDrawable.getPicture().get());

    // Test with null input.
    pictureDrawable.setPicture(Picture{});
    EXPECT_EQ(nullptr, pictureDrawable.getPicture());
}
