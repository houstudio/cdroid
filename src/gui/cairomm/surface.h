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

#ifndef __CAIROMM_SURFACE_H
#define __CAIROMM_SURFACE_H

#include <string>
#include <vector>
/* following is required for OS X */

#ifdef nil
#undef nil
//#include <sigc++/slot.h>
#define nil NULL
#else
//#include <sigc++/slot.h>
#endif

/* end OS X */

#include <cairomm/enums.h>
#include <cairomm/exception.h>
#include <cairomm/device.h>
#include <cairomm/fontoptions.h>
#include <cairomm/refptr.h>

//See xlib_surface.h for XlibSurface.
//See win32_surface.h for Win32Surface.
//See quartz_surface.h for QuartzSurface (Mac OS X).

#ifdef CAIRO_HAS_PDF_SURFACE
#include <cairo-pdf.h>
#endif // CAIRO_HAS_PDF_SURFACE
#ifdef CAIRO_HAS_PS_SURFACE
#include <cairo-ps.h>
#endif // CAIRO_HAS_PS_SURFACE
#ifdef CAIRO_HAS_SVG_SURFACE
#include <cairo-svg.h>
#endif // CAIRO_HAS_SVG_SURFACE

// Experimental surfaces
#ifdef CAIRO_HAS_GLITZ_SURFACE
#include <cairo-glitz.h>
#endif // CAIRO_HAS_GLITZ_SURFACE


namespace Cairo
{
class CAIROMM_API ImageSurface;
class CAIROMM_API MappedImageSurface;

/** A cairo surface represents an image, either as the destination of a drawing
 * operation or as source when drawing onto another surface. There are
 * different subtypes of cairo surface for different drawing backends.  This
 * class is a base class for all subtypes and should not be used directly
 *
 * Most surface types allow accessing the surface without using Cairo
 * functions. If you do this, keep in mind that it is mandatory that you call
 * Cairo::Surface::flush() before reading from or writing to the surface and that
 * you must use Cairo::Surface::mark_dirty() after modifying it.
 *
 * Surfaces are reference-counted objects that should be used via Cairo::RefPtr.
 */
class CAIROMM_API Surface
{
public:
  /**
   * %Cairo::Surface::Type is used to describe the type of a given surface. The
   * surface types are also known as "backends" or "surface backends" within
   * cairo.
   *
   * The surface type can be queried with Surface::get_type()
   *
   * The various Cairo::Surface functions can be used with surfaces of
   * any type, but some backends also provide type-specific functions
   * that must only be called with a surface of the appropriate
   * type.
   *
   * New entries may be added in future versions.
   *
   * @since 1.2
   **/
  enum class Type
  {
      /**
       * The surface is of type image
       */
      IMAGE = CAIRO_SURFACE_TYPE_IMAGE,

      /**
       * The surface is of type pdf
       */
      PDF = CAIRO_SURFACE_TYPE_PDF,

      /**
       * The surface is of type ps
       */
      PS = CAIRO_SURFACE_TYPE_PS,

      /**
       * The surface is of type xlim
       */
      XLIB = CAIRO_SURFACE_TYPE_XLIB,

      /**
       * The surface is of type xcb
       */
      XCB = CAIRO_SURFACE_TYPE_XCB,

      /**
       * The surface is of type glitz
       */
      GLITZ = CAIRO_SURFACE_TYPE_GLITZ,

      /**
       * The surface is of type quartz
       */
      QUARTZ = CAIRO_SURFACE_TYPE_QUARTZ,

#if !defined(WIN32) && !defined(CAIROMM_DISABLE_DEPRECATED)
      /**
       * The surface is of type win32
       *
       * @deprecated Use WIN32_SURFACE instead.
       */
      WIN32 = CAIRO_SURFACE_TYPE_WIN32,
#endif // !WIN32 && !CAIROMM_DISABLE_DEPRECATED
      /**
       * The surface is of type win32
       * @since 1.16.1
       */
      WIN32_SURFACE = CAIRO_SURFACE_TYPE_WIN32,

      /**
       * The surface is of type beos
       */
      BEOS = CAIRO_SURFACE_TYPE_BEOS,

      /**
       * The surface is of type directfb
       */
      DIRECTFB = CAIRO_SURFACE_TYPE_DIRECTFB,

      /**
       * The surface is of type svg
       */
      SVG = CAIRO_SURFACE_TYPE_SVG,

      /**
       * The surface is of type os2
       */
      OS2 = CAIRO_SURFACE_TYPE_OS2,

      /**
       * The surface is a win32 printing surface
       */
      WIN32_PRINTING = CAIRO_SURFACE_TYPE_WIN32_PRINTING,

      /**
       * The surface is of type quartz_image
       */
      QUARTZ_IMAGE = CAIRO_SURFACE_TYPE_QUARTZ_IMAGE,

      /**
       * The surface is of type script
       * @since 1.10
       */
      SCRIPT = CAIRO_SURFACE_TYPE_SCRIPT,

      /**
       * The surface is of type Qt
       * @since 1.10
       */
      QT = CAIRO_SURFACE_TYPE_QT,

      /**
       * The surface is of type recording
       * @since 1.10
       */
      RECORDING = CAIRO_SURFACE_TYPE_RECORDING,

      /**
       * The surface is a OpenVG surface
       * @since 1.10
       */
      VG = CAIRO_SURFACE_TYPE_VG,

      /**
       * The surface is of type OpenGL
       * @since 1.10
       */
      GL = CAIRO_SURFACE_TYPE_GL,

      /**
       * The surface is of type Direct Render Manager
       * @since 1.10
       */
      DRM = CAIRO_SURFACE_TYPE_DRM,

      /**
       * The surface is of type script 'tee' (a multiplexing surface)
       * @since 1.10
       */
      TEE = CAIRO_SURFACE_TYPE_TEE,

      /**
       * The surface is of type XML (for debugging)
       * @since 1.10
       */
      XML = CAIRO_SURFACE_TYPE_XML,

      /**
       * The surface is of type Skia
       * @since 1.10
       */
      SKIA = CAIRO_SURFACE_TYPE_SKIA,

      /**
       * The surface is of type The surface is a subsurface created with
       * Surface::create()
       * @since 1.10
       */
      SUBSURFACE = CAIRO_SURFACE_TYPE_SUBSURFACE
  };

  //TODO: Add new formats (at least 3) when we can add API. See cairo/src/cairo.h.
  /**
   * %Cairo::Surface::Format is used to identify the memory format of image data.
   *
   * New entries may be added in future versions.
   **/
  enum class Format
  {
      /**
       * Each pixel is a 32-bit quantity, with alpha in the upper 8 bits, then
       * red, then green, then blue. The 32-bit quantities are stored
       * native-endian. Pre-multiplied alpha is used. (That is, 50% transparent
       * red is 0x80800000, not 0x80ff0000.)
       */
      ARGB32 = CAIRO_FORMAT_ARGB32,

      /**
       * Each pixel is a 32-bit quantity, with the upper 8 bits unused. Red,
       * Green, and Blue are stored in the remaining 24 bits in that order.
       */
      RGB24 = CAIRO_FORMAT_RGB24,

      /**
       * Each pixel is a 8-bit quantity holding an alpha value
       */
      A8 = CAIRO_FORMAT_A8,

      /**
       * Each pixel is a 1-bit quentity holding an alpha value. Pixels are packed
       * together into 32-bit quantities. The ordering of the bits matches the
       * endianness of the platform. On a big-endian machine, the first pixel is in
       * the uppermost bit, on a little endian machine the first pixel is in the
       * least-significant bit.
       */
      A1 = CAIRO_FORMAT_A1,

      /**
       * Each pixel is a 16-bit quantity with red in the upper 5 bits, then green
       * in the middle 6 bits, and blue in the lower 5 bits
       */
      RGB16_565 = CAIRO_FORMAT_RGB16_565
  };

  /** For example:
   * <code>
   * ErrorStatus my_write_func(unsigned char* data, unsigned int length);
   * </code>
   *
   * This is the type of function which is called when a backend needs to write
   * data to an output stream. It is passed the data to write and the length of
   * the data in bytes. The write function should return CAIRO_STATUS_SUCCESS if
   * all the data was successfully written, CAIRO_STATUS_WRITE_ERROR otherwise.
   *
   * @param data the buffer containing the data to write
   * @param length the amount of data to write
   * @return the status code of the write operation
   */
  typedef ErrorStatus(*SlotWriteFunc)(const unsigned char* /*data*/, unsigned int /*length*/);

  /**
   * This is the type of function which is called when a backend needs to read
   * data from an input stream. It is passed the buffer to read the data into
   * and the length of the data in bytes. The read function should return
   * CAIRO_STATUS_SUCCESS if all the data was successfully read,
   * CAIRO_STATUS_READ_ERROR otherwise.
   *
   * @param data the buffer into which to read the data
   * @param length the amount of data to read
   * @return the status code of the read operation
   */
  typedef ErrorStatus(*SlotReadFunc)(unsigned char* /*data*/, unsigned int /*length*/);

  /** Create a C++ wrapper for the C instance. This C++ instance should then be
   * given to a RefPtr.
   *
   * @param cobject The C instance.
   * @param has_reference Whether we already have a reference. Otherwise, the
   * constructor will take an extra reference.
   */
  explicit Surface(cairo_surface_t* cobject, bool has_reference = false);

  Surface(const Surface&) = delete;
  Surface& operator=(const Surface&) = delete;

  virtual ~Surface();

  /**
   * Return mime data previously attached to surface using the specified mime
   * type. If no data has been attached with the given mime type then this
   * returns 0.
   *
   * @param mime_type The MIME type of the image data.
   * @param length This will be set to the length of the image data.
   * @returns The image data attached to the surface.
   * @since 1.10
   */
  const unsigned char* get_mime_data(const std::string& mime_type, unsigned long& length);


  /** For instance,
   * void on_destroy();
   */
  typedef void(*SlotDestroy)(void*data);

  /** Attach an image in the format mime_type to surface. To remove the data from
   * a surface, call unset_mime_data() with same mime type.
   *
   * The attached image (or filename) data can later be used by backends which
   * support it (currently: PDF, PS, SVG and Win32 Printing surfaces) to emit
   * this data instead of making a snapshot of the surface. This approach tends
   * to be faster and requires less memory and disk space.
   *
   * The recognized MIME types are the following: CAIRO_MIME_TYPE_JPEG,
   * CAIRO_MIME_TYPE_PNG, CAIRO_MIME_TYPE_JP2, CAIRO_MIME_TYPE_URI.
   *
   * See corresponding backend surface docs for details about which MIME types
   * it can handle. Caution: the associated MIME data will be discarded if you
   * draw on the surface afterwards. Use this function with care.
   *
   * @param mime_type The MIME type of the image data. param data The image
   * @data to attach to the surface. param length The length of the image data.
   * @param slot_destroy A callback slot that will be called when the Surface
   *   no longer needs the data. For instance, when the Surface is destroyed or
   *   when new image data is attached using the same MIME tpe.
   * @since 1.10
   */
  void set_mime_data(const std::string& mime_type, unsigned char* data,
                     unsigned long length, const SlotDestroy& slot_destroy);

  /** Remove the data from a surface. See set_mime_data().
   */
  void unset_mime_data(const std::string& mime_type);

  /** Retrieves the default font rendering options for the surface. This allows
   * display surfaces to report the correct subpixel order for rendering on
   * them, print surfaces to disable hinting of metrics and so forth. The
   * result can then be used with Cairo::ScaledFont::create().
   *
   * @param options 	a FontOptions object into which to store the retrieved
   * options. All existing values are overwritten
   */
  void get_font_options(FontOptions& options) const;

  /** This function finishes the surface and drops all references to external
   * resources. For example, for the Xlib backend it means that cairo will no
   * longer access the drawable, which can be freed. After calling
   * finish() the only valid operations on a surface are getting and setting
   * user data and referencing and destroying it. Further drawing to the
   * surface will not affect the surface but will instead trigger a
   * CAIRO_STATUS_SURFACE_FINISHED error.
   *
   * When the Surface is destroyed, cairo will call finish() if it hasn't been
   * called already, before freeing the resources associated with the Surface.
   */
  void finish();

  /** Do any pending drawing for the surface and also restore any temporary
   * modifications cairo has made to the surface's state. This function must
   * be called before switching from drawing on the surface with cairo to
   * drawing on it directly with native APIs. If the surface doesn't support
   * direct access, then this function does nothing.
   */
  void flush();

  /** Tells cairo to consider the data buffer dirty.
   *
   * In particular, if you've created an ImageSurface with a data buffer that
   * you've allocated yourself and you draw to that data buffer using means
   * other than cairo, you must call mark_dirty() before doing any additional
   * drawing to that surface with cairo.
   *
   * Note that if you do draw to the Surface outside of cairo, you must call
   * flush() before doing the drawing.
   */
  void mark_dirty();

  /** Marks a rectangular area of the given surface dirty.
   *
   * @param x 	 X coordinate of dirty rectangle
   * @param y 	Y coordinate of dirty rectangle
   * @param width 	width of dirty rectangle
   * @param height 	height of dirty rectangle
   */
  void mark_dirty(int x, int y, int width, int height);

  /** Sets an offset that is added to the device coordinates determined by the
   * CTM when drawing to surface. One use case for this function is when we
   * want to create a %Surface that redirects drawing for a portion of
   * an onscreen surface to an offscreen surface in a way that is completely
   * invisible to the user of the cairo API. Setting a transformation via
   * Cairo::Context::translate() isn't sufficient to do this, since functions like
   * Cairo::Context::device_to_user() will expose the hidden offset.
   *
   * Note that the offset only affects drawing to the surface, not using the
   * surface in a surface pattern.
   *
   * @param x_offset 	the offset in the X direction, in device units
   * @param y_offset 	the offset in the Y direction, in device units
   */
  void set_device_offset(double x_offset, double y_offset);

  /** Returns a previous device offset set by set_device_offset().
   */
  void get_device_offset(double& x_offset, double& y_offset) const;

  /** Sets a scale that is multiplied to the device coordinates determined by
   * the CTM when drawing to surface. One common use for this is to render to
   * very high resolution display devices at a scale factor, so that code that
   * assumes 1 pixel will be a certain size will still work. Setting a
   * transformation via Cairo::Context::scale() isn't sufficient to do this, since
   * functions like Cairo::Context::device_to_user() will expose the hidden scale.
   *
   * Note that the scale affects drawing to the surface as well as using the
   * surface in a source pattern.
   *
   * @param x_scale 	a scale factor in the X direction
   * @param y_scale 	a scale factor in the Y direction
   *
   * @newin{1,18}
   */
  void set_device_scale(double x_scale, double y_scale);

  /** Sets x and y scale to the same value.
   * See set_device_scale(double, double) for details.
   *
   * @param scale 	a scale factor in the X and Y direction
   *
   * @newin{1,18}
   */
  inline void set_device_scale(double scale) { set_device_scale(scale, scale); }

  /** Returns a previous device scale set by set_device_scale().
   *
   * @newin{1,18}
   */
  void get_device_scale(double& x_scale, double& y_scale) const;

  /** Returns the x and y average of a previous device scale set by
   * set_device_scale().
   *
   * @newin{1,18}
   */
  double get_device_scale() const;

  /**
   * Set the horizontal and vertical resolution for image fallbacks.
   *
   * When certain operations aren't supported natively by a backend, cairo will
   * fallback by rendering operations to an image and then overlaying that
   * image onto the output. For backends that are natively vector-oriented,
   * this function can be used to set the resolution used for these image
   * fallbacks, (larger values will result in more detailed images, but also
   * larger file sizes).
   *
   * Some examples of natively vector-oriented backends are the ps, pdf, and
   * svg backends.
   *
   * For backends that are natively raster-oriented, image fallbacks are still
   * possible, but they are always performed at the native device resolution.
   * So this function has no effect on those backends.
   *
   * Note: The fallback resolution only takes effect at the time of completing
   * a page (with Context::show_page() or Context::copy_page()) so there is
   * currently no way to have more than one fallback resolution in effect on a
   * single page.
   *
   * The default fallback resoultion is 300 pixels per inch in both dimensions.
   *
   * @param x_pixels_per_inch   Pixels per inch in the x direction
   * @param y_pixels_per_inch   Pixels per inch in the y direction
   *
   * @since 1.2
   */
  void set_fallback_resolution(double x_pixels_per_inch, double y_pixels_per_inch);

  /** This function returns the previous fallback resolution set by
   * set_fallback_resolution(), or default fallback resolution if never set.
   *
   * @param x_pixels_per_inch horizontal pixels per inch
   * @param y_pixels_per_inch vertical pixels per inch
   *
   * @since 1.8
   */
  void get_fallback_resolution(double& x_pixels_per_inch, double& y_pixels_per_inch) const;

  Type get_type() const;

  /**
   * This function returns the content type of surface which indicates whether
   * the surface contains color and/or alpha information.
   *
   * @since 1.8
   */
  Content get_content() const;

  /**
   * Emits the current page for backends that support multiple pages,
   * but doesn't clear it, so that the contents of the current page will
   * be retained for the next page.  Use show_page() if you want to get an empty
   * page after the emission.
   *
   * @since 1.6
   */
  void copy_page();

  /**
   * Emits and clears the current page for backends that support multiple pages.
   * Use copy_page() if you don't want to clear the page.
   *
   * @since 1.6
   */
  void show_page();

  /** Returns whether the surface supports sophisticated
   * Context::show_text_glyphs() operations.  That is, whether it actually uses
   * the provided text and cluster data to a Context::show_text_glyphs() call.
   *
   * Note: Even if this function returns %FALSE, a Context::show_text_glyphs()
   * operation targeted at this surface will still succeed.  It just will act
   * like a Context::show_glyphs() operation.  Users can use this function to
   * avoid computing UTF-8 text and cluster mapping if the target surface does
   * not use it.
   *
   * @since 1.8
   **/
  bool has_show_text_glyphs() const;

#ifdef CAIRO_HAS_PNG_FUNCTIONS

  /** Writes the contents of surface to a new file filename as a PNG image.
   *
   * @note For this function to be available, cairo must have been compiled
   * with PNG support
   *
   * @param filename	the name of a file to write to
   */
  void write_to_png(const std::string& filename);

  /** Writes the Surface to the write function.
   *
   * @note For this function to be available, cairo must have been compiled
   * with PNG support
   *
   * @param write_func  The function to be called when the backend needs to
   * write data to an output stream
   *
   * @since 1.8
   */
  void write_to_png_stream(const SlotWriteFunc& write_func);

#endif // CAIRO_HAS_PNG_FUNCTIONS

  /** Create a new image surface that is as compatible as possible for uploading
   * to and the use in conjunction with an existing surface.
   * However, this surface can still be used like any normal image surface.
   * Unlike Surface::create(const RefPtr<Surface> other, Content content,
   * int width, int height) the new image surface won't inherit the device scale
   * from this surface..
   *
   * Initially the surface contents are all 0 (transparent if contents have
   * transparency, black otherwise.)
   *
   * Use Surface::create(const RefPtr<Surface> other, Content content,
   * int width, int height) if you don't need an image surface.
   *
   * @param format The format for the new surface.
   * @param width Width of the new surface (in pixels).
   * @param height Height of the new surface (in pixels).
   *
   * @throws std::bad_alloc, Cairo::logic_error, std::ios_base::failure
   * @newin{1,20}
   */
  RefPtr<ImageSurface> create_similar_image(Format format, int width, int height);

  /** Checks whether @a mime_type is supported by the surface.
   *
   * @param mime_type The mime type.
   *
   * @newin{1,20}
   */
  bool supports_mime_type(const std::string& mime_type);

  /** Returns an image surface that is the most efficient mechanism for
   * modifying the backing store of the target surface.
   *
   * @note The use of the original surface as a target or source whilst it is
   * mapped is undefined. The result of mapping the surface multiple times is
   * undefined. Changing the device transform of the image surface or
   * of this surface before the image surface is deleted results in
   * undefined behavior.
   *
   * @param extents Limit the extraction to a rectangular region.
   *
   * @throws std::bad_alloc, Cairo::logic_error, std::ios_base::failure
   * @newin{1,20}
   */
  RefPtr<MappedImageSurface> map_to_image(const RectangleInt& extents);

  /** Returns an image surface that is the most efficient mechanism for
   * modifying the backing store of the target surface.
   * The region retrieved is the whole surface.
   *
   * @note The use of the original surface as a target or source whilst it is
   * mapped is undefined. The result of mapping the surface multiple times is
   * undefined. Changing the device transform of the image surface or
   * of this surface before the image surface is deleted results in
   * undefined behavior.
   *
   * @throws std::bad_alloc, Cairo::logic_error, std::ios_base::failure
   * @newin{1,20}
   */
  RefPtr<MappedImageSurface> map_to_image();

  /** This function returns the device for a surface
   * @return The device for this surface, or an empty RefPtr if the surface has
   * no associated device */
  RefPtr<Device> get_device();

  /** The underlying C cairo surface type
   */
  typedef cairo_surface_t cobject;
  /** Provides acces to the underlying C cairo surface
   */
  inline cobject* cobj() { return m_cobject; }
  /** Provides acces to the underlying C cairo surface
   */
  inline const cobject* cobj() const { return m_cobject; }

  #ifndef DOXYGEN_IGNORE_THIS
  ///For use only by the cairomm implementation.
  inline ErrorStatus get_status() const
  { return cairo_surface_status(const_cast<cairo_surface_t*>(cobj())); }

  void reference() const;
  void unreference() const;
  #endif //DOXYGEN_IGNORE_THIS

  /** Create a new surface that is as compatible as possible with an existing
   * surface. The new surface will use the same backend as other unless that is
   * not possible for some reason.
   *
   * @param other 	an existing surface used to select the backend of the new surface
   * @param content 	the content for the new surface
   * @param width 	width of the new surface, (in device-space units)
   * @param height 	height of the new surface (in device-space units)
   * @return 	a RefPtr to the newly allocated surface.
   */
  static RefPtr<Surface> create(const RefPtr<Surface> other, Content content, int width, int height);

  /** Create a new surface that is a rectangle within the target surface. All
   * operations drawn to this surface are then clipped and translated onto the
   * target surface. Nothing drawn via this sub-surface outside of its bounds is
   * drawn onto the target surface, making this a useful method for passing
   * constrained child surfaces to library routines that draw directly onto the
   * parent surface, i.e. with no further backend allocations, double buffering
   * or copies.
   *
   * @Note The semantics of subsurfaces have not been finalized yet unless the
   * rectangle is in full device units, is contained within the extents of the
   * target surface, and the target or subsurface's device transforms are not
   * changed.
   *
   * @param target an existing surface for which the sub-surface will point to
   * @param x the x-origin of the sub-surface from the top-left of the target surface (in device-space units)
   * @param y the y-origin of the sub-surface from the top-left of the target surface (in device-space units)
   * @param width width of the sub-surface (in device-space units)
   * @param height height of the sub-surface (in device-space units)
   *
   * @since 1.10
   */
  static RefPtr<Surface> create(const RefPtr<Surface>& target, double x, double y, double width, double height);

protected:
  /** The underlying C cairo surface type that is wrapped by this Surface
   */
  cobject* m_cobject;

}; // end class Surface

/** @example image-surface.cc
 * An example of using Cairo::ImageSurface class to render to PNG
 */

/** Image surfaces provide the ability to render to memory buffers either
 * allocated by cairo or by the calling code. The supported image formats are
 * those defined in Format
 *
 * An %ImageSurface is the most generic type of Surface and the only one that is
 * available by default.  You can either create an %ImageSurface whose data is
 * managed by Cairo, or you can create an %ImageSurface with a data buffer that
 * you allocated yourself so that you can have full access to the data.
 *
 * When you create an %ImageSurface with your own data buffer, you are free to
 * examine the results at any point and do whatever you want with it.  Note that
 * if you modify anything and later want to continue to draw to the surface
 * with cairo, you must let cairo know via Cairo::Surface::mark_dirty()
 *
 * Note that like all surfaces, an %ImageSurface is a reference-counted object that should be used via Cairo::RefPtr.
 */
class CAIROMM_API ImageSurface : public Surface
{
protected:
  //TODO?: Surface(cairo_surface_t *target);

public:

  /** Create a C++ wrapper for the C instance. This C++ instance should then be
   * given to a RefPtr.
   * @param cobject The C instance.
   * @param has_reference Whether we already have a reference. Otherwise, the
   * constructor will take an extra reference.
   */
  explicit ImageSurface(cairo_surface_t* cobject, bool has_reference = false);

  ~ImageSurface() override;

  /** Gets the width of the ImageSurface in pixels
   */
  int get_width() const;

  /** Gets the height of the ImageSurface in pixels
   */
  int get_height() const;

  /// @{
  /**
   * Get a pointer to the data of the image surface, for direct
   * inspection or modification.
   *
   * Return value: a pointer to the image data of this surface or NULL
   * if @surface is not an image surface.
   *
   * @since 1.2
   */
  unsigned char* get_data();
  const unsigned char* get_data() const;
  /// @}

  /**
   * Gets the format of the surface
   * @since 1.2
   */
  Format get_format() const;

  /**
   * Returns the stride of the image surface in bytes (or 0 if surface is not
   * an image surface). The stride is the distance in bytes from the beginning
   * of one row of the image data to the beginning of the next row.
   *
   * @since 1.2
   */
  int get_stride() const;

  /**
   * This function provides a stride value that will respect all alignment
   * requirements of the accelerated image-rendering code within cairo. Typical
   * usage will be of the form:
   *
   * @code
   * int stride;
   * unsigned char *data;
   * Cairo::RefPtr<Cairo::ImageSurface> surface;
   *
   * stride = Cairo::ImageSurface::format_stride_for_width (format, width);
   * data = malloc (stride * height);
   * surface = Cairo::ImageSurface::create (data, format, width, height);
   * @endcode
   *
   * @param format A Format value
   * @param width The desired width of an image surface to be created.
   * @return the appropriate stride to use given the desired format and width, or
   * -1 if either the format is invalid or the width too large.
   *
   * @since 1.6
   **/
  static int format_stride_for_width(Format format, int width);

  /**
   * Creates an image surface of the specified format and dimensions. Initially
   * the surface contents are all 0. (Specifically, within each pixel, each
   * color or alpha channel belonging to format will be 0. The contents of bits
   * within a pixel, but not belonging to the given format are undefined).
   *
   * @param format 	format of pixels in the surface to create
   * @param width 	width of the surface, in pixels
   * @param height 	height of the surface, in pixels
   * @return 	a RefPtr to the newly created surface.
   */
  static RefPtr<ImageSurface> create(Format format, int width, int height);

  /**
   * Creates an image surface for the provided pixel data. The output buffer
   * must be kept around until the surface is destroyed or finish() is called on
   * the surface. The initial contents of buffer will be used as the initial
   * image contents; you must explicitly clear the buffer, using, for example,
   * Context::rectangle() and Context::fill() if you want it cleared.
   *
   * Note that the stride may be larger than @a width * @a bytes_per_pixel to
   * provide proper alignment for each pixel and row. This alignment is required
   * to allow high-performance rendering within cairo. The correct way to obtain
   * a legal stride value is to call format_stride_for_width() with the desired
   * format and maximum image width value, and the use the resulting stride
   * value to allocate the data and to create the image surface. See
   * format_stride_for_width() for example code.
   *
   * @param data a pointer to a buffer supplied by the application in which to write contents. This pointer must be suitably aligned for any kind of variable, (for example, a pointer returned by malloc).
   * @param format the format of pixels in the buffer
   * @param width the width of the image to be stored in the buffer
   * @param height the height of the image to be stored in the buffer
   * @param stride the number of bytes between the start of rows in the buffer as allocated.
   *        This value should always be computed by format_stride_for_width()
   *        before allocating the data buffer.
   * @return a RefPtr to the newly created surface.
   */
  static RefPtr<ImageSurface> create(unsigned char* data, Format format, int width, int height, int stride);

#ifdef CAIRO_HAS_PNG_FUNCTIONS

  /** Creates a new image surface and initializes the contents to the given PNG
   * file.
   *
   * @note For this function to be available, cairo must have been compiled
   * with PNG support.
   *
   * @param filename	name of PNG file to load
   * @return	a RefPtr to the new cairo_surface_t initialized with the
   * contents of the PNG image file.
   */
  static RefPtr<ImageSurface> create_from_png(std::string filename);

  /** Creates a new image surface from PNG data read incrementally via the
   * read_func function.
   *
   * @note For this function to be available, cairo must have been compiled
   * with PNG support.
   *
   * @param read_func function called to read the data of the file
   * @return a RefPtr to the new cairo_surface_t initialized with the
   * contents of the PNG image file.
   */
  static RefPtr<ImageSurface> create_from_png_stream(const SlotReadFunc& read_func);

#endif // CAIRO_HAS_PNG_FUNCTIONS

}; // end class ImageSurface

/** @example mapped-surface.cc
 * An example of using Cairo::MappedImageSurface class to render to PDF
 */

/** Image surface which is mapped to a target surface.
 *
 * @see Surface::map_to_image()
 * @newin{1,20}
 */
class CAIROMM_API MappedImageSurface : public ImageSurface
{
public:
  /** Create a C++ wrapper for the C instance. This C++ instance should then be
   * given to a RefPtr.
   *
   * @param cobject The C instance.
   * @param ctarget The surface that has been mapped to this image surface.
   * @param has_reference Whether we already have a reference. Otherwise, the
   * constructor will take an extra reference.
   *
   * @newin{1,20}
   */
  MappedImageSurface(cairo_surface_t* cobject, cairo_surface_t* ctarget,
    bool has_reference = false);

  /** Destructor.
   *
   * Unmaps this image surface from the target surface that created it.
   * The contents of the image will be uploaded to the target surface.
   *
   * @newin{1,20}
   */
  ~MappedImageSurface() override;

protected:
  /** The C cairo surface which has been mapped to this image surface.
   */
  cobject* m_ctarget;
};


/**
 * A recording surface is a surface that records all drawing operations at the
 * highest level of the surface backend interface, (that is, the level of paint,
 * mask, stroke, fill, and show_text_glyphs). The recording surface can then be
 * "replayed" against any target surface by using it as a source surface.
 *
 * If you want to replay a surface so that the results in `target` will be
 * identical to the results that would have been obtained if the original
 * operations applied to the recording surface had instead been applied to the
 * target surface, you can use code like this:
 *
 *     Cairo::RefPtr<Cairo::Context> context = Cairo::Context::create(target);
 *     context->set_source(recording_surface, 0.0, 0.0);
 *     context->paint();
 *
 * A recording surface is logically unbounded, i.e. it has no implicit
 * constraint on the size of the drawing surface. However, in practice this is
 * rarely useful as you wish to replay against a particular target surface with
 * known bounds. For this case, it is more efficient to specify the target
 * extents to the recording surface upon creation.
 *
 * The recording phase of the recording surface is careful to snapshot all
 * necessary objects (paths, patterns, etc.), in order to achieve accurate
 * replay.
 *
 * Note that like all surfaces, a %RecordingSurface is a reference-counted object
 * that should be used via Cairo::RefPtr.
 */
class CAIROMM_API RecordingSurface : public Surface
{
public:

  /**
   * Create a C++ wrapper for the C instance. This C++ instance should then be
   * given to a RefPtr.
   * @param cobject The C instance.
   * @param has_reference Whether we already have a reference. Otherwise, the
   * constructor will take an extra reference.
   */
  explicit RecordingSurface(cairo_surface_t* cobject, bool has_reference = false);

  virtual ~RecordingSurface();

  /**
   * Measures the extents of the operations stored within the recording
   * surface.  This is useful to compute the required size of an image surface
   * (or equivalent) into which to replay the full sequence of drawing
   * operations.
   * @return a Rectangle with `x` and `y` set to the coordinates of the top-left
   * of the ink bounding box, and `width` and `height` set to the width and
   * height of the ink bounding box.
   */
  Rectangle ink_extents() const;

  /**
   * Get the extents of the recording surface, if the surface is bounded.
   * @param extents the Rectangle in which to store the extents if the recording
   * surface is bounded
   * @return true if the recording surface is bounded, false if the recording
   * surface is unbounded (in which case `extents` will not be set).
   */
  bool get_extents(Rectangle& extents) const;

  /**
   * Creates a recording surface which can be used to record all drawing
   * operations at the highest level (that is, the level of paint, mask, stroke,
   * fill and show_text_glyphs). The recording surface can then be "replayed"
   * against any target surface by using it as a source to drawing operations.
   * The recording surface will be unbounded.
   *
   * The recording phase of the recording surface is careful to snapshot all
   * necessary objects (paths, patterns, etc.), in order to achieve accurate
   * replay.
   *
   * @param content the content of the recording surface; defaults to
   * Cairo::Content::CONTENT_COLOR_ALPHA.
   * @return a RefPtr to the newly created recording surface.
   */
  static RefPtr<RecordingSurface> create(Content content = Content::CONTENT_COLOR_ALPHA);

  /**
   * Creates a recording surface which can be used to record all drawing
   * operations at the highest level (that is, the level of paint, mask, stroke,
   * fill and show_text_glyphs). The recording surface can then be "replayed"
   * against any target surface by using it as a source to drawing operations.
   * The recording surface will be bounded by the given rectangle.
   *
   * The recording phase of the recording surface is careful to snapshot all
   * necessary objects (paths, patterns, etc.), in order to achieve accurate
   * replay.
   *
   * @param	extents the extents to record
   * @param	content the content of the recording surface; defaults to
   * Cairo::Content::CONTENT_COLOR_ALPHA.
   * @return	a RefPtr to the newly created recording surface.
   */
  static RefPtr<RecordingSurface> create(const Rectangle& extents, Content content = Content::CONTENT_COLOR_ALPHA);

};


#ifdef CAIRO_HAS_PDF_SURFACE

/** @example pdf-surface.cc
 * An example of using Cairo::PdfSurface class to render to PDF
 */

typedef enum
{
  PDF_VERSION_1_4 = CAIRO_PDF_VERSION_1_4,
  PDF_VERSION_1_5 = CAIRO_PDF_VERSION_1_5
} PdfVersion;

/** A %PdfSurface provides a way to render PDF documents from cairo.  This
 * surface is not rendered to the screen but instead renders the drawing to a
 * PDF file on disk.
 *
 * @note For this Surface to be available, cairo must have been compiled with
 * PDF support
 */
class CAIROMM_API PdfSurface : public Surface
{
public:

  /** Create a C++ wrapper for the C instance. This C++ instance should then be
   * given to a RefPtr.
   *
   * @param cobject The C instance.
   * @param has_reference whether we already have a reference. Otherwise, the
   * constructor will take an extra reference.
   */
  explicit PdfSurface(cairo_surface_t* cobject, bool has_reference = false);
  ~PdfSurface() override;

  /** Creates a PdfSurface with a specified dimensions that will be saved as
   * the given filename
   *
   * @param filename    The name of the PDF file to save the surface to
   * @param width_in_points   The width of the PDF document in points
   * @param height_in_points   The height of the PDF document in points
   * @since 1.2
   */
  static RefPtr<PdfSurface> create(std::string filename, double width_in_points, double height_in_points);

  /** Creates a PdfSurface with a specified dimensions that will be written to
   * the given write function instead of saved directly to disk
   *
   * @param write_func  The function to be called when the backend needs to
   * write data to an output stream
   * @param width_in_points   The width of the PDF document in points
   * @param height_in_points   The height of the PDF document in points
   * @since 1.8
   */
  static RefPtr<PdfSurface> create_for_stream(const SlotWriteFunc& write_func, double width_in_points, double height_in_points);

/**
 * Changes the size of a PDF surface for the current (and subsequent) pages.
 *
 * This function should only be called before any drawing operations have been
 * performed on the current page. The simplest way to do this is to call this
 * function immediately after creating the surface or immediately after
 * completing a page with either Context::show_page() or Context::copy_page().
 *
 * @param width_in_points new surface width, in points (1 point == 1/72.0 inch)
 * @param height_in_points new surface height, in points (1 point == 1/72.0 inch)
 **/
  void set_size(double width_in_points, double height_in_points);

  /**
   * Restricts the generated PDF file to version. See get_versions() for a list
   * of available version values that can be used here.
   *
   * This function should only be called before any drawing operations have been
   * performed on the given surface. The simplest way to do this is to call this
   * function immediately after creating the surface.
   *
   * @since 1.10
   */
  void restrict_to_version(PdfVersion version);

  /** Retrieves the list of PDF versions supported by cairo. See
   * restrict_to_version().
   *
   * @since 1.10
   */
  static const std::vector<PdfVersion> get_versions();

  /** Get the string representation of the given version id. This function will
   * return an empty string if version isn't valid. See get_versions()
   * for a way to get the list of valid version ids.
   *
   * @since 1.10
   */
  static std::string version_to_string(PdfVersion version);
};

#endif  // CAIRO_HAS_PDF_SURFACE


#ifdef CAIRO_HAS_PS_SURFACE

/** @example ps-surface.cc
 * An example of using Cairo::PsSurface class to render to PostScript
 */

/**
 * describes the language level of the PostScript Language Reference that a
 * generated PostScript file will conform to.
 */
typedef enum {
    PS_LEVEL_2 = CAIRO_PS_LEVEL_2,
    PS_LEVEL_3 = CAIRO_PS_LEVEL_3
} PsLevel;

/** A %PsSurface provides a way to render PostScript documents from cairo.  This
 * surface is not rendered to the screen but instead renders the drawing to a
 * PostScript file on disk.
 *
 * @note For this Surface to be available, cairo must have been compiled with
 * PostScript support
 */
class CAIROMM_API PsSurface : public Surface
{
public:

  /** Create a C++ wrapper for the C instance. This C++ instance should then be
   * given to a RefPtr.
   *
   * @param cobject The C instance.
   * @param has_reference whether we already have a reference. Otherwise, the
   * constructor will take an extra reference.
   */
  explicit PsSurface(cairo_surface_t* cobject, bool has_reference = false);
  ~PsSurface() override;

  /** Creates a PsSurface with a specified dimensions that will be saved as the
   * given filename
   *
   * @param filename    The name of the PostScript file to save the surface to
   * @param width_in_points   The width of the PostScript document in points
   * @param height_in_points   The height of the PostScript document in points
   */
  static RefPtr<PsSurface> create(std::string filename, double width_in_points, double height_in_points);

  /** Creates a PsSurface with a specified dimensions that will be written to
   * the given write function instead of saved directly to disk
   *
   * @param write_func  The function to be called when the backend needs to
   * write data to an output stream
   * @param width_in_points   The width of the PostScript document in points
   * @param height_in_points   The height of the PostScript document in points
   *
   * @since 1.8
   */
  static RefPtr<PsSurface> create_for_stream(const SlotWriteFunc& write_func, double width_in_points, double height_in_points);

  /**
   * Changes the size of a PostScript surface for the current (and
   * subsequent) pages.
   *
   * This function should only be called before any drawing operations have been
   * performed on the current page. The simplest way to do this is to call this
   * function immediately after creating the surface or immediately after
   * completing a page with either Context::show_page() or Context::copy_page().
   *
   * @param width_in_points new surface width, in points (1 point == 1/72.0 inch)
   * @param height_in_points new surface height, in points (1 point == 1/72.0 inch)
   */
  void set_size(double width_in_points, double height_in_points);

  /** Emit a comment into the PostScript output for the given surface.  See the
   * cairo reference documentation for more information.
   *
   * @param comment a comment string to be emitted into the PostScript output
   */
  void dsc_comment(std::string comment);

  /**
   * This function indicates that subsequent calls to dsc_comment() should direct
   * comments to the Setup section of the PostScript output.
   *
   * This function should be called at most once per surface, and must be called
   * before any call to dsc_begin_page_setup() and before any drawing is performed
   * to the surface.
   */
  void dsc_begin_setup();

  /** This function indicates that subsequent calls to dsc_comment() should
   * direct comments to the PageSetup section of the PostScript output.
   *
   * This function call is only needed for the first page of a surface. It
   * should be called after any call to dsc_begin_setup() and before any drawing
   * is performed to the surface.
   */
  void dsc_begin_page_setup();

  /**
   * If eps is true, the PostScript surface will output Encapsulated
   * PostScript.
   *
   * This function should only be called before any drawing operations
   * have been performed on the current page. The simplest way to do
   * this is to call this function immediately after creating the
   * surface. An Encapsulated Postscript file should never contain more
   * than one page.
   *
   * @since 1.6
   **/
  void set_eps(bool eps);

  /** Check whether the PostScript surface will output Encapsulated PostScript.
   *
   * @since 1.8
   */
  bool get_eps() const;

  /**
   * Restricts the generated PostSript file to @level. See get_levels() for a
   * list of available level values that can be used here.
   *
   * This function should only be called before any drawing operations have been
   * performed on the given surface. The simplest way to do this is to call this
   * function immediately after creating the surface.
   *
   * @param level PostScript level
   *
   * @since 1.6
   **/
  void restrict_to_level(PsLevel level);

  /**
   * Used to retrieve the list of supported levels. See
   * restrict_to_level().
   *
   * @since 1.6
   **/
  static const std::vector<PsLevel> get_levels();

  /**
   * Get the string representation of the given level id. This function will
   * return an empty string if level id isn't valid. See get_levels() for a way
   * to get the list of valid level ids.
   *
   * @return the string associated to given level.
   *
   * @param level a level id
   *
   * @since 1.6
   **/
  static std::string level_to_string(PsLevel level);
};

#endif // CAIRO_HAS_PS_SURFACE


#ifdef CAIRO_HAS_SVG_SURFACE

/** @example svg-surface.cc
 * An example of using Cairo::SvgSurface class to render to SVG
 */

typedef enum
{
  SVG_VERSION_1_1 = CAIRO_SVG_VERSION_1_1,
  SVG_VERSION_1_2 = CAIRO_SVG_VERSION_1_2
} SvgVersion;

/** A %SvgSurface provides a way to render Scalable Vector Graphics (SVG) images
 * from cairo.  This surface is not rendered to the screen but instead renders
 * the drawing to an SVG file on disk.
 *
 * @note For this Surface to be available, cairo must have been compiled with
 * SVG support
 */
class CAIROMM_API SvgSurface : public Surface
{
public:

  /** Create a C++ wrapper for the C instance. This C++ instance should then be
   * given to a RefPtr.
   *
   * @param cobject The C instance.
   * @param has_reference whether we already have a reference. Otherwise, the
   * constructor will take an extra reference.
   */
  explicit SvgSurface(cairo_surface_t* cobject, bool has_reference = false);
  ~SvgSurface() override;


  /** Creates a SvgSurface with a specified dimensions that will be saved as the
   * given filename
   *
   * @param filename    The name of the SVG file to save the surface to
   * @param width_in_points   The width of the SVG document in points
   * @param height_in_points   The height of the SVG document in points
   */
  static RefPtr<SvgSurface> create(std::string filename, double width_in_points, double height_in_points);

  /** Creates a SvgSurface with a specified dimensions that will be written to
   * the given write function instead of saved directly to disk
   *
   * @param write_func  The function to be called when the backend needs to
   * write data to an output stream
   * @param width_in_points   The width of the SVG document in points
   * @param height_in_points   The height of the SVG document in points
   *
   * @since 1.8
   */
  static RefPtr<SvgSurface> create_for_stream(const SlotWriteFunc& write_func, double width_in_points, double height_in_points);

  /**
   * Restricts the generated SVG file to the given version. See get_versions()
   * for a list of available version values that can be used here.
   *
   * This function should only be called before any drawing operations have been
   * performed on the given surface. The simplest way to do this is to call this
   * function immediately after creating the surface.
   *
   * @since 1.2
   */
  void restrict_to_version(SvgVersion version);

  /** Retrieves the list of SVG versions supported by cairo. See
   * restrict_to_version().
   *
   * @since 1.2
   */
  static const std::vector<SvgVersion> get_versions();

  /** Get the string representation of the given version id. The returned string
   * will be empty if version isn't valid. See get_versions() for a way to get
   * the list of valid version ids.
   *
   * since: 1.2
   */
  static std::string version_to_string(SvgVersion version);
};

#endif // CAIRO_HAS_SVG_SURFACE


/*******************************************************************************
 * THE FOLLOWING SURFACE TYPES ARE EXPERIMENTAL AND NOT FULLY SUPPORTED
 ******************************************************************************/

#ifdef CAIRO_HAS_GLITZ_SURFACE

/** A %GlitzSurface provides a way to render to the X Window System using Glitz.
 * This provides a way to use OpenGL-accelerated graphics from cairo.  If you
 * want to use hardware-accelerated graphics within the X Window system, you
 * should use this Surface type.
 *
 * @note For this Surface to be available, cairo must have been compiled with
 * Glitz support.
 *
 * @warning This was an experimental surface. Glitz support has been removed
 * from cairo 1.18.
 */
class CAIROMM_API GlitzSurface : public Surface
{

public:

  /** Create a C++ wrapper for the C instance. This C++ instance should then be
   * given to a RefPtr.
   *
   * @param cobject The C instance.
   * @param has_reference whether we already have a reference. Otherwise, the
   * constructor will take an extra reference.
   */
  explicit GlitzSurface(cairo_surface_t* cobject, bool has_reference = false);

  ~GlitzSurface() override;

  /** Creates a new GlitzSurface
   *
   * @param surface  a glitz surface type
   */
  static RefPtr<GlitzSurface> create(glitz_surface_t *surface);

};

#endif // CAIRO_HAS_GLITZ_SURFACE

} // namespace Cairo

#endif //__CAIROMM_SURFACE_H

// vim: ts=2 sw=2 et
