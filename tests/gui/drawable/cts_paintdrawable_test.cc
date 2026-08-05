// AOSP CTS drawable test port (PaintDrawableTest.java). Pure-logic cases only.
//
// Skipped (per the port rules):
//  - testInflateTag: CDROID's ShapeDrawable::inflateTag is `private` (not protected as in Android),
//    so it cannot be re-exposed through a MyPaintDrawable subclass — the whole CTS case, which
//    drives inflateTag via a subclass override and asserts on Resources/XML/padding side effects,
//    cannot be ported. (CDROID inflateTag also takes a different signature: it does not accept a
//    Resources argument.)
//  - testSetCornerRadii's "shorter than 8" sub-case: CTS expects an ArrayIndexOutOfBoundsException
//    from Java's raw-array indexing. CDROID's setCornerRadii takes std::vector<float>; the
//    RoundRectShape ctor indexes by position and a short vector is undefined behavior rather than a
//    catchable exception, so that sub-case is dropped.
//
// CDROID PaintDrawable (a ShapeDrawable port) reproduces the Android setCornerRadius/setCornerRadii
// shape lifecycle (radius>0 builds an 8-element vector and sets a RoundRectShape; <=0 / empty vector
// clears the shape), so testSetCornerRadius and the vector-based testSetCornerRadii cases port
// as-is. `instanceof RoundRectShape` becomes dynamic_cast on the Shape base.
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/PaintDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <drawable/paintdrawable.h>
#include <drawable/shape.h>
#include <guienvironment.h>

using namespace cdroid;

// Helper mirroring CTS.getPaintDrawable(hasShape): a fresh PaintDrawable, with a RoundRectShape set
// when hasShape is true (CTS does this via setCornerRadius(1.5f)). PaintDrawable is returned by
// value (C++17 mandatory copy elision puts it directly in the caller's slot).
static PaintDrawable getPaintDrawable(bool hasShape) {
    PaintDrawable paintDrawable;
    if (hasShape) {
        paintDrawable.setCornerRadius(1.5f);
    }
    return paintDrawable;
}

// Empty fixture — pure logic, no per-case setup.
class CtsPaintDrawableTest : public testing::Test {};

TEST_F(CtsPaintDrawableTest, testConstructor) {
    PaintDrawable a;            // CTS new PaintDrawable()
    PaintDrawable b(0x0);       // CTS new PaintDrawable(0x0)
    PaintDrawable c(0xffffffff); // CTS new PaintDrawable(0xffffffff)
    (void)a; (void)b; (void)c;
    SUCCEED();
}

TEST_F(CtsPaintDrawableTest, testSetCornerRadius) {
    // Positive radius on a shape-less PaintDrawable installs a RoundRectShape.
    {
        PaintDrawable paintDrawable = getPaintDrawable(false);
        EXPECT_EQ(nullptr, paintDrawable.getShape());
        paintDrawable.setCornerRadius(1.5f);
        ASSERT_NE(nullptr, paintDrawable.getShape());
        EXPECT_NE(nullptr, dynamic_cast<RoundRectShape*>(paintDrawable.getShape()));
    }

    // radius == 0 clears the shape.
    {
        PaintDrawable paintDrawable = getPaintDrawable(true);
        ASSERT_NE(nullptr, paintDrawable.getShape());
        paintDrawable.setCornerRadius(0);
        EXPECT_EQ(nullptr, paintDrawable.getShape());
    }

    // negative radius also clears the shape.
    {
        PaintDrawable paintDrawable = getPaintDrawable(true);
        ASSERT_NE(nullptr, paintDrawable.getShape());
        paintDrawable.setCornerRadius(-2.5f);
        EXPECT_EQ(nullptr, paintDrawable.getShape());
    }
}

TEST_F(CtsPaintDrawableTest, testSetCornerRadii) {
    // Test with empty (CTS null): no shape installed.
    {
        PaintDrawable paintDrawable = getPaintDrawable(false);
        EXPECT_EQ(nullptr, paintDrawable.getShape());
        paintDrawable.setCornerRadii({});
        EXPECT_EQ(nullptr, paintDrawable.getShape());
    }

    // With a shape present, empty clears it.
    {
        PaintDrawable paintDrawable = getPaintDrawable(true);
        ASSERT_NE(nullptr, paintDrawable.getShape());
        paintDrawable.setCornerRadii({});
        EXPECT_EQ(nullptr, paintDrawable.getShape());
    }

    // CTS uses a Java float[8]; CDROID takes std::vector<float>.
    const std::vector<float> radii = {4.5f, 6.0f, 4.5f, 6.0f, 4.5f, 6.0f, 4.5f, 6.0f};

    // "shorter than 8" sub-case NOT ported: CTS expects ArrayIndexOutOfBoundsException from raw
    // array indexing; a short std::vector in CDROID is UB, not a catchable exception.

    // Correct 8-element vector installs a RoundRectShape.
    {
        PaintDrawable paintDrawable = getPaintDrawable(true); // has shape
        paintDrawable.setCornerRadii({}); // clear first (so we can verify re-install)
        EXPECT_EQ(nullptr, paintDrawable.getShape());
        paintDrawable.setCornerRadii(radii);
        ASSERT_NE(nullptr, paintDrawable.getShape());
        EXPECT_NE(nullptr, dynamic_cast<RoundRectShape*>(paintDrawable.getShape()));
    }
}
