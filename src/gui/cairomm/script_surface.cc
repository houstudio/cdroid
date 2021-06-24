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

#include <cairomm/script_surface.h>
#include <cairomm/private.h>

namespace Cairo {

#ifdef CAIRO_HAS_SCRIPT_SURFACE

ScriptSurface::ScriptSurface(cairo_surface_t* cobject, bool has_reference) :
    Surface(cobject, has_reference)
{}

ScriptSurface::~ScriptSurface()
{
  // surface is destroyed in base class
}

RefPtr<ScriptSurface> ScriptSurface::create(const RefPtr<Script>& script,
                                            Content content,
                                            double width, double height)
{
  auto cobject =
        cairo_script_surface_create(script->cobj(), static_cast<cairo_content_t>(content),
                                    width, height);
  check_status_and_throw_exception(cairo_surface_status(cobject));
  return make_refptr_for_instance<ScriptSurface>(new ScriptSurface(cobject, true /* has reference */));
}

RefPtr<ScriptSurface>
  ScriptSurface::create_for_target(const RefPtr<Script>& script,
                                   const RefPtr<Surface>& target)
{
  auto cobject =
        cairo_script_surface_create_for_target(script->cobj(), target->cobj());
  check_status_and_throw_exception(cairo_surface_status(cobject));
  return make_refptr_for_instance<ScriptSurface>(new ScriptSurface(cobject, true /* has reference */));
}

#endif // CAIRO_HAS_SCRIPT_SURFACE

} //namespace Cairo
