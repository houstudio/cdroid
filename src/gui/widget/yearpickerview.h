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
#ifndef __YEARPICKERVIEW_H__
#define __YEARPICKERVIEW_H__
#include <core/calendar.h>
#include <widget/listview.h>

namespace cdroid{

class YearPickerView:public ListView{
public:
    DECLARE_UIEVENT(void,OnYearSelectedListener,YearPickerView& view, int year);
private:
    class YearAdapter* mAdapter;
    int mViewSize;
    int mChildSize;
    OnYearSelectedListener mOnYearSelectedListener;
public:
    YearPickerView(int w,int h);
    YearPickerView(Context* context, const AttributeSet& attrs);
    void setOnYearSelectedListener(OnYearSelectedListener listener);
    void setYear(int year);
    void setSelectionCentered(int position);
    void setRange(Calendar& min, Calendar& max);
    int getFirstPositionOffset();
};
    
}//namespace

#endif/*__YEARPICKERVIEW_H__*/
