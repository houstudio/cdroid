#ifndef __PAGER_SNAP_HELPER_H__
#define __PAGER_SNAP_HELPER_H__
#include <widgetEx/recyclerview/snaphelper.h>
namespace cdroid{
class PagerSnapHelper:public SnapHelper {
public:
    static constexpr int MAX_SCROLL_ON_FLING_DURATION = 100; // ms
private:
    OrientationHelper* mVerticalHelper;
    OrientationHelper* mHorizontalHelper;
private:
    int distanceToCenter(RecyclerView::LayoutManager& layoutManager, View& targetView, OrientationHelper& helper);
    View* findCenterView(RecyclerView::LayoutManager& layoutManager, OrientationHelper& helper);
    View* findStartView(RecyclerView::LayoutManager& layoutManager, OrientationHelper& helper);
    OrientationHelper& getVerticalHelper(RecyclerView::LayoutManager& layoutManager);
    OrientationHelper& getHorizontalHelper(RecyclerView::LayoutManager& layoutManager);
protected:
    LinearSmoothScroller* createSnapScroller(RecyclerView::LayoutManager& layoutManager);
public:
    PagerSnapHelper();
    ~PagerSnapHelper()override;
    void calculateDistanceToFinalSnap(RecyclerView::LayoutManager& layoutManager, View& targetView,int distance[2])override;
    View* findSnapView(RecyclerView::LayoutManager& layoutManager)override;
    int findTargetSnapPosition(RecyclerView::LayoutManager& layoutManager, int velocityX,int velocityY)override;
};
}/*endof namespace*/
#endif/*__PAGER_SNAP_HELPER_H__*/
