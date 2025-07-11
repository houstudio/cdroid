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
#ifndef __RTLSPACINGHELPER_H__
#define __RTLSPACINGHELPER_H__
#include <limits.h>
namespace cdroid{
class RtlSpacingHelper{
public:
    static constexpr int UNDEFINED = INT_MIN;
private:
    int mLeft = 0;
    int mRight = 0;
    int mStart = UNDEFINED;
    int mEnd = UNDEFINED;
    int mExplicitLeft = 0;
    int mExplicitRight = 0;

    bool mIsRtl = false;
    bool mIsRelative = false;
public:
    int getLeft()const;
    int getRight()const;
    int getStart()const;
    int getEnd()const;
    void setRelative(int start, int end);
    void setAbsolute(int left, int right);
    void setDirection(bool isRtl);
};
}
#endif
