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

#include <iostream>
#include <cairomm/context.h>
#include <cairomm/fontface.h>
#include <cairomm/scaledfont.h>
#include <cairomm/private.h>

namespace
{

static const cairo_user_data_key_t USER_DATA_KEY_DEFAULT_TEXT_TO_GLYPHS =  {0};

} // anonymous namespace

namespace Cairo
{

FontFace::FontFace(cairo_font_face_t* cobject, bool has_reference)
: m_cobject(nullptr)
{
  if(has_reference)
    m_cobject = cobject;
  else
    m_cobject = cairo_font_face_reference(cobject);
}

FontFace::~FontFace()
{
  if(m_cobject)
    cairo_font_face_destroy(m_cobject);
}

void FontFace::reference() const
{
 cairo_font_face_reference(m_cobject);
}

void FontFace::unreference() const
{
  cairo_font_face_destroy(m_cobject);
}

/*
void* FontFace::get_user_data(const cairo_user_data_key_t *key)
{
  auto result = cairo_font_face_get_user_data(m_cobject, key);
  check_object_status_and_throw_exception(*this);
  return result;
}

void FontFace::set_user_data(const cairo_user_data_key_t* key, void *user_data, cairo_destroy_func_t destroy)
{
  const auto status = (ErrorStatus)cairo_font_face_set_user_data(m_cobject, key, user_data, destroy);
  check_status_and_throw_exception(status);
}
*/

FontType FontFace::get_type() const
{
  auto font_type = cairo_font_face_get_type(m_cobject);
  check_object_status_and_throw_exception(*this);
  return static_cast<FontType>(font_type);
}

// 'Toy' fonts
RefPtr<ToyFontFace>
ToyFontFace::create(const std::string& family, Slant slant, Weight weight)
{
  return make_refptr_for_instance<ToyFontFace>(new ToyFontFace(family, slant, weight));
}

ToyFontFace::ToyFontFace(const std::string& family, Slant slant, Weight weight) :
  FontFace(cairo_toy_font_face_create (family.c_str(),
                                       static_cast<cairo_font_slant_t>(slant),
           	                            static_cast<cairo_font_weight_t>(weight)),
           true /* has reference*/)
{
  check_status_and_throw_exception(cairo_font_face_status(m_cobject));
}

std::string ToyFontFace::get_family() const
{
  return std::string(cairo_toy_font_face_get_family(m_cobject));
}

ToyFontFace::Slant ToyFontFace::get_slant() const
{
  return Slant(cairo_toy_font_face_get_slant(m_cobject));
}

ToyFontFace::Weight ToyFontFace::get_weight() const
{
  return Weight(cairo_toy_font_face_get_weight(m_cobject));
}


//*************************//
// UserFont Implementation //
//*************************//

static const cairo_user_data_key_t user_font_key = {0};

static void
log_uncaught_exception(const char* message = nullptr)
{
  std::cerr << "*** cairomm: Uncaught exception in UserFont callback";
  if(message)
    std::cerr << ": " << message;

  std::cerr << std::endl;
}

cairo_status_t
UserFontFace::init_cb(cairo_scaled_font_t* scaled_font,
                      cairo_t *cr,
                      cairo_font_extents_t* metrics)
{
  auto face = cairo_scaled_font_get_font_face(scaled_font);
  // we've stored a pointer to the wrapper object in the C object's user_data
  auto instance =
    static_cast<UserFontFace*>(cairo_font_face_get_user_data(face,
                                                             &user_font_key));

  if(instance)
  {
    try
    {
      return instance->init(make_refptr_for_instance<ScaledFont>(new ScaledFont(scaled_font)),
                            make_refptr_for_instance<Context>(new Context(cr)),
                            static_cast<FontExtents&>(*metrics));
    }
    catch(const std::exception& ex)
    {
      log_uncaught_exception(ex.what());
    }
    catch( ... )
    {
      log_uncaught_exception();
    }
  }

  // this should never happen
  return CAIRO_STATUS_USER_FONT_ERROR;
}

ErrorStatus
UserFontFace::init(const RefPtr<ScaledFont>& /*scaled_font*/,
                   const RefPtr<Context>& /*cr*/,
                   FontExtents& extents)
{
  // fallback behavior is to set up the default text extents as described in the
  // cairo API documentation
  extents.ascent = 1.0;
  extents.descent = 0.0;
  extents.height = 1.0;
  extents.max_x_advance = 1.0;
  extents.max_y_advance = 0.0;
  return CAIRO_STATUS_SUCCESS;
}

cairo_status_t
UserFontFace::unicode_to_glyph_cb(cairo_scaled_font_t *scaled_font,
                                  unsigned long        unicode,
                                  unsigned long       *glyph)
{
  auto face = cairo_scaled_font_get_font_face(scaled_font);
  // we've stored a pointer to the wrapper object in the C object's user_data
  auto instance =
    static_cast<UserFontFace*>(cairo_font_face_get_user_data(face,
                                                             &user_font_key));
  if(instance)
  {
    try
    {
      return instance->unicode_to_glyph(make_refptr_for_instance<ScaledFont>(new ScaledFont(scaled_font)),
                                        unicode, *glyph);
    }
    catch(const std::exception& ex)
    {
      log_uncaught_exception(ex.what());
    }
    catch( ... )
    {
      log_uncaught_exception();
    }
  }

  // this should never happen
  return CAIRO_STATUS_USER_FONT_ERROR;
}

ErrorStatus
UserFontFace::unicode_to_glyph(const RefPtr<ScaledFont>& /*scaled_font*/,
                               unsigned long unicode,
                               unsigned long& glyph)
{
  // fallback behavior is just to map 1:1
  glyph = unicode;
  return CAIRO_STATUS_SUCCESS;
}

cairo_status_t
UserFontFace::text_to_glyphs_cb(cairo_scaled_font_t *scaled_font,
                                const char *utf8,
                                int utf8_len,
                                cairo_glyph_t **glyphs,
                                int *num_glyphs,
                                cairo_text_cluster_t **clusters,
                                int *num_clusters,
                                cairo_text_cluster_flags_t *cluster_flags)
{
  auto face = cairo_scaled_font_get_font_face(scaled_font);
  // we've stored a pointer to the wrapper object in the C object's user_data
  auto instance =
    static_cast<UserFontFace*>(cairo_font_face_get_user_data(face,
                                                             &user_font_key));

  if(instance)
  {
    try
    {
      std::vector<Glyph> glyph_v;
      std::vector<TextCluster> cluster_v;
      const std::string utf8_str(utf8, utf8 + utf8_len);
      auto local_flags = static_cast<TextClusterFlags>(0);

      auto status =
        instance->text_to_glyphs(make_refptr_for_instance<ScaledFont>(new
                                                    ScaledFont(scaled_font)),
                                 utf8_str, glyph_v, cluster_v, local_flags);

      // NOTE: see explanation in text_to_glyphs()
      if (cairo_font_face_get_user_data(face, &USER_DATA_KEY_DEFAULT_TEXT_TO_GLYPHS))
      {
        *num_glyphs = -1;
        return status;
      }

      // TODO: we re-allocate a new array and pass it back to the caller since
      // cairo will free the the returned array.  It sucks to do this excessive
      // allocation and copying, I don't see much alternative besides just
      // presenting a plain-C API.  If cairo didn't free the list, we could
      // possibly just pass back a .data() pointer or something...
      if(num_glyphs && glyphs)
      {
        *num_glyphs = glyph_v.size();
        if(!glyph_v.empty())
        {
          *glyphs = cairo_glyph_allocate(glyph_v.size());
          std::copy(glyph_v.begin(), glyph_v.end(), *glyphs);
        }
      }
      else
        return CAIRO_STATUS_USER_FONT_ERROR;

      // TODO: same for clusters
      if(num_clusters && clusters)
      {
        *num_clusters = cluster_v.size();
        if(!cluster_v.empty())
        {
          *clusters = cairo_text_cluster_allocate(cluster_v.size());
          std::copy(cluster_v.begin(), cluster_v.end(), *clusters);
        }
      }

      if(cluster_flags)
        *cluster_flags = static_cast<cairo_text_cluster_flags_t>(local_flags);

      return status;
    }
    catch(const std::exception& ex)
    {
      log_uncaught_exception(ex.what());
    }
    catch( ... )
    {
      log_uncaught_exception();
    }
  }

  // this should never happen
  return CAIRO_STATUS_USER_FONT_ERROR;
}

ErrorStatus
UserFontFace::text_to_glyphs(const RefPtr<ScaledFont>& /*scaled_font*/,
                             const std::string& /*utf8*/,
                             std::vector<Glyph>& /*glyphs*/,
                             std::vector<TextCluster>& /*clusters*/,
                             TextClusterFlags& /*cluster_flags*/)
{
  // this is a big hack to make up for the fact that we can't easily pass back a
  // negative value for the size of the glyph array, which is a special value in
  // the C API that means: "ignore this function and call unicode_to_glyph
  // instead".  If this default virtual function is ever called, it will set a
  // non-NULL value in the user_data, which we can read back in the
  // text_to_glyphs_cb and used as a signal to return -1 for the num_glyphs
  // parameter.
  // TODO: Is there a reentrancy requirement, and is this code reentrant?
  cairo_font_face_set_user_data(cobj(), &USER_DATA_KEY_DEFAULT_TEXT_TO_GLYPHS,
                                this, nullptr);
  return CAIRO_STATUS_SUCCESS;
}

cairo_status_t
UserFontFace::render_glyph_cb(cairo_scaled_font_t  *scaled_font,
                              unsigned long         glyph,
                              cairo_t              *cr,
                              cairo_text_extents_t *metrics)
{
  auto face = cairo_scaled_font_get_font_face(scaled_font);
  // we've stored a pointer to the wrapper object in the C object's user_data
  auto instance =
    static_cast<UserFontFace*>(cairo_font_face_get_user_data(face,
                                                             &user_font_key));
  if(instance)
  {
    try
    {
      return instance->render_glyph(make_refptr_for_instance<ScaledFont>(new ScaledFont(scaled_font)),
                                    glyph, make_refptr_for_instance<Context>(new Context(cr)),
                                    static_cast<TextExtents&>(*metrics));
    }
    catch(const std::exception& ex)
    {
      log_uncaught_exception(ex.what());
    }
    catch( ... )
    {
      log_uncaught_exception();
    }
  }

  // this should never happen
  return CAIRO_STATUS_USER_FONT_ERROR;
}

// no default implementation for UserFontFace::render_glyph(), user must
// implement it

UserFontFace::UserFontFace()
  : FontFace(cairo_user_font_face_create(), true /* has reference */)
{
  check_status_and_throw_exception(cairo_font_face_status(m_cobject));

  // store a pointer to the wrapper class in the user-data, so that when one of
  // the callback functions gets called (which has to be a plain-C function so
  // can't be a class member), we can get a reference to the wrapper class
  cairo_font_face_set_user_data(m_cobject, &user_font_key, this, nullptr);
  cairo_user_font_face_set_init_func(cobj(), init_cb);
  cairo_user_font_face_set_render_glyph_func(cobj(), render_glyph_cb);
  cairo_user_font_face_set_unicode_to_glyph_func(cobj(), unicode_to_glyph_cb);
  cairo_user_font_face_set_text_to_glyphs_func(cobj(), text_to_glyphs_cb);
}

UserFontFace::~UserFontFace()
{
}

#ifdef CAIRO_HAS_FT_FONT

RefPtr<FtFontFace>
FtFontFace::create(FT_Face face, int load_flags)
{
  return make_refptr_for_instance<FtFontFace>(new FtFontFace(face, load_flags));
}

FtFontFace::FtFontFace(FT_Face face, int load_flags) :
  FontFace(cairo_ft_font_face_create_for_ft_face (face, load_flags),
           true /* has reference*/)
{
  check_status_and_throw_exception(cairo_font_face_status(m_cobject));
}

#ifdef CAIRO_HAS_FC_FONT
RefPtr<FtFontFace>
FtFontFace::create(FcPattern* pattern)
{
  return make_refptr_for_instance<FtFontFace>(new FtFontFace(pattern));
}

FtFontFace::FtFontFace(FcPattern* pattern) :
  FontFace(cairo_ft_font_face_create_for_pattern (pattern),
           true /* has reference*/)
{
  check_status_and_throw_exception(cairo_font_face_status(m_cobject));
}
#endif // CAIRO_HAS_FC_FONT

void FtFontFace::set_synthesize(FtSynthesize synth_flags)
{
  cairo_ft_font_face_set_synthesize(m_cobject, synth_flags);
}

void FtFontFace::unset_synthesize(FtSynthesize synth_flags)
{
  cairo_ft_font_face_unset_synthesize(m_cobject, synth_flags);
}

FtSynthesize FtFontFace::get_synthesize() const
{
  return (FtSynthesize)cairo_ft_font_face_get_synthesize(m_cobject);
}

#endif // CAIRO_HAS_FT_FONT

} //namespace Cairo

// vim: ts=2 sw=2 et
