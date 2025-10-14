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

#ifndef __CAIROMM_MESH_PATTERN_H
#define __CAIROMM_MESH_PATTERN_H

#include <cairomm/pattern.h>
#include <cairomm/path.h>

namespace Cairo
{
/**
 * Mesh patterns are tensor-product patch meshes (type 7 shadings in PDF). Mesh
 * patterns may also be used to create other types of shadings that are special
 * cases of tensor-product patch meshes such as Coons patch meshes (type 6
 * shading in PDF) and Gouraud-shaded triangle meshes (type 4 and 5 shadings in
 * PDF).
 *
 * Mesh patterns consist of one or more tensor-product patches, which should be
 * defined before using the mesh pattern. Using a mesh pattern with a partially
 * defined patch as source or mask will put the context in an error status with
 * a status of CAIRO_STATUS_INVALID_MESH_CONSTRUCTION.
 *
 * A tensor-product patch is defined by 4 Bézier curves (side 0, 1, 2, 3) and
 * by 4 additional control points (P0, P1, P2, P3) that provide further control
 * over the patch and complete the definition of the tensor-product patch. The
 * corner C0 is the first point of the patch.
 *
 * Degenerate sides are permitted so straight lines may be used. A zero length
 * line on one side may be used to create 3 sided patches.
 *
 *
 *          C1     Side 1       C2
 *           +---------------+
 *           |               |
 *           |  P1       P2  |
 *           |               |
 *    Side 0 |               | Side 2
 *           |               |
 *           |               |
 *           |  P0       P3  |
 *           |               |
 *           +---------------+
 *         C0     Side 3        C3
 *
 * Each patch is constructed by first calling @c MeshPattern::begin_patch(),
 * then @c MeshPattern::move_to() to specify the first point in the patch
 * (C0). Then the sides are specified with calls to
 * @c MeshPattern::curve_to() and @c MeshPattern::line_to().
 *
 * The four additional control points (P0, P1, P2, P3) in a patch can be
 * specified with @c MeshPattern::set_control_point().
 *
 * At each corner of the patch (C0, C1, C2, C3) a color may be specified with
 * @c MeshPattern::set_corner_color_rgb() or @c
 * MeshPattern::set_corner_color_rgba(). Any corner whose color is not
 * explicitly specified defaults to transparent black.
 *
 * A Coons patch is a special case of the tensor-product patch where the
 * control points are implicitly defined by the sides of the patch. The default
 * value for any control point not specified is the implicit value for a Coons
 * patch, i.e. if no control points are specified the patch is a Coons patch.
 *
 * A triangle is a special case of the tensor-product patch where the control
 * points are implicitly defined by the sides of the patch, all the sides are
 * lines and one of them has length 0, i.e. if the patch is specified using
 * just 3 lines, it is a triangle. If the corners connected by the 0-length
 * side have the same color, the patch is a Gouraud-shaded triangle.
 *
 * Patches may be oriented differently to the above diagram. For example the
 * first point could be at the top left. The diagram only shows the
 * relationship between the sides, corners and control points. Regardless of
 * where the first point is located, when specifying colors, corner 0 will
 * always be the first point, corner 1 the point between side 0 and side 1 etc.
 *
 * Calling cairo_mesh_pattern_end_patch() completes the current patch. If less
 * than 4 sides have been defined, the first missing side is defined as a line
 * from the current point to the first point of the patch (C0) and the other
 * sides are degenerate lines from C0 to C0. The corners between the added
 * sides will all be coincident with C0 of the patch and their color will be
 * set to be the same as the color of C0.
 *
 * Additional patches may be added with additional calls to
 * @c MeshPattern::begin_patch() and @c MeshPattern::end_patch().
 *
 * @code
Cairo::RefPtr<Cairo::MeshPattern> pattern = Cairo::MeshPattern::create();

// Add a Coons patch
pattern->begin_patch();
pattern->move_to(0, 0);
pattern->curve_to(30, -30,  60,  30, 100, 0);
pattern->curve_to(60,  30, 130,  60, 100, 100);
pattern->curve_to(60,  70,  30, 130,   0, 100);
pattern->curve_to(30,  70, -30,  30,   0, 0);
pattern->set_corner_color_rgb(0, 1, 0, 0);
pattern->set_corner_color_rgb(1, 0, 1, 0);
pattern->set_corner_color_rgb(2, 0, 0, 1);
pattern->set_corner_color_rgb(3, 1, 1, 0);
pattern->end_patch();

// Add a Gouraud-shaded triangle
pattern->begin_patch()
pattern->move_to(100, 100);
pattern->line_to(130, 130);
pattern->line_to(130,  70);
pattern->set_corner_color_rgb(0, 1, 0, 0);
pattern->set_corner_color_rgb(1, 0, 1, 0);
pattern->set_corner_color_rgb(2, 0, 0, 1);
pattern->end_patch()
 * @endcode
 *
 * When two patches overlap, the last one that has been added is drawn over the
 * first one.
 *
 * When a patch folds over itself, points are sorted depending on their
 * parameter coordinates inside the patch. The v coordinate ranges from 0 to 1
 * when moving from side 3 to side 1; the u coordinate ranges from 0 to 1 when
 * going from side 0 to side 2. Points with higher v coordinate hide points
 * with lower v coordinate. When two points have the same v coordinate, the one
 * with higher u coordinate is above. This means that points nearer to side 1
 * are above points nearer to side 3; when this is not sufficient to decide
 * which point is above (for example when both points belong to side 1 or side
 * 3) points nearer to side 2 are above points nearer to side 0.
 *
 * For a complete definition of tensor-product patches, see the PDF
 * specification (ISO32000), which describes the parametrization in detail.
 *
 * @note The coordinates are always in pattern space. For a new pattern,
 * pattern space is identical to user space, but the relationship between the
 * spaces can be changed with @c Pattern::set_matrix().
 *
 * @newin{1,20}
 */
class CAIROMM_API MeshPattern : public Pattern
{
public:
  /**
   * Create a C++ wrapper for the C instance. This C++ instance should then be
   * given to a RefPtr.
   *
   * @param cobject The C instance.
   * @param has_reference whether we already have a reference. Otherwise, the
   * constructor will take an extra reference.
   *
   * @newin{1,20}
   */
  explicit MeshPattern(cairo_pattern_t* cobject, bool has_reference = false);

  ~MeshPattern() override;

  /**
   * Begin a patch in a mesh pattern.
   *
   * After calling this function, the patch shape should be defined with
   * @c MeshPattern::move_to(), @c MeshPattern::line_to() and
   * @c MeshPattern::curve_to().
   *
   * After defining the patch, MeshPattern::end_patch() must be called
   * before using pattern as a source or mask.
   *
   * @throws Cairo::logic_error if the pattern already has a current patch.
   *
   * @newin{1,20}
   */
  void begin_patch();

  /**
   * Indicates the end of the current patch in a mesh pattern.
   *
   * If the current patch has less than 4 sides, it is closed with a straight
   * line from the current point to the first point of the patch as if
   * @c MeshPattern::line_to() was used.
   *
   * @throws Cairo::logic_error if the pattern has no current patch or the
   * current patch has no current point.
   *
   * @newin{1,20}
   */
  void end_patch();

  /**
   * Define the first point of the current patch in a mesh pattern.
   *
   * After this call the current point will be (x, y).
   *
   * @param x, y the X and Y coordinates of the new position
   *
   * @throws Cairo::logic_error if the pattern has no current patch or the
   * current patch already has at least one side.
   *
   * @newin{1,20}
   */
  void move_to(double x, double y);

  /**
   * Adds a line to the current patch from the current point to position (x, y)
   * in pattern-space coordinates.
   *
   * If there is no current point before the call to @c MeshPattern::line_to()
   * this function will behave as @c MeshPattern::move_to(x, y).
   *
   * After this call the current point will be (x, y).
   *
   * @param x, y the X and Y coordinates of the end of the new line
   *
   * @throws Cairo::logic_error if the  pattern has no current patch or the
   * current patch already has 4 sides.
   *
   * @newin{1,20}
   */
  void line_to(double x, double y);

  /**
   * Adds a cubic Bézier spline to the current patch from the current point to
   * position (x3, y3) in pattern-space coordinates, using (x1, y1) and (x2,
   * y2) as the control points.
   *
   * If the current patch has no current point before the call to
   * @c MeshPattern::curve_to(), this function will behave as if preceded by
   * a call to @c MeshPattern::move_to(x1, y1).
   *
   * After this call the current point will be (x3, y3).
   *
   * @param x1, y1 the X and Y coordinates of the first control point
   * @param x2, y2 the X and Y coordinates of the second control point
   * @param x3, y3 the X and Y coordinates of the end of the curve
   *
   * @throws Cairo::logic_error if pattern has no current patch or the current
   * patch already has 4 sides.
   *
   * @newin{1,20}
   */
  void curve_to(double x1, double y1, double x2, double y2,
                double x3, double y3);

  /**
   * Sets an internal control point of the current patch.
   *
   * Valid values for point_num are from 0 to 3 and identify the control points.
   *
   * @param point_num the control point to set the position for
   * @param x, y the X and Y corrdinates of the control point
   *
   * @throws Cairo::logic_error if @a point_num is not valid or the pattern has
   * no current patch.
   *
   * @newin{1,20}
   */
  void set_control_point(unsigned int point_num, double x, double y);

  /**
   * Sets the color of a corner of the current patch in a mesh pattern.
   *
   * The color is specified in the same way as in @c set_source_rgb().
   *
   * Valid values for @a corner_num are from 0 to 3 and identify the corners.
   *
   * @param corner_num the corner to set the color for
   * @param red, green, blue the red, green and blue components of the color
   *        respectively.
   * @throws Cairo::logic_error if @a point_num is not valid or the pattern has
   * no current patch.
   *
   * @newin{1,20}
   */
  void set_corner_color_rgb(unsigned int corner_num, double red, double green,
                            double blue);

  /**
   * Sets the color of a corner of the current patch in a mesh pattern.
   *
   * The color is specified in the same way as in @c set_source_rgba().
   *
   * Valid values for @a corner_num are from 0 to 3 and identify the corners.
   *
   * @param corner_num the corner to set the color for
   * @param red, green, blue, alpha the red, green, blue and alpha components
   *        of the color respectively.
   * @throws Cairo::logic_error if @a point_num is not valid or the pattern has
   * no current patch.
   *
   * @newin{1,20}
   */
  void set_corner_color_rgba(unsigned int corner_num, double red, double green,
                             double blue, double alpha);

  /**
   * Returns the number of patches specified in the given mesh pattern.
   *
   * The number only includes patches which have been finished by calling
   * @c MeshPattern::end_patch(). For example it will be 0 during the
   * definition of the first patch.
   *
   * @newin{1,20}
   */
  unsigned int get_patch_count() const;

  /**
   * Returns path defining the patch @a patch_num for the mesh pattern.
   *
   * @a patch_num can range 0 to 1 less than the number returned by @c
   * get_patch_count().
   *
   * @param patch_num the patch number to return data for
   *
   * @throws Cairo::logic_error if @a patch_num or @a point_num is not valid
   *         for the pattern
   *
   * @newin{1,20}
   */
  RefPtr<Path> get_path(unsigned int patch_num) const;

  /**
   * Gets the control point @a point_num of patch @a patch_num for a mesh
   * pattern.
   *
   * @a patch_num can range 0 to 1 less than the number returned by
   * @c get_patch_count().
   *
   * Valid values for @a point_num are from 0 to 3 and identify the control
   * points.
   *
   * @param patch_num the patch number to return data for
   * @param point_num the control point number to return data for
   * @param[out] x, y return value for the X and Y corrdinates of the control point.
   *
   * @throws Cairo::logic_error if @a patch_num or @a point_num is not valid
   *         for the pattern
   *
   * @newin{1,20}
   */
  void get_control_point(unsigned int patch_num, unsigned int point_num,
                         double& x, double& y);

  /**
   * Gets the color information in corner @a corner_num of patch @a patch_num
   * for a mesh pattern.
   *
   * @a patch_num can range 0 to 1 less than the number returned by
   * @c get_patch_count().
   *
   * Valid values for @a point_num are from 0 to 3 and identify the control
   * points.
   *
   * @param patch_num the patch number to return data for
   * @param corner_num the corner number to return data for
   * @param[out] red, green, blue, alpha return value for the red, green, blue and
   *        alpha components of the corner color respectively.
   *
   * @throws Cairo::logic_error if @a patch_num or @a point_num is not valid
   *         for the pattern
   *
   * @newin{1,20}
   */
  void get_corner_color_rgba(unsigned int patch_num, unsigned int corner_num,
                             double& red, double& green, double& blue,
                             double& alpha);

  /**
   * Creates a new mesh pattern.
   *
   * Throws an exception on error.
   *
   * @newin{1,20}
   */
  static RefPtr<MeshPattern> create();
};
/////////////////////////////////////aded by zhhou/////////////////////////////////////////////////
class CAIROMM_API SweepGradient : public MeshPattern{
private:
   double m_cx;
   double m_cy;
   double m_radius;
protected:
   SweepGradient(double,double,double,double angleRadius,const std::vector<ColorStop>&);
public:
   explicit SweepGradient(cairo_pattern_t* cobject, bool has_reference = false);
   void add_sector(double x,double y,double r,double angleRadius,const ColorStop& from,const ColorStop&to);
   static RefPtr<SweepGradient> create(double cx,double cy,double r,double angleRadius,const std::vector<ColorStop>&stopColors);
};
} // namespace Cairo

#endif //__CAIROMM_MESH_PATTERN_H
