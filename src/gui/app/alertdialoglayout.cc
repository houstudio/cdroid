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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  0211-1301  USA
 *********************************************************************************/
#include <app/alertdialoglayout.h>
#include <widget/internal_R.h>
#include <view/layoutinflater.h>
#include <view/viewgroup.h>

namespace cdroid{
using namespace cdroid::internal;

DECLARE_WIDGET2(AlertDialogLayout, "AlertDialogLayout");

AlertDialogLayout::AlertDialogLayout(Context* context)
  :LinearLayout(context){
}

AlertDialogLayout::AlertDialogLayout(Context* context,const AttributeSet* attrs)
  :AlertDialogLayout(context,attrs,0,0){
}

AlertDialogLayout::AlertDialogLayout(Context* context,const AttributeSet* attrs,int defStyleAttr,int defStyleRes)
  :LinearLayout(context,attrs,defStyleAttr){
    (void)defStyleRes;   // LinearLayout has no 4-arg ctor yet
}

void AlertDialogLayout::onMeasure(int widthMeasureSpec,int heightMeasureSpec){
    if (!tryOnMeasure(widthMeasureSpec, heightMeasureSpec)) {
        // Failed to perform custom measurement, let superclass handle it.
        LinearLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);
    }
}

bool AlertDialogLayout::tryOnMeasure(int widthMeasureSpec,int heightMeasureSpec){
    View* topPanel = nullptr;
    View* buttonPanel = nullptr;
    View* middlePanel = nullptr;

    const int count = (int)getChildCount();
    for (int i = 0; i < count; i++) {
        const View* child = getChildAt(i);
        if (child->getVisibility() == View::GONE) {
            continue;
        }

        const int id = child->getId();
        switch (id) {
            case R::id::topPanel:
                topPanel = getChildAt(i);
                break;
            case R::id::buttonPanel:
                buttonPanel = getChildAt(i);
                break;
            case R::id::contentPanel:
            case R::id::customPanel:
                if (middlePanel != nullptr) {
                    // Both the content and custom are visible. Abort!
                    return false;
                }
                middlePanel = getChildAt(i);
                break;
            default:
                // Unknown top-level child. Abort!
                return false;
        }
    }

    const int heightMode = MeasureSpec::getMode(heightMeasureSpec);
    const int heightSize = MeasureSpec::getSize(heightMeasureSpec);
    const int widthMode = MeasureSpec::getMode(widthMeasureSpec);

    int childState = 0;
    int usedHeight = getPaddingTop() + getPaddingBottom();

    if (topPanel != nullptr) {
        topPanel->measure(widthMeasureSpec, MeasureSpec::UNSPECIFIED);

        usedHeight += topPanel->getMeasuredHeight();
        childState = View::combineMeasuredStates(childState, topPanel->getMeasuredState());
    }

    int buttonHeight = 0;
    int buttonWantsHeight = 0;
    if (buttonPanel != nullptr) {
        buttonPanel->measure(widthMeasureSpec, MeasureSpec::UNSPECIFIED);
        buttonHeight = resolveMinimumHeight(buttonPanel);
        buttonWantsHeight = buttonPanel->getMeasuredHeight() - buttonHeight;

        usedHeight += buttonHeight;
        childState = View::combineMeasuredStates(childState, buttonPanel->getMeasuredState());
    }

    int middleHeight = 0;
    if (middlePanel != nullptr) {
        int childHeightSpec;
        if (heightMode == MeasureSpec::UNSPECIFIED) {
            childHeightSpec = MeasureSpec::UNSPECIFIED;
        } else {
            childHeightSpec = MeasureSpec::makeMeasureSpec(
                    std::max(0, heightSize - usedHeight), heightMode);
        }

        middlePanel->measure(widthMeasureSpec, childHeightSpec);
        middleHeight = middlePanel->getMeasuredHeight();

        usedHeight += middleHeight;
        childState = View::combineMeasuredStates(childState, middlePanel->getMeasuredState());
    }

    int remainingHeight = heightSize - usedHeight;

    // Time for the "real" button measure pass. If we have remaining space,
    // make the button pane bigger up to its target height. Otherwise,
    // just remeasure the button at whatever height it needs.
    if (buttonPanel != nullptr) {
        usedHeight -= buttonHeight;

        const int heightToGive = std::min(remainingHeight, buttonWantsHeight);
        if (heightToGive > 0) {
            remainingHeight -= heightToGive;
            buttonHeight += heightToGive;
        }

        const int childHeightSpec = MeasureSpec::makeMeasureSpec(
                buttonHeight, MeasureSpec::EXACTLY);
        buttonPanel->measure(widthMeasureSpec, childHeightSpec);

        usedHeight += buttonPanel->getMeasuredHeight();
        childState = View::combineMeasuredStates(childState, buttonPanel->getMeasuredState());
    }

    // If we still have remaining space, make the middle pane bigger up
    // to the maximum height.
    if (middlePanel != nullptr && remainingHeight > 0) {
        usedHeight -= middleHeight;

        const int heightToGive = remainingHeight;
        remainingHeight -= heightToGive;
        middleHeight += heightToGive;

        // Pass the same height mode as we're using for the dialog itself.
        // If it's EXACTLY, then the middle pane MUST use the entire
        // height.
        const int childHeightSpec = MeasureSpec::makeMeasureSpec(
                middleHeight, heightMode);
        middlePanel->measure(widthMeasureSpec, childHeightSpec);

        usedHeight += middlePanel->getMeasuredHeight();
        childState = View::combineMeasuredStates(childState, middlePanel->getMeasuredState());
    }

    // Compute desired width as maximum child width.
    int maxWidth = 0;
    for (int i = 0; i < count; i++) {
        const View* child = getChildAt(i);
        if (child->getVisibility() != View::GONE) {
            maxWidth = std::max(maxWidth, child->getMeasuredWidth());
        }
    }

    maxWidth += getPaddingLeft() + getPaddingRight();

    const int widthSizeAndState = resolveSizeAndState(maxWidth, widthMeasureSpec, childState);
    const int heightSizeAndState = resolveSizeAndState(usedHeight, heightMeasureSpec, 0);
    setMeasuredDimension(widthSizeAndState, heightSizeAndState);

    // If the children weren't already measured EXACTLY, we need to run
    // another measure pass to for MATCH_PARENT widths.
    if (widthMode != MeasureSpec::EXACTLY) {
        forceUniformWidth(count, heightMeasureSpec);
    }

    return true;
}

/** Remeasures child views to exactly match the layout's measured width.
 *
 *  @param count the number of child views
 *  @param heightMeasureSpec the original height measure spec */
void AlertDialogLayout::forceUniformWidth(int count,int heightMeasureSpec){
    // Pretend that the linear layout has an exact size.
    const int uniformMeasureSpec = MeasureSpec::makeMeasureSpec(
            getMeasuredWidth(), MeasureSpec::EXACTLY);

    for (int i = 0; i < count; i++) {
        const View* child = getChildAt(i);
        if (child->getVisibility() != View::GONE) {
            LinearLayout::LayoutParams* lp = (LinearLayout::LayoutParams*)child->getLayoutParams();
            if (lp->width == ViewGroup::LayoutParams::MATCH_PARENT) {
                // Temporarily force children to reuse their old measured
                // height.
                const int oldHeight = lp->height;
                lp->height = child->getMeasuredHeight();

                // Remeasure with new dimensions.
                measureChildWithMargins(getChildAt(i), uniformMeasureSpec, 0, heightMeasureSpec, 0);
                lp->height = oldHeight;
            }
        }
    }
}

/** Attempts to resolve the minimum height of a view.
 *
 *  If the view doesn't have a minimum height set and only contains a single
 *  child, attempts to resolve the minimum height of the child view.
 *
 *  @param v the view whose minimum height to resolve
 *  @return the minimum height */
int AlertDialogLayout::resolveMinimumHeight(View* v){
    const int minHeight = v->getMinimumHeight();
    if (minHeight > 0) {
        return minHeight;
    }

    ViewGroup* vg = dynamic_cast<ViewGroup*>(v);
    if (vg != nullptr) {
        if (vg->getChildCount() == 1) {
            return resolveMinimumHeight(vg->getChildAt(0));
        }
    }

    return 0;
}

void AlertDialogLayout::onLayout(bool changed,int left,int top,int right,int bottom){
    const int paddingLeft = mPaddingLeft;

    // Where right end of child should go
    const int width = right - left;
    const int childRight = width - mPaddingRight;

    // Space available for child
    const int childSpace = width - paddingLeft - mPaddingRight;

    const int totalLength = getMeasuredHeight();
    const int count = (int)getChildCount();
    const int gravity = getGravity();
    const int majorGravity = gravity & Gravity::VERTICAL_GRAVITY_MASK;
    const int minorGravity = gravity & Gravity::RELATIVE_HORIZONTAL_GRAVITY_MASK;

    int childTop;
    switch (majorGravity) {
        case Gravity::BOTTOM:
            // totalLength contains the padding already
            childTop = mPaddingTop + bottom - top - totalLength;
            break;

        // totalLength contains the padding already
        case Gravity::CENTER_VERTICAL:
            childTop = mPaddingTop + (bottom - top - totalLength) / 2;
            break;

        case Gravity::TOP:
        default:
            childTop = mPaddingTop;
            break;
    }

    Drawable* dividerDrawable = getDividerDrawable();
    const int dividerHeight = dividerDrawable == nullptr ?
            0 : dividerDrawable->getIntrinsicHeight();

    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        if (child != nullptr && child->getVisibility() != View::GONE) {
            const int childWidth = child->getMeasuredWidth();
            const int childHeight = child->getMeasuredHeight();

            const LinearLayout::LayoutParams* lp =
                    (const LinearLayout::LayoutParams*)child->getLayoutParams();

            int layoutGravity = lp->gravity;
            if (layoutGravity < 0) {
                layoutGravity = minorGravity;
            }
            const int layoutDirection = getLayoutDirection();
            const int absoluteGravity = Gravity::getAbsoluteGravity(
                    layoutGravity, layoutDirection);

            int childLeft;
            switch (absoluteGravity & Gravity::HORIZONTAL_GRAVITY_MASK) {
                case Gravity::CENTER_HORIZONTAL:
                    childLeft = paddingLeft + ((childSpace - childWidth) / 2)
                            + lp->leftMargin - lp->rightMargin;
                    break;

                case Gravity::RIGHT:
                    childLeft = childRight - childWidth - lp->rightMargin;
                    break;

                case Gravity::LEFT:
                default:
                    childLeft = paddingLeft + lp->leftMargin;
                    break;
            }

            if (hasDividerBeforeChildAt(i)) {
                childTop += dividerHeight;
            }

            childTop += lp->topMargin;
            setChildFrame(child, childLeft, childTop, childWidth, childHeight);
            childTop += childHeight + lp->bottomMargin;
        }
    }
}

void AlertDialogLayout::setChildFrame(View* child,int left,int top,int width,int height){
    // CDROID View::layout takes (left, top, width, height), unlike AOSP's
    // (left, top, right, bottom) — pass sizes, not edges.
    child->layout(left, top, width, height);
}
};//endof namespace
