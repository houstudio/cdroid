/*********************************************************************************
 * Port of AOSP coretests android.widget.focus.GoneParentFocusedChildTest, with
 * the GoneParentFocusedChild companion activity rebuilt programmatically in a
 * private Window. Source of truth:
 * frameworks/base/core/tests/coretests/src/android/widget/focus/GoneParentFocusedChild{,Test}.java
 *********************************************************************************/
#include <gtest/gtest.h>
#include <cdroid.h>
#include <guienvironment.h>
#include "keysender.h"

using namespace cdroid;

/* When a parent is GONE, key events shouldn't go to its children, even if
   they have focus (part of investigation into issue 945150). */
class GoneParentFocusedChildTest : public testing::Test {
protected:
    Window* mActivity;
    LinearLayout* mLayout;
    LinearLayout* mGoneGroup;
    Button* mButton;

    /* GoneParentFocusedChild.onKeyUp sets mUnhandledKeyEvent; the CDSIDE
       equivalent signal is the UP leg falling through the whole window tree
       (Window::dispatchKeyEvent returns false = nobody consumed it, i.e. it
       reached the window/Activity callback). */
    bool mUnhandledKeyEvent;

    void SetUp() override {
        mUnhandledKeyEvent = false;
        App& app = App::getInstance();

        mActivity = new Window(&app, 0, 0, -1, -1);

        mLayout = new LinearLayout(&app);
        mLayout->setOrientation(LinearLayout::HORIZONTAL);
        mLayout->setLayoutParams(new ViewGroup::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT,
                ViewGroup::LayoutParams::MATCH_PARENT));

        /* NOTE: like the AOSP activity, mGoneGroup is never attached to the
           window tree — the focused child sits on a detached branch. */
        mGoneGroup = new LinearLayout(&app);
        mGoneGroup->setOrientation(LinearLayout::HORIZONTAL);
        mGoneGroup->setLayoutParams(new ViewGroup::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT,
                ViewGroup::LayoutParams::MATCH_PARENT));

        mButton = new Button(&app);
        mButton->setLayoutParams(new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::WRAP_CONTENT,
                ViewGroup::LayoutParams::WRAP_CONTENT));

        mGoneGroup->addView(mButton);

        mActivity->addView(mLayout);
        const int w = mActivity->getWidth() > 0 ? mActivity->getWidth() : 1080;
        const int h = mActivity->getHeight() > 0 ? mActivity->getHeight() : 1920;
        mLayout->measure(MeasureSpec::makeMeasureSpec(w, MeasureSpec::EXACTLY),
                         MeasureSpec::makeMeasureSpec(h, MeasureSpec::EXACTLY));
        mLayout->layout(0, 0, w, h);
        pumpUntilIdle();

        mGoneGroup->setVisibility(View::GONE);
        mButton->requestFocus();
    }
};

TEST_F(GoneParentFocusedChildTest, testPreconditinos) {
    ASSERT_NE(nullptr, mLayout);
    ASSERT_NE(nullptr, mGoneGroup);
    ASSERT_NE(nullptr, mButton);
    ASSERT_TRUE(mButton->hasFocus());
    ASSERT_EQ((int)View::GONE, (int)mGoneGroup->getVisibility());
    ASSERT_FALSE(mUnhandledKeyEvent);
}

TEST_F(GoneParentFocusedChildTest, testKeyEventGoesToActivity) {
    /* sendKeys(KEYCODE_J): the key must bypass the focused child on the GONE
       branch and reach the window (activity) callback. */
    const nsecs_t downTime = SystemClock::uptimeMillis();
    const bool handledDown = keynav::sendKeyDown(KeyEvent::KEYCODE_J, downTime, 0, mActivity);
    const bool handledUp = keynav::sendKeyUp(KeyEvent::KEYCODE_J, downTime, 0, mActivity);
    pumpUntilIdle();
    mUnhandledKeyEvent = !(handledDown || handledUp);
    EXPECT_TRUE(mUnhandledKeyEvent);
}
