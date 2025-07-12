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
#ifndef __GESTURE_POINT_H__
#define __GESTURE_POINT_H__
#include <gesture/gesturestore.h>
namespace cdroid{
class GesturePoint {
public:
    float x;
    float y;
    int64_t timestamp;
public:
    GesturePoint(float x, float y, int64_t t) {
        this->x = x;
        this->y = y;
        timestamp = t;
    }
    static GesturePoint deserialize(std::istream& in) {
        // Read X and Y
        const float x = GestureIOHelper::readFloat(in);
        const float y = GestureIOHelper::readFloat(in);
        // Read timestamp
        const int64_t timeStamp = GestureIOHelper::readLong(in);
        return GesturePoint{x, y, timeStamp};
    }
};
}/*endof namespace*/
#endif/*__GESTURE_POINT_H__*/
