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
#ifndef __COORDINATOR_LAYOUT_H__
#define __COORDINATOR_LAYOUT_H__
#include <view/viewgroup.h>
#include <view/windowinsets.h>
#include <widget/nestedscrollinghelper.h>
#include <widgetEx/directedacyclicgraph.h>

namespace cdroid{
class CoordinatorLayout:public ViewGroup{// implements NestedScrollingParent2 {
public:
    class Behavior;
    class LayoutParams:public ViewGroup::MarginLayoutParams {
    private:
        bool mDidBlockInteraction;
        bool mDidAcceptNestedScrollTouch;
        bool mDidAcceptNestedScrollNonTouch;
        bool mDidChangeAfterNestedScroll;
    protected:
        Behavior* mBehavior;
        friend class CoordinatorLayout;
        bool mBehaviorResolved = false;
    public: 
        int gravity = Gravity::NO_GRAVITY;
        int anchorGravity = Gravity::NO_GRAVITY;
        int keyline = -1;
        int mAnchorId = View::NO_ID;
        int insetEdge = Gravity::NO_GRAVITY;
        int dodgeInsetEdges = Gravity::NO_GRAVITY;
     public:
        int mInsetOffsetX;
        int mInsetOffsetY;
        View* mAnchorView;
        View* mAnchorDirectChild;
        Rect mLastChildRect;
        void* mBehaviorTag;
    private:
        void init();
        void resolveAnchorView(View* forChild, CoordinatorLayout& parent);
        bool verifyAnchorView(View* forChild, CoordinatorLayout& parent);
        bool shouldDodge(View* other, int layoutDirection);
    protected:
        void setLastChildRect(const Rect& r);
        Rect getLastChildRect()const;
        bool checkAnchorChanged()const;
        bool didBlockInteraction();
        bool isBlockingInteractionBelow(CoordinatorLayout& parent, View* child);
        void resetTouchBehaviorTracking();
        void resetNestedScroll(int type);
        void setNestedScrollAccepted(int type, bool accept);
        bool isNestedScrollAccepted(int type);
        bool getChangedAfterNestedScroll();
        void setChangedAfterNestedScroll(bool changed);
        void resetChangedAfterNestedScroll();
        bool dependsOn(CoordinatorLayout& parent, View* child, View* dependency);
        void invalidateAnchor();
        View* findAnchorView(CoordinatorLayout& parent, View* forChild);
    public: 
        LayoutParams(int width, int height);
        ~LayoutParams()override;
        LayoutParams(Context* context, const AttributeSet& attrs);
        LayoutParams(const LayoutParams& p);
        LayoutParams(const MarginLayoutParams& p);
        LayoutParams(const ViewGroup::LayoutParams& p);
        int getAnchorId()const;
        void setAnchorId(int id);   
        Behavior* getBehavior()const;
        void setBehavior(Behavior* behavior);
    };
private:
    static constexpr int TYPE_ON_INTERCEPT = 0;
    static constexpr int TYPE_ON_TOUCH = 1;
protected:
    static constexpr int EVENT_PRE_DRAW = 0;
    static constexpr int EVENT_NESTED_SCROLL = 1;
    static constexpr int EVENT_VIEW_REMOVED = 2;
private:
    std::vector<View*> mDependencySortedChildren;
    DirectedAcyclicGraph<View> mChildDag;

    std::vector<View*> mTempDependenciesList;
    int mTempIntPair[2];
    bool mDisallowInterceptReset;
    bool mIsAttachedToWindow;
    std::vector<int> mKeylines;

    View* mBehaviorTouchView;
    View* mNestedScrollingTarget;

    ViewTreeObserver::OnPreDrawListener mOnPreDrawListener;
    bool mNeedsPreDrawListener;

    WindowInsets* mLastInsets;
    bool mDrawStatusBarBackground;
    Drawable* mStatusBarBackground;

    ViewGroup::OnHierarchyChangeListener mOnHierarchyChangeListener;
    View::OnApplyWindowInsetsListener mApplyWindowInsetsListener;
    NestedScrollingParentHelper* mNestedScrollingParentHelper;
private:
    void initView();
    void resetTouchBehaviors(bool notifyOnInterceptTouchEvent);
    void getTopSortedChildren(std::vector<View*>& out);
    bool performIntercept(MotionEvent& ev,int type);
    int getKeyline(int index);
    void prepareChildren();
    WindowInsets dispatchApplyWindowInsetsToBehaviors(WindowInsets insets);
    void getDesiredAnchoredChildRectWithoutConstraints(View* child, int layoutDirection,
            const Rect& anchorRect, Rect& out,const LayoutParams* lp, int childWidth, int childHeight);
    void constrainChildRect(const LayoutParams& lp, Rect& out, int childWidth, int childHeight);
    void layoutChildWithAnchor(View* child, View* anchor, int layoutDirection);
    void layoutChildWithKeyline(View* child, int keyline, int layoutDirection);
    void layoutChild(View* child, int layoutDirection);
    static int resolveGravity(int gravity);
    static int resolveKeylineGravity(int gravity);
    static int resolveAnchoredChildGravity(int gravity);
    void offsetChildByInset(View* child,const Rect& inset,int layoutDirection);
    void setInsetOffsetX(View* child, int offsetX);
    void setInsetOffsetY(View* child, int offsetY);
    bool hasDependencies(View* child)const;
    void setupForInsets();
protected:
    void drawableStateChanged() override;
    bool verifyDrawable(Drawable* who)const override;
    WindowInsets setWindowInsets(const WindowInsets& insets);
    int getSuggestedMinimumWidth()override;
    int getSuggestedMinimumHeight()override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int l, int t, int r, int b)override;
    void recordLastChildRect(View* child, Rect& r);
    void getLastChildRect(View* child, Rect& out);
    void getChildRect(View* child, bool transform, Rect& out);
    void getDesiredAnchoredChildRect(View* child, int layoutDirection,const Rect& anchorRect, Rect& out);
    bool drawChild(Canvas& canvas, View* child, int64_t drawingTime)override;
    void onChildViewsChanged(int type);
    std::vector<View*> getDependencySortedChildren();
    void ensurePreDrawListener();
    void addPreDrawListener();
    void removePreDrawListener();
    void offsetChildToAnchor(View* child, int layoutDirection);
    LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* p)const override;
    LayoutParams* generateDefaultLayoutParams()const override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;

    void onRestoreInstanceState(Parcelable& state)override;
    Parcelable* onSaveInstanceState() override;
public:
    CoordinatorLayout(int w, int h);
    CoordinatorLayout(Context* context,const AttributeSet& attrs);
    ~CoordinatorLayout();
    void setOnHierarchyChangeListener(const OnHierarchyChangeListener& onHierarchyChangeListener)override;
    void onAttachedToWindow()override;
    void onDetachedFromWindow()override;
    void setStatusBarBackground(Drawable* bg);
    Drawable* getStatusBarBackground()const;
    void setVisibility(int visibility) override;
    void setStatusBarBackgroundResource(const std::string& resId);
    void setStatusBarBackgroundColor(int color);
    WindowInsets getLastWindowInsets();
    bool onInterceptTouchEvent(MotionEvent& ev) override;
    bool onTouchEvent(MotionEvent& ev)override;
    void requestDisallowInterceptTouchEvent(bool disallowIntercept)override;

    static Behavior* parseBehavior(Context* context,const AttributeSet& attrs,const std::string& name);
    LayoutParams* getResolvedLayoutParams(View* child);

    void getDescendantRect(View* descendant, Rect& out);
    void onMeasureChild(View* child, int parentWidthMeasureSpec, int widthUsed,int parentHeightMeasureSpec, int heightUsed);

    void onLayoutChild(View* child, int layoutDirection);
    void onDraw(Canvas& c)override;
    void setFitsSystemWindows(bool fitSystemWindows);// override;

    void dispatchDependentViewsChanged(View& view);

    std::vector<View*> getDependencies(View& child);

    std::vector<View*> getDependents(View& child);
    bool isPointInChildBounds(View& child, int x, int y);

    bool doViewsOverlap(View& first, View& second);
    LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;

    bool onStartNestedScroll(View* child, View* target, int nestedScrollAxes)override;
    bool onStartNestedScroll(View* child, View* target, int axes, int type)override;
    void onNestedScrollAccepted(View* child, View* target, int nestedScrollAxes)override;
    void onNestedScrollAccepted(View* child, View* target, int nestedScrollAxes, int type) override;
    void onStopNestedScroll(View* target)override;
    void onStopNestedScroll(View* target, int type)override;
    void onNestedScroll(View* target, int dxConsumed, int dyConsumed,int dxUnconsumed, int dyUnconsumed)override;
    void onNestedScroll(View* target, int dxConsumed, int dyConsumed,int dxUnconsumed, int dyUnconsumed, int type)override;
    void onNestedPreScroll(View* target, int dx, int dy, int consumed[]) override;

    void onNestedPreScroll(View* target, int dx, int dy, int* consumed, int  type)override;
    bool onNestedFling(View* target, float velocityX, float velocityY, bool consumed)override;
    bool onNestedPreFling(View* target, float velocityX, float velocityY)override;
    int getNestedScrollAxes()override;

    bool requestChildRectangleOnScreen(View* child,Rect& rectangle, bool immediate)override;

    struct AttachedBehavior {
	std::function<Behavior*()> getBehavior;
    };
};

/**
 * Interaction behavior plugin for child views of {@link CoordinatorLayout}.
 *
 * <p>A Behavior implements one or more interactions that a user can take on a child view.
 * These interactions may include drags, swipes, flings, or any other gestures.</p>
 *
 * @param <V> The View type that this Behavior operates on
 */
//template<typename V>
class CoordinatorLayout::Behavior{//; :public View {//<V extends View> {
    //static_assert(std::is_base_of<View, V>::value, "V must be a subclass of View");
public:
    Behavior() {}
    Behavior(Context* context, const AttributeSet& attrs) {}
    virtual void onAttachedToLayoutParams(CoordinatorLayout::LayoutParams& params) {}
    virtual void onDetachedFromLayoutParams() {}
    virtual bool onInterceptTouchEvent(CoordinatorLayout& parent, View& child, MotionEvent& ev) {
        return false;
    }
    virtual bool onTouchEvent(CoordinatorLayout& parent, View& child, MotionEvent& ev) {
        return false;
    }
    virtual int getScrimColor(CoordinatorLayout& parent, View& child) { return 0xFF000000; }
    virtual float getScrimOpacity(CoordinatorLayout& parent, View& child) { return .0f; }
    virtual bool blocksInteractionBelow(CoordinatorLayout& parent, View& child) {
        return getScrimOpacity(parent, child) > .0f;
    }
    virtual bool layoutDependsOn(CoordinatorLayout& parent, View& child, View& dependency) {
        return false;
    }
    virtual bool onDependentViewChanged(CoordinatorLayout& parent, View& child, View& dependency) { 
        return false;
    }
    virtual void onDependentViewRemoved(CoordinatorLayout& parent, View& child, View& dependency) {
    }
    virtual bool onMeasureChild(CoordinatorLayout& parent, View& child,int parentWidthMeasureSpec,
        int widthUsed, int parentHeightMeasureSpec, int heightUsed) {
        return false;
    }
    virtual bool onLayoutChild(CoordinatorLayout& parent, View& child, int layoutDirection) {
        return false;
    }
 
    static void setTag(View& child, void* tag);
    static void* getTag(View& child);

    virtual bool onStartNestedScroll(CoordinatorLayout& coordinatorLayout,
        View& child, View& directTargetChild, View& target, int axes) {
        return false;
    }
    virtual bool onStartNestedScroll(CoordinatorLayout& coordinatorLayout,View& child,
          View& directTargetChild, View& target, int axes, int type) {
        if (type == View::TYPE_TOUCH) {
            return onStartNestedScroll(coordinatorLayout, child, directTargetChild, target, axes);
        }
        return false;
    }
    virtual void onNestedScrollAccepted(CoordinatorLayout& coordinatorLayout,
        View& child, View& directTargetChild, View& target, int axes) {
    }
    virtual void onNestedScrollAccepted(CoordinatorLayout& coordinatorLayout, View& child, View& directTargetChild,
        View& target, int axes, int type) {
        if (type == View::TYPE_TOUCH) {
            onNestedScrollAccepted(coordinatorLayout, child, directTargetChild, target, axes);
        }
    }
    virtual void onStopNestedScroll(CoordinatorLayout& coordinatorLayout, View& child, View& target) {
    }
    virtual void onStopNestedScroll(CoordinatorLayout& coordinatorLayout, View& child, View& target, int type) {
        if (type == View::TYPE_TOUCH) {
            onStopNestedScroll(coordinatorLayout, child, target);
        }
    }
    virtual void onNestedScroll(CoordinatorLayout& coordinatorLayout, View& child,
        View& target, int dxConsumed, int dyConsumed, int dxUnconsumed, int dyUnconsumed) {
    }
    virtual void onNestedScroll(CoordinatorLayout& coordinatorLayout, View& child,
        View& target, int dxConsumed, int dyConsumed, int dxUnconsumed, int dyUnconsumed, int type) {
        if (type == View::TYPE_TOUCH) {
            onNestedScroll(coordinatorLayout, child, target, dxConsumed, dyConsumed, dxUnconsumed, dyUnconsumed);
        }
    }
    virtual void onNestedPreScroll(CoordinatorLayout& coordinatorLayout,
        View& child, View& target, int dx, int dy, int* consumed) {
    }
    virtual void onNestedPreScroll(CoordinatorLayout& coordinatorLayout, View& child, View& target,
        int dx, int dy, int* consumed, int type) {
        if (type == View::TYPE_TOUCH) {
            onNestedPreScroll(coordinatorLayout, child, target, dx, dy, consumed);
        }
    }
    virtual bool onNestedFling(CoordinatorLayout& coordinatorLayout, View& child, View& target,
        float velocityX, float velocityY, bool consumed) {
        return false;
    }
    virtual bool onNestedPreFling(CoordinatorLayout& coordinatorLayout,
        View& child, View& target, float velocityX, float velocityY) {
        return false;
    }
    WindowInsets onApplyWindowInsets(CoordinatorLayout& coordinatorLayout, View& child, WindowInsets& insets) {
        return insets;
    }
    virtual bool onRequestChildRectangleOnScreen(CoordinatorLayout& coordinatorLayout,
        View& child, const Rect& rectangle, bool immediate) {
        return false;
    }
    virtual void onRestoreInstanceState(CoordinatorLayout& parent, View& child, Parcelable& state) {
    }
    Parcelable* onSaveInstanceState(CoordinatorLayout& parent, View& child) { 
        return nullptr;
    }
    virtual bool getInsetDodgeRect(CoordinatorLayout& parent, View& child, Rect& rect) {
        return false;
    }
};

#if 0
class CoordinatorLayout::SavedState extends AbsSavedState {
    SparseArray<Parcelable> behaviorStates;

    public SavedState(Parcel source, ClassLoader loader) {
        super(source, loader);

        final int size = source.readInt();

        final int[] ids = new int[size];
        source.readIntArray(ids);

        final Parcelable[] states = source.readParcelableArray(loader);

        behaviorStates = new SparseArray<>(size);
        for (int i = 0; i < size; i++) {
            behaviorStates.append(ids[i], states[i]);
        }
    }

    public SavedState(Parcelable superState) {
        super(superState);
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        super.writeToParcel(dest, flags);

        final int size = behaviorStates != null ? behaviorStates.size() : 0;
        dest.writeInt(size);

        final int[] ids = new int[size];
        final Parcelable[] states = new Parcelable[size];

        for (int i = 0; i < size; i++) {
            ids[i] = behaviorStates.keyAt(i);
            states[i] = behaviorStates.valueAt(i);
        }
        dest.writeIntArray(ids);
        dest.writeParcelableArray(states, flags);

    }

    public static final Creator<SavedState> CREATOR =
            new ClassLoaderCreator<SavedState>() {
                @Override
                public SavedState createFromParcel(Parcel in, ClassLoader loader) {
                    return new SavedState(in, loader);
                }

                @Override
                public SavedState createFromParcel(Parcel in) {
                    return new SavedState(in, null);
                }

                @Override
                public SavedState[] newArray(int size) {
                    return new SavedState[size];
                }
            };
}
#endif
}/*endof namespace*/
#endif/*__COORDINATOR_LAYOUT_H__*/
