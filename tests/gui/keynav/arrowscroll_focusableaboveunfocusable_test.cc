/*********************************************************************************
 * Port of AOSP coretests android.widget.listview.arrowscroll.ListItemFocusableAboveUnfocusableTest
 * (+ ListItemFocusableAboveUnfocusable companion scenario). Source of truth:
 * frameworks/base/core/tests/coretests/src/android/widget/listview/arrowscroll/ListItemFocusableAboveUnfocusableTest.java
 * frameworks/base/core/tests/coretests/src/android/widget/listview/ListItemFocusableAboveUnfocusable.java
 *********************************************************************************/
#include <gtest/gtest.h>
#include <cdroid.h>
#include <guienvironment.h>
#include "keysender.h"
#include "listscenario.h"

using namespace cdroid;
using keynav::ListScenario;

class ListItemFocusableAboveUnfocusable : public ListScenario {
public:
    void init(Params& params) override {
        params.setNumItems(2)
                .setItemsFocusable(true)
                .setItemScreenSizeFactor(0.2)
                .setMustFillScreen(false);
    }

    View* createView(int position, ViewGroup* parent, int desiredHeight) override {
        if (position == 0) {
            return keynav::ListItemFactory::button(
                    position, parent->getContext(), getValueAtPosition(position), desiredHeight);
        }
        return ListScenario::createView(position, parent, desiredHeight);
    }
};

class ListItemFocusableAboveUnfocusableTest : public testing::Test {
protected:
    ListItemFocusableAboveUnfocusable* mActivity;
    ListView* mListView;

    void SetUp() override {
        mActivity = new ListItemFocusableAboveUnfocusable();
        mActivity->launch();
        mListView = mActivity->getListView();
    }
};

TEST_F(ListItemFocusableAboveUnfocusableTest, testPreconditions) {
    EXPECT_EQ(0, mListView->getSelectedItemPosition()) << "selected position";
    EXPECT_TRUE(mListView->getChildAt(0)->isFocused());
    EXPECT_FALSE(mListView->getChildAt(1)->isFocusable());
}

TEST_F(ListItemFocusableAboveUnfocusableTest, testMovingToUnFocusableTakesFocusAway) {
    keynav::sendKey(KeyEvent::KEYCODE_DPAD_DOWN, 0, mActivity->getWindow());

    EXPECT_FALSE(mListView->getChildAt(0)->isFocused())
            << "focused item should have lost focus";
    EXPECT_EQ(1, mListView->getSelectedItemPosition()) << "selected position";
}
