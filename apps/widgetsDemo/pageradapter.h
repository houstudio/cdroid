#pragma once
#include <widgetEx/recyclerview/recyclerview.h>

// Drives the ViewPager2: one distinct view type per demo page, so each page XML
// is inflated exactly once and bound by the matching setup*() in pages.cc.
class DemoPagerAdapter : public cdroid::RecyclerView::Adapter {
public:
    cdroid::RecyclerView::ViewHolder* onCreateViewHolder(cdroid::ViewGroup* parent, int viewType) override;
    void onBindViewHolder(cdroid::RecyclerView::ViewHolder& holder, int position) override;
    int getItemCount() override;
    int getItemViewType(int position) override;
};
