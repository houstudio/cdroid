// Ported from AOSP CTS StateSetTest.java (android.util.StateSet).
//
// CDROID adaptation:
//  - Java int[] -> std::vector<int>; `new int[N]` (zero-init) -> vector<int>(N, 0).
//  - CTS passes null stateSet -> empty vector<int>{} (CDROID has no null vector; Android treats
//    null like an empty state set in stateSetMatches).
//  - trimStateSet is void in CDROID (mutates in place) vs returns int[] in CTS; assert the
//    vector's size/contents after the call.
//  - testDump: CDROID StateSet has no dump() -> skipped.
//
// Original: cts/tests/tests/util/src/android/util/cts/StateSetTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <drawable/stateset.h>
#include <vector>

using namespace cdroid;

TEST(CtsStateSetTest, testTrimStateSet) {
    // CDROID trimStateSet mutates in place (void); CTS returns a new array.
    std::vector<int> stateSet = {1, 2, 3};
    StateSet::trimStateSet(stateSet, 3);
    EXPECT_EQ(3u, stateSet.size());
    EXPECT_EQ(1, stateSet[0]);
    EXPECT_EQ(2, stateSet[1]);
    EXPECT_EQ(3, stateSet[2]);

    StateSet::trimStateSet(stateSet, 2);
    EXPECT_EQ(2u, stateSet.size());
    EXPECT_EQ(1, stateSet[0]);
    EXPECT_EQ(2, stateSet[1]);
}

TEST(CtsStateSetTest, testDump) {
    // CDROID StateSet has no dump(states) printer; skipped.
    SUCCEED();
}

// --- CDROID supplementary API tests (not in CTS StateSetTest) ---

TEST(CtsStateSetTest, testIsWildCard) {
    EXPECT_TRUE(StateSet::isWildCard(StateSet::WILD_CARD));   // {} (empty)
    EXPECT_TRUE(StateSet::isWildCard(std::vector<int>{0}));   // {0}
    EXPECT_FALSE(StateSet::isWildCard(std::vector<int>{1}));
    EXPECT_FALSE(StateSet::isWildCard(std::vector<int>{-1}));
}

TEST(CtsStateSetTest, testStaticStateSets) {
    EXPECT_EQ((std::vector<int>{StateSet::ENABLED}),  StateSet::ENABLED_STATE_SET);
    EXPECT_EQ((std::vector<int>{StateSet::PRESSED}),  StateSet::PRESSED_STATE_SET);
    EXPECT_EQ((std::vector<int>{StateSet::FOCUSED}),  StateSet::FOCUSED_STATE_SET);
    EXPECT_EQ((std::vector<int>{StateSet::SELECTED}), StateSet::SELECTED_STATE_SET);
    EXPECT_EQ((std::vector<int>{StateSet::CHECKED}),  StateSet::CHECKED_STATE_SET);
}

TEST(CtsStateSetTest, testContainsAttribute) {
    std::vector<std::vector<int>> specs = {
        {StateSet::FOCUSED},
        {StateSet::PRESSED, -StateSet::SELECTED}
    };
    EXPECT_TRUE(StateSet::containsAttribute(specs, StateSet::FOCUSED));
    EXPECT_TRUE(StateSet::containsAttribute(specs, StateSet::PRESSED));
    EXPECT_TRUE(StateSet::containsAttribute(specs, StateSet::SELECTED));  // -SELECTED matches attr
    EXPECT_FALSE(StateSet::containsAttribute(specs, StateSet::CHECKED));
    EXPECT_FALSE(StateSet::containsAttribute({}, StateSet::FOCUSED));
}

TEST(CtsStateSetTest, testStateSetMatches) {
    // --- spec1 / stateSet1 ---
    std::vector<int> stateSpec1(2, 0);  // {0,0}
    std::vector<int> stateSet1(3, 0);   // {0,0,0}
    stateSpec1[0] = 1;
    stateSet1[0] = 1;
    EXPECT_TRUE(StateSet::stateSetMatches(stateSpec1, stateSet1));
    stateSet1[0] = 2;
    EXPECT_FALSE(StateSet::stateSetMatches(stateSpec1, stateSet1));
    stateSpec1[1] = 2;
    EXPECT_FALSE(StateSet::stateSetMatches(stateSpec1, stateSet1));
    stateSet1[1] = 1;
    EXPECT_TRUE(StateSet::stateSetMatches(stateSpec1, stateSet1));
    stateSet1[2] = 12345;
    EXPECT_TRUE(StateSet::stateSetMatches(stateSpec1, stateSet1));

    // --- spec2 / stateSet2 (must-not-match via negative) ---
    std::vector<int> stateSpec2(2, 0);
    std::vector<int> stateSet2(2, 0);
    stateSpec2[0] = 1;
    stateSpec2[1] = -2;
    stateSet2[0] = 1;
    EXPECT_TRUE(StateSet::stateSetMatches(stateSpec2, stateSet2));
    stateSet2[0] = 2;
    EXPECT_FALSE(StateSet::stateSetMatches(stateSpec2, stateSet2));
    stateSet2[1] = 1;
    EXPECT_FALSE(StateSet::stateSetMatches(stateSpec2, stateSet2));
    stateSet2[0] = 12345;
    EXPECT_TRUE(StateSet::stateSetMatches(stateSpec2, stateSet2));

    // --- spec3 / stateSet3 ---
    std::vector<int> stateSpec3(2, 0);
    std::vector<int> stateSet3(3, 0);
    stateSpec3[0] = -1;
    stateSet3[0] = 2;
    EXPECT_TRUE(StateSet::stateSetMatches(stateSpec3, stateSet3));
    stateSet3[1] = 12345;
    EXPECT_TRUE(StateSet::stateSetMatches(stateSpec3, stateSet3));
    stateSet3[0] = 1;
    EXPECT_FALSE(StateSet::stateSetMatches(stateSpec3, stateSet3));
    stateSpec3[1] = -2;
    EXPECT_FALSE(StateSet::stateSetMatches(stateSpec3, stateSet3));
    stateSet3[2] = 12345;
    EXPECT_FALSE(StateSet::stateSetMatches(stateSpec3, stateSet3));

    // --- spec4: all must-not-match, empty/zero stateSet ---
    std::vector<int> stateSpec4 = {-12345, -6789};
    EXPECT_TRUE(StateSet::stateSetMatches(stateSpec4, std::vector<int>{}));
    EXPECT_TRUE(StateSet::stateSetMatches(stateSpec4, std::vector<int>{0}));

    // --- spec5: single must-match, empty/zero stateSet ---
    std::vector<int> stateSpec5 = {12345};
    EXPECT_FALSE(StateSet::stateSetMatches(stateSpec5, std::vector<int>{}));
    EXPECT_FALSE(StateSet::stateSetMatches(stateSpec5, std::vector<int>{0}));

    // --- spec6: WILD_CARD ---
    std::vector<int> stateSpec6 = StateSet::WILD_CARD;
    EXPECT_TRUE(StateSet::stateSetMatches(stateSpec6, std::vector<int>{}));
    EXPECT_TRUE(StateSet::stateSetMatches(stateSpec6, std::vector<int>{0}));

    // --- spec7: null stateSet (CDROID: empty vector) ---
    std::vector<int> stateSpec7(3, 0);
    stateSpec7[0] = 1;
    EXPECT_FALSE(StateSet::stateSetMatches(stateSpec7, std::vector<int>{}));
    stateSpec7[1] = -1;
    EXPECT_FALSE(StateSet::stateSetMatches(stateSpec7, std::vector<int>{}));
    stateSpec7 = StateSet::WILD_CARD;
    EXPECT_TRUE(StateSet::stateSetMatches(stateSpec7, std::vector<int>{}));

    // --- spec8 / single int state (must-match + must-not-match) ---
    std::vector<int> stateSpec8(2, 0);
    stateSpec8[0] = 1;
    EXPECT_TRUE(StateSet::stateSetMatches(stateSpec8, 1));
    EXPECT_FALSE(StateSet::stateSetMatches(stateSpec8, 2));
    stateSpec8[1] = -12345;
    // CTS line 169: state1 is still 2 (from line 165); spec {1,-12345} vs state 2 -> 2!=1 fails.
    EXPECT_FALSE(StateSet::stateSetMatches(stateSpec8, 2));

    // --- spec9 / single int state (must-not-match positive) ---
    std::vector<int> stateSpec9(2, 0);
    stateSpec9[0] = -1;
    EXPECT_FALSE(StateSet::stateSetMatches(stateSpec9, 1));
    EXPECT_TRUE(StateSet::stateSetMatches(stateSpec9, 2));
    stateSpec9[1] = -12345;
    EXPECT_TRUE(StateSet::stateSetMatches(stateSpec9, 2));

    // --- spec10 / single int state == 0 ---
    std::vector<int> stateSpec10(3, 0);
    stateSpec10[0] = 1;
    EXPECT_FALSE(StateSet::stateSetMatches(stateSpec10, 0));
    stateSpec10[1] = -1;
    EXPECT_FALSE(StateSet::stateSetMatches(stateSpec10, 0));
    stateSpec10 = StateSet::WILD_CARD;
    EXPECT_TRUE(StateSet::stateSetMatches(stateSpec10, 0));
}
