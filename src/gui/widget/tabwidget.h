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
#ifndef __TABWIDGET_H__
#define __TABWIDGET_H__
#include <widget/linearlayout.h>

namespace cdroid{

class TabWidget:public LinearLayout{
public:
    DECLARE_UIEVENT(void,OnTabSelectionChanged,int,bool);
private:
    Rect mBounds;
    OnTabSelectionChanged mSelectionChangedListener;
    int mSelectedTab = -1;

    Drawable* mLeftStrip;
    Drawable* mRightStrip;

    bool mDrawBottomStrips = true;
    bool mStripMoved;

    // When positive, the widths and heights of tabs will be imposed so that
    // they fit in parent.
    int mImposedTabsHeight = -1;
    std::vector<int> mImposedTabWidths;
private:
    void initTab();
    void onClickChild(View&v,int idx);
protected:
    void onSizeChanged(int w, int h, int oldw, int oldh)override;
    int getChildDrawingOrder(int childCount, int i)override;
    void measureChildBeforeLayout(View* child, int childIndex, int widthMeasureSpec, int totalWidth,
          int heightMeasureSpec, int totalHeight)override;
    void measureHorizontal(int widthMeasureSpec, int heightMeasureSpec)override;
public:
    TabWidget(int w,int h);
    TabWidget(Context*ctx,const AttributeSet&atts);
    ~TabWidget()override;
    View*getChildTabViewAt(int index);
    int getTabCount()const;
    void setDividerDrawable(Drawable* drawable);
    void setDividerDrawable(const std::string&);
    void setLeftStripDrawable(Drawable* drawable);
    void setLeftStripDrawable(const std::string&);
    Drawable*getLeftStripDrawable();
    void setRightStripDrawable(Drawable* drawable);
    void setRightStripDrawable(const std::string& resId);
    Drawable*getRightStripDrawable();
    void setStripEnabled(bool stripEnabled);
    bool isStripEnabled()const;
    void childDrawableStateChanged(View* child)override;
    void dispatchDraw(Canvas& canvas)override;
    void setCurrentTab(int index);
    void focusCurrentTab(int index);
    void setEnabled(bool enabled)override;
    void addView(View* child)override;
    void removeAllViews()override;
    PointerIcon* onResolvePointerIcon(MotionEvent& event, int pointerIndex)override;
    void setTabSelectionListener(const OnTabSelectionChanged& listener);
    void onFocusChange(View* v, bool hasFocus);
};
    
}//namespace 
#endif
