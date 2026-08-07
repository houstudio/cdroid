// AOSP CTS drawable test port (StateListDrawableTest.java). Programmatic cases only — the
// density/preload cases (testPreloadDensity*) need bitmap density scaling and are NOT ported.
// Resource cases (testInflate/testMutate) are added separately once the selector XML assets are in
// the pak. CDROID has no framework android.R.attr.state_* int constants wired into the test, so
// state sets use arbitrary unique ints (the matching logic under test is value-agnostic); WILD_CARD
// is StateSet::WILD_CARD (== {}). mDrawableContainerState.getChildCount()/getChildren() are replaced
// by DrawableContainer's public getChildCount()/getChild(i).
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/StateListDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <memory>
#include <drawable/drawables.h>
#include <drawable/statelistdrawable.h>
#include <core/app.h>
#include <drawable/stateset.h>
#include <guienvironment.h>

using namespace cdroid;

namespace {
// Tracks onStateChange invocations (CTS MockStateListDrawable). onStateChange is protected in the
// base; overriding it public lets the testOnStateChange case invoke it directly.
class MockStateListDrawable : public StateListDrawable {
public:
    bool mHasCalledOnStateChanged = false;
    bool hasCalledOnStateChanged() const { return mHasCalledOnStateChanged; }
    void reset() { mHasCalledOnStateChanged = false; }
    bool onStateChange(const std::vector<int>& stateSet) override {
        const bool r = StateListDrawable::onStateChange(stateSet);
        mHasCalledOnStateChanged = true;
        return r;
    }
};
} // namespace

class CtsStateListDrawableTest : public testing::Test {
protected:
    std::unique_ptr<MockStateListDrawable> mDrawable;
    void SetUp() override { mDrawable.reset(new MockStateListDrawable()); }
};

TEST_F(CtsStateListDrawableTest, testStateListDrawable) {
    StateListDrawable* d = new StateListDrawable();
    EXPECT_NE(nullptr, d->getConstantState());
    delete d;
}

TEST_F(CtsStateListDrawableTest, testGetConstantState) {
    // Default-constructed (AOSP inflates from a selector resource); the constant-state contract
    // is default-instance logic and ports without the resource. StateListState inherits
    // DrawableContainer's contract: newDrawable yields a distinct instance backed by the same
    // constant state.
    StateListDrawable drawable;
    auto constantState = drawable.getConstantState();
    ASSERT_NE(nullptr, constantState);

    Drawable* copy = constantState->newDrawable();
    ASSERT_NE(nullptr, copy);
    EXPECT_NE(&drawable, copy);
    delete copy;
}

TEST_F(CtsStateListDrawableTest, testGetChangingConfigurations) {
    // Mirrors CtsVectorDrawableTest. DrawableContainer syncs the instance changingConfigurations
    // into the state snapshot inside getConstantState(), and ORs instance | state in
    // getChangingConfigurations(). Default-constructed instance; no resource needed.
    StateListDrawable drawable;
    auto constantState = drawable.getConstantState();

    // default
    ASSERT_NE(nullptr, constantState);
    EXPECT_EQ(0, constantState->getChangingConfigurations());
    EXPECT_EQ(0, drawable.getChangingConfigurations());

    // changing the drawable's configuration does not affect the cached state's snapshot
    drawable.setChangingConfigurations(0xff);
    EXPECT_EQ(0xff, drawable.getChangingConfigurations());
    EXPECT_EQ(0, constantState->getChangingConfigurations());

    // re-fetching the constant state reflects the new value
    constantState = drawable.getConstantState();
    EXPECT_EQ(0xff, constantState->getChangingConfigurations());

    // set a new configuration to the drawable; drawable ORs with the state's value
    drawable.setChangingConfigurations(0xff00);
    EXPECT_EQ(0xff, constantState->getChangingConfigurations());
    EXPECT_EQ(0xffff, drawable.getChangingConfigurations());
}

TEST_F(CtsStateListDrawableTest, testAddState) {
    EXPECT_EQ(0, mDrawable->getChildCount());

    // nothing happens if drawable is null
    mDrawable->reset();
    mDrawable->addState(StateSet::WILD_CARD, nullptr);
    EXPECT_EQ(0, mDrawable->getChildCount());
    EXPECT_FALSE(mDrawable->hasCalledOnStateChanged());

    // adding a real drawable selects it (onStateChange fires)
    mDrawable->reset();
    mDrawable->addState(StateSet::WILD_CARD, new ColorDrawable(0xFFFFFF00));
    EXPECT_EQ(1, mDrawable->getChildCount());
    EXPECT_TRUE(mDrawable->hasCalledOnStateChanged());

    mDrawable->reset();
    mDrawable->addState({1, -2}, new ColorDrawable(0xFFFFFF00));
    EXPECT_EQ(2, mDrawable->getChildCount());
    EXPECT_TRUE(mDrawable->hasCalledOnStateChanged());

    mDrawable->reset();
    mDrawable->addState(StateSet::WILD_CARD, new ColorDrawable(0xFFFFFF00));
    EXPECT_EQ(3, mDrawable->getChildCount());
    EXPECT_TRUE(mDrawable->hasCalledOnStateChanged());
}

TEST_F(CtsStateListDrawableTest, testIsStateful) {
    EXPECT_TRUE(StateListDrawable().isStateful());
}

TEST_F(CtsStateListDrawableTest, testGetStateCount) {
    StateListDrawable stateList;
    stateList.addState({0}, new ColorDrawable(0xFFFF0000));
    EXPECT_EQ(1, stateList.getStateCount());
    stateList.addState({1}, new ColorDrawable(0xFF00FF00));
    EXPECT_EQ(2, stateList.getStateCount());
    stateList.addState({2}, new ColorDrawable(0xFF0000FF));
    EXPECT_EQ(3, stateList.getStateCount());
}

TEST_F(CtsStateListDrawableTest, testGetStateDrawable) {
    StateListDrawable stateList;
    ColorDrawable* colorDrawable = new ColorDrawable(0xFFFF0000);
    stateList.addState({1}, colorDrawable);
    EXPECT_EQ(colorDrawable, stateList.getStateDrawable(0));
}

TEST_F(CtsStateListDrawableTest, testGetStateSet) {
    StateListDrawable stateList;
    ColorDrawable* colorDrawable = new ColorDrawable(0xFF00FF00);
    const std::vector<int> stateSet = {0};
    stateList.addState(stateSet, colorDrawable);
    const std::vector<int>& resolved = stateList.getStateSet(0);
    EXPECT_EQ(stateSet, resolved);
}

TEST_F(CtsStateListDrawableTest, testGetStateDrawableIndex) {
    StateListDrawable stateList;
    ColorDrawable* d1 = new ColorDrawable(0xFF00FFFF);
    ColorDrawable* d2 = new ColorDrawable(0xFFFFFF00);
    ColorDrawable* d3 = new ColorDrawable(0xFF00FF00);
    const std::vector<int> s1 = {42}, s2 = {27}, s3 = {57};
    stateList.addState(s1, d1);
    stateList.addState(s2, d2);
    stateList.addState(s3, d3);
    EXPECT_EQ(0, stateList.getStateDrawableIndex(s1));
    EXPECT_EQ(1, stateList.getStateDrawableIndex(s2));
    EXPECT_EQ(2, stateList.getStateDrawableIndex(s3));
}

TEST_F(CtsStateListDrawableTest, testOnStateChange) {
    mDrawable->addState({1, -2}, new ColorDrawable(0xFFFF00FF));
    mDrawable->addState(StateSet::WILD_CARD, new ColorDrawable(0xFFFFFF00));
    mDrawable->addState(StateSet::WILD_CARD, new ColorDrawable(0xFFFFFF00));

    // same state set again → onStateChange not called
    mDrawable->reset();
    mDrawable->setState(mDrawable->getState());
    EXPECT_FALSE(mDrawable->hasCalledOnStateChanged());

    // different state → onStateChange called
    mDrawable->reset();
    mDrawable->setState({1, -2});
    EXPECT_TRUE(mDrawable->hasCalledOnStateChanged());

    mDrawable->reset();
    mDrawable->setState(StateSet::WILD_CARD);
    EXPECT_TRUE(mDrawable->hasCalledOnStateChanged());

    // the focused+!selected set matches child[0]
    mDrawable->onStateChange({1, -2});
    EXPECT_EQ(mDrawable->getCurrent(), mDrawable->getChild(0));

    // a less-specific set still resolves via the wild card at child[1]
    mDrawable->onStateChange(StateSet::WILD_CARD);
    EXPECT_EQ(mDrawable->getCurrent(), mDrawable->getChild(1));
}

TEST_F(CtsStateListDrawableTest, testOnStateChangeWithWildCardAtFirst) {
    mDrawable->addState(StateSet::WILD_CARD, new ColorDrawable(0xFFFFFF00));
    mDrawable->addState({1, -2}, new ColorDrawable(0xFFFFFF00));
    // the first wild card matches although the second is more specific
    mDrawable->onStateChange({1, -2});
    EXPECT_EQ(mDrawable->getCurrent(), mDrawable->getChild(0));
}

TEST_F(CtsStateListDrawableTest, testInflate) {
    // cts_selector_correct.xml: <selector visible=false constantSize=true variablePadding=true>
    // with item[0]=focused+!pressed and a default item[1] (png refs replaced by <color>).
    Drawable* d = App::getInstance().getDrawable("@drawable/cts_selector_correct");
    ASSERT_NE(nullptr, d);
    auto* sd = dynamic_cast<StateListDrawable*>(d);
    ASSERT_NE(nullptr, sd);
    EXPECT_EQ(2, sd->getChildCount());
    // item[0]'s state set selects child[0]
    sd->setState(sd->getStateSet(0));
    EXPECT_EQ(sd->getCurrent(), sd->getChild(0));
    // the default (wild card) item selects child[1]
    sd->setState(StateSet::WILD_CARD);
    EXPECT_EQ(sd->getCurrent(), sd->getChild(1));
}

TEST_F(CtsStateListDrawableTest, testMutate) {
    mDrawable->addState({1}, new ColorDrawable(0xFFFF0000));
    mDrawable->addState({2}, new ColorDrawable(0xFF00FF00));
    // mutate() must succeed (return non-null) and not throw.
    EXPECT_NE(nullptr, mDrawable->mutate());
}
