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
#ifndef UI_LIBUI_LINEARLAYOUT_H_
#define UI_LIBUI_LINEARLAYOUT_H_

#include <view/viewgroup.h>

namespace cdroid{

class LinearLayout : public ViewGroup{
public:
    enum{
        HORIZONTAL=0,
        VERTICAL=1,
        SHOW_DIVIDER_NONE=0,
        SHOW_DIVIDER_BEGINNING=1,
        SHOW_DIVIDER_MIDDLE=2,
        SHOW_DIVIDER_END=4
    };
    class LayoutParams:public ViewGroup::MarginLayoutParams{
    public:
        float weight;
        int gravity = -1;
        LayoutParams(Context* c,const AttributeSet&attrs);
        LayoutParams(int width, int height);
        LayoutParams(int width, int height, float weight);
        LayoutParams(const LayoutParams& p);
        LayoutParams(const MarginLayoutParams&source);
        LayoutParams(const ViewGroup::LayoutParams&source);
    };

private:
    static constexpr int VERTICAL_GRAVITY_COUNT = 4;
    static constexpr int INDEX_CENTER_VERTICAL = 0;
    static constexpr int INDEX_TOP = 1;
    static constexpr int INDEX_BOTTOM = 2;
    static constexpr int INDEX_FILL = 3;
private:
    bool mAllowInconsistentMeasurement;
    bool mBaselineAligned;
    int mBaselineAlignedChildIndex;
    int mBaselineChildTop;
    int mOrientation;
    int mGravity;
    int mTotalLength;
    float mWeightSum;
    bool mUseLargestChild;
    std::vector<int>mMaxAscent;
    std::vector<int>mMaxDescent;
    int mDividerWidth;
    int mDividerHeight;
    int mShowDividers;
    int mDividerPadding;
    int mLayoutDirection;
    Drawable*mDivider;
    void initView();
    bool isShowingDividers()const;
    bool allViewsAreGoneBefore(int childIndex);
    bool allViewsAreGoneAfter(int childIndex);
    View* getLastNonGoneChild();
    void forceUniformWidth(int count, int heightMeasureSpec);
    void forceUniformHeight(int count, int widthMeasureSpec);
    void setChildFrame(View* child, int left, int top, int width, int height);
protected:
    virtual int getVirtualChildCount(){return getChildCount();}
    virtual View* getVirtualChildAt(int index){return getChildAt(index);}
    virtual int getChildrenSkipCount(View* child, int index);
    virtual void measureChildBeforeLayout(View* child, int childIndex,int widthMeasureSpec, 
             int totalWidth, int heightMeasureSpec,int totalHeight);
    virtual int getLocationOffset(View* child);
    virtual int getNextLocationOffset(View* child);
    virtual int measureNullChild(int childIndex){return 0;}
    virtual bool hasDividerBeforeChildAt(int childIndex);
    virtual bool hasDividerAfterChildAt(int childIndex);

    LayoutParams* generateDefaultLayoutParams()const override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* lp)const override;

    void onLayout(bool changed, int l, int t, int w, int h)override;
    void layoutVertical(int left, int top, int width, int height);
    void layoutHorizontal(int left, int top, int width, int height);
    virtual void measureHorizontal(int widthMeasureSpec, int heightMeasureSpec);
    virtual void measureVertical(int widthMeasureSpec, int heightMeasureSpec);
    void drawHorizontalDivider(Canvas& canvas, int top);
    void drawVerticalDivider(Canvas& canvas, int left);
    void drawDividersHorizontal(Canvas& canvas);
    void drawDividersVertical(Canvas& canvas);
    void onDraw(Canvas& canvas)override;
public:
    LinearLayout(int w,int h);
    LinearLayout(int x,int y,int w,int h);
    LinearLayout(Context* context,const AttributeSet& attrs);
    ~LinearLayout()override;
    LayoutParams* generateLayoutParams(const AttributeSet&)const override;
    void setShowDividers(int showDividers);
    int getShowDividers()const;
    Drawable*getDividerDrawable();
    void setDividerDrawable(Drawable*divider);
    void setDividerPadding(int padding);
    int getDividerPadding()const;
    int getDividerWidth()const;
    int getGravity()const;
    void setGravity(int gravity);
    void setHorizontalGravity(int horizontalGravity);
    void setVerticalGravity(int verticalGravity);
    virtual void setOrientation(int orientation);
    int getOrientation()const;
    bool shouldDelayChildPressedState()override;
    void setWeightSum(float weightSum);
    float getWeightSum()const;

    int getBaseline()override;
    bool isBaselineAligned()const;
    void setBaselineAligned(bool baselineAligned);
    bool isMeasureWithLargestChildEnabled()const;
    void setMeasureWithLargestChildEnabled(bool enabled);
    std::string getAccessibilityClassName()const override;
    void onRtlPropertiesChanged(int layoutDirection) override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
};

}/*namespace cdroid*/
#endif 
