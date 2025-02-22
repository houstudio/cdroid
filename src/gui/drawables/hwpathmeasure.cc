#include <drawables/hwpathmeasure.h>
namespace cdroid {
namespace hw{

// 计算两点间的距离
static double distance(double x1, double y1, double x2, double y2) {
    return std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

// 近似计算二次贝塞尔曲线的长度
static double quadratic_bezier_length(double x0, double y0, double x1, double y1, double x2, double y2, int segments = 10) {
    double length = 0.0;
    double prev_x = x0;
    double prev_y = y0;
    for (int i = 1; i <= segments; ++i) {
        double t = static_cast<double>(i) / segments;
        double one_minus_t = 1.0 - t;
        double x = one_minus_t * one_minus_t * x0 + 2 * one_minus_t * t * x1 + t * t * x2;
        double y = one_minus_t * one_minus_t * y0 + 2 * one_minus_t * t * y1 + t * t * y2;
        length += distance(prev_x, prev_y, x, y);
        prev_x = x;
        prev_y = y;
    }
    return length;
}

// 近似计算三次贝塞尔曲线的长度
static double cubic_bezier_length(double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3, int segments = 10) {
    double length = 0.0;
    double prev_x = x0;
    double prev_y = y0;
    for (int i = 1; i <= segments; ++i) {
        double t = static_cast<double>(i) / segments;
        double one_minus_t = 1.0 - t;
        double x = one_minus_t * one_minus_t * one_minus_t * x0 + 3 * one_minus_t * one_minus_t * t * x1 + 3 * one_minus_t * t * t * x2 + t * t * t * x3;
        double y = one_minus_t * one_minus_t * one_minus_t * y0 + 3 * one_minus_t * one_minus_t * t * y1 + 3 * one_minus_t * t * t * y2 + t * t * t * y3;
        length += distance(prev_x, prev_y, x, y);
        prev_x = x;
        prev_y = y;
    }
    return length;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

PathMeasure::PathMeasure(Cairo::RefPtr<cdroid::Path>inPath,bool){
}

// 计算路径的长度
double PathMeasure::getLength(){
    double length = 0.0;
    double prev_x = 0.0;
    double prev_y = 0.0;
    cairo_path_t* cpath ;//= path->cobj();
    for (int i=0;i<cpath->num_data;i+=i += cpath->data[i].header.length) {
        cairo_path_data_t*e=&cpath->data[i];
        switch (e->header.type) {
            case CAIRO_PATH_MOVE_TO: {
                prev_x = e[0].point.x;
                prev_y = e[0].point.y;
                break;
            }
            case CAIRO_PATH_LINE_TO: {
                double x = e[0].point.x;
                double y = e[0].point.y;
                length += distance(prev_x, prev_y, x, y);
                prev_x = x;
                prev_y = y;
                break;
            }
            case CAIRO_PATH_CURVE_TO: {
                double x1 = e[0].point.x;
                double y1 = e[0].point.y;
                double x2 = e[1].point.x;
                double y2 = e[1].point.y;
                double x3 = e[2].point.x;
                double y3 = e[2].point.y;
                length += cubic_bezier_length(prev_x, prev_y, x1, y1, x2, y2, x3, y3);
                prev_x = x3;
                prev_y = y3;
                break;
            }
            case CAIRO_PATH_CLOSE_PATH: {
                length += distance(prev_x, prev_y, cpath->data[0].point.x, cpath->data[0].point.y);
                break;
            }
            default:
                break;
        }
    }
    return length;
}

bool PathMeasure::getSegment(double startD, double stopD, Cairo::RefPtr<cdroid::Path>& dst, bool startWithMoveTo){
    return 0;
}
}
}
