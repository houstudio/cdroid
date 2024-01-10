#ifndef __ITEMTOUCH_HELPER_H__
#define __ITEMTOUCH_HELPER_H__
#include <widgetEx/recyclerview/recyclerview.h>
namespace cdroid{

class ItemTouchHelper:public RecyclerView::ItemDecoration{
        //implements RecyclerView.OnChildAttachStateChangeListener {
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
private:
    private static constexpr String TAG = "ItemTouchHelper";
    private static constexpr boolean DEBUG = false;
    private static constexpr int ACTIVE_POINTER_ID_NONE = -1;
    static constexpr int DIRECTION_FLAG_COUNT = 8;
    private static constexpr int ACTION_MODE_IDLE_MASK = (1 << DIRECTION_FLAG_COUNT) - 1;
    static constexpr int ACTION_MODE_SWIPE_MASK = ACTION_MODE_IDLE_MASK << DIRECTION_FLAG_COUNT;
    static constexpr int ACTION_MODE_DRAG_MASK = ACTION_MODE_SWIPE_MASK << DIRECTION_FLAG_COUNT;
    private static constexpr int PIXELS_PER_SECOND = 1000;
    final List<View> mPendingCleanup = new ArrayList<>();
    private final float[] mTmpPosition = new float[2];
    private ViewHolder mSelected = null;
    private float mInitialTouchX;
    private float mInitialTouchY;
    private float mSwipeEscapeVelocity;
    private float mMaxSwipeVelocity;
    private float mDx;
    private float mDy;
    private float mSelectedStartX;

    private float mSelectedStartY;

    private int mActivePointerId = ACTIVE_POINTER_ID_NONE;

    @NonNull
    private Callback mCallback;
    private int mActionState = ACTION_STATE_IDLE;
    private int mSelectedFlags;
    List<RecoverAnimation> mRecoverAnimations = new ArrayList<>();

    private int mSlop;

    private RecyclerView mRecyclerView;
    private final Runnable mScrollRunnable = new Runnable() {
        @Override
        public void run() {
            if (mSelected != null && scrollIfNecessary()) {
                if (mSelected != null) { //it might be lost during scrolling
                    moveIfNecessary(mSelected);
                }
                mRecyclerView.removeCallbacks(mScrollRunnable);
                ViewCompat.postOnAnimation(mRecyclerView, this);
            }
        }
    };

    private VelocityTracker mVelocityTracker;
    private List<ViewHolder> mSwapTargets;
    private List<Integer> mDistances;
    private RecyclerView.ChildDrawingOrderCallback mChildDrawingOrderCallback = null;
    private View mOverdrawChild = null;
    private int mOverdrawChildPosition = -1;
    private GestureDetectorCompat mGestureDetector;

    private ItemTouchHelperGestureListener mItemTouchHelperGestureListener;

    private final OnItemTouchListener mOnItemTouchListener = new OnItemTouchListener() {
        @Override
        public boolean onInterceptTouchEvent(@NonNull RecyclerView recyclerView,
                @NonNull MotionEvent event) {
        }

        @Override
        public void onTouchEvent(@NonNull RecyclerView recyclerView, @NonNull MotionEvent event) {
        }

        @Override
        public void onRequestDisallowInterceptTouchEvent(boolean disallowIntercept) {
        }
    };

    private Rect mTmpRect;

    private long mDragScrollStartTimeInMs;

    public ItemTouchHelper(@NonNull Callback callback);

    private static boolean hitTest(View child, float x, float y, float left, float top);

    public void attachToRecyclerView(@Nullable RecyclerView recyclerView) {
    private void setupCallbacks() {
    private void destroyCallbacks() {
    private void startGestureDetection() {
    private void stopGestureDetection() {
    private void getSelectedDxDy(float[] outPosition) {
    public void onDrawOver(Canvas c, RecyclerView parent, RecyclerView.State state)override;
    public void onDraw(Canvas c, RecyclerView parent, RecyclerView.State state)override;
    private void select(@Nullable ViewHolder selected, int actionState) {

    private void postDispatchSwipe(final RecoverAnimation anim, final int swipeDir) {
    private boolean hasRunningRecoverAnim() {
    private boolean scrollIfNecessary() {
    private List<ViewHolder> findSwapTargets(ViewHolder viewHolder) {
    private void moveIfNecessary(ViewHolder viewHolder) {
    public void onChildViewAttachedToWindow(@NonNull View view)override;

    public void onChildViewDetachedFromWindow(@NonNull View view)override;
    private void endRecoverAnimation(ViewHolder viewHolder, boolean override) {
    public void getItemOffsets(Rect outRect, View view, RecyclerView parent,
            RecyclerView.State state)override;
    private void obtainVelocityTracker() {
    private void releaseVelocityTracker() {
    private ViewHolder findSwipedView(MotionEvent motionEvent) {
    private void checkSelectForSwipe(int action, MotionEvent motionEvent, int pointerIndex) {
    private View findChildView(MotionEvent event) {
    public void startDrag(@NonNull ViewHolder viewHolder) {
    public void startSwipe(@NonNull ViewHolder viewHolder) {
    private RecoverAnimation findAnimation(MotionEvent event) {
    private void updateDxDy(MotionEvent ev, int directionFlags, int pointerIndex) {
    private int swipeIfNecessary(ViewHolder viewHolder) {
    private int checkHorizontalSwipe(ViewHolder viewHolder, int flags) {
    private int checkVerticalSwipe(ViewHolder viewHolder, int flags) {
    private void addChildDrawingOrderCallback() {
    private void removeChildDrawingOrderCallbackIfNecessary(View view) {
    
    public interface ViewDropHandler {

        void prepareForDrop(@NonNull View view, @NonNull View target, int x, int y);
    }

    public abstract static class Callback {

        @SuppressWarnings("WeakerAccess")
        public static final int DEFAULT_DRAG_ANIMATION_DURATION = 200;

        @SuppressWarnings("WeakerAccess")
        public static final int DEFAULT_SWIPE_ANIMATION_DURATION = 250;

        static final int RELATIVE_DIR_FLAGS = START | END
                | ((START | END) << DIRECTION_FLAG_COUNT)
                | ((START | END) << (2 * DIRECTION_FLAG_COUNT));

        private static final int ABS_HORIZONTAL_DIR_FLAGS = LEFT | RIGHT
                | ((LEFT | RIGHT) << DIRECTION_FLAG_COUNT)
                | ((LEFT | RIGHT) << (2 * DIRECTION_FLAG_COUNT));

        private static final Interpolator sDragScrollInterpolator = new Interpolator() {
            @Override
            public float getInterpolation(float t) {
                return t * t * t * t * t;
            }
        };

        private static final Interpolator sDragViewScrollCapInterpolator = new Interpolator() {
            @Override
            public float getInterpolation(float t) {
                t -= 1.0f;
                return t * t * t * t * t + 1.0f;
            }
        };

        private static final long DRAG_SCROLL_ACCELERATION_LIMIT_TIME_MS = 2000;

        private int mCachedMaxScrollSpeed = -1;

        @NonNull
        public static ItemTouchUIUtil getDefaultUIUtil() {
            return ItemTouchUIUtilImpl.INSTANCE;
        }

        public static int convertToRelativeDirection(int flags, int layoutDirection) {
            int masked = flags & ABS_HORIZONTAL_DIR_FLAGS;
            if (masked == 0) {
                return flags; // does not have any abs flags, good.
            }
            flags &= ~masked; //remove left / right.
            if (layoutDirection == ViewCompat.LAYOUT_DIRECTION_LTR) {
                // no change. just OR with 2 bits shifted mask and return
                flags |= masked << 2; // START is 2 bits after LEFT, END is 2 bits after RIGHT.
                return flags;
            } else {
                // add RIGHT flag as START
                flags |= ((masked << 1) & ~ABS_HORIZONTAL_DIR_FLAGS);
                // first clean RIGHT bit then add LEFT flag as END
                flags |= ((masked << 1) & ABS_HORIZONTAL_DIR_FLAGS) << 2;
            }
            return flags;
        }

        public static int makeMovementFlags(int dragFlags, int swipeFlags) {
            return makeFlag(ACTION_STATE_IDLE, swipeFlags | dragFlags)
                    | makeFlag(ACTION_STATE_SWIPE, swipeFlags)
                    | makeFlag(ACTION_STATE_DRAG, dragFlags);
        }

        public static int makeFlag(int actionState, int directions) {
            return directions << (actionState * DIRECTION_FLAG_COUNT);
        }

        public abstract int getMovementFlags(@NonNull RecyclerView recyclerView,
                @NonNull ViewHolder viewHolder);

        public int convertToAbsoluteDirection(int flags, int layoutDirection) {
            int masked = flags & RELATIVE_DIR_FLAGS;
            if (masked == 0) {
                return flags; // does not have any relative flags, good.
            }
            flags &= ~masked; //remove start / end
            if (layoutDirection == ViewCompat.LAYOUT_DIRECTION_LTR) {
                // no change. just OR with 2 bits shifted mask and return
                flags |= masked >> 2; // START is 2 bits after LEFT, END is 2 bits after RIGHT.
                return flags;
            } else {
                // add START flag as RIGHT
                flags |= ((masked >> 1) & ~RELATIVE_DIR_FLAGS);
                // first clean start bit then add END flag as LEFT
                flags |= ((masked >> 1) & RELATIVE_DIR_FLAGS) >> 2;
            }
            return flags;
        }

        final int getAbsoluteMovementFlags(RecyclerView recyclerView,
                ViewHolder viewHolder) {
            final int flags = getMovementFlags(recyclerView, viewHolder);
            return convertToAbsoluteDirection(flags, ViewCompat.getLayoutDirection(recyclerView));
        }

        boolean hasDragFlag(RecyclerView recyclerView, ViewHolder viewHolder) {
            final int flags = getAbsoluteMovementFlags(recyclerView, viewHolder);
            return (flags & ACTION_MODE_DRAG_MASK) != 0;
        }

        boolean hasSwipeFlag(RecyclerView recyclerView,
                ViewHolder viewHolder) {
            final int flags = getAbsoluteMovementFlags(recyclerView, viewHolder);
            return (flags & ACTION_MODE_SWIPE_MASK) != 0;
        }

        public boolean canDropOver(@NonNull RecyclerView recyclerView, @NonNull ViewHolder current,
                @NonNull ViewHolder target) {
            return true;
        }

        public abstract boolean onMove(@NonNull RecyclerView recyclerView,
                @NonNull ViewHolder viewHolder, @NonNull ViewHolder target);

        public boolean isLongPressDragEnabled() {
            return true;
        }

        public boolean isItemViewSwipeEnabled() {
            return true;
        }

        public int getBoundingBoxMargin() {
            return 0;
        }

        public float getSwipeThreshold(@NonNull ViewHolder viewHolder) {
            return .5f;
        }

        public float getMoveThreshold(@NonNull ViewHolder viewHolder) {
            return .5f;
        }

        public float getSwipeEscapeVelocity(float defaultValue) {
            return defaultValue;
        }

        public float getSwipeVelocityThreshold(float defaultValue) {
            return defaultValue;
        }

        public ViewHolder chooseDropTarget(@NonNull ViewHolder selected,
                @NonNull List<ViewHolder> dropTargets, int curX, int curY) {

        public abstract void onSwiped(@NonNull ViewHolder viewHolder, int direction);

        public void onSelectedChanged(@Nullable ViewHolder viewHolder, int actionState) {
        private int getMaxDragScroll(RecyclerView recyclerView) {
        public void onMoved(@NonNull final RecyclerView recyclerView,
                @NonNull final ViewHolder viewHolder, int fromPos, @NonNull final ViewHolder target,
                int toPos, int x, int y) {
        void onDraw(Canvas c, RecyclerView parent, ViewHolder selected,
                List<ItemTouchHelper.RecoverAnimation> recoverAnimationList,
                int actionState, float dX, float dY) {
        void onDrawOver(Canvas c, RecyclerView parent, ViewHolder selected,
                List<ItemTouchHelper.RecoverAnimation> recoverAnimationList,
                int actionState, float dX, float dY) {
        public void clearView(@NonNull RecyclerView recyclerView, @NonNull ViewHolder viewHolder) {
        public void onChildDraw(@NonNull Canvas c, @NonNull RecyclerView recyclerView,
                @NonNull ViewHolder viewHolder,
                float dX, float dY, int actionState, boolean isCurrentlyActive) {
        public void onChildDrawOver(@NonNull Canvas c, @NonNull RecyclerView recyclerView,
                ViewHolder viewHolder,
                float dX, float dY, int actionState, boolean isCurrentlyActive) {
        public long getAnimationDuration(@NonNull RecyclerView recyclerView, int animationType,
                float animateDx, float animateDy) {
        public int interpolateOutOfBoundsScroll(@NonNull RecyclerView recyclerView,
                int viewSize, int viewSizeOutOfBounds,
                int totalSize, long msSinceStartScroll);
    };

    public abstract static class SimpleCallback extends Callback {
        private int mDefaultSwipeDirs;
        private int mDefaultDragDirs;

        public SimpleCallback(int dragDirs, int swipeDirs) {
            mDefaultSwipeDirs = swipeDirs;
            mDefaultDragDirs = dragDirs;
        }

        public void setDefaultSwipeDirs(@SuppressWarnings("unused") int defaultSwipeDirs) {
            mDefaultSwipeDirs = defaultSwipeDirs;
        }

        public void setDefaultDragDirs(@SuppressWarnings("unused") int defaultDragDirs) {
            mDefaultDragDirs = defaultDragDirs;
        }

        public int getSwipeDirs(@SuppressWarnings("unused") @NonNull RecyclerView recyclerView,
                @NonNull @SuppressWarnings("unused") ViewHolder viewHolder) {
            return mDefaultSwipeDirs;
        }

        public int getDragDirs(@SuppressWarnings("unused") @NonNull RecyclerView recyclerView,
                @SuppressWarnings("unused") @NonNull ViewHolder viewHolder) {
            return mDefaultDragDirs;
        }

        @Override
        public int getMovementFlags(@NonNull RecyclerView recyclerView,
                @NonNull ViewHolder viewHolder) {
            return makeMovementFlags(getDragDirs(recyclerView, viewHolder),
                    getSwipeDirs(recyclerView, viewHolder));
        }
    }

    private class ItemTouchHelperGestureListener extends GestureDetector.SimpleOnGestureListener {

        private boolean mShouldReactToLongPress = true;

        ItemTouchHelperGestureListener() {
        }

        void doNotReactToLongPress() {
            mShouldReactToLongPress = false;
        }

        @Override
        public boolean onDown(MotionEvent e) {
            return true;
        }

        @Override
        public void onLongPress(MotionEvent e) {
            if (!mShouldReactToLongPress) {
                return;
            }
            View child = findChildView(e);
            if (child != null) {
                ViewHolder vh = mRecyclerView.getChildViewHolder(child);
                if (vh != null) {
                    if (!mCallback.hasDragFlag(mRecyclerView, vh)) {
                        return;
                    }
                    int pointerId = e.getPointerId(0);
                    // Long press is deferred.
                    // Check w/ active pointer id to avoid selecting after motion
                    // event is canceled.
                    if (pointerId == mActivePointerId) {
                        final int index = e.findPointerIndex(mActivePointerId);
                        final float x = e.getX(index);
                        final float y = e.getY(index);
                        mInitialTouchX = x;
                        mInitialTouchY = y;
                        mDx = mDy = 0f;
                        if (DEBUG) {
                            Log.d(TAG,
                                    "onlong press: x:" + mInitialTouchX + ",y:" + mInitialTouchY);
                        }
                        if (mCallback.isLongPressDragEnabled()) {
                            select(vh, ACTION_STATE_DRAG);
                        }
                    }
                }
            }
        }
    }

    private static class RecoverAnimation implements Animator.AnimatorListener {

        final float mStartDx;
        final float mStartDy;
        final float mTargetX;
        final float mTargetY;
        final ViewHolder mViewHolder;
        final int mActionState;
        private final ValueAnimator mValueAnimator;
        final int mAnimationType;
        boolean mIsPendingCleanup;
        float mX;
        float mY;
        boolean mOverridden = false;
        boolean mEnded = false;
        private float mFraction;
        RecoverAnimation(ViewHolder viewHolder, int animationType,
                int actionState, float startDx, float startDy, float targetX, float targetY);
        public void setDuration(long duration) {
            mValueAnimator.setDuration(duration);
        }

        public void start() {
            mViewHolder.setIsRecyclable(false);
            mValueAnimator.start();
        }

        public void cancel() {
            mValueAnimator.cancel();
        }

        public void setFraction(float fraction) {
            mFraction = fraction;
        }

        public void update();

        @Override
        public void onAnimationStart(Animator animation) {

        }

        @Override
        public void onAnimationEnd(Animator animation) {
        @Override
        public void onAnimationCancel(Animator animation) {

        @Override
        public void onAnimationRepeat(Animator animation) {

        }
    }
};
}/*endof namespace*/
#endif/*__ITEMTOUCH_HELPER_H__*/


