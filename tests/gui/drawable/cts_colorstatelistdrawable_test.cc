// AOSP CTS drawable test port (ColorStateListDrawableTest.java). ALL 26 CASES SKIPPED.
//
// Reason: ColorStateListDrawable is a NON-FUNCTIONAL STUB in CDROID and is intentionally NOT ported.
//  - It is absent from src/gui/drawable/drawables.cmake (so colorstatelistdrawable.cc is never
//    compiled into libcdroid).
//  - colorstatelistdrawable.cc is raw Java-flavored source that does not compile as C++: it uses
//    `null` instead of nullptr, Java member-access on pointers (`mState.mColor != null`,
//    `mColorDrawable.getColor()`), Java `static class` / `= null` field initializers in the header,
//    and has corrupt method names (`iColorStateListDrawable::...sStateful`,
//    `hColorStateListDrawable::...asFocusStateSpecified`, `cColorStateListDrawable::...anApplyTheme`).
//  - The header (colorstatelistdrawable.h) is equally invalid C++ and must NOT be included here, or
//    the test translation unit would fail to compile.
//
// Because the class does not exist at runtime, every CTS case — which constructs a
// ColorStateListDrawable and drives it through setColorStateList/getColor/getAlpha/setState/
// isStateful/hasFocusStateSpecified/getOpacity/getColorFilter/getConstantState/mutate and the
// Callback proxy paths — has no CDROID-side API to bind to. This file is created for completeness
// and lists each would-be case below as a documentation comment. When ColorStateListDrawable is
// eventually ported faithfully (the .cc rewritten as real C++ and added to drawables.cmake), these
// cases are the porting checklist.
//
// Skipped CTS cases (26):
//  1.  testDefaultConstructor           — new ColorStateListDrawable(); isStateful()==false;
//                                          getColorStateList().getDefaultColor()==ColorDrawable default.
//  2.  testDraw                         — pixel compare (RED/BLUE after setState). Rule: skip pixel.
//  3.  testGetCurrent                   — getCurrent() instanceof ColorDrawable.
//  4.  testIsStateful                   — isStateful() true with multi-state CSL; false after valueOf(solid).
//  5.  testHasFocusStateSpecified       — hasFocusStateSpecified() toggles on a focus-bearing CSL.
//  6.  testAlpha                        — setAlpha/clearAlpha drive getOpacity (TRANSLUCENT/TRANSPARENT/OPAQUE)
//                                          and getAlpha; CDROID PixelFormat has no RGB_888/565 but the
//                                          TRANSLUCENT/TRANSPARENT/OPAQUE triple is representable.
//  7.  testColorFilter                  — setColorFilter/getColorFilter round-trip (LightingColorFilter).
//  8.  testColorStateListAccess         — getColorStateList()/setColorStateList() round-trip; default-color
//                                          resolution via getColorForState(state, YELLOW).
//  9.  testSetState                     — setState(STATE_BLUE/RED) flips ColorDrawable.getColor().
//  10. testMutate                       — mutate() returns this and swaps ConstantState.
//  11. testInvalidationCallbackProxy    — getCurrent().invalidateSelf() forwards as *this to the host Callback.
//  12. testScheduleCallbackProxy        — getCurrent().scheduleSelf(r, t) forwards (this, r, t).
//  13. testUnscheduleCallbackProxy      — getCurrent().unscheduleSelf(r) forwards (this, r).
// (CTS lists 13 @Test methods; the 26 in the task brief counts each assertion group. Either way: 0 portable
//  until the class exists.)
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/ColorStateListDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <guienvironment.h>

using namespace cdroid;

// Empty fixture. No live cases because the class under test is a non-compiling stub (see header).
class CtsColorStateListDrawableTest : public testing::Test {};

// Disabled placeholder so the binary has at least one registered test method for this suite and the
// stub status is visible in test output. Enable once ColorStateListDrawable is faithfully ported.
TEST_F(CtsColorStateListDrawableTest, DISABLED_AllCasesSkippedClassIsStub) {
    GTEST_SKIP() << "ColorStateListDrawable is a non-compiling stub in CDROID (not in drawables.cmake; "
                    "colorstatelistdrawable.cc is raw Java-flavored source). All 26 CTS cases skipped.";
}
