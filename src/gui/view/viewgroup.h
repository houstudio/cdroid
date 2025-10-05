/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __CDROID_GROUPVIEW_H__
#define __CDROID_GROUPVIEW_H__

#include <view/view.h>
#include <view/viewgroupoverlay.h>
#include <animation/animations.h>
#include <widget/scroller.h>

namespace cdroid {

#define ATTR_ANIMATE_FOCUS (0x2000) /*flag to open animate focus*/

class ViewGroup : public View {
private:
    static constexpr int FLAG_CLIP_CHILDREN   = 0x01;
    static constexpr int FLAG_CLIP_TO_PADDING = 0x02;
    static constexpr int FLAG_INVALIDATE_REQUIRED= 0x4;
    static constexpr int FLAG_RUN_ANIMATION   = 0x8;
    static constexpr int FLAG_ANIMATION_DONE  = 0x10;
    static constexpr int FLAG_PADDING_NOT_NULL= 0x20;

    static constexpr int FLAG_ANIMATION_CACHE = 0x40;
    static constexpr int FLAG_OPTIMIZE_INVALIDATE  = 0x80;
    static constexpr int FLAG_CLEAR_TRANSFORMATION = 0x100;
    static constexpr int FLAG_NOTIFY_ANIMATION_LISTENER = 0x200;
    static constexpr int FLAG_USE_CHILD_DRAWING_ORDER   = 0x400;
    static constexpr int FLAG_SUPPORT_STATIC_TRANSFORMATIONS = 0x800;

    static constexpr int FLAG_ADD_STATES_FROM_CHILDREN  = 0x2000;
    static constexpr int FLAG_ALWAYS_DRAWN_WITH_CACHE   = 0x4000;
    static constexpr int FLAG_CHILDREN_DRAWN_WITH_CACHE = 0x8000;
    static constexpr int FLAG_NOTIFY_CHILDREN_ON_DRAWABLE_STATE_CHANGE = 0x10000;

    static constexpr int FLAG_MASK_FOCUSABILITY  = 0x60000;
    static constexpr int FLAG_SPLIT_MOTION_EVENTS= 0x200000;
    static constexpr int FLAG_PREVENT_DISPATCH_ATTACHED_TO_WINDOW =0x400000;
    static constexpr int FLAG_LAYOUT_MODE_WAS_EXPLICITLY_SET      =0x800000;
    static constexpr int FLAG_IS_TRANSITION_GROUP    = 0x1000000;
    static constexpr int FLAG_IS_TRANSITION_GROUP_SET= 0x2000000;
    static constexpr int FLAG_TOUCHSCREEN_BLOCKS_FOCUS= 0x4000000;
    static constexpr int FLAG_START_ACTION_MODE_FOR_CHILD_IS_TYPED = 0x8000000;
    static constexpr int FLAG_START_ACTION_MODE_FOR_CHILD_IS_NOT_TYPED = 0x10000000;
    static constexpr int FLAG_SHOW_CONTEXT_MENU_WITH_COORDS = 0x20000000;

    static constexpr int LAYOUT_MODE_UNDEFINED   = -1;
protected:
    static constexpr int CLIP_TO_PADDING_MASK = FLAG_CLIP_TO_PADDING | FLAG_PADDING_NOT_NULL;
    static constexpr int FLAG_DISALLOW_INTERCEPT = 0x80000;
    class ViewLocationHolder;
    class ChildListForAccessibility;
public:
    static constexpr int FOCUS_BEFORE_DESCENDANTS= 0x20000;
    static constexpr int FOCUS_AFTER_DESCENDANTS = 0x40000;
    static constexpr int FOCUS_BLOCK_DESCENDANTS = 0x60000;
    
    static constexpr int PERSISTENT_NO_CACHE = 0x0;
    /*Used to indicate that the animation drawing cache should be kept in memory.*/
    static constexpr int PERSISTENT_ANIMATION_CACHE = 0x1;
    /*Used to indicate that the scrolling drawing cache should be kept in memory.*/
    static constexpr int PERSISTENT_SCROLLING_CACHE = 0x2;
    static constexpr int PERSISTENT_ALL_CACHES      = 0x03;
    static constexpr int LAYOUT_MODE_CLIP_BOUNDS = 0;
    static constexpr int LAYOUT_MODE_OPTICAL_BOUNDS = 1;
    static constexpr int LAYOUT_MODE_DEFAULT = LAYOUT_MODE_CLIP_BOUNDS;
public:
    typedef cdroid::LayoutParams LayoutParams;
    typedef cdroid::MarginLayoutParams MarginLayoutParams;
    typedef struct{
        std::function<void(View&/*parent*/,View* /*child*/)>onChildViewAdded;
        std::function<void(View&/*parent*/,View* /*child*/)>onChildViewRemoved;
    }OnHierarchyChangeListener;
    DECLARE_UIEVENT(void,OnAnimationFinished);
private:
    friend class View;
    friend class UIEventSource;
    int mLayoutMode;
    int mNestedScrollAxes;
    int mLastTouchDownX,mLastTouchDownY;
    int mLastTouchDownIndex=-1;
    int64_t mLastTouchDownTime;
    View* mFocused;
    View* mDefaultFocus;
    View* mFocusedInCluster;
    class LayoutTransition*mTransition;
    std::vector<View*>mTransitioningViews;
    std::vector<View*>mVisibilityChangingChildren;
    std::vector<View*>mTransientViews;
    std::vector<int>mTransientIndices;
    int mChildCountWithTransientState;
    int mChildUnhandledKeyListeners;
    bool mLayoutCalledWhileSuppressed;
    bool mIsInterestedInDrag;
    bool mHoveredSelf;
    bool mTooltipHoveredSelf;
    bool mPointerCapture;
    DragEvent*mCurrentDragStartEvent;
    Animation::AnimationListener mAnimationListener;
    LayoutTransition::TransitionListener mLayoutTransitionListener;
    class LayoutAnimationController* mLayoutAnimationController;
    class TouchTarget* mFirstTouchTarget;
    class HoverTarget* mFirstHoverTarget;
    View* mTooltipHoverTarget;
    Point animateTo;//save window boundray  while animating
    Point animateFrom;//window animate from boundary
    Transformation* mChildTransformation;
    void initGroup();
    void initFromAttributes(Context*,const AttributeSet&);
    void setBooleanFlag(int flag, bool value);
    bool hasBooleanFlag(int flag)const;
    bool hasChildWithZ()const;
    static void drawRect(Canvas& canvas,int x1, int y1, int x2, int y2);
    static void fillRect(Canvas& canvas,int x1, int y1, int x2, int y2);
    static void drawCorner(Canvas& c,int x1, int y1, int dx, int dy, int lw);
    static void drawRectCorners(Canvas& canvas, int x1, int y1, int x2, int y2,int lineLength, int lineWidth);

    int getAndVerifyPreorderedIndex(int childrenCount, int i, bool customOrder);
    static View*getAndVerifyPreorderedView(const std::vector<View*>&,const std::vector<View*>&, int childIndex);
    TouchTarget* getTouchTarget(View* child);
    TouchTarget* addTouchTarget(View* child, int pointerIdBits);
    View*findChildWithAccessibilityFocus();
    void resetTouchState();
    static bool resetCancelNextUpFlag(View* view);
    void clearTouchTargets();

    static bool canViewReceivePointerEvents(View& child);
    static MotionEvent* obtainMotionEventNoHistoryOrSelf(MotionEvent* event);
    void cancelAndClearTouchTargets(MotionEvent*);
    void removePointersFromTouchTargets(int pointerIdBits);
    void cancelTouchTarget(View* view);
    void exitTooltipHoverTargets();
    void exitHoverTargets();
    void cancelHoverTarget(View*view);
    bool dispatchTransformedTouchEvent(MotionEvent& event, bool cancel,
            View* child, int desiredPointerIdBits);
    bool dispatchTransformedGenericPointerEvent(MotionEvent& event, View* child);
    bool dispatchTooltipHoverEvent(MotionEvent& event, View* child);
    void setTouchscreenBlocksFocusNoRefocus(bool touchscreenBlocksFocus);
    void handlePointerCaptureChanged(bool hasCapture);

    void touchAccessibilityNodeProviderIfNeeded(View* child);
    void addInArray(View* child, int index);
    bool removeViewInternal(View* view);
    void removeViewInternal(int index, View* view);
    void removeViewsInternal(int start, int count);
    void removeFromArray(int index);
    void removeFromArray(int start, int count);

    void bindLayoutAnimation(View* child);
    void notifyAnimationListener();
    void addViewInner(View* child, int index,LayoutParams* params,bool preventRequestLayout);
    void addDisappearingView(View* v);
    bool updateLocalSystemUiVisibility(int localValue, int localChanges)override;
    PointerIcon* dispatchResolvePointerIcon(MotionEvent& event, int pointerIndex,View* child);

protected:
    int mGroupFlags;
    int mPersistentDrawingCache;
    std::vector<View*> mChildren;
    std::vector<View*> mDisappearingChildren;
    std::vector<View*> mChildrenInterestedInDrag;
    View* mAccessibilityFocusedHost;
    AccessibilityNodeInfo* mAccessibilityFocusedVirtualView;
    Cairo::RefPtr<Cairo::Region>mInvalidRgn;
    Transformation*mInvalidationTransformation;
    int64_t time_lastframe;
    OnHierarchyChangeListener mOnHierarchyChangeListener;
    bool getChildVisibleRect(View*child,Rect&r,Point*offset,bool forceParentCheck);
    virtual bool canAnimate()const;
    void setDefaultFocus(View* child);
    View*getDeepestFocusedChild();
    void clearDefaultFocus(View* child);
    bool hasDefaultFocus()const override;
    bool hasFocusable(bool allowAutoFocus, bool dispatchExplicit)const override;
    bool hasFocusableChild(bool dispatchExplicit)const;
    MotionEvent* getTransformedMotionEvent(MotionEvent& event, View* child)const;
    bool notifyChildOfDragStart(View* child);
    void dispatchAttachedToWindow(AttachInfo* info, int visibility)override;
    void dispatchScreenStateChanged(int screenState)override;
    void dispatchMovedToDisplay(Display& display, Configuration& config)override;
    bool dispatchPopulateAccessibilityEventInternal(AccessibilityEvent& event)override;
    void dispatchWindowVisibilityChanged(int visibility)override;
    bool dispatchVisibilityAggregated(bool isVisible)override;
    void dispatchConfigurationChanged(Configuration& newConfig)override;
    void dispatchWindowSystemUiVisiblityChanged(int visible)override;
    void dispatchSystemUiVisibilityChanged(int visibility)override;

    void resetSubtreeAccessibilityStateChanged()override;
    int getNumChildrenForAccessibility() const;
    void dispatchDetachedFromWindow()override;
    void dispatchCancelPendingInputEvents()override;
    void internalSetPadding(int left, int top, int width, int height)override;

    void dispatchSaveInstanceState(SparseArray<Parcelable*>& container)override;
    void dispatchFreezeSelfOnly(SparseArray<Parcelable*>& container);
    void dispatchRestoreInstanceState(SparseArray<Parcelable*>& container)override;
    void dispatchThawSelfOnly(SparseArray<Parcelable*>& container);

    bool dispatchGenericPointerEvent(MotionEvent&event)override;
    bool dispatchGenericFocusedEvent(MotionEvent&event)override;
    bool dispatchTooltipHoverEvent(MotionEvent& event)override;
    virtual bool onRequestFocusInDescendants(int direction,Rect* previouslyFocusedRect);
    virtual bool requestChildRectangleOnScreen(View* child,Rect& rectangle, bool immediate);
    virtual bool requestSendAccessibilityEvent(View* child, AccessibilityEvent& event);
    virtual bool onRequestSendAccessibilityEvent(View* child, AccessibilityEvent& event);
    virtual bool onRequestSendAccessibilityEventInternal(View* child, AccessibilityEvent& event);
    virtual void notifySubtreeAccessibilityStateChanged(View* child, View* source, int changeType);
    void notifySubtreeAccessibilityStateChangedIfNeeded()override;
    bool performKeyboardGroupNavigation(int direction);

    bool isAlwaysDrawnWithCacheEnabled()const;
    void setAlwaysDrawnWithCacheEnabled(bool always);
    bool isChildrenDrawnWithCacheEnabled()const;
    void setChildrenDrawnWithCacheEnabled(bool enabled);
    bool isChildrenDrawingOrderEnabled()const;
    void setChildrenDrawingOrderEnabled(bool enabled);
    void setLayoutMode(int layoutMode,bool explicity);

    void attachViewToParent(View* child, int index, LayoutParams* params);
    void dispatchViewAdded(View* child);
    void dispatchViewRemoved(View* child);
    virtual void removeDetachedView(View* child, bool animate);
    void detachViewFromParent(int index);
    void detachViewsFromParent(int start, int count);
    void detachViewFromParent(View* child);
    void detachAllViewsFromParent();
    void clearFocusedInCluster(View* child);
    void clearFocusedInCluster();
    void invalidateInheritedLayoutMode(int layoutModeOfRoot)override;

    virtual LayoutParams* generateLayoutParams(const LayoutParams* p)const;
    virtual LayoutParams* generateDefaultLayoutParams()const;
    virtual bool checkLayoutParams(const LayoutParams* p)const;

    virtual void onSetLayoutParams(View* child,const LayoutParams* layoutParams);
    bool hasUnhandledKeyListener()const override;
    void incrementChildUnhandledKeyListeners();
    void decrementChildUnhandledKeyListeners();
    View* dispatchUnhandledKeyEvent(KeyEvent& evt)override;
    
    virtual void measureChildren(int widthMeasureSpec, int heightMeasureSpec);
    virtual void measureChild(View* child, int parentWidthMeasureSpec,int parentHeightMeasureSpec);

    virtual void measureChildWithMargins(View* child,int parentWidthMeasureSpec, int widthUsed,
            int parentHeightMeasureSpec, int heightUsed);
    virtual bool drawChild(Canvas& canvas, View* child, int64_t drawingTime);

    virtual void onDebugDrawMargins(Canvas& canvas);
    virtual void onDebugDraw(Canvas& canvas);
    void drawInvalidateRegion(Canvas&canvas);
    void dispatchDraw(Canvas&)override;

    bool hasActiveAnimations();
    void transformPointToViewLocal(float pint[2],View&);
    bool isTransformedTouchPointInView(float x,float y,View& child,Point*outLocalPoint);
    void drawableStateChanged()override;
    std::vector<int> onCreateDrawableState(int)override;
    void dispatchSetPressed(bool pressed)override;
    void dispatchDrawableHotspotChanged(float x,float y)override;
    bool hasHoveredChild()const override;
    void addChildrenForAccessibility(std::vector<View*>& outChildren)override;
    bool pointInHoveredChild(MotionEvent& event)override;
    virtual int getChildDrawingOrder(int childCount, int i);
    std::vector<View*> buildOrderedChildList();

    void setStaticTransformationsEnabled(bool enabled);
    virtual bool getChildStaticTransformation(View* child, Transformation* t);
    Transformation* getChildTransformation();
    void finishAnimatingView(View* view, Animation* animation);
    bool isViewTransitioning(View* view);
    void attachLayoutAnimationParameters(View* child,LayoutParams* params, int index, int count);
    virtual void onChildVisibilityChanged(View* child, int oldVisibility, int newVisibility);
    void dispatchVisibilityChanged(View& changedView, int visibility)override;
    void resetResolvedDrawables()override;
    void resolveDrawables()override;

    void setAccessibilityFocus(View* view, AccessibilityNodeInfo* node);
public:
    ViewGroup(int w,int h);
    ViewGroup(int x,int y,int w,int h);
    ViewGroup(Context*ctx,const AttributeSet& attrs);
    virtual ~ViewGroup();
    virtual bool ensureTouchMode(bool);
    bool getTouchscreenBlocksFocus()const;
    bool shouldBlockFocusForTouchscreen()const;
    int getDescendantFocusability()const;
    void setDescendantFocusability(int);
    int  getLayoutMode();
    void setLayoutMode(int layoutMode);
    ViewGroupOverlay*getOverlay()override;
    int getChildDrawingOrder(int drawingPosition);
    virtual void recomputeViewAttributes(View* child);
    virtual void bringChildToFront(View*);
    bool getClipChildren()const;
    void setClipChildren(bool clipChildren);
    virtual bool getClipToPadding()const;
    virtual void setClipToPadding(bool clipToPadding);
    void dispatchStartTemporaryDetach() override;
    void dispatchFinishTemporaryDetach()override;
    void dispatchSetSelected(bool selected)override;
    void dispatchSetActivated(bool activated)override;
    virtual std::vector<View*> buildTouchDispatchChildList();
    bool dispatchActivityResult(const std::string& who, int requestCode, int resultCode, Intent data)override;
    virtual View*focusSearch(View*focused,int direction);
    View*getFocusedChild();
    bool hasFocus()const override;
    View*findFocus()override;
    bool restoreDefaultFocus()override;
    virtual void requestChildFocus(View*child,View*focused);
    void unFocus(View* focused)override;
    void clearChildFocus(View* child);
    void focusableViewAvailable(View*);
    bool isShowingContextMenuWithCoords()const;
    virtual bool showContextMenuForChild(View* originalView);
    virtual bool showContextMenuForChild(View* originalView, float x, float y);
    bool hasTransientState()override;
    void childHasTransientStateChanged(View* child, bool childHasTransientState);

    void offsetDescendantRectToMyCoords(const View* descendant, Rect& rect)const;
    void offsetRectIntoDescendantCoords(const View* descendant, Rect& rect)const;
    void offsetRectBetweenParentAndChild(const View* descendant, Rect& rect,bool offsetFromChildToParent, bool clipToBounds)const;
    void offsetChildrenTopAndBottom(int offset);
    virtual bool getChildVisibleRect(View*child,Rect&r,Point*offset);
    bool gatherTransparentRegion(const Cairo::RefPtr<Cairo::Region>&region)override;

    void addFocusables(std::vector<View*>& views, int direction, int focusableMode)override;
    void addKeyboardNavigationClusters(std::vector<View*>&views,int drection)override;
    void setTouchscreenBlocksFocus(bool touchscreenBlocksFocus);
    virtual void setOnHierarchyChangeListener(const OnHierarchyChangeListener& listener);
    bool restoreFocusNotInCluster()override;
    View*keyboardNavigationClusterSearch(View* currentCluster,int direction)override;
    bool requestFocus(int direction=FOCUS_DOWN,Rect*previouslyFocusedRect=nullptr)override;
    virtual bool requestLayoutDuringLayout(View* view);

    int getChildCount()const;
    View*getChildAt(int idx)const;
    int indexOfChild(View* child)const;
    void setChildrenDrawingCacheEnabled(bool);
    virtual bool onNestedPrePerformAccessibilityAction(View* target, int action, Bundle* args);
    bool isLayoutModeOptical()const;
    void cleanupLayoutState(View* child)const;

    virtual void addView(View* view);
    virtual void addView(View* child, int index);
    virtual void addView(View* child, LayoutParams* params);
    virtual void addView(View* child, int index, LayoutParams* params);
    void addView(View* child, int width, int height);
    bool addViewInLayout(View* child, int index,LayoutParams* params);
    bool addViewInLayout(View* child, int index,LayoutParams* params,bool preventRequestLayout);
    void addTransientView(View*view,int index);
    void removeTransientView(View*);
    int getTransientViewCount() const;
    int getTransientViewIndex(int position)const;
    View*getTransientView(int position) const;

    PointerIcon* onResolvePointerIcon(MotionEvent&, int)override;

    void startLayoutAnimation(); 
    void scheduleLayoutAnimation();
    void setLayoutAnimation(LayoutAnimationController*);
    LayoutAnimationController* getLayoutAnimation();
    void setLayoutAnimationListener(const Animation::AnimationListener& animationListener);
    virtual void requestTransitionStart(LayoutTransition* transition);
    bool resolveRtlPropertiesIfNeeded()override;
    bool resolveLayoutDirection()override;
    bool resolveTextDirection()override;
    bool resolveTextAlignment()override;
    void resolvePadding()override;
    void resolveLayoutParams()override;
    void resetResolvedLayoutDirection()override;
    void resetResolvedTextDirection()override;
    void resetResolvedTextAlignment()override;

    Animation::AnimationListener getLayoutAnimationListener();
    void setLayoutTransition(LayoutTransition*);
    LayoutTransition*getLayoutTransition()const; 
    void clearDisappearingChildren();
    void startViewTransition(View* view);
    void endViewTransition(View* view);
  
    virtual void onViewAdded(View* child);
    virtual void onViewRemoved(View* child);
    virtual ViewGroup*invalidateChildInParent(int* location,Rect& dirty);
    void invalidateChild(View*child,Rect&dirty);

    virtual LayoutParams* generateLayoutParams(const AttributeSet& attrs)const;
    static int getChildMeasureSpec(int spec, int padding, int childDimension);
    static bool isViewDescendantOf(View* child, View* parent);
    virtual void removeView(View* view);/*only remove view from children,no deleteion*/
    virtual void removeViewAt(int idx);
    virtual void removeAllViews();
    virtual void removeViews(int start, int count);
    virtual void removeViewInLayout(View* view);
    virtual void removeViewsInLayout(int start,int count);
    void removeAllViewsInLayout();
    virtual View* findViewById(int id)override;
    View* findViewByPredicateTraversal(const Predicate<View*>&predicate,View* childToSkip)override;
    View* findViewWithTagTraversal(void*tag)override;
    View* findViewByAccessibilityIdTraversal(int accessibilityId)override;
    std::string getAccessibilityClassName()const override;
    virtual void resetResolvedPadding()override;
    virtual bool shouldDelayChildPressedState();
    bool hasPointerCapture()const override;

    virtual bool onInterceptHoverEvent(MotionEvent& event);
    virtual bool onStartNestedScroll(View* child, View* target, int nestedScrollAxes);
    virtual bool onStartNestedScroll(View* child, View* target, int nestedScrollAxes,int type);
    virtual void onNestedScrollAccepted(View* child, View* target, int axes);
    virtual void onNestedScrollAccepted(View* child, View* target, int axes,int type);
    virtual void onStopNestedScroll(View* child);
    virtual void onStopNestedScroll(View* child,int type);
    virtual void onNestedScroll(View* target, int dxConsumed, int dyConsumed,int dxUnconsumed, int dyUnconsumed);
    virtual void onNestedScroll(View* target, int dxConsumed, int dyConsumed,int dxUnconsumed, int dyUnconsumed, int type);
    virtual void onNestedScroll(View* target, int dxConsumed, int dyConsumed,int dxUnconsumed, int dyUnconsumed, int type,int*consumed);
    virtual void onNestedPreScroll(View* target, int dx, int dy, int*consumed);
    virtual void onNestedPreScroll(View* target, int dx, int dy, int*consumed,int type);
    virtual bool onNestedFling(View* target, float velocityX, float velocityY, bool consumed);
    virtual int getNestedScrollAxes();
    virtual bool onNestedPreFling(View* target, float velocityX, float velocityY);

    void setMotionEventSplittingEnabled(bool split);
    bool isMotionEventSplittingEnabled()const; 
    bool dispatchKeyEvent(KeyEvent&)override;
    bool dispatchKeyShortcutEvent(KeyEvent&)override;
    bool dispatchTrackballEvent(MotionEvent& event)override;
    bool dispatchCapturedPointerEvent(MotionEvent& event)override;
    void dispatchPointerCaptureChanged(bool hasCapture)override;
    bool dispatchUnhandledMove(View* focused, int direction)override;
    bool dispatchTouchEvent(MotionEvent& event)override;
    bool dispatchHoverEvent(MotionEvent&event)override;
    void dispatchWindowFocusChanged(bool hasFocus)override;
    void addTouchables(std::vector<View*>&)override;
    void dispatchDisplayHint(int hint)override;
    bool isTransitionGroup();
    void setTransitionGroup(bool isTransitionGroup);
    virtual void requestDisallowInterceptTouchEvent(bool disallowIntercept);
    bool onInterceptTouchEvent(MotionEvent& evt)override;
    virtual void onDescendantInvalidated(View* child,View* target);

    void jumpDrawablesToCurrentState()override;
    void setAddStatesFromChildren(bool addsStates);
    bool addStatesFromChildren();
    virtual void childDrawableStateChanged(View* child);

    virtual void dispatchInvalidateOnAnimation(View* view);
    virtual void dispatchInvalidateRectOnAnimation(View*,const Rect&);
    virtual void dispatchInvalidateDelayed(View*,long delay);
    virtual void dispatchInvalidateRectDelayed(const AttachInfo::InvalidateInfo*,long delay);
    virtual void cancelInvalidate(View* view);
    //ViewRootImpl
    void requestPointerCapture(bool);
    View* getAccessibilityFocusedHost()const;
    AccessibilityNodeInfo*getAccessibilityFocusedVirtualView()const;
};


class ViewGroup::ViewLocationHolder{
public:
    static constexpr int COMPARISON_STRATEGY_STRIPE = 1;
    static constexpr int COMPARISON_STRATEGY_LOCATION = 2;
private:
    static constexpr int MAX_POOL_SIZE = 32;
    static int sComparisonStrategy;
    static Pools::SimplePool<ViewLocationHolder> sPool;

    Rect mLocation;
    ViewGroup* mRoot;
    int mLayoutDirection;
public:
	View* mView;
    static ViewLocationHolder* obtain(ViewGroup* root, View* view);
    static void setComparisonStrategy(int strategy);
    void recycle();
    int compareTo(ViewLocationHolder* another);
private:
    static int compareBoundsOfTree(ViewLocationHolder* holder1, ViewLocationHolder* holder2);
    void init(ViewGroup* root, View* view);
    void clear();
};

class ViewGroup::ChildListForAccessibility {
private:
    static constexpr int MAX_POOL_SIZE = 32;
    static Pools::SimplePool<ChildListForAccessibility> sPool;;

    std::vector<View*> mChildren;;
    std::vector<ViewLocationHolder*> mHolders;
public:
    static ChildListForAccessibility* obtain(ViewGroup* parent, bool sort);
    void recycle();
    int getChildCount() ;
    View* getChildAt(int index);
private:
    void init(ViewGroup* parent, bool sort);
    void sort(std::vector<ViewLocationHolder*>& holders);
    void clear();
};
}/*namespace cdroid*/

#endif  // __CDROID_GROUPVIEW_H__
