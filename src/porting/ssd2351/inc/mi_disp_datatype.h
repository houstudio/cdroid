/* Sgs trade secret */
/* Copyright (c) [2019~2025] Sgs Technology Ltd.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
Sgs and be kept in strict confidence
(Sgs Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of Sgs Confidential
Information is unlawful and strictly prohibited. Sgs hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#ifndef _MI_DISP_DATATYPE_H_
#define _MI_DISP_DATATYPE_H_
#include "mi_sys_datatype.h"


#define MI_DEF_DISP_ERR(err)       MI_DEF_ERR(E_MI_MODULE_ID_DISP, E_MI_ERR_LEVEL_ERROR, err)
#define MI_ERR_DISP_INVALID_DEVID  MI_DEF_DISP_ERR(E_MI_ERR_INVALID_DEVID)
#define MI_ERR_DISP_INVALID_CHNID  MI_DEF_DISP_ERR(E_MI_ERR_INVALID_CHNID)
#define MI_ERR_DISP_ILLEGAL_PARAM  MI_DEF_DISP_ERR(E_MI_ERR_ILLEGAL_PARAM)
#define MI_ERR_DISP_CHN_EXIST      MI_DEF_DISP_ERR(E_MI_ERR_EXIST)
#define MI_ERR_DISP_CHN_UNEXIST    MI_DEF_DISP_ERR(E_MI_ERR_UNEXIST)
#define MI_ERR_DISP_NULL_PTR       MI_DEF_DISP_ERR(E_MI_ERR_NULL_PTR)
#define MI_ERR_DISP_NOT_CONFIG     MI_DEF_DISP_ERR(E_MI_ERR_NOT_CONFIG)
#define MI_ERR_DISP_NOT_SUPPORT    MI_DEF_DISP_ERR(E_MI_ERR_NOT_SUPPORT)
#define MI_ERR_DISP_NOT_PERMIT     MI_DEF_DISP_ERR(E_MI_ERR_NOT_PERM)
#define MI_ERR_DISP_NO_MEM         MI_DEF_DISP_ERR(E_MI_ERR_NOMEM)
#define MI_ERR_DISP_NOBUF          MI_DEF_DISP_ERR(E_MI_ERR_NOBUF)
#define MI_ERR_DISP_BUF_EMPTY      MI_DEF_DISP_ERR(E_MI_ERR_BUF_EMPTY)
#define MI_ERR_DISP_BUF_FULL       MI_DEF_DISP_ERR(E_MI_ERR_BUF_FULL)
#define MI_ERR_DISP_SYS_NOTREADY   MI_DEF_DISP_ERR(E_MI_ERR_SYS_NOTREADY)
#define MI_ERR_DISP_BADADDR        MI_DEF_DISP_ERR(E_MI_ERR_BADADDR)
#define MI_ERR_DISP_BUSY           MI_DEF_DISP_ERR(E_MI_ERR_BUSY)
#define MI_ERR_DISP_CHN_NOT_START  MI_DEF_DISP_ERR(E_MI_ERR_CHN_NOT_STARTED)
#define MI_ERR_DISP_CHN_NOT_STOP   MI_DEF_DISP_ERR(E_MI_ERR_CHN_NOT_STOPED)
#define MI_ERR_DISP_NOT_INIT       MI_DEF_DISP_ERR(E_MI_ERR_NOT_INIT)
#define MI_ERR_DISP_INITED         MI_DEF_DISP_ERR(E_MI_ERR_INITED)
#define MI_ERR_DISP_NOT_ENABLE     MI_DEF_DISP_ERR(E_MI_ERR_NOT_ENABLE)
#define MI_ERR_DISP_NOT_DISABLE    MI_DEF_DISP_ERR(E_MI_ERR_NOT_DISABLE)
#define MI_ERR_DISP_SYS_TIMEOUT    MI_DEF_DISP_ERR(E_MI_ERR_SYS_TIMEOUT)
#define MI_ERR_DISP_NOT_STARTED    MI_DEF_DISP_ERR(E_MI_ERR_DEV_NOT_STARTED)
#define MI_ERR_DISP_NOT_STOPED     MI_DEF_DISP_ERR(E_MI_ERR_DEV_NOT_STOPED)
#define MI_ERR_DISP_CHN_NO_CONTENT MI_DEF_DISP_ERR(E_MI_ERR_CHN_NO_CONTENT)
#define MI_ERR_DISP_NOVASPACE      MI_DEF_DISP_ERR(E_MI_ERR_NOVASPACE)
#define MI_ERR_DISP_NOITEM         MI_DEF_DISP_ERR(E_MI_ERR_NOITEM)
#define MI_ERR_DISP_FAILED         MI_DEF_DISP_ERR(E_MI_ERR_FAILED)

#define MI_ERR_DISP_DEV_NOT_CONFIG               MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_DEV_NOT_CONFIG)
#define MI_ERR_DISP_DEV_NOT_ENABLE               MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_DEV_NOT_ENABLE)
#define MI_ERR_DISP_DEV_ALREADY_ENABLED          MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_ALREADY_ENABLED)
#define MI_ERR_DISP_DEV_ALREADY_BOUND            MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_ALREADY_BOUND)
#define MI_ERR_DISP_DEV_NOT_BIND                 MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_NOT_BIND)
#define MI_ERR_DISP_LAYER_NOT_ENABLE             MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_LAYER_NOT_ENABLE)
#define MI_ERR_DISP_LAYER_NOT_DISABLE            MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_LAYER_NOT_DISABLE)
#define MI_ERR_DISP_LAYER_NOT_CONFIG             MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_LAYER_NOT_CONFIG)
#define MI_ERR_DISP_INPUTPORT_NOT_DISABLE        MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_INPUTPORT_NOT_DISABLE)
#define MI_ERR_DISP_INPUTPORT_NOT_ENABLE         MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_INPUTPORT_NOT_ENABLE)
#define MI_ERR_DISP_INPUTPORT_NOT_CONFIG         MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_INPUTPORT_NOT_CONFIG)
#define MI_ERR_DISP_INPUTPORT_NOT_ALLOC          MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_INPUTPORT_NOT_ALLOC)
#define MI_ERR_DISP_INVALID_PATTERN              MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_INVALID_PATTERN)
#define MI_ERR_DISP_INVALID_POSITION             MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_INVALID_POSITION)
#define MI_ERR_DISP_WAIT_TIMEOUT                 MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_WAIT_TIMEOUT)
#define MI_ERR_DISP_INVALID_VIDEO_FRAME          MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_INVALID_VIDEO_FRAME)
#define MI_ERR_DISP_INVALID_RECT_PARA            MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_INVALID_RECT_PARA)
#define MI_ERR_DISP_INPUTPORT_SHOW_AREA_OVERLAP  MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_INPUTPORT_SHOW_AREA_OVERLAP)
#define MI_ERR_DISP_INVALID_LAYERID              MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_INVALID_LAYERID)
#define MI_ERR_DISP_LAYER_ALREADY_BOUND          MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_LAYER_ALREADY_BOUND)
#define MI_ERR_DISP_LAYER_NOT_BIND               MI_DEF_DISP_ERR(E_MI_DISP_ERR_CODE_LAYER_NOT_BIND)

typedef MI_U32 MI_DISP_INPUTPORT;
typedef enum
{
    E_MI_DISP_ERR_CODE_DEV_NOT_CONFIG = 0x40,
    E_MI_DISP_ERR_CODE_DEV_NOT_ENABLE,
    E_MI_DISP_ERR_CODE_ALREADY_ENABLED,
    E_MI_DISP_ERR_CODE_ALREADY_BOUND,
    E_MI_DISP_ERR_CODE_NOT_BIND,
    E_MI_DISP_ERR_CODE_LAYER_NOT_ENABLE,
    E_MI_DISP_ERR_CODE_LAYER_NOT_DISABLE,
    E_MI_DISP_ERR_CODE_LAYER_NOT_CONFIG,
    E_MI_DISP_ERR_CODE_INPUTPORT_NOT_DISABLE,
    E_MI_DISP_ERR_CODE_INPUTPORT_NOT_ENABLE,
    E_MI_DISP_ERR_CODE_INPUTPORT_NOT_CONFIG,
    E_MI_DISP_ERR_CODE_INPUTPORT_NOT_ALLOC,
    E_MI_DISP_ERR_CODE_INVALID_PATTERN,
    E_MI_DISP_ERR_CODE_INVALID_POSITION,
    E_MI_DISP_ERR_CODE_WAIT_TIMEOUT,
    E_MI_DISP_ERR_CODE_INVALID_VIDEO_FRAME,
    E_MI_DISP_ERR_CODE_INVALID_RECT_PARA,
    E_MI_DISP_ERR_CODE_INPUTPORT_SHOW_AREA_OVERLAP, // The input port show area overlap.
    E_MI_DISP_ERR_CODE_INVALID_LAYERID,
    E_MI_DISP_ERR_CODE_LAYER_ALREADY_BOUND,
    E_MI_DISP_ERR_CODE_LAYER_NOT_BIND,
    E_MI_DISP_ERR_CODE_MAX
} MI_DISP_ErrCode_e;

typedef enum
{
    E_MI_DISP_INTF_CVBS = 0,
    E_MI_DISP_INTF_YPBPR,
    E_MI_DISP_INTF_VGA,
    E_MI_DISP_INTF_BT656,
    E_MI_DISP_INTF_BT1120,
    E_MI_DISP_INTF_HDMI,
    E_MI_DISP_INTF_LCD,
    E_MI_DISP_INTF_BT656_H,
    E_MI_DISP_INTF_BT656_L,
    E_MI_DISP_INTF_TTL,
    E_MI_DISP_INTF_MIPIDSI,
    E_MI_DISP_INTF_TTL_SPI_IF,
    E_MI_DISP_INTF_SRGB,
    E_MI_DISP_INTF_MCU,
    E_MI_DISP_INTF_MCU_NOFLM,
    E_MI_DISP_INTF_BT601,
    E_MI_DISP_INTF_BT1120_DDR,
    E_MI_DISP_INTF_LVDS,
    E_MI_DISP_INTF_LVDS1,
    E_MI_DISP_INTF_DUAL_LVDS,
    E_MI_DISP_INTF_MIPIDSI1,
    E_MI_DISP_INTF_MAX,
} MI_DISP_Interface_e;

typedef enum
{
    E_MI_DISP_OUTPUT_PAL = 0,
    E_MI_DISP_OUTPUT_NTSC,
    E_MI_DISP_OUTPUT_960H_PAL,  /* ITU-R BT.1302 960 x 576 at 50 Hz (interlaced)*/
    E_MI_DISP_OUTPUT_960H_NTSC, /* ITU-R BT.1302 960 x 480 at 60 Hz (interlaced)*/

    E_MI_DISP_OUTPUT_480i60,
    E_MI_DISP_OUTPUT_576i50,
    E_MI_DISP_OUTPUT_480P60,
    E_MI_DISP_OUTPUT_576P50,
    E_MI_DISP_OUTPUT_720P50,
    E_MI_DISP_OUTPUT_720P60,
    E_MI_DISP_OUTPUT_1080P24,
    E_MI_DISP_OUTPUT_1080P25,
    E_MI_DISP_OUTPUT_1080P30,
    E_MI_DISP_OUTPUT_1080I50,
    E_MI_DISP_OUTPUT_1080I60,
    E_MI_DISP_OUTPUT_1080P50,
    E_MI_DISP_OUTPUT_1080P60,

    E_MI_DISP_OUTPUT_640x480_60,   /* VESA 640 x 480 at 60 Hz (non-interlaced) CVT */
    E_MI_DISP_OUTPUT_800x600_60,   /* VESA 800 x 600 at 60 Hz (non-interlaced) */
    E_MI_DISP_OUTPUT_1024x768_60,  /* VESA 1024 x 768 at 60 Hz (non-interlaced) */
    E_MI_DISP_OUTPUT_1280x1024_60, /* VESA 1280 x 1024 at 60 Hz (non-interlaced) */
    E_MI_DISP_OUTPUT_1366x768_60,  /* VESA 1366 x 768 at 60 Hz (non-interlaced) */
    E_MI_DISP_OUTPUT_1440x900_60,  /* VESA 1440 x 900 at 60 Hz (non-interlaced) CVT Compliant */
    E_MI_DISP_OUTPUT_1280x800_60,  /* 1280*800@60Hz VGA@60Hz*/
    E_MI_DISP_OUTPUT_1680x1050_60, /* VESA 1680 x 1050 at 60 Hz (non-interlaced) */
    E_MI_DISP_OUTPUT_1920x2160_30, /* 1920x2160_30 */
    E_MI_DISP_OUTPUT_1600x1200_60, /* VESA 1600 x 1200 at 60 Hz (non-interlaced) */
    E_MI_DISP_OUTPUT_1920x1200_60, /* VESA 1920 x 1600 at 60 Hz (non-interlaced) CVT (Reduced Blanking)*/

    E_MI_DISP_OUTPUT_2560x1440_30,   /* 2560x1440_30 */
    E_MI_DISP_OUTPUT_2560x1440_50,   /* 2560x1440_50 */
    E_MI_DISP_OUTPUT_2560x1440_60,   /* 2560x1440_60 */
    E_MI_DISP_OUTPUT_2560x1600_60,   /* 2560x1600_60 */
    E_MI_DISP_OUTPUT_3840x2160_25,   /* 3840x2160_25 */
    E_MI_DISP_OUTPUT_3840x2160_30,   /* 3840x2160_30 */
    E_MI_DISP_OUTPUT_3840x2160_60,   /* 3840x2160_60 */
    E_MI_DISP_OUTPUT_1920x1080_5994, /* 1920x1080_59.94 */
    E_MI_DISP_OUTPUT_1920x1080_2997, /* 1920x1080_29.97 */
    E_MI_DISP_OUTPUT_1280x720_5994,  /* 1280X720_59.94 */
    E_MI_DISP_OUTPUT_1280x720_2997,  /* 1280x720_29.97 */
    E_MI_DISP_OUTPUT_3840x2160_2997, /* 3840x2160_29.97 */
    E_MI_DISP_OUTPUT_720P24,         /* 1280x720_24 */
    E_MI_DISP_OUTPUT_720P25,         /* 1280x720_25 */
    E_MI_DISP_OUTPUT_720P30,         /* 1280x720_30 */
    E_MI_DISP_OUTPUT_1920x1080_2398, /* 1920x1080_23.98*/
    E_MI_DISP_OUTPUt_3840x2160_24,   /* 3840x2160_24 */
    E_MI_DISP_OUTPUT_848x480_60,     /* 848x480_60 */
    E_MI_DISP_OUTPUT_1280x768_60,    /* 1280x768_60 */
    E_MI_DISP_OUTPUT_1280x960_60,    /* 1280x960_60 */
    E_MI_DISP_OUTPUT_1360x768_60,    /* 1360x768_60 */
    E_MI_DISP_OUTPUT_1400x1050_60,   /* 1400x1050_60 */
    E_MI_DISP_OUTPUT_1600x900_60,    /* 1600x900_60 */
    E_MI_DISP_OUTPUT_1920x1440_60,    /* 1920x1440_60 */

    E_MI_DISP_OUTPUT_USER,
    E_MI_DISP_OUTPUT_720I25,        /*only for bt1120 and bt656*/
    E_MI_DISP_OUTPUT_720I30,
    E_MI_DISP_OUTPUT_720I50,        /*only for bt1120 and bt656*/
    E_MI_DISP_OUTPUT_720I60,        /*only for bt1120 and bt656*/
    E_MI_DISP_OUTPUT_1080I25,       /*only for bt1120 and bt656*/
    E_MI_DISP_OUTPUT_1080I30,
    E_MI_DISP_OUTPUT_MAX,
} MI_DISP_OutputTiming_e;

typedef enum
{
    E_MI_DISP_ROTATE_NONE,
    E_MI_DISP_ROTATE_90,
    E_MI_DISP_ROTATE_180,
    E_MI_DISP_ROTATE_270,
    E_MI_DISP_ROTATE_NUM,
} MI_DISP_RotateMode_e;

typedef enum
{
    E_MI_DISP_CSC_MATRIX_BYPASS = 0, /* do not change color space */

    E_MI_DISP_CSC_MATRIX_BT601_TO_BT709, /* change color space from BT.601 to BT.709 */
    E_MI_DISP_CSC_MATRIX_BT709_TO_BT601, /* change color space from BT.709 to BT.601 */

    E_MI_DISP_CSC_MATRIX_BT601_TO_RGB_PC, /* change color space from BT.601 to RGB */
    E_MI_DISP_CSC_MATRIX_BT709_TO_RGB_PC, /* change color space from BT.709 to RGB */

    E_MI_DISP_CSC_MATRIX_RGB_TO_BT601_PC, /* change color space from RGB to BT.601 */
    E_MI_DISP_CSC_MATRIX_RGB_TO_BT709_PC, /* change color space from RGB to BT.709 */
    E_MI_DISP_CSC_MATRIX_USER, /* Change color space from PQbin */

    E_MI_DISP_CSC_MATRIX_NUM
} MI_DISP_CscMatrix_e;

typedef enum
{
    E_MI_DISP_SYNC_MODE_INVALID = 0,
    E_MI_DISP_SYNC_MODE_CHECK_PTS,
    E_MI_DISP_SYNC_MODE_FREE_RUN,
    E_MI_DISP_SYNC_MODE_NUM,
} MI_DISP_SyncMode_e;

typedef enum
{
    E_MI_LAYER_INPUTPORT_STATUS_INVALID = 0,
    E_MI_LAYER_INPUTPORT_STATUS_PAUSE,
    E_MI_LAYER_INPUTPORT_STATUS_RESUME,
    E_MI_LAYER_INPUTPORT_STATUS_STEP,
    E_MI_LAYER_INPUTPORT_STATUS_REFRESH,
    E_MI_LAYER_INPUTPORT_STATUS_SHOW,
    E_MI_LAYER_INPUTPORT_STATUS_HIDE,
    E_MI_LAYER_INPUTPORT_STATUS_NUM,
} MI_DISP_InputPortStatus_e;

typedef enum
{
    MI_DISP_WBC_SOURCE_DEV,   // video+UI
    MI_DISP_WBC_SOURCE_VIDEO, // video only
} MI_DISP_WBC_SourceType_e;

typedef struct MI_DISP_WBC_Source_s
{
    MI_DISP_WBC_SourceType_e eSourceType;
    MI_U32                   u32SourceId;
} MI_DISP_WBC_Source_t;

typedef struct MI_DISP_WBC_TargetSize_s
{
    MI_U32 u32Width;
    MI_U32 u32Height;
} MI_DISP_WBC_TargetSize_t;

typedef struct MI_DISP_WBC_Attr_s
{
    MI_DISP_WBC_TargetSize_t stTargetSize;
    MI_SYS_PixelFormat_e     ePixFormat;
    MI_SYS_CompressMode_e eCompressMode;
} MI_DISP_WBC_Attr_t;

typedef struct MI_DISP_SyncInfo_s
{
    MI_BOOL bSynm;   /* sync mode(0:timing,as BT.656; 1:signal,as LCD) */
    MI_BOOL bIop;    /* interlaced or progressive display(0:i; 1:p) */
    MI_U8   u8Intfb; /* interlace bit width while output */

    MI_U16 u16VStart; /* vertical de start */
    MI_U16 u16Vact;   /* vertical active area */
    MI_U16 u16Vbb;    /* vertical back blank porch */
    MI_U16 u16Vfb;    /* vertical front blank porch */

    MI_U16 u16HStart; /* herizontal de start */
    MI_U16 u16Hact;   /* herizontal active area */
    MI_U16 u16Hbb;    /* herizontal back blank porch */
    MI_U16 u16Hfb;    /* herizontal front blank porch */
    MI_U16 u16Hmid;   /* bottom herizontal active area */

    MI_U16 u16Bvact; /* bottom vertical active area */
    MI_U16 u16Bvbb;  /* bottom vertical back blank porch */
    MI_U16 u16Bvfb;  /* bottom vertical front blank porch */

    MI_U16 u16Hpw; /* horizontal pulse width */
    MI_U16 u16Vpw; /* vertical pulse width */

    MI_BOOL bIdv; /* inverse data valid of output */
    MI_BOOL bIhs; /* inverse horizontal synch signal */
    MI_BOOL bIvs; /* inverse vertical synch signal */
    MI_U32  u32FrameRate;
} MI_DISP_SyncInfo_t;

typedef struct MI_DISP_PubAttr_s
{
    MI_U32                 u32BgColor; /* Background color of a device, in RGB format. */
    MI_DISP_Interface_e    eIntfType;  /* Type of a out interface */
    MI_DISP_OutputTiming_e eIntfSync;  /* Type of a out interface timing */
    MI_DISP_SyncInfo_t     stSyncInfo; /* Information about out interface timings */
} MI_DISP_PubAttr_t;

typedef struct MI_DISP_CompressAttr_s
{
    MI_BOOL bSupportCompress; /* Whether to support compress */
} MI_DISP_CompressAttr_t;

typedef struct MI_DISP_VidWin_Rect_s
{
    MI_U16 u16X;
    MI_U16 u16Y;
    MI_U16 u16Width;
    MI_U16 u16Height;
} MI_DISP_VidWinRect_t;

typedef struct MI_DISP_VideoLayerSize_s
{
    MI_U32 u16Width;
    MI_U32 u16Height;
} MI_DISP_VideoLayerSize_t;

typedef struct MI_DISP_VideoLayerAttr_s
{
    MI_DISP_VidWinRect_t     stVidLayerDispWin; /* Display resolution */
    MI_DISP_VideoLayerSize_t stVidLayerSize;    /* Canvas size of the video layer */
    MI_SYS_PixelFormat_e     ePixFormat;        /* Pixel format of the video layer */
} MI_DISP_VideoLayerAttr_t;

typedef struct MI_DISP_Csc_s
{
    MI_DISP_CscMatrix_e eCscMatrix;
    MI_U32               u32Luma;       /* luminance:   0 ~ 100 default: 50 */
    MI_U32               u32Contrast;   /* contrast :   0 ~ 100 default: 50 */
    MI_U32               u32Hue;        /* hue      :   0 ~ 100 default: 50 */
    MI_U32               u32Saturation; /* saturation:  0 ~ 100 default: 40 */
} MI_DISP_Csc_t;

/* General Operation of InputPort */
typedef struct MI_DISP_InputPortAttr_s
{
    MI_DISP_VidWinRect_t stDispWin; /* rect of video out chn */
    MI_U16               u16SrcWidth;
    MI_U16               u16SrcHeight;
    MI_SYS_CompressMode_e eDecompressMode;
} MI_DISP_InputPortAttr_t;

typedef struct MI_DISP_Position_s
{
    MI_U16 u16X;
    MI_U16 u16Y;
} MI_DISP_Position_t;

typedef struct MI_DISP_QueryChannelStatus_s
{
    MI_BOOL                   bEnable;
    MI_DISP_InputPortStatus_e eStatus;
} MI_DISP_QueryChannelStatus_t;

typedef struct MI_DISP_VgaParam_s
{
    MI_DISP_Csc_t stCsc;   /* color space */
    MI_U32        u32Gain; /* current gain of VGA signals. [0, 64). default:0x30 */
    MI_U32        u32Sharpness;
} MI_DISP_VgaParam_t;

typedef struct MI_DISP_HdmiParam_s
{
    MI_DISP_Csc_t stCsc;   /* color space */
    MI_U32        u32Gain; /* current gain of HDMI signals. [0, 64). default:0x30 */
    MI_U32        u32Sharpness;
} MI_DISP_HdmiParam_t;

typedef struct MI_DISP_LcdParam_s
{
    MI_DISP_Csc_t stCsc; /* color space */
    MI_U32        u32Sharpness;
} MI_DISP_LcdParam_t;

typedef struct MI_DISP_CvbsParam_s
{
    MI_DISP_Csc_t stCsc;                     /* color space */
    MI_U32 u32Sharpness;
} MI_DISP_CvbsParam_t;

typedef struct MI_DISP_RotateConfig_s
{
    MI_DISP_RotateMode_e eRotateMode;
} MI_DISP_RotateConfig_t;

typedef struct
{
    MI_U16 u16RedOffset;
    MI_U16 u16GreenOffset;
    MI_U16 u16BlueOffset;

    MI_U16 u16RedColor;   // 00~FF, 0x80 is no change
    MI_U16 u16GreenColor; // 00~FF, 0x80 is no change
    MI_U16 u16BlueColor;  // 00~FF, 0x80 is no change
} MI_DISP_ColorTemperature_t;

typedef struct MI_DISP_GammaParam_s
{
    MI_BOOL bEn;
    MI_U16  u16EntryNum;
    union
    {
        MI_U8 * pu8ColorR;
        MI_PTR64    p64ColorR;
    };
    union
    {
        MI_U8 * pu8ColorG;
        MI_PTR64    p64ColorG;
    };
    union
    {
        MI_U8 * pu8ColorB;
        MI_PTR64    p64ColorB;
    };
} MI_DISP_GammaParam_t;

typedef struct MI_DISP_InitParam_s
{
    MI_U32 u32DevId;
    MI_U8 *u8Data;
} MI_DISP_InitParam_t;

typedef enum
{
    E_MI_DISP_MIPIDSI_PACKET_TYPE_DCS = 0,
    E_MI_DISP_MIPIDSI_PACKET_TYPE_GENERIC = 1,
    E_MI_DISP_MIPIDSI_PACKET_TYPE_MAX
} MI_DISP_MipiDsiPacketType_e;

typedef struct MI_DISP_PowerConfig_s
{
    MI_BOOL bEnable;
} MI_DISP_PowerConfig_t;

typedef struct MI_DISP_WriteMipiDsiCmd_s
{
    MI_DISP_MipiDsiPacketType_e ePacketType;
    MI_U32 u32CmdBufSize;
    MI_PTR64 p64CmdBuf;
} MI_DISP_WriteMipiDsiCmd_t;

typedef struct MI_DISP_ReadMipiDsiCmd_s
{
    MI_U8  u8RegAddr;
    MI_U32 u32CmdBufSize;
    MI_PTR64 p64CmdBuf;
} MI_DISP_ReadMipiDsiCmd_t;

#ifdef __cplusplus
extern "C"
{
#endif
#ifdef __cplusplus
}
#endif

#endif ///_MI_DISP_DATATYPE_H_
