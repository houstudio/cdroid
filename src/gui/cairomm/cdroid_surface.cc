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

#include <cairomm/cdroid_surface.h>
#include <cairomm/private.h>
#include <cdtypes.h>
#include <cdgraph.h>
#include <cdlog.h>
#ifdef CAIRO_HAS_CDROID_SURFACE
#include <cairo-cdroid.h>
#endif
namespace Cairo
{


CDroidSurface::CDroidSurface(cairo_surface_t* cobject, bool has_reference) :
    Surface(cobject, has_reference)
{
}

static cairo_format_t cairo_format(int fmt){
    switch(fmt){
    case GPF_ABGR:  return CAIRO_FORMAT_INVALID;
    case GPF_RGB565:return CAIRO_FORMAT_RGB16_565;
    case GPF_ARGB:  return CAIRO_FORMAT_ARGB32;
    case GPF_RGB32: return CAIRO_FORMAT_RGB24;
    case GPF_ARGB4444:
    case GPF_ARGB1555:
    default : return CAIRO_FORMAT_INVALID;
    }
}
CDroidSurface::CDroidSurface(void*s,bool has_reference)
 :Surface(nullptr,has_reference){
    auto cobject=cairo_cdroid_surface_create(s);
    if(has_reference)
        m_cobject = cobject;
    else
        m_cobject = cairo_surface_reference(cobject);
}

CDroidSurface::~CDroidSurface()
{
    // CDroidSurface is destroyed in base class
}

void* CDroidSurface::getSurface() const
{
#ifdef CAIRO_HAS_CDROID_SURFACE
    return cairo_cdroid_surface_get_surface(m_cobject);
#else
    return surface;
#endif
}

RefPtr<ImageSurface> CDroidSurface::get_image()
{
    RefPtr<ImageSurface> surface(new ImageSurface(cobj(), false /* no reference, owned by this win32surface*/));
    check_object_status_and_throw_exception(*this);
    return surface;
}

RefPtr<CDroidSurface> CDroidSurface::create(void* cdsurface,bool hasref)
{
#ifdef CAIRO_HAS_CDROID_SURFACE
    auto cobject = cairo_cdroid_surface_create(cdsurface);
    check_status_and_throw_exception(cairo_surface_status(cobject));
    return RefPtr<CDroidSurface>(new CDroidSurface(cobject, hasref/* has reference */));
#else
    return RefPtr<CDroidSurface>(new CDroidSurface(cdsurface));
#endif
}

RefPtr<CDroidSurface> CDroidSurface::create(Format format, int width, int height,bool hasref)
{
    void*cdsurface;
    GFXCreateSurface(&cdsurface,width,height,0,0);
#ifdef CAIRO_HAS_CDROID_SURFACE
    auto cobject=cairo_cdroid_surface_create(cdsurface);
    return RefPtr<CDroidSurface> (new CDroidSurface(cobject,hasref));//create_with_dib(format, width, height);
#else
    return RefPtr<CDroidSurface>(new CDroidSurface(cdurface));    
#endif // CAIRO_HAS_CDROID_SURFACE
}


} //namespace Cairo

// vim: ts=2 sw=2 et
