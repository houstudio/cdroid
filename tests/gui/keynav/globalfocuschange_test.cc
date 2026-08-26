/*********************************************************************************
 * Port of AOSP coretests android.view.GlobalFocusChangeTest, with the
 * GlobalFocusChange companion activity (and its focus_listener.xml, a plain
 * two-button row) rebuilt programmatically in a private Window (one AOSP
 * activity == one window). Source of truth:
 * frameworks/base/core/tests/coretests/src/android/view/GlobalFocusChange{,Test}.java
 * res/layout/focus_listener.xml
 *********************************************************************************/
#include <gtest/gtest.h>
#include <cdroid.h>
#include <guienvironment.h>
#include "keysender.h"

using namespace cdroid;

/* GlobalFocusChange: records ViewTreeObserver.OnGlobalFocusChangeListener
   callbacks. */
class GlobalFocusChangeTest : public testing::Test {
protected:
    Window* mActivity;
    LinearLayout* mLayout;
    Button* mLeft;
    Button* mRight;

    View* mOldFocus;
    View* mNewFocus;

    void SetUp() override {
        mOldFocus = mNewFocus = nullptr;
        App& app = App::getInstance();

        /* The activity: a private full-screen window. The harness listener
           removes stray windows after each case. */
        mActivity = new Window(&app, 0, 0, -1, -1);

        /* focus_listener.xml: horizontal row, "left"/"right" buttons. */
        mLayout = new LinearLayout(&app);
        mLayout->setOrientation(LinearLayout::HORIZONTAL);
        mLayout->setLayoutParams(new ViewGroup::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT,
                ViewGroup::LayoutParams::WRAP_CONTENT));

        mLeft = new Button(&app);
        mLeft->setText("left");
        LinearLayout::LayoutParams* leftLp = new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::WRAP_CONTENT,
                ViewGroup::LayoutParams::WRAP_CONTENT);
        leftLp->rightMargin = 3;
        mLeft->setLayoutParams(leftLp);

        mRight = new Button(&app);
        mRight->setText("right");
        mRight->setLayoutParams(new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::WRAP_CONTENT,
                ViewGroup::LayoutParams::WRAP_CONTENT));

        mLayout->addView(mLeft);
        mLayout->addView(mRight);

        /* setContentView + first traversal; View::layout takes (l, t, w, h). */
        mActivity->addView(mLayout);
        const int w = mActivity->getWidth() > 0 ? mActivity->getWidth() : 1080;
        const int h = mActivity->getHeight() > 0 ? mActivity->getHeight() : 1920;
        mLayout->measure(MeasureSpec::makeMeasureSpec(w, MeasureSpec::EXACTLY),
                         MeasureSpec::makeMeasureSpec(h, MeasureSpec::AT_MOST));
        mLayout->layout(0, 0, w, mLayout->getMeasuredHeight());
        pumpUntilIdle();

        /* onCreate: register the global focus listener on left's VTO. */
        mLeft->getViewTreeObserver()->addOnGlobalFocusChangeListener(
                ViewTreeObserver::OnGlobalFocusChangeListener(
                        [this](View* oldFocus, View* newFocus) {
                            mOldFocus = oldFocus;
                            mNewFocus = newFocus;
                        }));

        /* AOSP: the freshly launched activity window grants initial focus to
           the first focusable (left). */
        mLeft->requestFocus();
        pumpUntilIdle();
    }

    void TearDown() override {
        /* mActivity.reset() */
        mOldFocus = mNewFocus = nullptr;
    }
};

TEST_F(GlobalFocusChangeTest, testFocusChange) {
    keynav::sendKey(KeyEvent::KEYCODE_DPAD_RIGHT, 0, mActivity);

    EXPECT_FALSE(mLeft->isFocused());
    EXPECT_TRUE(mRight->isFocused());

    EXPECT_EQ(mLeft, mOldFocus);
    EXPECT_EQ(mRight, mNewFocus);
}

/* Needs TouchUtils.tapView (real tap injection to enter touch mode) — not
   available in this headless harness. AOSP itself marks the class @Suppress
   (flaky). */
TEST_F(GlobalFocusChangeTest, testEnterTouchMode) {
    GTEST_SKIP() << "needs TouchUtils.tapView touch-mode injection, not expressible headless";
}

/* Needs TouchUtils.tapView (see testEnterTouchMode). */
TEST_F(GlobalFocusChangeTest, testLeaveTouchMode) {
    GTEST_SKIP() << "needs TouchUtils.tapView touch-mode injection, not expressible headless";
}
