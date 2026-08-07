// AOSP CTS drawable test port (ShapeDrawableTest.java). Programmatic cases only — pixel/render
// cases (testDraw, testSetTint, testOnDraw*), the Paint-accessor cases (testGetPaint,
// testSetDither, testSetXfermode, testAccessShaderFactory — CDROID's ShapeDrawable exposes no
// getPaint()/ShaderFactory), the null-argument variant (testGetPaddingNull) and the XML inflate
// case (testInflate, needs the shapedrawable_test.xml asset in the pak) are NOT ported.
//
// CDROID divergences (verified against src/gui/drawable/shapedrawable.{h,cc} + shape.{h,cc}):
//  * Only the default ShapeDrawable() ctor is public; CTS's `new ShapeDrawable(Shape)` and
//    `new ShapeDrawable(null)` ctors do not exist — use setShape(null/shape) instead.
//  * getPadding() ALWAYS returns true in CDROID (CTS returns false for an all-zero padding Rect).
//    The boolean assertion is adjusted with a note; the field values are unchanged.
//  * setPadding(l,t,r,b) stores (l,t,r,b) into Rect fields (left,top,width,height), so the value
//    CTS reads as padding.right/bottom is read here as Rect.width/height.
//  * getOpacity() switches on alpha only (255=OPAQUE, 0=TRANSPARENT, else TRANSLUCENT); it does NOT
//    consider the shape, so CTS's "new ShapeDrawable(new RectShape()) → TRANSLUCENT" clause
//    diverges and is omitted (the three alpha-driven branches are ported).
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/ShapeDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <limits.h>
#include <drawable/shapedrawable.h>
#include <drawable/shape.h>
#include <drawable/drawables.h>   // ColorDrawable (unused directly but keeps drawables header grouped)
#include <drawable/colorfilters.h>
#include <core/porterduff.h>
#include <core/app.h>
#include <core/rect.h>
#include <guienvironment.h>

using namespace cdroid;

// Empty fixture — pure logic.
class CtsShapeDrawableTest : public testing::Test {};

TEST_F(CtsShapeDrawableTest, testConstructor) {
    // CDROID only exposes the default ctor; the Shape-arg / null-arg ctors that CTS exercises do
    // not exist (use setShape() instead).
    ShapeDrawable shapeDrawable;
    SUCCEED();
}

TEST_F(CtsShapeDrawableTest, testGetChangingConfigurations) {
    ShapeDrawable shapeDrawable;
    EXPECT_EQ(0, shapeDrawable.getChangingConfigurations());

    shapeDrawable.setChangingConfigurations(1);
    EXPECT_EQ(1, shapeDrawable.getChangingConfigurations());

    shapeDrawable.setChangingConfigurations(INT_MIN);
    EXPECT_EQ(INT_MIN, shapeDrawable.getChangingConfigurations());

    shapeDrawable.setChangingConfigurations(INT_MAX);
    EXPECT_EQ(INT_MAX, shapeDrawable.getChangingConfigurations());

    // CDROID (like AOSP): changing configurations ORs the constant-state's children-changing mask.
    shapeDrawable.setChangingConfigurations(1);
    shapeDrawable.getConstantState();
    shapeDrawable.setChangingConfigurations(2);
    EXPECT_EQ(3, shapeDrawable.getChangingConfigurations());
}

TEST_F(CtsShapeDrawableTest, testGetConstantState) {
    ShapeDrawable shapeDrawable;
    shapeDrawable.setChangingConfigurations(1);
    auto cs = shapeDrawable.getConstantState();
    ASSERT_NE(nullptr, cs);
    EXPECT_EQ(1, cs->getChangingConfigurations());
}

TEST_F(CtsShapeDrawableTest, testAccessIntrinsicHeight) {
    ShapeDrawable shapeDrawable;
    EXPECT_EQ(0, shapeDrawable.getIntrinsicHeight());

    shapeDrawable.setIntrinsicHeight(10);
    EXPECT_EQ(10, shapeDrawable.getIntrinsicHeight());

    shapeDrawable.setIntrinsicHeight(INT_MIN);
    EXPECT_EQ(INT_MIN, shapeDrawable.getIntrinsicHeight());

    shapeDrawable.setIntrinsicHeight(INT_MAX);
    EXPECT_EQ(INT_MAX, shapeDrawable.getIntrinsicHeight());
}

TEST_F(CtsShapeDrawableTest, testAccessIntrinsicWidth) {
    ShapeDrawable shapeDrawable;
    EXPECT_EQ(0, shapeDrawable.getIntrinsicWidth());

    shapeDrawable.setIntrinsicWidth(10);
    EXPECT_EQ(10, shapeDrawable.getIntrinsicWidth());

    shapeDrawable.setIntrinsicWidth(INT_MIN);
    EXPECT_EQ(INT_MIN, shapeDrawable.getIntrinsicWidth());

    shapeDrawable.setIntrinsicWidth(INT_MAX);
    EXPECT_EQ(INT_MAX, shapeDrawable.getIntrinsicWidth());
}

TEST_F(CtsShapeDrawableTest, testGetOpacity) {
    // CDROID's getOpacity switches on alpha only; the shape type is not consulted (CTS additionally
    // asserts `new ShapeDrawable(new RectShape()) → TRANSLUCENT`, which is omitted here because it
    // would diverge — under CDROID a default-alpha RectShape ShapeDrawable reports OPAQUE).
    ShapeDrawable shapeDrawable;
    ASSERT_EQ(255, shapeDrawable.getAlpha());
    EXPECT_EQ((int)PixelFormat::OPAQUE, shapeDrawable.getOpacity());

    shapeDrawable.setAlpha(0);
    EXPECT_EQ((int)PixelFormat::TRANSPARENT, shapeDrawable.getOpacity());

    shapeDrawable.setAlpha(128);
    EXPECT_EQ((int)PixelFormat::TRANSLUCENT, shapeDrawable.getOpacity());
}

TEST_F(CtsShapeDrawableTest, testAccessPadding) {
    ShapeDrawable shapeDrawable;
    Rect padding;
    // CDROID divergence: getPadding always returns true (CTS returns false for all-zero padding).
    EXPECT_TRUE(shapeDrawable.getPadding(padding));
    EXPECT_EQ(0, padding.left);
    EXPECT_EQ(0, padding.top);
    EXPECT_EQ(0, padding.width);   // CTS reads padding.right
    EXPECT_EQ(0, padding.height);  // CTS reads padding.bottom

    // setPadding(l,t,r,b) stores the four values into Rect.{left,top,width,height} respectively.
    shapeDrawable.setPadding(10, 10, 100, 100);
    EXPECT_TRUE(shapeDrawable.getPadding(padding));
    EXPECT_EQ(10, padding.left);
    EXPECT_EQ(10, padding.top);
    EXPECT_EQ(100, padding.width);   // CTS: right == 100
    EXPECT_EQ(100, padding.height);  // CTS: bottom == 100

    shapeDrawable.setPadding(0, 0, 0, 0);
    EXPECT_TRUE(shapeDrawable.getPadding(padding));   // CDROID: still returns true (CTS: false)
    EXPECT_EQ(0, padding.left);
    EXPECT_EQ(0, padding.top);
    EXPECT_EQ(0, padding.width);
    EXPECT_EQ(0, padding.height);

    // Rect overload: CDROID Rect = {left, top, width, height}. Passing {5,5,80,80} therefore means
    // left=5, top=5, right-value=80 (stored in width), bottom-value=80 (stored in height).
    shapeDrawable.setPadding(Rect{5, 5, 80, 80});
    EXPECT_TRUE(shapeDrawable.getPadding(padding));
    EXPECT_EQ(5, padding.left);
    EXPECT_EQ(5, padding.top);
    EXPECT_EQ(80, padding.width);
    EXPECT_EQ(80, padding.height);
}

TEST_F(CtsShapeDrawableTest, testAccessShape) {
    ShapeDrawable shapeDrawable;
    EXPECT_EQ(nullptr, shapeDrawable.getShape());

    auto* rectShape = new RectShape();
    shapeDrawable.setShape(rectShape);
    EXPECT_EQ(rectShape, shapeDrawable.getShape());

    shapeDrawable.setShape(nullptr);
    EXPECT_EQ(nullptr, shapeDrawable.getShape());
}

TEST_F(CtsShapeDrawableTest, testShapeWidthHeightAfterResize) {
    // CDROID's Shape (not ShapeDrawable) carries its own width/height, updated by resize(). This
    // covers the "shape width/height" part of the CTS intent (CTS only exercises intrinsic
    // width/height via ShapeDrawable; CDROID's Shape API exposes the geometry directly).
    RectShape rectShape;
    EXPECT_EQ(0, rectShape.getWidth());
    EXPECT_EQ(0, rectShape.getHeight());

    rectShape.resize(42, 63);
    EXPECT_EQ(42, rectShape.getWidth());
    EXPECT_EQ(63, rectShape.getHeight());

    OvalShape oval;
    oval.resize(100, 50);
    EXPECT_EQ(100, oval.getWidth());
    EXPECT_EQ(50, oval.getHeight());
}

TEST_F(CtsShapeDrawableTest, testSetAlpha) {
    ShapeDrawable shapeDrawable;
    // CDROID setAlpha stores the value (no clamping); CTS only verifies it doesn't throw.
    shapeDrawable.setAlpha(0);
    EXPECT_EQ(0, shapeDrawable.getAlpha());
    shapeDrawable.setAlpha(255);
    EXPECT_EQ(255, shapeDrawable.getAlpha());
    shapeDrawable.setAlpha(-1);
    EXPECT_EQ(-1, shapeDrawable.getAlpha());
    shapeDrawable.setAlpha(256);
    EXPECT_EQ(256, shapeDrawable.getAlpha());
    SUCCEED();
}

TEST_F(CtsShapeDrawableTest, testSetColorFilter) {
    ShapeDrawable shapeDrawable;

    cdroid::RefPtr<ColorFilter> cf = std::make_shared<PorterDuffColorFilter>(0xFF888888, PorterDuff::SRC_OVER);
    shapeDrawable.setColorFilter(cf);
    // CDROID forwards to its paint/state; getColorFilter returns the stored filter.
    EXPECT_EQ(cf.get(), shapeDrawable.getColorFilter().get());

    shapeDrawable.setColorFilter(nullptr);
    EXPECT_EQ(nullptr, shapeDrawable.getColorFilter().get());
}

TEST_F(CtsShapeDrawableTest, testMutateGetShape) {
    ShapeDrawable a;
    a.setShape(new OvalShape());

    // A new drawable from the same constant state shares the source shape initially.
    auto cs = a.getConstantState();
    ASSERT_NE(nullptr, cs);
    Drawable* bd = cs->newDrawable();
    ASSERT_NE(nullptr, bd);
    auto* b = dynamic_cast<ShapeDrawable*>(bd);
    ASSERT_NE(nullptr, b);

    // After a.mutate() the two shapes are distinct instances but remain the same kind.
    a.mutate();

    ASSERT_NE(nullptr, a.getShape());
    ASSERT_NE(nullptr, b->getShape());
    EXPECT_NE(nullptr, dynamic_cast<OvalShape*>(a.getShape()));
    EXPECT_NE(nullptr, dynamic_cast<OvalShape*>(b->getShape()));
    // mutate() deep-clones the shape for the mutated instance.
    EXPECT_NE(a.getShape(), b->getShape());

    delete b;
}
