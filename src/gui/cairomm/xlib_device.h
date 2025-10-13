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

#ifndef __CAIROMM_XLIB_DEVICE_H
#define __CAIROMM_XLIB_DEVICE_H

#include <cairomm/device.h>

namespace Cairo
{
#ifdef CAIRO_HAS_XLIB_SURFACE

/** An Xlib device.
 *
 * An %XlibDevice object can be created from an XlibSurface object with
 * Surface::get_device(). It returns a RefPtr<Device>, which can be cast
 * to a %RefPtr<%XlibDevice> if the %Surface is an %XlibSurface.
 * @code
 * Cairo::RefPtr<Device> device = xlib_surface->get_device();
 * Cairo::RefPtr<XlibDevice> xlib_device = std::dynamic_pointer_cast<Cairo::XlibDevice>(device);
 * if (xlib_device)
 *   do_something(xlib_device);
 * @endcode
 *
 * @note For this Device to be available, cairo must have been compiled with
 * Xlib support.
 *
 * @newin{1,20}
 */
class CAIROMM_API XlibDevice : public Device
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
  explicit XlibDevice(cairo_device_t* cobject, bool has_reference = false);

  ~XlibDevice() override;

  /** Restricts all future Xlib surfaces for this device to the specified
   * version of the RENDER extension.
   * This function exists solely for debugging purpose. It lets you find out
   * how Cairomm would behave with an older version of the RENDER extension.
   *
   * Use the special values -1 and -1 for disabling the RENDER extension.
   *
   * @param major_version Major version to restrict to.
   * @param minor_version Minor version to restrict to.
   *
   * @newin{1,20}
   */
  void debug_cap_xrender_version(int major_version, int minor_version);

  /** Returns the Xrender precision mode.
   *
   * @newin{1,20}
   */
  int debug_get_precision() const;

  /** The Xrender extension supports two modes of precision when rendering
   * trapezoids.
   * Set the precision to the desired mode.
   *
   * @newin{1,20}
   */
  void debug_set_precision(int precision);
};

#endif // CAIRO_HAS_XLIB_SURFACE

} // namespace Cairo

#endif //__CAIROMM_XLIB_DEVICE_H
