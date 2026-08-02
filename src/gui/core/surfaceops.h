/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __CDROID_SURFACE_OPS_H__
#define __CDROID_SURFACE_OPS_H__
#include <cairomm/surface.h>
#include <cairomm/pattern.h>

namespace cdroid{

/* Scale-blit `src` into a freshly allocated dstW x dstH ARGB32 surface. This is the
   decoder "scalePixels" step (akin to SkImage::scalePixels / Android
   Bitmap.createScaledBitmap): produce a destination-size surface in one shot so later
   drawing is a 1:1 blit instead of a per-frame software resample.
   Returns `src` unchanged when dst matches src dimensions (zero overhead) or on bad
   args (nullptr src / non-positive dst). */
Cairo::RefPtr<Cairo::ImageSurface> scaleSurface(
    const Cairo::RefPtr<Cairo::ImageSurface>& src, int dstW, int dstH,
    Cairo::SurfacePattern::Filter filter = Cairo::SurfacePattern::Filter::BILINEAR);

}/*namespace cdroid*/
#endif/*__CDROID_SURFACE_OPS_H__*/
