/* Copyright (C) 2005 The cairomm Development Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <cairomm/pattern.h>
#include <cairomm/private.h>
#include <cairomm/matrix.h>
#include <math.h>

namespace Cairo
{

Pattern::Pattern()
: m_cobject(nullptr)
{
}

Pattern::Pattern(cairo_pattern_t* cobject, bool has_reference)
: m_cobject(nullptr)
{
  if(has_reference)
    m_cobject = cobject;
  else
    m_cobject = cairo_pattern_reference(cobject);
}

Pattern::~Pattern()
{
  if(m_cobject)
    cairo_pattern_destroy(m_cobject);
}

void Pattern::reference() const
{
 cairo_pattern_reference(m_cobject);
}

void Pattern::unreference() const
{
  cairo_pattern_destroy(m_cobject);
}

void Pattern::set_matrix(const Matrix& matrix)
{
  cairo_pattern_set_matrix(m_cobject, (cairo_matrix_t*)&matrix);
  check_object_status_and_throw_exception(*this);
}

void Pattern::get_matrix(Matrix& matrix) const
{
  cairo_pattern_get_matrix(m_cobject, (cairo_matrix_t*)&matrix);
  check_object_status_and_throw_exception(*this);
}

Matrix Pattern::get_matrix() const
{
  Cairo::Matrix m;
  cairo_pattern_get_matrix(m_cobject, (cairo_matrix_t*)&m);
  check_object_status_and_throw_exception(*this);
  return m;
}

Pattern::Type Pattern::get_type() const
{
  auto pattern_type = cairo_pattern_get_type(m_cobject);
  check_object_status_and_throw_exception(*this);
  return static_cast<Type>(pattern_type);
}

void Pattern::set_extend(Extend extend)
{
  cairo_pattern_set_extend(m_cobject, (cairo_extend_t)extend);
  check_object_status_and_throw_exception(*this);
}

Pattern::Extend Pattern::get_extend() const
{
  const auto result = static_cast<Extend>(cairo_pattern_get_extend(m_cobject));
  check_object_status_and_throw_exception(*this);
  return result;
}

SolidPattern::SolidPattern(cairo_pattern_t* cobject, bool has_reference)
: Pattern(cobject, has_reference)
{
}
void
SolidPattern::get_rgba(double& red, double& green,
                        double& blue, double& alpha) const
{
  // ignore the return value since we know that this is a solid color pattern
  cairo_pattern_get_rgba(m_cobject, &red, &green, &blue, &alpha);
  check_object_status_and_throw_exception(*this);
}

SolidPattern::~SolidPattern()
{
}

RefPtr<SolidPattern> SolidPattern::create_rgb(double red, double green, double blue)
{
  auto cobject = cairo_pattern_create_rgb(red, green, blue);
  check_status_and_throw_exception(cairo_pattern_status(cobject)); 
  return make_refptr_for_instance<SolidPattern>(new SolidPattern(cobject, true /* has reference */));
}

RefPtr<SolidPattern> SolidPattern::create_rgba(double red, double green, double blue, double alpha)
{
  cairo_pattern_t* cobject  = cairo_pattern_create_rgba(red, green, blue, alpha);
  check_status_and_throw_exception(cairo_pattern_status(cobject));
  return make_refptr_for_instance<SolidPattern>(new SolidPattern(cobject, true /* has reference */));
}


SurfacePattern::SurfacePattern(const RefPtr<Surface>& surface)
{
  m_cobject = cairo_pattern_create_for_surface(surface->cobj());
  check_object_status_and_throw_exception(*this); 
}

RefPtr<Surface>
SurfacePattern::get_surface()
{
  cairo_surface_t* surface = nullptr;
  // we can ignore the return value since we know this is a surface pattern
  cairo_pattern_get_surface(const_cast<cairo_pattern_t*>(m_cobject), &surface);
  check_object_status_and_throw_exception(*this);
  return make_refptr_for_instance<Surface>(new Surface(surface, false /* does not have reference */));
}

RefPtr<const Surface>
SurfacePattern::get_surface() const
{
  return const_cast<SurfacePattern*>(this)->get_surface();
}

RefPtr<SurfacePattern> SurfacePattern::create(const RefPtr<Surface>& surface)
{
  return make_refptr_for_instance<SurfacePattern>(new SurfacePattern(surface));
}

SurfacePattern::SurfacePattern(cairo_pattern_t* cobject, bool has_reference)
: Pattern(cobject, has_reference)
{
}

SurfacePattern::~SurfacePattern()
{
}

void SurfacePattern::set_filter(Filter filter)
{
  cairo_pattern_set_filter(m_cobject, (cairo_filter_t)filter);
  check_object_status_and_throw_exception(*this);
}

SurfacePattern::Filter SurfacePattern::get_filter() const
{
  auto result = static_cast<Filter>(cairo_pattern_get_filter(m_cobject));
  check_object_status_and_throw_exception(*this);
  return result;
}



Gradient::Gradient()
{
}

Gradient::Gradient(cairo_pattern_t* cobject, bool has_reference)
: Pattern(cobject, has_reference)
{
}

Gradient::~Gradient()
{
}

void Gradient::add_color_stop_rgb(double offset, double red, double green, double blue)
{
  cairo_pattern_add_color_stop_rgb(m_cobject, offset, red, green, blue);
  check_object_status_and_throw_exception(*this);
}

void Gradient::add_color_stop_rgba(double offset, double red, double green, double blue, double alpha)
{
  cairo_pattern_add_color_stop_rgba(m_cobject, offset, red, green, blue, alpha);
  check_object_status_and_throw_exception(*this);
}

std::vector<ColorStop>
Gradient::get_color_stops() const
{
  std::vector<ColorStop> stops;

  int num_stops = 0;
  // we can ignore the return value since we know this is a gradient pattern
  cairo_pattern_get_color_stop_count(m_cobject, &num_stops);
  // since we know the total number of stops, we can avoid re-allocation with
  // each addition to the vector by pre-allocating the required number
  stops.reserve(num_stops);
  for(int i = 0; i < num_stops; ++i)
  {
    ColorStop stop;
    cairo_pattern_get_color_stop_rgba(m_cobject, i, &stop.offset, &stop.red,
                                      &stop.green, &stop.blue, &stop.alpha);
    stops.push_back(stop);
  }
  return stops;
}


LinearGradient::LinearGradient(double x0, double y0, double x1, double y1)
{
  m_cobject = cairo_pattern_create_linear(x0, y0, x1, y1);
  check_object_status_and_throw_exception(*this); 
}

void
LinearGradient::get_linear_points(double &x0, double &y0,
                                   double &x1, double &y1) const
{
  // ignore the return value since we know that this is a linear gradient
  // pattern
  cairo_pattern_get_linear_points(m_cobject, &x0, &y0, &x1, &y1);
  check_object_status_and_throw_exception(*this);
}


RefPtr<LinearGradient> LinearGradient::create(double x0, double y0, double x1, double y1)
{
  return make_refptr_for_instance<LinearGradient>(new LinearGradient(x0, y0, x1, y1));
}

LinearGradient::LinearGradient(cairo_pattern_t* cobject, bool has_reference)
: Gradient(cobject, has_reference)
{
}

LinearGradient::~LinearGradient()
{
}


RadialGradient::RadialGradient(double cx0, double cy0, double radius0, double cx1, double cy1, double radius1)
{
  m_cobject = cairo_pattern_create_radial(cx0, cy0, radius0, cx1, cy1, radius1);
  check_object_status_and_throw_exception(*this); 
}

void
RadialGradient::get_radial_circles(double& x0, double& y0, double& r0,
                                    double& x1, double& y1, double& r1) const
{
  // ignore the return value since we know that this is a radial gradient
  // pattern
  cairo_pattern_get_radial_circles(const_cast<cairo_pattern_t*>(m_cobject),
                                    &x0, &y0, &r0, &x1, &y1, &r1);
  check_object_status_and_throw_exception(*this); 
}


RefPtr<RadialGradient> RadialGradient::create(double cx0, double cy0, double radius0, double cx1, double cy1, double radius1)
{
  return make_refptr_for_instance<RadialGradient>(new RadialGradient(cx0, cy0, radius0, cx1, cy1, radius1));
}

RadialGradient::RadialGradient(cairo_pattern_t* cobject, bool has_reference)
: Gradient(cobject, has_reference)
{
}

RadialGradient::~RadialGradient()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MeshPattern::MeshPattern(){
    m_cobject=cairo_pattern_create_mesh();
    check_object_status_and_throw_exception(*this);
}

MeshPattern::MeshPattern(cairo_pattern_t* cobject, bool has_reference)
    :Pattern(cobject,has_reference){
}

void MeshPattern::begin_patch(){
    cairo_mesh_pattern_begin_patch(m_cobject);
}

void MeshPattern::end_patch(){
    cairo_mesh_pattern_end_patch(m_cobject);
}

void MeshPattern::line_to(double x,double y){
    cairo_mesh_pattern_line_to(m_cobject,x,y);
}

void MeshPattern::move_to(double x,double y){
    cairo_mesh_pattern_move_to(m_cobject,x,y);
}

void MeshPattern::curve_to(double x1,double y1,double x2,double y2,double x3,double y3){
    cairo_mesh_pattern_curve_to(m_cobject,x1,y1,x2,y2,x3,y3);
}

void MeshPattern::set_control_point(uint32_t point_num,double x,double y){
    cairo_mesh_pattern_set_control_point(m_cobject,point_num,x,y);
}

void MeshPattern::set_corner_color_rgb(uint32_t corner_num,double red,double green,double blue){
    cairo_mesh_pattern_set_corner_color_rgb(m_cobject,corner_num,red,green,blue);
}

void MeshPattern::set_corner_color_rgba(uint32_t corner_num,double red,double green,double blue,double alpha){
    cairo_mesh_pattern_set_corner_color_rgba(m_cobject,corner_num,red,green,blue,alpha);
}

int MeshPattern::get_corner_color_rgba(uint32_t patch_num,uint32_t corner_num,double& red,double& green,double& blue,double&alpha){
    return cairo_mesh_pattern_get_corner_color_rgba(m_cobject,patch_num,corner_num,&red,&green,&blue,&alpha);
}

int MeshPattern::get_control_point(uint32_t patch_num,uint32_t corner_num,double&x,double&y){
    return cairo_mesh_pattern_get_control_point(m_cobject,patch_num,corner_num,&x,&y);
}

RefPtr<MeshPattern>MeshPattern::create(){
    cairo_pattern_t*p=cairo_pattern_create_mesh();
    return make_refptr_for_instance<MeshPattern>(new MeshPattern());
}

SweepGradient::SweepGradient(double cx,double cy,double radius)
  :MeshPattern(){
    m_cx=cx;
    m_cy=cy;
    m_radius=radius; 
    printf("(%.2f,%.2f:%.2f):%p %p\r\n",m_cx,m_cy,m_radius,this,m_cobject);
}

SweepGradient::SweepGradient(cairo_pattern_t* cobject, bool has_reference)
 :MeshPattern(cobject, has_reference){
}

RefPtr<SweepGradient>SweepGradient::create(double cx,double cy,double radius){
    return make_refptr_for_instance<SweepGradient>(new SweepGradient(cx,cy,radius));
}

void SweepGradient::add_sector_patch( double angle_A,double A_r, double A_g, double A_b,double A_a,
         double angle_B,double B_r, double B_g, double B_b,double B_a){
    double r_sin_A, r_cos_A; 
    double r_sin_B, r_cos_B; 
    double h; 
    r_sin_A = m_radius * sin (angle_A); 
    r_cos_A = m_radius * cos (angle_A); 
    r_sin_B = m_radius * sin (angle_B); 
    r_cos_B = m_radius * cos (angle_B); 

    h = 4.0/3.0 * tan ((angle_B - angle_A)/4.0); 

    begin_patch(); 

    move_to (m_cx, m_cy); 
    line_to (m_cx + r_cos_A,  m_cy + r_sin_A); 

    curve_to(m_cx + r_cos_A - h * r_sin_A, m_cy + r_sin_A + h * r_cos_A, 
         m_cx + r_cos_B + h * r_sin_B, m_cy + r_sin_B - h * r_cos_B, 
         m_cx + r_cos_B, m_cy + r_sin_B); 

    set_corner_color_rgba (0, 1, 1, 1,A_a); 
    set_corner_color_rgba (1, A_r, A_g, A_b,A_a); 
    set_corner_color_rgba (2, B_r, B_g, B_b,B_a); 
    end_patch ();
}

void SweepGradient::add_sector_patch( double angle_A,double A_r, double A_g, double A_b,
         double angle_B,double B_r, double B_g, double B_b){
   add_sector_patch(angle_A,A_r,A_g,A_b,1.f,angle_B,B_r,B_g,B_b,1.f);
}

static void color2rgba(uint32_t c,float&r,float&g,float&b,float&a){
    a=(c>>24)/255.;
    r=((c>>16)&0xFF)/255.;
    g=((c>>8)&0xFF)/255.;
    b=(c&0xFF)/255;
}

void SweepGradient::add_sector_patch(double angleA,uint32_t colorA,double angleB, uint32_t colorB){
    float r1,g1,b1,a1,r2,g2,b2,a2;
    color2rgba(colorA,r1,g1,b1,a1);
    color2rgba(colorB,r2,g2,b2,a2);
    add_sector_patch(angleA,r1,g1,b1,a1,angleB,r2,g2,b2,a2);
}

} //namespace Cairo

// vim: ts=2 sw=2 et
