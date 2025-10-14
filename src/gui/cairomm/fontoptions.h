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

#ifndef __CAIROMM_FONTOPTIONS_H
#define __CAIROMM_FONTOPTIONS_H

#include <cairommconfig.h>

#include <cairomm/enums.h>
#include <string>
//#include <cairo.h>
#ifdef CAIRO_HAS_FT_FONT
#include <cairo-ft.h>
#endif // CAIRO_HAS_FT_FONT

namespace Cairo
{

/**
 * The font options specify how fonts should be rendered.  Most of the
 * time the font options implied by a surface are just right and do not
 * need any changes, but for pixel-based targets tweaking font options
 * may result in superior output on a particular display.
 */
class CAIROMM_API FontOptions
{
public:
  /**
   * Specifies the type of hinting to do on font outlines. Hinting is the process
   * of fitting outlines to the pixel grid in order to improve the appearance of
   * the result. Since hinting outlines involves distorting them, it also reduces
   * the faithfulness to the original outline shapes. Not all of the outline
   * hinting styles are supported by all font backends.
   *
   * New entries may be added in future versions.
   **/
  enum class HintStyle
  {
      /**
       * Use the default hint style for font backend and target device
       */
      DEFAULT = CAIRO_HINT_STYLE_DEFAULT,

      /**
       * Do not hint outlines
       */
      NONE = CAIRO_HINT_STYLE_NONE,

      /**
       * Hint outlines slightly to improve contrast while retaining food fidelity
       * to the original shapes
       */
      SLIGHT = CAIRO_HINT_STYLE_SLIGHT,

      /**
       * Hint outlines with medium strength giving a compromise between fidelity
       * to the original shapes and contrast
       */
      MEDIUM = CAIRO_HINT_STYLE_MEDIUM,

      /**
       * Hint outlines to maximize contrast
       */
      FULL = CAIRO_HINT_STYLE_FULL
  };

  /**
   * Specifies whether to hint font metrics; hinting font metrics means quantizing
   * them so that they are integer values in device space. Doing this improves the
   * consistency of letter and line spacing, however it also means that text will
   * be laid out differently at different zoom factors.
   **/
  enum class HintMetrics
  {
      /**
       * Hint metrics in the default manner for the font backend and target device
       */
      DEFAULT = CAIRO_HINT_METRICS_DEFAULT,

      /**
       * Do not hint font metrics
       */
      OFF = CAIRO_HINT_METRICS_OFF,

      /**
       * Hint font metrics
       */
      ON = CAIRO_HINT_METRICS_ON
  };

  FontOptions();
  explicit FontOptions(cairo_font_options_t* cobject, bool take_ownership = false);
  FontOptions(const FontOptions& src);

  virtual ~FontOptions();

  FontOptions& operator=(const FontOptions& src);

  bool operator ==(const FontOptions& src) const;
  //bool operator !=(const FontOptions& src) const;

  /**
   * Merges non-default options from @a other into this, replacing existing
   * values. This operation can be thought of as somewhat similar to compositing
   * @a other onto this with the operation of OPERATION_OVER.
   *
   * @param other another FontOptions
   **/
  void merge(const FontOptions& other);

  /**
   * Compute a hash for the font options object; this value will be useful when
   * storing an object containing a FontOptions in a hash table.
   *
   * @return the hash value for the font options object.  The return value can
   * be cast to a 32-bit type if a 32-bit hash value is needed.
   **/
  unsigned long hash() const;

  /**
   * Sets the antialiasing mode for the font options object. This
   * specifies the type of antialiasing to do when rendering text.
   *
   * @param antialias the new antialiasing mode.
   **/
  void set_antialias(Antialias antialias);

  /**
   * Gets the antialiasing mode for the font options object.
   *
   * @return the antialiasing mode
   **/
  Antialias get_antialias() const;

  /**
   * Sets the subpixel order for the font options object. The subpixel order
   * specifies the order of color elements within each pixel on the display
   * device when rendering with an antialiasing mode of
   * Cairo::ANTIALIAS_SUBPIXEL. See the documentation for SubpixelOrder for
   * full details.
   *
   * @param subpixel_order the new subpixel order.
   **/
  void set_subpixel_order(SubpixelOrder subpixel_order);

  /**
   * Gets the subpixel order for the font options object.  See the documentation
   * for SubpixelOrder for full details.
   *
   * @return the subpixel order for the font options object.
   **/
  SubpixelOrder get_subpixel_order() const;

  /**
   * Sets the hint style for font outlines for the font options object.  This
   * controls whether to fit font outlines to the pixel grid, and if so, whether
   * to optimize for fidelity or contrast.  See the documentation for
   * HintStyle for full details.
   *
   * @param hint_style the new hint style.
   **/
  void set_hint_style(HintStyle hint_style);

  /**
   * Gets the hint style for font outlines for the font options object.
   * See the documentation for HintStyle for full details.
   *
   * @return the hint style for the font options object.
   **/
  HintStyle get_hint_style() const;

  /**
   * Sets the metrics hinting mode for the font options object. This
   * controls whether metrics are quantized to integer values in
   * device units.
   * See the documentation for HintMetrics for full details.
   *
   * @param hint_metrics the new metrics hinting mode.
   **/
  void set_hint_metrics(HintMetrics hint_metrics);

  /**
   * Gets the metrics hinting mode for the font options object.  See the
   * documentation for HintMetrics for full details.
   *
   * Return value: the metrics hinting mode for the font options object.
   **/
  HintMetrics get_hint_metrics() const;

#ifdef CAIRO_HAS_FT_FONT
#ifdef CAIRO_HAS_FC_FONT
  /** Add options to a FcPattern based on a cairo_font_options_t font options
   * object. Options that are already in the pattern, are not overridden, so you
   * should call this function after calling FcConfigSubstitute() (the user's
   * settings should override options based on the surface type), but before
   * calling FcDefaultSubstitute().
   *
   * @param pattern an existing FcPattern.
   *
   * @since 1.8
   */
  void substitute(FcPattern* pattern);
#endif // CAIRO_HAS_FC_FONT
#endif // CAIRO_HAS_FT_FONT

  typedef cairo_font_options_t cobject;
  inline cobject* cobj() { return m_cobject; }
  inline const cobject* cobj() const { return m_cobject; }

  #ifndef DOXYGEN_IGNORE_THIS
  ///For use only by the cairomm implementation.
  inline ErrorStatus get_status() const
  { return cairo_font_options_status(const_cast<cairo_font_options_t*>(cobj())); }
  #endif //DOXYGEN_IGNORE_THIS

protected:

  cobject* m_cobject;
};

} // namespace Cairo

#endif //__CAIROMM_FONTOPTIONS_H

// vim: ts=2 sw=2 et
