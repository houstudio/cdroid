/*********************************************************************************
 * Port of AOSP coretests android.widget.listview.arrowscroll.ListInterleaveFocusablesTest
 * (+ ListInterleaveFocusables companion scenario). Source of truth:
 * frameworks/base/core/tests/coretests/src/android/widget/listview/arrowscroll/ListInterleaveFocusablesTest.java
 * frameworks/base/core/tests/coretests/src/android/widget/listview/ListInterleaveFocusables.java
 *********************************************************************************/
#include <gtest/gtest.h>
#include <cdroid.h>
#include <guienvironment.h>
#include "keysender.h"
#include "listscenario.h"
#include "listutil.h"

using namespace cdroid;
using keynav::ListScenario;

class ListInterleaveFocusables : public ListScenario {
public:
    const std::set<int> mFocusablePositions = {1, 3, 6};

    void init(Params& params) override {
        params.setNumItems(7)
                .setItemScreenSizeFactor(1.0 / 8)
                .setItemsFocusable(true)
                .setMustFillScreen(false);
    }

    View* createView(int position, ViewGroup* parent, int desiredHeight) override {
        if (mFocusablePositions.count(position)) {
            return keynav::ListItemFactory::button(
                    position, parent->getContext(), getValueAtPosition(position), desiredHeight);
        }
        return ListScenario::createView(position, parent, desiredHeight);
    }

    int getItemViewType(int position) override {
        return mFocusablePositions.count(position) ? 0 : 1;
    }

    int getViewTypeCount() override {
        return 2;
    }
};

class ListInterleaveFocusablesTest : public testing::Test {
protected:
    ListInterleaveFocusables* mActivity;
    ListView* mListView;
    keynav::ListUtil* mListUtil;

    void SetUp() override {
        mActivity = new ListInterleaveFocusables();
        mActivity->launch();
        mListView = mActivity->getListView();
        mListUtil = new keynav::ListUtil(mListView, mActivity->getWindow());
    }

    void assertSelectedViewFocus(bool isFocused) {
        View* view = mListView->getSelectedView();
        EXPECT_EQ(isFocused, view->isFocused()) << "selected view focused";
        EXPECT_EQ(!isFocused, view->isSelected()) << "selected position's isSelected "
                "should be the inverse of it having focus";
    }
};

TEST_F(ListInterleaveFocusablesTest, testPreconditions) {
    ASSERT_EQ(7, mListView->getChildCount());
    EXPECT_TRUE(mListView->getChildAt(1)->isFocusable());
    EXPECT_TRUE(mListView->getChildAt(3)->isFocusable());
    EXPECT_TRUE(mListView->getChildAt(6)->isFocusable());
}

TEST_F(ListInterleaveFocusablesTest, testGoingFromUnFocusableSelectedToFocusable) {
    keynav::sendKey(KeyEvent::KEYCODE_DPAD_DOWN, 0, mActivity->getWindow());

    EXPECT_EQ(1, mListView->getSelectedItemPosition()) << "selected item position";
    assertSelectedViewFocus(true);
}

/* go down from an item that isn't focusable, make sure it finds the focusable
   below (instead of above). this exposes a (now fixed) bug where the focus
   search was not starting from the right spot */
TEST_F(ListInterleaveFocusablesTest, testGoingDownFromUnFocusableSelectedToFocusableWithOtherFocusableAbove) {
    mListUtil->setSelectedPosition(2);
    keynav::sendKey(KeyEvent::KEYCODE_DPAD_DOWN, 0, mActivity->getWindow());
    EXPECT_EQ(3, mListView->getSelectedItemPosition()) << "selected item position";
    assertSelectedViewFocus(true);
}

/* same, but going up */
TEST_F(ListInterleaveFocusablesTest, testGoingUpFromUnFocusableSelectedToFocusableWithOtherFocusableAbove) {
    mListUtil->setSelectedPosition(2);
    keynav::sendKey(KeyEvent::KEYCODE_DPAD_UP, 0, mActivity->getWindow());
    EXPECT_EQ(1, mListView->getSelectedItemPosition()) << "selected item position";
    assertSelectedViewFocus(true);
}

/* Go down from a focusable when there is a focusable below, but it is more
   than one item away; make sure it won't give that item focus because it is
   too far away. */
TEST_F(ListInterleaveFocusablesTest, testGoingDownFromFocusableToUnfocusableWhenFocusableIsBelow) {
    mListUtil->setSelectedPosition(3);
    keynav::sendKey(KeyEvent::KEYCODE_DPAD_DOWN, 0, mActivity->getWindow());
    EXPECT_EQ(4, mListView->getSelectedItemPosition()) << "selected item position";
    assertSelectedViewFocus(false);
}

/* same but going up */
TEST_F(ListInterleaveFocusablesTest, testGoingUpFromFocusableToUnfocusableWhenFocusableIsBelow) {
    mListUtil->setSelectedPosition(6);
    keynav::sendKey(KeyEvent::KEYCODE_DPAD_UP, 0, mActivity->getWindow());
    EXPECT_EQ(5, mListView->getSelectedItemPosition()) << "selected item position";
    assertSelectedViewFocus(false);
}
