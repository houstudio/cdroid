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
#ifndef __CAIROMM_QUARTZ_FONT_H
#define __CAIROMM_QUARTZ_FONT_H

#include <cairo-features.h>

#ifdef CAIRO_HAS_QUARTZ_FONT
#include <cairo-quartz.h>
#include <cairomm/fontface.h>
#include <cairomm/scaledfont.h>

namespace Cairo
{

/**
 * The Quartz font backend is primarily used to render text on Apple
 * MacOS X systems.  The CGFont API is used for the internal
 * implementation of the font backend methods.
 *
 * @since 1.8
 */
class CAIROMM_API QuartzFontFace : public FontFace
{
public:

  /** Creates a new font for the Quartz font backend based on a CGFontRef. This
   * font can then be used with Context::set_font_face() or
   * ScaledFont::create().
   *
   * @param font a CGFontRef obtained through a method external to cairo.
   *
   * @since 1.8
   */
  static RefPtr<QuartzFontFace> create(CGFontRef font);

#ifndef __LP64__
  /** Creates a new font for the Quartz font backend based on an ATSUFontID.
   * This font can then be used with Context::set_font_face() or
   * ScaledFont::create().
   *
   * @param font_id an ATSUFontID for the font.
   *
   * @since 1.8
   */
  static RefPtr<QuartzFontFace> create(ATSUFontID font_id);
#endif


protected:
  QuartzFontFace(CGFontRef font);
#ifndef __LP64__
  QuartzFontFace(ATSUFontID font_id);
#endif
};

}

#endif // CAIRO_HAS_QUARTZ_FONT

#endif // __CAIROMM_QUARTZ_FONT_H
