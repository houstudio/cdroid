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
#include <gesture/orientedboundingbox.h>
namespace cdroid{

OrientedBoundingBox::OrientedBoundingBox(float angle, float cx, float cy, float w, float h) {
    orientation = angle;
    width = w;
    height = h;
    centerX = cx;
    centerY = cy;
    float ratio = w / h;
    if (ratio > 1.f) {
        squareness = 1.f / ratio;
    } else {
        squareness = ratio;
    }
}

static void mapPoints(const Cairo::Matrix& matrix, float points[2]) {
    const double x = points[0] * matrix.xx + points[1] * matrix.xy + matrix.x0;
    const double y = points[0] * matrix.yx + points[1] * matrix.yy + matrix.y0;
    points[0] = x;
    points[1] = y;
}

cdroid::Path OrientedBoundingBox::toPath() {
    Path path;
    float point[2];
    point[0] = -width / 2;
    point[1] = height / 2;
    Cairo::Matrix matrix;
    matrix.translate(-centerX, -centerY);
    matrix.rotate(orientation*M_PI/180.0);
    matrix.translate(centerX, centerY);

    mapPoints(matrix,point);
    path.move_to(point[0], point[1]);

    point[0] = -width / 2;
    point[1] = -height / 2;
    mapPoints(matrix,point);
    path.line_to(point[0], point[1]);

    point[0] = width / 2;
    point[1] = -height / 2;
    mapPoints(matrix,point);
    path.line_to(point[0], point[1]);

    point[0] = width / 2;
    point[1] = height / 2;
    mapPoints(matrix,point);
    path.line_to(point[0], point[1]);

    path.close_path();

    return path;
}
}/*endof namespace*/
