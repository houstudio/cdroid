/* Copyright (C) 2005 The cairomm Development Team
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

/* M_PI is defined in math.h in the case of Microsoft Visual C++ */

#include <cairommconfig.h>
#include <cairomm/context.h>
#include <cairomm/context_private.h>
#include <cairomm/mesh_pattern.h>
#include <cairomm/private.h>
#include <cairomm/surface.h>
#include <cairomm/script_surface.h>
#include <cairomm/scaledfont.h>
#include <cairomm/xcb_surface.h>

/* Solaris et. al. need math.h for M_PI too */
#include <cmath>

using namespace Cairo::Private;

namespace
{
Cairo::RefPtr<Cairo::Path> get_path_wrapper(cairo_path_t* cpath)
{
  auto cpp_path = Cairo::make_refptr_for_instance<Cairo::Path>(new Cairo::Path(cpath, true /* take ownership */));
  // If an exception is thrown, cpp_path's destructor will call ~Path(),
  // which will destroy cpath.
  Cairo::check_object_status_and_throw_exception(*cpp_path);
  return cpp_path;
}
} // anonymous namespace

namespace Cairo
{

Context::Context(const RefPtr<Surface>& target)
: m_cobject(nullptr)
{
  m_cobject = cairo_create(target->cobj());
  check_object_status_and_throw_exception(*this);
}

RefPtr<Context> Context::create(const RefPtr<Surface>& target)
{
  return make_refptr_for_instance<Context>(new Context(target));
}

Context::Context(cairo_t* cobject, bool has_reference)
: m_cobject(nullptr)
{
  if(has_reference)
    m_cobject = cobject;
  else
    m_cobject = cairo_reference(cobject);
}

Context::~Context()
{
  if(m_cobject)
    cairo_destroy(m_cobject);
}


void Context::reference() const
{
 cairo_reference(const_cast<cobject*>(cobj()));
}

void Context::unreference() const
{
  cairo_destroy(const_cast<cobject*>(cobj()));
}

void Context::save()
{
  cairo_save(cobj());
  check_object_status_and_throw_exception(*this);
}

void Context::restore()
{
  cairo_restore(cobj());
  check_object_status_and_throw_exception(*this);
}

void Context::set_operator(Operator op)
{
  cairo_set_operator(cobj(), static_cast<cairo_operator_t>(op));
  check_object_status_and_throw_exception(*this);
}

void Context::set_source(const RefPtr<const Pattern>& source)
{
  cairo_set_source(cobj(), const_cast<cairo_pattern_t*>(source->cobj()));
  check_object_status_and_throw_exception(*this);
}

void Context::set_source_rgb(double red, double green, double blue)
{
  cairo_set_source_rgb(cobj(), red, green, blue);
  check_object_status_and_throw_exception(*this);
}

void Context::set_source_rgba(double red, double green, double blue,
double alpha)
{
  cairo_set_source_rgba(cobj(), red, green, blue, alpha);
  check_object_status_and_throw_exception(*this);
}

void Context::set_source(const RefPtr<Surface>& surface, double x, double y)
{
  cairo_set_source_surface(cobj(), surface->cobj(), x, y);
  check_object_status_and_throw_exception(*this);
}

void Context::set_tolerance(double tolerance)
{
  cairo_set_tolerance(cobj(), tolerance);
  check_object_status_and_throw_exception(*this);
}

void Context::set_antialias(Antialias antialias)
{
  cairo_set_antialias(cobj(), static_cast<cairo_antialias_t>(antialias));
  check_object_status_and_throw_exception(*this);
}

void Context::set_fill_rule(FillRule fill_rule)
{
  cairo_set_fill_rule(cobj(), static_cast<cairo_fill_rule_t>(fill_rule));
  check_object_status_and_throw_exception(*this);
}

void Context::set_line_width(double width)
{
  cairo_set_line_width(cobj(), width);
  check_object_status_and_throw_exception(*this);
}

void Context::set_line_cap(LineCap line_cap)
{
  cairo_set_line_cap(cobj(), static_cast<cairo_line_cap_t>(line_cap));
  check_object_status_and_throw_exception(*this);
}

void Context::set_line_join(LineJoin line_join)
{
  cairo_set_line_join(cobj(), static_cast<cairo_line_join_t>(line_join));
  check_object_status_and_throw_exception(*this);
}

void Context::set_dash(const std::valarray<double>& dashes, double offset)
{
  std::vector<double> v(dashes.size());
  for(size_t i = 0; i < dashes.size(); ++i)
    v[i] = dashes[i];

  set_dash(v, offset);
}

void Context::set_dash(const std::vector<double>& dashes, double offset)
{
  cairo_set_dash(cobj(),
    (dashes.empty() ? nullptr : &dashes[0]),
    dashes.size(), offset);
  check_object_status_and_throw_exception(*this);
}

void Context::unset_dash()
{
  cairo_set_dash(cobj(), nullptr, 0, 0.0);
  check_object_status_and_throw_exception(*this);
}

void Context::set_miter_limit(double limit)
{
  cairo_set_miter_limit(cobj(), limit);
  check_object_status_and_throw_exception(*this);
}

void Context::translate(double tx, double ty)
{
  cairo_translate(cobj(), tx, ty);
  check_object_status_and_throw_exception(*this);
}

void Context::scale(double sx, double sy)
{
  cairo_scale(cobj(), sx, sy);
  check_object_status_and_throw_exception(*this);
}

void Context::rotate(double angle_radians)
{
  cairo_rotate(cobj(), angle_radians);
  check_object_status_and_throw_exception(*this);
}

void Context::rotate_degrees(double angle_degrees)
{
  cairo_rotate(cobj(), angle_degrees * M_PI/180.0);
  check_object_status_and_throw_exception(*this);
}

void Context::transform(const Matrix& matrix)
{
  cairo_transform(cobj(), &matrix);
  check_object_status_and_throw_exception(*this);
}

void Context::set_matrix(const Matrix& matrix)
{
  cairo_set_matrix(cobj(), &matrix);
  check_object_status_and_throw_exception(*this);
}

void Context::set_identity_matrix()
{
  cairo_identity_matrix(cobj());
  check_object_status_and_throw_exception(*this);
}

void Context::user_to_device(double& x, double& y) const
{
  cairo_user_to_device(const_cast<cobject*>(cobj()), &x, &y);
  check_object_status_and_throw_exception(*this);
}

void Context::user_to_device_distance(double& dx, double& dy) const
{
  cairo_user_to_device_distance(const_cast<cobject*>(cobj()), &dx, &dy);
  check_object_status_and_throw_exception(*this);
}

void Context::device_to_user(double& x, double& y) const
{
  cairo_device_to_user(const_cast<cobject*>(cobj()), &x, &y);
  check_object_status_and_throw_exception(*this);
}

void Context::device_to_user_distance(double& dx, double& dy) const
{
  cairo_device_to_user_distance(const_cast<cobject*>(cobj()), &dx, &dy);
  check_object_status_and_throw_exception(*this);
}

void Context::begin_new_path()
{
  cairo_new_path(cobj());
  check_object_status_and_throw_exception(*this);
}

void Context::begin_new_sub_path()
{
  cairo_new_sub_path(cobj());
  check_object_status_and_throw_exception(*this);
}

void Context::move_to(double x, double y)
{
  cairo_move_to(cobj(), x, y);
  check_object_status_and_throw_exception(*this);
}

void Context::line_to(double x, double y)
{
  cairo_line_to(cobj(), x, y);
  check_object_status_and_throw_exception(*this);
}

void Context::curve_to(double x1, double y1, double x2, double y2, double x3, double y3)
{
  cairo_curve_to(cobj(), x1, y1, x2, y2, x3, y3);
  check_object_status_and_throw_exception(*this);
}

void Context::arc(double xc, double yc, double radius, double angle1, double angle2)
{
  cairo_arc(cobj(), xc, yc, radius, angle1, angle2);
  check_object_status_and_throw_exception(*this);
}

void Context::arc_negative(double xc, double yc, double radius, double angle1, double angle2)
{
  cairo_arc_negative(cobj(), xc, yc, radius, angle1, angle2);
  check_object_status_and_throw_exception(*this);
}

void Context::rel_move_to(double dx, double dy)
{
  cairo_rel_move_to(cobj(), dx, dy);
  check_object_status_and_throw_exception(*this);
}

void Context::rel_line_to(double dx, double dy)
{
  cairo_rel_line_to(cobj(), dx, dy);
  check_object_status_and_throw_exception(*this);
}

void Context::rel_curve_to(double dx1, double dy1, double dx2, double dy2, double dx3, double dy3)
{
  cairo_rel_curve_to(cobj(), dx1, dy1, dx2, dy2, dx3, dy3);
  check_object_status_and_throw_exception(*this);
}

void Context::rectangle(double x, double y, double width, double height)
{
  cairo_rectangle(cobj(), x, y, width, height);
  check_object_status_and_throw_exception(*this);
}

void Context::close_path()
{
  cairo_close_path(cobj());
  check_object_status_and_throw_exception(*this);
}

void Context::paint()
{
  cairo_paint(cobj());
  check_object_status_and_throw_exception(*this);
}

void Context::paint_with_alpha(double alpha)
{
  cairo_paint_with_alpha(cobj(), alpha);
  check_object_status_and_throw_exception(*this);
}

void Context::mask(const RefPtr<const Pattern>& pattern)
{
  cairo_mask(cobj(), const_cast<cairo_pattern_t*>(pattern->cobj()));
  check_object_status_and_throw_exception(*this);
}

void Context::mask(const RefPtr<const Surface>& surface, double surface_x, double surface_y)
{
  cairo_mask_surface(cobj(), const_cast<cairo_surface_t*>(surface->cobj()), surface_x, surface_y);
  check_object_status_and_throw_exception(*this);
}

void Context::stroke()
{
  cairo_stroke(cobj());
  check_object_status_and_throw_exception(*this);
}

void Context::stroke_preserve()
{
  cairo_stroke_preserve(cobj());
  check_object_status_and_throw_exception(*this);
}

void Context::fill()
{
  cairo_fill(cobj());
  check_object_status_and_throw_exception(*this);
}

void Context::fill_preserve()
{
  cairo_fill_preserve(cobj());
  check_object_status_and_throw_exception(*this);
}

void Context::copy_page()
{
  cairo_copy_page(cobj());
  check_object_status_and_throw_exception(*this);
}

void Context::show_page()
{
  cairo_show_page(cobj());
  check_object_status_and_throw_exception(*this);
}

bool Context::in_stroke(double x, double y) const
{
  const bool result = cairo_in_stroke(const_cast<cobject*>(cobj()), x, y);
  check_object_status_and_throw_exception(*this);
  return result;
}

bool Context::in_fill(double x, double y) const
{
  const bool result = cairo_in_fill(const_cast<cobject*>(cobj()), x, y);
  check_object_status_and_throw_exception(*this);
  return result;
}

bool Context::in_clip(double x, double y) const
{
  const bool result = cairo_in_clip(const_cast<cobject*>(cobj()), x, y);
  check_object_status_and_throw_exception(*this);
  return result;
}

void Context::get_stroke_extents(double& x1, double& y1, double& x2, double& y2) const
{
  cairo_stroke_extents(const_cast<cobject*>(cobj()), &x1, &y1, &x2, &y2);
  check_object_status_and_throw_exception(*this);
}

void Context::get_fill_extents(double& x1, double& y1, double& x2, double& y2) const
{
  cairo_fill_extents(const_cast<cobject*>(cobj()), &x1, &y1, &x2, &y2);
  check_object_status_and_throw_exception(*this);
}

void Context::reset_clip()
{
  cairo_reset_clip(cobj());
  check_object_status_and_throw_exception(*this);
}

void Context::clip()
{
  cairo_clip(cobj());
  check_object_status_and_throw_exception(*this);
}

void Context::clip_preserve()
{
  cairo_clip_preserve(cobj());
  check_object_status_and_throw_exception(*this);
}

void Context::get_clip_extents(double& x1, double& y1, double& x2, double& y2) const
{
  cairo_clip_extents(const_cast<cobject*>(const_cast<cobject*>(cobj())), &x1, &y1, &x2, &y2);
  check_object_status_and_throw_exception(*this);
}

void Context::copy_clip_rectangle_list(std::vector<Rectangle>& rectangles) const
{
  cairo_rectangle_list_t* c_list = nullptr;
  // It would be nice if the cairo interface didn't copy it into a C array first
  // and just let us do the copying...
  c_list = cairo_copy_clip_rectangle_list(const_cast<cobject*>(const_cast<cobject*>(cobj())));
  // the rectangle list contains a status field that we need to check and the
  // cairo context also has a status that we need to check
  // FIXME: do we want to throw an exception if the clip can't be represented by
  // rectangles?  or do we just want to return an empty list?
  check_status_and_throw_exception(c_list->status);
  check_object_status_and_throw_exception(*this);
  // copy the C array into the passed C++ list
  rectangles.assign(c_list->rectangles,
                    c_list->rectangles + c_list->num_rectangles);
  // free the memory allocated to the C array since we've copied it into a
  // standard C++ container
  cairo_rectangle_list_destroy(c_list);
}

void Context::select_font_face(const std::string& family, ToyFontFace::Slant slant, ToyFontFace::Weight weight)
{
  cairo_select_font_face(cobj(), family.c_str(),
          static_cast<cairo_font_slant_t>(slant),
          static_cast<cairo_font_weight_t>(weight));
  check_object_status_and_throw_exception(*this);
}

void Context::set_font_size(double size)
{
  cairo_set_font_size(cobj(), size);
  check_object_status_and_throw_exception(*this);
}

void Context::set_font_matrix(const Matrix& matrix)
{
  cairo_set_font_matrix(cobj(), &matrix);
  check_object_status_and_throw_exception(*this);
}

void Context::get_font_matrix(Matrix& matrix) const
{
  cairo_get_font_matrix(const_cast<cobject*>(cobj()), &matrix);
  check_object_status_and_throw_exception(*this);
}

void Context::set_font_options(const FontOptions& options)
{
  cairo_set_font_options(cobj(), options.cobj());
  check_object_status_and_throw_exception(*this);
}

void Context::get_font_options(FontOptions& options) const
{
  cairo_get_font_options(const_cast<cobject*>(cobj()), options.cobj());
  check_object_status_and_throw_exception(*this);
}

void Context::set_scaled_font(const RefPtr<const ScaledFont>& scaled_font)
{
  cairo_set_scaled_font(cobj(), scaled_font->cobj());
  check_object_status_and_throw_exception(*this);
}

RefPtr<ScaledFont> Context::get_scaled_font()
{
  auto font = cairo_get_scaled_font(cobj());
  check_object_status_and_throw_exception(*this);
  return make_refptr_for_instance<ScaledFont>(new ScaledFont(font, false /* does not have reference */));
}

void Context::show_text(const std::string& utf8)
{
  cairo_show_text(cobj(), utf8.c_str());
  check_object_status_and_throw_exception(*this);
}

void Context::show_text_glyphs(const std::string& utf8,
                               const std::vector<Glyph>& glyphs,
                               const std::vector<TextCluster>& clusters,
                               TextClusterFlags cluster_flags)
{
  cairo_show_text_glyphs(cobj(), utf8.c_str(), utf8.size(),
                         (glyphs.empty() ? nullptr : &glyphs[0]),
                         glyphs.size(),
                         (clusters.empty() ? nullptr : &clusters[0]),
                         clusters.size(),
                         static_cast<cairo_text_cluster_flags_t>(cluster_flags));
  check_object_status_and_throw_exception(*this);
}

void Context::show_glyphs(const std::vector<Glyph>& glyphs)
{
  cairo_show_glyphs(cobj(),
    const_cast<cairo_glyph_t*>((glyphs.empty() ? nullptr : &glyphs[0])),
    glyphs.size());
  check_object_status_and_throw_exception(*this);
}

RefPtr<FontFace> Context::get_font_face()
{
  auto cfontface = cairo_get_font_face(cobj());
  check_object_status_and_throw_exception(*this);
  return make_refptr_for_instance<FontFace>(new FontFace(cfontface, false /* does not have reference */));
}

RefPtr<const FontFace> Context::get_font_face() const
{
  auto cfontface = cairo_get_font_face(const_cast<cobject*>(cobj()));
  check_object_status_and_throw_exception(*this);
  return make_refptr_for_instance<const FontFace>(new FontFace(cfontface, false /* does not have reference */));
}

void Context::get_font_extents(FontExtents& extents) const
{
  cairo_font_extents(const_cast<cobject*>(cobj()), &extents);
  check_object_status_and_throw_exception(*this);
}

void Context::set_font_face(const RefPtr<const FontFace>& font_face)
{
  cairo_set_font_face(cobj(), const_cast<cairo_font_face_t*>(font_face->cobj()));
  check_object_status_and_throw_exception(*this);
}

void Context::get_text_extents(const std::string& utf8, TextExtents& extents) const
{
  cairo_text_extents(const_cast<cobject*>(cobj()), utf8.c_str(), &extents);
  check_object_status_and_throw_exception(*this);
}

void Context::get_glyph_extents(const std::vector<Glyph>& glyphs, TextExtents& extents) const
{
  cairo_glyph_extents(const_cast<cobject*>(cobj()),
                      const_cast<cairo_glyph_t*>(glyphs.empty() ? nullptr : &glyphs[0]),
                      glyphs.size(), &extents);
  check_object_status_and_throw_exception(*this);
}

void Context::text_path(const std::string& utf8)
{
  cairo_text_path(cobj(), utf8.c_str());
  check_object_status_and_throw_exception(*this);
}

void Context::glyph_path(const std::vector<Glyph>& glyphs)
{
  cairo_glyph_path(cobj(),
    const_cast<cairo_glyph_t*>(glyphs.empty() ? nullptr : &glyphs[0]),
    glyphs.size());
  check_object_status_and_throw_exception(*this);
}

Context::Operator Context::get_operator() const
{
  const auto result =
    static_cast<Operator>(cairo_get_operator(const_cast<cobject*>(cobj())));
  check_object_status_and_throw_exception(*this);
  return result;
}

static RefPtr<Pattern> get_pattern_wrapper (cairo_pattern_t* pattern)
{
  auto pattern_type = cairo_pattern_get_type (pattern);
  switch (pattern_type)
  {
    case CAIRO_PATTERN_TYPE_SOLID:
      return make_refptr_for_instance<SolidPattern>(new SolidPattern(pattern, false /* does not have reference */));
      break;
    case CAIRO_PATTERN_TYPE_SURFACE:
      return make_refptr_for_instance<SurfacePattern>(new SurfacePattern(pattern, false /* does not have reference */));
      break;
    case CAIRO_PATTERN_TYPE_LINEAR:
      return make_refptr_for_instance<LinearGradient>(new LinearGradient(pattern, false /* does not have reference */));
      break;
    case CAIRO_PATTERN_TYPE_RADIAL:
      return make_refptr_for_instance<RadialGradient>(new RadialGradient(pattern, false /* does not have reference */));
      break;
    case CAIRO_PATTERN_TYPE_MESH:
      return make_refptr_for_instance<MeshPattern>(new MeshPattern(pattern, false /* does not have reference */));
      break;
    default:
      return make_refptr_for_instance<Pattern>(new Pattern(pattern, false /* does not have reference */));
  }
}

RefPtr<Pattern> Context::get_source()
{
  auto pattern = cairo_get_source(cobj());
  check_object_status_and_throw_exception(*this);
  return get_pattern_wrapper (pattern);
}

RefPtr<const Pattern> Context::get_source() const
{
  auto pattern = cairo_get_source(const_cast<cobject*>(cobj()));
  check_object_status_and_throw_exception(*this);
  return get_pattern_wrapper(pattern);
}

RefPtr<SurfacePattern> Context::get_source_for_surface()
{
  auto pattern = cairo_get_source(cobj());
  check_object_status_and_throw_exception(*this);
  auto pattern_type = cairo_pattern_get_type(pattern);
  if (pattern_type != CAIRO_PATTERN_TYPE_SURFACE)
    return {};
  return make_refptr_for_instance<SurfacePattern>(new SurfacePattern(pattern, false /* does not have reference */));
}

RefPtr<const SurfacePattern> Context::get_source_for_surface() const
{
  return const_cast<Context*>(this)->get_source_for_surface();
}

double Context::get_tolerance() const
{
  const auto result = cairo_get_tolerance(const_cast<cobject*>(cobj()));
  check_object_status_and_throw_exception(*this);
  return result;
}

Antialias Context::get_antialias() const
{
  const auto result = static_cast<Antialias>(cairo_get_antialias(const_cast<cobject*>(cobj())));
  check_object_status_and_throw_exception(*this);
  return result;
}

bool Context::has_current_point() const
{
  return cairo_has_current_point(const_cast<cobject*>(cobj()));
}

void Context::get_current_point(double& x, double& y) const
{
  cairo_get_current_point(const_cast<cobject*>(cobj()), &x, &y);
  check_object_status_and_throw_exception(*this);
}

Context::FillRule Context::get_fill_rule() const
{
  const auto result = static_cast<FillRule>(cairo_get_fill_rule(const_cast<cobject*>(cobj())));
  check_object_status_and_throw_exception(*this);
  return result;
}

double Context::get_line_width() const
{
  const auto result = cairo_get_line_width(const_cast<cobject*>(cobj()));
  check_object_status_and_throw_exception(*this);
  return result;
}

Context::LineCap Context::get_line_cap() const
{
  const auto result = static_cast<LineCap>(cairo_get_line_cap(const_cast<cobject*>(cobj())));
  check_object_status_and_throw_exception(*this);
  return result;
}

Context::LineJoin Context::get_line_join() const
{
  const auto result = static_cast<LineJoin>(cairo_get_line_join(const_cast<cobject*>(cobj())));
  check_object_status_and_throw_exception(*this);
  return result;
}

double Context::get_miter_limit() const
{
  const auto result = cairo_get_miter_limit(const_cast<cobject*>(cobj()));
  check_object_status_and_throw_exception(*this);
  return result;
}

void
Context::get_dash(std::vector<double>& dashes, double& offset) const
{
  // Allocate this array dynamically because some compilers complain about
  // allocating arrays on the stack when the array size isn't a compile-time
  // constant...
  const auto cnt = cairo_get_dash_count(const_cast<cobject*>(cobj()));
  auto dash_array = new double[cnt];
  cairo_get_dash(const_cast<cobject*>(cobj()), dash_array, &offset);
  check_object_status_and_throw_exception(*this);
  dashes.assign(dash_array, dash_array + cnt);
  delete[] dash_array;
}

void Context::get_matrix(Matrix& matrix)
{
  cairo_get_matrix(cobj(), &matrix);
  check_object_status_and_throw_exception(*this);
}

Matrix Context::get_matrix() const
{
  Cairo::Matrix m;
  cairo_get_matrix(const_cast<cobject*>(cobj()), (cairo_matrix_t*)&m);
  check_object_status_and_throw_exception(*this);
  return m;
}

static
RefPtr<Surface> get_surface_wrapper (cairo_surface_t* surface)
{
  auto surface_type = cairo_surface_get_type (surface);
  switch (surface_type)
  {
    case CAIRO_SURFACE_TYPE_IMAGE:
      return make_refptr_for_instance<ImageSurface>(new ImageSurface(surface, false /* does not have reference */));
      break;
#if CAIRO_HAS_PDF_SURFACE
    case CAIRO_SURFACE_TYPE_PDF:
      return make_refptr_for_instance<PdfSurface>(new PdfSurface(surface, false /* does not have reference */));
      break;
#endif
#if CAIRO_HAS_PS_SURFACE
    case CAIRO_SURFACE_TYPE_PS:
      return make_refptr_for_instance<PsSurface>(new PsSurface(surface, false /* does not have reference */));
      break;
#endif
#if CAIRO_HAS_XLIB_SURFACE
    case CAIRO_SURFACE_TYPE_XLIB:
      return wrap_surface_xlib(surface);
      break;
#endif
#if CAIRO_HAS_GLITZ_SURFACE
    case CAIRO_SURFACE_TYPE_GLITZ:
      return make_refptr_for_instance<GlitzSurface>(new GlitzSurface(surface, false /* does not have reference */));
      break;
#endif
#if CAIRO_HAS_QUARTZ_SURFACE
    case CAIRO_SURFACE_TYPE_QUARTZ:
      return wrap_surface_quartz(surface);
      break;
#endif
#if CAIRO_HAS_SCRIPT_SURFACE
    case CAIRO_SURFACE_TYPE_SCRIPT:
      return make_refptr_for_instance<ScriptSurface>(new ScriptSurface(surface, false));
      break;
#endif
#if CAIRO_HAS_WIN32_SURFACE
    case CAIRO_SURFACE_TYPE_WIN32:
      return wrap_surface_win32(surface);
      break;
#endif
#if CAIRO_HAS_SVG_SURFACE
    case CAIRO_SURFACE_TYPE_SVG:
      return make_refptr_for_instance<SvgSurface>(new SvgSurface(surface, false /* does not have reference */));
      break;
#endif
#if CAIRO_HAS_XCB_SURFACE
    case CAIRO_SURFACE_TYPE_XCB:
      return make_refptr_for_instance<XcbSurface>(new XcbSurface(surface, false /* does not have reference */));
      break;
#endif

    // the following surfaces are not directly supported in cairomm yet
    case CAIRO_SURFACE_TYPE_DIRECTFB:
    case CAIRO_SURFACE_TYPE_OS2:
    case CAIRO_SURFACE_TYPE_BEOS:
    default:
      return make_refptr_for_instance<Surface>(new Surface(surface, false /* does not have reference */));
  }
}

RefPtr<Surface> Context::get_target()
{
  auto surface = cairo_get_target(const_cast<cobject*>(cobj()));
  check_object_status_and_throw_exception(*this);
  return get_surface_wrapper (surface);
}

RefPtr<const Surface> Context::get_target() const
{
  auto surface = cairo_get_target(const_cast<cobject*>(cobj()));
  check_object_status_and_throw_exception(*this);
  return get_surface_wrapper(surface);
}

#ifndef CAIROMM_DISABLE_DEPRECATED
Path* Context::copy_path() const
{
  auto cresult = cairo_copy_path(const_cast<cobject*>(cobj()));
  check_object_status_and_throw_exception(*this);
  return new Path(cresult, true /* take ownership */); //The caller must delete it.
}
#endif //CAIROMM_DISABLE_DEPRECATED

RefPtr<Path> Context::copy_path2() const
{
  return get_path_wrapper(cairo_copy_path(const_cast<cobject*>(cobj())));
}

void Context::get_path_extents(double& x1, double& y1, double& x2, double& y2) const
{
  cairo_path_extents(const_cast<cobject*>(cobj()), &x1, &y1, &x2, &y2);
  check_object_status_and_throw_exception(*this);
}

#ifndef CAIROMM_DISABLE_DEPRECATED
Path* Context::copy_path_flat() const
{
  auto cresult = cairo_copy_path_flat(const_cast<cobject*>(cobj()));
  check_object_status_and_throw_exception(*this);
  return new Path(cresult, true /* take ownership */); //The caller must delete it.
}
#endif //CAIROMM_DISABLE_DEPRECATED

RefPtr<Path> Context::copy_path_flat2() const
{
  return get_path_wrapper(cairo_copy_path_flat(const_cast<cobject*>(cobj())));
}

void Context::append_path(const Path& path)
{
  cairo_append_path(cobj(), const_cast<cairo_path_t*>(path.cobj()));
  check_object_status_and_throw_exception(*this);
}

void Context::push_group()
{
  cairo_push_group(cobj());
  check_object_status_and_throw_exception(*this);
}

void Context::push_group_with_content(Content content)
{
  cairo_push_group_with_content(cobj(), static_cast<cairo_content_t>(content));
  check_object_status_and_throw_exception(*this);
}

RefPtr<Pattern> Context::pop_group()
{
  auto pattern = cairo_pop_group(cobj());
  check_object_status_and_throw_exception(*this);
  return get_pattern_wrapper(pattern);
}

void Context::pop_group_to_source()
{
  cairo_pop_group_to_source(cobj());
  check_object_status_and_throw_exception(*this);
}

RefPtr<Surface> Context::get_group_target()
{
  auto surface = cairo_get_group_target(cobj());
  // surface can be NULL if you're not between push/pop group calls
  if(!surface)
  {
    // FIXME: is this really the right way to handle this?
    throw_exception(CAIRO_STATUS_NULL_POINTER);
  }

  return get_surface_wrapper(surface);
}

RefPtr<const Surface> Context::get_group_target() const
{
  auto surface = cairo_get_group_target(const_cast<cobject*>(cobj()));
  // surface can be NULL if you're not between push/pop group calls
  if(!surface)
  {
    // FIXME: is this really the right way to handle this?
    throw_exception(CAIRO_STATUS_NULL_POINTER);
  }

  return get_surface_wrapper(surface);
}

SaveGuard::SaveGuard(const RefPtr<Context>& context)
: ctx_{context}
{
  if (ctx_)
    ctx_->save();
}

SaveGuard::~SaveGuard()
{
  if (ctx_)
    ctx_->restore();
}

} //namespace Cairo
