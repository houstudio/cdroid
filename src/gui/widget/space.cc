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
#include <widget/space.h>

namespace cdroid{

DECLARE_WIDGET(Space)
Space::Space(int w,int h):View(w,h){
    if (getVisibility() == VISIBLE) {
        setVisibility(INVISIBLE);
    }
}

Space::Space(Context*context,const AttributeSet& attrs)
  :View(context,attrs){
    if (getVisibility() == VISIBLE) {
        setVisibility(INVISIBLE);
    }
}

void Space::draw(Canvas& canvas) {
}

int Space::getDefaultSize2(int size, int measureSpec) {
    int result = size;
    int specMode = MeasureSpec::getMode(measureSpec);
    int specSize = MeasureSpec::getSize(measureSpec);

    switch (specMode) {
    case MeasureSpec::UNSPECIFIED:
        result = size;
        break;
    case MeasureSpec::AT_MOST:
        result = std::min(size, specSize);
        break;
    case MeasureSpec::EXACTLY:
        result = specSize;
        break;
    }
    return result;
}
void Space::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    setMeasuredDimension(
            getDefaultSize2(getSuggestedMinimumWidth(), widthMeasureSpec),
            getDefaultSize2(getSuggestedMinimumHeight(), heightMeasureSpec));
}

}
