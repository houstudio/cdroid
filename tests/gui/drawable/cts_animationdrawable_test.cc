// AOSP CTS drawable test port (AnimationDrawableTest.java). Pure-logic cases only.
//
// The CTS file drives a real animation from an Activity (polling for frame advances across
// multiple seconds) and verifies frame timing/pixels. Those cases — testSetVisible, testStart's
// polling clauses, testRun's polling, testUnscheduleSelf, testAccessOneShot's repeat verification,
// and the frame-pixel comparisons in testGetFrame — are NOT ported: they require the UI thread +
// real wall-clock frame scheduling, not pure logic. Here we verify the synchronous contracts:
// addFrame/getNumberOfFrames/getFrame/getDuration/isOneShot/setOneShot/selectDrawable and the
// start/stop/run isRunning() transitions (without waiting for frame advances).
//
// Other NOT-ported cases:
//   - testInflate* (resource XML + null-arg NullPointerException variants).
//   - testGetDurationTooLow/TooHigh: CDROID indexes mDurations via std::vector::operator[] (no
//     bounds check), so out-of-range is undefined behavior, not a throw — cannot EXPECT_THROW.
//
// CDROID divergences (faithfully reflected):
//   - getFrame(i) routes through DrawableContainerState::getChild which uses std::vector::at(), so
//     out-of-range throws std::out_of_range (CTS expects ArrayIndexOutOfBoundsException). The throw
//     cases assert std::out_of_range. The CTS clause `assertNull(getFrame(count))` is omitted:
//     CDROID throws rather than returning null at index==count.
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/AnimationDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <stdexcept>
#include <drawable/animationdrawable.h>
#include <drawable/drawables.h>
#include <drawable/drawablecontainer.h>
#include <core/color.h>
#include <guienvironment.h>

using namespace cdroid;

namespace {
// Populates `d` with 3 frames matching CTS's FRAMES_COUNT/durations (colors stand in for the png
// frames — only identity/count/duration matter, never pixels). AnimationDrawable is not reliably
// copyable/movable (user-declared dtor suppresses implicit move; implicit copy would shallow-share
// the shared_ptr state), so we populate a local in place rather than returning by value.
void addThreeFrames(AnimationDrawable& d) {
    d.addFrame(new ColorDrawable(Color::RED), 3000);
    d.addFrame(new ColorDrawable(Color::GREEN), 2000);
    d.addFrame(new ColorDrawable(Color::BLUE), 1000);
}
} // namespace

class CtsAnimationDrawableTest : public testing::Test {};

// CTS testConstructor.
TEST_F(CtsAnimationDrawableTest, testConstructor) {
    AnimationDrawable d;
    EXPECT_NE(nullptr, d.getConstantState());
    EXPECT_FALSE(d.isRunning());
    EXPECT_FALSE(d.isOneShot());
}

// CTS testGetNumberOfFrames. The CTS `addFrame(null, …)` clause (expects NullPointerException) is
// omitted — null-arg behavior is out of scope here. CTS also re-adds the *same* Drawable instance
// to show addFrame does not de-dup; that is omitted because CDROID's DrawableContainerState owns
// its children (~dtor does `delete` per child), so adding one pointer twice would double-free.
TEST_F(CtsAnimationDrawableTest, testGetNumberOfFrames) {
    AnimationDrawable d;
    addThreeFrames(d);
    ASSERT_EQ(3, d.getNumberOfFrames());

    d.addFrame(new ColorDrawable(Color::YELLOW), 2000);
    EXPECT_EQ(4, d.getNumberOfFrames());

    d.addFrame(new ColorDrawable(Color::MAGENTA), 2000);
    EXPECT_EQ(5, d.getNumberOfFrames());
}

// CTS testGetFrame. The CTS clause `assertNull(getFrame(FRAMES_COUNT))` is omitted: CDROID's
// getChild() uses std::vector::at() and throws std::out_of_range at index==count rather than
// returning null (see testGetFrameTooHigh).
TEST_F(CtsAnimationDrawableTest, testGetFrame) {
    AnimationDrawable d;
    addThreeFrames(d);
    EXPECT_EQ(3, d.getNumberOfFrames());

    EXPECT_NE(nullptr, d.getFrame(0));
    EXPECT_NE(nullptr, d.getFrame(1));
    EXPECT_NE(nullptr, d.getFrame(2));
    // Frames are distinct children.
    EXPECT_NE(d.getFrame(0), d.getFrame(1));
    EXPECT_NE(d.getFrame(1), d.getFrame(2));
}

// CTS testGetFrameTooLow. CDROID throws std::out_of_range (CTS: ArrayIndexOutOfBoundsException).
TEST_F(CtsAnimationDrawableTest, testGetFrameTooLow) {
    AnimationDrawable d;
    addThreeFrames(d);
    EXPECT_THROW(d.getFrame(-1), std::out_of_range);
}

// CTS testGetFrameTooHigh.
TEST_F(CtsAnimationDrawableTest, testGetFrameTooHigh) {
    AnimationDrawable d;
    addThreeFrames(d);
    EXPECT_THROW(d.getFrame(10), std::out_of_range);
}

// CTS testGetDuration. The CTS clause `getDuration(FRAMES_COUNT)==0` is omitted: CDROID indexes
// mDurations via operator[] (no bounds check), so out-of-range access is undefined behavior.
TEST_F(CtsAnimationDrawableTest, testGetDuration) {
    AnimationDrawable d;
    addThreeFrames(d);
    EXPECT_EQ(3000, d.getDuration(0));
    EXPECT_EQ(2000, d.getDuration(1));
    EXPECT_EQ(1000, d.getDuration(2));
}

// CTS testAccessOneShot (logic subset). The repeat/no-repeat verification requires real frame
// scheduling across seconds and is not ported; only the setOneShot/isOneShot contract is.
TEST_F(CtsAnimationDrawableTest, testAccessOneShot) {
    AnimationDrawable d;
    EXPECT_FALSE(d.isOneShot());

    d.setOneShot(true);
    EXPECT_TRUE(d.isOneShot());

    d.setOneShot(false);
    EXPECT_FALSE(d.isOneShot());
}

// CTS testStart (logic subset). Verifies the synchronous isRunning/getCurrent transitions; the
// polling clauses that wait for the second/third frame are not ported.
TEST_F(CtsAnimationDrawableTest, testStart) {
    AnimationDrawable d;
    addThreeFrames(d);
    EXPECT_FALSE(d.isRunning());

    d.start();
    EXPECT_TRUE(d.isRunning());
    EXPECT_EQ(d.getFrame(0), d.getCurrent());

    // stop() clears the running flag synchronously.
    d.stop();
    EXPECT_FALSE(d.isRunning());

    // stop() when not running is a no-op (must not throw / flip state).
    d.stop();
    EXPECT_FALSE(d.isRunning());
}

// CTS testRun (logic subset). run() kicks the animation; unscheduling stops it. The polling clause
// is not ported.
TEST_F(CtsAnimationDrawableTest, testRun) {
    AnimationDrawable d;
    addThreeFrames(d);
    EXPECT_FALSE(d.isRunning());

    // CDROID's AnimationDrawable::run() is protected (AOSP's is public); start() is the public
    // Animatable entry point that drives the frame loop, so use it here.
    d.start();
    EXPECT_TRUE(d.isRunning());

    d.stop();
    EXPECT_FALSE(d.isRunning());
}

// selectDrawable is inherited from DrawableContainer and underpins frame selection; CTS exercises
// it only indirectly via start/run. This case pins its contract directly: a valid index selects the
// child and reports a change; a repeat index reports no change; an out-of-range index is a no-op
// (returns true but leaves no current drawable).
TEST_F(CtsAnimationDrawableTest, testSelectDrawable) {
    AnimationDrawable d;
    addThreeFrames(d);

    EXPECT_TRUE(d.selectDrawable(1));
    EXPECT_EQ(d.getFrame(1), d.getCurrent());

    // Re-selecting the same index is a no-op (returns false).
    EXPECT_FALSE(d.selectDrawable(1));
    EXPECT_EQ(d.getFrame(1), d.getCurrent());

    EXPECT_TRUE(d.selectDrawable(2));
    EXPECT_EQ(d.getFrame(2), d.getCurrent());
}

// CTS testMutate (simplified). CTS inflates a sibling from a resource and mutates it; CDROID has no
// such asset wired here, so this verifies the programmatic contract: mutate() returns self and
// does not throw.
TEST_F(CtsAnimationDrawableTest, testMutate) {
    AnimationDrawable d;
    addThreeFrames(d);
    EXPECT_EQ(&d, d.mutate());
    EXPECT_EQ(3, d.getNumberOfFrames());
}

// CTS testGetTotalDuration (CDROID exposes this getter; not asserted separately in CTS but it is
// the sum of all frame durations — a pure-logic invariant worth pinning).
TEST_F(CtsAnimationDrawableTest, testGetTotalDuration) {
    AnimationDrawable d;
    addThreeFrames(d);
    EXPECT_EQ(3000 + 2000 + 1000, d.getTotalDuration());
}
