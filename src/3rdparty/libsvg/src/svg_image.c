/* svg_image.c: Data structures for SVG image elements

   Copyright (C) 2002 USC/Information Sciences Institute
   Copyright (C) 2009 Philip de Nier <philipn@users.sourceforge.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Carl Worth <cworth@isi.edu>
*/

#include "svgint.h"

#include <png.h>
#include <jpeglib.h>
#include <jerror.h>
#include <setjmp.h>


typedef struct svg_image_source {
    struct jpeg_source_mgr jpg_src_mgr;
    int is_file;
    FILE *file;
    const char *buffer;
    size_t buffer_size;
    size_t buffer_pos;
} svg_image_source_t;

typedef struct _svg_image_jpeg_err {
    struct jpeg_error_mgr pub;  /* "public" fields */
    jmp_buf setjmp_buf;         /* for return to caller */
} svg_image_jpeg_err_t;




static void
_svg_png_buffer_read (png_struct *png, unsigned char *data, size_t length)
{
    svg_image_source_t *source;

    source = (svg_image_source_t *) png_get_io_ptr (png);
    if (source == NULL) {
        png_error (png, "buffer read callback: null i/o pointer");
        return;
    }

    if (length > source->buffer_size - source->buffer_pos) {
        png_error (png, "buffer read callback: requested read past end of buffer");
        return;
    }

    memcpy (data, source->buffer + source->buffer_pos, length);

    source->buffer_pos += length;
}

static void
_svg_jpeg_init_source (struct jpeg_decompress_struct *cinfo)
{
    svg_image_source_t *source = (svg_image_source_t *) cinfo->src;

    source->jpg_src_mgr.next_input_byte = (const unsigned char *) source->buffer;
    source->jpg_src_mgr.bytes_in_buffer = source->buffer_size;
}

static boolean
_svg_jpeg_fill_input_buffer (struct jpeg_decompress_struct *cinfo)
{
    return 1;
}

static void
_svg_jpeg_skip_input_data (struct jpeg_decompress_struct *cinfo, long num_bytes)
{
    svg_image_source_t *source = (svg_image_source_t *) cinfo->src;

    if (source->jpg_src_mgr.bytes_in_buffer < (size_t) num_bytes) {
        source->jpg_src_mgr.next_input_byte = (const unsigned char *) source->buffer;
        source->jpg_src_mgr.bytes_in_buffer = 0;
    } else {
        source->jpg_src_mgr.next_input_byte += num_bytes;
        source->jpg_src_mgr.bytes_in_buffer -= num_bytes;
    }
}

static void
_svg_jpeg_term_source (struct jpeg_decompress_struct *cinfo)
{
    svg_image_source_t *source = (svg_image_source_t *) cinfo->src;

    source->jpg_src_mgr.next_input_byte = (const unsigned char *) source->buffer;
    source->jpg_src_mgr.bytes_in_buffer = 0;
}

static void
premultiply_data (png_structp png, png_row_infop row_info, png_bytep data)
{
    unsigned int i;

    for (i = 0; i < row_info->rowbytes; i += 4) {
        unsigned char *b = &data[i];
        unsigned char alpha = b[3];
        unsigned long pixel = ((((b[0] * alpha) / 255) << 0) |
                               (((b[1] * alpha) / 255) << 8) |
                               (((b[2] * alpha) / 255) << 16) | (alpha << 24));
        unsigned long *p = (unsigned long *) b;
        *p = pixel;
    }
}

static svg_status_t
_svg_image_read_png (svg_image_source_t *source, char **data, unsigned int *width,
                     unsigned int *height)
{
    unsigned int i;
#define PNG_SIG_SIZE 8
    unsigned char png_sig[PNG_SIG_SIZE];
    int sig_bytes;
    png_struct *png;
    png_info *info;
    png_uint_32 png_width, png_height;
    int depth, color_type, interlace;
    unsigned int pixel_size;
    png_byte **row_pointers;

    if (source->is_file) {
        sig_bytes = (int) fread (png_sig, 1, PNG_SIG_SIZE, source->file);
    } else {
        if (PNG_SIG_SIZE > source->buffer_size)
            sig_bytes = (int) source->buffer_size;
        else
            sig_bytes = PNG_SIG_SIZE;

        memcpy (png_sig, source->buffer, sig_bytes);
    }

    if (png_check_sig (png_sig, sig_bytes) == 0)
        return SVGINT_STATUS_IMAGE_NOT_PNG;

    /* XXX: Perhaps we'll want some other error handlers? */
    png = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png == NULL)
        return SVG_STATUS_NO_MEMORY;

    info = png_create_info_struct (png);
    if (info == NULL) {
        png_destroy_read_struct (&png, NULL, NULL);
        return SVG_STATUS_NO_MEMORY;
    }

    if (source->is_file) {
        png_init_io (png, source->file);
        png_set_sig_bytes (png, sig_bytes);
    } else {
        source->buffer_pos = 0;
        png_set_read_fn (png, (void *) source, _svg_png_buffer_read);
    }

    png_read_info (png, info);

    png_get_IHDR (png, info, &png_width, &png_height, &depth, &color_type, &interlace, NULL, NULL);
    *width = png_width;
    *height = png_height;

    /* XXX: I still don't know what formats will be exported in the
       libsvg -> svg_render_engine interface. For now, I'm converting
       everything to 32-bit RGBA. */

    /* convert palette/gray image to rgb */
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb (png);

    /* expand gray bit depth if needed */
    if (color_type == PNG_COLOR_TYPE_GRAY && depth < 8)
#if PNG_LIBPNG_VER > 10209  /* png_set_gray_1_2_4_to_8 deprecated in v1.2.9 */
        png_set_expand_gray_1_2_4_to_8 (png);
#else
        png_set_gray_1_2_4_to_8 (png);
#endif

    /* transform transparency to alpha */
    if (png_get_valid (png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha (png);

    if (depth == 16)
        png_set_strip_16 (png);

    if (depth < 8)
        png_set_packing (png);

    /* convert grayscale to RGB */
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb (png);

    if (interlace != PNG_INTERLACE_NONE)
        png_set_interlace_handling (png);

    png_set_bgr (png);
    png_set_filler (png, 0xff, PNG_FILLER_AFTER);

    png_set_read_user_transform_fn (png, premultiply_data);

    png_read_update_info (png, info);

    pixel_size = 4;
    *data = malloc (png_width * png_height * pixel_size);
    if (*data == NULL)
        return SVG_STATUS_NO_MEMORY;

    row_pointers = malloc (png_height * sizeof (char *));
    for (i = 0; i < png_height; i++)
        row_pointers[i] = (png_byte *) (*data + i * png_width * pixel_size);

    png_read_image (png, row_pointers);
    png_read_end (png, info);

    free (row_pointers);

    png_destroy_read_struct (&png, &info, NULL);

    return SVG_STATUS_SUCCESS;
}

static void
_svg_image_jpeg_error_exit (j_common_ptr cinfo)
{
    svgint_status_t status;
    svg_image_jpeg_err_t *err = (svg_image_jpeg_err_t *) cinfo->err;

    /* Are there any other error codes we might care about? */
    switch (err->pub.msg_code) {
    case JERR_NO_SOI:
        status = SVGINT_STATUS_IMAGE_NOT_JPEG;
        break;
    default:
        status = SVG_STATUS_PARSE_ERROR;
        break;
    }

    longjmp (err->setjmp_buf, status);
}

static svg_status_t
_svg_image_read_jpeg (svg_image_source_t *source, char **data, unsigned int *width,
                      unsigned int *height)
{
    svgint_status_t status;
    struct jpeg_decompress_struct cinfo;
    svg_image_jpeg_err_t jpeg_err;
    JSAMPARRAY buf;
    unsigned int i, row_stride;
    unsigned char *out, *in;

    cinfo.err = jpeg_std_error (&jpeg_err.pub);
    jpeg_err.pub.error_exit = _svg_image_jpeg_error_exit;

    status = setjmp (jpeg_err.setjmp_buf);
    if (status)
        return status;

    jpeg_create_decompress (&cinfo);
    if (source->is_file) {
        jpeg_stdio_src (&cinfo, source->file);
    } else {
        source->jpg_src_mgr.init_source = _svg_jpeg_init_source;
        source->jpg_src_mgr.fill_input_buffer = _svg_jpeg_fill_input_buffer;
        source->jpg_src_mgr.skip_input_data = _svg_jpeg_skip_input_data;
        source->jpg_src_mgr.resync_to_restart = jpeg_resync_to_restart;
        source->jpg_src_mgr.term_source = _svg_jpeg_term_source;

        source->jpg_src_mgr.next_input_byte = (const unsigned char *) source->buffer;
        source->jpg_src_mgr.bytes_in_buffer = source->buffer_size;

        cinfo.src = (struct jpeg_source_mgr *) &source;
    }
    jpeg_read_header (&cinfo, TRUE);
    jpeg_start_decompress (&cinfo);

    row_stride = cinfo.output_width * cinfo.output_components;
    *width = cinfo.output_width;
    *height = cinfo.output_height;
    buf = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) & cinfo, JPOOL_IMAGE, row_stride, 1);

    *data = malloc (cinfo.output_width * cinfo.output_height * 4);
    out = (unsigned char *) *data;
    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines (&cinfo, buf, 1);
        in = buf[0];
        for (i = 0; i < cinfo.output_width; i++) {
            switch (cinfo.num_components) {
            case 1:
                out[3] = 0xff;
                out[2] = in[0];
                out[1] = in[1];
                out[0] = in[2];
                in += 1;
                out += 4;
                break;
            default:
            case 4:
                out[3] = 0xff;
                out[2] = in[0];
                out[1] = in[1];
                out[0] = in[2];
                in += 3;
                out += 4;
            }
        }
    }
    jpeg_finish_decompress (&cinfo);
    jpeg_destroy_decompress (&cinfo);

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_image_read_image (svg_image_source_t *source, char **data, unsigned int *width,
                       unsigned int *height)
{
    svgint_status_t status;

    status = _svg_image_read_png (source, data, width, height);
    if (status == 0)
        return SVG_STATUS_SUCCESS;

    if (status != SVGINT_STATUS_IMAGE_NOT_PNG)
        return status;

    status = _svg_image_read_jpeg (source, data, width, height);
    if (status == 0)
        return SVG_STATUS_SUCCESS;

    /* XXX: need to support SVG images as well */

    if (status != SVGINT_STATUS_IMAGE_NOT_JPEG)
        return status;

    return SVG_STATUS_PARSE_ERROR;
}

static svg_status_t
_svg_image_read_image_file (const char *filename, char **data, unsigned int *width,
                            unsigned int *height)
{
    svg_image_source_t source;
    svg_status_t status;

    source.is_file = 1;
    source.file = fopen (filename, "rb");
    if (source.file == NULL)
        return SVG_STATUS_FILE_NOT_FOUND;

    status = _svg_image_read_image (&source, data, width, height);

    fclose (source.file);

    return status;
}

static svg_status_t
_svg_image_read_image_buffer (const char *buffer, size_t buffer_size, char **data,
                              unsigned int *width, unsigned int *height)
{
    svg_image_source_t source;

    source.is_file = 0;
    source.buffer = buffer;
    source.buffer_size = buffer_size;
    source.buffer_pos = 0;

    return _svg_image_read_image (&source, data, width, height);
}




svg_status_t
_svg_image_init (svg_image_t *image)
{
    _svg_view_box_init (&image->view_box_template);

    _svg_length_init_unit (&image->x, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&image->y, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);
    _svg_length_init_unit (&image->width, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&image->height, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);

    image->uri = NULL;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_image_init_copy (svg_image_t *image, svg_image_t *other)
{
    svg_status_t status;

    *image = *other;
    image->uri = NULL;

    if (other->uri != NULL) {
        status = _svg_uri_clone (other->uri, &image->uri);
        if (status)
            return status;
    }

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_image_deinit (svg_image_t *image)
{
    if (image->uri != NULL)
        _svg_destroy_uri (image->uri);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_image_apply_attributes (svg_element_t *image_element, const svg_qattrs_t *attributes)
{
    svg_image_t *image = &image_element->e.image;
    const char *aspect, *href;
    svg_uri_t *uri;
    svg_status_t status;

    status = _svg_attribute_get_length (attributes, "x", &image->x, "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (image_element, "x", status);

    status = _svg_attribute_get_length (attributes, "y", &image->y, "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (image_element, "y", status);


    status = _svg_attribute_get_length (attributes, "width", &image->width, "0");
    if (status)
        return _svg_element_return_property_error (image_element, "width", status);
    if (image->width.value < 0)
        return _svg_element_return_property_error (image_element, "width",
                                                   SVG_STATUS_INVALID_VALUE);

    status = _svg_attribute_get_length (attributes, "height", &image->height, "0");
    if (status)
        return _svg_element_return_property_error (image_element, "height", status);
    if (image->height.value < 0)
        return _svg_element_return_property_error (image_element, "height",
                                                   SVG_STATUS_INVALID_VALUE);


    _svg_attribute_get_string (attributes, "preserveAspectRatio", &aspect, "xMidYMid meet");
    status = _svg_view_box_parse_aspect_ratio (aspect, &image->view_box_template);
    if (status)
        return _svg_element_return_property_error (image_element, "preserveAspectRatio", status);


    status = _svg_attribute_get_string_ns (attributes, XLINK_NAMESPACE_INDEX, "href", &href, NULL);
    if (status)
        return _svg_element_return_property_error (image_element, "href", status);

    status = _svg_create_uri (href, &uri);
    if (status)
        return _svg_element_return_property_error (image_element, "href", status);

    if (_svg_uri_is_data_scheme (uri)) {
        image->uri = uri;
        uri = NULL;
    } else if (href[0] != '\0' && _svg_uri_is_relative (uri)) {
        status = _svg_uri_create_absolute (image_element->node->base_uri, uri, &image->uri);

        _svg_destroy_uri (uri);

        if (status)
            return _svg_element_return_property_error (image_element, "href", status);
    } else {
        image->uri = uri;
        uri = NULL;
    }

    status = _svg_register_image_uri (image_element->svg, image->uri, &image->index);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

int
_svg_image_peek_render (svg_element_t *element, svg_render_engine_t *engine)
{
    if (element->e.image.width.value == 0 || element->e.image.height.value == 0)
        return 0;

    return _svg_engine_support_render_image (engine);
}

svg_status_t
_svg_image_render (svg_element_t *element, svg_render_engine_t *engine, void *closure)
{
    svg_image_t *image = &element->e.image;
    svg_status_t status;
    const char *uri_str;

    if (image->width.value == 0 || image->height.value == 0)
        return SVG_STATUS_SUCCESS;

    uri_str = _svg_get_image_uri (element->svg, image->index);
    if (uri_str == NULL)
        return SVG_STATUS_INTERNAL_ERROR;


    status = _svg_engine_begin_element (engine, closure, element->id, element->klass);
    if (status)
        return status;

    status = _svg_transform_render (&element->transform, engine, closure);
    if (status)
        return status;

    status = _svg_engine_end_transform (engine, closure);
    if (status)
        return status;

    status = _svg_style_render (element->node, engine, closure);
    if (status)
        return status;


    status = _svg_engine_render_image (engine, closure, uri_str, image->index,
                                       &image->view_box_template, &image->x, &image->y,
                                       &image->width, &image->height);
    if (status)
        return status;

    status = _svg_engine_end_element (engine, closure);
    if (status)
        return status;


    return SVG_STATUS_SUCCESS;
}

svg_status_t
svg_get_bgra_image (const char *uri_str, unsigned char **image_data, unsigned int *image_data_width,
                    unsigned int *image_data_height)
{
    svg_status_t status;
    char *filename;
    int is_temp_copy;
    unsigned char *data;
    size_t data_size;
    char *media_type;

    if (svg_is_data_scheme_uri (uri_str)) {
        status = svg_decode_data_scheme_uri (uri_str, &media_type, &data, &data_size);
        if (status)
            return status;

        status = svg_get_bgra_image_from_buffer (media_type, data, data_size, image_data,
                                                 image_data_width, image_data_height);

        if (media_type != NULL)
            free (media_type);
        if (data != NULL)
            free (data);

        if (status)
            return status;
    } else {

        status = svg_get_image_filename (uri_str, &filename, &is_temp_copy);
        if (status)
            return status;

        status = _svg_image_read_image_file (filename, (char **) image_data, image_data_width,
                                             image_data_height);

        if (is_temp_copy)
            remove (filename);
        free (filename);

        if (status)
            return status;
    }

    return SVG_STATUS_SUCCESS;
}

svg_status_t
svg_get_bgra_image_from_buffer (const char *media_type, const unsigned char *buffer,
                                size_t buffer_size, unsigned char **data,
                                unsigned int *data_width, unsigned int *data_height)
{
    return _svg_image_read_image_buffer ((char *) buffer, buffer_size, (char **) data, data_width,
                                         data_height);
}

svg_status_t
svg_get_image_filename (const char *uri_str, char **filename, int *is_temp_copy)
{
    svg_status_t status;
    svg_uri_t *uri;

    if (uri_str == NULL || strlen (uri_str) == 0)
        return SVG_STATUS_FILE_NOT_FOUND;

    status = _svg_create_uri (uri_str, &uri);
    if (status)
        return status;

    status = _svg_resource_get_access_filename (uri, filename, is_temp_copy);

    _svg_destroy_uri (uri);

    return status;
}

svg_status_t
svg_get_relative_uri (svg_t *svg, const char *uri_str, char **rel_uri_str)
{
    svg_uri_t *uri;
    svg_uri_t *rel_uri;
    svg_status_t status;

    if (uri_str == NULL || svg->document_uri == NULL)
        return SVG_STATUS_INVALID_CALL;

    status = _svg_create_uri (uri_str, &uri);
    if (status)
        return status;

    if (_svg_uri_is_data_scheme (uri)) {
        _svg_destroy_uri (uri);
        return SVG_STATUS_INVALID_CALL;
    }

    status = _svg_uri_create_relative (svg->document_uri, uri, &rel_uri);
    if (status) {
        _svg_destroy_uri (uri);
        return status;
    }

    _svg_destroy_uri (uri);

    status = _svg_uri_to_string (rel_uri, rel_uri_str);
    if (status) {
        _svg_destroy_uri (rel_uri);
        return status;
    }

    _svg_destroy_uri (rel_uri);

    return SVG_STATUS_SUCCESS;
}

