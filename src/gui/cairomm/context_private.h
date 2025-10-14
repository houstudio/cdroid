/* Copyright (C) 2008 The cairomm Development Team
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

#ifndef __CAIROMM_CONTEXT_PRIVATE_H
#define __CAIROMM_CONTEXT_PRIVATE_H

#include <cairomm/refptr.h>
#include <cairomm/surface.h>

namespace Cairo
{

namespace Private
{

RefPtr<Surface> wrap_surface_quartz(cairo_surface_t*);
RefPtr<Surface> wrap_surface_win32(cairo_surface_t*);
RefPtr<Surface> wrap_surface_xlib(cairo_surface_t*);

} // namespace Private

} // namespace Cairo

#endif // __CAIROMM_CONTEXT_PRIVATE_H

// vim: ts=2 sw=2 et
