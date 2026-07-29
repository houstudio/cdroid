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

/*
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.Rectangle.
 * Header-only: pure value type with inline accessors.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_RECTANGLE_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_RECTANGLE_H

namespace cdroid {

/** Simple rect class */
class Rectangle {
  public:
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    void setBounds(int x, int y, int width, int height) {
        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;
    }

    void grow(int w, int h) {
        x -= w;
        y -= h;
        width += 2 * w;
        height += 2 * h;
    }

    bool intersects(const Rectangle& bounds) const {
        return x >= bounds.x && x < bounds.x + bounds.width
               && y >= bounds.y && y < bounds.y + bounds.height;
    }

    bool contains(int px, int py) const {
        return px >= x && px < x + width
               && py >= y && py < y + height;
    }

    int getCenterX() const {
        return (x + width) / 2;
    }
    int getCenterY() const {
        return (y + height) / 2;
    }
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_RECTANGLE_H
