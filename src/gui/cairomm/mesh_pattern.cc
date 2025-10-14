/* Copyright (C) 2025 The cairomm Development Team
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
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <cairomm/mesh_pattern.h>
#include <cairomm/private.h>

namespace Cairo
{
MeshPattern::MeshPattern(cairo_pattern_t* cobject, bool has_reference) :
    Pattern(cobject, has_reference)
{}

MeshPattern::~MeshPattern()
{
  // pattern is destroyed in base class
}

void MeshPattern::begin_patch()
{
  cairo_mesh_pattern_begin_patch(m_cobject);
  check_object_status_and_throw_exception(*this);
}

void MeshPattern::end_patch()
{
  cairo_mesh_pattern_end_patch(m_cobject);
  check_object_status_and_throw_exception(*this);
}


void MeshPattern::move_to(double x, double y)
{
  cairo_mesh_pattern_move_to(m_cobject, x, y);
  check_object_status_and_throw_exception(*this);
}

void MeshPattern::line_to(double x, double y)
{
  cairo_mesh_pattern_line_to(m_cobject, x, y);
  check_object_status_and_throw_exception(*this);
}

void MeshPattern::curve_to(double x1, double y1, double x2, double y2,
                           double x3, double y3)
{
  cairo_mesh_pattern_curve_to(m_cobject, x1, y1, x2, y2, x3, y3);
  check_object_status_and_throw_exception(*this);
}

void MeshPattern::set_control_point(unsigned int point_num, double x, double y)
{
  cairo_mesh_pattern_set_control_point(m_cobject, point_num, x, y);
  check_object_status_and_throw_exception(*this);
}


void MeshPattern::set_corner_color_rgb(unsigned int corner_num, double red,
                                       double green, double blue)
{
  cairo_mesh_pattern_set_corner_color_rgb(m_cobject, corner_num, red, green,
                                          blue);
  check_object_status_and_throw_exception(*this);
}

void MeshPattern::set_corner_color_rgba(unsigned int corner_num, double red,
                                        double green, double blue, double alpha)
{
  cairo_mesh_pattern_set_corner_color_rgba(m_cobject, corner_num, red, green,
                                           blue, alpha);
  check_object_status_and_throw_exception(*this);
}

unsigned int MeshPattern::get_patch_count() const
{
  unsigned int res;
  cairo_mesh_pattern_get_patch_count(m_cobject, &res);
  return res;
}

RefPtr<Path> MeshPattern::get_path(unsigned int patch_num) const
{
  cairo_path_t* cpath = cairo_mesh_pattern_get_path(m_cobject, patch_num);
  auto cpp_path = make_refptr_for_instance<Path>(new Path(cpath, true /* take ownership */));
  // If an exception is thrown, cpp_path's destructor will call ~Path(),
  // which will destroy cpath.
  check_object_status_and_throw_exception(*cpp_path);
  return cpp_path;
}

void MeshPattern::get_control_point(unsigned int patch_num, unsigned int point_num,
                                    double& x, double& y)
{
  cairo_mesh_pattern_get_control_point(m_cobject, patch_num, point_num, &x, &y);
  check_object_status_and_throw_exception(*this);
}

void MeshPattern::get_corner_color_rgba(unsigned int patch_num, unsigned int corner_num,
                                        double& red, double& green, double& blue,
                                        double& alpha)
{
  cairo_mesh_pattern_get_corner_color_rgba(m_cobject, patch_num, corner_num,
                                           &red, &green, &blue, &alpha);
  check_object_status_and_throw_exception(*this);
}

RefPtr<MeshPattern> MeshPattern::create()
{
  cairo_pattern_t* cobject = cairo_pattern_create_mesh();
  auto cpp_object = make_refptr_for_instance<MeshPattern>(new MeshPattern(cobject, true /* has reference */));
  // If an exception is thrown, cpp_object's destructor will call ~MeshPattern(),
  // which will destroy cobject.
  check_object_status_and_throw_exception(*cpp_object);
  return cpp_object;
}

///////////////////////////////////////////////////////////////////////////////////////////
#define blend(a,b,c) (a+(b-a)*c)
#define blendWithoutPremultiply blend
SweepGradient::SweepGradient(double cx,double cy,double r,double angleRadians,const std::vector<ColorStop>&stopColors)
  :MeshPattern(cairo_pattern_create_mesh()){
    std::vector<ColorStop>stops = stopColors;
    ColorStop front = stops.front();
    ColorStop back  = stops.back();
    auto interpolatedStop = [&] (double fraction) -> ColorStop {
            //auto offset = blend(front.offset, back.offset, fraction);
            auto r = blendWithoutPremultiply(front.red, back.red, fraction);
            auto g = blendWithoutPremultiply(front.green, back.green, fraction);
            auto b = blendWithoutPremultiply(front.blue, back.blue, fraction);
            auto a = blendWithoutPremultiply(front.alpha, back.alpha, fraction);
            return { fraction, r,g,b,a };
        };
    if (stops.size() == 1){
        back.alpha=front.alpha!=0.f?0.f:1.f;
        stops = { front, back };
    }
    // It's not possible to paint an entire circle with a single Bezier curve.
    // To have a good approximation to a circle it's necessary to use at least four Bezier curves.
    // So add three additional interpolated stops, allowing for four Bezier curves.
    if (stops.size() == 2) {
        // The first two checks avoid degenerated interpolations. These interpolations
        // may cause Cairo to enter really slow operations with huge bezier parameters.
        if (front.offset == 1.0) {
            back.offset = 0.f;
            for(int i=0;i<3;i++)stops.push_back(front);
            for(int i=0;i<4;i++)stops.at(i).offset=.25f*i;
        } else if (back.offset == 0.0) {
            front.offset = 1.f;
            for(int i=0;i<3;i++)stops.push_back(back);
        } else {
            for(int i=0;i<3;i++)stops.push_back(stops.back());
        }
        for(int i=0;i<5;i++){
            ColorStop& c=stops.at(i);
            c = interpolatedStop(.25f*i);
        }
    }else{//3 colorstops
        for(int i=0;i<stops.size();i++){
           ColorStop& c=stops.at(i);
           c.offset=float(i)/(stops.size()-1);
        }
    }

    auto first=stops.front();
    auto last =stops.back();
    if (first.offset > 0.0f)
        stops.insert(stops.begin(), { 0.0f, first.red,first.green,first.blue,first.alpha });
    if (last.offset < 1.0f)
        stops.push_back({ 1.0f, last.red,last.green,last.blue,last.alpha });

    //auto gradient = adoptRef(cairo_pattern_create_mesh());
    for (size_t i = 0; i < stops.size() - 1; i++)
        add_sector(cx, cy, r, angleRadians, stops[i], stops[i + 1]);//, globalAlpha);*/
}

SweepGradient::SweepGradient(cairo_pattern_t* cobject, bool has_reference)
 :MeshPattern(cobject, has_reference){
}

RefPtr<SweepGradient>SweepGradient::create(double cx,double cy,double r,double angleRadians,const std::vector<ColorStop>&stopColors){
    return make_refptr_for_instance<SweepGradient>(new SweepGradient(cx,cy,r,angleRadians,stopColors));
}

static constexpr double deg0 = 0;
static constexpr double deg90 = M_PI / 2;
static constexpr double deg180 = M_PI;
static constexpr double deg270 = 3 * M_PI / 2;
static constexpr double deg360 = 2 * M_PI;

static double normalizeAngle(double angle){
    double tmp = std::fmod(angle, deg360);
    if (tmp < 0)
        tmp += deg360;
    return tmp;
}

void SweepGradient::add_sector(double cx,double cy,double r,double angleRadians,const ColorStop& from,const ColorStop&to){
    const double angOffset = 0.25; // 90 degrees.

    // Substract 90 degrees so angles start from top left.
    // Convert to radians and add angleRadians offset.
    double angleStart = ((from.offset - angOffset) * 2 * M_PI) + angleRadians;
    double angleEnd = ((to.offset - angOffset) * 2 * M_PI) + angleRadians;

    // Calculate center offset depending on quadrant.
    //
    // All sections belonging to the same quadrant share a common center. As we move
    // along the circle, sections belonging to a new quadrant will have a different
    // center. If all sections had the same center, the center will get overridden as
    // the sections get painted.
    double cxOffset, cyOffset;
    auto actualAngleStart = normalizeAngle(angleStart);
    if (actualAngleStart >= deg0 && actualAngleStart < deg90) {
        cxOffset = 0;
        cyOffset = 0;
    } else if (actualAngleStart >= deg90 && actualAngleStart < deg180) {
        cxOffset = -1;
        cyOffset = 0;
    } else if (actualAngleStart >= deg180 && actualAngleStart < deg270) {
        cxOffset = -1;
        cyOffset = -1;
    } else if (actualAngleStart >= deg270 && actualAngleStart < deg360) {
        cxOffset = 0;
        cyOffset = -1;
    } else {
        cxOffset = 0;
        cyOffset = 0;
    }
    // The center offset for each of the sections is 1 pixel, since in theory nothing
    // can be smaller than 1 pixel. However, in high-resolution displays 1 pixel is
    // too wide, and that makes the separation between sections clearly visible by a
    // straight white line. To fix this issue, I set the size of the offset not to
    // 1 pixel but 0.10. This has proved to work OK both in low-resolution displays
    // as well as high-resolution displays.
    const double offsetWidth = 0.1;
    cx = cx + cxOffset * offsetWidth;
    cy = cy + cyOffset * offsetWidth;

    // Calculate starting point, ending point and control points of Bezier curve.
    double f = 4 * tan((angleEnd - angleStart) / 4) / 3;
    double x0 =  cx + (r * cos(angleStart));
    double y0 =  cy + (r * sin(angleStart));
   
    double x1 =  cx + (r * cos(angleStart)) - f * (r * sin(angleStart));
    double y1 =  cy + (r * sin(angleStart)) + f * (r * cos(angleStart));
    
    double x2 =  cx + (r * cos(angleEnd)) + f * (r * sin(angleEnd));
    double y2 =  cy + (r * sin(angleEnd)) - f * (r * cos(angleEnd));
   
    double x3 =  cx + (r * cos(angleEnd));
    double y3 =  cy + (r * sin(angleEnd));
    double globalAlpha =1.f;
    // Add patch with shape of the sector and gradient colors.
    begin_patch();
    move_to( cx, cy);
    line_to(x0, y0);
    curve_to(x1, y1, x2, y2, x3, y3);
    /*setCornerColorRGBA(gradient, 0, from, globalAlpha);
    setCornerColorRGBA(gradient, 1, from, globalAlpha);
    setCornerColorRGBA(gradient, 2, to, globalAlpha);
    setCornerColorRGBA(gradient, 3, to, globalAlpha);*/
    set_corner_color_rgba(0, from.red, from.green, from.blue, from.alpha * globalAlpha);
    set_corner_color_rgba(1, from.red, from.green, from.blue, from.alpha * globalAlpha);
    set_corner_color_rgba(2, to.red, to.green, to.blue, to.alpha * globalAlpha);
    set_corner_color_rgba(3, to.red, to.green, to.blue, to.alpha * globalAlpha);
    end_patch();
}
} //namespace Cairo
