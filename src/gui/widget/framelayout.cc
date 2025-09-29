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
#include <widget/framelayout.h>
#include <porting/cdlog.h>

namespace cdroid{

DECLARE_WIDGET(FrameLayout)

FrameLayout::FrameLayout(int w,int h):ViewGroup(w,h){
    mMeasureAllChildren = false;
    mForegroundPaddingLeft= mForegroundPaddingRight = 0;
    mForegroundPaddingTop = mForegroundPaddingBottom= 0;
}

FrameLayout::FrameLayout(Context* context,const AttributeSet& attrs)
    :ViewGroup(context,attrs){
    mMeasureAllChildren = attrs.getBoolean("measureAllChildren",false);
    mForegroundPaddingLeft = mForegroundPaddingRight  = 0;
    mForegroundPaddingTop  = mForegroundPaddingBottom = 0;
}

//@android.view.RemotableViewMethod
void FrameLayout::setForegroundGravity(int foregroundGravity){
    if (getForegroundGravity() != foregroundGravity) {
        ViewGroup::setForegroundGravity(foregroundGravity);
        // calling get* again here because the set above may apply default constraints
        Drawable* foreground = getForeground();
        if (getForegroundGravity() == Gravity::FILL && foreground ) {
            Rect padding;
            if (foreground->getPadding(padding)) {
                mForegroundPaddingLeft = padding.left;
                mForegroundPaddingTop = padding.top;
                mForegroundPaddingRight = padding.width;
                mForegroundPaddingBottom = padding.height;
            }
        } else {
            mForegroundPaddingLeft = 0;
            mForegroundPaddingTop  = 0;
            mForegroundPaddingRight = 0;
            mForegroundPaddingBottom= 0;
        }
        requestLayout();
    }
}

FrameLayout::LayoutParams* FrameLayout::generateDefaultLayoutParams()const {
    return new FrameLayout::LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::MATCH_PARENT);
}

FrameLayout::LayoutParams* FrameLayout::generateLayoutParams(const AttributeSet& attrs)const {
    return new FrameLayout::LayoutParams(getContext(), attrs);
}

bool FrameLayout::checkLayoutParams(const ViewGroup::LayoutParams* p)const{
    return dynamic_cast<const FrameLayout::LayoutParams*>(p);
}

FrameLayout::LayoutParams* FrameLayout::generateLayoutParams(const ViewGroup::LayoutParams* lp)const {
    if (sPreserveMarginParamsInLayoutParamConversion) {
        if (dynamic_cast<const LayoutParams*>(lp)) {
            return new FrameLayout::LayoutParams(*(LayoutParams*) lp);
        } else if (dynamic_cast<const MarginLayoutParams*>(lp)) {
            return new FrameLayout::LayoutParams(*(MarginLayoutParams*)lp);
        }
    }
    return new FrameLayout::LayoutParams(*lp);
}

int FrameLayout::getPaddingLeftWithForeground() {
    return isForegroundInsidePadding() ? std::max(mPaddingLeft, mForegroundPaddingLeft) :
        mPaddingLeft + mForegroundPaddingLeft;
}

int FrameLayout::getPaddingRightWithForeground() {
    return isForegroundInsidePadding() ? std::max(mPaddingRight, mForegroundPaddingRight) :
        mPaddingRight + mForegroundPaddingRight;
}

int FrameLayout::getPaddingTopWithForeground() {
    return isForegroundInsidePadding() ? std::max(mPaddingTop, mForegroundPaddingTop) :
            mPaddingTop + mForegroundPaddingTop;
}

int FrameLayout::getPaddingBottomWithForeground() {
    return isForegroundInsidePadding() ? std::max(mPaddingBottom, mForegroundPaddingBottom) :
        mPaddingBottom + mForegroundPaddingBottom;
}

void FrameLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    int count = getChildCount();

    bool measureMatchParentChildren =MeasureSpec::getMode(widthMeasureSpec) != MeasureSpec::EXACTLY ||
        MeasureSpec::getMode(heightMeasureSpec) != MeasureSpec::EXACTLY;
    mMatchParentChildren.clear();

    int maxHeight = 0;
    int maxWidth = 0;
    int childState = 0;

    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        if (mMeasureAllChildren || child->getVisibility() != GONE) {
            measureChildWithMargins(child, widthMeasureSpec, 0, heightMeasureSpec, 0);
            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
            maxWidth = std::max(maxWidth,child->getMeasuredWidth() + lp->leftMargin + lp->rightMargin);
            maxHeight = std::max(maxHeight,child->getMeasuredHeight() + lp->topMargin + lp->bottomMargin);
            LOGV("%p margin:%d,%d-%d,%d size:%dx%d",child,lp->leftMargin,lp->topMargin,lp->rightMargin,lp->bottomMargin,maxWidth,maxHeight);
            childState = combineMeasuredStates(childState, child->getMeasuredState());
            if (measureMatchParentChildren) {
                if (lp->width == LayoutParams::MATCH_PARENT ||
                    lp->height == LayoutParams::MATCH_PARENT) {
                    mMatchParentChildren.push_back(child);
                }
            }
        }
    }

    // Account for padding too
    maxWidth += getPaddingLeftWithForeground() + getPaddingRightWithForeground();
    maxHeight += getPaddingTopWithForeground() + getPaddingBottomWithForeground();

    // Check against our minimum height and width
    maxHeight= std::max(maxHeight, getSuggestedMinimumHeight());
    maxWidth = std::max(maxWidth, getSuggestedMinimumWidth());

    // Check against our foreground's minimum height and width
    Drawable* drawable = getForeground();
    if (drawable != nullptr) {
        maxHeight = std::max(maxHeight, drawable->getMinimumHeight());
        maxWidth = std::max(maxWidth, drawable->getMinimumWidth());
    }

    setMeasuredDimension(resolveSizeAndState(maxWidth, widthMeasureSpec, childState),
                resolveSizeAndState(maxHeight, heightMeasureSpec,childState << MEASURED_HEIGHT_STATE_SHIFT));

    count = mMatchParentChildren.size();
    for (int i = 0; i < count; i++) {
        View* child = mMatchParentChildren.at(i);
        MarginLayoutParams* lp = (MarginLayoutParams*) child->getLayoutParams();

        int childWidthMeasureSpec;
        if (lp->width == LayoutParams::MATCH_PARENT) {
            const int width = std::max(0, getMeasuredWidth()
                        - getPaddingLeftWithForeground() - getPaddingRightWithForeground()
                        - lp->leftMargin - lp->rightMargin);
            childWidthMeasureSpec = MeasureSpec::makeMeasureSpec(width, MeasureSpec::EXACTLY);
        } else {
            childWidthMeasureSpec = getChildMeasureSpec(widthMeasureSpec,
                        getPaddingLeftWithForeground() + getPaddingRightWithForeground() +
                        lp->leftMargin + lp->rightMargin,lp->width);
        }

        int childHeightMeasureSpec;
        if (lp->height == LayoutParams::MATCH_PARENT) {
            const int height = std::max(0, getMeasuredHeight()
                        - getPaddingTopWithForeground() - getPaddingBottomWithForeground()
                        - lp->topMargin - lp->bottomMargin);
            childHeightMeasureSpec = MeasureSpec::makeMeasureSpec(height, MeasureSpec::EXACTLY);
        } else {
            childHeightMeasureSpec = getChildMeasureSpec(heightMeasureSpec,
                        getPaddingTopWithForeground() + getPaddingBottomWithForeground() +
                        lp->topMargin + lp->bottomMargin,lp->height);
        }

        child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
    }
}

void FrameLayout::onLayout(bool changed, int left, int top, int width, int height) {
    LOGV("FrameLayout::onLayout %d,%d,%d,%d",left,top,width,height);
    layoutChildren(left, top, width, height, false /* no force left gravity */);
}

void FrameLayout::layoutChildren(int left, int top, int width, int height, bool forceLeftGravity) {
    const int count = getChildCount();

    const int parentLeft = getPaddingLeftWithForeground();
    const int parentRight= width - getPaddingRightWithForeground();

    const int parentTop  = getPaddingTopWithForeground();
    const int parentBottom =height - getPaddingBottomWithForeground();

    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        if (child->getVisibility() != GONE) {
            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();

            int childWidth = child->getMeasuredWidth();
            int childHeight = child->getMeasuredHeight();

            int childLeft=0;
            int childTop =0;

            int gravity = lp->gravity;
            if (gravity == -1) {
                gravity = DEFAULT_CHILD_GRAVITY;
            }

            int layoutDirection = getLayoutDirection();
            int absoluteGravity = Gravity::getAbsoluteGravity(gravity, layoutDirection);
            int verticalGravity = gravity & Gravity::VERTICAL_GRAVITY_MASK;

            switch (absoluteGravity & Gravity::HORIZONTAL_GRAVITY_MASK) {
            case Gravity::CENTER_HORIZONTAL:
                childLeft = parentLeft + (parentRight - parentLeft - childWidth) / 2 +
                    lp->leftMargin - lp->rightMargin;
                break;
            case Gravity::RIGHT:
                if (!forceLeftGravity) {
                    childLeft = parentRight - childWidth - lp->rightMargin;
                    break;
                }
            case Gravity::LEFT:
            default: childLeft = parentLeft + lp->leftMargin;
            }

            switch (verticalGravity) {
            case Gravity::TOP:
                childTop = parentTop + lp->topMargin;
                break;
            case Gravity::CENTER_VERTICAL:
                childTop = parentTop + (parentBottom - parentTop - childHeight) / 2 +
                    lp->topMargin - lp->bottomMargin;
                break;
            case Gravity::BOTTOM:
                childTop = parentBottom - childHeight - lp->bottomMargin;
                break;
            default:  childTop = parentTop + lp->topMargin;
            }

            LOGV("child %p marin: %d,%d,%d,%d bounds:%d,%d-%d,%d",child,lp->leftMargin,lp->topMargin ,
                        lp->rightMargin,lp->bottomMargin,childLeft, childTop, childWidth,childHeight);
            child->layout(childLeft, childTop, childWidth,childHeight);
        }
    }
}

void FrameLayout::setMeasureAllChildren(bool measureAll) {
    mMeasureAllChildren = measureAll;
}

bool FrameLayout::getMeasureAllChildren()const {
    return mMeasureAllChildren;
}

std::string FrameLayout::getAccessibilityClassName()const{
    return "FrameLayout";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FrameLayout::LayoutParams::LayoutParams(Context* c,const AttributeSet& attrs)
    :MarginLayoutParams(c,attrs){
    gravity=attrs.getGravity("layout_gravity",UNSPECIFIED_GRAVITY);
}

FrameLayout::LayoutParams::LayoutParams(int width, int height)
    :LayoutParams(width,height,UNSPECIFIED_GRAVITY){
}

FrameLayout::LayoutParams::LayoutParams(int width, int height, int gravity)
    :MarginLayoutParams(width,height){
    this->gravity=gravity;
}

FrameLayout::LayoutParams::LayoutParams(const ViewGroup::LayoutParams&source)
   :MarginLayoutParams(source){
    gravity= UNSPECIFIED_GRAVITY;
}

FrameLayout::LayoutParams::LayoutParams(const MarginLayoutParams& source)
    :MarginLayoutParams(source){
    gravity= UNSPECIFIED_GRAVITY;
}

FrameLayout::LayoutParams::LayoutParams(const FrameLayout::LayoutParams& source)
    :MarginLayoutParams(source){
    this->gravity = source.gravity;
}

}
