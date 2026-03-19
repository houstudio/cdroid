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
#ifndef __SCROLLBAR_UTILS_H__
#define __SCROLLBAR_UTILS_H__
namespace cdroid{
class ScrollBarUtils {
public:
    static int getThumbLength(int size, int thickness, int extent, int range) {
        // Avoid the tiny thumb.
        const int minLength = thickness * 2;
        int length = std::round((float) size * extent / range);
        if (length < minLength) {
            length = minLength;
        }
        return length;
    }

    static int getThumbOffset(int size, int thumbLength, int extent, int range, int offset) {
        // Avoid the too-big thumb.
        int thumbOffset = std::round((float) (size - thumbLength) * offset / (range - extent));
        if (thumbOffset > size - thumbLength) {
            thumbOffset = size - thumbLength;
        }
        return thumbOffset;
    }
};
}/*endof namespace*/
#endif
