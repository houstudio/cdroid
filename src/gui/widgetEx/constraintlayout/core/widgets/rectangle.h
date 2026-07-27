/*
 * Copyright (C) 2016 The Android Open Source Project
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
 *
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
