/*********************************************************************************
 * Port of AOSP coretests android.widget.focus.RequestFocusTest, with the
 * RequestFocus companion activity (and its focus_after_removal.xml: a 2x2
 * button grid) rebuilt programmatically in a private Window. Mockito's
 * InOrder-verified CombinedListeners becomes a recording listener whose
 * invocation log is asserted in order. Source of truth:
 * frameworks/base/core/tests/coretests/src/android/widget/focus/RequestFocus{,Test}.java
 * res/layout/focus_after_removal.xml
 *********************************************************************************/
#include <gtest/gtest.h>
#include <cdroid.h>
#include <guienvironment.h>
#include "keysender.h"

using namespace cdroid;

/* RequestFocusTest is set up to exercise cases where the views that have focus
   become invisible or GONE. */
class RequestFocusTest : public testing::Test {
protected:
    Window* mActivity;

    LinearLayout* mRoot;
    LinearLayout* mLeftLayout;
    Button* mTopLeftButton;
    Button* mBottomLeftButton;
    Button* mTopRightButton;
    Button* mBottomRightButton;

    /* CombinedListeners mock: records onFocusChange / onGlobalFocusChanged
       invocations in order. */
    struct Recorder {
        std::vector<std::string> calls;
        View::OnFocusChangeListener onFocusChange =
                View::OnFocusChangeListener([this](View& v, bool hasFocus) {
                    calls.push_back(std::string("onFocusChange ") + tag(v) +
                                    (hasFocus ? " true" : " false"));
                });
        ViewTreeObserver::OnGlobalFocusChangeListener onGlobalFocusChange =
                ViewTreeObserver::OnGlobalFocusChangeListener([this](View* oldFocus, View* newFocus) {
                    calls.push_back(std::string("onGlobalFocusChanged ") + tag(*oldFocus) +
                                    " " + tag(*newFocus));
                });
        static std::string tag(View& v) {
            Button* b = dynamic_cast<Button*>(&v);
            return b ? (std::string)b->getText() : std::string("view");
        }
    };

    void SetUp() override {
        App& app = App::getInstance();
        mActivity = new Window(&app, 0, 0, -1, -1);

        mRoot = new LinearLayout(&app);
        mRoot->setOrientation(LinearLayout::HORIZONTAL);
        mRoot->setLayoutParams(new ViewGroup::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT,
                ViewGroup::LayoutParams::WRAP_CONTENT));

        /* focus_after_removal.xml: left column {topLeft, bottomLeft}, right
           column {topRight, bottomRight}; 3dip right margins. */
        mLeftLayout = new LinearLayout(&app);
        mLeftLayout->setOrientation(LinearLayout::VERTICAL);
        mLeftLayout->setLayoutParams(new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::WRAP_CONTENT,
                ViewGroup::LayoutParams::WRAP_CONTENT));
        mTopLeftButton = addButton(mLeftLayout, "left_top");
        mBottomLeftButton = addButton(mLeftLayout, "left_bottom");
        mRoot->addView(mLeftLayout);

        LinearLayout* rightLayout = new LinearLayout(&app);
        rightLayout->setOrientation(LinearLayout::VERTICAL);
        rightLayout->setLayoutParams(new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::WRAP_CONTENT,
                ViewGroup::LayoutParams::WRAP_CONTENT));
        mTopRightButton = addButton(rightLayout, "right_top");
        mBottomRightButton = addButton(rightLayout, "right_bottom");
        mRoot->addView(rightLayout);

        mActivity->addView(mRoot);
        const int w = mActivity->getWidth() > 0 ? mActivity->getWidth() : 1080;
        const int h = mActivity->getHeight() > 0 ? mActivity->getHeight() : 1920;
        mRoot->measure(MeasureSpec::makeMeasureSpec(w, MeasureSpec::EXACTLY),
                       MeasureSpec::makeMeasureSpec(h, MeasureSpec::AT_MOST));
        mRoot->layout(0, 0, w, mRoot->getMeasuredHeight());
        pumpUntilIdle();

        /* RequestFocus.onCreate: bottomRightButton.requestFocus(). */
        mBottomRightButton->requestFocus();
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

TEST_F(RequestFocusTest, testSetUpConditions) {
    ASSERT_NE(nullptr, mActivity);
    ASSERT_NE(nullptr, mTopLeftButton);
    ASSERT_NE(nullptr, mTopRightButton);
    ASSERT_NE(nullptr, mBottomLeftButton);
    ASSERT_NE(nullptr, mBottomRightButton);
    ASSERT_TRUE(mBottomRightButton->hasFocus());
}

TEST_F(RequestFocusTest, testPostedRequestFocus) {
    /* mHandler.post { mBottomLeftButton.requestFocus() }: post onto the main
       looper and pump until it runs (AOSP waits 500ms). */
    mBottomLeftButton->post([this]() { mBottomLeftButton->requestFocus(); });
    pumpFor(500);
    ASSERT_TRUE(mBottomLeftButton->hasFocus());
}

/* AOSP expects ViewRootImpl$CalledFromWrongThreadException from the
   instrumentation thread. The gtest body IS the CDROID UI thread, so a wrong
   thread cannot be expressed here. */
TEST_F(RequestFocusTest, testWrongThreadRequestFocusFails) {
    GTEST_SKIP() << "gtest runs on the UI thread; wrong-thread requestFocus is not expressible";
}

TEST_F(RequestFocusTest, testOnFocusChangeCallbackOrderWhenClearingFocusOfFirstFocusable) {
    /* Get the first focusable: getRootView().getParent() is the ViewRoot —
       here, the window. */
    Button* clearingFocusButton = mTopLeftButton;
    Button* gainingFocusButton = mTopLeftButton;

    View* focusCandidate = mActivity->focusSearch(nullptr, View::FOCUS_FORWARD);
    ASSERT_EQ(clearingFocusButton, focusCandidate);
    ASSERT_EQ(gainingFocusButton, focusCandidate);

    clearingFocusButton->requestFocus();
    ASSERT_TRUE(clearingFocusButton->hasFocus());

    Recorder recorder;
    clearingFocusButton->setOnFocusChangeListener(recorder.onFocusChange);
    gainingFocusButton->setOnFocusChangeListener(recorder.onFocusChange);
    clearingFocusButton->getViewTreeObserver()->addOnGlobalFocusChangeListener(
            recorder.onGlobalFocusChange);

    clearingFocusButton->clearFocus();
    pumpUntilIdle();

    ASSERT_EQ((size_t)3, recorder.calls.size());
    EXPECT_EQ("onFocusChange left_top false", recorder.calls[0]);
    EXPECT_EQ("onGlobalFocusChanged left_top left_top", recorder.calls[1]);
    EXPECT_EQ("onFocusChange left_top true", recorder.calls[2]);
}

TEST_F(RequestFocusTest, testOnFocusChangeCallbackOrderWhenClearingFocusOfNotFirstFocusable) {
    Button* clearingFocusButton = mTopRightButton;
    Button* gainingFocusButton = mTopLeftButton;

    View* focusCandidate = mActivity->focusSearch(nullptr, View::FOCUS_FORWARD);
    ASSERT_NE(clearingFocusButton, focusCandidate);
    ASSERT_EQ(gainingFocusButton, focusCandidate);

    clearingFocusButton->requestFocus();
    ASSERT_TRUE(clearingFocusButton->hasFocus());

    Recorder recorder;
    clearingFocusButton->setOnFocusChangeListener(recorder.onFocusChange);
    gainingFocusButton->setOnFocusChangeListener(recorder.onFocusChange);
    clearingFocusButton->getViewTreeObserver()->addOnGlobalFocusChangeListener(
            recorder.onGlobalFocusChange);

    clearingFocusButton->clearFocus();
    pumpUntilIdle();

    ASSERT_EQ((size_t)3, recorder.calls.size());
    EXPECT_EQ("onFocusChange right_top false", recorder.calls[0]);
    EXPECT_EQ("onGlobalFocusChanged right_top left_top", recorder.calls[1]);
    EXPECT_EQ("onFocusChange left_top true", recorder.calls[2]);
}
