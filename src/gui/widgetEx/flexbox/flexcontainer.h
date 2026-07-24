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
#ifndef __FLEX_CONTAINER_H__
#define __FLEX_CONTAINER_H__
#include <view/view.h>
#include <widgetEx/flexbox/flexline.h>
namespace cdroid{
class FlexContainer {
public:
    static constexpr int NOT_SET = -1;
    virtual ~FlexContainer()=default;
    virtual int getFlexItemCount()const=0;
    virtual View* getFlexItemAt(int index)const=0;
    virtual View* getReorderedFlexItemAt(int index)const=0;

    virtual void addView(View* view)=0;
    virtual void addView(View* view, int index)=0;
    virtual void removeAllViews()=0;
    virtual void removeViewAt(int index)=0;

    virtual int getFlexDirection()const=0;
    virtual void setFlexDirection( int flexDirection)=0;

    virtual int getFlexWrap()const=0;
    virtual void setFlexWrap(int flexWrap)=0;

    virtual int getJustifyContent()const=0;
    virtual void setJustifyContent(int justifyContent)=0;

    virtual int getAlignContent()const=0;
    virtual void setAlignContent(int alignContent)=0;
    virtual int getAlignItems()const=0;

    virtual void setAlignItems(int alignItems)=0;

    virtual std::vector<FlexLine> getFlexLines()const=0;

    virtual bool isMainAxisDirectionHorizontal() const=0;

    virtual int getDecorationLengthMainAxis(View* view, int index, int indexInFlexLine)const=0;
    virtual int getDecorationLengthCrossAxis(View* view)const=0;

    virtual int getPaddingTop()=0;
    virtual int getPaddingLeft()=0;
    virtual int getPaddingRight()=0;
    virtual int getPaddingBottom()=0;
    virtual int getPaddingStart()=0;
    virtual int getPaddingEnd()=0;

    virtual int getChildWidthMeasureSpec(int widthSpec, int padding, int childDimension)const=0;
    virtual int getChildHeightMeasureSpec(int heightSpec, int padding, int childDimension)const=0;

    virtual int getLargestMainSize()const=0;

    virtual int getSumOfCrossSize()const=0;

    virtual void onNewFlexItemAdded(View* view, int index, int indexInFlexLine, FlexLine& flexLine)=0;
    virtual void onNewFlexLineAdded(FlexLine& flexLine)=0;
    virtual void setFlexLines(const std::vector<FlexLine>& flexLines)=0;

    virtual int getMaxLine()const=0;
    virtual void setMaxLine(int maxLine)=0;

    virtual std::vector<FlexLine>& getFlexLinesInternal()=0;
    virtual void updateViewCache(int position, View* view)=0;
};
}/*endof namespace*/
#endif/*__FLEX_CONTAINER_H__*/
