// Port of SupportWearDemos SimpleRecyclerViewDemo: a plain RecyclerView
// (LinearLayoutManager) inside a BoxInsetLayout, 100 fixed-height TextViews.
#include <core/app.h>
#include <cdroid.h>
#include <core/activityfactory.h>
#include <widget/textview.h>
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include "R.h"

using namespace cdroid;

namespace {
constexpr int ITEM_COUNT = 100;
constexpr int ELEMENT_HEIGHT_DP = 50;
constexpr int ELEMENT_TEXT_SIZE = 14;
}

class SimpleRecyclerViewDemo : public Window {
private:
    class ViewHolder : public RecyclerView::ViewHolder {
    public:
        TextView* mView;
        explicit ViewHolder(TextView* itemView) : RecyclerView::ViewHolder(itemView), mView(itemView) {}
    };
    class DemoAdapter : public RecyclerView::Adapter {
    public:
        RecyclerView::ViewHolder* onCreateViewHolder(ViewGroup* parent, int viewType) override {
            TextView* view = new TextView(parent->getContext(), nullptr);
            view->setHeight(ELEMENT_HEIGHT_DP);
            view->setTextSize(ELEMENT_TEXT_SIZE);
            return new ViewHolder(view);
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
    SimpleRecyclerViewDemo() : Window(&App::getInstance(), 0, 0, -1, -1) {
        ViewGroup* root = (ViewGroup*)LayoutInflater::from(getContext())
                ->inflate(weardemos::R::layout::rv_demo, this, false);
        addView(root);

        RecyclerView* rv = (RecyclerView*)root->findViewById(weardemos::R::id::rv_container);
        rv->setLayoutManager(new LinearLayoutManager(getContext()));
        rv->setAdapter(new DemoAdapter());
    }
};
REGISTER_ACTIVITY(SimpleRecyclerViewDemo);
