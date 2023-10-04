#ifndef _CAMERA_PIXEL_FORMAT_H_
#define _CAMERA_PIXEL_FORMAT_H_

/*
 * 和 module_driver下头文件camera_pixel_format.h保持一致
 */


/*  Four-character-code (FOURCC) */
#define camera_fourcc(a, b, c, d)\
    ((unsigned int)(a) | ((unsigned int)(b) << 8) | ((unsigned int)(c) << 16) | ((unsigned int)(d) << 24))

#define camera_fourcc_be(a, b, c, d)  (camera_fourcc(a, b, c, d) | (1 << 31))

/*
 * Camera Pixel Format (re-define from videodev2.h)
 */
typedef enum {
    /* RGB formats */
    CAMERA_PIX_FMT_RGB332     = camera_fourcc('R', 'G', 'B', '1'), /*  8  RGB-3-3-2     */
    CAMERA_PIX_FMT_RGB444     = camera_fourcc('R', '4', '4', '4'), /* 16  xxxxrrrr ggggbbbb */
    CAMERA_PIX_FMT_ARGB444    = camera_fourcc('A', 'R', '1', '2'), /* 16  aaaarrrr ggggbbbb */
    CAMERA_PIX_FMT_XRGB444    = camera_fourcc('X', 'R', '1', '2'), /* 16  xxxxrrrr ggggbbbb */
    CAMERA_PIX_FMT_RGB555     = camera_fourcc('R', 'G', 'B', 'O'), /* 16  RGB-5-5-5     */
    CAMERA_PIX_FMT_ARGB555    = camera_fourcc('A', 'R', '1', '5'), /* 16  ARGB-1-5-5-5  */
    CAMERA_PIX_FMT_XRGB555    = camera_fourcc('X', 'R', '1', '5'), /* 16  XRGB-1-5-5-5  */
    CAMERA_PIX_FMT_RGB565     = camera_fourcc('R', 'G', 'B', 'P'), /* 16  RGB-5-6-5     */
    CAMERA_PIX_FMT_RGB555X    = camera_fourcc('R', 'G', 'B', 'Q'), /* 16  RGB-5-5-5 BE  */
    CAMERA_PIX_FMT_ARGB555X   = camera_fourcc_be('A', 'R', '1', '5'), /* 16  ARGB-5-5-5 BE */
    CAMERA_PIX_FMT_XRGB555X   = camera_fourcc_be('X', 'R', '1', '5'), /* 16  XRGB-5-5-5 BE */
    CAMERA_PIX_FMT_RGB565X    = camera_fourcc('R', 'G', 'B', 'R'), /* 16  RGB-5-6-5 BE  */
    CAMERA_PIX_FMT_BGR666     = camera_fourcc('B', 'G', 'R', 'H'), /* 18  BGR-6-6-6     */
    CAMERA_PIX_FMT_BGR24      = camera_fourcc('B', 'G', 'R', '3'), /* 24  BGR-8-8-8     */
    CAMERA_PIX_FMT_RGB24      = camera_fourcc('R', 'G', 'B', '3'), /* 24  RGB-8-8-8     */
    CAMERA_PIX_FMT_RBG24      = camera_fourcc('R', 'B', 'G', '3'), /* 24  RGB-8-8-8     */
    CAMERA_PIX_FMT_GBR24      = camera_fourcc('G', 'B', 'R', '3'), /* 24  RGB-8-8-8     */
    CAMERA_PIX_FMT_BGR32      = camera_fourcc('B', 'G', 'R', '4'), /* 32  BGR-8-8-8-8   */
    CAMERA_PIX_FMT_ABGR32     = camera_fourcc('A', 'R', '2', '4'), /* 32  BGRA-8-8-8-8  */
    CAMERA_PIX_FMT_XBGR32     = camera_fourcc('X', 'R', '2', '4'), /* 32  BGRX-8-8-8-8  */
    CAMERA_PIX_FMT_RGB32      = camera_fourcc('R', 'G', 'B', '4'), /* 32  RGB-8-8-8-8   */
    CAMERA_PIX_FMT_ARGB32     = camera_fourcc('B', 'A', '2', '4'), /* 32  ARGB-8-8-8-8  */
    CAMERA_PIX_FMT_XRGB32     = camera_fourcc('B', 'X', '2', '4'), /* 32  XRGB-8-8-8-8  */

    /* Grey formats */
    CAMERA_PIX_FMT_GREY       = camera_fourcc('G', 'R', 'E', 'Y'), /*  8  Greyscale     */
    CAMERA_PIX_FMT_Y4         = camera_fourcc('Y', '0', '4', ' '), /*  4  Greyscale     */
    CAMERA_PIX_FMT_Y6         = camera_fourcc('Y', '0', '6', ' '), /*  6  Greyscale     */
    CAMERA_PIX_FMT_Y10        = camera_fourcc('Y', '1', '0', ' '), /* 10  Greyscale     */
    CAMERA_PIX_FMT_Y12        = camera_fourcc('Y', '1', '2', ' '), /* 12  Greyscale     */
    CAMERA_PIX_FMT_Y16        = camera_fourcc('Y', '1', '6', ' '), /* 16  Greyscale     */
    CAMERA_PIX_FMT_Y16_BE     = camera_fourcc_be('Y', '1', '6', ' '), /* 16  Greyscale BE  */

    /* Luminance+Chrominance formats */
    CAMERA_PIX_FMT_YVU410     = camera_fourcc('Y', 'V', 'U', '9'), /*  9  YVU 4:1:0     */
    CAMERA_PIX_FMT_YVU420     = camera_fourcc('Y', 'V', '1', '2'), /* 12  YVU 4:2:0     */
    CAMERA_PIX_FMT_JZ420B     = camera_fourcc('J', 'Z', '1', '2'), /* 12  YUV 4:2:0 B   */
    CAMERA_PIX_FMT_YUYV       = camera_fourcc('Y', 'U', 'Y', 'V'), /* 16  YUV 4:2:2     */
    CAMERA_PIX_FMT_YYUV       = camera_fourcc('Y', 'Y', 'U', 'V'), /* 16  YUV 4:2:2     */
    CAMERA_PIX_FMT_YVYU       = camera_fourcc('Y', 'V', 'Y', 'U'), /* 16 YVU 4:2:2 */
    CAMERA_PIX_FMT_UYVY       = camera_fourcc('U', 'Y', 'V', 'Y'), /* 16  YUV 4:2:2     */
    CAMERA_PIX_FMT_VYUY       = camera_fourcc('V', 'Y', 'U', 'Y'), /* 16  YUV 4:2:2     */
    CAMERA_PIX_FMT_YUV422P    = camera_fourcc('4', '2', '2', 'P'), /* 16  YVU422 planar */
    CAMERA_PIX_FMT_YUV411P    = camera_fourcc('4', '1', '1', 'P'), /* 16  YVU411 planar */
    CAMERA_PIX_FMT_Y41P       = camera_fourcc('Y', '4', '1', 'P'), /* 12  YUV 4:1:1     */
    CAMERA_PIX_FMT_YUV444     = camera_fourcc('Y', '4', '4', '4'), /* 16  xxxxyyyy uuuuvvvv */
    CAMERA_PIX_FMT_YUV555     = camera_fourcc('Y', 'U', 'V', 'O'), /* 16  YUV-5-5-5     */
    CAMERA_PIX_FMT_YUV565     = camera_fourcc('Y', 'U', 'V', 'P'), /* 16  YUV-5-6-5     */
    CAMERA_PIX_FMT_YUV32      = camera_fourcc('Y', 'U', 'V', '4'), /* 32  YUV-8-8-8-8   */
    CAMERA_PIX_FMT_YUV410     = camera_fourcc('Y', 'U', 'V', '9'), /*  9  YUV 4:1:0     */
    CAMERA_PIX_FMT_YUV420     = camera_fourcc('Y', 'U', '1', '2'), /* 12  YUV 4:2:0     */
    CAMERA_PIX_FMT_HI240      = camera_fourcc('H', 'I', '2', '4'), /*  8  8-bit color   */
    CAMERA_PIX_FMT_HM12       = camera_fourcc('H', 'M', '1', '2'), /*  8  YUV 4:2:0 16x16 macroblocks */
    CAMERA_PIX_FMT_M420       = camera_fourcc('M', '4', '2', '0'), /* 12  YUV 4:2:0 2 lines y, 1 line uv interleaved */

    /* two planes -- one Y, one Cr + Cb interleaved  */
    CAMERA_PIX_FMT_NV12       = camera_fourcc('N', 'V', '1', '2'), /* 12  Y/CbCr 4:2:0  */
    CAMERA_PIX_FMT_NV21       = camera_fourcc('N', 'V', '2', '1'), /* 12  Y/CrCb 4:2:0  */
    CAMERA_PIX_FMT_NV16       = camera_fourcc('N', 'V', '1', '6'), /* 16  Y/CbCr 4:2:2  */
    CAMERA_PIX_FMT_NV61       = camera_fourcc('N', 'V', '6', '1'), /* 16  Y/CrCb 4:2:2  */
    CAMERA_PIX_FMT_NV24       = camera_fourcc('N', 'V', '2', '4'), /* 24  Y/CbCr 4:4:4  */
    CAMERA_PIX_FMT_NV42       = camera_fourcc('N', 'V', '4', '2'), /* 24  Y/CrCb 4:4:4  */

    /* Bayer formats - see http://www.siliconimaging.com/RGB%20Bayer.htm */
    CAMERA_PIX_FMT_SBGGR8     = camera_fourcc('B', 'A', '8', '1'), /*  8  BGBG.. GRGR.. */
    CAMERA_PIX_FMT_SGBRG8     = camera_fourcc('G', 'B', 'R', 'G'), /*  8  GBGB.. RGRG.. */
    CAMERA_PIX_FMT_SGRBG8     = camera_fourcc('G', 'R', 'B', 'G'), /*  8  GRGR.. BGBG.. */
    CAMERA_PIX_FMT_SRGGB8     = camera_fourcc('R', 'G', 'G', 'B'), /*  8  RGRG.. GBGB.. */
    CAMERA_PIX_FMT_SBGGR10    = camera_fourcc('B', 'G', '1', '0'), /* 10  BGBG.. GRGR.. */
    CAMERA_PIX_FMT_SGBRG10    = camera_fourcc('G', 'B', '1', '0'), /* 10  GBGB.. RGRG.. */
    CAMERA_PIX_FMT_SGRBG10    = camera_fourcc('B', 'A', '1', '0'), /* 10  GRGR.. BGBG.. */
    CAMERA_PIX_FMT_SRGGB10    = camera_fourcc('R', 'G', '1', '0'), /* 10  RGRG.. GBGB.. */

    CAMERA_PIX_FMT_SBGGR12    = camera_fourcc('B', 'G', '1', '2'), /* 12  BGBG.. GRGR.. */
    CAMERA_PIX_FMT_SGBRG12    = camera_fourcc('G', 'B', '1', '2'), /* 12  GBGB.. RGRG.. */
    CAMERA_PIX_FMT_SGRBG12    = camera_fourcc('B', 'A', '1', '2'), /* 12  GRGR.. BGBG.. */
    CAMERA_PIX_FMT_SRGGB12    = camera_fourcc('R', 'G', '1', '2'), /* 12  RGRG.. GBGB.. */

    /* Add New Item */
    CAMERA_PIX_FMT_SBGGR16    = camera_fourcc('B', 'Y', 'R', '2'), /* 16  BGBG.. GRGR.. */
    CAMERA_PIX_FMT_SGBRG16    = camera_fourcc('Y', 'B', 'R', '2'), /* 16  GBGB.. RGRG.. */
    CAMERA_PIX_FMT_SGRBG16    = camera_fourcc('Y', 'R', 'B', '2'), /* 16  GRGR.. BGBG.. */
    CAMERA_PIX_FMT_SRGGB16    = camera_fourcc('R', 'Y', 'B', '2'), /* 16  RGRG.. GBGB.. */
} camera_pixel_fmt;


/*
 * Sensor Pixel Format (re-define from media-bus-format.h)
 */
typedef enum {
    SENSOR_PIXEL_FMT_FIXED                  = 0x0001,

    /* RGB - next is    0x1018 */
    SENSOR_PIXEL_FMT_RGB444_1X12            = 0x1016,
    SENSOR_PIXEL_FMT_RGB444_2X8_PADHI_BE    = 0x1001,
    SENSOR_PIXEL_FMT_RGB444_2X8_PADHI_LE    = 0x1002,
    SENSOR_PIXEL_FMT_RGB555_2X8_PADHI_BE    = 0x1003,
    SENSOR_PIXEL_FMT_RGB555_2X8_PADHI_LE    = 0x1004,
    SENSOR_PIXEL_FMT_RGB565_1X16            = 0x1017,
    SENSOR_PIXEL_FMT_BGR565_2X8_BE          = 0x1005,
    SENSOR_PIXEL_FMT_BGR565_2X8_LE          = 0x1006,
    SENSOR_PIXEL_FMT_RGB565_2X8_BE          = 0x1007,
    SENSOR_PIXEL_FMT_RGB565_2X8_LE          = 0x1008,
    SENSOR_PIXEL_FMT_RGB666_1X18            = 0x1009,
    SENSOR_PIXEL_FMT_RBG888_1X24            = 0x100e,
    SENSOR_PIXEL_FMT_RGB666_1X24_CPADHI     = 0x1015,
    SENSOR_PIXEL_FMT_RGB666_1X7X3_SPWG      = 0x1010,
    SENSOR_PIXEL_FMT_BGR888_1X24            = 0x1013,
    SENSOR_PIXEL_FMT_GBR888_1X24            = 0x1014,
    SENSOR_PIXEL_FMT_RGB888_1X24            = 0x100a,
    SENSOR_PIXEL_FMT_RGB888_2X12_BE         = 0x100b,
    SENSOR_PIXEL_FMT_RGB888_2X12_LE         = 0x100c,
    SENSOR_PIXEL_FMT_RGB888_1X7X4_SPWG      = 0x1011,
    SENSOR_PIXEL_FMT_RGB888_1X7X4_JEIDA     = 0x1012,
    SENSOR_PIXEL_FMT_ARGB8888_1X32          = 0x100d,
    SENSOR_PIXEL_FMT_RGB888_1X32_PADHI      = 0x100f,

    /* YUV (including grey) - next is   0x2026 */
    SENSOR_PIXEL_FMT_Y8_1X8                 = 0x2001,
    SENSOR_PIXEL_FMT_UV8_1X8                = 0x2015,
    SENSOR_PIXEL_FMT_UYVY8_1_5X8            = 0x2002,
    SENSOR_PIXEL_FMT_VYUY8_1_5X8            = 0x2003,
    SENSOR_PIXEL_FMT_YUYV8_1_5X8            = 0x2004,
    SENSOR_PIXEL_FMT_YVYU8_1_5X8            = 0x2005,
    SENSOR_PIXEL_FMT_UYVY8_2X8              = 0x2006,
    SENSOR_PIXEL_FMT_VYUY8_2X8              = 0x2007,
    SENSOR_PIXEL_FMT_YUYV8_2X8              = 0x2008,
    SENSOR_PIXEL_FMT_YVYU8_2X8              = 0x2009,
    SENSOR_PIXEL_FMT_Y10_1X10               = 0x200a,
    SENSOR_PIXEL_FMT_UYVY10_2X10            = 0x2018,
    SENSOR_PIXEL_FMT_VYUY10_2X10            = 0x2019,
    SENSOR_PIXEL_FMT_YUYV10_2X10            = 0x200b,
    SENSOR_PIXEL_FMT_YVYU10_2X10            = 0x200c,
    SENSOR_PIXEL_FMT_Y12_1X12               = 0x2013,
    SENSOR_PIXEL_FMT_UYVY12_2X12            = 0x201c,
    SENSOR_PIXEL_FMT_VYUY12_2X12            = 0x201d,
    SENSOR_PIXEL_FMT_YUYV12_2X12            = 0x201e,
    SENSOR_PIXEL_FMT_YVYU12_2X12            = 0x201f,
    SENSOR_PIXEL_FMT_UYVY8_1X16             = 0x200f,
    SENSOR_PIXEL_FMT_VYUY8_1X16             = 0x2010,
    SENSOR_PIXEL_FMT_YUYV8_1X16             = 0x2011,
    SENSOR_PIXEL_FMT_YVYU8_1X16             = 0x2012,
    SENSOR_PIXEL_FMT_YDYUYDYV8_1X16         = 0x2014,
    SENSOR_PIXEL_FMT_UYVY10_1X20            = 0x201a,
    SENSOR_PIXEL_FMT_VYUY10_1X20            = 0x201b,
    SENSOR_PIXEL_FMT_YUYV10_1X20            = 0x200d,
    SENSOR_PIXEL_FMT_YVYU10_1X20            = 0x200e,
    SENSOR_PIXEL_FMT_VUY8_1X24              = 0x2024,
    SENSOR_PIXEL_FMT_YUV8_1X24              = 0x2025,
    SENSOR_PIXEL_FMT_UYVY12_1X24            = 0x2020,
    SENSOR_PIXEL_FMT_VYUY12_1X24            = 0x2021,
    SENSOR_PIXEL_FMT_YUYV12_1X24            = 0x2022,
    SENSOR_PIXEL_FMT_YVYU12_1X24            = 0x2023,
    SENSOR_PIXEL_FMT_YUV10_1X30             = 0x2016,
    SENSOR_PIXEL_FMT_AYUV8_1X32             = 0x2017,

    /* Bayer - next is  0x3019 */
    SENSOR_PIXEL_FMT_SBGGR8_1X8             = 0x3001,
    SENSOR_PIXEL_FMT_SGBRG8_1X8             = 0x3013,
    SENSOR_PIXEL_FMT_SGRBG8_1X8             = 0x3002,
    SENSOR_PIXEL_FMT_SRGGB8_1X8             = 0x3014,

    SENSOR_PIXEL_FMT_SBGGR10_1X10           = 0x3007,
    SENSOR_PIXEL_FMT_SGBRG10_1X10           = 0x300e,
    SENSOR_PIXEL_FMT_SGRBG10_1X10           = 0x300a,
    SENSOR_PIXEL_FMT_SRGGB10_1X10           = 0x300f,
    SENSOR_PIXEL_FMT_SBGGR12_1X12           = 0x3008,
    SENSOR_PIXEL_FMT_SGBRG12_1X12           = 0x3010,
    SENSOR_PIXEL_FMT_SGRBG12_1X12           = 0x3011,
    SENSOR_PIXEL_FMT_SRGGB12_1X12           = 0x3012,

    /*
     * 以下不常用
     */
    SENSOR_PIXEL_FMT_SBGGR10_ALAW8_1X8      = 0x3015,
    SENSOR_PIXEL_FMT_SGBRG10_ALAW8_1X8      = 0x3016,
    SENSOR_PIXEL_FMT_SGRBG10_ALAW8_1X8      = 0x3017,
    SENSOR_PIXEL_FMT_SRGGB10_ALAW8_1X8      = 0x3018,
    SENSOR_PIXEL_FMT_SBGGR10_DPCM8_1X8      = 0x300b,
    SENSOR_PIXEL_FMT_SGBRG10_DPCM8_1X8      = 0x300c,
    SENSOR_PIXEL_FMT_SGRBG10_DPCM8_1X8      = 0x3009,
    SENSOR_PIXEL_FMT_SRGGB10_DPCM8_1X8      = 0x300d,
    SENSOR_PIXEL_FMT_SBGGR10_2X8_PADHI_BE   = 0x3003,
    SENSOR_PIXEL_FMT_SBGGR10_2X8_PADHI_LE   = 0x3004,
    SENSOR_PIXEL_FMT_SBGGR10_2X8_PADLO_BE   = 0x3005,
    SENSOR_PIXEL_FMT_SBGGR10_2X8_PADLO_LE   = 0x3006,

    /*
     * 以下为新增加配置
     * Bayer - next is  0x3025
     */
    SENSOR_PIXEL_FMT_SBGGR12_ALAW8_1X8      = 0x3019,   /* 8-bit Bayer BGBG/GRGR (A-law) */
    SENSOR_PIXEL_FMT_SGBRG12_ALAW8_1X8      = 0x301a,
    SENSOR_PIXEL_FMT_SGRBG12_ALAW8_1X8      = 0x301b,
    SENSOR_PIXEL_FMT_SRGGB12_ALAW8_1X8      = 0x301c,
    SENSOR_PIXEL_FMT_SBGGR12_DPCM8_1X8      = 0x301d,
    SENSOR_PIXEL_FMT_SGBRG12_DPCM8_1X8      = 0x301e,
    SENSOR_PIXEL_FMT_SGRBG12_DPCM8_1X8      = 0x301f,
    SENSOR_PIXEL_FMT_SRGGB12_DPCM8_1X8      = 0x3020,
    SENSOR_PIXEL_FMT_SBGGR12_2X8_PADHI_BE   = 0x3021,
    SENSOR_PIXEL_FMT_SBGGR12_2X8_PADHI_LE   = 0x3022,
    SENSOR_PIXEL_FMT_SBGGR12_2X8_PADLO_BE   = 0x3023,
    SENSOR_PIXEL_FMT_SBGGR12_2X8_PADLO_LE   = 0x3024,


    /* JPEG compressed formats - next is    0x4002 */
    SENSOR_PIXEL_FMT_JPEG_1X8               = 0x4001,

    /* Vendor specific formats - next is    0x5002 */

    /* S5C73M3 sensor specific interleaved UYVY and JPEG */
    SENSOR_PIXEL_FMT_S5C_UYVY_JPEG_1X8      = 0x5001,

    /* HSV - next is    0x6002 */
    SENSOR_PIXEL_FMT_AHSV8888_1X32          = 0x6001,
} sensor_pixel_fmt;


#define sensor_fmt_is_8BIT(fmt)     ( (fmt == SENSOR_PIXEL_FMT_Y8_1X8)      ||  \
                                    (fmt == SENSOR_PIXEL_FMT_SBGGR8_1X8)    ||  \
                                    (fmt == SENSOR_PIXEL_FMT_SGBRG8_1X8)    ||  \
                                    (fmt == SENSOR_PIXEL_FMT_SGRBG8_1X8)    ||  \
                                    (fmt == SENSOR_PIXEL_FMT_SRGGB8_1X8) )

#define sensor_fmt_is_YUV422(fmt)   ( (fmt == SENSOR_PIXEL_FMT_UYVY8_2X8)   ||  \
                                    (fmt == SENSOR_PIXEL_FMT_VYUY8_2X8)     ||  \
                                    (fmt == SENSOR_PIXEL_FMT_YUYV8_2X8)     ||  \
                                    (fmt == SENSOR_PIXEL_FMT_YVYU8_2X8)     ||  \
                                    (fmt == SENSOR_PIXEL_FMT_UYVY8_1X16)    ||  \
                                    (fmt == SENSOR_PIXEL_FMT_VYUY8_1X16)    ||  \
                                    (fmt == SENSOR_PIXEL_FMT_YUYV8_1X16)    ||  \
                                    (fmt == SENSOR_PIXEL_FMT_YVYU8_1X16)    ||  \
                                    (fmt == SENSOR_PIXEL_FMT_YDYUYDYV8_1X16) )

#define camera_fmt_is_8BIT(fmt)     ( (fmt == CAMERA_PIX_FMT_GREY)    ||  \
                                    (fmt == CAMERA_PIX_FMT_SBGGR8)    ||  \
                                    (fmt == CAMERA_PIX_FMT_SGBRG8)    ||  \
                                    (fmt == CAMERA_PIX_FMT_SGRBG8)    ||  \
                                    (fmt == CAMERA_PIX_FMT_SRGGB8))

#define camera_fmt_is_16BIT(fmt)    ( (fmt == CAMERA_PIX_FMT_Y16)     ||  \
                                    (fmt == CAMERA_PIX_FMT_SBGGR16)   ||  \
                                    (fmt == CAMERA_PIX_FMT_SGBRG16)   ||  \
                                    (fmt == CAMERA_PIX_FMT_SGRBG16)   ||  \
                                    (fmt == CAMERA_PIX_FMT_SRGGB16))

#define camera_fmt_is_NV12(fmt)     ( (fmt == CAMERA_PIX_FMT_NV12)    ||  \
                                    (fmt == CAMERA_PIX_FMT_NV21)      ||  \
                                    (fmt == CAMERA_PIX_FMT_YVU420)    ||  \
                                    (fmt == CAMERA_PIX_FMT_JZ420B) )

#define camera_fmt_is_YUV422(fmt)   ( (fmt == CAMERA_PIX_FMT_YUYV)    ||  \
                                    (fmt == CAMERA_PIX_FMT_YYUV)      ||  \
                                    (fmt == CAMERA_PIX_FMT_YVYU)      ||  \
                                    (fmt == CAMERA_PIX_FMT_UYVY)      ||  \
                                    (fmt == CAMERA_PIX_FMT_VYUY)      ||  \
                                    (fmt == CAMERA_PIX_FMT_YUV422P)   ||  \
                                    (fmt == CAMERA_PIX_FMT_YUV411P) )

#endif /* _CAMERA_PIXEL_FORMAT_H_ */
