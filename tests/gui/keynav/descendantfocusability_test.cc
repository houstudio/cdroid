/*********************************************************************************
 * Port of AOSP coretests android.widget.focus.DescendantFocusabilityTest, with
 * the DescendantFocusability companion activity (and its
 * descendant_focusability.xml: three vertical LinearLayouts flagged
 * beforeDescendants / afterDescendants / blocksDescendants, each holding one
 * Button) rebuilt programmatically. Source of truth:
 * frameworks/base/core/tests/coretests/src/android/widget/focus/DescendantFocusability{,Test}.java
 * res/layout/descendant_focusability.xml
 *********************************************************************************/
#include <gtest/gtest.h>
#include <cdroid.h>
#include <guienvironment.h>
#include "keysender.h"

using namespace cdroid;

class DescendantFocusabilityTest : public testing::Test {
protected:
    Window* mActivity;

    LinearLayout* mRoot;
    LinearLayout* mBeforeDescendants;
    LinearLayout* mAfterDescendants;
    LinearLayout* mBlocksDescendants;
    Button* mBeforeDescendantsChild;
    Button* mAfterDescendantsChild;
    Button* mBlocksDescendantsChild;

    void SetUp() override {
        App& app = App::getInstance();
        mActivity = new Window(&app, 0, 0, -1, -1);

        mRoot = new LinearLayout(&app);
        mRoot->setOrientation(LinearLayout::VERTICAL);
        mRoot->setLayoutParams(new ViewGroup::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT,
                ViewGroup::LayoutParams::MATCH_PARENT));

        mBeforeDescendants = addGroup(ViewGroup::FOCUS_BEFORE_DESCENDANTS, &mBeforeDescendantsChild);
        mAfterDescendants = addGroup(ViewGroup::FOCUS_AFTER_DESCENDANTS, &mAfterDescendantsChild);
        mBlocksDescendants = addGroup(ViewGroup::FOCUS_BLOCK_DESCENDANTS, &mBlocksDescendantsChild);

        mActivity->addView(mRoot);
        const int w = mActivity->getWidth() > 0 ? mActivity->getWidth() : 1080;
        const int h = mActivity->getHeight() > 0 ? mActivity->getHeight() : 1920;
        mRoot->measure(MeasureSpec::makeMeasureSpec(w, MeasureSpec::EXACTLY),
                       MeasureSpec::makeMeasureSpec(h, MeasureSpec::EXACTLY));
        mRoot->layout(0, 0, w, h);
        pumpUntilIdle();
    }

    LinearLayout* addGroup(int descendantFocusability, Button** outChild) {
        App& app = App::getInstance();
        LinearLayout* group = new LinearLayout(&app);
        group->setOrientation(LinearLayout::VERTICAL);
        group->setLayoutParams(new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT,
                ViewGroup::LayoutParams::WRAP_CONTENT));
        group->setDescendantFocusability(descendantFocusability);

        Button* child = new Button(&app);
        child->setLayoutParams(new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT,
                ViewGroup::LayoutParams::WRAP_CONTENT));
        group->addView(child);

        mRoot->addView(group);
        *outChild = child;
        return group;
    }
};

TEST_F(DescendantFocusabilityTest, testPreconditions) {
    EXPECT_EQ((int)ViewGroup::FOCUS_BEFORE_DESCENDANTS, mBeforeDescendants->getDescendantFocusability());
    EXPECT_EQ((int)ViewGroup::FOCUS_AFTER_DESCENDANTS, mAfterDescendants->getDescendantFocusability());
    EXPECT_EQ((int)ViewGroup::FOCUS_BLOCK_DESCENDANTS, mBlocksDescendants->getDescendantFocusability());

    EXPECT_TRUE(mBeforeDescendantsChild->isFocusable());
    EXPECT_TRUE(mAfterDescendantsChild->isFocusable());
    EXPECT_TRUE(mBlocksDescendantsChild->isFocusable());
}

TEST_F(DescendantFocusabilityTest, testBeforeDescendants) {
    mBeforeDescendants->setFocusable(true);

    EXPECT_TRUE(mBeforeDescendants->requestFocus());
    EXPECT_TRUE(mBeforeDescendants->isFocused());

    mBeforeDescendants->setFocusable(false);
    mBeforeDescendants->requestFocus();
    EXPECT_TRUE(mBeforeDescendantsChild->isFocused());
}

TEST_F(DescendantFocusabilityTest, testAfterDescendants) {
    mAfterDescendants->setFocusable(true);

    EXPECT_TRUE(mAfterDescendants->requestFocus());
    EXPECT_TRUE(mAfterDescendantsChild->isFocused());

    mAfterDescendants->setFocusable(false);
    EXPECT_TRUE(mAfterDescendants->requestFocus());
    EXPECT_TRUE(mAfterDescendantsChild->isFocused());
}

TEST_F(DescendantFocusabilityTest, testBlocksDescendants) {
    mBlocksDescendants->setFocusable(true);
    EXPECT_TRUE(mBlocksDescendants->requestFocus());
    EXPECT_TRUE(mBlocksDescendants->isFocused());
    EXPECT_FALSE(mBlocksDescendantsChild->isFocused());

    mBlocksDescendants->setFocusable(false);
    EXPECT_FALSE(mBlocksDescendants->requestFocus());
    EXPECT_FALSE(mBlocksDescendants->isFocused());
    EXPECT_FALSE(mBlocksDescendantsChild->isFocused());
}

TEST_F(DescendantFocusabilityTest, testChildOfDescendantBlockerRequestFocusFails) {
    EXPECT_FALSE(mBlocksDescendantsChild->requestFocus());
}

/* Needs TouchUtils.clickView (real touch injection to enter touch mode) — not
   available in this headless harness. */
TEST_F(DescendantFocusabilityTest, testBeforeDescendantsEnterTouchMode) {
    GTEST_SKIP() << "needs TouchUtils.clickView touch-mode injection, not expressible headless";
}

/* Needs TouchUtils.clickView (see testBeforeDescendantsEnterTouchMode). */
TEST_F(DescendantFocusabilityTest, testAfterDescendantsEnterTouchMode) {
    GTEST_SKIP() << "needs TouchUtils.clickView touch-mode injection, not expressible headless";
}
