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
#ifndef __FRAMELAYOUT_H__
#define __FRAMELAYOUT_H__
#include <view/viewgroup.h>
namespace cdroid{

class FrameLayout:public ViewGroup{
private:
    static constexpr int DEFAULT_CHILD_GRAVITY = (Gravity::TOP | Gravity::START);
public:
    class LayoutParams:public MarginLayoutParams{
    public:
        enum{
            UNSPECIFIED_GRAVITY=-1
        };
        int gravity;
        LayoutParams(Context* c,const AttributeSet& attrs);
        LayoutParams(int width, int height);
        LayoutParams(int width, int height, int gravity);
        LayoutParams(const ViewGroup::LayoutParams&);
        LayoutParams(const MarginLayoutParams& source) ;
        LayoutParams(const LayoutParams& source);
    };
private:
    bool mMeasureAllChildren;
    int mForegroundPaddingLeft;
    int mForegroundPaddingTop;
    int mForegroundPaddingRight;
    int mForegroundPaddingBottom;
    std::vector<View*>mMatchParentChildren;
protected:
    int getPaddingLeftWithForeground();
    int getPaddingRightWithForeground();
    int getPaddingTopWithForeground();
    int getPaddingBottomWithForeground();

    LayoutParams* generateDefaultLayoutParams()const override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* lp)const override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int left, int top, int width, int height)override;
    void layoutChildren(int left, int top, int width, int height, bool forceLeftGravity);

public:
    FrameLayout(int w,int h);
    FrameLayout(Context* context,const AttributeSet& attrs);
    void setForegroundGravity(int foregroundGravity);
    void setMeasureAllChildren(bool measureAll);
    bool getMeasureAllChildren()const;
    LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;
    std::string getAccessibilityClassName()const override;
};
}//namespace
#endif
