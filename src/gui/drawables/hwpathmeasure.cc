#include <drawables/hwpathmeasure.h>
namespace cdroid {
namespace hw{


PathMeasure::PathMeasure(Cairo::RefPtr<cdroid::Path>inPath,bool){
    mPath=inPath;
}

double PathMeasure::distance(const Point& p1, const Point& p2) {
    return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
}

double PathMeasure::curveLength(const Point& p0, const Point& p1, const Point& p2, const Point& p3) {
    // Approximate the length of a cubic Bezier curve using a simple method
    double length = 0.0;
    Point prev = p0;
    const int steps = 10;
    for (int i = 1; i <= steps; ++i) {
        double t = static_cast<double>(i) / steps;
        Point point = interpolateCurve(p0, p1, p2, p3, t);
        length += distance(prev, point);
        prev = point;
    }
    return length;
}

PathMeasure::Point PathMeasure::interpolate(const Point& p1, const Point& p2, double t) {
    return { p1.x + t * (p2.x - p1.x), p1.y + t * (p2.y - p1.y) };
}

PathMeasure::Point PathMeasure::interpolateCurve(const Point& p0, const Point& p1, const Point& p2, const Point& p3, double t) {
    double u = 1.0 - t;
    double tt = t * t;
    double uu = u * u;
    double uuu = uu * u;
    double ttt = tt * t;

    Point p = {
        uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x,
        uuu * p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y
    };
    return p;
}

double PathMeasure::getLength() {
    Point last_point;
    double length =0;
    auto m_path=mPath->copy_path();
    for (int i = 0; i < m_path->num_data; ) {
        cairo_path_data_t* data = &m_path->data[i];
        switch (data->header.type) {
        case CAIRO_PATH_MOVE_TO:
            last_point = { data[1].point.x, data[1].point.y };
            i += 2;
            break;
        case CAIRO_PATH_LINE_TO: {
            Point current_point = { data[1].point.x, data[1].point.y };
            length += distance(last_point, current_point);
            last_point = current_point;
            i += 2;
            break;
        }
        case CAIRO_PATH_CURVE_TO: {
            Point p1 = { data[1].point.x, data[1].point.y };
            Point p2 = { data[2].point.x, data[2].point.y };
            Point p3 = { data[3].point.x, data[3].point.y };
            length += curveLength(last_point, p1, p2, p3);
            last_point = p3;
            i += 4;
            break;
        }
        case CAIRO_PATH_CLOSE_PATH:
            i += 1;
            break;
        default:
            break;
        }
    }
    cairo_path_destroy(m_path);
    return length;
}

bool PathMeasure::getSegment(double startD, double stopD, Cairo::RefPtr<cdroid::Path>& dst, bool startWithMoveTo) {
    double length = 0.0;
    Point last_point;
    bool segment_started = false;
    auto m_path=mPath->copy_path();
    for (int i = 0; i < m_path->num_data; ) {
        cairo_path_data_t* data = &m_path->data[i];
        switch (data->header.type) {
            case CAIRO_PATH_MOVE_TO:
                last_point = { data[1].point.x, data[1].point.y };
                if (startWithMoveTo && !segment_started && length >= startD) {
                    dst->move_to(last_point.x, last_point.y);
                    segment_started = true;
                }
                i += 2;
                break;
            case CAIRO_PATH_LINE_TO: {
                Point current_point = { data[1].point.x, data[1].point.y };
                double segment_length = distance(last_point, current_point);
                if (length + segment_length >= startD && length <= stopD) {
                    if (!segment_started) {
                        dst->move_to(interpolate(last_point, current_point, (startD - length) / segment_length).x,
                                     interpolate(last_point, current_point, (startD - length) / segment_length).y);
                        segment_started = true;
                    }
                    if (length + segment_length > stopD) {
                        dst->line_to(interpolate(last_point, current_point, (stopD - length) / segment_length).x,
                                     interpolate(last_point, current_point, (stopD - length) / segment_length).y);
                        i=m_path->num_data;//make for goto end
                    } else {
                        dst->line_to(current_point.x, current_point.y);
                    }
                }
                length += segment_length;
                last_point = current_point;
                i += 2;
                break;
            }
            case CAIRO_PATH_CURVE_TO: {
                Point p1 = { data[1].point.x, data[1].point.y };
                Point p2 = { data[2].point.x, data[2].point.y };
                Point p3 = { data[3].point.x, data[3].point.y };
                double segment_length = curveLength(last_point, p1, p2, p3);
                if (length + segment_length >= startD && length <= stopD) {
                    if (!segment_started) {
                        dst->move_to(interpolateCurve(last_point, p1, p2, p3, (startD - length) / segment_length).x,
                                     interpolateCurve(last_point, p1, p2, p3, (startD - length) / segment_length).y);
                        segment_started = true;
                    }
                    if (length + segment_length > stopD) {
                        dst->curve_to(p1.x, p1.y, p2.x, p2.y, interpolateCurve(last_point, p1, p2, p3, (stopD - length) / segment_length).x,
                                      interpolateCurve(last_point, p1, p2, p3, (stopD - length) / segment_length).y);
                        i=m_path->num_data;//make for goto end
                    } else {
                        dst->curve_to(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
                    }
                }
                length += segment_length;
                last_point = p3;
                i += 4;
                break;
            }
            case CAIRO_PATH_CLOSE_PATH:
                i += 1;
                break;
            default:  break;
        }
    }
    cairo_path_destroy(m_path);
    return true;
}

}/**/
}/**/
