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

#ifndef __CAIROMM_CONTEXT_H
#define __CAIROMM_CONTEXT_H

#include <vector>
#include <utility>
#include <cairomm/surface.h>
#include <cairomm/fontface.h>
#include <cairomm/matrix.h>
#include <cairomm/pattern.h>
#include <cairomm/path.h>
#include <cairomm/scaledfont.h>
#include <cairomm/types.h>
#include <valarray>
#include <vector>
#include <cairo.h>


namespace Cairo
{

/**
 * Context is the main class used to draw in cairomm. It contains the current
 * state of the rendering device, including coordinates of yet to be drawn 
 * shapes.
 * 
 * In the simplest case, create a Context with its target Surface, set its
 * drawing options (line width, color, etc), create shapes with methods like
 * move_to() and line_to(), and then draw the shapes to the Surface using
 * methods such as stroke() or fill().
 *
 * Context is a reference-counted object that should be used via Cairo::RefPtr.
 */
class CAIROMM_API Context
{
protected:
  explicit Context(const RefPtr<Surface>& target);

public:

  /**
   * Operator is used to set the compositing operator for all cairo
   * drawing operations.
   *
   * The default operator is Cairo::Context::Operator::OVER.
   *
   * The operators marked as @a unbounded modify their destination even outside
   * of the mask layer (that is, their effect is not bound by the mask layer).
   * However, their effect can still be limited by way of clipping.
   *
   * To keep things simple, the operator descriptions here document the behavior
   * for when both source and destination are either fully transparent or fully
   * opaque. The actual implementation works for translucent layers too. For a
   * more detailed explanation of the effects of each operator, including the
   * mathematical definitions, see
   * <a href="http://cairographics.org/operators/">this</a>
   *
   */
  enum class Operator
  {
      /**
       * Clear destination layer (bounded)
       */
      CLEAR = CAIRO_OPERATOR_CLEAR,

      /**
       * Replace destination layer (bounded)
       */
      SOURCE = CAIRO_OPERATOR_SOURCE,

      /**
       * Draw source layer on top of destination layer (bounded)
       */
      OVER = CAIRO_OPERATOR_OVER,

      /**
       * Draw source where there was destination content (unbounded)
       */
      IN = CAIRO_OPERATOR_IN,

      /**
       * Draw source where there was no destination content (unbounded)
       */
      OUT = CAIRO_OPERATOR_OUT,

      /**
       * Draw source on top of destination content and only there
       */
      ATOP = CAIRO_OPERATOR_ATOP,

      /**
       * Ignore the source
       */
      DEST = CAIRO_OPERATOR_DEST,

      /**
       * Draw destination on top of source
       */
      DEST_OVER = CAIRO_OPERATOR_DEST_OVER,

      /**
       * Leave destination only where there was source content (unbounded)
       */
      DEST_IN = CAIRO_OPERATOR_DEST_IN,

      /**
       * Leave destination only where there was no source content
       */
      DEST_OUT = CAIRO_OPERATOR_DEST_OUT,

      /**
       * Leave destination on top of source content and only there (unbounded)
       */
      DEST_ATOP = CAIRO_OPERATOR_DEST_ATOP,

      /**
       * Source and destination are shown where there is only one of them
       */
      XOR = CAIRO_OPERATOR_XOR,

      /**
       * Source and destination layers are accumulated
       */
      ADD = CAIRO_OPERATOR_ADD,

      /**
       * Like over, but assuming source and dest are disjoint geometries
       */
      SATURATE = CAIRO_OPERATOR_SATURATE
  };

  /**
   * FillRule is used to select how paths are filled. For both fill rules,
   * whether or not a point is included in the fill is determined by taking a ray
   * from that point to infinity and looking at intersections with the path. The
   * ray can be in any direction, as long as it doesn't pass through the end
   * point of a segment or have a tricky intersection such as intersecting
   * tangent to the path. (Note that filling is not actually implemented in this
   * way. This is just a description of the rule that is applied.)
   *
   * The default fill rule is Cairo::Context::FillRule::WINDING.
   *
   * New entries may be added in future versions.
   */
  enum class FillRule
  {
      /**
       * If the path crosses the ray from left-to-right, counts +1. If the path
       * crosses the ray from right to left, counts -1. (Left and right are
       * determined from the perspective of looking along the ray from the
       * starting point.) If the total count is non-zero, the point will be
       * filled.
       */
      WINDING = CAIRO_FILL_RULE_WINDING,

      /**
       * Counts the total number of intersections, without regard to the
       * orientation of the contour. If the total number of intersections is odd,
       * the point will be filled.
       */
      EVEN_ODD = CAIRO_FILL_RULE_EVEN_ODD
  };

  /**
   * Specifies how to render the endpoints of the path when stroking.
   *
   * The default line cap style is Cairo::Context::LineCap::BUTT.
   */
  enum class LineCap
  {
      /**
       * Start(stop) the line exactly at the start(end) point
       */
      BUTT = CAIRO_LINE_CAP_BUTT,

      /**
       * Use a round ending, the center of the circle is the end point
       */
      ROUND = CAIRO_LINE_CAP_ROUND,

      /**
       * Use a squared ending, the center of the square is the end point
       */
      SQUARE = CAIRO_LINE_CAP_SQUARE
  };

  /**
   * Specifies how to render the junction of two lines when stroking.
   *
   * The default line join style is Cairo::Context::LineJoin::MITER.
   */
  enum class LineJoin
  {
      /**
       * Use a sharp (angled) corner, see Context::set_miter_limit()
       */
      MITER = CAIRO_LINE_JOIN_MITER,

      /**
       * Use a rounded join, the center of the circle is the joint point
       */
      ROUND = CAIRO_LINE_JOIN_ROUND,

      /**
       * Use cut-off join, the join is cut off at half the line width from the
       * join point
       */
      BEVEL = CAIRO_LINE_JOIN_BEVEL
  };

  /** Create a C++ wrapper for the C instance. This C++ instance should then be
   * given to a RefPtr.
   *
   * @param cobject The C instance.
   * @param has_reference Whether we already have a reference. Otherwise, the
   * constructor will take an extra reference.
   */
  explicit Context(cairo_t* cobject, bool has_reference = false);

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  static RefPtr<Context> create(const RefPtr<Surface>& target);

  virtual ~Context();

  /** Makes a copy of the current state of the Context and saves it on an
   * internal stack of saved states. When restore() is called, it will be
   * restored to the saved state. Multiple calls to save() and restore() can be
   * nested; each call to restore() restores the state from the matching paired
   * save().
   *
   * It isn't necessary to clear all saved states before a cairo_t is freed.
   * Any saved states will be freed when the Context is destroyed.
   *
   * @sa restore(), SaveGuard
   */
  void save();

  /** Restores cr to the state saved by a preceding call to save() and removes
   * that state from the stack of saved states.
   *
   * @sa save(), SaveGuard
   */
  void restore();

  /** Sets the compositing operator to be used for all drawing operations. See
   * Operator for details on the semantics of each available compositing
   * operator.
   *
   * @param op	a compositing operator, specified as a Operator
   */
  void set_operator(Operator op);

  /** Sets the source pattern within the Context to source. This Pattern will
   * then be used for any subsequent drawing operation until a new source
   * pattern is set.
   *
   * Note: The Pattern's transformation matrix will be locked to the user space
   * in effect at the time of set_source(). This means that further
   * modifications of the current transformation matrix will not affect the
   * source pattern.
   *
   * @param source	a Pattern to be used as the source for subsequent drawing
   * operations.
   *
   * @sa Pattern::set_matrix()
   * @sa set_source_rgb()
   * @sa set_source_rgba()
   * @sa set_source(const RefPtr<Surface>& surface, double x, double y)
   */
  void set_source(const RefPtr<const Pattern>& source);

  /** Sets the source pattern within the Context to an opaque color. This
   * opaque color will then be used for any subsequent drawing operation until
   * a new source pattern is set.
   *
   * The color components are floating point numbers in the range 0 to 1. If
   * the values passed in are outside that range, they will be clamped.
   *
   * @param red	red component of color
   * @param green	green component of color
   * @param blue	blue component of color
   *
   * @sa set_source_rgba()
   * @sa set_source()
   */
  void set_source_rgb(double red, double green, double blue);

  /** Sets the source pattern within the Context to a translucent color. This
   * color will then be used for any subsequent drawing operation until a new
   * source pattern is set.
   *
   * The color and alpha components are floating point numbers in the range 0
   * to 1. If the values passed in are outside that range, they will be
   * clamped.
   *
   * @param red	red component of color
   * @param green	green component of color
   * @param blue	blue component of color
   * @param alpha	alpha component of color
   *
   * @sa set_source_rgb()
   * @sa set_source()
   */
  void set_source_rgba(double red, double green, double blue, double alpha);

  /** This is a convenience function for creating a pattern from a Surface and
   * setting it as the source
   *
   * The x and y parameters give the user-space coordinate at which the Surface
   * origin should appear. (The Surface origin is its upper-left corner before
   * any transformation has been applied.) The x and y patterns are negated and
   * then set as translation values in the pattern matrix.
   *
   * Other than the initial translation pattern matrix, as described above, all
   * other pattern attributes, (such as its extend mode), are set to the
   * default values as in Context::create(const RefPtr<Surface>& target). The
   * resulting pattern can be queried with get_source() so that these
   * attributes can be modified if desired, (eg. to create a repeating pattern
   * with Pattern::set_extend()).
   *
   * @param surface  	a Surface to be used to set the source pattern
   * @param x  	User-space X coordinate for surface origin
   * @param y  	User-space Y coordinate for surface origin
   */
  void set_source(const RefPtr<Surface>& surface, double x, double y);

  /** Sets the tolerance used when converting paths into trapezoids. Curved
   * segments of the path will be subdivided until the maximum deviation
   * between the original path and the polygonal approximation is less than
   * tolerance. The default value is 0.1. A larger value will give better
   * performance, a smaller value, better appearance. (Reducing the value from
   * the default value of 0.1 is unlikely to improve appearance significantly.)
   * The accuracy of paths within Cairo is limited by the precision of its 
   * internal arithmetic, and the prescribed @tolerance is restricted to the 
   * smallest representable internal value.
   * 
   * @param tolerance	the tolerance, in device units (typically pixels)
   */
  void set_tolerance(double tolerance);

  /** Set the antialiasing mode of the rasterizer used for drawing shapes. This
   * value is a hint, and a particular backend may or may not support a
   * particular value. At the current time, no backend supports
   * Cairo::ANTIALIAS_SUBPIXEL when drawing shapes.
   *
   * Note that this option does not affect text rendering, instead see
   * FontOptions::set_antialias().
   *
   * @param antialias	the new antialiasing mode
   */
  void set_antialias(Antialias antialias);

  /** Set the current fill rule within the cairo Context. The fill rule is used
   * to determine which regions are inside or outside a complex (potentially
   * self-intersecting) path. The current fill rule affects both fill() and
   * clip(). See FillRule for details on the semantics of each available fill
   * rule.
   *
   * The default fill rule is Cairo::FILL_RULE_WINDING.
   * 
   * @param fill_rule	a fill rule, specified as a FillRule
   */
  void set_fill_rule(FillRule fill_rule);

  /** Sets the current line width within the cairo Context. The line width
   * specifies the diameter of a pen that is circular in user-space, (though 
   * device-space pen may be an ellipse in general due to scaling/shear/rotation 
   * of the CTM).
   *
   * Note: When the description above refers to user space and CTM it refers to
   * the user space and CTM in effect at the time of the stroking operation,
   * not the user space and CTM in effect at the time of the call to
   * set_line_width(). The simplest usage makes both of these spaces
   * identical. That is, if there is no change to the CTM between a call to
   * set_line_width() and the stroking operation, then one can just pass
   * user-space values to set_line_width() and ignore this note.
   *
   * As with the other stroke parameters, the current line cap style is
   * examined by stroke(), stroke_extents(), and stroke_to_path(), but does not
   * have any effect during path construction.
   * 
   * The default line width value is 2.0.
   *
   * @param width	a line width, as a user-space value
   */
  void set_line_width(double width);

  /** Sets the current line cap style within the cairo Context. See
   * LineCap for details about how the available line cap styles are drawn.
   *
   * As with the other stroke parameters, the current line cap style is
   * examined by stroke(), stroke_extents(), and stroke_to_path(), but does not
   * have any effect during path construction.
   * 
   * The default line cap style is Cairo::Context::LineCap::BUTT.
   *
   * @param line_cap	a line cap style, as a LineCap
   */
  void set_line_cap(LineCap line_cap);

  /** Sets the current line join style within the cairo Context. See LineJoin
   * for details about how the available line join styles are drawn.
   *
   * As with the other stroke parameters, the current line join style is
   * examined by stroke(), stroke_extents(), and stroke_to_path(), but does not
   * have any effect during path construction.
   *
   * The default line join style is Cairo::Context::LineJoin::MITER.
   *
   * @param line_join	a line joint style, as a LineJoin
   */
  void set_line_join(LineJoin line_join);

  /**
   * Alternate version of set_dash().  You'll probably want to use the one that
   * takes a std::vector argument instead.
   */
  void set_dash(const std::valarray<double>& dashes, double offset);

  /** Sets the dash pattern to be used by stroke(). A dash pattern is specified
   * by dashes, an array of positive values. Each value provides the user-space
   * length of altenate "on" and "off" portions of the stroke. The offset
   * specifies an offset into the pattern at which the stroke begins.
   *
   * Each "on" segment will have caps applied as if the segment were a separate
   * sub-path. In particular, it is valid to use an "on" length of 0.0 with
   * Cairo::Context::LineCap::ROUND or Cairo::Context::LineCap::SQUARE in order to
   * distribute dots or squares along a path.
   *
   * Note: The length values are in user-space units as evaluated at the time
   * of stroking. This is not necessarily the same as the user space at the
   * time of set_dash().
   *
   * If dashes is empty dashing is disabled. If the size of dashes is 1, a
   * symmetric pattern is assumed with alternating on and off portions of the
   * size specified by the single value in dashes.
   *
   * It is invalid for any value in dashes to be negative, or for all values to
   * be 0.  If this is the case, an exception will be thrown
   *
   * @param dashes	an array specifying alternate lengths of on and off portions
   * @param offset	an offset into the dash pattern at which the stroke should start
   *
   * @exception
   */
  void set_dash(const std::vector<double>& dashes, double offset);

  /** This function disables a dash pattern that was set with set_dash()
   */
  void unset_dash();
  
  /**
   * Sets the current miter limit within the cairo context.
   *
   * If the current line join style is set to Cairo::Context::LineJoin::MITER (see
   * set_line_join()), the miter limit is used to determine whether the lines
   * should be joined with a bevel instead of a miter. Cairo divides the length
   * of the miter by the line width. If the result is greater than the miter
   * limit, the style is converted to a bevel.
   *
   * As with the other stroke parameters, the current line miter limit is
   * examined by stroke(), stroke_extents(), and stroke_to_path(), but does not
   * have any effect during path construction.
   *
   * The default miter limit value is 10.0, which will convert joins with
   * interior angles less than 11 degrees to bevels instead of miters. For
   * reference, a miter limit of 2.0 makes the miter cutoff at 60 degrees, and
   * a miter limit of 1.414 makes the cutoff at 90 degrees.
   *
   * A miter limit for a desired angle can be computed as: miter_limit =
   * 1/sin(angle/2)
   *
   * @param limit miter limit to set
   */
  void set_miter_limit(double limit);

  /** Modifies the current transformation matrix (CTM) by translating the
   * user-space origin by (tx, ty). This offset is interpreted as a user-space
   * coordinate according to the CTM in place before the new call to
   * translate. In other words, the translation of the user-space origin
   * takes place after any existing transformation.
   *
   * @param tx	amount to translate in the X direction
   * @param ty	amount to translate in the Y direction
   */
  void translate(double tx, double ty);

  /** Modifies the current transformation matrix (CTM) by scaling the X and Y
   * user-space axes by sx and sy respectively. The scaling of the axes takes
   * place after any existing transformation of user space.
   *
   * @param sx	scale factor for the X dimension
   * @param sy	scale factor for the Y dimension
   */
  void scale(double sx, double sy);

  /** Modifies the current transformation matrix (CTM) by rotating the
   * user-space axes by angle radians. The rotation of the axes takes places
   * after any existing transformation of user space. The rotation direction
   * for positive angles is from the positive X axis toward the positive Y
   * axis.
   *
   * @param angle	angle (in radians) by which the user-space axes will be
   * rotated
   */
  void rotate(double angle_radians);

  /** A convenience wrapper around rotate() that accepts angles in degrees
   *
   * @param angle_degrees angle (in degrees) by which the user-space axes
   * should be rotated
   */
  void rotate_degrees(double angle_degres);

  /** Modifies the current transformation matrix (CTM) by applying matrix as an
   * additional transformation. The new transformation of user space takes
   * place after any existing transformation.
   *
   * @param matrix	a transformation to be applied to the user-space axes
   */
  void transform(const Matrix& matrix);

  /** Modifies the current transformation matrix (CTM) by setting it equal to
   * matrix.
   *
   * @param matrix	a transformation matrix from user space to device space
   */
  void set_matrix(const Matrix& matrix);

  /** Resets the current transformation matrix (CTM) by setting it equal to the
   * identity matrix. That is, the user-space and device-space axes will be
   * aligned and one user-space unit will transform to one device-space unit.
   */
  void set_identity_matrix();

  /** Transform a coordinate from user space to device space by multiplying the
   * given point by the current transformation matrix (CTM).
   *
   * @param x	X value of coordinate (in/out parameter)
   * @param y	Y value of coordinate (in/out parameter)
   */
  void user_to_device(double& x, double& y) const;

  /** Transform a distance vector from user space to device space. This
   * function is similar to user_to_device() except that the translation
   * components of the CTM will be ignored when transforming (dx,dy).
   *
   * @param dx	X component of a distance vector (in/out parameter)
   * @param dy	Y component of a distance vector (in/out parameter)
   */
  void user_to_device_distance(double& dx, double& dy) const;

  /** Transform a coordinate from device space to user space by multiplying the
   * given point by the inverse of the current transformation matrix (CTM).
   *
   * @param x	X value of coordinate (in/out parameter)
   * @param y	Y value of coordinate (in/out parameter)
   */
  void device_to_user(double& x, double& y) const;

  /** Transform a distance vector from device space to user space. This
   * function is similar to device_to_user() except that the translation
   * components of the inverse CTM will be ignored when transforming (dx,dy).
   *
   * @param dx	X component of a distance vector (in/out parameter)
   * @param dy	Y component of a distance vector (in/out parameter)
   */
  void device_to_user_distance(double& dx, double& dy) const;

  /** Clears the current path. After this call there will be no current point.
   */
  void begin_new_path();

  /** Begin a new subpath. Note that the existing path is not affected. After
   * this call there will be no current point.
   *
   * In many cases, this call is not needed since new subpaths are frequently
   * started with move_to().
   *
   * A call to begin_new_sub_path() is particularly useful when beginning a new
   * subpath with one of the arc() calls. This makes things easier as it is no
   * longer necessary to manually compute the arc's initial coordinates for a
   * call to move_to().
   *
   * @since 1.2
   */
  void begin_new_sub_path();

  /** If the current subpath is not empty, begin a new subpath. After this call
   * the current point will be (x, y).
   *
   * @param x	the X coordinate of the new position
   * @param y	the Y coordinate of the new position
   */
  void move_to(double x, double y);

  /** Adds a line to the path from the current point to position (x, y) in
   * user-space coordinates. After this call the current point will be (x, y).
   * 
   * If there is no current point before the call to line_to()
   * this function will behave as move_to(x, y).
   *
   * @param x	the X coordinate of the end of the new line
   * @param y	the Y coordinate of the end of the new line
   */
  void line_to(double x, double y);

  /** Adds a cubic Bezier spline to the path from the current point to position
   * (x3, y3) in user-space coordinates, using (x1, y1) and (x2, y2) as the
   * control points. After this call the current point will be (x3, y3).
   *
   * If there is no current point before the call to curve_to()
   * this function will behave as if preceded by a call to
   * move_to(x1, y1).
   * 
   * @param x1	the X coordinate of the first control point
   * @param y1	the Y coordinate of the first control point
   * @param x2	the X coordinate of the second control point
   * @param y2	the Y coordinate of the second control point
   * @param x3	the X coordinate of the end of the curve
   * @param y3	the Y coordinate of the end of the curve
   */
  void curve_to(double x1, double y1, double x2, double y2, double x3, double y3);

  /** Adds a circular arc of the given radius to the current path. The arc is
   * centered at (@a xc, @a yc), begins at @a angle1 and proceeds in the direction of
   * increasing angles to end at @a angle2. If @a angle2 is less than @a angle1 it will
   * be progressively increased by 2*M_PI until it is greater than @a angle1.
   *
   * If there is a current point, an initial line segment will be added to the
   * path to connect the current point to the beginning of the arc. If this
   * initial line is undesired, it can be avoided by calling
   * begin_new_sub_path() before calling arc().
   *
   * Angles are measured in radians. An angle of 0 is in the direction of the
   * positive X axis (in user-space). An angle of M_PI/2.0 radians (90 degrees) is
   * in the direction of the positive Y axis (in user-space). Angles increase
   * in the direction from the positive X axis toward the positive Y axis. So
   * with the default transformation matrix, angles increase in a clockwise
   * direction.
   *
   * ( To convert from degrees to radians, use degrees * (M_PI / 180.0). )
   *
   * This function gives the arc in the direction of increasing angles; see
   * arc_negative() to get the arc in the direction of decreasing angles.
   *
   * The arc is circular in user-space. To achieve an elliptical arc, you can
   * scale the current transformation matrix by different amounts in the X and
   * Y directions. For example, to draw an ellipse in the box given by x, y,
   * width, height:
   *
   * @code
   * context->save();
   * context->translate(x, y);
   * context->scale(width / 2.0, height / 2.0);
   * context->arc(0.0, 0.0, 1.0, 0.0, 2 * M_PI);
   * context->restore();
   * @endcode
   *
   * @param xc	X position of the center of the arc
   * @param yc	Y position of the center of the arc
   * @param radius	the radius of the arc
   * @param angle1	the start angle, in radians
   * @param angle2	the end angle, in radians
   */
  void arc(double xc, double yc, double radius, double angle1, double angle2);

  /** Adds a circular arc of the given @a radius to the current path. The arc is
   * centered at (@a xc, @a yc), begins at @a angle1 and proceeds in the direction of
   * decreasing angles to end at @a angle2. If @a angle2 is greater than @a angle1 it
   * will be progressively decreased by 2*M_PI until it is greater than @a angle1.
   *
   * See arc() for more details. This function differs only in the direction of
   * the arc between the two angles.
   *
   * @param xc	X position of the center of the arc
   * @param yc	Y position of the center of the arc
   * @param radius	the radius of the arc
   * @param angle1	the start angle, in radians
   * @param angle2	the end angle, in radians
   */
  void arc_negative(double xc, double yc, double radius, double angle1, double angle2);

  /** If the current subpath is not empty, begin a new subpath. After this call
   * the current point will offset by (x, y).
   *
   * Given a current point of (x, y),
   * @code
   * rel_move_to(dx, dy)
   * @endcode
   * is logically equivalent to
   * @code
   * move_to(x + dx, y + dy)
   * @endcode
   *
   * @param dx	the X offset
   * @param dy	the Y offset
   *
   * It is an error to call this function with no current point. Doing
   * so will cause this to shutdown with a status of
   * CAIRO_STATUS_NO_CURRENT_POINT. Cairomm will then throw an exception.
   */
  void rel_move_to(double dx, double dy);

  /** Relative-coordinate version of line_to(). Adds a line to the path from
   * the current point to a point that is offset from the current point by (dx,
   * dy) in user space. After this call the current point will be offset by
   * (dx, dy).
   *
   * Given a current point of (x, y),
   * @code
   * rel_line_to(dx, dy)
   * @endcode
   * is logically equivalent to
   * @code
   * line_to(x + dx, y + dy).
   * @endcode
   *
   * @param dx	the X offset to the end of the new line
   * @param dy	the Y offset to the end of the new line
   *
   * It is an error to call this function with no current point. Doing
   * so will cause this to shutdown with a status of
   * CAIRO_STATUS_NO_CURRENT_POINT. Cairomm will then throw an exception.
   */
  void rel_line_to(double dx, double dy);

  /** Relative-coordinate version of curve_to(). All offsets are relative to
   * the current point. Adds a cubic Bezier spline to the path from the current
   * point to a point offset from the current point by (dx3, dy3), using points
   * offset by (dx1, dy1) and (dx2, dy2) as the control points.  After this
   * call the current point will be offset by (dx3, dy3).
   *
   * Given a current point of (x, y),
   * @code
   * rel_curve_to(dx1, dy1, dx2, dy2, dx3, dy3)
   * @endcode
   * is logically equivalent to
   * @code
   * curve_to(x + dx1, y + dy1, x + dx2, y + dy2, x + dx3, y + dy3).
   * @endcode
   *
   * @param dx1	the X offset to the first control point
   * @param dy1	the Y offset to the first control point
   * @param dx2	the X offset to the second control point
   * @param dy2	the Y offset to the second control point
   * @param dx3	the X offset to the end of the curve
   * @param dy3	the Y offset to the end of the curve
   *
   * It is an error to call this function with no current point. Doing
   * so will cause this to shutdown with a status of
   * CAIRO_STATUS_NO_CURRENT_POINT. Cairomm will then throw an exception.
   */
  void rel_curve_to(double dx1, double dy1, double dx2, double dy2, double dx3, double dy3);

  /** Adds a closed-subpath rectangle of the given size to the current path at
   * position (x, y) in user-space coordinates.
   *
   * This function is logically equivalent to:
   *
   * @code
   * context->move_to(x, y);
   * context->rel_line_to(width, 0);
   * context->rel_line_to(0, height);
   * context->rel_line_to(-width, 0);
   * context->close_path();
   * @endcode
   *
   * @param x	the X coordinate of the top left corner of the rectangle
   * @param y	the Y coordinate to the top left corner of the rectangle
   * @param width	the width of the rectangle
   * @param height	the height of the rectangle
   */
  void rectangle(double x, double y, double width, double height);

  /** Adds a line segment to the path from the current point to the beginning
   * of the current subpath, (the most recent point passed to move_to()), and
   * closes this subpath. After this call the current point will be at the 
   * joined endpoint of the sub-path.
   *
   * The behavior of close_path() is distinct from simply calling line_to()
   * with the equivalent coordinate in the case of stroking.  When a closed
   * subpath is stroked, there are no caps on the ends of the subpath. Instead,
   * there is a line join connecting the final and initial segments of the
   * subpath.
   * 
   * If there is no current point before the call to close_path(),
   * this function will have no effect.
   *
   */
  void close_path();

  /** A drawing operator that paints the current source everywhere within the
   * current clip region.
   */
  void paint();

  /** A drawing operator that paints the current source everywhere within the
   * current clip region using a mask of constant alpha value alpha. The effect
   * is similar to paint(), but the drawing is faded out using the alpha
   * value.
   *
   * @param alpha	an alpha value, between 0 (transparent) and 1 (opaque)
   */
  void paint_with_alpha(double alpha);

  /** A drawing operator that paints the current source using the alpha channel
   * of pattern as a mask. (Opaque areas of mask are painted with the source,
   * transparent areas are not painted.)
   *
   * @param pattern a Pattern
   */
  void mask(const RefPtr<const Pattern>& pattern);

  /** A drawing operator that paints the current source using the alpha channel
   * of surface as a mask. (Opaque areas of surface are painted with the
   * source, transparent areas are not painted.)
   *
   * @param surface	a Surface
   * @param surface_x	X coordinate at which to place the origin of surface
   * @param surface_y	Y coordinate at which to place the origin of surface
   */
  void mask(const RefPtr<const Surface>& surface, double surface_x, double surface_y);

  /** A drawing operator that strokes the current Path according to the current
   * line width, line join, line cap, and dash settings. After stroke(),
   * the current Path will be cleared from the cairo Context.
   *
   * @sa set_line_width()
   * @sa set_line_join()
   * @sa set_line_cap()
   * @sa set_dash()
   * @sa stroke_preserve().
   * 
   * Note: Degenerate segments and sub-paths are treated specially and
   * provide a useful result. These can result in two different
   * situations:
   *
   * 1. Zero-length "on" segments set in set_dash(). If the cap style is
   * Cairo::Context::LineCap::ROUND or Cairo::Context::LineCap::SQUARE then these segments will
   * be drawn as circular dots or squares respectively. In the case of
   * Cairo::Context::LineCap::SQUARE, the orientation of the squares is determined by
   * the direction of the underlying path.
   *
   * 2. A sub-path created by move_to() followed by either a close_path() or
   * one or more calls to line_to() to the same coordinate as the move_to(). If
   * the cap style is Cairo::Context::LineCap::ROUND then these sub-paths will be drawn
   * as circular dots. Note that in the case of Cairo::Context::LineCap::SQUARE a
   * degenerate sub-path will not be drawn at all, (since the correct
   * orientation is indeterminate).
   *
   * In no case will a cap style of Cairo::Context::LineCap::BUTT cause anything to be
   * drawn in the case of either degenerate segments or sub-paths.
   */
  void stroke();

  /** A drawing operator that strokes the current Path according to the current
   * line width, line join, line cap, and dash settings. Unlike stroke(),
   * stroke_preserve() preserves the Path within the cairo Context.
   *
   * @sa set_line_width()
   * @sa set_line_join()
   * @sa set_line_cap()
   * @sa set_dash()
   * @sa stroke_preserve().
   */
  void stroke_preserve();

  /** A drawing operator that fills the current path according to the current
   * fill rule, (each sub-path is implicitly closed before being filled). After
   * fill(), the current path will be cleared from the cairo context.
   *
   * @sa set_fill_rule()
   * @sa fill_preserve()
   */
  void fill();

  /** A drawing operator that fills the current path according to the current
   * fill rule, (each sub-path is implicitly closed before being filled).
   * Unlike fill(), fill_preserve() preserves the path within the
   * cairo Context.
   *
   * @sa set_fill_rule()
   * @sa fill().
   */
  void fill_preserve();
  
  /**
   * Emits the current page for backends that support multiple pages, but
   * doesn't clear it, so, the contents of the current page will be retained
   * for the next page too.  Use show_page() if you want to get an
   * empty page after the emission.
   *
   * This is a convenience function that simply calls Surface::copy_page() on
   * @a cr's target.
   */
  void copy_page();
  
  /**
   * Emits and clears the current page for backends that support multiple
   * pages.  Use copy_page() if you don't want to clear the page.
   *
   * This is a convenience function that simply calls
   * Surface::show_page() on @a cr's target.
   */
  void show_page();
  
  /**
   * Tests whether the given point is inside the area that would be
   * affected by a stroke() operation given the current path and
   * stroking parameters. Surface dimensions and clipping are not taken
   * into account.
   *
   * @param x X coordinate of the point to test
   * @param y Y coordinate of the point to test
   * @returns A non-zero value if the point is inside, or zero if outside.
   *
   * @sa stroke()
   * @sa set_line_width()
   * @sa set_line_join()
   * @sa set_line_cap()
   * @sa set_dash()
   * @sa stroke_preserve().
   *
   */
  bool in_stroke(double x, double y) const;

  /**
   * Tests whether the given point is inside the area that would be
   * affected by a fill() operation given the current path and
   * filling parameters. Surface dimensions and clipping are not taken
   * into account.
   *
   * @param x X coordinate of the point to test
   * @param y Y coordinate of the point to test
   * @returns A non-zero value if the point is inside, or zero if outside.
   *
   * @sa fill()
   * @sa set_fill_rule()
   * @sa fill_preserve()
   */
  bool in_fill(double x, double y) const;

  /**
   * Tests whether the given point is inside the area that would be visible
   * through the current clip, i.e. the area that would be filled by a paint()
   * operation.
   *
   * Return value: A non-zero value if the point is inside, or zero if outside.
   *
   * @param x X coordinate of the point to test
   * @param y Y coordinate of the point to test
   *
   * @sa clip()
   * @sa clip_preserve()
   *
   * @since 1.10
   */
  bool in_clip(double x, double y) const;

  /**
   * Computes a bounding box in user coordinates covering the area that would
   * be affected, (the "inked" area), by a stroke() operation given the current
   * path and stroke parameters. If the current path is empty, returns an empty
   * rectangle ((0,0), (0,0)). Surface dimensions and clipping are not taken
   * into account.
   *
   * Note that if the line width is set to exactly zero, then stroke_extents()
   * will return an empty rectangle. Contrast with path_extents() which can be
   * used to compute the non-empty bounds as the line width approaches zero.
   *
   * Note that stroke_extents() must necessarily do more work to compute the
   * precise inked areas in light of the stroke parameters, so path_extents()
   * may be more desirable for sake of performance if non-inked path extents
   * are desired.
   *
   * @param x1 left of the resulting extents
   * @param y1 top of the resulting extents
   * @param x2 right of the resulting extents
   * @param y2 bottom of the resulting extents
   *
   * @sa stroke()
   * @sa set_line_width()
   * @sa set_line_join()
   * @sa set_line_cap()
   * @sa set_dash()
   * @sa stroke_preserve()
   */
  void get_stroke_extents(double& x1, double& y1, double& x2, double& y2) const;

  /**
   * Computes a bounding box in user coordinates covering the area that would
   * be affected, (the "inked" area), by a fill() operation given the current
   * path and fill parameters. If the current path is empty, returns an empty
   * rectangle ((0,0), (0,0)). Surface dimensions and clipping are not taken
   * into account.
   *
   * Contrast with path_extents(), which is similar, but returns non-zero
   * extents for some paths with no inked area, (such as a simple line
   * segment).
   *
   * Note that fill_extents() must necessarily do more work to compute the
   * precise inked areas in light of the fill rule, so path_extents() may be
   * more desirable for sake of performance if the non-inked path extents are
   * desired.
   *
   * @param x1 left of the resulting extents
   * @param y1 top of the resulting extents
   * @param x2 right of the resulting extents
   * @param y2 bottom of the resulting extents
   *
   * @sa fill()
   * @sa set_fill_rule()
   * @sa full_preserve()
   */
  void get_fill_extents(double& x1, double& y1, double& x2, double& y2) const;

  /** Reset the current clip region to its original, unrestricted state. That
   * is, set the clip region to an infinitely large shape containing the target
   * surface. Equivalently, if infinity is too hard to grasp, one can imagine
   * the clip region being reset to the exact bounds of the target surface.
   *
   * Note that code meant to be reusable should not call reset_clip() as it
   * will cause results unexpected by higher-level code which calls clip().
   * Consider using save() and restore() around clip() as a more robust means
   * of temporarily restricting the clip region.
   */
  void reset_clip();

  /** Establishes a new clip region by intersecting the current clip region
   * with the current Path as it would be filled by fill() and according to the
   * current fill rule.
   *
   * After clip(), the current path will be cleared from the cairo Context.
   *
   * The current clip region affects all drawing operations by effectively
   * masking out any changes to the surface that are outside the current clip
   * region.
   *
   * Calling clip() can only make the clip region smaller, never larger.  But
   * the current clip is part of the graphics state, so a temporary restriction
   * of the clip region can be achieved by calling clip() within a
   * save()/restore() pair. The only other means of increasing the size of the
   * clip region is reset_clip().
   *
   * @sa set_fill_rule()
   */
  void clip();

  /** Establishes a new clip region by intersecting the current clip region
   * with the current path as it would be filled by fill() and according to the
   * current fill rule.
   *
   * Unlike clip(), clip_preserve preserves the path within the cairo
   * Context.
   *
   * @sa clip()
   * @sa set_fill_rule()
   */
  void clip_preserve();

  /**
   * Computes a bounding box in user coordinates covering the area inside the
   * current clip.
   *
   * @param x1 left of the resulting extents
   * @param y1 top of the resulting extents
   * @param x2 right of the resulting extents
   * @param y2 bottom of the resulting extents
   *
   * @since 1.4
   */
  void get_clip_extents(double& x1, double& y1, double& x2, double& y2) const;

  /**
   * Returns the current clip region as a list of rectangles in user coordinates.
   *
   * This function will throw an exception if the clip region cannot be
   * represented as a list of user-space rectangles.
   *
   * @param rectangles a vector to store the rectangles into
   *
   * @exception
   *
   * @since 1.4
   */
  void copy_clip_rectangle_list(std::vector<Rectangle>& rectangles) const;

  /**
   * Selects a family and style of font from a simplified description as a
   * family name, slant and weight. Cairo provides no operation to list
   * available family names on the system (this is a "toy", remember), but the
   * standard CSS2 generic family names, ("serif", "sans-serif", "cursive",
   * "fantasy", "monospace"), are likely to work as expected.
   *
   * Note: The select_font_face() function call is part of what the cairo
   * designers call the "toy" text API. It is convenient for short demos and
   * simple programs, but it is not expected to be adequate for serious
   * text-using applications.
   *
   * If @a family starts with the string "@cairo:", or if no native font
   * backends are compiled in, cairo will use an internal font family. The
   * internal font family recognizes many modifiers in the @family string, most
   * notably, it recognizes the string "monospace". That is, the family name
   * "@cairo:monospace" will use the monospace version of the internal font
   * family.
   *
   * For "real" font selection, see the font-backend-specific
   * Cairo::FontFace::create functions for the font backend you are using. (For
   * example, if you are using the freetype-based cairo-ft font backend, see
   * Cairo::FtFontFace::create().) The resulting font face could then be used
   * with Cairo::ScaledFont::create() and set_scaled_font().
   *
   * Similarly, when using the "real" font support, you can call directly into
   * the underlying font system, (such as fontconfig or freetype), for
   * operations such as listing available fonts, etc.
   *
   * It is expected that most applications will need to use a more
   * comprehensive font handling and text layout library, (for example, pango),
   * in conjunction with cairo.
   *
   * If text is drawn without a call to select_font_face(), (nor
   * set_font_face() nor set_scaled_font()), the default family is
   * platform-specific, but is essentially "sans-serif". Default slant is
   * Cairo::FONT_SLANT_NORMAL, and default weight is Cairo::FONT_WEIGHT_NORMAL.
   *
   * This function is equivalent to a call to Cairo::ToyFontFace::create()
   * followed by set_font_face().
   *
   * @param family a font family name, encoded in UTF-8
   * @param slant the slant for the font
   * @param weight the weight for the font
   */
  void select_font_face(const std::string& family, ToyFontFace::Slant slant, ToyFontFace::Weight weight);

  /**
   * Sets the current font matrix to a scale by a factor of @a size, replacing
   * any font matrix previously set with set_font_size() or set_font_matrix().
   * This results in a font size of @a size user space units. (More precisely,
   * this matrix will result in the font's em-square being a @size by @a size
   * square in user space.)
   *
   * If text is drawn without a call to set_font_size(), (nor set_font_matrix()
   * nor set_scaled_font()), the default font size is 10.0.
   *
   * @param size the new font size, in user space units)
   */
  void set_font_size(double size);

  /**
   * Sets the current font matrix to @matrix. The font matrix gives a
   * transformation from the design space of the font (in this space, the
   * em-square is 1 unit by 1 unit) to user space. Normally, a simple scale is
   * used (see set_font_size()), but a more complex font matrix can be used to
   * shear the font or stretch it unequally along the two axes
   *
   * @param matrix a Cairo::Matrix describing a transform to be applied to the
   * current font.
   */
  void set_font_matrix(const Matrix& matrix);

  /**
   * Returns the current font matrix
   *
   * @param matrix a Cairo::Matrix to store the results into (in/out parameter)
   * @sa set_font_matrix()
   */
  void get_font_matrix(Matrix& matrix) const;

  /**
   * Sets a set of custom font rendering options. Rendering options are derived
   * by merging these options with the options derived from underlying surface;
   * if the value in @a options has a default value (like
   * Cairo::ANTIALIAS_DEFAULT), then the value from the surface is used.
   *
   * @param options font options to use
   */
  void set_font_options(const FontOptions& options);

  /**
   * Retrieves font rendering options set via set_font_options(). Note that the
   * returned options do not include any options derived from the underlying
   * surface; they are literally the options passed to set_font_options().
   *
   * @param options a FontOptions object into which to store the retrieved
   *   options. All existing values are overwritten
   * @since 1.8
   */
  void get_font_options(FontOptions& options) const;

  /**
   * Replaces the current font face, font matrix, and font options in the
   * context with those of the @a scaled_font. Except for some translation, the
   * current CTM of the context should be the same as that of the
   * #cairo_scaled_font_t, which can be accessed using
   * Cairo::ScaledFont::get_ctm().
   *
   * @param scaled_font a scaled font
   * @since 1.8
   */
  void set_scaled_font(const RefPtr<const ScaledFont>& scaled_font);

  /** Gets the current scaled font.
   *
   * @since 1.8
   */
  RefPtr<ScaledFont> get_scaled_font();

  /**
   * A drawing operator that generates the shape from a string of UTF-8
   * characters, rendered according to the current font_face, font_size
   * (font_matrix), and font_options.
   *
   * This function first computes a set of glyphs for the string of text. The
   * first glyph is placed so that its origin is at the current point. The
   * origin of each subsequent glyph is offset from that of the previous glyph
   * by the advance values of the previous glyph.
   *
   * After this call the current point is moved to the origin of where the
   * next glyph would be placed in this same progression. That is, the current
   * point will be at the origin of the final glyph offset by its advance
   * values. This allows for easy display of a single logical string with
   * multiple calls to show_text().
   *
   * Note: The show_text() function call is part of what the cairo
   * designers call the "toy" text API. It is convenient for short demos and
   * simple programs, but it is not expected to be adequate for serious
   * text-using applications. See show_glyphs() for the "real" text
   * display API in cairo.
   *
   * @param utf8 a string containing text encoded in UTF-8
   */
  void show_text(const std::string& utf8);

  /**
   * A drawing operator that generates the shape from an array of glyphs,
   * rendered according to the current font face, font size (font matrix), and
   * font options.
   *
   * @param glyphs vector of glyphs to show
   * @param num_glyphs number of glyphs to show
   */
  void show_glyphs(const std::vector<Glyph>& glyphs);

  /**
   * This operation has rendering effects similar to show_glyphs() but, if the
   * target surface supports it, uses the provided text and cluster mapping to
   * embed the text for the glyphs shown in the output. If the target does not
   * support the extended attributes, this function acts like the basic
   * show_glyphs() as if it had been passed @a glyphs and @a num_glyphs.
   *
   * The mapping between @a utf8 and @a glyphs is provided by an array of
   * <firstterm>clusters</firstterm>. Each cluster covers a number of text
   * bytes and glyphs, and neighboring clusters cover neighboring areas of @a
   * utf8 and @a glyphs. The clusters should collectively cover @a utf8 and @a
   * glyphs in entirety.
   *
   * The first cluster always covers bytes from the beginning of @a utf8. If @a
   * cluster_flags do not have the Cairo::TEXT_CLUSTER_FLAG_BACKWARD set, the
   * first cluster also covers the beginning of @a glyphs, otherwise it covers
   * the end of the @a glyphs array and following clusters move backward.
   *
   * See Cairo::TextCluster for constraints on valid clusters.
   *
   * @param utf8: a string of text encoded in UTF-8
   * @param glyphs: vector of glyphs to show
   * @param clusters: vector of cluster mapping information
   * @param cluster_flags: cluster mapping flags
   *
   * @since 1.8
   */
  void show_text_glyphs(const std::string& utf8,
                        const std::vector<Glyph>& glyphs,
                        const std::vector<TextCluster>& clusters,
                        TextClusterFlags cluster_flags);
  /// @{
  /** Gets the current font face
   */
  RefPtr<FontFace> get_font_face();
  RefPtr<const FontFace> get_font_face() const;
  /// @}

  /**
   * Gets the font extents for the currently selected font.
   *
   * @param extents a Cairo::FontExtents object
   */
  void get_font_extents(FontExtents& extents) const;

  /**
   * Replaces the current font face in the context with @a font_face
   * @a font_face. The replaced font face in the context will be destroyed if
   * there are no other references to it.
   *
   * @param font_face a font face
   */
  //FIXME: C API acceps NULL to restore the default font. Does C++ API support that?
  void set_font_face(const RefPtr<const FontFace>& font_face);

  /**
   * Gets the extents for a string of text. The extents describe a user-space
   * rectangle that encloses the "inked" portion of the text, (as it would be
   * drawn by show_text()). Additionally, the x_advance and y_advance values
   * indicate the amount by which the current point would be advanced by
   * show_text().
   *
   * Note that whitespace characters do not directly contribute to the size of
   * the rectangle (extents.width and extents.height). They do contribute
   * indirectly by changing the position of non-whitespace characters. In
   * particular, trailing whitespace characters are likely to not affect the
   * size of the rectangle, though they will affect the x_advance and y_advance
   * values.
   *
   * @param utf8 a string of text encoded in UTF-8
   * @param extents a TextExtents object
   */
  void get_text_extents(const std::string& utf8, TextExtents& extents) const;

  /**
   * Gets the extents for an array of glyphs. The extents describe a user-space
   * rectangle that encloses the "inked" portion of the glyphs, (as they would
   * be drawn by show_glyphs()). Additionally, the x_advance and y_advance
   * values indicate the amount by which the current point would be advanced by
   * show_glyphs().
   *
   * Note that whitespace glyphs do not contribute to the size of the rectangle
   * (extents.width and extents.height).
   *
   * @param glyphs a vector of glyphs
   * @param extents a TextExtents object
   */
  void get_glyph_extents(const std::vector<Glyph>& glyphs, TextExtents& extents) const;

  /**
   * Adds closed paths for text to the current path. The generated path if
   * filled, achieves an effect similar to that of show_text().
   *
   * Text conversion and positioning is done similar to show_text().
   *
   * Like show_text(), After this call the current point is moved to the origin
   * of where the next glyph would be placed in this same progression. That is,
   * the current point will be at the origin of the final glyph offset by its
   * advance values. This allows for chaining multiple calls to to text_path()
   * without having to set current point in between.
   *
   * Note: The text_path() function call is part of what the cairo designers
   * call the "toy" text API. It is convenient for short demos and simple
   * programs, but it is not expected to be adequate for serious text-using
   * applications. See glyph_path() for the "real" text path API in cairo.
   *
   * @param utf8 a string of text encoded in UTF-8
   */
  void text_path(const std::string& utf8);

  /** Adds closed paths for the glyphs to the current path. The generated path
   * if filled, achieves an effect similar to that of show_glyphs().
   *
   * @param glyphs a vector of glyphs
   */
  void glyph_path(const std::vector<Glyph>& glyphs);

  /** Gets the current compositing operator for a cairo Context
   */
  Operator get_operator() const;

  /// @{
  /** Gets the current source pattern for the %Context
   */
  RefPtr<Pattern> get_source();
  RefPtr<const Pattern> get_source() const;

  /** Gets the current source surface pattern for the %Context, if any.
   *
   * @returns The source pattern, if it is a surface pattern,
   *          else an empty RefPtr.
   */
  RefPtr<SurfacePattern> get_source_for_surface();
  RefPtr<const SurfacePattern> get_source_for_surface() const;
  /// @}

  /** Gets the current tolerance value, as set by set_tolerance()
   */
  double get_tolerance() const;

  /** Gets the current shape antialiasing mode, as set by set_antialias()
   */
  Antialias get_antialias() const;

  /** Gets the current point of the current path, which is conceptually the
   * final point reached by the path so far.
   *
   * The current point is returned in the user-space coordinate system. If
   * there is no defined current point then x and y will both be set to 0.0. It
   * is possible to check this in advance with has_current_point().
   *
   * Most path construction functions alter the current point. See the
   * following for details on how they affect the current point: clear_path(),
   * move_to(), line_to(), curve_to(), arc(), rel_move_to(), rel_line_to(),
   * rel_curve_to(), arc(), and text_path()
   *
   * Some functions use and alter the current point but do not otherwise change
   * current path: show_text().
   *
   * Some functions unset the current path and as a result, current point:
   * fill(), stroke().
   *
   * @param x	return value for X coordinate of the current point
   * @param y	return value for Y coordinate of the current point
   *
   * @sa has_current_point()
   */
  void get_current_point (double& x, double& y) const;

  /**
   * Checks if there is a current point defined. See get_current_point() for
   * details on the current point.
   *
   * @returns @c true if a current point is defined.
   *
   * @since 1.6
   */
  bool has_current_point() const;

  /** Gets the current fill rule, as set by set_fill_rule().
   */
  FillRule get_fill_rule() const;

  /**
   * Gets the current line width, as set by set_line_width(). Note that the
   * value is unchanged even if the CTM has changed between the calls to
   * set_line_width() and get_line_width().
   */
  double get_line_width() const;

  /** Gets the current line cap style, as set by set_line_cap()
   */
  LineCap get_line_cap() const;

  /** Gets the current line join style, as set by set_line_join()
   */
  LineJoin get_line_join() const;

  /** Gets the current miter limit, as set by set_miter_limit()
   */
  double get_miter_limit() const;

  /**
   * Gets the current dash array and offset.
   *
   * @param dashes return value for the dash array.
   * @param offset return value for the current dash offset.
   *
   * @since 1.4
   */
  void get_dash(std::vector<double>& dashes, double& offset) const;


  /** Stores the current transformation matrix (CTM) into matrix.
   *
   * @param matrix return value for the matrix
   */
  void get_matrix(Matrix& matrix);

  /**
   * Returns the current transformation matrix (CTM)
   * @since 1.8
   */
  Matrix get_matrix() const;

  /// @{
  /** Gets the target surface associated with this Context.
   *
   * @exception
   */
  RefPtr<Surface> get_target();
  RefPtr<const Surface> get_target() const;
  /// @}

#ifndef CAIROMM_DISABLE_DEPRECATED
  /** Creates a copy of the current path and returns it to the user.
   *
   * @todo See cairo_path_data_t for hints on how to iterate over the returned
   * data structure.
   *
   * @note The caller owns the Path object returned from this function.  The
   * Path object must be freed when you are finished with it.
   *
   * @deprecated 1.20: Use copy_path2() instead.
   */
  Path* copy_path() const;
#endif //CAIROMM_DISABLE_DEPRECATED

  /** Creates a copy of the current path and returns it to the user.
   *
   * @throws std::bad_alloc, Cairo::logic_error, std::ios_base::failure
   * @newin{1,20}
   */
  RefPtr<Path> copy_path2() const;

  /**
   * Computes a bounding box in user-space coordinates covering the points on
   * the current path. If the current path is empty, returns an empty rectangle
   * ((0,0), (0,0)). Stroke parameters, fill rule, surface dimensions and
   * clipping are not taken into account.
   *
   * Contrast with fill_extents() and stroke_extents() which return the extents
   * of only the area that would be "inked" by the corresponding drawing
   * operations.
   *
   * The result of path_extents() is defined as equivalent to the limit of
   * stroke_extents() with Cairo::Context::LineCap::ROUND as the line width
   * approaches 0.0, (but never reaching the empty-rectangle returned by
   * stroke_extents() for a line width of 0.0).
   *
   * Specifically, this means that zero-area sub-paths such as
   * move_to();line_to() segments, (even degenerate cases where the coordinates
   * to both calls are identical), will be considered as contributing to the
   * extents. However, a lone move_to() will not contribute to the results of
   * path_extents().
   *
   * @param x1 left of the resulting extents
   * @param y1 top of the resulting extents
   * @param x2 right of the resulting extents
   * @param y2 bottom of the resulting extents
   *
   * @since 1.6
   */
  void get_path_extents(double& x1, double& y1, double& x2, double& y2) const;

#ifndef CAIROMM_DISABLE_DEPRECATED
  /** Gets a flattened copy of the current path and returns it to the user
   *
   * @todo See cairo_path_data_t for hints on how to iterate over the returned
   * data structure.
   *
   * This function is like copy_path() except that any curves in the path will
   * be approximated with piecewise-linear approximations, (accurate to within
   * the current tolerance value). That is, the result is guaranteed to not have
   * any elements of type CAIRO_PATH_CURVE_TO which will instead be
   * replaced by a series of CAIRO_PATH_LINE_TO elements.
   *
   * @note The caller owns the Path object returned from this function.  The
   * Path object must be freed when you are finished with it.
   *
   * @deprecated 1.20: Use copy_path_flat2() instead.
   */
  Path* copy_path_flat() const;
#endif //CAIROMM_DISABLE_DEPRECATED

  /** Gets a flattened copy of the current path and returns it to the user.
   *
   * This function is like copy_path2() except that any curves in the path will
   * be approximated with piecewise-linear approximations, (accurate to within
   * the current tolerance value). That is, the result is guaranteed to not have
   * any elements of type Path::ElementType::CURVE_TO which will instead be
   * replaced by a series of Path::ElementType::LINE_TO elements.
   *
   * @throws std::bad_alloc, Cairo::logic_error, std::ios_base::failure
   * @newin{1,20}
   */
  RefPtr<Path> copy_path_flat2() const;

  /** Append the path onto the current path. The path may be either the return
   * value from one of copy_path() or copy_path_flat() or it may be constructed
   * manually.
   *
   * @param path Path to be appended
   */
  void append_path(const Path& path);

  /** Temporarily redirects drawing to an intermediate surface known as a group.
   * The redirection lasts until the group is completed by a call to pop_group()
   * or pop_group_to_source(). These calls provide the result of any drawing to
   * the group as a pattern, (either as an explicit object, or set as the source
   * pattern).
   *
   * This group functionality can be convenient for performing intermediate
   * compositing. One common use of a group is to render objects as opaque
   * within the group, (so that they occlude each other), and then blend the
   * result with translucence onto the destination.
   *
   * Groups can be nested arbitrarily deep by making balanced calls to
   * push_group()/pop_group(). Each call pushes/pops the new target group
   * onto/from a stack.
   *
   * The push_group() function calls save() so that any changes to the graphics
   * state will not be visible outside the group, (the pop_group functions call
   * restore()).
   *
   * By default the intermediate group will have a content type of
   * CONTENT_COLOR_ALPHA. Other content types can be chosen for the group by
   * using push_group_with_content() instead.
   *
   * As an example, here is how one might fill and stroke a path with
   * translucence, but without any portion of the fill being visible under the
   * stroke:
   *
   * @code
   * cr->push_group();
   * cr->set_source(fill_pattern);
   * cr->fill_preserve();
   * cr->set_source(stroke_pattern);
   * cr->stroke();
   * cr->pop_group_to_source();
   * cr->paint_with_alpha(alpha);
   * @endcode
   *
   * @since 1.2
   */
  void push_group();

  /**
   * Temporarily redirects drawing to an intermediate surface known as a
   * group. The redirection lasts until the group is completed by a call
   * to pop_group() or pop_group_to_source(). These calls provide the result of
   * any drawing to the group as a pattern, (either as an explicit object, or set
   * as the source pattern).
   *
   * The group will have a content type of @content. The ability to control this
   * content type is the only distinction between this function and push_group()
   * which you should see for a more detailed description of group rendering.
   *
   * @param content indicates the type of group that will be created
   *
   * @since 1.2
   */
  void push_group_with_content(Content content);

  /**
   * Terminates the redirection begun by a call to push_group() or
   * push_group_with_content() and returns a new pattern containing the results
   * of all drawing operations performed to the group.
   *
   * The pop_group() function calls restore(), (balancing a call to save() by
   * the push_group function), so that any changes to the graphics state will
   * not be visible outside the group.
   *
   * @return a (surface) pattern containing the results of all drawing
   * operations performed to the group.
   *
   * @since 1.2
   */
  RefPtr<Pattern> pop_group();

  /**
   * Terminates the redirection begun by a call to push_group() or
   * push_group_with_content() and installs the resulting pattern as the source
   * pattern in the given cairo Context.
   *
   * The behavior of this function is equivalent to the sequence of operations:
   *
   * @code
   * RefPtr<Pattern> group = cr->pop_group();
   * cr->set_source(group);
   * @endcode
   *
   * but is more convenient as their is no need for a variable to store
   * the short-lived pointer to the pattern.
   *
   * The pop_group() function calls restore(), (balancing a call to save() by
   * the push_group function), so that any changes to the graphics state will
   * not be visible outside the group.
   *
   * @since 1.2
   */
  void pop_group_to_source();

  /**
   * Gets the target surface for the current group as started by the most recent
   * call to push_group() or push_group_with_content().
   *
   * This function will return NULL if called "outside" of any group rendering
   * blocks, (that is, after the last balancing call to pop_group() or
   * pop_group_to_source()).
   *
   * @exception
   *
   * @since 1.2
   */
  RefPtr<Surface> get_group_target();

  /** Same as the non-const version but returns a reference to a const Surface
   *
   * @since 1.2
   */
  RefPtr<const Surface> get_group_target() const;

  /** The base cairo C type that is wrapped by Cairo::Context
   */
  typedef cairo_t cobject;

  /** Gets a pointer to the base C type that is wrapped by the Context
   */
  inline cobject* cobj() { return m_cobject; }

  /** Gets a pointer to the base C type that is wrapped by the Context
   */
  inline const cobject* cobj() const { return m_cobject; }

#ifndef DOXYGEN_IGNORE_THIS
  ///For use only by the cairomm implementation.
  inline ErrorStatus get_status() const
  { return cairo_status(const_cast<cairo_t*>(cobj())); }

  void reference() const;
  void unreference() const;
#endif //DOXYGEN_IGNORE_THIS

protected:
  cobject* m_cobject;
};

/** RAII-style context save/restore class.
 * Cairo::Context::save() is called automatically when the object is created,
 * and Cairo::Context::restore() is called when the object is destroyed.
 * This allows you to write code such as:
 * @code
 * // context initial state
 * {
 *   Cairo::SaveGuard saver(context);
 *   ... // manipulate context
 * }
 * // context is restored to initial state
 * @endcode
 *
 * @newin{1,18}
 */
class SaveGuard final
{
public:
  /// Constructor, the context is saved.
  CAIROMM_API explicit SaveGuard(const RefPtr<Context>& context);

#ifndef DOXYGEN_IGNORE_THIS
  // noncopyable
  SaveGuard(const SaveGuard&) = delete;
  SaveGuard& operator=(const SaveGuard&) = delete;
  // nonmovable
  SaveGuard(SaveGuard&&) = delete;
  SaveGuard& operator=(SaveGuard&&) = delete;
#endif //DOXYGEN_IGNORE_THIS

  /// Destructor, the context is restored.
  CAIROMM_API ~SaveGuard();

private:
  RefPtr<Context> ctx_;
};

} // namespace Cairo

#endif //__CAIROMM_CONTEXT_H
