#ifndef __SNAP_HELPER_H__
#define __SNAP_HELPER_H__
#include <widgetEx/recyclerview/recyclerview.h>

namespace cdroid{
class OrientationHelper;
class LinearSmoothScroller;
class SnapHelper{// extends RecyclerView.OnFlingListener {
public:
    static constexpr float MILLISECONDS_PER_INCH = 100.f;
private:
    bool mScrolled;
    Scroller* mGravityScroller;
    // Handles the snap on scroll case.
    RecyclerView::OnScrollListener mScrollListener;
protected:
    RecyclerView* mRecyclerView;
private:
    void setupCallbacks();
    void destroyCallbacks();
    bool snapFromFling(RecyclerView::LayoutManager& layoutManager, int velocityX,int velocityY);
protected:
    void snapToTargetExistingView();
    RecyclerView::SmoothScroller* createScroller(RecyclerView::LayoutManager& layoutManager);
    LinearSmoothScroller* createSnapScroller(RecyclerView::LayoutManager& layoutManager);
public:
    SnapHelper();
    virtual ~SnapHelper();
    bool onFling(int velocityX, int velocityY);
    void attachToRecyclerView(RecyclerView* recyclerView);

    void calculateScrollDistance(int velocityX, int velocityY,int snapDistance[2]);
    virtual void calculateDistanceToFinalSnap(RecyclerView::LayoutManager& layoutManager,View& targetView,int distance[2])=0;
    virtual View* findSnapView(RecyclerView::LayoutManager& layoutManager)=0;

    virtual int findTargetSnapPosition(RecyclerView::LayoutManager& layoutManager, int velocityX,
            int velocityY)=0;
};
}/*endof namespace*/
#endif/*__SNAP_HELPER_H__*/
