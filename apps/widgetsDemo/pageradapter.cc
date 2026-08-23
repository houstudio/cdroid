#include "pageradapter.h"
#include "pages.h"
#include <view/layoutinflater.h>
#include <R.h>

using namespace cdroid;

namespace {
constexpr int PAGE_COUNT = 10;
const int PAGES[PAGE_COUNT] = {
    widgetsDemo::R::layout::page_buttons,
    widgetsDemo::R::layout::page_progress,
    widgetsDemo::R::layout::page_text,
    widgetsDemo::R::layout::page_images,
    widgetsDemo::R::layout::page_animation,
    widgetsDemo::R::layout::page_lists,
    widgetsDemo::R::layout::page_misc,
    widgetsDemo::R::layout::page_datetime,
    widgetsDemo::R::layout::page_constraint,
    widgetsDemo::R::layout::page_motion,
};
} // namespace

RecyclerView::ViewHolder* DemoPagerAdapter::onCreateViewHolder(ViewGroup* parent, int viewType) {
    View* v = LayoutInflater::from(parent->getContext())->inflate(PAGES[viewType], parent, false);
    return new RecyclerView::ViewHolder(v);
}

void DemoPagerAdapter::onBindViewHolder(RecyclerView::ViewHolder& holder, int position) {
    View* page = holder.itemView;
    switch (position) {
        case 0: setupButtons(page);   break;
        case 1: setupProgress(page);  break;
        case 2: setupText(page);      break;
        case 3: setupImages(page);    break;
        case 4: setupAnimation(page); break;
        case 5: setupLists(page);     break;
        case 6: setupMisc(page);      break;
        case 7: setupDateTime(page);  break;
        case 8: setupConstraint(page); break;
        case 9: setupMotion(page);     break;
    }
}

int DemoPagerAdapter::getItemCount() { return PAGE_COUNT; }

int DemoPagerAdapter::getItemViewType(int position) { return position; }
