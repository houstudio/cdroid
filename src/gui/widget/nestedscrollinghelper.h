#ifndef __NESTED_SCROLLING_HELPERS_H__
#define __NESTED_SCROLLING_HELPERS_H__
#include <view/viewgroup.h>

namespace cdroid{

class NestedScrollingParentHelper {
private:
    ViewGroup* mViewGroup;
    int mNestedScrollAxes;
public:
    NestedScrollingParentHelper(ViewGroup* viewGroup); 
    void onNestedScrollAccepted(View* child,View* target,int axes);
    void onNestedScrollAccepted(View* child,View* target,int axes,int type);
    int  getNestedScrollAxes()const;
    void onStopNestedScroll(View* target);
    void onStopNestedScroll(View* target,int type); 
};

class NestedScrollingChildHelper {
private :
   ViewGroup* mNestedScrollingParentTouch;
   ViewGroup* mNestedScrollingParentNonTouch;
   View* mView;
   bool mIsNestedScrollingEnabled;
   int  mTempNestedScrollConsumed[2];
private:
    ViewGroup* getNestedScrollingParentForType(int type);
    void setNestedScrollingParentForType(int type, ViewGroup* p);

public:
    NestedScrollingChildHelper(View* view);
    void setNestedScrollingEnabled(bool enabled);

    bool isNestedScrollingEnabled();
    bool hasNestedScrollingParent();
    bool hasNestedScrollingParent(int type);
    bool startNestedScroll( int axes);
    bool startNestedScroll( int axes, int type);
    void stopNestedScroll();

    void stopNestedScroll(int type);
    bool dispatchNestedScroll(int dxConsumed, int dyConsumed,
        int dxUnconsumed, int dyUnconsumed, int offsetInWindow[]);

    bool dispatchNestedScroll(int dxConsumed, int dyConsumed,
       int dxUnconsumed, int dyUnconsumed, int offsetInWindow[],int type);

    bool dispatchNestedPreScroll(int dx, int dy, int consumed[],int offsetInWindow[]);

    bool dispatchNestedPreScroll(int dx, int dy, int consumed[] ,int offsetInWindow[],int type);
    bool dispatchNestedFling(float velocityX, float velocityY, bool consumed);
    bool dispatchNestedPreFling(float velocityX, float velocityY);
    void onDetachedFromWindow();
    void onStopNestedScroll( View* child);
};
}//endof namespace

#endif//__NESTED_SCROLLING_HELPERS_H__
