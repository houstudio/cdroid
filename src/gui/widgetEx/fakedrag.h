#ifndef __FAKE_DRAG_H__
#define __FAKE_DRAG_H__
#include <widgetEx/recyclerview/recyclerview.h>
class ViewPager2;
namespace cdroid{
class ScrollEventAdapter;
class FakeDrag {
private:
    ViewPager2* mViewPager;
    ScrollEventAdapter* mScrollEventAdapter;
    RecyclerView* mRecyclerView;
    VelocityTracker* mVelocityTracker;
    int mMaximumVelocity;
    float mRequestedDragDistance;
    int mActualDraggedDistance;
    long mFakeDragBeginTime;
private:
    void beginFakeVelocityTracker();
    void addFakeMotionEvent(long time, int action, float x, float y);
public:
    FakeDrag(ViewPager2* viewPager, ScrollEventAdapter* scrollEventAdapter, RecyclerView* recyclerView);
    bool isFakeDragging();
    bool beginFakeDrag();
    bool fakeDragBy(float offsetPxFloat);
    bool endFakeDrag();
};
}/*endof namespace*/
#endif
