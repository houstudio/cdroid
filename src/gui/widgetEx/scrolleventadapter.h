#ifndef __SCROLLEVENT_ADAPTER_H__
#define __SCROLLEVENT_ADAPTER_H__
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/viewpager2.h>
namespace cdroid{
class ScrollEventAdapter{
private:
    static constexpr int STATE_IDLE = 0;
    static constexpr int STATE_IN_PROGRESS_MANUAL_DRAG = 1;
    static constexpr int STATE_IN_PROGRESS_SMOOTH_SCROLL = 2;
    static constexpr int STATE_IN_PROGRESS_IMMEDIATE_SCROLL = 3;
    static constexpr int STATE_IN_PROGRESS_FAKE_DRAG = 4;
    static constexpr int NO_POSITION = -1;
private:
    class ScrollEventValues {
    public:
        int mPosition;
        float mOffset;
        int mOffsetPx;
        // to avoid a synthetic accessor
        ScrollEventValues();
        void reset();
    };
    ViewPager2::OnPageChangeCallback mCallback;
    ViewPager2* mViewPager;
    RecyclerView* mRecyclerView;
    LinearLayoutManager* mLayoutManager;
    ScrollEventValues mScrollValues;
    // state related fields
    int mAdapterState;
    int mScrollState;
    int mDragStartPosition;
    int mTarget;
    bool mDispatchSelected;
    bool mScrollHappened;
    bool mDataSetChangeHappened;
    bool mFakeDragging;
private:
    void resetState();
    void updateScrollEventValues();
    void startDrag(bool isFakeDrag);
    bool isInAnyDraggingState();
    void dispatchStateChanged(int state);
    void dispatchSelected(int target);
    void dispatchScrolled(int position, float offset, int offsetPx);
    int getPosition();
public:
    ScrollEventAdapter(ViewPager2* viewPager);
    virtual void onScrollStateChanged(RecyclerView& recyclerView, int newState);
    virtual void onScrolled(RecyclerView& recyclerView, int dx, int dy);

    void notifyDataSetChangeHappened();
    void notifyProgrammaticScroll(int target, bool smooth);
    void notifyBeginFakeDrag();
    void notifyEndFakeDrag();
    void setOnPageChangeCallback(const ViewPager2::OnPageChangeCallback& callback);
    int getScrollState();
    bool isIdle();
    bool isDragging();
    bool isFakeDragging();
    double getRelativeScrollPosition();
};
}/*endof namespace*/
#endif/*__SCROLLEVENT_ADAPTER_H__*/
