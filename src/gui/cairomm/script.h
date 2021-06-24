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

#ifndef __CAIROMM_SCRIPT_H
#define __CAIROMM_SCRIPT_H

#include <string>
#include <cairomm/device.h>
#include <cairomm/surface.h> // SlotWriteFunc
#include <cairomm/enums.h>

#ifdef CAIRO_HAS_SCRIPT_SURFACE
#include <cairo-script.h>
#endif

namespace Cairo {

#ifdef CAIRO_HAS_SCRIPT_SURFACE

class ScriptSurface;

/**
 * A set of script output variants for the script surface.
 *
 * @since 1.12
 */
enum ScriptMode {

    /// The output will be in readable text (default).
    SCRIPT_MODE_ASCII = CAIRO_SCRIPT_MODE_ASCII,

    /// The output will use byte codes.
    SCRIPT_MODE_BINARY = CAIRO_SCRIPT_MODE_BINARY
};

/**
 * The script surface provides the ability to render to a native script that
 * matches the cairo drawing model. The scripts can be replayed using tools under
 * the util/cairo-script directoriy, or with cairo-perf-trace.
 *
 * @since 1.12
 */
class CAIROMM_API Script : public Device {
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
  explicit Script(cairo_device_t* cobject, bool has_reference = false);
  ~Script() override;

  /**
   * Converts the record operations in recording_surface into a script.
   *
   * @param recording_surface The recording surface to replay
   *
   * Throws an exception on error.
   *
   * @since 1.12
   */
  void add_from_recording_surface(const RefPtr<ScriptSurface>& recording_surface);

  /**
   * Queries the script for its current output mode.
   *
   * @since 1.12
   */
  ScriptMode get_mode() const;

  /**
   * Change the output mode of the script.
   *
   * @param mode The new mode.
   *
   * @since 1.12
   */
  void set_mode(ScriptMode new_mode);

  /**
   * Emit a string verbatim into the script.
   *
   * @param comment The string to emit
   */
  void write_comment(const std::string& comment);

  /**
   * Creates a output device for emitting the script, used when creating the
   * individual surfaces.
   *
   * @param filename The name (path) of the file to write the script to.
   *
   * Throws an exception on error.
   *
   * @since 1.12
   */
  static RefPtr<Script> create(const std::string& filename);

  /**
   * Creates a output device for emitting the script, used when creating the
   * individual surfaces.
   *
   * @param write_func Callback function passed the bytes written to the script
   *
   * @since 1.12
   */
  static RefPtr<Script> create_for_stream(const Surface::SlotWriteFunc& write_func);

};

#endif // CAIRO_HAS_SCRIPT_SURFACE

} // namespace Cairo

#endif //__CAIROMM_SCRIPT_SURFACE_H

