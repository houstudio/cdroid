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

#include <cairomm/xcb_surface.h>
#include <cairomm/private.h>

namespace Cairo {

#ifdef CAIRO_HAS_XCB_SURFACE

XcbSurface::XcbSurface(cairo_surface_t* cobject, bool has_reference) :
  Surface(cobject, has_reference)
{}

XcbSurface::~XcbSurface()
{
  // surface is destroyed in base class
}

void XcbSurface::set_size(int width, int height)
{
  cairo_xcb_surface_set_size(m_cobject, width, height);
  check_object_status_and_throw_exception(*this);
}

void XcbSurface::set_drawable(xcb_drawable_t drawable, int width, int height)
{
  cairo_xcb_surface_set_drawable(m_cobject, drawable, width, height);
  check_object_status_and_throw_exception(*this);
}

RefPtr<XcbSurface> XcbSurface::create(xcb_connection_t *connection,
                                      xcb_drawable_t drawable,
                                      xcb_visualtype_t *visual,
                                      int width, int height)
{
  cairo_surface_t* cobject = cairo_xcb_surface_create(connection, drawable,
                                                      visual, width, height);
  auto cpp_object = make_refptr_for_instance<XcbSurface>(new XcbSurface(cobject, true /* has reference */));
  // If an exception is thrown, cpp_object's destructor will call ~Surface(),
  // which will destroy cobject.
  check_object_status_and_throw_exception(*cpp_object);
  return cpp_object;
}

RefPtr<XcbSurface> XcbSurface::create_for_bitmap(xcb_connection_t *connection,
                                                 xcb_screen_t *screen,
                                                 xcb_pixmap_t bitmap,
                                                 int width, int height)
{
  cairo_surface_t* cobject =
        cairo_xcb_surface_create_for_bitmap(connection, screen, bitmap,
                                            width, height);
  auto cpp_object = make_refptr_for_instance<XcbSurface>(new XcbSurface(cobject, true /* has reference */));
  check_object_status_and_throw_exception(*cpp_object);
  return cpp_object;
}

RefPtr<XcbSurface>
  XcbSurface::create_with_xrender_format(xcb_connection_t *connection,
                                         xcb_screen_t *screen,
                                         xcb_drawable_t drawable,
                                         xcb_render_pictforminfo_t *format,
                                         int width, int height)
{
  cairo_surface_t* cobject =
        cairo_xcb_surface_create_with_xrender_format(connection, screen, drawable,
                                                     format, width, height);
  auto cpp_object = make_refptr_for_instance<XcbSurface>(new XcbSurface(cobject, true /* has reference */));
  check_object_status_and_throw_exception(*cpp_object);
  return cpp_object;
}
#endif // CAIRO_HAS_XCB_SURFACE

} //namespace Cairo
