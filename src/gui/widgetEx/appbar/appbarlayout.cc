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
#include <widgetEx/appbar/appbarlayout.h>
#include <widgetEx/widgetex_styleable.h>
#include <content/typedarray.h>
#include <cdlog.h>

namespace cdroid{
using namespace cdroid::internal;

// --- LayoutParams: app:layout_scrollFlags ------------------------------------

AppBarLayout::LayoutParams::LayoutParams(Context* c, const AttributeSet& attrs)
    : LinearLayout::LayoutParams(c, attrs) {
    // AOSP reads scrollViewFlags from AppBarLayout_Layout.
    auto ta = c->obtainStyledAttributes(attrs,
            cdroid::internal::R::styleable::AppBarLayoutLayout, 0, 0);
    scrollFlags = ta->getInt(cdroid::internal::R::styleable::AppBarLayoutLayout_layout_scrollFlags, 0);
}

AppBarLayout::LayoutParams::LayoutParams(int width, int height)
    : LinearLayout::LayoutParams(width, height) {}

AppBarLayout::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& source)
    : LinearLayout::LayoutParams(source) {}

// --- Behavior: header scroll on nested scroll --------------------------------

bool AppBarLayout::Behavior::onStartNestedScroll(CoordinatorLayout& /*parent*/, View& child,
        View& /*directTargetChild*/, View& /*target*/, int axes) {
    // AOSP HeaderBehavior: we only handle vertical scrolling of a scrollable
    // AppBarLayout child.
    AppBarLayout* abl = dynamic_cast<AppBarLayout*>(&child);
    return abl != nullptr && (axes & View::SCROLL_AXIS_VERTICAL) != 0
        && abl->hasScrollableChildren();
}

void AppBarLayout::Behavior::onNestedPreScroll(CoordinatorLayout& parent, View& child,
        View& /*target*/, int /*dx*/, int dy, int* consumed, int /*type*/) {
    // AOSP AppBarLayout.Behavior.onNestedPreScroll: scroll the header out/in
    // before the scrolling child consumes.
    if (dy == 0) return;
    AppBarLayout* abl = dynamic_cast<AppBarLayout*>(&child);
    if (abl == nullptr) return;

    const int range = abl->getTotalScrollRange();
    const int oldOffset = abl->getCurrentOffset();
    const int newOffset = std::min(0, std::max(-range, oldOffset - dy));
    if (newOffset != oldOffset) {
        abl->setOffset(newOffset);
        abl->onOffsetChanged(newOffset);
        // dy consumed = how far the header actually moved.
        consumed[1] = oldOffset - newOffset;
        parent.invalidate();
    }
}

void AppBarLayout::Behavior::onNestedScroll(CoordinatorLayout& parent, View& child, View& target,
        int dxConsumed, int dyConsumed, int dxUnconsumed, int dyUnconsumed,
        int /*type*/, int* /*consumed*/) {
    // AOSP: scroll the header back in for unconsumed downward scroll
    // (ENTER_ALWAYS). Simplified: unconsumed dy > 0 scrolls the header down.
    if (dyUnconsumed <= 0) return;
    AppBarLayout* abl = dynamic_cast<AppBarLayout*>(&child);
    if (abl == nullptr) return;
    const int newOffset = std::min(0, abl->getCurrentOffset() + dyUnconsumed);
    if (newOffset != abl->getCurrentOffset()) {
        abl->setOffset(newOffset);
        abl->onOffsetChanged(newOffset);
        parent.invalidate();
    }
}

void AppBarLayout::Behavior::onStopNestedScroll(CoordinatorLayout& /*parent*/, View& /*child*/,
        View& /*target*/, int /*type*/) {
    // AOSP snaps; snap is deferred (no animator here).
}

bool AppBarLayout::Behavior::onMeasureChild(CoordinatorLayout& /*parent*/, View& /*child*/,
        int /*parentWidthMeasureSpec*/, int /*widthUsed*/,
        int /*parentHeightMeasureSpec*/, int /*heightUsed*/) {
    // Default measure is fine for the CDROID substrate.
    return false;
}

bool AppBarLayout::Behavior::onLayoutChild(CoordinatorLayout& parent, View& child, int layoutDirection) {
    // Lay out like the parent would, then apply the current offset.
    parent.onLayoutChild(&child, layoutDirection);
    AppBarLayout* abl = dynamic_cast<AppBarLayout*>(&child);
    if (abl != nullptr) {
        addAccessibilityDelegateIfNeeded(parent, *abl);
    }
    return true;
}

// Material HeaderBehavior.addAccessibilityDelegateIfNeeded: the CoordinatorLayout
// is presented to accessibility as a ScrollView carrying the bar's collapse/
// expand actions.
void AppBarLayout::Behavior::addAccessibilityDelegateIfNeeded(
        CoordinatorLayout& coordinatorLayout, AppBarLayout& appBarLayout) {
    if (!coordinatorLayout.hasAccessibilityDelegate()) {
        coordinatorLayout.setAccessibilityDelegate(
                std::make_shared<AccessibilityDelegate>(this, &coordinatorLayout, &appBarLayout));
    }
}

AppBarLayout::Behavior::AccessibilityDelegate::AccessibilityDelegate(
        Behavior* behavior, CoordinatorLayout* parent, AppBarLayout* appBarLayout)
    : mBehavior(behavior), mParent(parent), mAppBarLayout(appBarLayout) {
}

View* AppBarLayout::Behavior::AccessibilityDelegate::getChildWithScrollingBehavior(
        CoordinatorLayout& coordinatorLayout) {
    const int childCount = coordinatorLayout.getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = coordinatorLayout.getChildAt(i);
        CoordinatorLayout::LayoutParams* lp =
                (CoordinatorLayout::LayoutParams*) child->getLayoutParams();
        if (dynamic_cast<AppBarLayout::ScrollingViewBehavior*>(lp->getBehavior()) != nullptr) {
            return child;
        }
    }
    return nullptr;
}

bool AppBarLayout::Behavior::AccessibilityDelegate::childrenHaveScrollFlags(
        AppBarLayout& appBarLayout) {
    const int childCount = appBarLayout.getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = appBarLayout.getChildAt(i);
        LayoutParams* childLp = (LayoutParams*) child->getLayoutParams();
        const int flags = childLp->scrollFlags;
        if (flags != LayoutParams::SCROLL_FLAG_NO_SCROLL) {
            return true;
        }
    }
    return false;
}

void AppBarLayout::Behavior::AccessibilityDelegate::onInitializeAccessibilityNodeInfo(
        View& host, AccessibilityNodeInfo& info) {
    View::AccessibilityDelegate::onInitializeAccessibilityNodeInfo(host, info);
    info.setClassName("android.widget.ScrollView");
    if (mAppBarLayout->getTotalScrollRange() == 0) {
        return;
    }
    View* scrollingView = getChildWithScrollingBehavior(*mParent);
    // Don't add actions if a child view doesn't have the behavior that will cause the
    // ABL to scroll.
    if (scrollingView == nullptr) {
        return;
    }

    // Don't add actions if the children do not have scrolling flags.
    if (!childrenHaveScrollFlags(*mAppBarLayout)) {
        return;
    }

    // CDROID substrate: the offset the scrolling sibling sees is the ABL's own
    // current offset (material HeaderBehavior.getTopBottomOffsetForScrollingSibling).
    const int offsetForScrollingSibling = mAppBarLayout->getCurrentOffset();

    if (offsetForScrollingSibling != -mAppBarLayout->getTotalScrollRange()) {
        // Add a collapsing action/forward if the view offset isn't the ABL scroll range.
        // (The same offset means the view is completely collapsed).
        info.addAction(&AccessibilityNodeInfo::AccessibilityAction::ACTION_SCROLL_FORWARD);
        info.setScrollable(true);
    }

    // Don't add an expanding action if the sibling offset is 0, which would mean the
    // ABL is completely expanded.
    if (offsetForScrollingSibling != 0) {
        if (scrollingView->canScrollVertically(-1)) {
            const int dy = -mAppBarLayout->getDownNestedPreScrollRange();
            // Offset by non-zero.
            if (dy != 0) {
                info.addAction(&AccessibilityNodeInfo::AccessibilityAction::ACTION_SCROLL_BACKWARD);
                info.setScrollable(true);
            }
        } else {
            info.addAction(&AccessibilityNodeInfo::AccessibilityAction::ACTION_SCROLL_BACKWARD);
            info.setScrollable(true);
        }
    }
}

bool AppBarLayout::Behavior::AccessibilityDelegate::performAccessibilityAction(
        View& host, int action, Bundle* args) {
    if (action == AccessibilityNodeInfo::ACTION_SCROLL_FORWARD) {
        mAppBarLayout->setExpanded(false);
        return true;
    } else if (action == AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD) {
        if (mAppBarLayout->getCurrentOffset() != 0) {
            View* scrollingView = getChildWithScrollingBehavior(*mParent);
            if (scrollingView == nullptr) return false;
            if (scrollingView->canScrollVertically(-1)) {
                // Expanding action. If the view can scroll down, expand the app bar
                // reflecting the logic in onNestedPreScroll.
                const int dy = -mAppBarLayout->getDownNestedPreScrollRange();
                // Offset by non-zero.
                if (dy != 0) {
                    int consumed[2] = {0, 0};
                    mBehavior->onNestedPreScroll(*mParent, static_cast<View&>(*mAppBarLayout),
                            *scrollingView, 0, dy, consumed, View::TYPE_NON_TOUCH);
                    return true;
                }
            } else {
                // If the view can't scroll down, we are probably at the top of the
                // scrolling content so expand completely.
                mAppBarLayout->setExpanded(true);
                return true;
            }
        }
    } else {
        return View::AccessibilityDelegate::performAccessibilityAction(host, action, args);
    }
    return false;
}

// --- ScrollingViewBehavior: pin the content below the header -----------------

bool AppBarLayout::ScrollingViewBehavior::layoutDependsOn(
        CoordinatorLayout& /*parent*/, View& /*child*/, View& dependency) {
    return dynamic_cast<AppBarLayout*>(&dependency) != nullptr;
}

bool AppBarLayout::ScrollingViewBehavior::onDependentViewChanged(
        CoordinatorLayout& parent, View& child, View& dependency) {
    // AOSP: offset the content to sit below the (possibly scrolled) header.
    AppBarLayout* abl = dynamic_cast<AppBarLayout*>(&dependency);
    if (abl == nullptr) return false;
    const int offset = abl->getMeasuredHeight() + abl->getCurrentOffset();
    child.offsetTopAndBottom(offset - child.getTop()
            - ((CoordinatorLayout::LayoutParams*)child.getLayoutParams())->topMargin);
    return false;
}

bool AppBarLayout::ScrollingViewBehavior::onMeasureChild(
        CoordinatorLayout& parent, View& child,
        int parentWidthMeasureSpec, int widthUsed,
        int parentHeightMeasureSpec, int heightUsed) {
    // AOSP: measure the child with the AppBarLayout's scroll range removed
    // from the available height (overlayTop).
    for (size_t i = 0; i < parent.getChildCount(); i++) {
        View* dep = parent.getChildAt(i);
        if (dynamic_cast<AppBarLayout*>(dep) == nullptr) continue;
        AppBarLayout* abl = (AppBarLayout*) dep;
        if (abl->getVisibility() == View::GONE) continue;
        const int scrollRange = abl->getTotalScrollRange() - mOverlayTop;
        const int height = MeasureSpec::getSize(parentHeightMeasureSpec)
                - scrollRange - heightUsed;
        child.measure(parentWidthMeasureSpec,
                MeasureSpec::makeMeasureSpec(height, MeasureSpec::AT_MOST));
        return true;
    }
    return false;
}

// --- AppBarLayout --------------------------------------------------------------

AppBarLayout::AppBarLayout(Context* context, const AttributeSet* attrs)
    : AppBarLayout(context, attrs, 0) {}

AppBarLayout::AppBarLayout(Context* context, const AttributeSet* attrs, int defStyleAttr)
    : LinearLayout(context, attrs, defStyleAttr) {
    setOrientation(LinearLayout::VERTICAL);
    mBehavior = new Behavior();
    AttachedBehavior::getBehavior = [this]() { return mBehavior; };

    // AppBarLayout styleable (lift-on-scroll etc. deferred). DEF_STYLE_RES =
    // Widget_Design_AppBarLayout (upstream's android:* items in that style are
    // inert through the View ctor chain, matching upstream behavior).
    auto ta = context->obtainStyledAttributes(attrs,
            cdroid::internal::R::styleable::AppBarLayout, defStyleAttr,
            cdroid::internal::R::style::Widget_Design_AppBarLayout);
    (void)ta;
}

AppBarLayout::~AppBarLayout() {
    delete mBehavior;
}

LinearLayout::LayoutParams* AppBarLayout::generateDefaultLayoutParams() const {
    // AOSP: children default to scrollable header LayoutParams.
    return new LayoutParams(ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::WRAP_CONTENT);
}

void AppBarLayout::addOnOffsetChangedListener(const OnOffsetChangedListener& listener) {
    mListeners.push_back(listener);
}

void AppBarLayout::onOffsetChanged(int offset) {
    mCurrentOffset = offset;
    invalidate();
    // AOSP iterates listeners forward in insertion order.
    for (auto& l : mListeners) {
        if (l) l(*this, offset);
    }
}

void AppBarLayout::setOffset(int offset) {
    const int range = getTotalScrollRange();
    mCurrentOffset = std::min(0, std::max(-range, offset));
    // Move the whole bar by the offset (HeaderBehavior setTopAndBottomOffset).
    if (getParent() != nullptr) {
        offsetTopAndBottom(mCurrentOffset - (getTop() - ((ViewGroup*)getParent())->getPaddingTop()));
    }
}

void AppBarLayout::setExpanded(bool expanded) {
    setOffset(expanded ? 0 : -getTotalScrollRange());
    onOffsetChanged(mCurrentOffset);
}

int AppBarLayout::getTotalScrollRange() {
    if (mTotalScrollRange != INVALID_SCROLL_RANGE) {
        return mTotalScrollRange;
    }
    int range = 0;
    for (int i = 0; i < getChildCount(); i++) {
        View* child = getChildAt(i);
        if (child->getVisibility() == View::GONE) continue;
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
        const int childHeight = child->getMeasuredHeight();
        const int flags = lp->scrollFlags;
        if ((flags & LayoutParams::SCROLL_FLAG_SCROLL) != 0) {
            range += childHeight + lp->topMargin + lp->bottomMargin;
            if ((flags & LayoutParams::SCROLL_FLAG_EXIT_UNTIL_COLLAPSED) != 0) {
                range -= child->getMinimumHeight();
                break;
            }
        } else {
            break;
        }
    }
    mTotalScrollRange = std::max(0, range);
    return mTotalScrollRange;
}

bool AppBarLayout::hasScrollableChildren() {
    return getTotalScrollRange() != 0;
}

int AppBarLayout::getDownNestedPreScrollRange() {
    if (mDownPreScrollRange != INVALID_SCROLL_RANGE) {
        // If we already have a valid value, return it
        return mDownPreScrollRange;
    }

    int range = 0;
    for (int i = getChildCount() - 1; i >= 0; i--) {
        View* child = getChildAt(i);
        if (child->getVisibility() == View::GONE) {
            // Gone views should not be included in the scroll range calculation.
            continue;
        }
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
        const int childHeight = child->getMeasuredHeight();
        const int flags = lp->scrollFlags;

        if ((flags & LayoutParams::FLAG_QUICK_RETURN) == LayoutParams::FLAG_QUICK_RETURN) {
            // First take the margin into account
            int childRange = lp->topMargin + lp->bottomMargin;
            // The view has the quick return flag combination...
            if ((flags & LayoutParams::SCROLL_FLAG_ENTER_ALWAYS_COLLAPSED) != 0) {
                // If they're set to enter collapsed, use the minimum height
                childRange += child->getMinimumHeight();
            } else if ((flags & LayoutParams::SCROLL_FLAG_EXIT_UNTIL_COLLAPSED) != 0) {
                // Only enter by the amount of the collapsed height
                childRange += childHeight - child->getMinimumHeight();
            } else {
                // Else use the full height
                childRange += childHeight;
            }
            // Material's first-child fitsSystemWindows clamp runs against
            // childHeight - getTopInset(); the inset machinery is not ported
            // (top inset is always 0), so the clamp is a no-op here.
            range += childRange;
        } else if (range > 0) {
            // If we've hit an non-quick return scrollable view, and we've already hit a
            // quick return view, return now
            break;
        }
    }
    return mDownPreScrollRange = std::max(0, range);
}

DECLARE_WIDGET(AppBarLayout)

// XML-declared behaviors (CoordinatorLayout app:layout_behavior values).
REGISTER_BEHAVIOR(AppBarLayout::Behavior, AppBarLayout_Behavior, "AppBarLayout$Behavior");
REGISTER_BEHAVIOR(AppBarLayout::ScrollingViewBehavior, AppBarLayout_ScrollingViewBehavior,
                 "AppBarLayout$ScrollingViewBehavior");

}//namespace cdroid
