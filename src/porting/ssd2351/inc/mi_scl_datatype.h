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
#ifndef _MI_SCL_DATATYPE_H_
#define _MI_SCL_DATATYPE_H_
#include "mi_sys_datatype.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        E_MI_ERR_SCL_INVALID_DEVID,
        E_MI_ERR_SCL_INVALID_CHNID,
        E_MI_ERR_SCL_INVALID_PORTID,
        E_MI_ERR_SCL_ILLEGAL_PARAM,
        E_MI_ERR_SCL_EXIST,
        E_MI_ERR_SCL_UNEXIST,
        E_MI_ERR_SCL_NULL_PTR,
        E_MI_ERR_SCL_NOT_SUPPORT,
        E_MI_ERR_SCL_NOT_PERM,
        E_MI_ERR_SCL_NOMEM,
        E_MI_ERR_SCL_NOBUF,
        E_MI_ERR_SCL_BUF_EMPTY,
        E_MI_ERR_SCL_NOTREADY,
        E_MI_ERR_SCL_BUSY,
    } MI_SCL_ErrCode_e;

#define MI_SCL_OK (0)
#define MI_ERR_SCL_INVALID_DEVID \
    MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_INVALID_DEVID) // 0XA0222000
#define MI_ERR_SCL_INVALID_CHNID \
    MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_INVALID_CHNID) // 0XA0222001
#define MI_ERR_SCL_INVALID_PORTID MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_INVALID_PORTID)
#define MI_ERR_SCL_ILLEGAL_PARAM  MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_ILLEGAL_PARAM)
#define MI_ERR_SCL_EXIST          MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_EXIST)
#define MI_ERR_SCL_UNEXIST        MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_UNEXIST)
#define MI_ERR_SCL_NULL_PTR       MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_NULL_PTR)
#define MI_ERR_SCL_NOT_SUPPORT    MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_NOT_SUPPORT)
#define MI_ERR_SCL_NOT_PERM       MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_NOT_PERM)
#define MI_ERR_SCL_NOMEM          MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_NOMEM)
#define MI_ERR_SCL_NOBUF          MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_NOBUF)
#define MI_ERR_SCL_BUF_EMPTY      MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_BUF_EMPTY)
#define MI_ERR_SCL_NOTREADY       MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_NOTREADY)
#define MI_ERR_SCL_BUSY           MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_BUSY)

#define MI_SCL_DIRECT_OSD_CNT_MAX (8)

#define MI_SCL_DEV_ISP_REALTIME0 (0x0)
#define MI_SCL_DEV_RDMA0         (0x1)
#define MI_SCL_DEV_VIF_REALTIME0 (0x2)
#define MI_SCL_DEV_DIPR0         (0x3)
#define MI_SCL_DEV_ISP_REALTIME1 (0x4)
#define MI_SCL_DEV_RDMA2         (0x5)
#define MI_SCL_DEV_VIF_REALTIME1 (0x6)
#define MI_SCL_DEV_DIPR1         (0x7)
#define MI_SCL_DEV_HVP_REALTIME0 (0x8)

#define MI_SCL_MULTI_DEV_MASK                (0x8000)
#define MI_SCL_CREATE_MULTI_DEV(masterDevId) (MI_SCL_MULTI_DEV_MASK | 1 << masterDevId)

    typedef MI_U32 MI_SCL_DEV;
    typedef MI_U32 MI_SCL_CHANNEL;
    typedef MI_U32 MI_SCL_PORT;

    typedef enum
    {
        E_MI_SCL_HWSCLID_INVALID = 0,
        E_MI_SCL_HWSCL0          = 0x0001,
        E_MI_SCL_HWSCL1          = 0x0002,
        E_MI_SCL_HWSCL2          = 0x0004,
        E_MI_SCL_HWSCL3          = 0x0008,
        E_MI_SCL_HWSCL4          = 0x0010,
        E_MI_SCL_HWSCL5          = 0x0020,
        E_MI_SCL_HWSCL6          = 0x0040,
        E_MI_SCL_HWSCL7          = 0x0080,
        E_MI_SCL_HWSCL8          = 0x0100,
        E_MI_SCL_HWSCL_MAX       = 0xffff,
    } MI_SCL_HWSclId_e;

    typedef struct MI_SCL_DevAttr_s
    {
        MI_U32 u32NeedUseHWOutPortMask;
    } MI_SCL_DevAttr_t;

    typedef struct MI_SCL_ChannelAttr_s
    {
        MI_U32 u32Reserved;
    } MI_SCL_ChannelAttr_t;

    typedef struct MI_SCL_ChnParam_s
    {
        MI_SYS_Rotate_e eRot;
    } MI_SCL_ChnParam_t;

    typedef struct MI_SCL_OutPortParam_s
    {
        MI_SYS_WindowRect_t   stSCLOutCropRect;
        MI_SYS_WindowSize_t   stSCLOutputSize;
        MI_BOOL               bMirror;
        MI_BOOL               bFlip;
        MI_SYS_PixelFormat_e  ePixelFormat;
        MI_SYS_CompressMode_e eCompressMode;
    } MI_SCL_OutPortParam_t;

    typedef enum
    {
        E_MI_SCL_FILTER_TYPE_AUTO,
        E_MI_SCL_FILTER_TYPE_BYPASS,
        E_MI_SCL_FILTER_TYPE_BILINEAR,
        E_MI_SCL_FILTER_TYPE_BICUBIC,
        E_MI_SCL_FILTER_TYPE_SELFDEFINED,
        E_MI_SCL_FILTER_TYPE_MAX,
    } MI_SCL_FilterType_e;

    typedef struct MI_SCL_DirectBuf_s
    {
        MI_SYS_PixelFormat_e ePixelFormat;
        MI_U32               u32Width;
        MI_U32               u32Height;
        MI_PHY               phyAddr[2];
        MI_U32               u32Stride[2];
        MI_U32               u32BuffSize;
    } MI_SCL_DirectBuf_t;

    typedef struct MI_SCL_DirectBufOsdArgb1555Alpha_s
    {
        MI_U8 u8BgAlpha;
        MI_U8 u8FgAlpha;
    } MI_SCL_DirectOsdArgb1555Alpha_t;

    typedef union
    {
        MI_SCL_DirectOsdArgb1555Alpha_t stArgb1555Alpha;
        MI_U8                           u8ConstantAlpha;
    } MI_SCL_DirectOsdAlphaModePara_u; /*NOLINT*/

    typedef enum
    {
        E_MI_SCL_DIRECT_BUF_OSD_PIXEL_ALPHA = 0,
        E_MI_SCL_DIRECT_BUF_OSD_CONSTANT_ALPHT
    } MI_SCL_DirectOsdAlphaMode_e;

    typedef struct MI_SCL_DirectOsdAlphaAttr_s
    {
        MI_SCL_DirectOsdAlphaMode_e     eAlphaMode;
        MI_SCL_DirectOsdAlphaModePara_u stAlphaPara;
    } MI_SCL_DirectOsdAlphaAttr_t;

    typedef struct MI_SCL_DirectOsdInfo_s
    {
        MI_U32                      u32X;
        MI_U32                      u32Y;
        MI_U32                      u32Width;
        MI_U32                      u32Height;
        MI_PHY                      phyAddr;
        MI_U32                      u32Stride;
        MI_SCL_DirectOsdAlphaAttr_t stOsdAlphaAttr;
    } MI_SCL_DirectOsdInfo_t;

    typedef struct MI_SCL_DirectOsdBuf_s
    {
        MI_U8                  u8OsdBufCnt;
        MI_SYS_PixelFormat_e   ePixelFormat;
        MI_SCL_DirectOsdInfo_t astOsdBuf[MI_SCL_DIRECT_OSD_CNT_MAX];
    } MI_SCL_DirectOsdBuf_t;
#ifdef __cplusplus
}
#endif
#endif
