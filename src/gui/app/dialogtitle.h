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
#ifndef __CDROID_APP_DIALOGTITLE_H__
#define __CDROID_APP_DIALOGTITLE_H__
#include <widget/textview.h>

namespace cdroid{
/** Used by dialogs to change the font size and number of lines to try to fit
 *  the text to the available space. */
class DialogTitle:public TextView{
public:
    DialogTitle(Context* context);
    DialogTitle(Context* context,const AttributeSet* attrs);
    // AOSP 4-arg form; TextView has no defStyleRes ctor yet, defStyleAttr only.
    DialogTitle(Context* context,const AttributeSet* attrs,int defStyleAttr,int defStyleRes=0);
protected:
    void onMeasure(int widthMeasureSpec,int heightMeasureSpec)override;
};
};//endof namespace
#endif//__CDROID_APP_DIALOGTITLE_H__
