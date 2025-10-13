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

#include <cairomm/xlib_device.h>
#include <cairomm/private.h>

#if CAIRO_HAS_XLIB_SURFACE
#include <cairo-xlib.h>
#endif

namespace Cairo
{
#ifdef CAIRO_HAS_XLIB_SURFACE

XlibDevice::XlibDevice(cairo_device_t* cobject, bool has_reference) :
  Device(cobject, has_reference)
{}

XlibDevice::~XlibDevice()
{
  // device is destroyed in base class
}

void XlibDevice::debug_cap_xrender_version(int major_version, int minor_version)
{
  cairo_xlib_device_debug_cap_xrender_version(m_cobject, major_version,
                                              minor_version);
}

int XlibDevice::debug_get_precision() const
{
  return cairo_xlib_device_debug_get_precision(m_cobject);
}

void XlibDevice::debug_set_precision(int precision)
{
  cairo_xlib_device_debug_set_precision(m_cobject, precision);
}

#endif // CAIRO_HAS_XLIB_SURFACE

} //namespace Cairo
