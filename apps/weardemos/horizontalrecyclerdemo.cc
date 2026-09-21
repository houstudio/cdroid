// Horizontal-scroll test page (CDROID-added, not from SupportWearDemos): a
// horizontally scrolling RecyclerView. Its purpose is the swipe-to-dismiss
// interplay — WearGestureInterceptionDetector.canScroll must suppress the
// dismiss gesture over the list (Detector.java:100-114), so a right-drag
// scrolls, and only a drag at the left edge (cannot scroll further left)
// dismisses. Colored items make horizontal motion obvious in screenshots.
#include <core/app.h>
#include <cdroid.h>
#include <core/activityfactory.h>
#include <widget/textview.h>
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include "R.h"

using namespace cdroid;

namespace {
constexpr int ITEM_COUNT = 12;
const unsigned int ITEM_COLORS[] = {
    0xFFF44336, 0xFFE91E63, 0xFF9C27B0, 0xFF673AB7,
    0xFF3F51B5, 0xFF2196F3, 0xFF009688, 0xFF4CAF50,
    0xFFFFC107, 0xFFFF9800, 0xFFFF5722, 0xFF795548,
};
}

class HorizontalRecyclerViewDemo : public Window {
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
            view->setLayoutParams(new RecyclerView::LayoutParams(240, 240));
            view->setGravity(Gravity::CENTER);
            view->setTextColor((int)0xFFFFFFFF);
            view->setTextSize(18);
            return new ViewHolder(view);
        }
        void onBindViewHolder(RecyclerView::ViewHolder& holder, int position) override {
            ViewHolder& vh = static_cast<ViewHolder&>(holder);
            vh.mView->setText("Item " + std::to_string(position));
            vh.mView->setBackgroundColor((int)ITEM_COLORS[position % (sizeof(ITEM_COLORS) / sizeof(ITEM_COLORS[0]))]);
        }
        int getItemCount() override { return ITEM_COUNT; }
    };
public:
    HorizontalRecyclerViewDemo() : Window(&App::getInstance(), 0, 0, -1, -1) {
        RecyclerView* rv = new RecyclerView(getContext(), nullptr);
        rv->setLayoutManager(new LinearLayoutManager(getContext(),
                LinearLayoutManager::HORIZONTAL, false));
        rv->setAdapter(new DemoAdapter());
        addView(rv, LayoutParams::MATCH_PARENT, LayoutParams::MATCH_PARENT);
    }
};
REGISTER_ACTIVITY(HorizontalRecyclerViewDemo);
