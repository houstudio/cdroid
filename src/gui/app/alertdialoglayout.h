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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  0211-1301  USA
 *********************************************************************************/
#ifndef __CDROID_APP_ALERTDIALOGLAYOUT_H__
#define __CDROID_APP_ALERTDIALOGLAYOUT_H__
#include <widget/linearlayout.h>

namespace cdroid{
/** Special implementation of linear layout that's capable of laying out alert
 *  dialog components.
 *
 *  A dialog consists of up to three panels. All panels are optional, and a
 *  dialog may contain only a single panel. The panels are laid out according
 *  to the following guidelines:
 *  <ul>
 *      <li>topPanel: exactly wrap_content</li>
 *      <li>contentPanel OR customPanel: at most fill_parent, first priority
 *      for extra space</li>
 *      <li>buttonPanel: at least minHeight, at most wrap_content, second
 *      priority for extra space</li>
 *  </ul> */
class AlertDialogLayout:public LinearLayout{
private:
    bool tryOnMeasure(int widthMeasureSpec,int heightMeasureSpec);
    void forceUniformWidth(int count,int heightMeasureSpec);
    int  resolveMinimumHeight(View* v);
    void setChildFrame(View* child,int left,int top,int width,int height);
public:
    AlertDialogLayout(Context* context);
    AlertDialogLayout(Context* context,const AttributeSet* attrs);
    // AOSP 4-arg form; LinearLayout has no defStyleRes ctor yet, defStyleAttr only.
    AlertDialogLayout(Context* context,const AttributeSet* attrs,int defStyleAttr,int defStyleRes=0);
protected:
    void onMeasure(int widthMeasureSpec,int heightMeasureSpec)override;
    void onLayout(bool changed,int left,int top,int right,int bottom)override;
};
};//endof namespace
#endif//__CDROID_APP_ALERTDIALOGLAYOUT_H__
