/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * Port of com.android.internal.widget.DialogViewAnimator (android-36).
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
#include <widget/dialogviewanimator.h>
#include <cdlog.h>

namespace cdroid{

DECLARE_WIDGET2(DialogViewAnimator, "com.android.internal.widget.DialogViewAnimator")

DialogViewAnimator::DialogViewAnimator(Context*ctx)
    :DialogViewAnimator(ctx,nullptr){}

DialogViewAnimator::DialogViewAnimator(Context* context,const AttributeSet* attrs)
  :DialogViewAnimator(context,attrs,0){
}

DialogViewAnimator::DialogViewAnimator(Context* context,const AttributeSet* attrs,int defStyleAttr)
  :ViewAnimator(context,attrs,defStyleAttr){
}

void DialogViewAnimator::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    const bool measureMatchParentChildren =
            MeasureSpec::getMode(widthMeasureSpec) != MeasureSpec::EXACTLY ||
                    MeasureSpec::getMode(heightMeasureSpec) != MeasureSpec::EXACTLY;

    int maxHeight = 0;
    int maxWidth = 0;
    int childState = 0;

    // First measure all children and record maximum dimensions where the
    // spec isn't MATCH_PARENT.
    const int count = getChildCount();
    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        if (getMeasureAllChildren() || child->getVisibility() != GONE) {
            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
            const bool matchWidth  = lp->width  == LayoutParams::MATCH_PARENT;
            const bool matchHeight = lp->height == LayoutParams::MATCH_PARENT;
            if (measureMatchParentChildren && (matchWidth || matchHeight)) {
                mMatchParentChildren.push_back(child);
            }

            measureChildWithMargins(child, widthMeasureSpec, 0, heightMeasureSpec, 0);

            // Measured dimensions only count against the maximum
            // dimensions if they're not MATCH_PARENT.
            int state = 0;

            if (measureMatchParentChildren && !matchWidth) {
                maxWidth = std::max(maxWidth, child->getMeasuredWidth()
                        + lp->leftMargin + lp->rightMargin);
                state |= child->getMeasuredWidthAndState() & MEASURED_STATE_MASK;
            }

            if (measureMatchParentChildren && !matchHeight) {
                maxHeight = std::max(maxHeight, child->getMeasuredHeight()
                        + lp->topMargin + lp->bottomMargin);
                state |= (child->getMeasuredHeightAndState() >> MEASURED_HEIGHT_STATE_SHIFT)
                        & (MEASURED_STATE_MASK >> MEASURED_HEIGHT_STATE_SHIFT);
            }

            childState = combineMeasuredStates(childState, state);
        }
    }

    // Account for padding too.
    maxWidth += getPaddingLeft() + getPaddingRight();
    maxHeight += getPaddingTop() + getPaddingBottom();

    // Check against our minimum height and width.
    maxHeight = std::max(maxHeight, getSuggestedMinimumHeight());
    maxWidth  = std::max(maxWidth, getSuggestedMinimumWidth());

    // Check against our foreground's minimum height and width.
    Drawable* drawable = getForeground();
    if (drawable != nullptr) {
        maxHeight = std::max(maxHeight, drawable->getMinimumHeight());
        maxWidth  = std::max(maxWidth, drawable->getMinimumWidth());
    }

    setMeasuredDimension(resolveSizeAndState(maxWidth, widthMeasureSpec, childState),
            resolveSizeAndState(maxHeight, heightMeasureSpec,
                    childState << MEASURED_HEIGHT_STATE_SHIFT));

    // Measure remaining MATCH_PARENT children again using real dimensions.
    const int matchCount = mMatchParentChildren.size();
    for (int i = 0; i < matchCount; i++) {
        View* child = mMatchParentChildren.at(i);
        MarginLayoutParams* lp = (MarginLayoutParams*) child->getLayoutParams();

        int childWidthMeasureSpec;
        if (lp->width == LayoutParams::MATCH_PARENT) {
            childWidthMeasureSpec = MeasureSpec::makeMeasureSpec(
                    getMeasuredWidth() - getPaddingLeft() - getPaddingRight()
                            - lp->leftMargin - lp->rightMargin,
                    MeasureSpec::EXACTLY);
        } else {
            childWidthMeasureSpec = getChildMeasureSpec(widthMeasureSpec,
                    getPaddingLeft() + getPaddingRight() + lp->leftMargin + lp->rightMargin,
                    lp->width);
        }

        int childHeightMeasureSpec;
        if (lp->height == LayoutParams::MATCH_PARENT) {
            childHeightMeasureSpec = MeasureSpec::makeMeasureSpec(
                    getMeasuredHeight() - getPaddingTop() - getPaddingBottom()
                            - lp->topMargin - lp->bottomMargin,
                    MeasureSpec::EXACTLY);
        } else {
            childHeightMeasureSpec = getChildMeasureSpec(heightMeasureSpec,
                    getPaddingTop() + getPaddingBottom() + lp->topMargin + lp->bottomMargin,
                    lp->height);
        }

        child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
    }

    mMatchParentChildren.clear();
}

}//endofnamespace
