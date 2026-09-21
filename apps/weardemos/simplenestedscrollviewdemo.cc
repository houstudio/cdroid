// Port of SupportWearDemos SimpleNestedScrollViewDemo: a NestedScrollView
// inside a BoxInsetLayout, filled with 100 fixed-height TextViews.
#include <core/app.h>
#include <cdroid.h>
#include <core/activityfactory.h>
#include <widget/linearlayout.h>
#include <widget/textview.h>
#include "R.h"

using namespace cdroid;

namespace {
constexpr int ITEM_COUNT = 100;
constexpr int ITEM_HEIGHT_DP = 50;
constexpr int ITEM_TEXT_SIZE = 14;
}

class SimpleNestedScrollViewDemo : public Window {
public:
    SimpleNestedScrollViewDemo() : Window(&App::getInstance(), 0, 0, -1, -1) {
        ViewGroup* root = (ViewGroup*)LayoutInflater::from(getContext())
                ->inflate(weardemos::R::layout::nested_sv_demo, this, false);
        addView(root);

        LinearLayout* linearLayout = (LinearLayout*)root->findViewById(
                weardemos::R::id::linear_layout);
        for (int i = 0; i < ITEM_COUNT; i++) {
            TextView* textView = new TextView(getContext(), nullptr);
            textView->setHeight(ITEM_HEIGHT_DP);
            textView->setTextSize(ITEM_TEXT_SIZE);
            textView->setText("Item " + std::to_string(i));
            linearLayout->addView(textView);
        }
    }
};
REGISTER_ACTIVITY(SimpleNestedScrollViewDemo);
