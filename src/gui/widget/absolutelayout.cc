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
#include <widget/absolutelayout.h>
namespace cdroid{

DECLARE_WIDGET(AbsoluteLayout)

AbsoluteLayout::LayoutParams::LayoutParams(int width, int height, int x, int y)
    :ViewGroup::LayoutParams(width,height){
    this->x=x;
    this->y=y;
}

AbsoluteLayout::LayoutParams::LayoutParams(Context* c,const AttributeSet& attrs)
    :ViewGroup::LayoutParams(c,attrs){
    x=attrs.getDimensionPixelOffset("layout_x");
    y=attrs.getDimensionPixelOffset("layout_y");
}

AbsoluteLayout::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& source)
    :ViewGroup::LayoutParams(source){
    x=0;
    y=0;
}
AbsoluteLayout::LayoutParams::LayoutParams(const LayoutParams& source)
    :ViewGroup::LayoutParams(source){
    x=source.x;
    y=source.y;
}

////////////////////////////////////////////////////////////////////////////////////////////

AbsoluteLayout::AbsoluteLayout(int w,int h):ViewGroup(w,h){
}

AbsoluteLayout::AbsoluteLayout(Context* context,const AttributeSet& attrs)
    :ViewGroup(context,attrs){
}

void AbsoluteLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    int count = getChildCount();

    int maxHeight = 0;
    int maxWidth = 0;

    // Find out how big everyone wants to be
    measureChildren(widthMeasureSpec, heightMeasureSpec);

    // Find rightmost and bottom-most child
    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        if (child->getVisibility() != GONE) {
            int childRight;
            int childBottom;

            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();

            childRight = lp->x + child->getMeasuredWidth();
            childBottom= lp->y + child->getMeasuredHeight();

            maxWidth = std::max(maxWidth, childRight);
            maxHeight= std::max(maxHeight, childBottom);
        }
    }

    // Account for padding too
    maxWidth += mPaddingLeft + mPaddingRight;
    maxHeight += mPaddingTop + mPaddingBottom;

    // Check against minimum height and width
    maxHeight= std::max(maxHeight, getSuggestedMinimumHeight());
    maxWidth = std::max(maxWidth, getSuggestedMinimumWidth());
        
    setMeasuredDimension(resolveSizeAndState(maxWidth, widthMeasureSpec, 0),
        resolveSizeAndState(maxHeight, heightMeasureSpec, 0));
}

AbsoluteLayout::LayoutParams* AbsoluteLayout::generateDefaultLayoutParams()const{
    return new AbsoluteLayout::LayoutParams(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT, 0, 0);
}

AbsoluteLayout::LayoutParams* AbsoluteLayout::generateLayoutParams(const AttributeSet& attrs)const {
    return new AbsoluteLayout::LayoutParams(getContext(),attrs);
}

bool AbsoluteLayout::checkLayoutParams(const ViewGroup::LayoutParams* p)const {
    return dynamic_cast<const AbsoluteLayout::LayoutParams*>(p);
}

AbsoluteLayout::LayoutParams* AbsoluteLayout::generateLayoutParams(const ViewGroup::LayoutParams* p)const {
    return new AbsoluteLayout::LayoutParams(*p);
}

void AbsoluteLayout::onLayout(bool changed, int l, int t,int w, int h){
    int count = getChildCount();

    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        if (child->getVisibility() != GONE) {
            LayoutParams* lp =(LayoutParams*) child->getLayoutParams();

            int childLeft = mPaddingLeft + lp->x;
            int childTop = mPaddingTop + lp->y;
            child->layout(childLeft, childTop,child->getMeasuredWidth(),child->getMeasuredHeight());
        }
    }
}

std::string AbsoluteLayout::getAccessibilityClassName()const{
    return "AbsoluteLayout";
}
}
