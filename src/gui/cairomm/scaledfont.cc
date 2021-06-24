/* Copyright (C) 2006 The cairomm Development Team
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

#include <cairomm/scaledfont.h>
#include <cairomm/fontface.h>
#include <cairomm/private.h>  // for check_status_and_throw_exception

namespace Cairo
{

ScaledFont::ScaledFont(cobject* cobj, bool has_reference)
: m_cobject(nullptr)
{
  if(has_reference)
    m_cobject = cobj;
  else
    m_cobject = cairo_scaled_font_reference(cobj);
}

ScaledFont::ScaledFont(const RefPtr<FontFace>& font_face, const Matrix& font_matrix,
                       const Matrix& ctm, const FontOptions& options)
: m_cobject(nullptr)
{
  m_cobject =
    cairo_scaled_font_create(font_face->cobj(),
                             &font_matrix,
                             &ctm,
                             options.cobj());
  check_object_status_and_throw_exception(*this);
}

ScaledFont::~ScaledFont()
{
  if (cobj())
    cairo_scaled_font_destroy(cobj());
}

RefPtr<ScaledFont> ScaledFont::create(const RefPtr<FontFace>& font_face, const Matrix& font_matrix,
    const Matrix& ctm, const FontOptions& options)
{
  return make_refptr_for_instance<ScaledFont>(new ScaledFont(font_face, font_matrix, ctm, options));
}

void ScaledFont::get_extents(FontExtents& extents) const
{
  cairo_scaled_font_extents(m_cobject, static_cast<cairo_font_extents_t*>(&extents));
  check_object_status_and_throw_exception(*this);
}

RefPtr<FontFace> ScaledFont::get_font_face() const
{
  auto face = cairo_scaled_font_get_font_face(m_cobject);
  check_object_status_and_throw_exception(*this);
  return make_refptr_for_instance<FontFace>(new FontFace(face, false /* returned face doesn't have a reference */));
}

void ScaledFont::get_font_options(FontOptions& options) const
{
  cairo_scaled_font_get_font_options(m_cobject, options.cobj());
  check_object_status_and_throw_exception(*this);
}

void ScaledFont::get_font_matrix(Matrix& font_matrix) const
{
  cairo_scaled_font_get_font_matrix(m_cobject,
                                    &font_matrix);
  check_object_status_and_throw_exception(*this);
}

void ScaledFont::get_ctm(Matrix& ctm) const
{
  cairo_scaled_font_get_ctm(m_cobject, &ctm);
  check_object_status_and_throw_exception(*this);
}

FontType ScaledFont::get_type() const
{
  auto font_type = cairo_scaled_font_get_type(m_cobject);
  check_object_status_and_throw_exception(*this);
  return static_cast<FontType>(font_type);
}

void
ScaledFont::text_to_glyphs (double x,
                            double y,
                            const std::string& utf8,
                            std::vector<Glyph>& glyphs,
                            std::vector<TextCluster>& clusters,
                            TextClusterFlags& cluster_flags)
{
  int num_glyphs = -1;
  int num_clusters = -1;
  cairo_glyph_t* c_glyphs = nullptr;
  cairo_text_cluster_t* c_clusters = nullptr;
  auto status = cairo_scaled_font_text_to_glyphs(cobj(), x, y,
                                                           utf8.c_str(),
                                                           utf8.size(),
                                                           &c_glyphs,
                                                           &num_glyphs,
                                                           &c_clusters,
                                                           &num_clusters,
                                                           reinterpret_cast<cairo_text_cluster_flags_t*>(&cluster_flags));
  if (num_glyphs > 0 && c_glyphs) {
    glyphs.assign(static_cast<Glyph*>(c_glyphs),
                  static_cast<Glyph*>(c_glyphs + num_glyphs));
    cairo_glyph_free(c_glyphs);
  }
  if (num_clusters > 0 && c_clusters) {
    clusters.assign(static_cast<TextCluster*>(c_clusters),
                    static_cast<TextCluster*>(c_clusters + num_clusters));
    cairo_text_cluster_free(c_clusters);
  }
  check_status_and_throw_exception(status);
  check_object_status_and_throw_exception(*this);
}

void ScaledFont::get_scale_matrix(Matrix& scale_matrix) const
{
  cairo_scaled_font_get_scale_matrix(const_cast<cairo_scaled_font_t*>(cobj()), &scale_matrix);
  check_object_status_and_throw_exception(*this);
}

#ifdef CAIRO_HAS_FT_FONT
FtScaledFont::FtScaledFont(const RefPtr<FtFontFace>& font_face, const Matrix& font_matrix,
                           const Matrix& ctm, const FontOptions& options) :
  ScaledFont(font_face, font_matrix, ctm, options)
{
  check_object_status_and_throw_exception(*this);
}

RefPtr<FtScaledFont>
FtScaledFont::create(const RefPtr<FtFontFace>& font_face,
                     const Matrix& font_matrix, const Matrix& ctm,
                     const FontOptions& options)
{
  return make_refptr_for_instance<FtScaledFont>(new FtScaledFont(font_face, font_matrix, ctm, options));
}

FT_Face FtScaledFont::lock_face()
{
  auto face = cairo_ft_scaled_font_lock_face(cobj());
  check_object_status_and_throw_exception(*this);
  return face;
}

void FtScaledFont::unlock_face()
{
  cairo_ft_scaled_font_unlock_face(cobj());
  check_object_status_and_throw_exception(*this);
}
#endif // CAIRO_HAS_FT_FONT

}   // namespace Cairo
// vim: ts=2 sw=2 et
