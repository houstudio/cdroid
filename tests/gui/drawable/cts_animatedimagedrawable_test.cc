// AOSP CTS drawable test port (AnimatedImageDrawableTest.java). The overwhelming majority of the
// AOSP cases are NOT portable headless: they build the drawable by decoding a real animated
// GIF/WEBP binary asset via ImageDecoder (createFromImageDecoder / decodeInBackground), drive it
// on the UI thread inside an AnimatedImageActivity with an ImageView, await start/end via Animatable2
// callbacks against frame timings, and several pixel-compare the rendered frames (Cairo != Skia).
// None of ImageDecoder-on-a-binary-asset / Activity / ImageView / render-thread timing / pixel
// comparison is reachable in the headless test environment.
//
// Ported (default-instance + ConstantState contract, mirroring CtsVectorDrawableTest):
//   testEmptyConstructor, testGetOpacity, testAutoMirrored, testRepeatCount,
//   testGetConstantState, testMutate, testGetChangingConfigurations.
// The ConstantState/mutate cases use getConstantState().newDrawable() to produce a shared-state
// sibling (AOSP uses two resource-cached instances; the copy-on-write contract under test is the
// same). AnimatedImageState.mAlpha is initialized to 255 (AOSP default) + copied in the copy ctor
// (both fixed as part of this port) so mutate's alpha independence is now testable.
//
// Skipped (asset/Activity/pixel/stream-dependent, see above): testDecodeAnimatedImageDrawable,
//   testRegisterWithoutLooper, testRegisterCallback, testClearCallbacks, testUnregisterCallback,
//   testLifeCycle, testLifeCycleSoftware, testAddCallbackAfterStart, testStop, testRepeatCounts,
//   testRepeatCountInfinite, testEncodedRepeats, testColorFilter, testExif, testPostProcess,
//   testCreateFromXml*, testMissingSrcInflate, testAutoMirroredFromXml, testAutoStartFromXml,
//   testAutoMirroredDrawing, testRepeatCountFromXml, testInfiniteRepeatCountFromXml,
//   testInputStream, testByteBuffer, testReadOnlyByteBuffer, testDirectByteBuffer.
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/AnimatedImageDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <drawable/animatedimagedrawable.h>

using namespace cdroid;

class CtsAnimatedImageDrawableTest : public testing::Test {};

TEST_F(CtsAnimatedImageDrawableTest, testEmptyConstructor) {
    AnimatedImageDrawable drawable;
}

TEST_F(CtsAnimatedImageDrawableTest, testGetOpacity) {
    // AOSP decodes a real image first; CDROID's AnimatedImageDrawable::getOpacity is unconditionally
    // TRANSLUCENT, so the assertion holds for a default-constructed instance too.
    AnimatedImageDrawable drawable;
    EXPECT_EQ((int)PixelFormat::TRANSLUCENT, drawable.getOpacity());
}

TEST_F(CtsAnimatedImageDrawableTest, testAutoMirrored) {
    // AOSP's testAutoMirrored decodes a real image, then exercises the set/get pair; that pair is
    // default-instance logic, so it is ported without the decode dependency.
    AnimatedImageDrawable drawable;
    EXPECT_FALSE(drawable.isAutoMirrored());

    drawable.setAutoMirrored(true);
    EXPECT_TRUE(drawable.isAutoMirrored());

    drawable.setAutoMirrored(false);
    EXPECT_FALSE(drawable.isAutoMirrored());
}

TEST_F(CtsAnimatedImageDrawableTest, testRepeatCount) {
    // AOSP's testRepeatCounts decodes a real image (encoded repeat count), then asserts
    // setRepeatCount/getRepeatCount round-trip and REPEAT_INFINITE restore. The set/get round-trip
    // is default-instance logic, ported here; the encoded-count assertion is asset-dependent.
    AnimatedImageDrawable drawable;
    drawable.setRepeatCount(5);
    EXPECT_EQ(5, drawable.getRepeatCount());

    drawable.setRepeatCount(AnimatedImageDrawable::REPEAT_INFINITE);
    EXPECT_EQ(AnimatedImageDrawable::REPEAT_INFINITE, drawable.getRepeatCount());
}

TEST_F(CtsAnimatedImageDrawableTest, testGetConstantState) {
    AnimatedImageDrawable drawable;
    auto constantState = drawable.getConstantState();
    ASSERT_NE(nullptr, constantState);
    // newDrawable yields a distinct instance backed by the same constant state.
    Drawable* copy = constantState->newDrawable();
    ASSERT_NE(nullptr, copy);
    EXPECT_NE(&drawable, copy);
    delete copy;
}

TEST_F(CtsAnimatedImageDrawableTest, testMutate) {
    // mutate() must give this drawable a private constant-state copy (copy-on-write), so a state
    // change on the mutated instance does not affect a sibling produced from the same
    // getConstantState(). Mirrors AOSP testMutate (which uses two resource-cached instances).
    AnimatedImageDrawable d1;
    ASSERT_EQ(255, d1.getAlpha());  // default alpha, now properly initialized
    AnimatedImageDrawable* d2 = dynamic_cast<AnimatedImageDrawable*>(d1.getConstantState()->newDrawable());
    ASSERT_NE(nullptr, d2);
    ASSERT_EQ(255, d2->getAlpha());

    d1.mutate();
    d1.setAlpha(100);
    EXPECT_EQ(100, d1.getAlpha());
    EXPECT_EQ(255, d2->getAlpha());  // sibling unaffected — mutate copied the state

    d2->mutate();
    d2->setAlpha(50);
    EXPECT_EQ(100, d1.getAlpha());
    EXPECT_EQ(50, d2->getAlpha());
    delete d2;
}

TEST_F(CtsAnimatedImageDrawableTest, testGetChangingConfigurations) {
    AnimatedImageDrawable drawable;
    auto constantState = drawable.getConstantState();
    ASSERT_NE(nullptr, constantState);

    // default
    EXPECT_EQ(0, constantState->getChangingConfigurations());
    EXPECT_EQ(0, drawable.getChangingConfigurations());

    // changing the drawable's configuration does not affect the cached state's snapshot
    drawable.setChangingConfigurations(0xff);
    EXPECT_EQ(0xff, drawable.getChangingConfigurations());
    EXPECT_EQ(0, constantState->getChangingConfigurations());

    // re-fetching the constant state reflects the new value
    constantState = drawable.getConstantState();
    EXPECT_EQ(0xff, constantState->getChangingConfigurations());

    // set a new configuration; drawable ORs with the state's value
    drawable.setChangingConfigurations(0xff00);
    EXPECT_EQ(0xff, constantState->getChangingConfigurations());
    EXPECT_EQ(0xffff, drawable.getChangingConfigurations());
}
