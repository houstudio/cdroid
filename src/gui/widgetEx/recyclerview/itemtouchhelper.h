#ifndef __ITEMTOUCH_HELPER_H__
#define __ITEMTOUCH_HELPER_H__
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/recyclerview/itemtouchuiutil.h>
namespace cdroid{

class ItemTouchHelper:public RecyclerView::ItemDecoration{
    //implements RecyclerView.OnChildAttachStateChangeListener {
protected:
    static constexpr bool _DEBUG = false;
    static constexpr int ACTIVE_POINTER_ID_NONE = -1;
    static constexpr int DIRECTION_FLAG_COUNT = 8;
    static constexpr int ACTION_MODE_IDLE_MASK = (1 << DIRECTION_FLAG_COUNT) - 1;
    static constexpr int PIXELS_PER_SECOND = 1000;
    static constexpr int ACTION_MODE_SWIPE_MASK = ACTION_MODE_IDLE_MASK << DIRECTION_FLAG_COUNT;
    static constexpr int ACTION_MODE_DRAG_MASK = ACTION_MODE_SWIPE_MASK << DIRECTION_FLAG_COUNT;
public:
    static constexpr int UP = 1;
    static constexpr int DOWN = 1 << 1;
    static constexpr int LEFT = 1 << 2;
    static constexpr int RIGHT = 1 << 3;

    // If you change these relative direction values, update Callback#convertToAbsoluteDirection,
    // Callback#convertToRelativeDirection.

    static constexpr int START = LEFT << 2;
    static constexpr int END = RIGHT << 2;
    static constexpr int ACTION_STATE_IDLE = 0;
    static constexpr int ACTION_STATE_SWIPE = 1;
    static constexpr int ACTION_STATE_DRAG = 2;
    static constexpr int ANIMATION_TYPE_SWIPE_SUCCESS = 1 << 1;
    static constexpr int ANIMATION_TYPE_SWIPE_CANCEL = 1 << 2;
    static constexpr int ANIMATION_TYPE_DRAG = 1 << 3;
    class Callback;
    class SimpleCallback;
    class RecoverAnimation;
    class ItemTouchHelperGestureListener;
protected:
    std::vector<View*> mPendingCleanup;
    std::vector<RecoverAnimation*> mRecoverAnimations;
    RecyclerView* mRecyclerView;
private:
    float mTmpPosition[2];
    RecyclerView::ViewHolder* mSelected = nullptr;
    float mInitialTouchX;
    float mInitialTouchY;
    float mSwipeEscapeVelocity;
    float mMaxSwipeVelocity;
    float mDx;
    float mDy;
    float mSelectedStartX;
    float mSelectedStartY;
    int mActivePointerId = ACTIVE_POINTER_ID_NONE;

    Callback* mCallback;
    int mActionState = ACTION_STATE_IDLE;
    int mSelectedFlags;
    int mSlop;

    Runnable mScrollRunnable;
    VelocityTracker* mVelocityTracker;
    std::vector<RecyclerView::ViewHolder*> mSwapTargets;
    std::vector<int> mDistances;
    RecyclerView::ChildDrawingOrderCallback mChildDrawingOrderCallback;
    View* mOverdrawChild = nullptr;
    int mOverdrawChildPosition = -1;
    //GestureDetectorCompat mGestureDetector;
    //ItemTouchHelperGestureListener mItemTouchHelperGestureListener;

    RecyclerView::OnItemTouchListener mOnItemTouchListener;
    RecyclerView::OnChildAttachStateChangeListener mOnChildAttachStateChangeListener;
    long mDragScrollStartTimeInMs;
    static bool hitTest(View& child, float x, float y, float left, float top);

    void setupCallbacks();
    void destroyCallbacks();
    void startGestureDetection();
    void stopGestureDetection();
    void getSelectedDxDy(float outPosition[2]);
    void select(RecyclerView::ViewHolder* selected, int actionState);

    void postDispatchSwipe(RecoverAnimation* anim,int swipeDir);
    bool hasRunningRecoverAnim();
    bool scrollIfNecessary();
    std::vector<RecyclerView::ViewHolder*> findSwapTargets(RecyclerView::ViewHolder* viewHolder);
    void moveIfNecessary(RecyclerView::ViewHolder& viewHolder);
    void endRecoverAnimation(RecyclerView::ViewHolder& viewHolder, bool overrided);
    void obtainVelocityTracker();
    void releaseVelocityTracker();
    RecyclerView::ViewHolder* findSwipedView(MotionEvent& motionEvent);
    void checkSelectForSwipe(int action, MotionEvent& motionEvent, int pointerIndex);
         View* findChildView(MotionEvent& event);
    RecoverAnimation* findAnimation(MotionEvent& event);
    void updateDxDy(MotionEvent& ev, int directionFlags, int pointerIndex);
    int swipeIfNecessary(RecyclerView::ViewHolder& viewHolder);
    int checkHorizontalSwipe(RecyclerView::ViewHolder& viewHolder, int flags);
    int checkVerticalSwipe(RecyclerView::ViewHolder& viewHolder, int flags);
    void addChildDrawingOrderCallback();
    void removeChildDrawingOrderCallbackIfNecessary(View* view);
    /*binding functions for mOnItemTouchListener*/
    bool onInterceptTouchEvent(RecyclerView& recyclerView,MotionEvent& event);
    void onTouchEvent(RecyclerView& recyclerView,MotionEvent& event);
    void onRequestDisallowInterceptTouchEvent(bool disallowIntercept);
public:
    ItemTouchHelper(Callback* callback);
    void attachToRecyclerView(RecyclerView* recyclerView);
    void onDrawOver(Canvas& c, RecyclerView& parent, RecyclerView::State& state)override;
    void onDraw(Canvas& c, RecyclerView& parent, RecyclerView::State& state)override;
    void onChildViewAttachedToWindow(View& view);
    void onChildViewDetachedFromWindow(View& view);
    void getItemOffsets(Rect& outRect, View& view, RecyclerView& parent, RecyclerView::State& state)override;
    void startDrag(RecyclerView::ViewHolder& viewHolder);
    void startSwipe(RecyclerView::ViewHolder& viewHolder);

    /*public interface ViewDropHandler {
        void prepareForDrop(View& view,View& target, int x, int y);
    };*/
};/*endof ItemTouchHelper*/

class ItemTouchHelper::Callback {
private:
    static constexpr int ABS_HORIZONTAL_DIR_FLAGS = LEFT | RIGHT | ((LEFT | RIGHT) << DIRECTION_FLAG_COUNT)
            | ((LEFT | RIGHT) << (2 * DIRECTION_FLAG_COUNT));
    static constexpr long DRAG_SCROLL_ACCELERATION_LIMIT_TIME_MS = 2000;
public:
    static constexpr int DEFAULT_DRAG_ANIMATION_DURATION = 200;
    static constexpr int DEFAULT_SWIPE_ANIMATION_DURATION = 250;
    static constexpr int RELATIVE_DIR_FLAGS = START | END | ((START | END) << DIRECTION_FLAG_COUNT)
            | ((START | END) << (2 * DIRECTION_FLAG_COUNT));
private:
    friend ItemTouchHelper;
    static Interpolator *sDragScrollInterpolator;
    static Interpolator *sDragViewScrollCapInterpolator;
    int mCachedMaxScrollSpeed = -1;
    int getMaxDragScroll(RecyclerView& recyclerView);
protected:
    int getAbsoluteMovementFlags(RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder);
    bool hasDragFlag(RecyclerView& recyclerView, RecyclerView::ViewHolder& viewHolder);
    bool hasSwipeFlag(RecyclerView& recyclerView, RecyclerView::ViewHolder& viewHolder);
    void onDraw(Canvas& c, RecyclerView& parent, RecyclerView::ViewHolder* selected,
         std::vector<ItemTouchHelper::RecoverAnimation*>& recoverAnimationList,int actionState, float dX, float dY);
    void onDrawOver(Canvas& c, RecyclerView& parent, RecyclerView::ViewHolder* selected,
         std::vector<ItemTouchHelper::RecoverAnimation*>& recoverAnimationList, int actionState, float dX, float dY);
public:
    static ItemTouchUIUtil& getDefaultUIUtil();
    static int convertToRelativeDirection(int flags, int layoutDirection);
    static int makeMovementFlags(int dragFlags, int swipeFlags);
    static int makeFlag(int actionState, int directions);
    virtual int getMovementFlags(RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder)=0;
    int convertToAbsoluteDirection(int flags, int layoutDirection);
    bool canDropOver(RecyclerView& recyclerView,RecyclerView::ViewHolder& current,RecyclerView::ViewHolder& target);
    virtual bool onMove(RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder,RecyclerView::ViewHolder& target)=0;
    virtual bool isLongPressDragEnabled();
    virtual bool isItemViewSwipeEnabled();
    virtual int getBoundingBoxMargin();
    virtual float getSwipeThreshold(RecyclerView::ViewHolder& viewHolder);
    virtual float getMoveThreshold(RecyclerView::ViewHolder& viewHolder);
    virtual float getSwipeEscapeVelocity(float defaultValue);
    virtual float getSwipeVelocityThreshold(float defaultValue);
    virtual RecyclerView::ViewHolder* chooseDropTarget(RecyclerView::ViewHolder& selected,std::vector<RecyclerView::ViewHolder*>& dropTargets, int curX, int curY);
    virtual void onSwiped(RecyclerView::ViewHolder& viewHolder, int direction)=0;
    virtual void onSelectedChanged(RecyclerView::ViewHolder* viewHolder, int actionState);
    virtual void onMoved(RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder, int fromPos,RecyclerView::ViewHolder& target,
            int toPos, int x, int y);
    virtual void clearView(RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder);
    virtual void onChildDraw(Canvas& c, RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder,
            float dX, float dY, int actionState, bool isCurrentlyActive);
    virtual void onChildDrawOver(Canvas& c,RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder,
            float dX, float dY, int actionState, bool isCurrentlyActive);
    virtual long getAnimationDuration(RecyclerView& recyclerView, int animationType,
            float animateDx, float animateDy);
    virtual int interpolateOutOfBoundsScroll(RecyclerView& recyclerView,
            int viewSize, int viewSizeOutOfBounds,int totalSize, long msSinceStartScroll);
};

class ItemTouchHelper::SimpleCallback:public ItemTouchHelper::Callback{
private:
    friend ItemTouchHelper;
    int mDefaultSwipeDirs;
    int mDefaultDragDirs;
public:
    SimpleCallback(int dragDirs, int swipeDirs);
    void setDefaultSwipeDirs(int defaultSwipeDirs);
    void setDefaultDragDirs(int defaultDragDirs);
    int getSwipeDirs(RecyclerView& recyclerView, RecyclerView::ViewHolder& viewHolder);
    int getDragDirs(RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder);
    int getMovementFlags(RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder)override;
};

#if 0
class ItemTouchHelper::ItemTouchHelperGestureListener:public GestureDetector.SimpleOnGestureListener {
private:
    bool mShouldReactToLongPress = true;
public:
    ItemTouchHelperGestureListener();
    void doNotReactToLongPress();
    bool onDown(MotionEvent& e);
    void onLongPress(MotionEvent& e);
};
#endif

class ItemTouchHelper::RecoverAnimation{
protected:
    friend ItemTouchHelper;
    float mStartDx;
    float mStartDy;
    float mTargetX;
    float mTargetY;
    RecyclerView::ViewHolder* mViewHolder;
    int mActionState;
    ValueAnimator* mValueAnimator;
    int mAnimationType;
    bool mIsPendingCleanup;
    float mX;
    float mY;
    bool mOverridden = false;
    bool mEnded = false;
    float mFraction;
    Animator::AnimatorListener mAnimatorListener;
public:
    RecoverAnimation(RecyclerView::ViewHolder* viewHolder, int animationType,
            int actionState, float startDx, float startDy, float targetX, float targetY);
    virtual ~RecoverAnimation();
    void setDuration(long duration);
    void start();
    void cancel();
    void setFraction(float fraction);
    void update();
    virtual void onAnimationStart(Animator& animation,bool/*isReverse*/);
    virtual void onAnimationEnd(Animator& animation,bool/*isReverse*/);
    virtual void onAnimationCancel(Animator& animation);
    virtual void onAnimationRepeat(Animator& animation);
};
}/*endof namespace*/
#endif/*__ITEMTOUCH_HELPER_H__*/
