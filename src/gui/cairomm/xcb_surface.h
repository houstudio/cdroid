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

#ifndef __CAIROMM_XCB_SURFACE_H
#define __CAIROMM_XCB_SURFACE_H

#include <cairomm/surface.h>

#ifdef CAIRO_HAS_XCB_SURFACE
#include <cairo-xcb.h>
#endif

namespace Cairo {

#ifdef CAIRO_HAS_XCB_SURFACE

/** Creates an XCB surface that draws to the given drawable.
 * The way that colors are represented in the drawable is specified by
 * the provided visual.
 *
 * @note If drawable is a Window, then the function
 * XcbSurface::set_size() must be called whenever the size of the window
 * changes.
 *
 * When drawable is a Window containing child windows then drawing to the
 * created surface will be clipped by those child windows. When the created
 * surface is used as a source, the contents of the children will be included.
 *
 * @note For this Surface to be available, cairo must have been compiled with
 * XCB support.
 *
 * @newin{1,20}
 */
class CAIROMM_API XcbSurface : public Surface
{
public:
  /** Create a C++ wrapper for the C instance.
   * This C++ instance should then be given to a RefPtr.
   *
   * @param cobject The C instance.
   * @param has_reference Whether we already have a reference. Otherwise, the
   * constructor will take an extra reference.
   *
   * @newin{1,20}
   */
  explicit XcbSurface(cairo_surface_t* cobject, bool has_reference = false);
  ~XcbSurface() override;

  /** Informs cairo of the new size of the XCB drawable underlying the surface.
   * For a surface created for a window (rather than a pixmap), this function
   * must be called each time the size of the window changes. (For a subwindow,
   * you are normally resizing the window yourself, but for a toplevel window,
   * it is necessary to listen for ConfigureNotify events.)
   *
   * A pixmap can never change size, so it is never necessary to call this
   * function on a surface created for a pixmap.
   *
   * If Surface::flush() wasn't called, some pending operations might be
   * discarded.
   *
   * Throws an exception on error.
   *
   * @param width The new width of the surface.
   * @param height The new height of the surface.
   *
   * @newin{1,20}
   */
  void set_size(int width, int height);

  /** Informs cairo of the new drawable and size of the XCB drawable underlying
   * the surface.
   *
   * If Surface::flush() wasn't called, some pending operations might be
   * discarded.
   *
   * Throws an exception on error.
   *
   * @param drawable The new drawable of the surface.
   * @param width The new width of the surface.
   * @param height The new height of the surface.
   *
   * @newin{1,20}
   */
  void set_drawable(xcb_drawable_t drawable, int width, int height);

  /** Creates an XCB surface that draws to the given drawable.
   * The way that colors are represented in the drawable is specified by
   * the provided visual.
   *
   * @note If drawable is a Window, then the function XcbSurface::set_size()
   * must be called whenever the size of the window changes.
   *
   * When drawable is a Window containing child windows then drawing to the
   * created surface will be clipped by those child windows. When the created
   * surface is used as a source, the contents of the children will be included.
   *
   * @param connection An XCB connection.
   * @param drawable An XCB drawable.
   * @param visual The visual to use for drawing to drawable. The depth of the
   *        visual must match the depth of the drawable. Currently, only
   *        TrueColor visuals are fully supported.
   * @param width The current width of drawable.
   * @param height The current height of drawable.
   *
   * @throws std::bad_alloc, Cairo::logic_error, std::ios_base::failure
   * @newin{1,20}
   */
  static RefPtr<XcbSurface> create(xcb_connection_t *connection,
                                   xcb_drawable_t drawable,
                                   xcb_visualtype_t *visual, int width, int height);

  /** Creates an XCB surface that draws to the given bitmap.
   * This will be drawn to as a Cairo::Surface::Format::A1 object.
   *
   * @param connection An XCB connection.
   * @param screen The XCB screen associated with the bitmap.
   * @param bitmap An XCB drawable (a Pixmap with depth 1).
   * @param width The current width of bitmap.
   * @param height The current height of bitmap.
   *
   * @throws std::bad_alloc, Cairo::logic_error, std::ios_base::failure
   * @newin{1,20}
   */
  static RefPtr<XcbSurface> create_for_bitmap(xcb_connection_t *connection,
                                              xcb_screen_t *screen,
                                              xcb_pixmap_t bitmap,
                                              int width, int height);

  /** Creates an XCB surface that draws to the given drawable.
   * The way that colors are represented in the drawable is specified by
   * the provided picture format.
   *
   * @note If drawable is a Window, then the function XcbSurface::set_size()
   * must be called whenever the size of the window changes.
   *
   * When drawable is a Window containing child windows then drawing to the
   * created surface will be clipped by those child windows. When the created
   * surface is used as a source, the contents of the children will be included.
   *
   * @param connection An XCB connection.
   * @param screen The XCB screen associated with the drawable.
   * @param drawable An XCB drawable.
   * @param format The picture format to use for drawing to drawable. The depth
   *        of format mush match the depth of the drawable.
   * @param width The current width of drawable.
   * @param height The current height of drawable.
   *
   * @throws std::bad_alloc, Cairo::logic_error, std::ios_base::failure
   * @newin{1,20}
   */
  static RefPtr<XcbSurface>
    create_with_xrender_format(xcb_connection_t *connection,
                               xcb_screen_t *screen, xcb_drawable_t drawable,
                               xcb_render_pictforminfo_t *format,
                               int width, int height);
};

#endif // CAIRO_HAS_XCB_SURFACE

} // namespace Cairo

#endif //__CAIROMM_XCB_SURFACE_H
