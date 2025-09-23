#ifndef __WEARABLE_LINEAR_LAYOUTMANAGER_H__
#define __WEARABLE_LINEAR_LAYOUTMANAGER_H__
#include <widgetEx/recyclerview/linearlayoutmanager.h>
namespace cdroid{
class CurvingLayoutCallback;
class WearableLinearLayoutManager:public LinearLayoutManager {
public:
    DECLARE_UIEVENT(void,LayoutCallback,View&,RecyclerView&);
private:
    CurvingLayoutCallback*mCurvingLayoutCallback;
    LayoutCallback mLayoutCallback;
    void updateLayout();
public:
    WearableLinearLayoutManager(Context* context);
    WearableLinearLayoutManager(Context* context,const LayoutCallback& layoutCallback);
    ~WearableLinearLayoutManager()override;

    void setLayoutCallback(const LayoutCallback& layoutCallback);
    LayoutCallback getLayoutCallback() const;

    int scrollVerticallyBy(int dy, RecyclerView::Recycler& recycler, RecyclerView::State& state) override;
    void onLayoutChildren(RecyclerView::Recycler& recycler, RecyclerView::State& state) override;
};
}/*endof namespace*/
#endif/*__WEARABLE_LINEAR_LAYOUTMANAGER_H__*/
