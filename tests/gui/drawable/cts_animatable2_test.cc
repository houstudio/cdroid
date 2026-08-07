// AOSP CTS drawable test port (Animatable2_AnimationCallbackTest.java) plus a register/unregister
// round-trip case for Animatable2's callback registry (the task brief's "register/unregister
// AnimationCallback" coverage, which CTS only covers indirectly elsewhere).
//
// CDROID notes:
//  - Animatable2 lives in drawable/drawable.h (nested as a top-level class, not nested in Drawable).
//  - Animatable2::AnimationCallback derives from EventSet and exposes onAnimationStart/onAnimationEnd
//    as CallbackBase<void,Drawable&> members (callable functors, NOT virtual methods). An empty
//    callback's functor is a default-constructed std::function -> invoking it is a no-op (matches
//    CTS's "these are no-op methods, just make sure they don't crash"). CTS passes null; CDROID's
//    signature takes a Drawable& so a dummy drawable is supplied instead — the no-crash intent is
//    preserved.
//  - registerAnimationCallback/unregisterAnimationCallback are pure-virtual on Animatable2. The CTS
//    AnimationCallback test doesn't exercise them, so a minimal concrete Animatable2 (storing
//    callbacks in a vector) is provided to validate register/unregister identity semantics. EventSet
//    identity (shared mID) is what makes unregister find the originally-registered callback even
//    though the registry stores a copy.
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/Animatable2_AnimationCallbackTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <vector>
#include <drawable/drawable.h>
#include <drawable/drawables.h>
#include <core/app.h>
#include <guienvironment.h>

using namespace cdroid;

namespace {
// Minimal concrete Drawable (CTS MockDrawable). Only draw() is pure-virtual in CDROID's Drawable.
class MockDrawable : public Drawable {
public:
    void draw(Canvas&) override {}
};

// Minimal concrete Animatable2 so register/unregisterAnimationCallback can be exercised. CTS's own
// AnimationCallback test does not need this; it is added here to cover the registry contract.
class MockAnimatable2 : public Animatable2 {
public:
    void start() override {}
    void stop() override {}
    bool isRunning() override { return false; }
    void registerAnimationCallback(const AnimationCallback& callback) override {
        mCallbacks.push_back(callback); // copy shares the EventSet identity (mID)
    }
    bool unregisterAnimationCallback(const AnimationCallback& callback) override {
        for (auto it = mCallbacks.begin(); it != mCallbacks.end(); ++it) {
            if (*it == callback) { // EventSet::operator== compares mID
                mCallbacks.erase(it);
                return true;
            }
        }
        return false;
    }
    size_t callbackCount() const { return mCallbacks.size(); }
private:
    std::vector<AnimationCallback> mCallbacks;
};
} // namespace

class CtsAnimatable2Test : public testing::Test {};

TEST_F(CtsAnimatable2Test, testCallback) {
    // CTS: AnimationCallback's onAnimationStart/onAnimationEnd are no-ops; ensure they don't crash.
    // CDROID's AnimationCallback holds them as CallbackBase members; an empty callback invokes an
    // empty std::function (no-op). CTS passes null; CDROID's signature takes Drawable&, so a dummy
    // drawable stands in.
    Animatable2::AnimationCallback callback;
    MockDrawable drawable;
    callback.onAnimationStart(drawable);
    callback.onAnimationEnd(drawable);
    SUCCEED();
}

TEST_F(CtsAnimatable2Test, testRegisterUnregisterAnimationCallback) {
    // Exercises the Animatable2 callback-registry contract: register adds, unregister removes by
    // identity, unregister of an unknown callback returns false and leaves the registry intact.
    MockAnimatable2 a;
    Animatable2::AnimationCallback cb;
    EXPECT_EQ(0u, a.callbackCount());

    a.registerAnimationCallback(cb);
    EXPECT_EQ(1u, a.callbackCount());

    // A second, independently-constructed callback has a distinct EventSet identity.
    Animatable2::AnimationCallback other;
    EXPECT_FALSE(a.unregisterAnimationCallback(other));
    EXPECT_EQ(1u, a.callbackCount());

    // Removing the originally-registered callback succeeds (EventSet identity is shared by the copy).
    EXPECT_TRUE(a.unregisterAnimationCallback(cb));
    EXPECT_EQ(0u, a.callbackCount());

    // Removing it again fails (already gone).
    EXPECT_FALSE(a.unregisterAnimationCallback(cb));
    EXPECT_EQ(0u, a.callbackCount());
}
