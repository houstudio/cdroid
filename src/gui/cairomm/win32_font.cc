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

#include <cairomm/win32_font.h>

#ifdef CAIRO_HAS_WIN32_FONT
#include <cairomm/private.h>

namespace Cairo {


Win32FontFace::Win32FontFace(LOGFONTW* logfont) :
  FontFace(cairo_win32_font_face_create_for_logfontw(logfont), true)
{
  check_object_status_and_throw_exception(*this);
}

Win32FontFace::Win32FontFace(HFONT font) :
  FontFace(cairo_win32_font_face_create_for_hfont(font), true)
{
  check_object_status_and_throw_exception(*this);
}

Win32FontFace::Win32FontFace(LOGFONTW* logfont, HFONT font) :
  FontFace(cairo_win32_font_face_create_for_logfontw_hfont(logfont, font), true)
{
  check_object_status_and_throw_exception(*this);
}

RefPtr<Win32FontFace> Win32FontFace::create(LOGFONTW* logfont)
{
  return make_refptr_for_instance<Win32FontFace>(new Win32FontFace(logfont));
}

RefPtr<Win32FontFace> Win32FontFace::create(HFONT font)
{
  return make_refptr_for_instance<Win32FontFace>(new Win32FontFace(font));
}

RefPtr<Win32FontFace> Win32FontFace::create(LOGFONTW* logfont, HFONT font)
{
  return make_refptr_for_instance<Win32FontFace>(new Win32FontFace(logfont, font));
}

// ScaledFont
Win32ScaledFont::Win32ScaledFont(const RefPtr<Win32FontFace>& font_face,
                                 const Matrix& font_matrix,
                                 const Matrix& ctm,
                                 const FontOptions& options) :
  ScaledFont(font_face, font_matrix, ctm, options)
{
  check_object_status_and_throw_exception(*this);
}

RefPtr<Win32ScaledFont>
Win32ScaledFont::create(const RefPtr<Win32FontFace>& font_face,
                        const Matrix& font_matrix, const Matrix& ctm,
                        const FontOptions& options)
{
  return make_refptr_for_instance<Win32ScaledFont>(new Win32ScaledFont(font_face, font_matrix,
                                                     ctm, options));
}

void Win32ScaledFont::select_font(HDC hdc)
{
  auto status = cairo_win32_scaled_font_select_font(cobj(), hdc);
  check_status_and_throw_exception(status);
  check_object_status_and_throw_exception(*this);
}

void Win32ScaledFont::done_font()
{
  cairo_win32_scaled_font_done_font(cobj());
  check_object_status_and_throw_exception(*this);
}

double Win32ScaledFont::get_metrics_factor() const
{
  auto val = cairo_win32_scaled_font_get_metrics_factor(const_cast<cairo_scaled_font_t*>(cobj()));
  check_object_status_and_throw_exception(*this);
  return val;
}

void Win32ScaledFont::get_logical_to_device(Matrix& logical_to_device) const
{
  cairo_win32_scaled_font_get_logical_to_device(const_cast<cairo_scaled_font_t*>(cobj()),
                                              static_cast<cairo_matrix_t*>(&logical_to_device));
  check_object_status_and_throw_exception(*this);
}

void Win32ScaledFont::get_device_to_logical(Matrix& device_to_logical) const
{
  cairo_win32_scaled_font_get_device_to_logical(const_cast<cairo_scaled_font_t*>(cobj()),
                                              static_cast<cairo_matrix_t*>(&device_to_logical));
  check_object_status_and_throw_exception(*this);
}


}

#endif // CAIRO_HAS_WIN32_FONT
