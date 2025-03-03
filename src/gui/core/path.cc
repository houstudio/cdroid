#include <core/path.h>
#include <cairomm/surface.h>
#include <cairomm/context.h>
#include <cairomm/private.h>
#include <porting/cdlog.h>
#include <map>
namespace cdroid{

static Cairo::RefPtr<Cairo::ImageSurface> mPathSurface =Cairo::ImageSurface::create(Cairo::Surface::Format::A8,1,1);
Path::Path(){
    mCTX = Cairo::Context::create(mPathSurface);
}

Path::Path(const Path&o):Path(){
    o.append_to_context(mCTX);
}

Path::cobject* Path::copy_path()const{
    auto cresult = cairo_copy_path_flat(mCTX->cobj());
    return cresult;
}

void Path::reset(){
    mCTX->begin_new_path();
}

bool Path::is_convex()const{
    cairo_path_t *path = cairo_copy_path(mCTX->cobj());
    int i, j, k;
    int num_points = 0;
    double *points = NULL;
    int sign = 0;

    // 统计路径中的点数
    for (i = 0; i < path->num_data; i += path->data[i].header.length) {
        if (path->data[i].header.type == CAIRO_PATH_MOVE_TO ||
            path->data[i].header.type == CAIRO_PATH_LINE_TO) {
            num_points++;
        }
    }

    // 至少需要三个点才能构成多边形
    if (num_points < 3) {
        return false;
    }

    points = (double *)malloc(num_points * 2 * sizeof(double));
    if (points == NULL) {
        return false;
    }

    // 提取路径中的点
    j = 0;
    for (i = 0; i < path->num_data; i += path->data[i].header.length) {
        if (path->data[i].header.type == CAIRO_PATH_MOVE_TO ||
            path->data[i].header.type == CAIRO_PATH_LINE_TO) {
            points[j++] = path->data[i + 1].point.x;
            points[j++] = path->data[i + 1].point.y;
        }
    }

    // 检查多边形的凹凸性
    for (i = 0; i < num_points; i++) {
        j = (i + 1) % num_points;
        k = (i + 2) % num_points;

        double dx1 = points[2 * j] - points[2 * i];
        double dy1 = points[2 * j + 1] - points[2 * i + 1];
        double dx2 = points[2 * k] - points[2 * j];
        double dy2 = points[2 * k + 1] - points[2 * j + 1];

        double cross_product = dx1 * dy2 - dy1 * dx2;

        if (sign == 0) {
            sign = (cross_product > 0) ? 1 : -1;
        } else if ((cross_product > 0 && sign < 0) || (cross_product < 0 && sign > 0)) {
            free(points);
            return false;
        }
    }
    cairo_path_destroy(path);
    free(points);
    return true;
}

void Path::begin_new_sub_path(){
    mCTX->begin_new_sub_path();
}

void Path::close_path(){
    mCTX->close_path();
}

void Path::append_to_context(Cairo::Context*to)const{
    const Cairo::RefPtr<Cairo::Path>from = Cairo::make_refptr_for_instance<Cairo::Path>(mCTX->copy_path());
    to->append_path(*from);
}

void Path::append_to_context(const Cairo::RefPtr<Cairo::Context>&to)const{
    Cairo::RefPtr<Cairo::Path>from = Cairo::make_refptr_for_instance<Cairo::Path>(mCTX->copy_path());
    to->append_path(*from);
}

void Path::set_fill_rule(Cairo::Context::FillRule fill_rule){
    mCTX->set_fill_rule(fill_rule);
}

void Path::move_to(double x,double y){
    mCTX->move_to(x,y);
}

void Path::rel_move_to(double x,double y){
    mCTX->rel_move_to(x,y);
}

void Path::line_to(double x,double y){
    mCTX->line_to(x,y);
}

void Path::rel_line_to(double x,double y){
    mCTX->rel_line_to(x,y);
}

void Path::curve_to(double x1, double y1, double x2, double y2, double x3, double y3){
    mCTX->curve_to(x1,y1,x2,y2,x3,y3);
}

void Path::rel_curve_to(double x1, double y1, double x2, double y2, double x3, double y3){
    mCTX->rel_curve_to(x1,y1,x2,y2,x3,y3);
}

void Path::quad_to(double x1, double y1, double x2, double y2){
    double x0, y0;
    mCTX->get_current_point(x0,y0);

    //Control points for cubic bezier curve
    double cp1x = x0 + 2.f / 3.f * (x1 - x0);
    double cp1y = y0 + 2.f / 3.f * (y1 - y0);
    double cp2x = cp1x + (x2 - x0) / 3.f;
    double cp2y = cp1y + (y2 - y0) / 3.f;

    mCTX->curve_to(cp1x, cp1y, cp2x, cp2y, x2, y2);
}

void Path::rel_quad_to(double dx1, double dy1, double dx2, double dy2){
    double x0, y0;
    mCTX->get_current_point(x0,y0);
    double x1 = x0 + dx1;
    double y1 = y0 + dy1;
    double x2 = x0 + dx2;
    double y2 = y0 + dy2;

    double cp1x = x0 + 2.f / 3.f * (x1 - x0);
    double cp1y = y0 + 2.f / 3.f * (y1 - y0);
    double cp2x = cp1x + (x2 - x0) / 3.f;
    double cp2y = cp1y + (y2 - y0) / 3.f;

    mCTX->curve_to(cp1x, cp1y, cp2x, cp2y, x2, y2);
}

void Path::arc(double xc, double yc, double radius, double angle1, double angle2){
    mCTX->arc(xc,yc,radius,angle1,angle2);
}

void Path::arc_to(double x1, double y1, double x2, double y2, double radius) {
    // Current point
    double x0, y0;
    mCTX->get_current_point(x0, y0);

    // Calculate the angles
    double angle1 = atan2(y1 - y0, x1 - x0);
    double angle2 = atan2(y2 - y1, x2 - x1);

    // Calculate the center of the circle
    double centerX = x1 + radius * cos((angle1 + angle2) / 2);
    double centerY = y1 + radius * sin((angle1 + angle2) / 2);

    // Draw the arc
    mCTX->arc(centerX, centerY, radius, angle1, angle2);
}

void Path::arc_to(double rx, double ry, double angle, bool largeArc, bool sweepFlag, double x, double y) {
    // Current point
    double x0, y0;
    mCTX->get_current_point(x0, y0);

    // Convert angle from degrees to radians
    angle *= M_PI / 180.0;

    // Calculate the middle point of the line from the current point to the end point
    double dx2 = (x0 - x) / 2.0;
    double dy2 = (y0 - y) / 2.0;

    // Calculate the coordinates in the rotated system
    double x1 = cos(angle) * dx2 + sin(angle) * dy2;
    double y1 = -sin(angle) * dx2 + cos(angle) * dy2;

    // Ensure radii are large enough
    double lambda = (x1 * x1) / (rx * rx) + (y1 * y1) / (ry * ry);
    if (lambda > 1) {
        rx *= sqrt(lambda);
        ry *= sqrt(lambda);
    }

    // Calculate the center in the rotated system
    double sign = (largeArc != sweepFlag) ? -1 : 1;
    double sq = ((rx * rx * ry * ry) - (rx * rx * y1 * y1) - (ry * ry * x1 * x1)) / ((rx * rx * y1 * y1) + (ry * ry * x1 * x1));
    sq = (sq < 0) ? 0 : sq;
    double coef = sign * sqrt(sq);
    double cx1 = coef * ((rx * y1) / ry);
    double cy1 = coef * -((ry * x1) / rx);

    // Calculate the center in the original system
    double sx2 = (x0 + x) / 2.0;
    double sy2 = (y0 + y) / 2.0;
    double cx = sx2 + (cos(angle) * cx1 - sin(angle) * cy1);
    double cy = sy2 + (sin(angle) * cx1 + cos(angle) * cy1);

    // Calculate the start and end angles
    double ux = (x1 - cx1) / rx;
    double uy = (y1 - cy1) / ry;
    double vx = (-x1 - cx1) / rx;
    double vy = (-y1 - cy1) / ry;
    double n = sqrt((ux * ux) + (uy * uy));
    double p = ux; // (1 * ux) + (0 * uy)
    sign = (uy < 0) ? -1.0 : 1.0;
    double startAng = sign * acos(p / n); // in radians

    n = sqrt((ux * ux + uy * uy) * (vx * vx + vy * vy));
    p = ux * vx + uy * vy;
    sign = (ux * vy - uy * vx < 0) ? -1.0 : 1.0;
    double sweepAng = sign * acos(p / n); // in radians
    if (!sweepFlag && sweepAng > 0) {
        sweepAng -= 2 * M_PI;
    } else if (sweepFlag && sweepAng < 0) {
        sweepAng += 2 * M_PI;
    }

    // Save the current transformation matrix
    mCTX->save();

    // Translate and scale to match the ellipse
    mCTX->translate(cx, cy);
    mCTX->scale(rx, ry);
    mCTX->rotate(angle);

    // Draw the arc
    if(sweepFlag)
        mCTX->arc_negative(0, 0, 1, startAng, startAng + sweepAng);
    else
        mCTX->arc(0, 0, 1, startAng, startAng + sweepAng);

    // Restore the transformation matrix
    mCTX->restore();
}

void Path::add_oval(int left,int top,int width,int height){
    double center_x = left + width / 2;
    double center_y = top + height / 2;
    double radius_x = width / 2;
    double radius_y = height / 2;

    // 保存当前绘图状态
    mCTX->save();

    // 平移到椭圆的中心位置
    mCTX->translate(center_x, center_y);

    // 缩放坐标系，以绘制椭圆
    mCTX->scale(radius_x, radius_y);

    // 绘制一个单位圆，由于前面进行了缩放，实际上绘制的是椭圆
    mCTX->arc(0.0, 0.0, 1.0, 0.0, 2 * M_PI);
    mCTX->restore();
}

void Path::rectangle(double x, double y, double width, double height){
    mCTX->rectangle(x,y,width,height);
}

void Path::round_rectangle(double x,double y,double width,double height,const std::vector<float>& radii){
    //const RectF rect={x,y,width,height};
    round_rectangle({float(x),float(y),float(width),float(height)},radii);
}

void Path::round_rectangle(const RectF&rect,const std::vector<float>& radii){
    constexpr double circleControlPoint=0.447715;
    move_to(rect.left+radii[0],rect.top);
    line_to(rect.right()-radii[2],rect.top);
    if((radii[2]>0||radii[3])){//topright
         curve_to(rect.right()-radii[2]*circleControlPoint,rect.top,
             rect.right(),rect.top+radii[3]*circleControlPoint,
             rect.right(),rect.top+radii[3]); 
    }
    line_to(rect.right(),rect.bottom()-radii[5]);

    if(radii[4]>0||radii[5]){//bottomright 
         curve_to(rect.right(),rect.bottom()-radii[5]*circleControlPoint,
             rect.right()-radii[4]*circleControlPoint, rect.bottom(),
             rect.right()-radii[4],rect.bottom());
    }
    line_to(rect.left+radii[4],rect.bottom());

    if(radii[6]>0||radii[7]>0){//bottomleft
        curve_to(rect.left+radii[6]*circleControlPoint,rect.bottom(),
            rect.left,rect.bottom()-radii[7]*circleControlPoint,
            rect.left,rect.bottom()-radii[7]);
    }
    line_to(rect.left,rect.top+radii[7]);

    if(radii[0]>0||radii[1]>0){//topleft
        curve_to(rect.left,rect.top+radii[1]*circleControlPoint,
           rect.left+radii[0]*circleControlPoint,rect.top,
           rect.left+radii[0],rect.top);
    }
    mCTX->close_path();//closeSubpath();
}

void Path::append_path(const Path&other){
    Cairo::RefPtr<Cairo::Path>path=Cairo::make_refptr_for_instance<Cairo::Path>(other.mCTX->copy_path());
    mCTX->append_path(*path);
}

void Path::compute_bounds(RectF&bounds, bool include_stroke){
    double x1, y1, x2, y2;
    if (include_stroke) {
        // 计算描边路径的边界
        mCTX->get_stroke_extents(x1, y1, x2, y2);
    } else {
        // 计算填充路径的边界
        mCTX->get_fill_extents(x1, y1, x2, y2);
    }
    LOGD("%s extents: (%.f,%.d,%.f,%.d)",(include_stroke?"Stroke":"Fill"),x1,y1,x2,y2);
    bounds={float(x1),float(y1),float(x2-x1),float(y2-y1)};
}

int Path::fromSVGPathData(const std::string&pathData,std::function<void(int8_t,const std::vector<float>&)>fun){
    return 0;
}

}
