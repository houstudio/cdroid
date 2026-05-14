/**
 * Copyright (C) 2009 - 2012 SC 4ViewSoft SRL
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __CLICKABLE_AREA_H__
#define __CLICKABLE_AREA_H__
#include <core/rect.h>
namespace cdroid{
class ClickableArea {
private:
    RectF rect;
    double x;
    double y;
    int pointIndex;
public:
    ClickableArea()=default;
    ClickableArea(const ClickableArea&)=default;
    ClickableArea(const RectF& rect, double x, double y,int index) {
        this->rect = rect;
        this->x = x;
        this->y = y;
        pointIndex=index;
    }
    RectF getRect() const{
        return rect;
    }
    double getX() const{
        return x;
    }
    double getY() const{
        return y;
    }
    int getPointIndex()const{
        return pointIndex;
    }
};
}/*endof namespace*/
#endif/*__CLICKABLE_AREA_H__*/
