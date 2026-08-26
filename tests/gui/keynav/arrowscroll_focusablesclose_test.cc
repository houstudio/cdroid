/*********************************************************************************
 * Port of AOSP coretests android.widget.listview.arrowscroll.ListItemFocusablesCloseTest
 * (+ ListItemFocusablesClose companion scenario). Source of truth:
 * frameworks/base/core/tests/coretests/src/android/widget/listview/arrowscroll/ListItemFocusablesCloseTest.java
 * frameworks/base/core/tests/coretests/src/android/widget/listview/ListItemFocusablesClose.java
 *********************************************************************************/
#include <gtest/gtest.h>
#include <cdroid.h>
#include <guienvironment.h>
#include "keysender.h"
#include "listscenario.h"

using namespace cdroid;
using keynav::ListScenario;

class ListItemFocusablesClose : public ListScenario {
public:
    /* Get the child of a list item. */
    View* getChildOfItem(int listIndex, int index) {
        return ((ViewGroup*)getListView()->getChildAt(listIndex))->getChildAt(index);
    }

    void init(Params& params) override {
        params.setItemsFocusable(true)
                .setNumItems(2)
                .setItemScreenSizeFactor(0.55);
    }

    View* createView(int position, ViewGroup* parent, int desiredHeight) override {
        return keynav::ListItemFactory::twoButtonsSeparatedByFiller(
                position, parent->getContext(), desiredHeight);
    }
};

class ListItemFocusablesCloseTest : public testing::Test {
protected:
    ListItemFocusablesClose* mActivity;
    ListView* mListView;
    int mListTop;
    int mListBottom;

    void SetUp() override {
        mActivity = new ListItemFocusablesClose();
        mActivity->launch();
        mListView = mActivity->getListView();
        mListTop = mListView->getListPaddingTop();
        mListBottom = mListView->getHeight() - mListView->getListPaddingBottom();
    }
};

TEST_F(ListItemFocusablesCloseTest, testPreconditions) {
    ASSERT_NE(nullptr, mListView);
    EXPECT_TRUE(mListView->getAdapter()->areAllItemsEnabled());
    EXPECT_TRUE(mListView->getItemsCanFocus());
    EXPECT_EQ(0, mListView->getSelectedItemPosition());
    LinearLayout* first = (LinearLayout*)mListView->getSelectedView();
    pumpUntilIdle();
    EXPECT_EQ(mListView->getListPaddingTop(), first->getTop())
            << "first item should be at top of screen";
    EXPECT_TRUE(first->getChildAt(0)->isFocused())
            << "first button of first list item should have focus";
    EXPECT_LT(first->getHeight(), mListView->getHeight())
            << "item should be shorter than list for this test to make sense";
    EXPECT_EQ(2, mListView->getChildCount()) << "two items should be on screen";
    EXPECT_LT(mActivity->getChildOfItem(1, 0)->getBottom(), mListBottom)
            << "first button of second item should be on screen";
}

TEST_F(ListItemFocusablesCloseTest, testChangeFocusWithinItem) {
    LinearLayout* first = (LinearLayout*)mListView->getSelectedView();
    const int topOfFirstItemBefore = first->getTop();
    keynav::sendKey(KeyEvent::KEYCODE_DPAD_DOWN, 0, mActivity->getWindow());
    EXPECT_TRUE(first->getChildAt(2)->isFocused())
            << "focus should have moved to second button of first item";
    EXPECT_EQ(0, mListView->getSelectedItemPosition())
            << "selection should not have changed";
    EXPECT_EQ(topOfFirstItemBefore, first->getTop())
            << "list item should not have been shifted";

    keynav::sendKey(KeyEvent::KEYCODE_DPAD_UP, 0, mActivity->getWindow());
    EXPECT_TRUE(first->getChildAt(0)->isFocused())
            << "focus should have moved back to first button of first item";
    EXPECT_EQ(topOfFirstItemBefore, first->getTop())
            << "list item should not have been shifted";
}

TEST_F(ListItemFocusablesCloseTest, testMoveDownToButtonInDifferentSelection) {
    LinearLayout* first = (LinearLayout*)mListView->getSelectedView();
    const int topOfFirstItemBefore = first->getTop();
    keynav::sendKey(KeyEvent::KEYCODE_DPAD_DOWN, 0, mActivity->getWindow());
    keynav::sendKey(KeyEvent::KEYCODE_DPAD_DOWN, 0, mActivity->getWindow());

    EXPECT_EQ(1, mListView->getSelectedItemPosition())
            << "selection should have moved to second item";
    LinearLayout* selectedItem = (LinearLayout*)mListView->getSelectedView();
    EXPECT_TRUE(selectedItem->getChildAt(0)->isFocused())
            << "first button of second item should have focus";
    EXPECT_EQ(topOfFirstItemBefore, first->getTop())
            << "list item should not have been shifted";
}

TEST_F(ListItemFocusablesCloseTest, testMoveUpToButtonInDifferentSelection) {
    keynav::sendKey(KeyEvent::KEYCODE_DPAD_DOWN, 0, mActivity->getWindow());
    keynav::sendKey(KeyEvent::KEYCODE_DPAD_DOWN, 0, mActivity->getWindow());
    ASSERT_EQ(1, mListView->getSelectedItemPosition());
    ASSERT_TRUE(mActivity->getChildOfItem(1, 0)->hasFocus());

    keynav::sendKey(KeyEvent::KEYCODE_DPAD_UP, 0, mActivity->getWindow());
    EXPECT_EQ(0, mListView->getSelectedItemPosition())
            << "first list item should have selection";
    EXPECT_TRUE(mActivity->getChildOfItem(0, 2)->hasFocus())
            << "second button of first item should have focus";
}
