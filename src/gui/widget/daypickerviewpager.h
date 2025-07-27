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
#ifndef __DAYPICKER_VIEWPAGER_H__
#define __DAYPICKER_VIEWPAGER_H__
#include <widget/viewpager.h>
namespace cdroid{

class DayPickerViewPager:public ViewPager{
private:
    std::vector<View*>mMatchParentChildren;
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
public:
    DayPickerViewPager(Context* context,const AttributeSet& attrs);
    View*findViewByPredicateTraversal(const Predicate<View*>&predicate,View* childToSkip)override;
};
}//endof namespace
#endif
