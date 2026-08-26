/*********************************************************************************
 * Port of AOSP coretests android.widget.focus.FocusAfterRemovalTest, with the
 * FocusAfterRemoval companion activity (focus_after_removal.xml 2x2 button
 * grid; each button flips a visibility flag on click) rebuilt programmatically
 * in a private Window. Source of truth:
 * frameworks/base/core/tests/coretests/src/android/widget/focus/FocusAfterRemoval{,Test}.java
 * res/layout/focus_after_removal.xml
 *********************************************************************************/
#include <gtest/gtest.h>
#include <cdroid.h>
#include <guienvironment.h>
#include "keysender.h"

using namespace cdroid;

/* Exercises cases where elements of the UI are removed (and focus should go
   somewhere). */
class FocusAfterRemovalTest : public testing::Test {
protected:
    Window* mActivity;

    LinearLayout* mLeftLayout;
    Button* mTopLeftButton;
    Button* mBottomLeftButton;
    Button* mTopRightButton;
    Button* mBottomRightButton;

    void SetUp() override {
        App& app = App::getInstance();
        mActivity = new Window(&app, 0, 0, -1, -1);

        LinearLayout* root = new LinearLayout(&app);
        root->setOrientation(LinearLayout::HORIZONTAL);
        root->setLayoutParams(new ViewGroup::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT,
                ViewGroup::LayoutParams::WRAP_CONTENT));

        mLeftLayout = new LinearLayout(&app);
        mLeftLayout->setOrientation(LinearLayout::VERTICAL);
        mLeftLayout->setLayoutParams(new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::WRAP_CONTENT,
                ViewGroup::LayoutParams::WRAP_CONTENT));
        mTopLeftButton = addButton(mLeftLayout, "left_top");
        mBottomLeftButton = addButton(mLeftLayout, "left_bottom");
        root->addView(mLeftLayout);

        LinearLayout* rightLayout = new LinearLayout(&app);
        rightLayout->setOrientation(LinearLayout::VERTICAL);
        rightLayout->setLayoutParams(new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::WRAP_CONTENT,
                ViewGroup::LayoutParams::WRAP_CONTENT));
        mTopRightButton = addButton(rightLayout, "right_top");
        mBottomRightButton = addButton(rightLayout, "right_bottom");
        root->addView(rightLayout);

        mActivity->addView(root);
        const int w = mActivity->getWidth() > 0 ? mActivity->getWidth() : 1080;
        const int h = mActivity->getHeight() > 0 ? mActivity->getHeight() : 1920;
        root->measure(MeasureSpec::makeMeasureSpec(w, MeasureSpec::EXACTLY),
                      MeasureSpec::makeMeasureSpec(h, MeasureSpec::AT_MOST));
        root->layout(0, 0, w, root->getMeasuredHeight());
        pumpUntilIdle();

        /* Click listeners (FocusAfterRemoval.onCreate):
           top left -> parent layout GONE; bottom left -> parent INVISIBLE;
           top right -> itself GONE; bottom right -> itself INVISIBLE. */
        LinearLayout* left = mLeftLayout;
        mTopLeftButton->setOnClickListener([left](View&) { left->setVisibility(View::GONE); });
        mBottomLeftButton->setOnClickListener([left](View&) { left->setVisibility(View::INVISIBLE); });
        Button* topRight = mTopRightButton;
        mTopRightButton->setOnClickListener([topRight](View&) { topRight->setVisibility(View::GONE); });
        Button* bottomRight = mBottomRightButton;
        mBottomRightButton->setOnClickListener([bottomRight](View&) { bottomRight->setVisibility(View::INVISIBLE); });

        /* focus_after_removal.xml has <requestFocus/>; the observable initial
           state is topLeft holding focus (first focusable in the fresh
           activity window). */
        mTopLeftButton->requestFocus();
        pumpUntilIdle();
    }

    Button* addButton(ViewGroup* parent, const std::string& text) {
        App& app = App::getInstance();
        Button* b = new Button(&app);
        b->setText(text);
        LinearLayout::LayoutParams* lp = new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::WRAP_CONTENT,
                ViewGroup::LayoutParams::WRAP_CONTENT);
        lp->rightMargin = 3;
        b->setLayoutParams(lp);
        parent->addView(b);
        return b;
    }
};

TEST_F(FocusAfterRemovalTest, testSetUpConditions) {
    ASSERT_NE(nullptr, mLeftLayout);
    ASSERT_NE(nullptr, mTopLeftButton);
    ASSERT_NE(nullptr, mTopRightButton);
    ASSERT_NE(nullptr, mBottomLeftButton);
    ASSERT_NE(nullptr, mBottomRightButton);

    ASSERT_TRUE(mTopLeftButton->hasFocus());
}

/* if a parent layout becomes GONE when one of its children has focus, make
   sure the focus moves to something visible (bug 827087) */
TEST_F(FocusAfterRemovalTest, testFocusLeavesWhenParentLayoutIsGone) {
    /* clicking on this button makes its parent linear layout GONE */
    keynav::sendKey(KeyEvent::KEYCODE_DPAD_CENTER, 0, mActivity);
    EXPECT_EQ((int)View::GONE, (int)mLeftLayout->getVisibility());

    EXPECT_TRUE(mTopRightButton->hasFocus());
}

TEST_F(FocusAfterRemovalTest, testFocusLeavesWhenParentLayoutInvisible) {
    /* move down to bottom left button */
    keynav::sendKey(KeyEvent::KEYCODE_DPAD_DOWN, 0, mActivity);
    ASSERT_TRUE(mBottomLeftButton->hasFocus());

    /* clicking on this button makes its parent linear layout INVISIBLE */
    keynav::sendKey(KeyEvent::KEYCODE_DPAD_CENTER, 0, mActivity);
    EXPECT_EQ((int)View::INVISIBLE, (int)mLeftLayout->getVisibility());

    EXPECT_TRUE(mTopRightButton->hasFocus());
}

TEST_F(FocusAfterRemovalTest, testFocusLeavesWhenFocusedViewBecomesGone) {
    /* move to top right */
    keynav::sendKey(KeyEvent::KEYCODE_DPAD_RIGHT, 0, mActivity);
    ASSERT_TRUE(mTopRightButton->hasFocus());

    /* click making it GONE */
    keynav::sendKey(KeyEvent::KEYCODE_DPAD_CENTER, 0, mActivity);
    EXPECT_EQ((int)View::GONE, (int)mTopRightButton->getVisibility());

    EXPECT_TRUE(mTopLeftButton->hasFocus());
}

TEST_F(FocusAfterRemovalTest, testFocusLeavesWhenFocusedViewBecomesInvisible) {
    /* move to bottom right */
    keynav::sendKey(KeyEvent::KEYCODE_DPAD_RIGHT, 0, mActivity);
    keynav::sendKey(KeyEvent::KEYCODE_DPAD_DOWN, 0, mActivity);
    ASSERT_TRUE(mBottomRightButton->hasFocus());

    /* click making it INVISIBLE */
    keynav::sendKey(KeyEvent::KEYCODE_DPAD_CENTER, 0, mActivity);
    EXPECT_EQ((int)View::INVISIBLE, (int)mBottomRightButton->getVisibility());

    EXPECT_TRUE(mTopLeftButton->hasFocus());
}
