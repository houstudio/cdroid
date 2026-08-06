// AOSP CTS drawable test port (AnimatedStateListDrawableTest.java). Pure-logic cases only.
//
// Ported (5): testStateListDrawable, testAddState (keyframe add), testAddTransition (child-count
// growth), testIsStateful, testOnStateChange (state->keyframe matching via the inherited
// StateListDrawable logic — the part CTS labels "keyframe match").
//
// Skipped (4):
//  - testPreloadDensity / testPreloadDensityConstantSize: bitmap density scaling (CDROID has no
//    resource-density pipeline; rule: skip density).
//  - testInflate / testParsingTransitionDefinedWithAVD: require the real CTS resources
//    animated_state_list_density / animated_state_list_with_avd (AVD) which are not in the test
//    pak; rule: skip cases needing resources CDROID doesn't ship.
//
// CDROID divergences noted inline:
//  - CTS expects IllegalArgumentException from addState/addTransition with a null drawable/transition.
//    CDROID's addState/addTransition use FATAL_IF which only LOGS (FatalMessage::~FatalMessage does
//    not abort/throw), so the null sub-cases cannot be ported as EXPECT_THROW and adding a null
//    child is unsafe — the null sub-assertions are skipped.
//  - CTS testOnStateChange asserts that, with transitions registered, getCurrent() is the transition
//    drawable while the transition runs. That depends on a live Choreographer/animation loop driving
//    the Transition; the selection is runtime-dependent, so only the keyframe-matching half is
//    ported (transitions omitted).
//
// addState(stateSet, drawable, id) takes an explicit keyframe id (CTS passes R.id.*); arbitrary
// unique ints stand in, mirroring how cts_statelistdrawable_test.cc uses arbitrary state ints.
// getChildCount()/getChild(i) come from DrawableContainer (public) via StateListDrawable.
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/AnimatedStateListDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <memory>
#include <drawable/animatedstatelistdrawable.h>
#include <drawable/drawables.h>
#include <drawable/stateset.h>
#include <core/app.h>
#include <guienvironment.h>

using namespace cdroid;

namespace {
// Minimal Animatable drawable (CTS MockTransition = MockDrawable implements Animatable, Animatable2).
// CDROID's Animatable is a pure interface (no Drawable base), so MI from Drawable + Animatable mirrors
// CTS without a diamond. Used as the transition payload for addTransition.
class MockAnimatableDrawable : public Drawable, public Animatable {
public:
    void draw(Canvas&) override {}
    void start() override {}
    void stop() override {}
    bool isRunning() override { return false; }
};
} // namespace

// Empty fixture — pure logic, no per-case setup. App/Context is provided process-wide by
// GUIEnvironment (see guienvironment.h).
class CtsAnimatedStateListDrawableTest : public testing::Test {};

TEST_F(CtsAnimatedStateListDrawableTest, testAnimatedStateListDrawable) {
    // CTS: testStateListDrawable — constructor leaves a non-null ConstantState.
    AnimatedStateListDrawable asld;
    EXPECT_NE(nullptr, asld.getConstantState());
}

TEST_F(CtsAnimatedStateListDrawableTest, testAddState) {
    AnimatedStateListDrawable asld;
    EXPECT_EQ(0, asld.getChildCount());

    // CTS expects IllegalArgumentException when the drawable is null. CDROID's addState FATAL_IF-logs
    // without throwing and would add an unsafe null child, so the null sub-case is NOT ported.

    constexpr int ID_FOCUSED = 1;
    constexpr int ID_UNFOCUSED = 2;
    const std::vector<int> STATE_FOCUSED = {1}; // arbitrary unique state int (no framework R.attr.*)

    Drawable* unfocused = new ColorDrawable(0xFF0000FF);
    asld.addState(StateSet::WILD_CARD, unfocused, ID_FOCUSED);
    EXPECT_EQ(1, asld.getChildCount());

    Drawable* focused = new ColorDrawable(0xFFFF0000);
    asld.addState(STATE_FOCUSED, focused, ID_UNFOCUSED);
    EXPECT_EQ(2, asld.getChildCount());
}

TEST_F(CtsAnimatedStateListDrawableTest, testAddTransition) {
    AnimatedStateListDrawable asld;

    constexpr int ID_FOCUSED = 1;
    constexpr int ID_UNFOCUSED = 2;
    const std::vector<int> STATE_FOCUSED = {1};

    Drawable* focused = new ColorDrawable(0xFFFF0000);
    Drawable* unfocused = new ColorDrawable(0xFF0000FF);
    asld.addState(STATE_FOCUSED, focused, ID_FOCUSED);
    asld.addState(StateSet::WILD_CARD, unfocused, ID_UNFOCUSED);
    ASSERT_EQ(2, asld.getChildCount());

    // CTS expects IllegalArgumentException when the transition is null; skipped (see file header).
    asld.addTransition(ID_FOCUSED, ID_UNFOCUSED, new MockAnimatableDrawable(), false);
    EXPECT_EQ(3, asld.getChildCount());

    asld.addTransition(ID_UNFOCUSED, ID_FOCUSED, new MockAnimatableDrawable(), false);
    EXPECT_EQ(4, asld.getChildCount());

    asld.addTransition(ID_FOCUSED, ID_UNFOCUSED, new MockAnimatableDrawable(), true);
    EXPECT_EQ(5, asld.getChildCount());
}

TEST_F(CtsAnimatedStateListDrawableTest, testIsStateful) {
    // AnimatedStateListDrawable is always stateful (it overrides isStateful() to return true).
    EXPECT_TRUE(AnimatedStateListDrawable().isStateful());
}

TEST_F(CtsAnimatedStateListDrawableTest, testOnStateChangeKeyframeMatch) {
    // CTS testOnStateChange also registers transitions and asserts getCurrent() == the transition
    // drawable while it runs. That half is runtime/animation-loop dependent (see file header) and is
    // omitted; this case exercises the state -> keyframe-drawable matching that ASLD inherits from
    // StateListDrawable (selectTransition returns false when no transition is registered, so
    // selectDrawable lands on the matched keyframe).
    AnimatedStateListDrawable asld;

    constexpr int ID_FOCUSED = 1;
    constexpr int ID_UNFOCUSED = 2;
    const std::vector<int> STATE_FOCUSED = {1};
    const std::vector<int> STATE_EMPTY = {};

    Drawable* focused = new ColorDrawable(0xFFFF0000);
    Drawable* unfocused = new ColorDrawable(0xFF0000FF);
    asld.addState(STATE_FOCUSED, focused, ID_FOCUSED);
    asld.addState(StateSet::WILD_CARD, unfocused, ID_UNFOCUSED);

    // empty state -> wild card keyframe (unfocused)
    asld.setState(STATE_EMPTY);
    EXPECT_EQ(unfocused, asld.getCurrent());

    // focused state -> focused keyframe
    asld.setState(STATE_FOCUSED);
    EXPECT_EQ(focused, asld.getCurrent());

    // back to empty -> unfocused again
    asld.setState(STATE_EMPTY);
    EXPECT_EQ(unfocused, asld.getCurrent());
}
