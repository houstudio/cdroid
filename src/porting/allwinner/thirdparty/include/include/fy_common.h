#ifndef __FY_COMMON_H__
#define __FY_COMMON_H__

#include "fy_type.h"
#include "fy_math.h"
#include "fy_defines.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#ifndef VER_X
    #define VER_X 1
#endif

#ifndef VER_Y
    #define VER_Y 0
#endif

#ifndef VER_Z
    #define VER_Z 0
#endif

#ifndef VER_P
    #define VER_P 0
#endif

#ifndef VER_B
    #define VER_B 0
#endif

#ifdef FY_DEBUG
    #define VER_D " Debug"
#else
    #define VER_D " Release"
#endif

#define __DEFINE_TO_STR(s)   #s
#define DEFINE_TO_STR(s)     __DEFINE_TO_STR(s)

#define __MK_VERSION(x,y,z,p,b) #x"."#y"."#z"."#p" B"#b
#define MK_VERSION(x,y,z,p,b) __MK_VERSION(x,y,z,p,b)
#define MPP_VERSION  MPP_VER_PRIX MK_VERSION(VER_X,VER_Y,VER_Z,VER_P,VER_B) VER_D

#define BUILD_VERSION DEFINE_TO_STR(VER_B)

#define FYMODULE_VERSION(name, version) \
			"[" name "] Version:[v" version " (" BUILD_VERSION ")] Build Time:[" __DATE__ "-" __TIME__ "]"


#define VERSION_NAME_MAXLEN 64
typedef struct fyMPP_VERSION_S
{
	FY_CHAR aVersion[VERSION_NAME_MAXLEN];
}MPP_VERSION_S;


typedef struct
{
    FY_U32 base;
    void * vbase;
    FY_U32 size;
}MEM_INFO_S;

typedef struct fyPOINT_S
{
    FY_S32 s32X;
    FY_S32 s32Y;
}POINT_S;

typedef struct fySIZE_S
{
    FY_U32 u32Width;
    FY_U32 u32Height;
} SIZE_S;

typedef struct fyRECT_S
{
    FY_S32 s32X;
    FY_S32 s32Y;
    FY_U32 u32Width;
    FY_U32 u32Height;
}RECT_S;

typedef struct fyCROP_INFO_S
{
	FY_BOOL bEnable;
	RECT_S  stRect;
}CROP_INFO_S;

typedef enum fyROTATE_E
{
    ROTATE_NONE = 0,               /* no rotate */
    ROTATE_90   = 1,               /* 90 degrees clockwise */
    ROTATE_180  = 2,               /* 180 degrees clockwise */
    ROTATE_270  = 3,               /* 270 degrees clockwise */
    ROTATE_BUTT
} ROTATE_E;

typedef struct fyBORDER_S
{
    FY_U32 u32TopWidth;            /* top border weight, in pixel*/
    FY_U32 u32BottomWidth;         /* bottom border weight, in pixel*/
    FY_U32 u32LeftWidth;           /* left border weight, in pixel*/
    FY_U32 u32RightWidth;          /* right border weight, in pixel*/
    FY_U32 u32Color;               /* border color, RGB888*/
} BORDER_S;

typedef enum fyASPECT_RATIO_E
{
    ASPECT_RATIO_NONE   = 0,        /* full screen */
    ASPECT_RATIO_AUTO   = 1,        /* ratio no change, 1:1*/
    ASPECT_RATIO_MANUAL = 2,        /* ratio manual set */
    ASPECT_RATIO_BUTT
}ASPECT_RATIO_E;

typedef struct fyASPECT_RATIO_S
{
    ASPECT_RATIO_E enMode;          /* aspect ratio mode: none/auto/manual */
    FY_U32         u32BgColor;      /* background color, RGB 888 */
    RECT_S         stVideoRect;     /* valid in ASPECT_RATIO_MANUAL mode */
} ASPECT_RATIO_S;


typedef struct fyALPHA_S
{
    FY_BOOL bAlphaEnable;   /**<  alpha enable flag */
    FY_BOOL bAlphaChannel;  /**<  alpha channel enable flag */
    FY_U8 u8Alpha0;         /**<  alpha0 value, used in ARGB1555 */
    FY_U8 u8Alpha1;         /**<  alpha1 value, used in ARGB1555 */
    FY_U8 u8GlobalAlpha;    /**<  global alpha value */
    FY_U8 u8Reserved;
} ALPHA_S;

typedef FY_S32 AI_CHN;
typedef FY_S32 AO_CHN;
typedef FY_S32 AENC_CHN;
typedef FY_S32 ADEC_CHN;
typedef FY_S32 AUDIO_DEV;
typedef FY_S32 AVENC_CHN;
typedef FY_S32 VI_DEV;
typedef FY_S32 VI_WAY;
typedef FY_S32 VI_CHN;
typedef FY_S32 VO_DEV;
typedef FY_S32 VO_LAYER;
typedef FY_S32 VO_CHN;
typedef FY_S32 VO_WBC;
typedef FY_S32 GRAPHIC_LAYER;
typedef FY_S32 VENC_CHN;
typedef FY_S32 VDEC_CHN;
typedef FY_S32 VENC_GRP;
typedef FY_S32 VO_GRP;
typedef FY_S32 VDA_CHN;
typedef FY_S32 IVE_HANDLE;
typedef FY_S32 CLS_HANDLE;
typedef FY_S32 FD_CHN;
typedef FY_S32 MD_CHN;
typedef FY_S32 VPPU_CHN;
typedef FY_S32 VPPU_PTH;


#define FY_INVALID_CHN (-1)
#define FY_INVALID_WAY (-1)
#define FY_INVALID_LAYER (-1)
#define FY_INVALID_DEV (-1)
#define FY_INVALID_HANDLE (-1)
#define FY_INVALID_VALUE (-1)
#define FY_INVALID_TYPE (-1)

typedef enum fyMOD_ID_E
{
    FY_ID_CMPI    = 0,
    FY_ID_VB      = 1,
    FY_ID_SYS     = 2,
    FY_ID_RGN	  = 3,
    FY_ID_CHNL    = 4,
    FY_ID_VDEC    = 5,
    FY_ID_GROUP   = 6,
    FY_ID_VPSS    = 7,
    FY_ID_VENC    = 8,
    FY_ID_VDA     = 9,
    FY_ID_H264E   = 10,
    FY_ID_JPEGED  = 11,
    FY_ID_MPEG4E  = 12,
    FY_ID_H264D   = 13,
    FY_ID_JPEGD   = 14,
    FY_ID_VOU     = 15,
    FY_ID_VIU     = 16,
    FY_ID_DSU     = 17,
    FY_ID_VALG    = 18,
    FY_ID_RC      = 19,
    FY_ID_AIO     = 20,
    FY_ID_AI      = 21,
    FY_ID_AO      = 22,
    FY_ID_AENC    = 23,
    FY_ID_ADEC    = 24,
    FY_ID_AVENC   = 25,
    FY_ID_PCIV    = 26,
    FY_ID_PCIVFMW = 27,
    FY_ID_ISP	  = 28,
    FY_ID_IVE	  = 29,

    FY_ID_DCCM    = 31,
    FY_ID_DCCS    = 32,
    FY_ID_PROC    = 33,
    FY_ID_LOG     = 34,
    FY_ID_MST_LOG = 35,
    FY_ID_VD      = 36,

    FY_ID_VCMP    = 38,
    FY_ID_FB      = 39,
    FY_ID_HDMI    = 40,
    FY_ID_VOIE    = 41,
    FY_ID_TDE     = 42,
    FY_ID_USR     = 43,
    FY_ID_VEDU    = 44,
    FY_ID_VGS     = 45,
    FY_ID_H265E   = 46,
	FY_ID_FD	  = 47,
	FY_ID_ODT	  = 48, //Object Detection and Tracing
	FY_ID_VQA	  = 49, //Video Quality Analysis
	FY_ID_LPR 	  = 50, //License Plate Recognition
	FY_ID_FISHEYE = 51,
	FY_ID_VPPU    = 52,
//fh module
	FY_ID_BGM 	 = 60,
	FY_ID_BGMX 	 = 61,
	FY_ID_BGMSW  = 62,
    FY_ID_BUTT,
    //for following only for driver log
    FY_ID_VIU_LLD,
    FY_ID_VOU_LLD,
    FY_ID_VPPU_LLD,
    FY_ID_VPU_LLD,
    FY_ID_JPEG_LLD,
    FY_ID_LOG_BUTT,
} MOD_ID_E;

typedef struct fyMPP_CHN_S
{
    MOD_ID_E    enModId;
    FY_S32      s32DevId;
    FY_S32      s32ChnId;
} MPP_CHN_S;

#define MPP_MOD_VIU       "vi"
#define MPP_MOD_VOU       "vo"
#define MPP_MOD_HDMI      "hdmi"
#define MPP_MOD_DSU       "dsu"
#define MPP_MOD_VGS       "vgs"

#define MPP_MOD_CHNL      "chnl"
#define MPP_MOD_VENC      "venc"
#define MPP_MOD_GRP       "grp"
#define MPP_MOD_VDA       "vda"
#define MPP_MOD_VPSS      "vpss"
#define MPP_MOD_RGN       "rgn"
#define MPP_MOD_IVE       "ive"
#define MPP_MOD_FD        "fd"
#define MPP_MOD_MD		  "md"

#define MPP_MOD_H264E     "h264e"
#define MPP_MOD_H265E     "h265e"
#define MPP_MOD_JPEGE     "jpege"
#define MPP_MOD_MPEG4E    "mpeg4e"

#define MPP_MOD_VDEC      "vdec"
#define MPP_MOD_H264D     "h264d"
#define MPP_MOD_JPEGD     "jpegd"

#define MPP_MOD_AI        "ai"
#define MPP_MOD_AO        "ao"
#define MPP_MOD_AENC      "aenc"
#define MPP_MOD_ADEC      "adec"
#define MPP_MOD_AIO       "aio"
#define MPP_MOD_ACODEC	  "acodec"


#define MPP_MOD_VB        "vb"
#define MPP_MOD_SYS       "sys"
#define MPP_MOD_CMPI      "cmpi"

#define MPP_MOD_PCIV      "pciv"
#define MPP_MOD_PCIVFMW   "pcivfmw"

#define MPP_MOD_PROC      "proc"
#define MPP_MOD_LOG       "logfy"
#define MPP_MOD_MST_LOG   "mstlogfy"

#define MPP_MOD_DCCM      "dccm"
#define MPP_MOD_DCCS      "dccs"

#define MPP_MOD_VCMP      "vcmp"
#define MPP_MOD_FB        "fb"

#define MPP_MOD_RC        "rc"

#define MPP_MOD_VOIE      "voie"

#define MPP_MOD_TDE       "tde"
#define MPP_MOD_ISP       "isp"
#define MPP_MOD_USR       "usr"

/* We just coyp this value of payload type from RTP/RTSP definition */
typedef enum
{
    PT_PCMU          = 0,
    PT_1016          = 1,
    PT_G721          = 2,
    PT_GSM           = 3,
    PT_G723          = 4,
    PT_DVI4_8K       = 5,
    PT_DVI4_16K      = 6,
    PT_LPC           = 7,
    PT_PCMA          = 8,
    PT_G722          = 9,
    PT_S16BE_STEREO  = 10,
    PT_S16BE_MONO    = 11,
    PT_QCELP         = 12,
    PT_CN            = 13,
    PT_MPEGAUDIO     = 14,
    PT_G728          = 15,
    PT_DVI4_3        = 16,
    PT_DVI4_4        = 17,
    PT_G729          = 18,
    PT_G711A         = 19,
    PT_G711U         = 20,
    PT_G726          = 21,
    PT_G726_32K      = 22,
    PT_G729A         = 23,
    PT_LPCM          = 24,
    PT_CelB          = 25,
    PT_JPEG          = 26,
    PT_CUSM          = 27,
    PT_NV            = 28,
    PT_PICW          = 29,
    PT_CPV           = 30,
    PT_H261          = 31,
    PT_MPEGVIDEO     = 32,
    PT_MPEG2TS       = 33,
    PT_H263          = 34,
    PT_SPEG          = 35,
    PT_MPEG2VIDEO    = 36,
    PT_AAC           = 37,
    PT_WMA9STD       = 38,
    PT_HEAAC         = 39,
    PT_PCM_VOICE     = 40,
    PT_PCM_AUDIO     = 41,
    PT_AACLC         = 42,
    PT_MP3           = 43,
    PT_ADPCMA        = 49,
    PT_AEC           = 50,
    PT_X_LD          = 95,
    PT_H264          = 96,
    PT_D_GSM_HR      = 200,
    PT_D_GSM_EFR     = 201,
    PT_D_L8          = 202,
    PT_D_RED         = 203,
    PT_D_VDVI        = 204,
    PT_D_BT656       = 220,
    PT_D_H263_1998   = 221,
    PT_D_MP1S        = 222,
    PT_D_MP2P        = 223,
    PT_D_BMPEG       = 224,
    PT_MP4VIDEO      = 230,
    PT_MP4AUDIO      = 237,
    PT_VC1           = 238,
    PT_JVC_ASF       = 255,
    PT_D_AVI         = 256,
    PT_DIVX3		 = 257,
    PT_AVS			 = 258,
    PT_REAL8		 = 259,
    PT_REAL9		 = 260,
    PT_VP6			 = 261,
    PT_VP6F			 = 262,
    PT_VP6A			 = 263,
    PT_SORENSON	 	 = 264,
    PT_H265          = 265,
    PT_VP8           = 266,
    PT_MVC           = 267,
    PT_MAX           = 268,
    /* add by fysilicon */
    PT_AMR           = 1001,
    PT_MJPEG         = 1002,
    PT_AMRWB         = 1003,
    PT_BUTT
}PAYLOAD_TYPE_E;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif  /* _FY_COMMON_H_ */

