#include<stdio.h>
#include<string.h>
#include<png.h>
#include<pixman.h>
#include <stdlib.h>

struct png_read_closure_t{
    void*closure;
    void*png_data;
    int(*read_func)(void*closure,png_bytep,png_size_t);
};

/* Unpremultiplies data and converts native endian ARGB => RGBA bytes */
static void unpremultiply_data (png_structp png, png_row_infop row_info, png_bytep data)
{
    unsigned int i;

    for (i = 0; i < row_info->rowbytes; i += 4) {
        uint8_t *b = &data[i];
        uint32_t pixel;
        uint8_t  alpha;

        memcpy (&pixel, b, sizeof (uint32_t));
        alpha = (pixel & 0xff000000) >> 24;
        if (alpha == 0) {
            b[0] = b[1] = b[2] = b[3] = 0;
        } else {
            b[0] = (((pixel & 0xff0000) >> 16) * 255 + alpha / 2) / alpha;
            b[1] = (((pixel & 0x00ff00) >>  8) * 255 + alpha / 2) / alpha;
            b[2] = (((pixel & 0x0000ff) >>  0) * 255 + alpha / 2) / alpha;
            b[3] = alpha;
        }
    }
}
static inline int multiply_alpha (int alpha, int color)
{
    int temp = (alpha * color) + 0x80;
    return ((temp + (temp >> 8)) >> 8);
}

/* Premultiplies data and converts RGBA bytes => native endian */
static void premultiply_data (png_structp   png,png_row_infop row_info, png_bytep     data)
{
    unsigned int i;

    for (i = 0; i < row_info->rowbytes; i += 4) {
        uint8_t *base  = &data[i];
        uint8_t  alpha = base[3];
        uint32_t p;

        if (alpha == 0) {
            p = 0;
        } else {
            uint8_t  red   = base[0];
            uint8_t  green = base[1];
            uint8_t  blue  = base[2];

            if (alpha != 0xff) {
                red   = multiply_alpha (alpha, red);
                green = multiply_alpha (alpha, green);
                blue  = multiply_alpha (alpha, blue);
            }
            p = (alpha << 24) | (red << 16) | (green << 8) | (blue << 0);
        }
        memcpy (base, &p, sizeof (uint32_t));
    }
}

/* Converts RGBx bytes to native endian xRGB */
static void convert_bytes_to_data (png_structp png, png_row_infop row_info, png_bytep data)
{
    unsigned int i;

    for (i = 0; i < row_info->rowbytes; i += 4) {
        uint8_t *base  = &data[i];
        uint8_t  red   = base[0];
        uint8_t  green = base[1];
        uint8_t  blue  = base[2];
        uint32_t pixel;

        pixel = (0xff << 24) | (red << 16) | (green << 8) | (blue << 0);
        memcpy (base, &pixel, sizeof (uint32_t));
    }
}

static void stream_read_func (png_structp png, png_bytep data, png_size_t size)
{
    int status;
    struct png_read_closure_t *png_closure;

    png_closure = (struct png_read_closure_t*)png_get_io_ptr (png);
    status = png_closure->read_func (png_closure->closure, data, size);
    if (status) {
        png_error (png, NULL);
    }
}

pixman_image_t* read_png (struct png_read_closure_t *png_closure)
{
    pixman_image_t * volatile surface;
    png_struct *png = NULL;
    png_info *info;
    png_byte * volatile data = NULL;
    png_byte ** volatile row_pointers = NULL;
    png_uint_32 png_width, png_height;
    int depth, color_type, interlace, stride;
    unsigned int i;
    pixman_format_code_t format;
    int status;
    unsigned char *mime_data;
    unsigned long mime_data_length;

    png_closure->png_data = NULL;//_cairo_memory_stream_create ();

    /* XXX: Perhaps we'll want some other error handlers? */
    png = png_create_read_struct (PNG_LIBPNG_VER_STRING,&status,NULL,NULL);//png_simple_error_callback,png_simple_warning_callback);
    if (png == NULL) {
        //surface = _cairo_surface_create_in_error (_cairo_error (CAIRO_STATUS_NO_MEMORY));
        goto BAIL;
    }

    info = png_create_info_struct (png);
    if (info == NULL) {
        //surface = _cairo_surface_create_in_error (_cairo_error (CAIRO_STATUS_NO_MEMORY));
        goto BAIL;
    }

    png_set_read_fn (png, png_closure, stream_read_func);

    status = 0;
#ifdef PNG_SETJMP_SUPPORTED
    if (setjmp (png_jmpbuf (png))) {
        //surface = _cairo_surface_create_in_error (status);
        goto BAIL;
    }
#endif
    png_read_info (png, info);

    png_get_IHDR (png, info, &png_width, &png_height, &depth, &color_type, &interlace, NULL, NULL);
    if (status) { /* catch any early warnings */
        //surface = _cairo_surface_create_in_error (status);
        goto BAIL;
    }

    /* convert palette/gray image to rgb */
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb (png);

    /* expand gray bit depth if needed */
    if (color_type == PNG_COLOR_TYPE_GRAY) {
#if PNG_LIBPNG_VER >= 10209
        png_set_expand_gray_1_2_4_to_8 (png);
#else
        png_set_gray_1_2_4_to_8 (png);
#endif
    }

    /* transform transparency to alpha */
    if (png_get_valid (png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha (png);

    if (depth == 16)
        png_set_strip_16 (png);

    if (depth < 8)
        png_set_packing (png);

    /* convert grayscale to RGB */
    if (color_type == PNG_COLOR_TYPE_GRAY ||  color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        png_set_gray_to_rgb (png);
    }

    if (interlace != PNG_INTERLACE_NONE)
        png_set_interlace_handling (png);

    png_set_filler (png, 0xff, PNG_FILLER_AFTER);
    /* recheck header after setting EXPAND options */
    png_read_update_info (png, info);
    png_get_IHDR (png, info, &png_width, &png_height, &depth,
                  &color_type, &interlace, NULL, NULL);
    if (depth != 8 || ! (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA))
    {
        //surface = _cairo_surface_create_in_error (_cairo_error (CAIRO_STATUS_READ_ERROR));
        goto BAIL;
    }

    switch (color_type) {
        default:
            break;//ASSERT_NOT_REACHED;
            /* fall-through just in case ;-) */

        case PNG_COLOR_TYPE_RGB_ALPHA:
            format = PIXMAN_a8r8g8b8;//CAIRO_FORMAT_ARGB32;
            png_set_read_user_transform_fn (png, premultiply_data);
            break;

        case PNG_COLOR_TYPE_RGB:
            format = PIXMAN_r8g8b8;//CAIRO_FORMAT_RGB24;
            png_set_read_user_transform_fn (png, convert_bytes_to_data);
            break;
    }

    //stride = cairo_format_stride_for_width (format, png_width);
    if (stride < 0) {
        //surface = _cairo_surface_create_in_error (_cairo_error (CAIRO_STATUS_INVALID_STRIDE));
        goto BAIL;
    }

    data = (png_byte*)malloc(png_height*stride);
    if (data == NULL) {
        //surface = _cairo_surface_create_in_error (_cairo_error (CAIRO_STATUS_NO_MEMORY));
        goto BAIL;
    }

    row_pointers =(png_byte**)malloc(png_height*sizeof (char *));
    if (row_pointers == NULL) {
        //surface = _cairo_surface_create_in_error (_cairo_error (CAIRO_STATUS_NO_MEMORY));
        goto BAIL;
    }

    for (i = 0; i < png_height; i++)
        row_pointers[i] = &data[i * (ptrdiff_t)stride];
    png_read_image (png, row_pointers);
    png_read_end (png, info);

    if (status) { /* catch any late warnings - probably hit an error already */
        //surface = _cairo_surface_create_in_error (status);
        goto BAIL;
    }

    surface = pixman_image_create_bits(format,png_width, png_height,(uint32_t*)data,stride);
    if (surface==NULL)
        goto BAIL;

    png_closure->png_data = NULL;
    if (status) {
        //cairo_surface_destroy (surface);
        //surface = _cairo_surface_create_in_error (status);
        goto BAIL;
    }

    if (status) {
        //cairo_surface_destroy (surface);
        //surface = _cairo_surface_create_in_error (status);
        goto BAIL;
    }

 BAIL:
    free (row_pointers);
    free (data);
    if (png != NULL)
        png_destroy_read_struct (&png, &info, NULL);
    if (png_closure->png_data != NULL) {
        //cairo_status_t status_ignored;
        //status_ignored = _cairo_output_stream_destroy (png_closure->png_data);
    }

    return surface;
}

