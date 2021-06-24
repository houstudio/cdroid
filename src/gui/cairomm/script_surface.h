/* Copyright (C) 2014 The cairomm Development Team
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

#ifndef __CAIROMM_SCRIPT_SURFACE_H
#define __CAIROMM_SCRIPT_SURFACE_H

#include <cairomm/surface.h>
#include <cairomm/script.h>

namespace Cairo {

#ifdef CAIRO_HAS_SCRIPT_SURFACE

/**
 * The script surface provides the ability to render to a native script that
 * matches the cairo drawing model. The scripts can be replayed using tools under
 * the util/cairo-script directoriy, or with cairo-perf-trace.
 *
 * @since 1.12
 */
class CAIROMM_API ScriptSurface : public Surface {
public:

  /**
   * Create a C++ wrapper for the C instance. This C++ instance should then be
   * given to a RefPtr.
   *
   * @param cobject The C instance.
   * @param has_reference whether we already have a reference. Otherwise, the
   * constructor will take an extra reference.
   *
   * @since 1.12
   */
  explicit ScriptSurface(cairo_surface_t* cobject, bool has_reference = false);
  ~ScriptSurface() override;

  /**
   *  Create a new surface that will emit its rendering through script.
   *
   * Throws an exception on error.
   *
   * @param script The script (output device)
   * @param content The content of the surface
   * @param width Width in pixels
   * @param height Height in pixels
   *
   * @since 1.12
   */
  static RefPtr<ScriptSurface> create(const RefPtr<Script>& script,
                                      Content content, double width, double height);

  /**
   * Create a proxy surface that will render to target and record the operations
   * to device.
   *
   * Throws an exception on error.
   *
   * @param script The script (output device)
   * @param target A target surface to wrap
   *
   * @since 1.12
   */
  static RefPtr<ScriptSurface> create_for_target(const RefPtr<Script>& script,
                                                 const RefPtr<Surface>& target);
};

#endif // CAIRO_HAS_SCRIPT_SURFACE

} // namespace Cairo

#endif //__CAIROMM_SCRIPT_SURFACE_H

