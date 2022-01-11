#ifndef __FY_COMM_VIDEO_H__
#define __FY_COMM_VIDEO_H__

#include "fy_type.h"
#include "fy_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define FISHEYE_MAX_REGION_NUM      4
#define FISHEYE_MAX_OFFSET          127

typedef enum fyPIC_SIZE_E
{
    PIC_QCIF = 0,
    PIC_CIF,
    PIC_2CIF,
    PIC_HD1,
    PIC_D1,
    PIC_960H,
    PIC_1280H,
    PIC_1440H,

    PIC_QVGA,    /* 320 * 240 */
    PIC_VGA,     /* 640 * 480 */
    PIC_XGA,     /* 1024 * 768 */
    PIC_SXGA,    /* 1400 * 1050 */
    PIC_UXGA,    /* 1600 * 1200 */
    PIC_QXGA,    /* 2048 * 1536 */

    PIC_WVGA,    /* 854 * 480 */
    PIC_WSXGA,   /* 1680 * 1050 */
    PIC_WUXGA,   /* 1920 * 1200 */
    PIC_WQXGA,   /* 2560 * 1600 */

    PIC_HD720,   /* 1280 * 720 */
    PIC_HD1080,  /* 1920 * 1080 */
    PIC_qHD,     /*960 * 540*/

    PIC_UHD2K,   /* 2560 * 1440 */
    PIC_UHD4K,   /* 3840*2160 */

	PIC_1080LITE,/* 960*1080 */

    PIC_BUTT
}PIC_SIZE_E;

typedef enum fyVIDEO_NORM_E
{
    VIDEO_ENCODING_MODE_PAL=0,
    VIDEO_ENCODING_MODE_NTSC,
    VIDEO_ENCODING_MODE_AUTO,
    VIDEO_ENCODING_MODE_BUTT
} VIDEO_NORM_E;

typedef enum fyVIDEO_CONTROL_MODE_E
{
    VIDEO_CONTROL_MODE_SLAVER=0,
    VIDEO_CONTROL_MODE_MASTER,
    VIDEO_CONTROL_MODE_BUTT
}VIDEO_CONTROL_MODE_E;

typedef enum fyPIXEL_FORMAT_E
{
    PIXEL_FORMAT_RGB_1BPP = 0,
    PIXEL_FORMAT_RGB_2BPP,
    PIXEL_FORMAT_RGB_4BPP,
    PIXEL_FORMAT_RGB_8BPP,
    PIXEL_FORMAT_RGB_444,

    PIXEL_FORMAT_RGB_4444,
    PIXEL_FORMAT_RGB_555,
    PIXEL_FORMAT_RGB_565,
    PIXEL_FORMAT_RGB_1555,

    /*  9 reserved */
    PIXEL_FORMAT_RGB_888,
    PIXEL_FORMAT_RGB_8888,

    PIXEL_FORMAT_RGB_PLANAR_888,
    PIXEL_FORMAT_RGB_BAYER_8BPP,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    PIXEL_FORMAT_RGB_BAYER_14BPP,

    PIXEL_FORMAT_RGB_BAYER,         /* 16 bpp */

    PIXEL_FORMAT_YUV_A422,
    PIXEL_FORMAT_YUV_A444,

    PIXEL_FORMAT_YUV_PLANAR_422,
    PIXEL_FORMAT_YUV_PLANAR_420,

    PIXEL_FORMAT_YUV_PLANAR_444,

    PIXEL_FORMAT_YUV_SEMIPLANAR_422,    // 22
    PIXEL_FORMAT_YUV_SEMIPLANAR_420,    // 23
    PIXEL_FORMAT_YUV_SEMIPLANAR_444,

    PIXEL_FORMAT_UYVY_PACKAGE_422,
    PIXEL_FORMAT_YUYV_PACKAGE_422,
    PIXEL_FORMAT_VYUY_PACKAGE_422,
    PIXEL_FORMAT_YCbCr_PLANAR,

    PIXEL_FORMAT_YUV_400,

    /******FY add begin********/
    PIXEL_FORMAT_YUV_422_10BPP=43,
    PIXEL_FORMAT_YUV_420_BLOCK,     //<yuv420 block for venc

    PIXEL_FORMAT_YUV_422_MIRROR,    //only support 1D compress mode.

    PIXEL_FORMAT_YUV_FIELD_422,

    PIXEL_FORMAT_YVU_SEMIPLANAR_420,	// NV21

    /******FY add end*********/

    PIXEL_FORMAT_BUTT
} fyPIXEL_FORMAT_E;

typedef struct fyVIDEO_VBI_INFO_S
{
    FY_U32 au32Data[32];
    FY_U32 u32Len;
}VIDEO_VBI_INFO_S;

typedef enum fyVIDEO_FIELD_E
{
    VIDEO_FIELD_TOP         = 0x1,    /* even field */
    VIDEO_FIELD_BOTTOM      = 0x2,    /* odd field */
    VIDEO_FIELD_INTERLACED  = 0x3,    /* two interlaced fields */
    VIDEO_FIELD_FRAME       = 0x4,    /* frame */

    VIDEO_FIELD_BUTT
} VIDEO_FIELD_E;

typedef enum fyVIDEO_FORMAT_E
{
    VIDEO_FORMAT_LINEAR     = 0x0,      /* nature video line */
    VIDEO_FORMAT_TILE       = 0x1,      /* tile cell: 256pixel x 16line, default tile mode */
    VIDEO_FORMAT_TILE64     = 0x2,      /* tile cell: 64pixel x 16line */
    VIDEO_FORMAT_BLOCK      = 0x3,      /* macro block */
    VIDEO_FORMAT_BUTT
} VIDEO_FORMAT_E;

typedef enum fyCOMPRESS_MODE_E
{
    COMPRESS_MODE_NONE      = 0x0,      /* no compress */
    COMPRESS_MODE_SEG       = 0x1,      /* compress unit is 256 bytes as a segment, default seg mode */
    COMPRESS_MODE_SEG128    = 0x2,      /* compress unit is 128 bytes as a segment */
    COMPRESS_MODE_LINE      = 0x3,      /* compress unit is the whole line */
    COMPRESS_MODE_FRAME     = 0x4,      /* compress unit is the whole frame */

    /*FY add, compress type 1D,match VI:VI_EC_LEVEL_E*/
    COMPRESS_MODE_128byte   = 0x5,  /*compress rate 50%*/
    COMPRESS_MODE_160byte   = 0x6,  /*compress rate 63%*/
    COMPRESS_MODE_192byte   = 0x7,  /*compress rate 75%*/

    /*FY add, compress type TILE,*/
    COMPRESS_MODE_TILE_192  = 0x8,

    /*FY add, compress type SLICE,*/
    COMPRESS_MODE_SLICE     = 0x9,

    COMPRESS_MODE_CF10      = 0xa,
    COMPRESS_MODE_FBC       = 0xb,

    /*FY add, compress type TILE,*/
    COMPRESS_MODE_TILE_224  = 0xc,
    COMPRESS_MODE_TILE_256  = 0xd,

    COMPRESS_MODE_MFBC      = 0xe,

    COMPRESS_MODE_BUTT
} COMPRESS_MODE_E;

typedef enum fyVIDEO_DISPLAY_MODE_E
{
    VIDEO_DISPLAY_MODE_PREVIEW      = 0x0,
    VIDEO_DISPLAY_MODE_PLAYBACK     = 0x1,

    VIDEO_DISPLAY_MODE_BUTT
} VIDEO_DISPLAY_MODE_E;

typedef enum fyFRAME_FLASH_TYPE_E
{
    FRAME_FLASH_OFF  = 0,
    FRAME_FLASH_ON   = 1,
    FRAME_FLASH_BUTT,
}FRAME_FLASH_TYPE_E;
typedef enum fyFISHEYE_MOUNT_MODE_E
{
    FISHEYE_DESKTOP_MOUNT   = 0,        /* desktop mount mode */
    FISHEYE_CEILING_MOUNT   = 1,        /* ceiling mount mode */
    FISHEYE_WALL_MOUNT      = 2,        /* wall mount mode */
    FISHEYE_MOUNT_MODE_BUTT
}FISHEYE_MOUNT_MODE_E;
typedef enum fyFISHEYE_VIEW_MODE_E
{
    FISHEYE_VIEW_360_PANORAMA   = 0,    /* 360 panorama mode of fisheye correction */
    FISHEYE_VIEW_180_PANORAMA   = 1,    /* 180 panorama mode of fisheye correction */
    FISHEYE_VIEW_NORMAL         = 2,    /* normal mode of fisheye correction */
    FISHEYE_NO_TRANSFORMATION   = 3,    /* no fisheye correction */
    FISHEYE_VIEW_MODE_BUTT
}FISHEYE_VIEW_MODE_E;
typedef enum fyFISHEYE_FILTER_MODE_E
{
    FISHEYE_FILTER_BILINEAR     = 0,    /* bilinear filter */
    FISHEYE_FILTER_LINEAR       = 1,    /* linear filter */
    FISHEYE_FILTER_NEAREST      = 2,    /* nearest filter */
    FISHEYE_FILTER_MODE_BUTT
}FISHEYE_FILTER_MODE_E;

typedef struct fyVIDEO_SUPPLEMENT_S
{
    FY_U32   u32JpegDcfPhyAddr;
    FY_VOID *pJpegDcfVirAddr;

    FRAME_FLASH_TYPE_E enFlashType;
}VIDEO_SUPPLEMENT_S;

typedef struct fyVIDEO_FRAME_S
{
    FY_U32          u32Width;
    FY_U32          u32Height;
    VIDEO_FIELD_E   u32Field;
    fyPIXEL_FORMAT_E  enPixelFormat;

    VIDEO_FORMAT_E  enVideoFormat;
    COMPRESS_MODE_E enCompressMode;

    FY_U32          u32PhyAddr[3];
    FY_VOID         *pVirAddr[3];
    FY_U32          u32Stride[3];

    FY_U32          u32HeaderPhyAddr[3];
    FY_VOID         *pHeaderVirAddr[3];
    FY_U32          u32HeaderStride[3];

    FY_S16          s16OffsetTop;       /* top offset of show area */
    FY_S16          s16OffsetBottom;    /* bottom offset of show area */
    FY_S16          s16OffsetLeft;      /* left offset of show area */
    FY_S16          s16OffsetRight;     /* right offset of show area */

    FY_U64          u64pts;
    FY_U32          u32TimeRef;

    FY_U32          u32PrivateData;
    VIDEO_SUPPLEMENT_S stSupplement;
}VIDEO_FRAME_S;

typedef struct fyVIDEO_FRAME_INFO_S
{
    VIDEO_FRAME_S stVFrame;
    FY_U32 u32PoolId;
} VIDEO_FRAME_INFO_S;

/* VI Mix-Capture info. */
typedef struct fyVI_MIXCAP_STAT_S
{
    FY_BOOL bMixCapMode;    /* In mix-capture mode or not. */
    FY_BOOL bHasDownScale;  /* VI Frame is downscaled or not. */
} VI_MIXCAP_STAT_S;

/* VI output frame info. */
typedef struct fyVI_FRAME_INFO_S
{
    VI_MIXCAP_STAT_S stMixCapState; /* VI Mix-Capture info. */
    VIDEO_FRAME_INFO_S stViFrmInfo; /* Video frame info. */
    FY_BOOL bFlashed;               /* Flashed Video frame or not. */
}VI_FRAME_INFO_S;


typedef struct fyBITMAP_S
{
    fyPIXEL_FORMAT_E enPixelFormat;  /* Bitmap's pixel format */
    FY_U32 u32Width;               /* Bitmap's width */
    FY_U32 u32Height;              /* Bitmap's height */
    FY_VOID *pData;                /* Address of Bitmap's data */
} BITMAP_S;


typedef struct fyFISHEYE_GPU_PRI_S
{
    FISHEYE_FILTER_MODE_E   enYFilter;      /* Fiter mode for Luma */
    FISHEYE_FILTER_MODE_E   enCbCrFilter;   /* Fiter mode for chroma */
    FY_U32                  u32CCMPhyAddr;  /* Physical address of correction coordinate memory , the size is region's width x height x sizeof(float) x 2. */
}FISHEYE_GPU_PRI_S;


typedef struct fyFISHEYE_REGION_ATTR_S
{
    FISHEYE_VIEW_MODE_E     enViewMode;     /* fisheye view mode */
    FY_U32                  u32InRadius;    /* inner radius of fisheye correction region [0, u32OutRadius) */
    FY_U32                  u32OutRadius;   /* out radius of fisheye correction region [1, max(width/2 of input picture, height/2 of input picture)] */
    FY_U32                  u32Pan;         /* [0, 360] */
    FY_U32                  u32Tilt;        /* [0, 360] */
    FY_U32                  u32HorZoom;     /* [1, 4095] */
    FY_U32                  u32VerZoom;     /* [1, 4095] */
    RECT_S                  stOutRect;      /* output image info after fisheye correction range of width [960, 4608],
                                               rang of height [360, 3456], rang of x [0, 4608), rang of y [0, 3456) */
   FISHEYE_GPU_PRI_S        stGPUPrivate;   /* GPU related attribute. Only for GPU use */
}FISHEYE_REGION_ATTR_S;


typedef struct fyFISHEYE_ATTR_S
{
    FY_BOOL                 bEnable;            /* whether enable fisheye correction or not */
    FY_BOOL                 bLMF;               /* whether fisheye len's LMF coefficient is from user config
                                                        or from default linear config */
    FY_BOOL                 bBgColor;           /* whether use background color or not */
    FY_U32                  u32BgColor;         /* the background color ARGB8888 */

    FY_S32                  s32HorOffset;       /* the horizontal offset between image center and physical center of len */
    FY_S32                  s32VerOffset;       /* the vertical offset between image center and physical center of len */

    FY_U32                  u32TrapezoidCoef;   /* strength coefficient of trapezoid correction */

    FISHEYE_MOUNT_MODE_E    enMountMode;        /* fisheye mount mode */

    FY_U32                  u32RegionNum;       /* fisheye correction region number */
    FISHEYE_REGION_ATTR_S   astFisheyeRegionAttr[FISHEYE_MAX_REGION_NUM];/* attribution of fisheye correction region */
}FISHEYE_ATTR_S;


typedef struct fyFISHEYE_CONFIG_S
{
    FY_U16                  au16LMFCoef[128];   /* LMF coefficient of fisheye len */
}FISHEYE_CONFIG_S;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* _FY_COMM_VIDEO_H_ */

