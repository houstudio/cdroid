/* Copyright (C) 2008 Jonathon Jongsma
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

#include <cairomm/quartz_font.h>

#ifdef CAIRO_HAS_QUARTZ_FONT
#include <cairomm/private.h>

namespace Cairo {


QuartzFontFace::QuartzFontFace(CGFontRef font) :
  FontFace(cairo_quartz_font_face_create_for_cgfont(font), true)
{
  check_object_status_and_throw_exception(*this);
}

RefPtr<QuartzFontFace> QuartzFontFace::create(CGFontRef font)
{
  return make_refptr_for_instance<QuartzFontFace>(new QuartzFontFace(font));
}


#ifndef __LP64__
QuartzFontFace::QuartzFontFace(ATSUFontID font_id) :
  FontFace(cairo_quartz_font_face_create_for_atsu_font_id(font_id), true)
{
  check_object_status_and_throw_exception(*this);
}

RefPtr<QuartzFontFace> QuartzFontFace::create(ATSUFontID font_id)
{
  return make_refptr_for_instance<QuartzFontFace>(new QuartzFontFace(font_id));
}
#endif

}

#endif // CAIRO_HAS_QUARTZ_FONT
