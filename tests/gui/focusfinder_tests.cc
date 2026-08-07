// Ported from AOSP frameworks/base/core/tests/coretests FocusFinderTest.java.
// Tests the focus-search algorithm (isCandidate / beamsOverlap / isBetterCandidate /
// majorAxisDistance) directly via a helper that exposes CDROID's protected members.
//
// CDROID adaptation:
//  - android.graphics.Rect(l,t,r,b) -> cdroid::Rect::MakeLTRB(l,t,r,b) (stores {l,t,w,h}).
//  - View.FOCUS_* -> cdroid::View::FOCUS_*.
//  - CDROID FocusFinder keeps isCandidate/beamsOverlap/isBetterCandidate protected; a
//    FocusFinderTestHelper subclass re-exposes them via `using`.
//  - AOSP beamBeats() is private and not ported by CDROID; the one assertBeamBeats call in
//    testGmailReplyButtonsScenario is dropped (isBetterCandidate already exercises that logic).
//
// Original: frameworks/base/core/tests/coretests/src/android/view/FocusFinderTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <view/focusfinder.h>
#include <view/view.h>
#include <core/rect.h>

using namespace cdroid;

namespace {
// Expose the protected focus-algorithm primitives for testing.
class FocusFinderTestHelper : public FocusFinder {
public:
    using FocusFinder::isCandidate;
    using FocusFinder::beamsOverlap;
    using FocusFinder::isBetterCandidate;
    static int majorAxisDistance(int direction,const Rect& source,const Rect& dest) {
        return FocusFinder::majorAxisDistance(direction,source,dest);
    }

    static int majorAxisDistanceToFarEdge(int direction,const Rect& source,const Rect& dest) {
        return FocusFinder::majorAxisDistanceToFarEdge(direction,source,dest);
    }
};
FocusFinderTestHelper ff;

// android.graphics.Rect(l,t,r,b) ctor equivalent.
using R = Rect;
const R M(int l, int t, int r, int b) { return Rect::MakeLTRB(l, t, r, b); }

void assertDirectionIsCandidate(int direction, const R& src, const R& dest) {
    EXPECT_TRUE(ff.isCandidate(src, dest, direction)) << "expected candidate";
}
void assertIsNotCandidate(int direction, const R& src, const R& dest) {
    EXPECT_FALSE(ff.isCandidate(src, dest, direction)) << "expected NOT candidate";
}
void assertBeamsOverlap(int direction, const R& a, const R& b) {
    EXPECT_TRUE(ff.beamsOverlap(direction, a, b)) << "expected beams overlap";
}
void assertBeamsDontOverlap(int direction, const R& a, const R& b) {
    EXPECT_FALSE(ff.beamsOverlap(direction, a, b)) << "expected beams don't overlap";
}
void assertBetterCandidate(int direction, const R& src, const R& better, const R& worse) {
    EXPECT_TRUE(ff.isBetterCandidate(direction, src, better, worse))  << "expected better";
    EXPECT_FALSE(ff.isBetterCandidate(direction, src, worse, better)) << "expected not better";
}
} // namespace

TEST(FocusFinderTest, testPreconditions) {
    EXPECT_NE(nullptr, &FocusFinder::getInstance());
}

TEST(FocusFinderTest, testBelowNotCandidateForDirectionUp) {
    assertIsNotCandidate(View::FOCUS_UP, M(0,30,10,40), M(0,50,10,60));
}

TEST(FocusFinderTest, testAboveShareEdgeEdgeOkForDirectionUp) {
    R src = M(0,30,10,40);
    R dest = M(0,20,10,30);  // dest.bottom == src.top
    assertDirectionIsCandidate(View::FOCUS_UP, src, dest);
}

TEST(FocusFinderTest, testCompletelyContainedNotCandidate) {
    assertIsNotCandidate(View::FOCUS_UP, M(0,30,10,40), M(0,20,10,50));
}

TEST(FocusFinderTest, testContainedWithCommonBottomNotCandidate) {
    assertIsNotCandidate(View::FOCUS_UP, M(0,30,10,40), M(0,20,10,40));
}

TEST(FocusFinderTest, testOverlappingIsCandidateWhenBothEdgesAreInDirection) {
    assertDirectionIsCandidate(View::FOCUS_UP, M(0,30,10,40), M(0,25,10,35));
}

TEST(FocusFinderTest, testTopEdgeOfDestAtOrAboveTopOfSrcNotCandidateForDown) {
    assertIsNotCandidate(View::FOCUS_DOWN, M(0,30,10,40), M(0,20,10,50));
    assertIsNotCandidate(View::FOCUS_DOWN, M(0,30,10,40), M(0,30,10,50));
}

TEST(FocusFinderTest, testSameRectBeamsOverlap) {
    assertBeamsOverlap(View::FOCUS_DOWN, M(0,0,50,50), M(0,0,50,50));
}

TEST(FocusFinderTest, testOverlapBeamsRightLeftUpToEdge) {
    assertBeamsOverlap(View::FOCUS_LEFT, M(0,0,50,50), M(0,0,50,50));
    assertBeamsOverlap(View::FOCUS_LEFT, M(0,0,50,50), M(0,50,50,100));
    assertBeamsDontOverlap(View::FOCUS_LEFT, M(0,0,50,50), M(0,51,50,101));
}

TEST(FocusFinderTest, testOverlapBeamsUpDownUpToEdge) {
    assertBeamsOverlap(View::FOCUS_DOWN, M(0,0,50,50), M(0,0,50,50));
    assertBeamsOverlap(View::FOCUS_DOWN, M(0,0,50,50), M(50,0,100,50));
    assertBeamsDontOverlap(View::FOCUS_DOWN, M(0,0,50,50), M(51,0,101,50));
}

TEST(FocusFinderTest, testDirectlyAboveTrumpsAboveLeft) {
    assertBetterCandidate(View::FOCUS_DOWN, M(0,0,30,30), M(0,40,30,70), M(-20,40,10,70));
}

TEST(FocusFinderTest, testAboveInBeamTrumpsSlightlyCloserOutOfBeam) {
    assertBetterCandidate(View::FOCUS_DOWN, M(0,0,30,30), M(0,40,30,70), M(31,41,61,71));
}

TEST(FocusFinderTest, testOutOfBeamBeatsInBeamUp) {
    assertBetterCandidate(View::FOCUS_DOWN, M(0,0,30,30), M(31,0,61,30), M(0,31,30,61));
}

TEST(FocusFinderTest, testSomeCandidateBetterThanNonCandidate) {
    assertBetterCandidate(View::FOCUS_DOWN, M(0,0,30,30), M(0,31,30,61), M(0,0,30,30));
}

TEST(FocusFinderTest, testVerticalFocusSearchScenario) {
    assertBetterCandidate(View::FOCUS_DOWN, M(0,0,30,30), M(0,30,30,60), M(0,60,30,90));
}

TEST(FocusFinderTest, testBeamsOverlapMajorAxisCloserMinorAxisFurther) {
    assertBetterCandidate(View::FOCUS_DOWN, M(0,0,30,30), M(0,31,30,61), M(31,32,61,62));
}

TEST(FocusFinderTest, testMusicPlaybackScenario) {
    assertBetterCandidate(View::FOCUS_LEFT, M(227,185,312,231), M(195,386,266,438), M(124,386,195,438));
}

TEST(FocusFinderTest, testOutOfBeamOverlapBeatsOutOfBeamFurtherOnMajorAxis) {
    assertBetterCandidate(View::FOCUS_DOWN, M(0,0,50,50), M(60,40,110,90), M(60,70,110,120));
}

TEST(FocusFinderTest, testInBeamTrumpsOutOfBeamOverlapping) {
    assertBetterCandidate(View::FOCUS_DOWN, M(0,0,50,50), M(0,60,50,110), M(51,1,101,51));
}

TEST(FocusFinderTest, testOverlappingBeatsNonOverlapping) {
    assertBetterCandidate(View::FOCUS_DOWN, M(0,0,50,50), M(0,40,50,90), M(0,75,50,125));
}

TEST(FocusFinderTest, testEditContactScenarioLeftFromDiscardChangesGoesToSaveContactInLandscape) {
    assertBetterCandidate(View::FOCUS_LEFT, M(357,258,478,318), M(2,258,100,318), M(106,120,424,184));
}

TEST(FocusFinderTest, testGridWithTouchingEdges) {
    assertBetterCandidate(View::FOCUS_DOWN, M(106,49,212,192), M(106,192,212,335), M(0,192,106,335));
    assertBetterCandidate(View::FOCUS_DOWN, M(106,49,212,192), M(106,192,212,335), M(212,192,318,335));
}

TEST(FocusFinderTest, testSearchFromEmptyRect) {
    assertBetterCandidate(View::FOCUS_DOWN, M(0,0,0,0), M(0,0,320,45), M(0,45,320,545));
}

TEST(FocusFinderTest, testGmailReplyButtonsScenario) {
    // AOSP also asserts beamBeats() here (private, not ported by CDROID); isBetterCandidate
    // already encodes that logic, so the better/worse assertion is the faithful subset.
    assertBetterCandidate(View::FOCUS_LEFT, M(223,380,312,417), M(102,380,210,417), M(111,443,206,480));
    assertBeamsOverlap(View::FOCUS_LEFT, M(223,380,312,417), M(102,380,210,417));
    assertBeamsDontOverlap(View::FOCUS_LEFT, M(223,380,312,417), M(111,443,206,480));
    EXPECT_LT(FocusFinderTestHelper::majorAxisDistance(View::FOCUS_LEFT, M(223,380,312,417), M(102,380,210,417)),
              FocusFinderTestHelper::majorAxisDistanceToFarEdge(View::FOCUS_LEFT, M(223,380,312,417), M(111,443,206,480)));
}

TEST(FocusFinderTest, testGmailScenarioBug1203288) {
    assertBetterCandidate(View::FOCUS_DOWN, M(0,2,480,82), M(344,87,475,124), M(0,130,480,203));
}

TEST(FocusFinderTest, testBeamAlwaysBeatsHoriz) {
    assertBetterCandidate(View::FOCUS_RIGHT, M(0,0,50,50), M(150,0,200,50), M(60,51,110,101));
    assertBetterCandidate(View::FOCUS_LEFT,  M(150,0,200,50), M(0,0,50,50), M(49,99,149,101));
}

TEST(FocusFinderTest, testIsCandidateOverlappingEdgeFromEmptyRect) {
    assertDirectionIsCandidate(View::FOCUS_DOWN,  M(0,0,0,0), M(0,0,20,1));
    assertDirectionIsCandidate(View::FOCUS_UP,    M(0,0,0,0), M(0,-1,20,0));
    assertDirectionIsCandidate(View::FOCUS_LEFT,  M(0,0,0,0), M(-1,0,0,20));
    assertDirectionIsCandidate(View::FOCUS_RIGHT, M(0,0,0,0), M(0,0,1,20));
}
