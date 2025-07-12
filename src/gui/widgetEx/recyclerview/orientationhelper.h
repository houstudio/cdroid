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
#ifndef __ORIENTATION_HELPER_H__
#define __ORIENTATION_HELPER_H__
#include <widgetEx/recyclerview/recyclerview.h>
namespace cdroid{
class OrientationHelper{
private:
    static constexpr int INVALID_SIZE = INT_MIN;
public:
    static constexpr int HORIZONTAL = RecyclerView::HORIZONTAL;
    static constexpr int VERTICAL = RecyclerView::VERTICAL;
protected:
    int mLastTotalSpace;
    Rect mTmpRect;
    RecyclerView::LayoutManager*mLayoutManager;
public:
    OrientationHelper(RecyclerView::LayoutManager* layoutManager);
public:
    virtual ~OrientationHelper()=default;
    RecyclerView::LayoutManager* getLayoutManager();
    void onLayoutComplete();
    int getTotalSpaceChange();
    virtual int getDecoratedStart(View* view)=0;
    virtual int getDecoratedEnd(View* view)=0;
    virtual int getTransformedEndWithDecoration(View* view)=0;
    virtual int getTransformedStartWithDecoration(View* view)=0;
    virtual int getDecoratedMeasurement(View* view)=0;
    virtual int getDecoratedMeasurementInOther(View* view)=0;
    virtual int getStartAfterPadding()=0;
    virtual int getEndAfterPadding()=0;
    virtual int getEnd()=0;
    virtual void offsetChildren(int amount)=0;
    virtual int getTotalSpace()=0;
    virtual void offsetChild(View* view, int offset)=0;
    virtual int getEndPadding()=0;
    virtual int getMode()=0;
    virtual int getModeInOther()=0;

    static OrientationHelper* createOrientationHelper(RecyclerView::LayoutManager* layoutManager,int orientation);
    static OrientationHelper* createHorizontalHelper(RecyclerView::LayoutManager* layoutManager);
    static OrientationHelper* createVerticalHelper(RecyclerView::LayoutManager* layoutManager);
};

}/*endof namespace*/
#endif/*__ORIENTATION_H__*/
