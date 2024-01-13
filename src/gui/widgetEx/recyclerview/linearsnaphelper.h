#ifndef __LINEAR_SNAP_HELPER_H__
#define __LINEAR_SNAP_HELPER_H__
#include <widgetEx/recyclerview/snaphelper.h>
namespace cdroid{
class LinearSnapHelper:public SnapHelper {
private:
    static constexpr float INVALID_DISTANCE = 1.f;

    // Orientation helpers are lazily created per LayoutManager.
    OrientationHelper* mVerticalHelper;
    OrientationHelper* mHorizontalHelper;
private:
    int distanceToCenter(RecyclerView::LayoutManager& layoutManager, View& targetView, OrientationHelper& helper);
    int estimateNextPositionDiffForFling(RecyclerView::LayoutManager& layoutManager,
            OrientationHelper& helper, int velocityX, int velocityY);
    View* findCenterView(RecyclerView::LayoutManager& layoutManager, OrientationHelper& helper);
    float computeDistancePerChild(RecyclerView::LayoutManager& layoutManager, OrientationHelper& helper);
    OrientationHelper& getVerticalHelper(RecyclerView::LayoutManager& layoutManager);
    OrientationHelper& getHorizontalHelper(RecyclerView::LayoutManager& layoutManager);
public:
    LinearSnapHelper();
    ~LinearSnapHelper()override;
    void calculateDistanceToFinalSnap(RecyclerView::LayoutManager& layoutManager,View& targetView,int distance[2])override;
    int findTargetSnapPosition(RecyclerView::LayoutManager& layoutManager, int velocityX,int velocityY)override;
    View* findSnapView(RecyclerView::LayoutManager& layoutManager)override;
};
}/*endof namespace*/
#endif/*__LINEAR_SNAP_HELPER_H__*/
