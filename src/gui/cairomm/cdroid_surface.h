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

#ifndef __CAIROMM_CDROID_SURFACE_H
#define __CAIROMM_CDROID_SURFACE_H

#include <cairomm/surface.h>
#include <cairomm/enums.h>
#include <cairo-features.h>

// This header is not included by cairomm.h because it requires Windows headers that 
// tend to pollute the namespace with non-prefixed #defines and typedefs.
// You may include it directly if you need to use this API.

namespace Cairo
{

/** A Win32Surface provides a way to render within Microsoft Windows.  If you
 * want to draw to the screen within a Microsoft Windows application, you
 * should use this Surface type.
 *
 * @note For this Surface to be available, cairo must have been compiled with
 * Win32 support
 */
class CDroidSurface : public Surface
{
private:
   CDroidSurface(void*s,bool has_reference = false);
public:

  /** Create a C++ wrapper for the C instance. This C++ instance should then be
   * given to a RefPtr.
   *
   * @param cobject The C instance.
   * @param has_reference whether we already have a reference. Otherwise, the
   * constructor will take an extra reference.
   */
  explicit CDroidSurface(cairo_surface_t* cobject, bool has_reference = false);
  ~CDroidSurface() override;

  /** Returns the HDC associated with this surface, or NULL if none. Also
   * returns NULL if the surface is not a win32 surface.
   *
   * @return HDC or NULL if no HDC available.
   */
   void* getSurface()const;//get cdroid surface
  //TODO: What does owned mean here? murrayc
  //TODO: If this is just a get, shouldn't there be a const version too? murrayc
  /** Returns a ImageSurface that refers to the same bits as the DIB of the
   * Win32 surface. If the passed-in win32 surface is not a DIB surface, the
   * returned surface will be NULL
   *
   * @return a ImageSurface (owned by the Win32Surface), or an empty RefPtr if the win32
   * surface is not a DIB.
   *
   * @since 1.8
   */
  RefPtr<ImageSurface> get_image();

  /** Creates a cairo surface that targets the given DC. The DC will be queried
   * for its initial clip extents, and this will be used as the size of the
   * cairo surface. Also, if the DC is a raster DC, it will be queried for its
   * pixel format and the cairo surface format will be set appropriately.
   *
   * @param hdc the DC to create a surface for.
   * @return the newly created surface.
   */
  static RefPtr<CDroidSurface> create(void* cdroidsurface,bool hasref=true);

  /**
   * @deprecated use create_with_dib()
   */
  static RefPtr<CDroidSurface> create(Format format, int width, int height,bool hasref=true);

};

} // namespace Cairo

#endif //__CAIROMM_CDROID_SURFACE_H
// vim: ts=2 sw=2 et
