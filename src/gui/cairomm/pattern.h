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

#ifndef __CAIROMM_PATTERN_H
#define __CAIROMM_PATTERN_H

#include <cairomm/surface.h>
#include <cairomm/enums.h>
#include <cairo.h>


namespace Cairo
{
struct ColorStop
{
  double offset;
  double red, green, blue, alpha;
};

class CAIROMM_API Matrix;

/**
 * Cairo::Pattern is the paint with which cairo draws. The primary use of
 * patterns is as the source for all cairo drawing operations, although they
 * can also be used as masks, that is, as the brush too.
 *
 * This is a reference-counted object that should be used via Cairo::RefPtr.
 */
class CAIROMM_API Pattern
{
protected:
  //Use derived constructors.

  //TODO?: Pattern(cairo_pattern_t *target);

public:

  /**
   * Type is used to describe the type of a given pattern.
   *
   * The pattern type can be queried with Pattern::get_type().
   *
   * New entries may be added in future versions.
   *
   * @since 1.2
   **/
  enum class Type
  {
      /**
       * The pattern is a solid (uniform) color. It may be opaque or translucent.
       */
      SOLID = CAIRO_PATTERN_TYPE_SOLID,

      /**
       * The pattern is a based on a surface (an image)
       */
      SURFACE = CAIRO_PATTERN_TYPE_SURFACE,

      /**
       * The pattern is a linear gradient.
       */
      LINEAR = CAIRO_PATTERN_TYPE_LINEAR,

      /**
       * The pattern is a radial gradient.
       */
      RADIAL = CAIRO_PATTERN_TYPE_RADIAL
  };

  /**
   * Cairo::Extend is used to describe how pattern color/alpha will be determined
   * for areas "outside" the pattern's natural area, (for example, outside the
   * surface bounds or outside the gradient geometry).
   *
   * Mesh patterns are not affected by the extend mode.
   *
   * The default extend mode is Cairo::Pattern::Extend::NONE for surface patterns and
   * Cairo::Pattern::Extend::PAD for gradient patterns.
   *
   * New entries may be added in future versions.
   **/
  enum class Extend
  {
      /**
       * Pixels outside of the source pattern are fully transparent
       */
      NONE = CAIRO_EXTEND_NONE,

      /**
       * The pattern is tiled by repeating
       */
      REPEAT = CAIRO_EXTEND_REPEAT,

      /**
       * The pattern is tiled by reflecting at the edges (Implemented for surface
       * patterns since 1.6)
       */
      REFLECT = CAIRO_EXTEND_REFLECT,

      /**
       * Pixels outside of the pattern copy the closest pixel from the source
       * (Since 1.2; but only implemented for surface patterns since 1.6)
       */
      PAD = CAIRO_EXTEND_PAD
  };

  /** Create a C++ wrapper for the C instance. This C++ instance should then be given to a RefPtr.
   * @param cobject The C instance.
   * @param has_reference Whether we already have a reference. Otherwise, the constructor will take an extra reference.
   */
  explicit Pattern(cairo_pattern_t* cobject, bool has_reference = false);

  Pattern(const Pattern&) = delete;
  Pattern& operator=(const Pattern&) = delete;

  virtual ~Pattern();

  /**
   * Sets the pattern's transformation matrix to @matrix. This matrix is a
   * transformation from user space to pattern space.
   *
   * When a pattern is first created it always has the identity matrix for its
   * transformation matrix, which means that pattern space is initially
   * identical to user space.
   *
   * Important: Please note that the direction of this transformation matrix is
   * from user space to pattern space. This means that if you imagine the flow
   * from a pattern to user space (and on to device space), then coordinates in
   * that flow will be transformed by the inverse of the pattern matrix.
   *
   * For example, if you want to make a pattern appear twice as large as it
   * does by default the correct code to use is:
   *
   * @code
   * pattern->set_matrix(scaling_matrix(0.5, 0.5));
   * @endcode
   *
   * Meanwhile, using values of 2.0 rather than 0.5 in the code above
   * would cause the pattern to appear at half of its default size.
   *
   * Also, please note the discussion of the user-space locking semantics of
   * set_source().
   **/
  void set_matrix(const Matrix& matrix);

  /**
   * Returns the pattern's transformation matrix
   */
  void get_matrix(Matrix& matrix) const;

  /**
   * Returns the pattern's transformation matrix
   * @since 1.8
   */
  Matrix get_matrix() const;

  /**
   * Returns the type of the pattern
   *
   * @since 1.2
   */
  Type get_type() const;

  /**
   * Sets the mode to be used for drawing outside the area of a pattern. See
   * Cairo::Extend for details on the semantics of each extend strategy.
   *
   * The default extend mode is Cairo::Pattern::Extend::NONE for surface patterns and
   * Cairo::Pattern::Extend::PAD for gradient patterns.
   *
   * @param Cairo::Extend describing how the area outsize of the pattern will
   *   be drawn
   *
   * @since 1.12
   */
  void set_extend(Extend extend);

  /**
   * Gets the current extend mode See Cairo::Extend for details on the
   * semantics of each extend strategy.
   * @since 1.12
   */
  Extend get_extend() const;

  typedef cairo_pattern_t cobject;
  inline cobject* cobj() { return m_cobject; }
  inline const cobject* cobj() const { return m_cobject; }

  #ifndef DOXYGEN_IGNORE_THIS
  ///For use only by the cairomm implementation.
  inline ErrorStatus get_status() const
  { return cairo_pattern_status(const_cast<cairo_pattern_t*>(cobj())); }
  #endif //DOXYGEN_IGNORE_THIS

  void reference() const;
  void unreference() const;

protected:
  //Used by derived types only.
  Pattern();

  cobject* m_cobject;
};

class CAIROMM_API SolidPattern : public Pattern
{
protected:

public:

  /** Create a C++ wrapper for the C instance.
   * @param cobject The C instance.
   * @param has_reference Whether we already have a reference. Otherwise, the constructor will take an extra reference.
   */
  explicit SolidPattern(cairo_pattern_t* cobject, bool has_reference = false);

  /**
   * Gets the solid color for a solid color pattern.
   *
   * @param red return value for red component of color
   * @param green return value for green component of color
   * @param blue return value for blue component of color
   * @param alpha return value for alpha component of color
   *
   * @since 1.4
   **/
  void get_rgba(double& red, double& green,
                double& blue, double& alpha) const;

  /**
   * Creates a new Cairo::Pattern corresponding to an opaque color. The color
   * components are floating point numbers in the range 0 to 1. If the values
   * passed in are outside that range, they will be clamped.
   *
   * @param red red component of the color
   * @param green green component of the color
   * @param blue blue component of the color
   */
  static RefPtr<SolidPattern> create_rgb(double red, double green, double blue);

  /**
   * Creates a new Cairo::Pattern corresponding to a translucent color. The color
   * components are floating point numbers in the range 0 to 1. If the values
   * passed in are outside that range, they will be clamped.
   *
   * @param red red component of the color
   * @param green green component of the color
   * @param blue blue component of the color
   * @param alpha alpha component of the color
   */
  static RefPtr<SolidPattern> create_rgba(double red, double green,
                                          double blue, double alpha);

  //TODO?: SolidPattern(cairo_pattern_t *target);
  ~SolidPattern() override;
};

class CAIROMM_API SurfacePattern : public Pattern
{
protected:

  explicit SurfacePattern(const RefPtr<Surface>& surface);

  //TODO?: SurfacePattern(cairo_pattern_t *target);

public:

  /**
   * Filter is used to indicate what filtering should be applied when
   * reading pixel values from patterns. See Cairo::SurfacePattern::set_filter()
   * for indicating the desired filter to be used with a particular pattern.
   */
  enum class Filter
  {
      /**
       * A high-performance filter, with quality similar to Cairo::Patern::Filter::NEAREST
       */
      FAST = CAIRO_FILTER_FAST,

      /**
       * A reasonable-performance filter, with quality similar to
       * Cairo::BILINEAR
       */
      GOOD = CAIRO_FILTER_GOOD,

      /**
       * The highest-quality available, performance may not be suitable for
       * interactive use.
       */
      BEST = CAIRO_FILTER_BEST,

      /**
       * Nearest-neighbor filtering
       */
      NEAREST = CAIRO_FILTER_NEAREST,

      /**
       * Linear interpolation in two dimensions
       */
      BILINEAR = CAIRO_FILTER_BILINEAR,

      /**
       * This filter value is currently unimplemented, and should not be used in
       * current code.
       */
      GAUSSIAN = CAIRO_FILTER_GAUSSIAN
  };

  /** Create a C++ wrapper for the C instance. This C++ instance should then be given to a RefPtr.
   * @param cobject The C instance.
   * @param has_reference Whether we already have a reference. Otherwise, the constructor will take an extra reference.
   */
  explicit SurfacePattern(cairo_pattern_t* cobject, bool has_reference = false);

  /// @{
  /**
   * Gets the surface associated with this pattern
   *
   * @since 1.4
   **/
  RefPtr<const Surface> get_surface () const;
  RefPtr<Surface> get_surface ();
  /// @}

  ~SurfacePattern() override;

  /**
   * Create a new Cairo::Pattern for the given surface.
   */
  static RefPtr<SurfacePattern> create(const RefPtr<Surface>& surface);

  /**
   * Sets the filter to be used for resizing when using this pattern.
   * See Cairo::Filter for details on each filter.
   *
   * Note that you might want to control filtering even when you do not have an
   * explicit Cairo::Pattern object, (for example when using
   * Cairo::Context::set_source_surface()). In these cases, it is convenient to
   * use Cairo::Context::get_source() to get access to the pattern that cairo
   * creates implicitly.
   *
   * @param filter Cairo::Filter describing the filter to use for resizing the
   *   pattern
   */
  void set_filter(Filter filter);

  /**
   * Gets the current filter for a pattern. See Cairo::Filter for details on
   * each filter.
   */
  Filter get_filter() const;
};

class CAIROMM_API Gradient : public Pattern
{
protected:
  //Use derived constructors.

  //TODO?: Gradient(cairo_pattern_t *target);

public:

  /** Create a C++ wrapper for the C instance. This C++ instance should then be given to a RefPtr.
   * @param cobject The C instance.
   * @param has_reference Whether we already have a reference. Otherwise, the constructor will take an extra reference.
   */
  explicit Gradient(cairo_pattern_t* cobject, bool has_reference = false);

  ~Gradient() override;

  /**
   * Adds an opaque color stop to a gradient pattern. The offset
   * specifies the location along the gradient's control vector. For
   * example, a linear gradient's control vector is from (x0,y0) to
   * (x1,y1) while a radial gradient's control vector is from any point
   * on the start circle to the corresponding point on the end circle.
   *
   * The color is specified in the same way as in Context::set_source_rgb().
   *
   * If two (or more) stops are specified with identical offset values,
   * they will be sorted according to the order in which the stops are
   * added, (stops added earlier will compare less than stops added
   * later). This can be useful for reliably making sharp color
   * transitions instead of the typical blend.
   *
   * @param offset an offset in the range [0.0 .. 1.0]
   * @param red red component of color
   * @param green green component of color
   * @param blue blue component of color
   **/
  void add_color_stop_rgb(double offset, double red, double green, double blue);

  /**
   * Adds a translucent color stop to a gradient pattern. The offset
   * specifies the location along the gradient's control vector. For
   * example, a linear gradient's control vector is from (x0,y0) to
   * (x1,y1) while a radial gradient's control vector is from any point
   * on the start circle to the corresponding point on the end circle.
   *
   * The color is specified in the same way as in Context::set_source_rgba().
   *
   * If two (or more) stops are specified with identical offset values,
   * they will be sorted according to the order in which the stops are
   * added, (stops added earlier will compare less than stops added
   * later). This can be useful for reliably making sharp color
   * transitions instead of the typical blend.
   *
   * @param offset an offset in the range [0.0 .. 1.0]
   * @param red red component of color
   * @param green green component of color
   * @param blue blue component of color
   * @param alpha alpha component of color
   */
  void add_color_stop_rgba(double offset, double red, double green, double blue, double alpha);

  /**
   * Gets the color stops and offsets for this Gradient
   *
   * @since 1.4
   */
  std::vector<ColorStop> get_color_stops() const;


protected:
  Gradient();
};

class CAIROMM_API LinearGradient : public Gradient
{
protected:

  LinearGradient(double x0, double y0, double x1, double y1);

public:

  /** Create a C++ wrapper for the C instance. This C++ instance should then be given to a RefPtr.
   * @param cobject The C instance.
   * @param has_reference Whether we already have a reference. Otherwise, the constructor will take an extra reference.
   */
  explicit LinearGradient(cairo_pattern_t* cobject, bool has_reference = false);

  /**
   * @param x0 return value for the x coordinate of the first point
   * @param y0 return value for the y coordinate of the first point
   * @param x1 return value for the x coordinate of the second point
   * @param y1 return value for the y coordinate of the second point
   *
   * Gets the gradient endpoints for a linear gradient.
   *
   * @since 1.4
   **/
  void get_linear_points(double &x0, double &y0,
                         double &x1, double &y1) const;

  //TODO?: LinearGradient(cairo_pattern_t *target);
  ~LinearGradient() override;

  /**
   * Create a new linear gradient Cairo::Pattern along the line defined by (x0,
   * y0) and (x1, y1). Before using the gradient pattern, a number of color
   * stops should be defined using Cairo::Gradient::add_color_stop_rgb() or
   * Cairo::Gradient::add_color_stop_rgba().
   *
   * Note: The coordinates here are in pattern space. For a new pattern,
   * pattern space is identical to user space, but the relationship between the
   * spaces can be changed with Cairo::Pattern::set_matrix().
   *
   * @param x0 x coordinate of the start point
   * @param y0 y coordinate of the start point
   * @param x1 x coordinate of the end point
   * @param y1 y coordinate of the end point
   */
  static RefPtr<LinearGradient> create(double x0, double y0, double x1, double y1);
};

class CAIROMM_API RadialGradient : public Gradient
{
protected:

  RadialGradient(double cx0, double cy0, double radius0, double cx1, double cy1, double radius1);

public:

  /** Create a C++ wrapper for the C instance. This C++ instance should then be given to a RefPtr.
   * @param cobject The C instance.
   * @param has_reference Whether we already have a reference. Otherwise, the constructor will take an extra reference.
   */
  explicit RadialGradient(cairo_pattern_t* cobject, bool has_reference = false);

  /**
   * Gets the gradient endpoint circles for a radial gradient, each
   * specified as a center coordinate and a radius.
   *
   * @param x0 return value for the x coordinate of the center of the first (inner) circle
   * @param y0 return value for the y coordinate of the center of the first (inner) circle
   * @param r0 return value for the radius of the first (inner) circle
   * @param x1 return value for the x coordinate of the center of the second (outer) circle
   * @param y1 return value for the y coordinate of the center of the second (outer) circle
   * @param r1 return value for the radius of the second (outer) circle
   *
   * @since 1.4
   */
  void get_radial_circles(double& x0, double& y0, double& r0,
                          double& x1, double& y1, double& r1) const;

  //TODO?: RadialGradient(cairo_pattern_t *target);
  ~RadialGradient() override;


  /**
   * Creates a new radial gradient #cairo_pattern_t between the two circles
   * defined by (cx0, cy0, radius0) and (cx1, cy1, radius1). Before using the
   * gradient pattern, a number of color stops should be defined using
   * Cairo::Gradient::add_color_stop_rgb() or
   * Cairo::Gradient::add_color_stop_rgba().
   *
   * @note The coordinates here are in pattern space. For a new pattern,
   * pattern space is identical to user space, but the relationship between the
   * spaces can be changed with Cairo::Pattern::set_matrix().
   *
   * @param cx0 x coordinate for the center of the start circle
   * @param cy0 y coordinate for the center of the start circle
   * @param radius0 radius of the start circle
   * @param cx1 x coordinate for the center of the end circle
   * @param cy1 y coordinate for the center of the end circle
   * @param radius1 radius of the end circle
   */
  static RefPtr<RadialGradient> create(double cx0, double cy0, double radius0, double cx1, double cy1, double radius1);
};

} // namespace Cairo

#endif //__CAIROMM_PATTERN_H

// vim: ts=2 sw=2 et
