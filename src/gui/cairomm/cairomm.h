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

#ifndef __CAIROMM_H
#define __CAIROMM_H

/** @mainpage Cairomm: A C++ wrapper for the cairo graphics library
 *
 * @section License
 * Cairomm is available under the terms of the LGPL license
 *
 * @section Introduction
 * If you're just beginning to learn cairomm, a good place to start is with the
 * Cairo::Surface and Cairo::Context classes.  In general terms, you draw onto
 * a Surface using the graphics settings specified in your Context.
 *
 */

#include <cairommconfig.h>
#include <cairomm/context.h>
#include <cairomm/device.h>
#include <cairomm/enums.h>
#include <cairomm/exception.h>
#include <cairomm/fontface.h>
#include <cairomm/fontoptions.h>
#include <cairomm/matrix.h>
#include <cairomm/mesh_pattern.h>
#include <cairomm/path.h>
#include <cairomm/pattern.h>
#include <cairomm/quartz_font.h>
#include <cairomm/quartz_surface.h>
#include <cairomm/refptr.h>
#include <cairomm/region.h>
#include <cairomm/scaledfont.h>
#include <cairomm/script.h>
#include <cairomm/script_surface.h>
#include <cairomm/surface.h>
#include <cairomm/types.h>
#include <cairomm/win32_font.h>
#include <cairomm/win32_surface.h>
#include <cairomm/xcb_device.h>
#include <cairomm/xcb_surface.h>
#include <cairomm/xlib_device.h>
#include <cairomm/xlib_surface.h>

#endif //__CAIROMM_H
