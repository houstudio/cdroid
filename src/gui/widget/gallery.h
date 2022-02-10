#ifndef __GALLERY_H__
#define __GALLERY_H__
#include <widget/absspinner.h>
namespace cdroid{

class Gallery:public AbsSpinner{
private:
    static constexpr int SCROLL_TO_FLING_UNCERTAINTY_TIMEOUT = 250;
    class FlingRunnable:public Runnable{
    public:
        Scroller*mScroller;
        int mLastFlingX;
        Gallery*mGallery;
        void startCommon();
    public:
        FlingRunnable(Gallery*g);
        void startUsingVelocity(int initalVelocity);
        void startUsingDistance(int distance);
        void stop(bool scrollIntoSlots);
        void endFling(bool scrollIntoSlots);
        void operator()()override;
    };
    int mSpacing = 0;
    int mAnimationDuration = 400;
    float mUnselectedAlpha;
    int mLeftMost;
    int mRightMost;
    int mGravity;
    int mDownTouchPosition;
    View* mDownTouchView;
    FlingRunnable* mFlingRunnable;
    Runnable mDisableSuppressSelectionChangedRunnable;
    bool mShouldStopFling;
    View* mSelectedChild;
    bool mShouldCallbackDuringFling = true;
    bool mShouldCallbackOnUnselectedItemClick = true;
    bool mSuppressSelectionChanged;
    bool mReceivedInvokeKeyDown;
    //AdapterContextMenuInfo mContextMenuInfo;
    bool mIsFirstScroll;
    bool mIsRtl = true;
    int mSelectedCenterOffset;

private:
    void offsetChildrenLeftAndRight(int offset);
    int getCenterOfGallery();
    static int getCenterOfView(View* view);
    void detachOffScreenChildren(bool toLeft);
    void scrollIntoSlots();
    void onFinishedMovement();
    void setSelectionToCenterChild();
    void fillToGalleryLeft();
    void fillToGalleryLeftRtl();
    void fillToGalleryLeftLtr();
    void fillToGalleryRight();
    void fillToGalleryRightRtl();
    void fillToGalleryRightLtr();
    View*makeAndAddView(int position, int offset, int x, bool fromLeft);
    void setUpChild(View* child, int offset, int x, bool fromLeft);
    int calculateTop(View* child, bool duringLayout);
    void dispatchPress(View* child);
    void dispatchUnpress();
    bool dispatchLongPress(View& view, int position, long id, float x, float y,bool useOffsets);
    bool scrollToChild(int childPosition);
    void updateSelectedItemMetadata();
protected:
    void onAttachedToWindow()override;
    bool getChildStaticTransformation(View* child, Transformation& t);
    int  computeHorizontalScrollExtent()override; 
    int   computeHorizontalScrollOffset()override;
    int computeHorizontalScrollRange()override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    ViewGroup::LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* p)const override;
    ViewGroup::LayoutParams* generateDefaultLayoutParams()const override;
    void onLayout(bool changed, int l, int t, int w, int h)override;
    int getChildHeight(View* child)override;

    void trackMotionScroll(int deltaX);
    int getLimitedMotionScrollAmount(bool motionToLeft, int deltaX);
    void onUp();
    void onCancel();
    bool moveDirection(int direction);

    void selectionChanged()override;
    void layout(int delta, bool animate)override;
    void dispatchSetPressed(bool pressed)override;
    void setSelectedPositionInt(int position)override;
    int getChildDrawingOrder(int childCount, int i)override;
public:
    Gallery(Context* context,const AttributeSet& attrs);
    void setCallbackDuringFling(bool shouldCallback);
    void setCallbackOnUnselectedItemClick(bool shouldCallback);
    void setAnimationDuration(int animationDurationMillis);
    void setSpacing(int spacing);
    void setUnselectedAlpha(float unselectedAlpha);
    ViewGroup::LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;
    bool onTouchEvent(MotionEvent& event)override;

    //override from OnGestureListener BEGIN
    bool onSingleTapUp(MotionEvent& e);
    bool onFling(MotionEvent& e1, MotionEvent& e2, float velocityX, float velocityY);
    bool onScroll(MotionEvent& e1,MotionEvent& e2, float distanceX, float distanceY);
    bool onDown(MotionEvent& e);
    void onLongPress(MotionEvent& e);
    void onShowPress(MotionEvent& e);
    //override from OnGestureListener END;

    void dispatchSetSelected(bool selected)override;
    bool dispatchKeyEvent(KeyEvent& event)override;
    bool onKeyDown(int keyCode, KeyEvent& event)override;
    bool onKeyUp(int keyCode, KeyEvent& event)override;
    void setGravity(int gravity);
};

}//endof namespace
#endif
