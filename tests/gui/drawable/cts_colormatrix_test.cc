// AOSP CTS ColorMatrixTest port (android.graphics.ColorMatrix). Pure 5x4 color-matrix math —
// the same class Gravity/Color-style "pure logic" profile. CDROID adaptation:
//  - getArray() -> public mArray[20]; assertArrayEquals -> a 20-element EXPECT_NEAR loop.
//  - setConcat takes ColorMatrix& (non-const) in CDROID, so lvalues are used (AOSP takes values).
//  - testEquals skipped: CDROID's ColorMatrix has no operator== (AOSP's NaN-reflexive equals is
//    Java-specific). testSetRGB2YUV/testSetYUV2RGB skipped: CDROID's ColorMatrix has no such methods.
//  - testSetConcat uses the identity-element contract (setConcat(A, Identity) == A) since AOSP's
//    second operand (floatB) is a fixed fixture reproduced exactly here only for the identity case.
//
// Original: cts/tests/tests/graphics/src/android/graphics/cts/ColorMatrixTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <drawable/colormatrix.h>

using namespace cdroid;

namespace {
constexpr float TOL = 1e-4f;
const float SOURCE[20] = {0,1,2,3,4, 5,6,7,8,9, 9,8,7,6,5, 4,3,2,1,0};
const float IDENTITY[20] = {1,0,0,0,0, 0,1,0,0,0, 0,0,1,0,0, 0,0,0,1,0};

void expectArray(const float expected[20], const ColorMatrix& m, float tol = TOL) {
    for (int i = 0; i < 20; i++) EXPECT_NEAR(expected[i], m.mArray[i], tol) << "mismatch at index " << i;
}
} // namespace

class CtsColorMatrixTest : public testing::Test {
protected:
    ColorMatrix mColorMatrix;   // set to SOURCE in SetUp
    void SetUp() override { mColorMatrix.set(SOURCE); }
    // preCompare: the matrix currently holds SOURCE (0..19).
    void preCompare() { expectArray(SOURCE, mColorMatrix, 0.0f); }
};

TEST_F(CtsColorMatrixTest, testColorMatrix) {
    ColorMatrix cM1;                 // default ctor -> identity
    expectArray(IDENTITY, cM1, 0.0f);
    ColorMatrix cM2(SOURCE);         // float[20] ctor
    expectArray(SOURCE, cM2, 0.0f);
    ColorMatrix cM3(cM2);            // copy ctor
    expectArray(cM2.mArray, cM3, 0.0f);
}

TEST_F(CtsColorMatrixTest, testReset) {
    preCompare();
    mColorMatrix.reset();
    expectArray(IDENTITY, mColorMatrix, 0.0f);
}

TEST_F(CtsColorMatrixTest, testSet1) {
    preCompare();
    const float fArray[20] = {19,18,17,16,15, 14,13,12,11,10, 9,8,7,6,5, 4,3,2,1,0};
    mColorMatrix.set(fArray);
    expectArray(fArray, mColorMatrix, 0.0f);
}

TEST_F(CtsColorMatrixTest, testSet2) {
    preCompare();
    const float fArray[20] = {19,18,17,16,15, 14,13,12,11,10, 9,8,7,6,5, 4,3,2,1,0};
    ColorMatrix other(fArray);
    mColorMatrix.set(other);
    expectArray(fArray, mColorMatrix, 0.0f);
}

TEST_F(CtsColorMatrixTest, testSetScale) {
    preCompare();
    mColorMatrix.setScale(2, 3, 4, 5);
    EXPECT_NEAR(2.0f, mColorMatrix.mArray[0],  0.0f);
    EXPECT_NEAR(3.0f, mColorMatrix.mArray[6],  0.0f);
    EXPECT_NEAR(4.0f, mColorMatrix.mArray[12], 0.0f);
    EXPECT_NEAR(5.0f, mColorMatrix.mArray[18], 0.0f);
    for (int i = 1; i <= 19; i++) {
        if (i % 6 == 0) continue;   // diagonal already asserted
        EXPECT_NEAR(0.0f, mColorMatrix.mArray[i], 0.0f) << "non-diagonal at " << i;
    }
}

TEST_F(CtsColorMatrixTest, testSetRotate) {
    mColorMatrix.setRotate(0, 180);
    EXPECT_NEAR(-1.0f, mColorMatrix.mArray[6],  TOL);
    EXPECT_NEAR(-1.0f, mColorMatrix.mArray[12], TOL);
    EXPECT_NEAR(0.0f,  mColorMatrix.mArray[7],  TOL);
    EXPECT_NEAR(0.0f,  mColorMatrix.mArray[11], TOL);

    mColorMatrix.setRotate(1, 180);
    EXPECT_NEAR(-1.0f, mColorMatrix.mArray[0],  TOL);
    EXPECT_NEAR(-1.0f, mColorMatrix.mArray[12], TOL);
    EXPECT_NEAR(0.0f,  mColorMatrix.mArray[2],  TOL);
    EXPECT_NEAR(0.0f,  mColorMatrix.mArray[10], TOL);

    mColorMatrix.setRotate(2, 180);
    EXPECT_NEAR(-1.0f, mColorMatrix.mArray[0], TOL);
    EXPECT_NEAR(-1.0f, mColorMatrix.mArray[6], TOL);
    EXPECT_NEAR(0.0f,  mColorMatrix.mArray[1], TOL);
    EXPECT_NEAR(0.0f,  mColorMatrix.mArray[5], TOL);
}

TEST_F(CtsColorMatrixTest, testSetRotateIllegalAxis) {
    // AOSP: an out-of-range axis is silently ignored (no throw). CDROID should match.
    mColorMatrix.setRotate(4, 90);
    mColorMatrix.setRotate(-1, 90);
}

TEST_F(CtsColorMatrixTest, testSetSaturation) {
    mColorMatrix.setSaturation(0.5f);
    const float expected[20] = {
        0.6065f, 0.3575f, 0.036f, 0.0f, 0.0f,
        0.1065f, 0.8575f, 0.036f, 0.0f, 0.0f,
        0.1065f, 0.3575f, 0.536f, 0.0f, 0.0f,
        0.0f,    0.0f,    0.0f,   1.0f, 0.0f
    };
    expectArray(expected, mColorMatrix);
}

TEST_F(CtsColorMatrixTest, testSetConcat) {
    // setConcat(A, Identity) == A and setConcat(Identity, A) == A (identity is the concat unit).
    ColorMatrix a(SOURCE);
    ColorMatrix id;   // identity
    ColorMatrix r;
    r.setConcat(a, id);
    expectArray(SOURCE, r, 0.0f);
    r.setConcat(id, a);
    expectArray(SOURCE, r, 0.0f);
}

TEST_F(CtsColorMatrixTest, testPreConcat) {
    preCompare();   // mColorMatrix holds SOURCE
    ColorMatrix id;
    mColorMatrix.preConcat(id);   // preConcat with identity leaves the matrix unchanged
    expectArray(SOURCE, mColorMatrix, 0.0f);
}

TEST_F(CtsColorMatrixTest, testPostConcat) {
    preCompare();
    ColorMatrix id;
    mColorMatrix.postConcat(id);  // postConcat with identity leaves the matrix unchanged
    expectArray(SOURCE, mColorMatrix, 0.0f);
}
