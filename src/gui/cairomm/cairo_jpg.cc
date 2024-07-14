/* Copyright (C) 2020 The CDROID Development Team
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <setjmp.h>
#include <cairo.h>
#include <jpeglib.h>
#include <turbojpeg.h>
#include <cairo_jpg.h>
#include <istream>
#include <cdtypes.h>
#include <cdlog.h>

//
/*! Define this to use an alternate implementation of
 * cairo_image_surface_create_from_jpeg() which fstat(3)s the file before
 * reading (see below). For huge files this /may/ be slightly faster. */
#undef CAIRO_JPEG_USE_FSTAT

/*! This is the read block size for the stream reader
 * cairo_image_surface_create_from_jpeg_stream(). */
#ifdef USE_CAIRO_READ_FUNC_LEN_T
#define CAIRO_JPEG_IO_BLOCK_SIZE 4096
#else
/*! Block size has to be one if cairo_read_func_t is in use because of the lack
 * to detect EOF (truncated reads). */
#define CAIRO_JPEG_IO_BLOCK_SIZE 4096
/*! In case of original cairo_read_func_t is used fstat() should be used for
 * performance reasons (see CAIRO_JPEG_USE_FSTAT above). */
#define CAIRO_JPEG_USE_FSTAT
#endif


#if defined(ENABLE_JPEG)||defined(ENABLE_TURBOJPEG)
struct JpegStream {
    jpeg_source_mgr pub;
    std::istream* stream;
    unsigned char buffer [4096];
};
static void init_source (j_decompress_ptr cinfo) {
    auto src = reinterpret_cast<JpegStream*>(cinfo->src);
}

static boolean fill_buffer (j_decompress_ptr cinfo) {
    // Read to buffer
    JpegStream* src = reinterpret_cast<JpegStream*>(cinfo->src);
    src->stream->read((char*)src->buffer,sizeof(src->buffer));
    src->pub.next_input_byte = src->buffer;
    src->pub.bytes_in_buffer = src->stream->gcount();// How many yo could read

    return (!src->stream->eof())||src->pub.bytes_in_buffer;
}

static void skip (j_decompress_ptr cinfo, long num_bytes) {
    JpegStream*src = reinterpret_cast<JpegStream*>(cinfo->src);
    if (num_bytes > 0) {
        while (num_bytes > (long)src->pub.bytes_in_buffer) {
            num_bytes -= (long)src->pub.bytes_in_buffer;
            (void)(*src->pub.fill_input_buffer) (cinfo);
        }
        src->pub.next_input_byte += (size_t)num_bytes;
        src->pub.bytes_in_buffer -= (size_t)num_bytes;
    }
}

static void term (j_decompress_ptr cinfo) {
    // Close the stream, can be nop
}

static void make_jpeg_stream (j_decompress_ptr cinfo, std::istream* in) {
    JpegStream * src;
    if (cinfo->src == NULL) {/* first time for this JPEG object? */
        cinfo->src =(struct jpeg_source_mgr*)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo,0/*POOL_PERMANENT*/,sizeof(JpegStream));
        src = reinterpret_cast<JpegStream*> (cinfo->src);
    }
    src = reinterpret_cast<JpegStream*> (cinfo->src);
    src->pub.init_source = init_source;
    src->pub.fill_input_buffer = fill_buffer;
    src->pub.skip_input_data = skip;
    src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
    src->pub.term_source = term;
    src->stream = in;
    src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
    src->pub.next_input_byte = NULL; /* until buffer loaded */
}
#ifndef LIBJPEG_TURBO_VERSION
/*! This function makes a covnersion for "odd" pixel sizes which typically is a
 * conversion from a 3-byte to a 4-byte (or more) pixel size or vice versa.
 * The conversion is done from the source buffer src to the destination buffer
 * dst. The caller MUST ensure that src and dst have the correct memory size.
 * This is dw * num for dst and sw * num for src. src and dst may point to the
 * same memory address.
 * @param dst Pointer to destination buffer.
 * @param dw Pixel width (in bytes) of pixels in destination buffer, dw >= 3.
 * @param src Pointer to source buffer.
 * @param sw Pixel width (in bytes) of pixels in source buffer, sw >= 3.
 * @param num Number of pixels to convert, num >= 1; */
static void pix_conv(unsigned char *dst, int dw, const unsigned char *src, int sw, int num) {
    int si, di;

    // safety check
    if (dw < 3 || sw < 3 || dst == NULL || src == NULL)
        return;

    num--;
    for (si = num * sw, di = num * dw; si >= 0; si -= sw, di -= dw) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        dst[di + 2] = src[si    ];
        dst[di + 1] = src[si + 1];
        dst[di + 0] = src[si + 2];
#else
        // FIXME: This is untested, it may be wrong.
        dst[di - 3] = src[si - 3];
        dst[di - 2] = src[si - 2];
        dst[di - 1] = src[si - 1];
#endif
    }
}
#endif


/*! This function creates a JPEG file in memory from a Cairo image surface.
 * @param sfc Pointer to a Cairo surface. It should be an image surface of
 * either CAIRO_FORMAT_ARGB32 or CAIRO_FORMAT_RGB24. Other formats are
 * converted to CAIRO_FORMAT_RGB24 before compression.
 * Please note that this may give unexpected results because JPEG does not
 * support transparency. Thus, default background color is used to replace
 * transparent regions. The default background color is black if not specified
 * explicitly. Thus converting e.g. PDF surfaces without having any specific
 * background color set will apear with black background and not white as you
 * might expect. In such cases it is suggested to manually convert the surface
 * to RGB24 before calling this function.
 * @param data Pointer to a memory pointer. This parameter receives a pointer
 * to the memory area where the final JPEG data is found in memory. This
 * function reserves the memory properly and it has to be freed by the caller
 * with free(3).
 * @param len Pointer to a variable of type size_t which will receive the final
 * lenght of the memory buffer.
 * @param quality Compression quality, 0-100.
 * @return On success the function returns CAIRO_STATUS_SUCCESS. In case of
 * error CAIRO_STATUS_INVALID_FORMAT is returned. */
cairo_status_t cairo_image_surface_write_to_jpeg_mem(cairo_surface_t *sfc, unsigned char **data, size_t *len, int quality) {
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    cairo_surface_t *other = NULL;

    // check valid input format (must be IMAGE_SURFACE && (ARGB32 || RGB24))
    if (cairo_surface_get_type(sfc) != CAIRO_SURFACE_TYPE_IMAGE ||
            (cairo_image_surface_get_format(sfc) != CAIRO_FORMAT_ARGB32 &&
             cairo_image_surface_get_format(sfc) != CAIRO_FORMAT_RGB24)) {
        // create a similar surface with a proper format if supplied input format
        // does not fulfill the requirements
        double x1, y1, x2, y2;
        other = sfc;
        cairo_t *ctx = cairo_create(other);
        // get extents of original surface
        cairo_clip_extents(ctx, &x1, &y1, &x2, &y2);
        cairo_destroy(ctx);

        // create new image surface
        sfc = cairo_surface_create_similar_image(other, CAIRO_FORMAT_RGB24, x2 - x1, y2 - y1);
        if (cairo_surface_status(sfc) != CAIRO_STATUS_SUCCESS)
            return CAIRO_STATUS_INVALID_FORMAT;

        // paint original surface to new surface
        ctx = cairo_create(sfc);
        cairo_set_source_surface(ctx, other, 0, 0);
        cairo_paint(ctx);
        cairo_destroy(ctx);
    }

    // finish queued drawing operations
    cairo_surface_flush(sfc);

    // init jpeg compression structures
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    // set compression parameters
    jpeg_mem_dest(&cinfo, data, (unsigned long*)len);
    cinfo.image_width = cairo_image_surface_get_width(sfc);
    cinfo.image_height = cairo_image_surface_get_height(sfc);
#ifdef LIBJPEG_TURBO_VERSION
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    //cinfo.in_color_space = JCS_EXT_BGRX;
    cinfo.in_color_space = cairo_image_surface_get_format(sfc) == CAIRO_FORMAT_ARGB32 ? JCS_EXT_BGRA : JCS_EXT_BGRX;
#else
    //cinfo.in_color_space = JCS_EXT_XRGB;
    cinfo.in_color_space = cairo_image_surface_get_format(sfc) == CAIRO_FORMAT_ARGB32 ? JCS_EXT_ARGB : JCS_EXT_XRGB;
#endif
    cinfo.input_components = 4;
#else//LIBJPEG_TURBO_VERSION
    cinfo.in_color_space = JCS_RGB;
    cinfo.input_components = 3;
#endif
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);

    // start compressor
    jpeg_start_compress(&cinfo, TRUE);

    // loop over all lines and compress
    while (cinfo.next_scanline < cinfo.image_height) {
#ifdef LIBJPEG_TURBO_VERSION
        row_pointer[0] = cairo_image_surface_get_data(sfc) + (cinfo.next_scanline
                         * cairo_image_surface_get_stride(sfc));
#else
        unsigned char row_buf[3 * cinfo.image_width];
        pix_conv(row_buf, 3, cairo_image_surface_get_data(sfc) +
                 (cinfo.next_scanline * cairo_image_surface_get_stride(sfc)), 4, cinfo.image_width);
        row_pointer[0] = row_buf;
#endif
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    // finalize and close everything
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    // destroy temporary image surface (if available)
    if (other != NULL)
        cairo_surface_destroy(sfc);

    return CAIRO_STATUS_SUCCESS;
}


/*! This is the internal write function which is called by
 * cairo_image_surface_write_to_jpeg(). It is not exported. */
static cairo_status_t cj_write(void *closure, const unsigned char *data, unsigned int length) {
    return write((intptr_t) closure, data, length) < length ?
           CAIRO_STATUS_WRITE_ERROR : CAIRO_STATUS_SUCCESS;
}


/*! This function writes JPEG file data from a Cairo image surface by using the
 * user-supplied stream writer function write_func().
 * @param sfc Pointer to a Cairo *image* surface. Its format must either be
 * CAIRO_FORMAT_ARGB32 or CAIRO_FORMAT_RGB24. Other formats are not supported
 * by this function, yet.
 * @param write_func Function pointer to a function which is actually writing
 * the data.
 * @param closure Pointer to user-supplied variable which is directly passed to
 * write_func().
 * @param quality Compression quality, 0-100.
 * @return This function calles cairo_image_surface_write_to_jpeg_mem() and
 * returns its return value. */
cairo_status_t cairo_image_surface_write_to_jpeg_stream(cairo_surface_t *sfc, cairo_write_func_t write_func, void *closure, int quality) {
    cairo_status_t e;
    unsigned char *data = NULL;
    size_t len = 0;

    // create JPEG data in memory from surface
    if ((e = cairo_image_surface_write_to_jpeg_mem(sfc, &data, &len, quality)) != CAIRO_STATUS_SUCCESS)
        return e;

    // write whole memory block with stream function
    e = write_func(closure, data, len);

    // free JPEG memory again and return the return value
    free(data);
    return e;

}


/*! This function creates a JPEG file from a Cairo image surface.
 * @param sfc Pointer to a Cairo *image* surface. Its format must either be
 * CAIRO_FORMAT_ARGB32 or CAIRO_FORMAT_RGB24. Other formats are not supported
 * by this function, yet.
 * @param filename Pointer to the filename.
 * @param quality Compression quality, 0-100.
 * @return In case of success CAIRO_STATUS_SUCCESS is returned. If an error
 * occured while opening/creating the file CAIRO_STATUS_DEVICE_ERROR is
 * returned. The error can be tracked down by inspecting errno(3). The function
 * internally calles cairo_image_surface_write_to_jpeg_stream() and returnes
 * its return value respectively (see there). */
cairo_status_t cairo_image_surface_write_to_jpeg(cairo_surface_t *sfc, const char *filename, int quality) {
    cairo_status_t e;
    int outfile;

    // Open/create new file
    if ((outfile = open(filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
        return CAIRO_STATUS_DEVICE_ERROR;

    // write surface to file
    e = cairo_image_surface_write_to_jpeg_stream(sfc, cj_write, (void*)(intptr_t) outfile, quality);

    // close file again and return
    close(outfile);
    return e;
}



/*! This function decompresses a JPEG image from a memory buffer and creates a
 * Cairo image surface.
 * @param data Pointer to JPEG data (i.e. the full contents of a JPEG file read
 * into this buffer).
 * @param len Length of buffer in bytes.
 * @return Returns a pointer to a cairo_surface_t structure. It should be
 * checked with cairo_surface_status() for errors. */
#define MIN(a,b)((a)>(b)?(b):(a))
struct decoder_error_mgr {
    struct jpeg_error_mgr pub; // "public" fields for IJG library
    jmp_buf setjmp_buffer;     // For handling catastropic errors
};

static void handle_jpeg_error(j_common_ptr cinfo) {
    struct decoder_error_mgr *err = (struct decoder_error_mgr*)(cinfo->err);
    LOGE("JPEG read/write error");
    jpeg_destroy_decompress((j_decompress_ptr)cinfo);
    longjmp(err->setjmp_buffer, 1);
}

cairo_surface_t *cairo_image_surface_create_from_jpeg_stdstream(std::istream&is) {
    struct jpeg_decompress_struct cinfo;
    struct decoder_error_mgr jerr;
    JSAMPROW row_pointer[1];
    cairo_surface_t *sfc;
    // initialize jpeg decompression structures
    if (setjmp(jerr.setjmp_buffer)){
        return sfc;
    }
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = handle_jpeg_error;
    jpeg_create_decompress(&cinfo);
    make_jpeg_stream(&cinfo,&is);
    (void) jpeg_read_header(&cinfo, TRUE);
    if(cinfo.image_width>1920&&cinfo.image_height>1080){
       cinfo.scale_num=1;
       cinfo.scale_denom=MIN(cinfo.image_width/1920,cinfo.image_height/1080);
    }

#ifdef LIBJPEG_TURBO_VERSION
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    cinfo.out_color_space = JCS_EXT_BGRA;
#else
    cinfo.out_color_space = JCS_EXT_ARGB;
#endif
#else
    cinfo.out_color_space = JCS_RGB;
#endif
    // start decompressor
    (void) jpeg_start_decompress(&cinfo);
    // create Cairo image surface
    sfc = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, cinfo.output_width, cinfo.output_height);
    if (cairo_surface_status(sfc) != CAIRO_STATUS_SUCCESS) {
        jpeg_destroy_decompress(&cinfo);
        return sfc;
    }

    // loop over all scanlines and fill Cairo image surface
    while (cinfo.output_scanline < cinfo.output_height) {
        unsigned char *row_address = cairo_image_surface_get_data(sfc) +
                                     (cinfo.output_scanline * cairo_image_surface_get_stride(sfc));
        row_pointer[0] = row_address;
        if(jpeg_read_scanlines(&cinfo, row_pointer, 1)!=1){
            LOGD("jpeg data corrupt at scanline=%d/%d",cinfo.output_scanline,cinfo.output_height);
            break;
        }
#ifndef LIBJPEG_TURBO_VERSION
        pix_conv(row_address, 4, row_address, 3, cinfo.output_width);
#endif
    }

    // finish and close everything
    cairo_surface_mark_dirty(sfc);
    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    // set jpeg mime data
    cairo_surface_set_mime_data(sfc, CAIRO_MIME_TYPE_JPEG, NULL, 0, NULL,NULL);

    return sfc;
}

#ifdef ENABLE_TURBOJPEG

cairo_surface_t *cairo_image_surface_create_from_turbojpeg_stdstream(std::istream&is){
    int width,height,subsamp,colorspace;
    unsigned char*buffer=NULL;
    std::streambuf::pos_type buffersize=0;
    cairo_surface_t*sfc=NULL;
    tjhandle handle=NULL;

    LOGV("srteam.good=%d fail=%d",is.good(),is.fail());
    if(!is.good())goto decend;

    is.seekg(0,std::ios::end);
    LOGV("seekg.state=%d %d,%d,%d,%d",is.rdstate(),is.good(),is.bad(),is.fail(),is.eof());
    buffersize=is.tellg(); 
    LOGV("buffersize=%d",buffersize);
    if((buffersize<=0)||is.fail())goto decend;

    buffer=(unsigned char*)malloc(buffersize);
    if(NULL==buffer)goto decend;

    is.seekg(0,std::ios::beg);
    is.read((char*)buffer,buffersize);

    handle=tjInitDecompress();
    tjDecompressHeader3(handle,buffer,buffersize,&width,&height,&subsamp,&colorspace);
    if(width>1920&&height>1080){
        int numScalingFactors;
        tjscalingfactor *factors=tjGetScalingFactors(&numScalingFactors);
        for(int i=0;i<numScalingFactors;i++){
            if( (width*factors[i].num/factors[i].denom<=1920)||(height*factors[i].num/factors[i].denom<=1080)){
                width=width*factors[i].num/factors[i].denom;
                height=height*factors[i].num/factors[i].denom;
            }
        }
    }
    sfc = cairo_image_surface_create(CAIRO_FORMAT_RGB24,width,height);
    LOGV("jpeg.size=%dx%d colorspace=%d pitch=%d",width,height,colorspace,cairo_image_surface_get_stride(sfc));

    tjDecompress2(handle,buffer,buffersize,
          cairo_image_surface_get_data(sfc),
          cairo_image_surface_get_width(sfc),
          cairo_image_surface_get_stride(sfc),
          cairo_image_surface_get_height(sfc),
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
          TJPF_BGRX,TJFLAG_FASTDCT);
#else
          TJPF_RGBX,TJFLAG_FASTDCT);
#endif
decend:
    cairo_surface_set_mime_data(sfc, CAIRO_MIME_TYPE_JPEG, NULL, 0, NULL,NULL);
    if(handle)tjDestroy(handle);
    if(buffer)free(buffer);
    return sfc;
}
#endif

/*! This function decompresses a JPEG image from a memory buffer and creates a
 * Cairo image surface.
 * @param data Pointer to JPEG data (i.e. the full contents of a JPEG file read
 * into this buffer).
 * @param len Length of buffer in bytes.
 * @return Returns a pointer to a cairo_surface_t structure. It should be
 * checked with cairo_surface_status() for errors. */
cairo_surface_t *cairo_image_surface_create_from_jpeg_mem(void *data, size_t len) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    cairo_surface_t *sfc;

    // initialize jpeg decompression structures
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, (unsigned char*)data, len);
    (void) jpeg_read_header(&cinfo, TRUE);

#ifdef LIBJPEG_TURBO_VERSION
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    cinfo.out_color_space = JCS_EXT_BGRA;
#else
    cinfo.out_color_space = JCS_EXT_ARGB;
#endif
#else
    cinfo.out_color_space = JCS_RGB;
#endif

    // start decompressor
    (void) jpeg_start_decompress(&cinfo);

    // create Cairo image surface
    sfc = cairo_image_surface_create(CAIRO_FORMAT_RGB24, cinfo.output_width, cinfo.output_height);
    if (cairo_surface_status(sfc) != CAIRO_STATUS_SUCCESS) {
        jpeg_destroy_decompress(&cinfo);
        return sfc;
    }

    // loop over all scanlines and fill Cairo image surface
    while (cinfo.output_scanline < cinfo.output_height) {
        unsigned char *row_address = cairo_image_surface_get_data(sfc) +
                                     (cinfo.output_scanline * cairo_image_surface_get_stride(sfc));
        row_pointer[0] = row_address;
        (void) jpeg_read_scanlines(&cinfo, row_pointer, 1);
#ifndef LIBJPEG_TURBO_VERSION
        pix_conv(row_address, 4, row_address, 3, cinfo.output_width);
#endif
    }

    // finish and close everything
    cairo_surface_mark_dirty(sfc);
    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    // set jpeg mime data
    cairo_surface_set_mime_data(sfc, CAIRO_MIME_TYPE_JPEG, (unsigned char*)data, len, free, data);

    return sfc;
}


/*! This function reads an JPEG image from a stream and creates a Cairo image
 * surface.
 * @param read_func Pointer to function which reads data.
 * @param closure Pointer which is passed to read_func().
 * @return Returns a pointer to a cairo_surface_t structure. It should be
 * checked with cairo_surface_status() for errors.
 * @note If the surface returned is invalid you can use errno(3) to determine
 * further reasons. Errno is set according to realloc(3). If you
 * intend to check errno you shall set it to 0 before calling this function
 * because it modifies errno only in case of an error.
 */
#ifdef USE_CAIRO_READ_FUNC_LEN_T
cairo_surface_t *cairo_image_surface_create_from_jpeg_stream(cairo_read_func_len_t read_func, void *closure)
#else
cairo_surface_t *cairo_image_surface_create_from_jpeg_stream(cairo_read_func_t read_func, void *closure)
#endif
{
    void *data, *tmp;
    ssize_t len, rlen;
    int eof = 0;

    // read all data into memory buffer in blocks of CAIRO_JPEG_IO_BLOCK_SIZE
    for (len = 0, data = NULL; !eof; len += rlen) {
        // grow memory buffer and check for error
        if ((tmp = realloc(data, len + CAIRO_JPEG_IO_BLOCK_SIZE)) == NULL)
            break;
        data = tmp;

        // read bytes into buffer and check for error
        rlen = read_func(closure,(unsigned char*) data + len, CAIRO_JPEG_IO_BLOCK_SIZE);
#ifdef USE_CAIRO_READ_FUNC_LEN_T
        // check for error
        if (rlen == -1)
            break;

        // check if EOF occured
        if (rlen < CAIRO_JPEG_IO_BLOCK_SIZE)
            eof++;
#else
        // check for error
        if (rlen == CAIRO_STATUS_READ_ERROR)
            eof++;
        else rlen=CAIRO_JPEG_IO_BLOCK_SIZE;
#endif
    }

    // check for error in read loop
    if (!eof) {
        free(data);
        return cairo_image_surface_create(CAIRO_FORMAT_INVALID, 0, 0);
    }

    // call jpeg decompression and return surface
    return cairo_image_surface_create_from_jpeg_mem(data, len);
}


#ifdef CAIRO_JPEG_USE_FSTAT
/*! This function reads an JPEG image from a file an creates a Cairo image
 * surface. Internally the filesize is determined with fstat(2) and then the
 * whole data is read at once.
 * @param filename Pointer to filename of JPEG file.
 * @return Returns a pointer to a cairo_surface_t structure. It should be
 * checked with cairo_surface_status() for errors.
 * @note If the returned surface is invalid you can use errno to determine
 * further reasons. Errno is set according to fopen(3) and malloc(3). If you
 * intend to check errno you shall set it to 0 before calling this function
 * because it does not modify errno itself. */
cairo_surface_t *cairo_image_surface_create_from_jpeg(const char *filename) {
    void *data;
    int infile;
    struct stat stat;

    // open input file
    if ((infile = open(filename, O_RDONLY)) == -1)
        return cairo_image_surface_create(CAIRO_FORMAT_INVALID, 0, 0);

    // get stat structure for file size
    if (fstat(infile, &stat) == -1)
        return cairo_image_surface_create(CAIRO_FORMAT_INVALID, 0, 0);

    // allocate memory
    if ((data = malloc(stat.st_size)) == NULL)
        return cairo_image_surface_create(CAIRO_FORMAT_INVALID, 0, 0);

    // read data
    if (read(infile, data, stat.st_size) < stat.st_size){
        free(data);
        return cairo_image_surface_create(CAIRO_FORMAT_INVALID, 0, 0);
    }
    close(infile);

    cairo_surface_t*result = cairo_image_surface_create_from_jpeg_mem(data, stat.st_size);
    free(data);
    return result;
}

#else


/*! This is the read function which is called by
 * cairo_image_surface_create_from_jpeg_stream() (non-fstat-version below). It
 * is not exported. */
#ifdef USE_CAIRO_READ_FUNC_LEN_T
static ssize_t cj_read(void *closure, unsigned char *data, unsigned int length) {
    return read((intptr_t) closure, data, length);
}
#else
static cairo_status_t cj_read(void *closure, unsigned char *data, unsigned int length) {
    return read((intptr_t) closure, data, length) < length ? CAIRO_STATUS_READ_ERROR : CAIRO_STATUS_SUCCESS;
}
#endif


/*! This function reads an JPEG image from a file an creates a Cairo image
 * surface. Internally the function calls
 * cairo_image_surface_create_from_jpeg_stream() to actually read the data.
 * @param filename Pointer to filename of JPEG file.
 * @return Returns a pointer to a cairo_surface_t structure. It should be
 * checked with cairo_surface_status() for errors.
 * @note If the returned surface is invalid you can use errno to determine
 * further reasons. Errno is set according to fopen(3) and malloc(3). If you
 * intend to check errno you shall set it to 0 before calling this function
 * because it does not modify errno itself. */
cairo_surface_t *cairo_image_surface_create_from_jpeg(const char *filename) {
    cairo_surface_t *sfc;
    int infile;

    // open input file
    if ((infile = open(filename, O_RDONLY)) == -1)
        return cairo_image_surface_create(CAIRO_FORMAT_INVALID, 0, 0);

    // call stream loading function
    sfc = cairo_image_surface_create_from_jpeg_stream(cj_read, (void*)(intptr_t) infile);
    close(infile);

    return sfc;
}

#endif
#endif //define(ENABLE_JPEG)||defined(ENABLE_TURBOJPEG)

