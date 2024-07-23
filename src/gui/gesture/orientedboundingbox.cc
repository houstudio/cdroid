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
