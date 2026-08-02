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
#include <core/surfaceops.h>
#include <cairomm/context.h>

namespace cdroid{

Cairo::RefPtr<Cairo::ImageSurface> scaleSurface(
    const Cairo::RefPtr<Cairo::ImageSurface>& src, int dstW, int dstH,
    Cairo::SurfacePattern::Filter filter){
    if(!src || dstW<=0 || dstH<=0) return src;
    if(dstW==src->get_width() && dstH==src->get_height()) return src;

    Cairo::RefPtr<Cairo::ImageSurface> dst =
        Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, dstW, dstH);
    Cairo::RefPtr<Cairo::Context> ctx = Cairo::Context::create(dst);
    ctx->scale((double)dstW / src->get_width(), (double)dstH / src->get_height());
    ctx->set_source(src, 0.0, 0.0);
    Cairo::RefPtr<Cairo::SurfacePattern> pat = ctx->get_source_for_surface();
    if(pat) pat->set_filter(filter);
    ctx->paint();
    return dst;
}

}/*namespace cdroid*/
