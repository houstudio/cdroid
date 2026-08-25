/*********************************************************************************
 * Copyright (C) 2019] [houzh@msn.com]
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
#include <widgetEx/navigationview/bottomnavigationmenuview.h>
#include <widgetEx/navigationview/bottomnavigationitemview.h>
#include <widgetEx/navigationview/navigationbarview.h>
#include <widget/framelayout.h>
#include <cdlog.h>

namespace cdroid{
namespace {
// design_bottom_navigation_* item width band (dp at the default density).
constexpr int INACTIVE_ITEM_MAX_WIDTH = 256;
constexpr int INACTIVE_ITEM_MIN_WIDTH = 96;
constexpr int ACTIVE_ITEM_MAX_WIDTH = 168;
constexpr int ACTIVE_ITEM_MIN_WIDTH = 96;
} // namespace

BottomNavigationMenuView::BottomNavigationMenuView(Context* context, const AttributeSet* attrs)
    : NavigationBarMenuView(context, attrs)
    , mInactiveItemMaxWidth(INACTIVE_ITEM_MAX_WIDTH)
    , mInactiveItemMinWidth(INACTIVE_ITEM_MIN_WIDTH)
    , mActiveItemMaxWidth(ACTIVE_ITEM_MAX_WIDTH)
    , mActiveItemMinWidth(ACTIVE_ITEM_MIN_WIDTH) {
    FrameLayout::LayoutParams* params = new FrameLayout::LayoutParams(
            LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
    params->gravity = Gravity::CENTER;
    setLayoutParams(params);
}

NavigationBarItemView* BottomNavigationMenuView::createNavigationBarItemView(Context* context) {
    return new BottomNavigationItemView(context);
}

void BottomNavigationMenuView::setItemHorizontalTranslationEnabled(bool itemHorizontalTranslationEnabled) {
    mItemHorizontalTranslationEnabled = itemHorizontalTranslationEnabled;
}

bool BottomNavigationMenuView::isItemHorizontalTranslationEnabled() const {
    return mItemHorizontalTranslationEnabled;
}

void BottomNavigationMenuView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    const int width = MeasureSpec::getSize(widthMeasureSpec);
    // Use visible item count to calculate widths
    const int visibleCount = getCurrentVisibleContentItemCount();
    // Use total item counts to measure children
    const int totalCount = getChildCount();
    mTempChildWidths.clear();

    int totalWidth = 0;
    int maxHeight = 0;

    const int parentHeight = MeasureSpec::getSize(heightMeasureSpec);
    const int heightSpec = MeasureSpec::makeMeasureSpec(parentHeight, MeasureSpec::AT_MOST);

    if (getItemIconGravity() == NavigationBarView::ITEM_ICON_GRAVITY_TOP) {
        if (isShifting(getLabelVisibilityMode(), visibleCount)
                && mItemHorizontalTranslationEnabled) {
            View* activeChild = getChildAt(getSelectedItemPosition());
            int activeItemWidth = mActiveItemMinWidth;
            if (activeChild != nullptr && activeChild->getVisibility() != View::GONE) {
                // Do an AT_MOST measure pass on the active child to get its desired
                // width, and resize the active child view based on that width
                activeChild->measure(
                        MeasureSpec::makeMeasureSpec(mActiveItemMaxWidth, MeasureSpec::AT_MOST),
                        heightSpec);
                activeItemWidth = std::max(activeItemWidth, activeChild->getMeasuredWidth());
            }
            const int activeGone = (activeChild == nullptr
                    || activeChild->getVisibility() == View::GONE) ? 0 : 1;
            const int inactiveCount = visibleCount - activeGone;
            const int activeMaxAvailable = width - inactiveCount * mInactiveItemMinWidth;
            const int activeWidth = std::min(activeMaxAvailable,
                    std::min(activeItemWidth, mActiveItemMaxWidth));
            const int inactiveMaxAvailable =
                    (width - activeWidth) / (inactiveCount == 0 ? 1 : inactiveCount);
            const int inactiveWidth = std::min(inactiveMaxAvailable, mInactiveItemMaxWidth);
            int extra = width - activeWidth - inactiveWidth * inactiveCount;

            for (int i = 0; i < totalCount; i++) {
                int tempChildWidth = 0;
                if (getChildAt(i)->getVisibility() != View::GONE) {
                    tempChildWidth = (i == getSelectedItemPosition()) ? activeWidth : inactiveWidth;
                    // Account for integer division which sometimes leaves some extra pixel
                    // spaces. e.g. If the nav was 10px wide, and 3 children were measured
                    // to be 3px-3px-3px, there would be a 1px gap somewhere, which this
                    // fills in.
                    if (extra > 0) {
                        tempChildWidth++;
                        extra--;
                    }
                }
                mTempChildWidths.push_back(tempChildWidth);
            }
        } else {
            const int maxAvailable = width / (visibleCount == 0 ? 1 : visibleCount);
            const int childWidth = std::min(maxAvailable, mActiveItemMaxWidth);
            int extra = width - childWidth * visibleCount;
            for (int i = 0; i < totalCount; i++) {
                int tempChildWidth = 0;
                if (getChildAt(i)->getVisibility() != View::GONE) {
                    tempChildWidth = childWidth;
                    if (extra > 0) {
                        tempChildWidth++;
                        extra--;
                    }
                }
                mTempChildWidths.push_back(tempChildWidth);
            }
        }

        for (int i = 0; i < totalCount; i++) {
            View* child = getChildAt(i);
            if (child->getVisibility() == View::GONE) {
                continue;
            }
            child->measure(
                    MeasureSpec::makeMeasureSpec(mTempChildWidths[i], MeasureSpec::EXACTLY),
                    heightSpec);
            LayoutParams* params = child->getLayoutParams();
            params->width = child->getMeasuredWidth();
            totalWidth += child->getMeasuredWidth();
            maxHeight = std::max(maxHeight, child->getMeasuredHeight());
        }
    } else { // icon gravity is start
        const int childCount = visibleCount == 0 ? 1 : visibleCount;
        // 3 items: 60% of the bar; 4: 70%; 5: 80%; 6+: 90%.
        const int minChildWidth = std::round(std::min((childCount + 3) / 10.f, 0.9f) * width / childCount);
        const int maxChildWidth = std::round((float)width / childCount);
        for (int i = 0; i < totalCount; i++) {
            View* child = getChildAt(i);
            if (child->getVisibility() != View::GONE) {
                child->measure(
                        MeasureSpec::makeMeasureSpec(maxChildWidth, MeasureSpec::AT_MOST),
                        heightSpec);
                if (child->getMeasuredWidth() < minChildWidth) {
                    child->measure(
                            MeasureSpec::makeMeasureSpec(minChildWidth, MeasureSpec::EXACTLY),
                            heightSpec);
                }
                totalWidth += child->getMeasuredWidth();
                maxHeight = std::max(maxHeight, child->getMeasuredHeight());
            }
        }
    }

    setMeasuredDimension(totalWidth, std::max(maxHeight, getSuggestedMinimumHeight()));
}

void BottomNavigationMenuView::onLayout(bool changed, int left, int top, int right, int bottom) {
    const int count = getChildCount();
    const int width = right - left;
    const int height = bottom - top;
    int used = 0;
    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        if (child->getVisibility() == View::GONE) {
            continue;
        }
        if (getLayoutDirection() == View::LAYOUT_DIRECTION_RTL) {
            child->layout(width - used - child->getMeasuredWidth(), 0,
                    width - used, height);
        } else {
            child->layout(used, 0, child->getMeasuredWidth() + used, height);
        }
        used += child->getMeasuredWidth();
    }
}

} // namespace cdroid
