// AOSP CTS drawable test port (ColorDrawableTest.java). Pure-logic cases only — render/pixel
// cases (testDraw, testSetColorFilter, testSetTint — all use Bitmap.getPixel via
// DrawableTestUtils.getPixel) and the XML-inflate case (testInflate parses colordrawable_test.xml
// via Resources/XmlPullParser and CDROID's inflate takes (parser, attrs) with no Resources arg) are
// NOT ported. CDROID's ColorDrawable has NO default constructor — CTS's `new ColorDrawable()`
// (Android default = transparent color 0) maps to ColorDrawable(0). The alpha-modulation arithmetic
// (CTS testAccessAlpha) is reproduced verbatim by CDROID's setAlpha/applyAlpha and passes as-is.
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/ColorDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <limits.h>
#include <drawable/drawable.h>
#include <drawable/drawables.h>
#include <drawable/colorfilters.h>
#include <core/porterduff.h>
#include <guienvironment.h>

using namespace cdroid;

// Empty fixture — pure logic, no per-case setup. App/Context is provided process-wide by
// GUIEnvironment (see guienvironment.h).
class CtsColorDrawableTest : public testing::Test {};

TEST_F(CtsColorDrawableTest, testConstructors) {
    // CDROID's ColorDrawable has no default (no-arg) constructor — CTS's `new ColorDrawable()` is
    // represented here by ColorDrawable(0) (Android's no-arg default color is transparent == 0).
    ColorDrawable d0(0);
    ColorDrawable d1(1);
    (void)d0;
    (void)d1;
    SUCCEED();
}

TEST_F(CtsColorDrawableTest, testAccessAlpha) {
    // CTS uses `new ColorDrawable()` (transparent, color 0); CDROID uses ColorDrawable(0).
    // CTS reuses one variable via Java reference reassignment; CDROID's ColorDrawable is not
    // generally copy/Move-assignable (Callback + shared state), so each phase uses its own instance.
    {
        ColorDrawable colorDrawable(0);
        EXPECT_EQ(0, colorDrawable.getAlpha());
        colorDrawable.setAlpha(128);
        EXPECT_EQ(0, colorDrawable.getAlpha());
    }
    {
        ColorDrawable colorDrawable(1 << 24);  // 0x01000000
        EXPECT_EQ(1, colorDrawable.getAlpha());
        colorDrawable.setAlpha(128);
        EXPECT_EQ(0, colorDrawable.getAlpha());
        colorDrawable.setAlpha(255);
        EXPECT_EQ(1, colorDrawable.getAlpha());
    }
}

TEST_F(CtsColorDrawableTest, testGetChangingConfigurations) {
    ColorDrawable colorDrawable(0);
    EXPECT_EQ(0, colorDrawable.getChangingConfigurations());

    colorDrawable.setChangingConfigurations(1);
    EXPECT_EQ(1, colorDrawable.getChangingConfigurations());

    colorDrawable.setChangingConfigurations(INT_MIN);
    EXPECT_EQ(INT_MIN, colorDrawable.getChangingConfigurations());

    colorDrawable.setChangingConfigurations(INT_MAX);
    EXPECT_EQ(INT_MAX, colorDrawable.getChangingConfigurations());
}

TEST_F(CtsColorDrawableTest, testGetConstantState) {
    ColorDrawable colorDrawable(0);
    ASSERT_NE(nullptr, colorDrawable.getConstantState());
    EXPECT_EQ(colorDrawable.getChangingConfigurations(),
              colorDrawable.getConstantState()->getChangingConfigurations());
}

TEST_F(CtsColorDrawableTest, testGetOpacity) {
    {
        ColorDrawable colorDrawable(0);  // transparent
        EXPECT_EQ((int)PixelFormat::TRANSPARENT, colorDrawable.getOpacity());
    }
    {
        ColorDrawable colorDrawable(255 << 24);  // 0xFF000000 opaque
        EXPECT_EQ((int)PixelFormat::OPAQUE, colorDrawable.getOpacity());
    }
    {
        ColorDrawable colorDrawable(1 << 24);  // 0x01000000 translucent (alpha 1)
        EXPECT_EQ((int)PixelFormat::TRANSLUCENT, colorDrawable.getOpacity());
    }
}

TEST_F(CtsColorDrawableTest, testGetColorFilter) {
    // Logic-only half of CTS testGetColorFilter: setting a ColorFilter then retrieving it returns
    // the same instance. (CTS's testSetColorFilter/setTint are pixel-based and not ported.)
    ColorDrawable d(0xFFFFFFFF);
    // ColorFilter is abstract; allocate a concrete PorterDuffColorFilter through a ColorFilter
    // base pointer (RefPtr<ColorFilter> == std::shared_ptr<ColorFilter>).
    RefPtr<ColorFilter> colorFilter(new PorterDuffColorFilter(0x000000, PorterDuff::SRC_OVER));
    d.setColorFilter(colorFilter);

    // RefPtr (shared_ptr) compares by stored pointer; .get() yields the raw ColorFilter*.
    EXPECT_EQ(colorFilter.get(), d.getColorFilter().get());
}
