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

#ifndef __CAIROMM_ENUMS_H
#define __CAIROMM_ENUMS_H

#include <cairo.h>
#ifdef CAIRO_HAS_FT_FONT
#include <cairo-ft.h>
#endif //CAIRO_HAS_FT_FONT

namespace Cairo
{

#ifndef DOXYGEN_IGNORE_THIS
//This is only used internally, but it must be in a public header because we inline some methods.
//Actually, it is used now by the UserFontFace set_*_func() slots, which are public. murrayc.
typedef cairo_status_t ErrorStatus;
#endif //DOXYGEN_IGNORE_THIS

/**
 * Specifies the type of antialiasing to do when rendering text or shapes.
 *
 * The interpretation of Cairo::ANTIALIAS_DEFAULT is left entirely up to
 * the backend.
 */
typedef enum
{
    /**
     * Use the default antialiasing for the subsystem and target device
     */
    ANTIALIAS_DEFAULT = CAIRO_ANTIALIAS_DEFAULT,

    /**
     * Use bilevel alpha mask
     */
    ANTIALIAS_NONE = CAIRO_ANTIALIAS_NONE,

    /**
     * Perform single-color antialiasing (using shades of gray for black text on
     * white background, for example).
     */
    ANTIALIAS_GRAY = CAIRO_ANTIALIAS_GRAY,

    /**
     * Perform antialiasing by taing advantage of the order of subpixel elements
     * on devices such as LCD panels
     */
    ANTIALIAS_SUBPIXEL = CAIRO_ANTIALIAS_SUBPIXEL
} Antialias;

/**
 * Cairo::Content is used to describe the content that a surface will contain,
 * whether color information, alpha information (translucence vs. opacity), or
 * both.
 */
typedef enum
{
    /**
     * The surface will hold color content only.
     */
    CONTENT_COLOR = CAIRO_CONTENT_COLOR,

    /**
     * The surface will hold alpha content only.
     */
    CONTENT_ALPHA = CAIRO_CONTENT_ALPHA,

    /**
     * The surface will hold color and alpha content.
     */
    CONTENT_COLOR_ALPHA = CAIRO_CONTENT_COLOR_ALPHA
} Content;

/**
 * The subpixel order specifies the order of color elements within each pixel on
 * the display device when rendering with an antialiasing mode of
 * Cairo::ANTIALIAS_SUBPIXEL.
 **/
typedef enum
{
    /**
     * Use the default subpixel order for for the target device
     */
    SUBPIXEL_ORDER_DEFAULT = CAIRO_SUBPIXEL_ORDER_DEFAULT,

    /**
     * Subpixel elements are arranged horizontally with red at the left
     */
    SUBPIXEL_ORDER_RGB = CAIRO_SUBPIXEL_ORDER_RGB,

    /**
     * Subpixel elements are arranged horizontally with blue at the left
     */
    SUBPIXEL_ORDER_BGR = CAIRO_SUBPIXEL_ORDER_BGR,

    /**
     * Subpixel elements are arranged vertically with red at the top
     */
    SUBPIXEL_ORDER_VRGB = CAIRO_SUBPIXEL_ORDER_VRGB,

    /**
     * Subpixel elements are arranged vertically with blue at the top
     */
    SUBPIXEL_ORDER_VBGR = CAIRO_SUBPIXEL_ORDER_VBGR
} SubpixelOrder;


/**
 * Cairo::FontType is used to describe the type of a given font face or scaled
 * font. The font types are also known as "font backends" within cairo.
 *
 * New entries may be added in future versions.
 *
 * @since 1.2
 **/
typedef enum
{
    /**
     * The font was created using cairo's toy font api
     */
    FONT_TYPE_TOY = CAIRO_FONT_TYPE_TOY,

    /**
     * The font is of type FreeType
     */
    FONT_TYPE_FT = CAIRO_FONT_TYPE_FT,

    /**
     * The font is of type Win32
     */
    FONT_TYPE_WIN32 = CAIRO_FONT_TYPE_WIN32,

    /**
     * The font is of type Quartz
     * @since 1.6
     */
    FONT_TYPE_QUARTZ = CAIRO_FONT_TYPE_QUARTZ,

    /**
     * The font was created using cairo's user font api
     * @since 1.8
     */
    FONT_TYPE_USER = CAIRO_FONT_TYPE_USER
} FontType;

/** Specifies properties of a text cluster mapping.
 *
 * @since 1.8
 **/
typedef enum
{
    /**
     * The clusters in the cluster array map to glyphs in the glyph array from
     * end to start.
     */
    TEXT_CLUSTER_FLAG_BACKWARD = CAIRO_TEXT_CLUSTER_FLAG_BACKWARD
} TextClusterFlags;

/**
 * A set of synthesis options to control how FreeType renders the glyphs for a
 * particular font face.
 *
 * FreeType provides the ability to synthesize different glyphs from a base
 * font, which is useful if you lack those glyphs from a true bold or oblique
 * font.
 *
 * Individual synthesis features of a @c FtFontFace can be set using
 * @c FtFontFace::set_synthesize(), or disabled using
 * @c FtFontFace::unset_synthesize(). The currently enabled set of synthesis
 * options can be queried with @c FtFontFace::get_synthesize().
 *
 * Note: that when synthesizing glyphs, the font metrics returned will only be
 * estimates.
 *
 * @since 1.12
 */
#ifdef CAIRO_HAS_FT_FONT
enum FtSynthesize {

#ifndef CAIROMM_DISABLE_DEPRECATED
    /** Embolden the glyphs (redraw with a pixel offset).
     * @deprecated 1.20: Use FT_SYNTHESIZE_BOLD instead.
     */
    FT_SYNTHESIZE_BOLT = CAIRO_FT_SYNTHESIZE_BOLD,
#endif
    /** Embolden the glyphs (redraw with a pixel offset).
     * @newin{1,20}
     */
    FT_SYNTHESIZE_BOLD = CAIRO_FT_SYNTHESIZE_BOLD,

    /// Slant the glyph outline by 12 degrees to the right.
    FT_SYNTHESIZE_OBLIQUE = CAIRO_FT_SYNTHESIZE_OBLIQUE
};

inline FtSynthesize operator|(FtSynthesize a, FtSynthesize b)
{
  return static_cast<FtSynthesize>(static_cast<int>(a) | static_cast<int>(b));
}

inline FtSynthesize operator&(FtSynthesize a, FtSynthesize b)
{
    return static_cast<FtSynthesize>(static_cast<int>(a) & static_cast<int>(b));
}
#endif //CAIRO_HAS_FT_FONT
} // namespace Cairo

#endif //__CAIROMM_ENUMS_H
