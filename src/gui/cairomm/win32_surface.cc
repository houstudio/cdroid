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

#include <cairomm/win32_surface.h>
#include <cairomm/private.h>

namespace Cairo
{

#ifdef CAIRO_HAS_WIN32_SURFACE

Win32Surface::Win32Surface(cairo_surface_t* cobject, bool has_reference) :
    Surface(cobject, has_reference)
{}

Win32Surface::~Win32Surface()
{
  // surface is destroyed in base class
}

HDC Win32Surface::get_dc() const
{
  return cairo_win32_surface_get_dc(m_cobject);
}

RefPtr<ImageSurface> Win32Surface::get_image()
{
  RefPtr<ImageSurface> surface(new ImageSurface(cairo_win32_surface_get_image(cobj()),
                                                false /* no reference, owned by this win32surface*/));
  check_object_status_and_throw_exception(*this);
  return surface;
}

RefPtr<Win32Surface> Win32Surface::create(HDC hdc)
{
  auto cobject = cairo_win32_surface_create(hdc);
  check_status_and_throw_exception(cairo_surface_status(cobject));
  return make_refptr_for_instance<Win32Surface>(new Win32Surface(cobject, true /* has reference */));
}

RefPtr<Win32Surface> Win32Surface::create_with_dib(Format format, int width, int height)
{
  auto cobject = cairo_win32_surface_create_with_dib((cairo_format_t)format, width, height);
  check_status_and_throw_exception(cairo_surface_status(cobject));
  return make_refptr_for_instance<Win32Surface>(new Win32Surface(cobject, true /* has reference */));
}

RefPtr<Win32Surface> Win32Surface::create_with_ddb(HDC hdc, Format format, int width, int height)
{
  auto cobject =
    cairo_win32_surface_create_with_ddb(hdc, (cairo_format_t)format, width, height);
  check_status_and_throw_exception(cairo_surface_status(cobject));
  return make_refptr_for_instance<Win32Surface>(new Win32Surface(cobject, true /* has reference */));
}

Win32PrintingSurface::Win32PrintingSurface(cairo_surface_t* cobject, bool has_reference)
    : Surface(cobject, has_reference)
{
}

Win32PrintingSurface::~Win32PrintingSurface()
{
  // surface is destroyed in base class
}

RefPtr<Win32PrintingSurface> Win32PrintingSurface::create(HDC hdc)
{
  auto cobject = cairo_win32_surface_create(hdc);
  check_status_and_throw_exception(cairo_surface_status(cobject));
  return make_refptr_for_instance<Win32PrintingSurface>(new Win32PrintingSurface(cobject, true /* has reference */));
}

#endif // CAIRO_HAS_WIN32_SURFACE

} //namespace Cairo

// vim: ts=2 sw=2 et
