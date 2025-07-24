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
#ifndef __ABSOLUTELAYOUT_H__
#define __ABSOLUTELAYOUT_H__
#include <view/viewgroup.h>
namespace cdroid{


class AbsoluteLayout:public ViewGroup{
public:
    class LayoutParams:public ViewGroup::LayoutParams{
    public:
        int x;
        int y;
        LayoutParams(int width, int height, int x, int y);
        LayoutParams(Context* c,const AttributeSet& attrs);
        LayoutParams(const ViewGroup::LayoutParams& source);
        LayoutParams(const LayoutParams& source);
    };
protected:
    LayoutParams* generateDefaultLayoutParams()const override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* p)const override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int l, int t,int w, int h)override;
public:
    AbsoluteLayout(int w,int h);
    AbsoluteLayout(Context* context,const AttributeSet& attrs);
    LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;
    std::string getAccessibilityClassName()const override;
};
}
#endif
