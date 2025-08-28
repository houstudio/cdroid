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
#ifndef __DROPDOWN_LISTVIEW_H__
#define __DROPDOWN_LISTVIEW_H__
#include <widget/listview.h>
#include <widget/autoscrollhelper.h>
namespace cdroid{

class DropDownListView:public ListView{
private:
    bool mListSelectionHidden;
    bool mHijackFocus;
    bool mDrawsInPressedState;
    AbsListViewAutoScroller* mScrollHelper;
    Runnable mResolveHoverRunnable;
    void clearPressedItem();
    void setPressedItem(View* child, int position, float x, float y);
protected:
    void drawableStateChanged()override;
    bool touchModeDrawsInPressedState()const override;
    View* obtainView(int position, bool* isScrap)override;
public:
    DropDownListView(Context*,bool hijackfocus);
    ~DropDownListView()override;
    bool shouldShowSelector()const override;
    bool onTouchEvent(MotionEvent& ev)override;
    bool onHoverEvent(MotionEvent& ev)override;
    bool onForwardedEvent(MotionEvent& event, int activePointerId);
    void setListSelectionHidden(bool hideListSelection);
    bool isInTouchMode()const override;
    bool hasWindowFocus()const override;
    bool isFocused()const override;
    bool hasFocus()const override;
};

}//endof namespace
#endif
