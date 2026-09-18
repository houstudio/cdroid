/*********************************************************************************
 * Port of AOSP coretests android.widget.listview.arrowscroll.ListItemFocusablesFarApartTest
 * (+ ListItemFocusablesFarApart companion scenario). Source of truth:
 * frameworks/base/core/tests/coretests/src/android/widget/listview/arrowscroll/ListItemFocusablesFarApartTest.java
 * frameworks/base/core/tests/coretests/src/android/widget/listview/ListItemFocusablesFarApart.java
 *********************************************************************************/
#include <gtest/gtest.h>
#include <cdroid.h>
#include <guienvironment.h>
#include "keysender.h"
#include "listscenario.h"

using namespace cdroid;
using keynav::ListScenario;

/* A list where each item is tall with buttons that are farther apart than the
   screen size. We don't want to jump over content off screen to the next
   button, we need to pan across the intermediate part. */
class ListItemFocusablesFarApart : public ListScenario {
public:
    void init(Params& params) override {
        params.setItemsFocusable(true)
                .setNumItems(2)
                .setItemScreenSizeFactor(2);
    }

    View* createView(int position, ViewGroup* parent, int desiredHeight) override {
        return keynav::ListItemFactory::twoButtonsSeparatedByFiller(
                position, parent->getContext(), desiredHeight);
    }
};

class ListItemFocusablesFarApartTest : public testing::Test {
protected:
    ListItemFocusablesFarApart* mActivity;
    ListView* mListView;
    int mListTop;
    int mListBottom;

    void SetUp() override {
        mActivity = new ListItemFocusablesFarApart();
        mActivity->launch();
        mListView = mActivity->getListView();
        mListTop = mListView->getListPaddingTop();
        mListBottom = mListView->getHeight() - mListView->getListPaddingBottom();
    }

    /* Get the child of a list item. */
    View* getChildOfItem(int listIndex, int index) {
        return ((ViewGroup*)mListView->getChildAt(listIndex))->getChildAt(index);
    }

    int getTopOfChildOfItem(int listIndex, int index) {
        ViewGroup* listItem = (ViewGroup*)mListView->getChildAt(listIndex);
        View* child = listItem->getChildAt(index);
        return child->getTop() + listItem->getTop();
    }

    int getBottomOfChildOfItem(int listIndex, int index) {
        ViewGroup* listItem = (ViewGroup*)mListView->getChildAt(listIndex);
        View* child = listItem->getChildAt(index);
        return child->getBottom() + listItem->getTop();
    }
};

TEST_F(ListItemFocusablesFarApartTest, testPreconditions) {
    ASSERT_NE(nullptr, mListView);
    ASSERT_EQ(1, mListView->getChildCount());
    const int topOfFirstButton = getTopOfChildOfItem(0, 0);
    const int topOfSecondButton = getTopOfChildOfItem(0, 2);
    EXPECT_GT(topOfSecondButton - topOfFirstButton, mListView->getMaxScrollAmount());
}

TEST_F(ListItemFocusablesFarApartTest, testPanWhenNextFocusableTooFarDown) {
    int expectedTop = mListView->getChildAt(0)->getTop();

    Button* topButton = (Button*)getChildOfItem(0, 0);

    int counter = 0;
    while (getTopOfChildOfItem(0, 2) > mListBottom) {
        /* just to make sure we never end up with an infinite loop */
        if (counter > 5) FAIL() << "couldn't reach next button within " << counter << " downs";

        if (getBottomOfChildOfItem(0, 0) < mListTop) {
            EXPECT_FALSE(topButton->isFocused()) << "after " << counter
                    << " downs, top button not visible, should not have focus";
            EXPECT_FALSE(mListView->getChildAt(0)->hasFocus()) << "after " << counter
                    << " downs, neither top button nor bottom button visible, "
                    "nothing within first list item should have focus";
        } else {
            EXPECT_TRUE(topButton->isFocused()) << "after " << counter
                    << " downs, top button still visible, should have focus";
        }

        EXPECT_EQ(expectedTop, mListView->getChildAt(0)->getTop()) << "after "
                << counter << " downs, should have panned by max scroll amount";

        keynav::sendKey(KeyEvent::KEYCODE_DPAD_DOWN, 0, mActivity->getWindow());
        expectedTop -= mListView->getMaxScrollAmount();
        counter++;
    }

    /* at this point, the second button is visible on screen. it should have
       focus */
    EXPECT_TRUE(getChildOfItem(0, 2)->isFocused());
}
