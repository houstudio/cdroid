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
#include <core/typedarray.h>
#include <cdlog.h>

namespace cdroid{
using namespace cdroid::internal;

// --- LayoutParams: app:layout_scrollFlags ------------------------------------

AppBarLayout::LayoutParams::LayoutParams(Context* c, const AttributeSet* attrs)
    : LinearLayout::LayoutParams(c, *attrs) {
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
    return true;
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

AppBarLayout::AppBarLayout(Context* context, const AttributeSet& attrs)
    : AppBarLayout(context, &attrs, 0) {}

AppBarLayout::AppBarLayout(Context* context, const AttributeSet* attrs, int defStyleAttr)
    : LinearLayout(context, attrs, defStyleAttr) {
    setOrientation(LinearLayout::VERTICAL);
    mBehavior = new Behavior();
    AttachedBehavior::getBehavior = [this]() { return mBehavior; };

    // AppBarLayout styleable (lift-on-scroll etc. deferred).
    auto ta = context->obtainStyledAttributes(attrs,
            cdroid::internal::R::styleable::AppBarLayout, defStyleAttr, 0);
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

DECLARE_WIDGET(AppBarLayout)

}//namespace cdroid
