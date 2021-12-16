/*
 * Copyright (C) 2015 UI project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __NGLUI_GROUPVIEW_H__
#define __NGLUI_GROUPVIEW_H__

#include <widget/view.h>
#include <core/scroller.h>
#include <animations.h>

namespace cdroid {

#define ATTR_ANIMATE_FOCUS (0x2000) /*flag to open animate focus*/

class ViewGroup : public View {
public:
    typedef cdroid::LayoutParams LayoutParams;
    typedef cdroid::MarginLayoutParams MarginLayoutParams;
    DECLARE_UIEVENT(void,OnAnimationFinished);
    DECLARE_UIEVENT(void,OnHierarchyChangeListener,ViewGroup&,View*,bool addremove);
    enum{
        FLAG_CLIP_CHILDREN   = 0x01,
        FLAG_CLIP_TO_PADDING = 0x02,
        FLAG_INVALIDATE_REQUIRED= 0x4,
        FLAG_RUN_ANIMATION   = 0x8,
        FLAG_ANIMATION_DONE  = 0x10,
        FLAG_PADDING_NOT_NULL= 0x20,
        CLIP_TO_PADDING_MASK = FLAG_CLIP_TO_PADDING | FLAG_PADDING_NOT_NULL,

        FLAG_ANIMATION_CACHE = 0x40,
        FLAG_OPTIMIZE_INVALIDATE  = 0x80,
        FLAG_CLEAR_TRANSFORMATION = 0x100,
        FLAG_NOTIFY_ANIMATION_LISTENER = 0x200,
        FLAG_USE_CHILD_DRAWING_ORDER   = 0x400,
        FLAG_SUPPORT_STATIC_TRANSFORMATIONS = 0x800,

        FLAG_ADD_STATES_FROM_CHILDREN  = 0x2000,
        FLAG_ALWAYS_DRAWN_WITH_CACHE   = 0x4000,
        FLAG_CHILDREN_DRAWN_WITH_CACHE = 0x8000,
        FLAG_NOTIFY_CHILDREN_ON_DRAWABLE_STATE_CHANGE = 0x10000,

        FLAG_TOUCHSCREEN_BLOCKS_FOCUS = 0x4000000,
        FOCUS_BEFORE_DESCENDANTS= 0x20000,
        FOCUS_AFTER_DESCENDANTS = 0x40000,
        FOCUS_BLOCK_DESCENDANTS = 0x60000,
        FLAG_MASK_FOCUSABILITY  = 0x60000,
        FLAG_DISALLOW_INTERCEPT = 0x80000,
        FLAG_SPLIT_MOTION_EVENTS= 0x200000,
        FLAG_PREVENT_DISPATCH_ATTACHED_TO_WINDOW =0x400000,
        FLAGS_IS_TRANSITION_GROUP    = 0x1000000,
        FLAGS_IS_TRANSITION_GROUP_SET= 0x2000000,
    };
    enum{
        LAYOUT_MODE_UNDEFINED  =-1,
        LAYOUT_MODE_CLIP_BOUNDS=0,
        LAYOUT_MODE_OPTICAL_BOUNDS=1,
        LAYOUT_MODE_DEFAULT = LAYOUT_MODE_CLIP_BOUNDS
    };
    static bool DEBUG_DRAW;
private:
    friend class View;
    int mLayoutMode;
    int mNestedScrollAxes;
    int mLastTouchDownX,mLastTouchDownY;
    int mLastTouchDownIndex=-1;
    long mLastTouchDownTime;
    View* mFocused;
    View* mDefaultFocus;
    View* mFocusedInCluster;
    class LayoutTransition*mTransition;
    std::vector<View*>mTransitioningViews;
    std::vector<View*>mVisibilityChangingChildren;
    std::vector<View*>mTransientViews;
    std::vector<int>mTransientIndices;
    int mChildCountWithTransientState;
    bool mLayoutCalledWhileSuppressed;
    Animation::AnimationListener mAnimationListener;
    LayoutTransition::TransitionListener mLayoutTransitionListener;
    class LayoutAnimationController* mLayoutAnimationController;
    class TouchTarget* mFirstTouchTarget;
    POINT animateTo;//save window boundray  while animating
    POINT animateFrom;//window animate from boundary
    Transformation* mChildTransformation;
    void initGroup();
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
    void resetTouchState();
    static bool resetCancelNextUpFlag(View* view);
    void clearTouchTargets();

    static bool canViewReceivePointerEvents(View& child);
    void cancelAndClearTouchTargets(MotionEvent*);
    void removePointersFromTouchTargets(int pointerIdBits);
    void cancelTouchTarget(View* view);
    void cancelHoverTarget(View*view);

    bool dispatchTransformedTouchEvent(MotionEvent& event, bool cancel,
            View* child, int desiredPointerIdBits);
    bool dispatchTransformedGenericPointerEvent(MotionEvent& event, View* child);

    void setTouchscreenBlocksFocusNoRefocus(bool touchscreenBlocksFocus);

    void addInArray(View* child, int index);
    bool removeViewInternal(View* view);
    void removeViewInternal(int index, View* view);
    void removeViewsInternal(int start, int count);
    void removeFromArray(int index);
    void removeFromArray(int start, int count);

    void bindLayoutAnimation(View* child);
    void notifyAnimationListener();
    View&addViewInner(View* child, int index,LayoutParams* params,bool preventRequestLayout);
    void addDisappearingView(View* v);
protected:
    int mGroupFlags;
    std::vector<View*> mChildren;
    std::vector<View*>mDisappearingChildren;
    RefPtr<Region>mInvalidRgn;
    Transformation*mInvalidationTransformation;
    LONGLONG time_lastframe;
    OnHierarchyChangeListener mOnHierarchyChangeListener;
    virtual bool canAnimate()const;
    void setDefaultFocus(View* child);
    void clearDefaultFocus(View* child);
    bool hasFocusable(bool allowAutoFocus, bool dispatchExplicit)const override;
    bool hasFocusableChild(bool dispatchExplicit)const;
    void dispatchAttachedToWindow(AttachInfo* info, int visibility)override;
    bool dispatchVisibilityAggregated(bool isVisible);
    void dispatchDetachedFromWindow()override;
    bool dispatchGenericFocusedEvent(MotionEvent&event)override;
    virtual bool onRequestFocusInDescendants(int direction,Rect* previouslyFocusedRect);
    virtual bool requestChildRectangleOnScreen(View* child,Rect& rectangle, bool immediate);
    bool performKeyboardGroupNavigation(int direction);

    bool isChildrenDrawingOrderEnabled()const;
    void setChildrenDrawingOrderEnabled(bool enabled);

    void attachViewToParent(View* child, int index, LayoutParams* params);
    void dispatchViewAdded(View* child);
    void dispatchViewRemoved(View* child);
    void removeDetachedView(View* child, bool animate);
    void detachViewsFromParent(int start, int count);
    void detachViewFromParent(View* child);
    void detachAllViewsFromParent();
    void clearFocusedInCluster(View* child);
    void clearFocusedInCluster();

    virtual LayoutParams* generateLayoutParams(const LayoutParams* p)const;
    virtual LayoutParams* generateDefaultLayoutParams()const;
    virtual bool checkLayoutParams(const LayoutParams* p)const;

    virtual void onSetLayoutParams(View* child,LayoutParams* layoutParams);

    void measureChildren(int widthMeasureSpec, int heightMeasureSpec);
    void measureChild(View* child, int parentWidthMeasureSpec,int parentHeightMeasureSpec);

    virtual void measureChildWithMargins(View* child,int parentWidthMeasureSpec, int widthUsed,
            int parentHeightMeasureSpec, int heightUsed);
    virtual bool drawChild(Canvas& canvas, View* child, long drawingTime);

    void onDebugDrawMargins(Canvas& canvas);
    void onDebugDraw(Canvas& canvas);
    void drawInvalidateRegion(Canvas&canvas);
    void dispatchDraw(Canvas&)override;

    bool hasActiveAnimations();
    void transformPointToViewLocal(float pint[2],View&);
    bool isTransformedTouchPointInView(int x,int y,View& child,Point*outLocalPoint);
    void drawableStateChanged()override;
    std::vector<int> onCreateDrawableState()const override;
    void dispatchSetPressed(bool pressed)override;
    void dispatchDrawableHotspotChanged(float x,float y)override;
    virtual int getChildDrawingOrder(int childCount, int i);
    std::vector<View*> buildOrderedChildList();

    virtual bool getChildStaticTransformation(View* child, Transformation* t);
    Transformation* getChildTransformation();
    void finishAnimatingView(View* view, Animation* animation);
    bool isViewTransitioning(View* view);
    void attachLayoutAnimationParameters(View* child,LayoutParams* params, int index, int count);
    void onChildVisibilityChanged(View* child, int oldVisibility, int newVisibility);
public:
    ViewGroup(int w,int h);
    ViewGroup(int x,int y,int w,int h);
    ViewGroup(Context*ctx,const AttributeSet& attrs);
    virtual ~ViewGroup();
    bool getTouchscreenBlocksFocus()const;
    bool shouldBlockFocusForTouchscreen()const;
    int getDescendantFocusability()const;
    void setDescendantFocusability(int);

    bool getClipChildren()const;
    void setClipChildren(bool clipChildren);
    bool getClipToPadding()const;
    void setClipToPadding(bool clipToPadding);
    void dispatchStartTemporaryDetach() override;
    void dispatchFinishTemporaryDetach()override;
    void dispatchSetSelected(bool selected)override;
    void dispatchSetActivated(bool activated)override;
    virtual std::vector<View*> buildTouchDispatchChildList();

    View*focusSearch(View*focused,int direction)const;
    View*getFocusedChild();
    View*findFocus()override;
    bool restoreDefaultFocus()override;
    virtual void requestChildFocus(View*child,View*focused);
    void unFocus(View* focused)override;
    void clearChildFocus(View* child);
    void focusableViewAvailable(View*);
    bool hasTransientState()override;
    void childHasTransientStateChanged(View* child, bool childHasTransientState);

    void offsetDescendantRectToMyCoords(View* descendant, Rect& rect)const;
    void offsetRectIntoDescendantCoords(View* descendant, Rect& rect)const;
    void offsetRectBetweenParentAndChild(View* descendant,Rect& rect,bool offsetFromChildToParent, bool clipToBounds)const;
    void offsetChildrenTopAndBottom(int offset);

    void addFocusables(std::vector<View*>& views, int direction, int focusableMode)override;
    void addKeyboardNavigationClusters(std::vector<View*>&views,int drection)override;
    void setOnHierarchyChangeListener(OnHierarchyChangeListener listener);
    bool restoreFocusNotInCluster();
    View*keyboardNavigationClusterSearch(View* currentCluster,int direction)override;
    bool requestFocus(int direction=FOCUS_DOWN,Rect*previouslyFocusedRect=nullptr)override;
    virtual bool requestLayoutDuringLayout(View* view);

    int getChildCount()const;
    View*getChildAt(int idx)const;
    int indexOfChild(View* child)const;
    bool isLayoutModeOptical()const;
    void cleanupLayoutState(View* child)const;

    virtual View& addView(View* view);
    virtual View& addView(View* child, int index);
    virtual View& addView(View* child, LayoutParams* params);
    virtual View& addView(View* child, int index, LayoutParams* params);
    View& addView(View* child, int width, int height);
    bool addViewInLayout(View* child, int index,LayoutParams* params);
    bool addViewInLayout(View* child, int index,LayoutParams* params,bool preventRequestLayout);
    void addTransientView(View*view,int index);
    void removeTransientView(View*);
   
    void startLayoutAnimation(); 
    void scheduleLayoutAnimation();
    void setLayoutAnimation(LayoutAnimationController*);
    LayoutAnimationController* getLayoutAnimation();
    void setLayoutAnimationListener(Animation::AnimationListener animationListener);
    Animation::AnimationListener getLayoutAnimationListener();
    void setLayoutTransition(LayoutTransition*);
    LayoutTransition*getLayoutTransition()const; 
    void clearDisappearingChildren();
    void startViewTransition(View* view);
    void endViewTransition(View* view);
  
    virtual void onViewAdded(View* child);
    virtual void onViewRemoved(View* child);
    void invalidateChild(View*child,Rect&dirty);
    ViewGroup*invalidateChildInParent(int* location,Rect& dirty);

    virtual LayoutParams* generateLayoutParams(const AttributeSet& attrs)const;
    static int getChildMeasureSpec(int spec, int padding, int childDimension);
    virtual void removeView(View* view);/*only remove view from children,no deleteion*/
    virtual void removeViewAt(int idx);
    virtual void removeAllViews();
    virtual void removeViews(int start, int count);
    virtual void removeViewInLayout(View* view);
    virtual void removeViewsInLayout(int start,int count);
    void removeAllViewsInLayout();
    virtual View* findViewById(int id)const;
    View* findViewByPredicateTraversal(std::function<bool(const View*)>predicate,View* childToSkip)const override;
    View* findViewWithTagTraversal(void*tag)const override;
    virtual bool shouldDelayChildPressedState();

    virtual bool onStartNestedScroll(View* child, View* target, int nestedScrollAxes);
    virtual void onNestedScrollAccepted(View* child, View* target, int axes);
    virtual void onStopNestedScroll(View* child);
    virtual void onNestedScroll(View* target, int dxConsumed, int dyConsumed,int dxUnconsumed, int dyUnconsumed);
    virtual void onNestedPreScroll(View* target, int dx, int dy, int*consumed);
    bool onNestedFling(View* target, float velocityX, float velocityY, bool consumed);
    int getNestedScrollAxes()const;
    bool onNestedPreFling(View* target, float velocityX, float velocityY);

    void setMotionEventSplittingEnabled(bool split);
    bool isMotionEventSplittingEnabled()const; 
    bool dispatchKeyEvent(KeyEvent&)override;
    bool dispatchUnhandledMove(View* focused, int direction)override;
    bool dispatchTouchEvent(MotionEvent& event)override;
    virtual void requestDisallowInterceptTouchEvent(bool disallowIntercept);
    bool onInterceptTouchEvent(MotionEvent& evt)override;

    void jumpDrawablesToCurrentState()override;
    void setAddStatesFromChildren(bool addsStates);
    bool addStatesFromChildren();
    virtual void childDrawableStateChanged(View* child);

    virtual void dispatchInvalidateOnAnimation(View* view);
    virtual void cancelInvalidate(View* view);
};

}  // namespace ui

#endif  // __NGLUI_GROUPVIEW_H__
