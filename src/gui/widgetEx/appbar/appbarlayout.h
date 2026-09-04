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
#ifndef __APP_BAR_LAYOUT_H__
#define __APP_BAR_LAYOUT_H__
// Port of com.google.android.material.appbar.AppBarLayout — the scrolling
// header bar. CDROID substrate: the lift-on-scroll/status-bar-foreground/
// interpolation machinery is not ported (stubbed per AGENTS.md); the scroll
// range/offset core and the CoordinatorLayout Behavior run on CDROID's
// Behavior base.
#include <widget/linearlayout.h>
#include <widgetEx/coordinatorlayout/coordinatorlayout.h>
#include <functional>

namespace cdroid{

class AppBarLayout : public LinearLayout, public CoordinatorLayout::AttachedBehavior {
public:
    /** Callback on vertical offset change. */
    using OnOffsetChangedListener = std::function<void(AppBarLayout&, int)>;

    class LayoutParams : public LinearLayout::LayoutParams {
    public:
        static constexpr int SCROLL_FLAG_NO_SCROLL = 0;
        static constexpr int SCROLL_FLAG_SCROLL = 0x1;
        static constexpr int SCROLL_FLAG_EXIT_UNTIL_COLLAPSED = 0x2;
        static constexpr int SCROLL_FLAG_ENTER_ALWAYS = 0x4;
        static constexpr int SCROLL_FLAG_ENTER_ALWAYS_COLLAPSED = 0x8;
        static constexpr int SCROLL_FLAG_SNAP = 0x10;
        /** Material: SCROLL_FLAG_SCROLL | SCROLL_FLAG_ENTER_ALWAYS. */
        static constexpr int FLAG_QUICK_RETURN = SCROLL_FLAG_SCROLL | SCROLL_FLAG_ENTER_ALWAYS;

        int scrollFlags = 0;
        LayoutParams(Context* c, const AttributeSet& attrs);
        LayoutParams(int width, int height);
        LayoutParams(const ViewGroup::LayoutParams& source);
    };

    /** The scrolling Behavior (attached automatically per AttachedBehavior). */
    class Behavior : public CoordinatorLayout::Behavior {
    protected:
        int mOffsetDelta = 0;
        bool mSkipNestedPreScroll = false;
    private:
        // Material HeaderBehavior.addAccessibilityDelegateIfNeeded: a delegate on
        // the CoordinatorLayout presenting the bar as a ScrollView with collapse/
        // expand actions. material's helpers (getChildWithScrollingBehavior,
        // childrenHaveScrollFlags) live on the anonymous class here.
        class AccessibilityDelegate : public View::AccessibilityDelegate {
        private:
            Behavior* mBehavior;
            CoordinatorLayout* mParent;
            AppBarLayout* mAppBarLayout;
            View* getChildWithScrollingBehavior(CoordinatorLayout& coordinatorLayout);
            bool childrenHaveScrollFlags(AppBarLayout& appBarLayout);
        public:
            AccessibilityDelegate(Behavior* behavior, CoordinatorLayout* parent,
                    AppBarLayout* appBarLayout);
            void onInitializeAccessibilityNodeInfo(View& host, AccessibilityNodeInfo& info) override;
            bool performAccessibilityAction(View& host, int action, Bundle* args) override;
        };
        void addAccessibilityDelegateIfNeeded(CoordinatorLayout& coordinatorLayout,
                AppBarLayout& appBarLayout);
    public:
        // androidx ViewOffsetBehavior/HeaderBehavior ctors (XML Behavior inflation).
        Behavior() = default;
        Behavior(Context* context, const AttributeSet* attrs)
            : CoordinatorLayout::Behavior(context, attrs) {}

        bool onStartNestedScroll(CoordinatorLayout& parent, View& child, View& directTargetChild,
                View& target, int axes) override;
        void onNestedPreScroll(CoordinatorLayout& parent, View& child, View& target,
                int dx, int dy, int* consumed, int type) override;
        void onNestedScroll(CoordinatorLayout& parent, View& child, View& target,
                int dxConsumed, int dyConsumed, int dxUnconsumed, int dyUnconsumed,
                int type, int* consumed) override;
        void onStopNestedScroll(CoordinatorLayout& parent, View& child, View& target, int type) override;
        bool onMeasureChild(CoordinatorLayout& parent, View& child,
                int parentWidthMeasureSpec, int widthUsed,
                int parentHeightMeasureSpec, int heightUsed) override;
        bool onLayoutChild(CoordinatorLayout& parent, View& child, int layoutDirection) override;
    };

    /** Pin a sibling below the header (material ScrollingViewBehavior). */
    class ScrollingViewBehavior : public CoordinatorLayout::Behavior {
    protected:
        int mOverlayTop = 0;
    public:
        // androidx HeaderScrollingViewBehavior ctors (XML Behavior inflation).
        ScrollingViewBehavior() = default;
        ScrollingViewBehavior(Context* context, const AttributeSet* attrs)
            : CoordinatorLayout::Behavior(context, attrs) {}

        bool layoutDependsOn(CoordinatorLayout& parent, View& child, View& dependency) override;
        bool onDependentViewChanged(CoordinatorLayout& parent, View& child, View& dependency) override;
        bool onMeasureChild(CoordinatorLayout& parent, View& child,
                int parentWidthMeasureSpec, int widthUsed,
                int parentHeightMeasureSpec, int heightUsed) override;
        bool onLayoutChild(CoordinatorLayout& parent, View& child, int layoutDirection) override;
    };
private:
    static constexpr int INVALID_SCROLL_RANGE = -1;
    int mCurrentOffset = 0;
    int mTotalScrollRange = INVALID_SCROLL_RANGE;
    int mDownPreScrollRange = INVALID_SCROLL_RANGE;
    std::vector<OnOffsetChangedListener> mListeners;
    Behavior* mBehavior = nullptr;
protected:
    LinearLayout::LayoutParams* generateDefaultLayoutParams() const override;
public:
    AppBarLayout(Context* context, const AttributeSet* attrs);
    AppBarLayout(Context* context, const AttributeSet* attrs, int defStyleAttr);
    ~AppBarLayout() override;

    void addOnOffsetChangedListener(const OnOffsetChangedListener& listener);
    int getTotalScrollRange();
    /** Material getDownNestedPreScrollRange: the range the bar re-enters for
        a downward nested scroll (enter-always/quick-return children). */
    int getDownNestedPreScrollRange();
    bool hasScrollableChildren();
    int getCurrentOffset() const { return mCurrentOffset; }
    // Header offset control: negative offsets scroll the bar up (AOSP setExpanded
    // drives the same path).
    void setExpanded(bool expanded);
    void setOffset(int offset);

    void onOffsetChanged(int offset);
};

}//namespace cdroid
#endif/*__APP_BAR_LAYOUT_H__*/
