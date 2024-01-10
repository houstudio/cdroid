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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
#endif
