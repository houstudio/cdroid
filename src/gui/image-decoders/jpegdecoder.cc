#include <stdio.h>
#include <stddef.h>
#include <setjmp.h>
#include <jpeglib.h>
#include <turbojpeg.h>
#include <image-decoders/imagedecoder.h>
#include <cdlog.h>

namespace cdroid{

struct decoder_error_mgr {
    struct jpeg_error_mgr pub; // "public" fields for IJG library
    jmp_buf setjmp_buffer;     // For handling catastropic errors
};

struct JpegStream {
    jpeg_source_mgr pub;
    std::istream* stream;
    unsigned char buffer [4096];
};

static void init_source (j_decompress_ptr cinfo) {
    auto src = (JpegStream*)(cinfo->src);
}

static void handle_jpeg_error(j_common_ptr cinfo) {
    struct decoder_error_mgr *err = (struct decoder_error_mgr*)(cinfo->err);
    jpeg_destroy_decompress((j_decompress_ptr)cinfo);
    longjmp(err->setjmp_buffer, 1);
    LOGE("JPEG read/write error");
}

static boolean fill_buffer (j_decompress_ptr cinfo) {
    // Read to buffer
    JpegStream* src = (JpegStream*)(cinfo->src);
    src->stream->read((char*)src->buffer,sizeof(src->buffer));
    src->pub.next_input_byte = src->buffer;
    src->pub.bytes_in_buffer = src->stream->gcount();// How many yo could read

    return (!src->stream->eof())||src->pub.bytes_in_buffer;
}

static void skip (j_decompress_ptr cinfo, long num_bytes) {
    JpegStream*src = (JpegStream*)cinfo->src;
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

static int gcd(int a, int b) {
    if (b == 0) return a;
    return gcd(b, a % b);
}

static void decimalToFraction(double decimal,int& scale_num,int& scale_denom) {
    const int precision = 1000000;
    const int numerator = decimal * precision;
    const int denominator = precision;
    const int commonDivisor = gcd(numerator, denominator);
    scale_num   = numerator / commonDivisor;
    scale_denom = denominator / commonDivisor;
}

struct PRIVATE{
    struct jpeg_decompress_struct cinfo;
    struct decoder_error_mgr jerr;
};

JPEGDecoder::JPEGDecoder(std::istream&stm):ImageDecoder(stm){
    mPrivate = new  PRIVATE;
    struct jpeg_decompress_struct* cinfo = &mPrivate->cinfo;
    struct decoder_error_mgr* jerr = &mPrivate->jerr;

    cinfo->err = jpeg_std_error(&jerr->pub);
    jerr->pub.error_exit = handle_jpeg_error;
    jpeg_create_decompress(cinfo);
    make_jpeg_stream(cinfo,istream);
}

JPEGDecoder::~JPEGDecoder(){
   delete mPrivate;
}

Cairo::RefPtr<Cairo::ImageSurface> JPEGDecoder::decode(float scale,void*targetProfile){
    Cairo::RefPtr<Cairo::ImageSurface>image;
    struct jpeg_decompress_struct* cinfo = &mPrivate->cinfo;
    struct decoder_error_mgr* jerr = &mPrivate->jerr;
    int scale_num,scale_denom;
    JSAMPROW row_pointer[1];
    // initialize jpeg decompression structures
    if (setjmp(jerr->setjmp_buffer)){
        return nullptr;
    }
    if((mImageWidth==-1)||(mImageHeight==-1)){
        decodeSize();
    }

    decimalToFraction(scale,scale_num,scale_denom);
    if(scale_num){
        cinfo->scale_num  = scale_num;
        cinfo->scale_denom= scale_denom;
    }

#ifdef LIBJPEG_TURBO_VERSION
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    cinfo->out_color_space = JCS_EXT_BGRA;
#else
    cinfo->out_color_space = JCS_EXT_ARGB;
#endif
#else
    cinfo->out_color_space = JCS_RGB;
#endif
    // start decompressor
    (void) jpeg_start_decompress(cinfo);
    // create Cairo image surface
    image = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, cinfo->output_width, cinfo->output_height);
    if (image ==nullptr) {
        jpeg_destroy_decompress(cinfo);
        return nullptr;
    }

    // loop over all scanlines and fill Cairo image surface
    while (cinfo->output_scanline < cinfo->output_height) {
        unsigned char *row_address = image->get_data() +cinfo->output_scanline * image->get_stride();
        row_pointer[0] = row_address;
        if(jpeg_read_scanlines(cinfo, row_pointer, 1)!=1){
            LOGW("jpeg data corrupt at scanline=%d/%d",cinfo->output_scanline,cinfo->output_height);
            break;
        }
#ifndef LIBJPEG_TURBO_VERSION
        pix_conv(row_address, 4, row_address, 3, cinfo->output_width);
#endif
    }

    // finish and close everything
    (void) jpeg_finish_decompress(cinfo);
    jpeg_destroy_decompress(cinfo);

    // set jpeg mime data
    image->set_mime_data(CAIRO_MIME_TYPE_JPEG, nullptr, 0, nullptr);

    return image;
}

bool JPEGDecoder::decodeSize(){
    struct jpeg_decompress_struct* cinfo = &mPrivate->cinfo;
    jpeg_read_header(cinfo, TRUE);
    mImageWidth = cinfo->image_width;
    mImageHeight= cinfo->image_height;
    LOGD("imagesize=%dx%d",mImageWidth,mImageHeight);
    return true;
}
}/*endof namespacee*/
