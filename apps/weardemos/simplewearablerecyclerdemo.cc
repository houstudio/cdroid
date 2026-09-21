// Port of SupportWearDemos SimpleWearableRecyclerViewDemo: a
// WearableRecyclerView driven by WearableLinearLayoutManager (the curved list
// + circular bezel scrolling), 100 plain TextView holders.
#include <core/app.h>
#include <cdroid.h>
#include <core/activityfactory.h>
#include <widget/textview.h>
#include <widgetEx/wear/wearablerecyclerview.h>
#include <widgetEx/wear/wearablelinearlayoutmanager.h>
#include <widgetEx/recyclerview/recyclerview.h>
#include "R.h"

using namespace cdroid;

namespace {
constexpr int ITEM_COUNT = 100;
}

class SimpleWearableRecyclerViewDemo : public Window {
private:
    class ViewHolder : public RecyclerView::ViewHolder {
    public:
        TextView* mView;
        explicit ViewHolder(TextView* itemView) : RecyclerView::ViewHolder(itemView), mView(itemView) {}
    };
    class DemoAdapter : public RecyclerView::Adapter {
    public:
        RecyclerView::ViewHolder* onCreateViewHolder(ViewGroup* parent, int viewType) override {
            return new ViewHolder(new TextView(parent->getContext(), nullptr));
        }
        void onBindViewHolder(RecyclerView::ViewHolder& holder, int position) override {
            ViewHolder& vh = static_cast<ViewHolder&>(holder);
            vh.mView->setText("Holder at position " + std::to_string(position));
            // Java setTag(Object) boxed the position; CDROID tags are void*.
            vh.mView->setTag((void*)(intptr_t)position);
        }
        int getItemCount() override { return ITEM_COUNT; }
    };
public:
    SimpleWearableRecyclerViewDemo() : Window(&App::getInstance(), 0, 0, -1, -1) {
        ViewGroup* root = (ViewGroup*)LayoutInflater::from(getContext())
                ->inflate(weardemos::R::layout::wrv_demo, this, false);
        addView(root);

        WearableRecyclerView* wrv = (WearableRecyclerView*)root->findViewById(
                weardemos::R::id::wrv_container);
        wrv->setLayoutManager(new WearableLinearLayoutManager(getContext()));
        wrv->setAdapter(new DemoAdapter());
        wrv->setCircularScrollingGestureEnabled(true);
        wrv->setEdgeItemsCenteringEnabled(true);
    }
};
REGISTER_ACTIVITY(SimpleWearableRecyclerViewDemo);
