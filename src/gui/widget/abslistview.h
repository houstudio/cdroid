#ifndef __ABS_LISTVIEW_H__
#define __ABS_LISTVIEW_H__
#include <core/sparsearray.h>
#include <widget/adapterview.h>
#include <widget/recyclebin.h>
#include <widget/overscroller.h>
#include <widget/edgeeffect.h>
#include <widget/popupwindow.h>
#include <widget/filterable.h>

namespace cdroid{
class ListPopupWindow;
class AbsListView:public AdapterView,Filter::FilterListener{
private:
    static constexpr float FLING_DESTRETCH_FACTOR = 4.f;
    static constexpr int OVERSCROLL_LIMIT_DIVISOR = 3;
    static constexpr int CHECK_POSITION_SEARCH_DISTANCE=20;
public:
    friend RecycleBin;
    friend ListPopupWindow;
    static constexpr int CHOICE_MODE_NONE=0;
    static constexpr int CHOICE_MODE_SINGLE=1;
    static constexpr int CHOICE_MODE_MULTIPLE=2;
    static constexpr int CHOICE_MODE_MULTIPLE_MODAL=3;

    static constexpr int TOUCH_MODE_REST=-1;
    static constexpr int TOUCH_MODE_DOWN=0;
    static constexpr int TOUCH_MODE_TAP=1;
    static constexpr int TOUCH_MODE_DONE_WAITING=2;
    static constexpr int TOUCH_MODE_SCROLL=3;
    static constexpr int TOUCH_MODE_FLING=4;
    static constexpr int TOUCH_MODE_OVERSCROLL=5;
    static constexpr int TOUCH_MODE_OVERFLING =6;

    static constexpr int TRANSCRIPT_MODE_DISABLED=0;
    static constexpr int TRANSCRIPT_MODE_NORMAL=1;
    static constexpr int TRANSCRIPT_MODE_ALWAYS_SCROLL=2;

    static constexpr int LAYOUT_NORMAL =0;
    static constexpr int LAYOUT_FORCE_TOP =1;
    static constexpr int LAYOUT_SET_SELECTION =2;
    static constexpr int LAYOUT_FORCE_BOTTOM =3;
    static constexpr int LAYOUT_SPECIFIC =4;
    static constexpr int LAYOUT_SYNC =5;
    static constexpr int LAYOUT_MOVE_SELECTION =6;

    class LayoutParams:public ViewGroup::LayoutParams{
    private:
        void init();
    public:
        int viewType;
        bool recycledHeaderFooter;
        bool forceAdd;
        int scrappedFromPosition;
        long itemId ;
        bool isEnabled;
        LayoutParams();
        LayoutParams(const ViewGroup::LayoutParams&);
        LayoutParams(int w,int h);
        LayoutParams(int w,int h,int vt);
        LayoutParams(Context*ctx,const AttributeSet&atts);
    };
    struct OnScrollListener{
        static constexpr int SCROLL_STATE_IDLE=0;
        static constexpr int SCROLL_STATE_TOUCH_SCROLL =1;
        static constexpr int SCROLL_STATE_FLING =2;
        // void onScrollStateChanged(AbsListView view, int scrollState)
        std::function<void(AbsListView&view,int)>onScrollStateChanged;
        //void onScroll(AbsListView view, int firstVisibleItem, int visibleItemCount,int totalItemCount);
        std::function<void(AbsListView&view,int,int,int)>onScroll;
    }; 
    DECLARE_UIEVENT(void,MultiChoiceModeListener,/*ActionMode mode,*/int position, long id, bool checked);
protected:
    class ListItemAccessibilityDelegate:public AccessibilityDelegate{
    public:
       void onInitializeAccessibilityNodeInfo(View& host, AccessibilityNodeInfo& info)override;
       bool performAccessibilityAction(View& host, int action, Bundle* arguments)override;
    };
    class AbsPositionScroller:public Runnable {
    public:
        virtual void start(int position)=0;
        virtual void start(int position, int boundPosition)=0;
        virtual void startWithOffset(int position, int offset)=0;
        virtual void startWithOffset(int position, int offset, int duration)=0;
        virtual void stop()=0;
    };
    class PositionScroller:public AbsPositionScroller{
    private:
        friend AbsListView;
        static constexpr int SCROLL_DURATION = 200;    
        static constexpr int MOVE_DOWN_POS = 1;
        static constexpr int MOVE_UP_POS = 2;
        static constexpr int MOVE_DOWN_BOUND = 3;
        static constexpr int MOVE_UP_BOUND = 4;
        static constexpr int MOVE_OFFSET = 5;
    
        int mMode;
        int mTargetPos;
        int mBoundPos;
        int mLastSeenPos;
        int mScrollDuration;
        int mExtraScroll;
        int mOffsetFromTop;
        AbsListView*mLV;
    private:
        void scrollToVisible(int targetPos, int boundPos, int duration);
    public:
        PositionScroller(AbsListView*);
        void start(int position)override;
        void start(int position, int boundPosition)override;
        void startWithOffset(int position, int offset)override;
        void startWithOffset(int position, int offset, int duration)override;
        void stop()override;
        void doScroll();
    };
private:
    class AbsRunnable;
    class CheckForTap;
    class WindowRunnable;
    class PerformClick;
    class CheckForLongPress;
    class CheckForKeyLongPress;
    class FlingRunnable;
private:
    constexpr static bool PROFILE_SCROLLING = false;
    static const bool PROFILE_FLINGING = true;
    enum{
        INVALID_POINTER=-1,
        TOUCH_MODE_UNKNOWN = -1,
        TOUCH_MODE_ON = 0,
        TOUCH_MODE_OFF = 1,
    };
    int mLastTouchMode = -1;
    int mTranscriptMode;
    int mCacheColorHint;
    int mDirection;
    int mFirstPositionDistanceGuess;
    int mLastPositionDistanceGuess;
    bool mSmoothScrollbarEnabled;
    bool mTextFilterEnabled;
    bool mPopupHidden;
    bool mFiltered;
    bool mIsChildViewEnabled;
    bool mForceTranscriptScroll;
    int mScrollOffset[2] ;
    int mScrollConsumed[2];
    ContextMenuInfo* mContextMenuInfo;
    class FastScroller* mFastScroll;
    std::vector<int>mSelectorState;
    int mLastScrollState;
    int mLastAccessibilityScrollEventFromIndex;
    int mLastAccessibilityScrollEventToIndex;
    ListItemAccessibilityDelegate* mAccessibilityDelegate;
    CheckForLongPress* mPendingCheckForLongPress;
    CheckForTap* mPendingCheckForTap;
    CheckForKeyLongPress* mPendingCheckForKeyLongPress;
    ViewTreeObserver::OnGlobalLayoutListener mGlobalLayoutListener;
    ViewTreeObserver::OnTouchModeChangeListener mTouchModeChangeListener;
    AbsListView::PerformClick* mPerformClick;
    FlingRunnable* mFlingRunnable;
    Runnable mTouchModeReset;
    Runnable mClearScrollingCache;
    Runnable mPostScrollRunner;
    bool mHasPerformedLongPress;
    bool mScrollProfilingStarted = false;
    bool mFlingProfilingStarted =false;
    PopupWindow*mPopup;
    class EditText* mTextFilter;
    OnScrollListener mOnScrollListener;
private:
    void initAbsListView(const AttributeSet&atts);
    void useDefaultSelector();
    std::vector<int>getDrawableStateForSelector();
    void setItemViewLayoutParams(View* child, int position);
    void initOrResetVelocityTracker();
    void initVelocityTrackerIfNotExists();
    void initHapticScrollFeedbackProviderIfNotExists();
    void recycleVelocityTracker();
    bool canScrollUp()const;
    bool canScrollDown()const;
    void drawSelector(Canvas&canvas);
    void updateOnScreenCheckedViews();
    void onTouchUp(MotionEvent& ev);
    void onTouchDown(MotionEvent& ev);
    void onTouchMove(MotionEvent&,MotionEvent&);
    void stopEdgeGlowRecede(float);
    bool shouldAbsorb(EdgeEffect* edgeEffect, int velocity);
    int consumeFlingInStretch(int unconsumed);
    bool shouldDisplayEdgeEffects()const;
    void onTouchCancel();
    void onSecondaryPointerUp(MotionEvent&);
    bool contentFits();
    void positionSelector(int position, View* sel, bool manageHotspot, float x, float y);
    bool startScrollIfNeeded(int x, int y, MotionEvent* vtev);
    void scrollIfNeeded(int x, int y, MotionEvent* vtev);
    void setFastScrollerEnabledUiThread(bool enabled);
    void setFastScrollerAlwaysVisibleUiThread(bool alwaysShow);
    int  releaseGlow(int deltaY, int x);
    bool isGlowActive()const;
    bool doesTouchStopStretch()const;
    void invalidateTopGlow();
    void invalidateBottomGlow();
    void invalidateEdgeEffects();
    void finishGlows();
    void createScrollingCache();
    void clearScrollingCache();
    void dismissPopup();
    void showPopup();
    void positionPopup();
    bool acceptFilter()const;
    void createTextFilter(bool animateEntrance);
    EditText* getTextFilterInput();
    void onTouchModeChanged(bool isInTouchMode);//called by ViewTreeObserver
    void onGlobalLayout();
    static bool isItemClickable(View* view);
    bool showContextMenuInternal(float x, float y, bool useOffsets);
    bool showContextMenuForChildInternal(View* originalView, float x, float y,bool useOffsets);
protected:
    int mChoiceMode;
    int mCheckedItemCount;
    int mTouchSlop;
    float mDensityScale;
    int mSelectedTop;
    int mStackFromBottom;
    bool mScrollingCacheEnabled;
    bool mFastScrollEnabled;
    bool mFastScrollAlwaysVisible;
    std::string mFastScrollStyle;
    bool mGlobalLayoutListenerAddedFilter;
    int mSelectorPosition;
    int mResurrectToPosition;
    int mMinimumVelocity;
    int mMaximumVelocity;
    float mVelocityScale = 1.0f;
    int mOverscrollDistance;
    int mOverflingDistance;
    View * mScrollUp ;
    View * mScrollDown;
    bool mCachingStarted;
    bool mCachingActive;
    Runnable mPositionScrollAfterLayout;
    EdgeEffect* mEdgeGlowTop;
    EdgeEffect* mEdgeGlowBottom;
    AbsPositionScroller* mPositionScroller;
    int mLastHandledItemCount;
    bool mDrawSelectorOnTop;
    bool mAdapterHasStableIds;
    int mSelectionLeftPadding;
    int mSelectionTopPadding ;
    int mSelectionRightPadding;
    int mSelectionBottomPadding;
    int mOverscrollMax;
    bool mIsScrap[2]; 
    bool mIsDetaching;
    Rect mSelectorRect;
    Rect mListPadding;/*The view's padding*/
    int mWidthMeasureSpec;
    float mVerticalScrollFactor;
    int mLayoutMode;
    RecycleBin*mRecycler;
    AdapterDataSetObserver*mDataSetObserver;
    SparseBooleanArray* mCheckStates;
    LongSparseArray<int>* mCheckedIdStates;
    Drawable* mSelector;
    int mMotionPosition;
    int mMotionViewOriginalTop;
    int mMotionViewNewTop;
    int mMotionX,mMotionY;
    int mLastY;
    int mNestedYOffset;
    int mActivePointerId;
    int mMotionCorrection;
    int mTouchMode;
    VelocityTracker* mVelocityTracker;
    HapticScrollFeedbackProvider*mHapticScrollFeedbackProvider;
    class ActionMode*mChoiceActionMode;
    MultiChoiceModeListener mMultiChoiceModeCallback;
    virtual void resetList();
    int computeVerticalScrollExtent()override;
    int computeVerticalScrollOffset()override;
    int computeVerticalScrollRange ()override;
    AbsPositionScroller*createPositionScroller();
    virtual bool trackMotionScroll(int deltaY, int incrementalDeltaY);
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void updateScrollIndicators();
    void setScrollIndicatorViews(View* up, View* down);
    void internalSetPadding(int left, int top, int width, int height)override;
    void onSizeChanged(int w, int h, int oldw, int oldh)override;
    void handleBoundsChange();
    virtual bool touchModeDrawsInPressedState()const;
    virtual bool shouldShowSelector()const;
    void updateSelectorState();
    void drawableStateChanged()override;
    virtual void layoutChildren();
    View*getAccessibilityFocusedChild(View* focusedView);
    bool shouldDrawSelector();
    void dispatchDraw(Canvas& canvas)override;
    void dispatchSetPressed(bool pressed)override;
    void onLayout(bool changed, int l, int t, int w, int h)override;
    void onCancelPendingInputEvents()override;
    void confirmCheckedPositionsById();
    void handleDataChanged()override;
    static int getDistance(const Rect& source,const Rect& dest, int direction);
    virtual View* obtainView(int position, bool*outMetadata);
    void positionSelector(int position, View* sel);
    void hideSelector();
    void setVisibleRangeHint(int start,int end);
    void setEdgeEffectColor(int color);
    void setBottomEdgeEffectColor( int color);
    void setTopEdgeEffectColor(int color);
    int  getTopEdgeEffectColor()const;
    int  getBottomEdgeEffectColor()const;
    int  reconcileSelectedPosition();
    void requestLayoutIfNecessary();
    int getSelectionModeForAccessibility();
 
    bool resurrectSelection(); 
    bool resurrectSelectionIfNeeded();
    void onAttachedToWindow()override;
    void onDetachedFromWindow()override;
    void onWindowFocusChanged(bool)override;
    virtual void setSelectionInt(int position)=0; 
    virtual int getHeightForPosition(int position);
    virtual int getHeaderViewsCount()const;
    virtual int getFooterViewsCount()const;
    virtual int findMotionRow(int y)=0;
    virtual void fillGap(bool down)=0;
    int findClosestMotionRow(int y);
    
    ContextMenuInfo*createContextMenuInfo(View* view, int position, long id);
    ContextMenuInfo*getContextMenuInfo()override;
    void positionSelectorLikeTouch(int position, View* sel, float x, float y);
    void positionSelectorLikeFocus(int position, View* sel);
    void keyPressed();
    void invokeOnItemScrollListener();
    bool performStylusButtonPressAction(MotionEvent& ev);
    bool handleScrollBarDragging(MotionEvent& event)override;
    bool performLongPress(View* child,int longPressPosition,long longPressId);
    bool performLongPress(View* child,int longPressPosition,long longPressId,int x,int y);
    LayoutParams*generateDefaultLayoutParams()const override;
    LayoutParams*generateLayoutParams(const ViewGroup::LayoutParams* p)const override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    void onFocusChanged(bool gainFocus, int direction,Rect* previouslyFocusedRect)override;
    void onOverScrolled(int scrollX, int scrollY, bool clampedX, bool clampedY)override;
    void draw(Canvas&canvas)override;
public:
    AbsListView(int w,int h);
    AbsListView(Context*,const AttributeSet&atts);
    ~AbsListView();
    void setAdapter(Adapter*adapter)override;
    int getCheckedItemCount()const;
    int getCheckedItemPositions(SparseBooleanArray&array);
    bool isItemChecked(int position)const;
    int getCheckedItemIds(std::vector<long>&ids)const;
    //Returns the currently checked item. The result is only valid forCHOICE_MODE_SINGLE
    int getCheckedItemPosition()const;
    void setItemChecked(int position, bool value);
    int getChoiceMode()const;
    void setChoiceMode(int choiceMode);
    void setMultiChoiceModeListener(const MultiChoiceModeListener& listener);
    void setOnScrollListener(const OnScrollListener&);
    bool isScrollingCacheEnabled()const;
    void setScrollingCacheEnabled(bool enabled);

    std::string getAccessibilityClassName()const override;
    void sendAccessibilityEventUnchecked(AccessibilityEvent& event)override;
    void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info)override;
    bool performAccessibilityActionInternal(int action, Bundle* arguments)override;
    View* findViewByAccessibilityIdTraversal(int accessibilityId)override;
    virtual void onInitializeAccessibilityNodeInfoForItem(View* view, int position, AccessibilityNodeInfo& info);

    void reportScrollStateChange(int newState);
    void setFastScrollEnabled(bool);
    bool isFastScrollEnabled()const;
    int  getVerticalScrollbarWidth()const override;
    void setVerticalScrollbarPosition(int position)override;
    void setFastScrollStyle(const std::string& styleid);
    void setFastScrollAlwaysVisible(bool alwaysShow);
    bool isFastScrollAlwaysVisible()const;

    void clearChoices();
    void setDrawSelectorOnTop(bool onTop);
    bool isDrawSelectorOnTop()const;
    Drawable*getSelector();
    void setSelector(Drawable*drawable);
    void setSelector(const std::string&resid);
    void getFocusedRect(Rect& r)override;

    void setScrollBarStyle(int style)override;
    void setSmoothScrollbarEnabled(bool);
    bool isSmoothScrollbarEnabled()const;
    void setStackFromBottom(bool stackFromBottom);
    bool isStackFromBottom()const;

    bool isTextFilterEnabled()const;
    void setTextFilterEnabled(bool);
    void setFilterText(const std::string& filterText);
    void clearTextFilter();
    bool hasTextFilter()const;
    const std::string getTextFilter()const;
    void beforeTextChanged(const std::string& s, int start, int count, int after);//override textwatcher
    void onTextChanged(const std::string& s, int start, int before, int count);//override textwatcher
    void onFilterComplete(int count)override;
    bool verifyDrawable(Drawable* dr)const override;
    void jumpDrawablesToCurrentState()override;
    void dispatchDrawableHotspotChanged(float x, float y)override;
    int pointToPosition(int x, int y);
    long pointToRowId(int x, int y);
    bool performItemClick(View& view, int position, long id)override;
    bool showContextMenu()override;
    bool showContextMenu(float x, float y)override;
    bool showContextMenuForChild(View* originalView)override;
    bool showContextMenuForChild(View* originalView, float x, float y)override;
    bool onKeyDown(int keyCode, KeyEvent& event)override;
    bool onKeyUp(int keyCode, KeyEvent& event)override;
    bool onInterceptTouchEvent(MotionEvent& ev)override;
    bool onTouchEvent(MotionEvent& ev)override;
    bool onInterceptHoverEvent(MotionEvent& event)override;
    PointerIcon* onResolvePointerIcon(MotionEvent& event, int pointerIndex)override;
    void onRtlPropertiesChanged(int layoutDirection)override;
    bool onGenericMotionEvent(MotionEvent& event)override;
    void fling(int velocity);
    bool onStartNestedScroll(View* child, View* target, int nestedScrollAxes)override;
    void onNestedScrollAccepted(View* child, View* target, int axes)override;
    void onNestedScroll(View* target, int dxConsumed, int dyConsumed,int dxUnconsumed, int dyUnconsumed)override;
    bool onNestedFling(View* target, float velocityX, float velocityY, bool consumed)override;
    void requestDisallowInterceptTouchEvent(bool disallowIntercept)override;
    void addTouchables(std::vector<View*>& views)override;
    View* getSelectedView()override;
    LayoutParams*generateLayoutParams(const AttributeSet& attrs)const override;
    void setSelectionFromTop(int position, int y);
    int getCacheColorHint()const;
    virtual void setCacheColorHint(int color);
    int getListPaddingTop()const;
    int getListPaddingBottom()const;
    int getListPaddingLeft()const;
    int getListPaddingRight()const;
    void setTranscriptMode(int);
    int getTranscriptMode()const;

    bool isSelectedChildViewEnabled()const;
    void setSelectedChildViewEnabled(bool selectedChildViewEnabled);

    void setFriction(float friction);
    void setVelocityScale(float scale);
    void smoothScrollToPosition(int position);
    void smoothScrollToPositionFromTop(int position, int offset, int duration);
    void smoothScrollToPositionFromTop(int position, int offset);
    void smoothScrollToPosition(int position, int boundPosition);
    void smoothScrollBy(int distance, int duration);
    void smoothScrollBy(int distance, int duration, bool linear,bool suppressEndFlingStateChangeCall);
    void smoothScrollByOffset(int position);
    void scrollListBy(int y);
    bool canScrollList(int direction);
    void reclaimViews(std::vector<View*>& views);
};

class AbsListView::AbsRunnable{
protected:
    AbsListView*mLV;
    Runnable mRunnable;
public:
    AbsRunnable(AbsListView*);
    virtual void run()=0;
    void postDelayed(long);
    void removeCallbacks();
};

class AbsListView::CheckForTap:public AbsRunnable{
public:
    CheckForTap(AbsListView*);
    float x,y;
    void run()override;
};

class AbsListView::WindowRunnable:public AbsRunnable{
protected:
    int mOriginalAttachCount;
public:
    WindowRunnable(AbsListView*);
    void rememberWindowAttachCount();
    bool sameWindow();
};

class AbsListView::PerformClick:public WindowRunnable{
public:
    PerformClick(AbsListView*lv);
    int mClickMotionPosition;
    void run()override;
};

class AbsListView::CheckForLongPress:public WindowRunnable{
private:
    friend class AbsListView;
    static constexpr int INVALID_COORD=-1;
    float mX ,mY;
public:
    CheckForLongPress(AbsListView*);
    void setCoords(float x, float y);
    void run()override;
};

class AbsListView::CheckForKeyLongPress:public WindowRunnable{
public:
    CheckForKeyLongPress(AbsListView*lv);
    void run()override;
};

class AbsListView::FlingRunnable:public AbsRunnable{
public:
    static constexpr int FLYWHEEL_TIMEOUT = 40;
    OverScroller* mScroller;
    int mLastFlingY;
    bool mSuppressIdleStateChangeCall;
    Runnable mCheckFlywheel;
    void checkFlyWheel();
public:
    FlingRunnable(AbsListView*lv);
    ~FlingRunnable();
    float getSplineFlingDistance(int velocity)const;
    void start(int initialVelocity);
    void startSpringback();
    void startOverfling(int initialVelocity);
    void edgeReached(int delta);
    void startScroll(int distance, int duration, bool linear,
            bool suppressEndFlingStateChangeCall);
    void endFling();
    void flywheelTouch();
    void run()override;
    void postOnAnimation();
};

}//namespace
#endif
