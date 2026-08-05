// AOSP CTS drawable test port (LevelListDrawableTest.java). Programmatic cases only — the
// null-argument inflate variants (testInflateWithNull*, testInflateMissingContent) and the
// resource-based inflate/mutate cases are added separately once the level-list XML assets are in
// the pak. mDrawableContainerState.getChildCount()/getChildren() are replaced by DrawableContainer's
// public getChildCount()/getChild(i).
//
// Original: cts/tests/tests/graphics/src/android/graphics/drawable/cts/LevelListDrawableTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <memory>
#include <limits.h>
#include <drawable/drawables.h>
#include <drawable/levellistdrawable.h>
#include <core/app.h>
#include <guienvironment.h>

using namespace cdroid;

namespace {
// Tracks onLevelChange invocations (CTS MockLevelListDrawable). onLevelChange is protected; the
// public override lets testOnLevelChange call it directly.
class MockLevelListDrawable : public LevelListDrawable {
public:
    bool mHasCalledOnLevelChanged = false;
    bool hasCalledOnLevelChanged() const { return mHasCalledOnLevelChanged; }
    void reset() { mHasCalledOnLevelChanged = false; }
    bool onLevelChange(int level) override {
        const bool r = LevelListDrawable::onLevelChange(level);
        mHasCalledOnLevelChanged = true;
        return r;
    }
};
} // namespace

class CtsLevelListDrawableTest : public testing::Test {
protected:
    std::unique_ptr<MockLevelListDrawable> mDrawable;
    void SetUp() override { mDrawable.reset(new MockLevelListDrawable()); }
};

TEST_F(CtsLevelListDrawableTest, testLevelListDrawable) {
    LevelListDrawable* d = new LevelListDrawable();
    EXPECT_NE(nullptr, d->getConstantState());
    delete d;
}

TEST_F(CtsLevelListDrawableTest, testAddLevel) {
    EXPECT_EQ(0, mDrawable->getChildCount());

    // nothing happens if drawable is null
    mDrawable->reset();
    mDrawable->addLevel(0, 0, nullptr);
    EXPECT_EQ(0, mDrawable->getChildCount());
    EXPECT_FALSE(mDrawable->hasCalledOnLevelChanged());

    // adding a real drawable selects it (onLevelChange fires)
    mDrawable->reset();
    mDrawable->addLevel(INT_MAX, INT_MIN, new ColorDrawable(0xFF00FF00));
    EXPECT_EQ(1, mDrawable->getChildCount());
    EXPECT_TRUE(mDrawable->hasCalledOnLevelChanged());

    mDrawable->reset();
    mDrawable->addLevel(INT_MIN, INT_MAX, new ColorDrawable(0xFFFF0000));
    EXPECT_EQ(2, mDrawable->getChildCount());
    EXPECT_TRUE(mDrawable->hasCalledOnLevelChanged());
}

TEST_F(CtsLevelListDrawableTest, testOnLevelChange) {
    mDrawable->addLevel(0, 0, new ColorDrawable(0xFF0000FF));
    mDrawable->addLevel(0, 0, new ColorDrawable(0xFFFF00FF));
    mDrawable->addLevel(0, 10, new ColorDrawable(0xFFFFFF00));

    // same level → onLevelChange not called
    mDrawable->reset();
    mDrawable->setLevel(mDrawable->getLevel());
    EXPECT_FALSE(mDrawable->hasCalledOnLevelChanged());

    // different level → onLevelChange called
    mDrawable->reset();
    mDrawable->setLevel(mDrawable->getLevel() - 1);
    EXPECT_TRUE(mDrawable->hasCalledOnLevelChanged());

    // level 10 matches the [0,10] child at index 2
    mDrawable->onLevelChange(10);
    EXPECT_EQ(mDrawable->getCurrent(), mDrawable->getChild(2));

    // level 0 matches the first [0,0] child at index 0
    mDrawable->onLevelChange(0);
    EXPECT_EQ(mDrawable->getCurrent(), mDrawable->getChild(0));

    // level 100 matches nothing → current is null
    mDrawable->onLevelChange(100);
    EXPECT_EQ(nullptr, mDrawable->getCurrent());
}

TEST_F(CtsLevelListDrawableTest, testInflate) {
    // cts_level_list_correct.xml: [100,200]→child[0], [200,300]→child[1] (png refs → <color>).
    Drawable* d = App::getInstance().getDrawable("@drawable/cts_level_list_correct");
    ASSERT_NE(nullptr, d);
    auto* ld = dynamic_cast<LevelListDrawable*>(d);
    ASSERT_NE(nullptr, ld);
    EXPECT_EQ(2, ld->getChildCount());
    ld->setLevel(150);
    EXPECT_EQ(ld->getCurrent(), ld->getChild(0));
    ld->setLevel(250);
    EXPECT_EQ(ld->getCurrent(), ld->getChild(1));
    ld->setLevel(50);
    EXPECT_EQ(nullptr, ld->getCurrent());
}

TEST_F(CtsLevelListDrawableTest, testMutate) {
    mDrawable->addLevel(0, 10, new ColorDrawable(0xFF0000FF));
    EXPECT_NE(nullptr, mDrawable->mutate());
}
