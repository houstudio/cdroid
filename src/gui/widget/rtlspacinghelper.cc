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
#include <widget/rtlspacinghelper.h>
namespace cdroid{

int RtlSpacingHelper::getLeft()const{
    return mLeft;
}

int RtlSpacingHelper::getRight()const{
     return mRight;
}

int RtlSpacingHelper::getStart()const{
    return mIsRtl ? mRight : mLeft;
}

int RtlSpacingHelper::getEnd()const{
    return mIsRtl ? mLeft : mRight;
}

void RtlSpacingHelper::setRelative(int start, int end) {
    mStart = start;
    mEnd = end;
    mIsRelative = true;
    if (mIsRtl) {
        if (end != UNDEFINED) mLeft = end;
        if (start != UNDEFINED) mRight = start;
    } else {
        if (start != UNDEFINED) mLeft = start;
        if (end != UNDEFINED) mRight = end;
    }
}

void RtlSpacingHelper::setAbsolute(int left, int right) {
    mIsRelative = false;
    if (left != UNDEFINED) mLeft = mExplicitLeft = left;
    if (right != UNDEFINED) mRight = mExplicitRight = right;
}

void RtlSpacingHelper::setDirection(bool isRtl) {
    if (isRtl == mIsRtl) {
        return;
    }
    mIsRtl = isRtl;
    if (mIsRelative) {
        if (isRtl) {
            mLeft = mEnd != UNDEFINED ? mEnd : mExplicitLeft;
            mRight = mStart != UNDEFINED ? mStart : mExplicitRight;
        } else {
            mLeft = mStart != UNDEFINED ? mStart : mExplicitLeft;
            mRight = mEnd != UNDEFINED ? mEnd : mExplicitRight;
        }
    } else {
        mLeft = mExplicitLeft;
        mRight = mExplicitRight;
    }
}

}//endof namespace
