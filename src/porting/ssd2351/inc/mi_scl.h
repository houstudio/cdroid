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
#ifndef _MI_SCL_H_
#define _MI_SCL_H_

#include "mi_scl_datatype.h"

#define SCL_MAJOR_VERSION   3
#define SCL_SUB_VERSION     0
#define MACRO_TO_STR(macro) #macro
#define SCL_VERSION_STR(major_version, sub_version)                                                                 \
    (                                                                                                               \
        {                                                                                                           \
            char *tmp =                                                                                             \
                sub_version / 100  ? "mi_scl_version_" MACRO_TO_STR(major_version) "." MACRO_TO_STR(sub_version)    \
                : sub_version / 10 ? "mi_scl_version_" MACRO_TO_STR(major_version) ".0" MACRO_TO_STR(sub_version)   \
                                   : "mi_scl_version_" MACRO_TO_STR(major_version) ".00" MACRO_TO_STR(sub_version); \
            tmp;                                                                                                    \
        })
#define MI_SCL_API_VERSION VPE_VERSION_STR(SCL_MAJOR_VERSION, SCL_SUB_VERSION)

#ifdef __cplusplus
extern "C"
{
#endif

    MI_S32 MI_SCL_CreateDevice(MI_SCL_DEV DevId, MI_SCL_DevAttr_t *pstSclDevAttr);
    MI_S32 MI_SCL_DestroyDevice(MI_SCL_DEV DevId);

    MI_S32 MI_SCL_CreateChannel(MI_SCL_DEV DevId, MI_SCL_CHANNEL ChnId, MI_SCL_ChannelAttr_t *pstChnAttr);
    MI_S32 MI_SCL_DestroyChannel(MI_SCL_DEV DevId, MI_SCL_CHANNEL ChnId);

    MI_S32 MI_SCL_SetChnParam(MI_SCL_DEV DevId, MI_SCL_CHANNEL ChnId, MI_SCL_ChnParam_t *pstChnParam);
    MI_S32 MI_SCL_GetChnParam(MI_SCL_DEV DevId, MI_SCL_CHANNEL ChnId, MI_SCL_ChnParam_t *pstChnParam);

    MI_S32 MI_SCL_SetInputPortCrop(MI_SCL_DEV DevId, MI_SCL_CHANNEL ChnId, MI_SYS_WindowRect_t *pstCropInfo);
    MI_S32 MI_SCL_GetInputPortCrop(MI_SCL_DEV DevId, MI_SCL_CHANNEL ChnId, MI_SYS_WindowRect_t *pstCropInfo);

    MI_S32 MI_SCL_StartChannel(MI_SCL_DEV DevId, MI_SCL_CHANNEL ChnId);
    MI_S32 MI_SCL_StopChannel(MI_SCL_DEV DevId, MI_SCL_CHANNEL ChnId);

    MI_S32 MI_SCL_SetOutputPortParam(MI_SCL_DEV DevId, MI_SCL_CHANNEL ChnId, MI_SCL_PORT PortId,
                                     MI_SCL_OutPortParam_t *pstOutPortParam);
    MI_S32 MI_SCL_GetOutputPortParam(MI_SCL_DEV DevId, MI_SCL_CHANNEL ChnId, MI_SCL_PORT PortId,
                                     MI_SCL_OutPortParam_t *pstOutPortParam);

    MI_S32 MI_SCL_EnableOutputPort(MI_SCL_DEV DevId, MI_SCL_CHANNEL ChnId, MI_SCL_PORT PortId);
    MI_S32 MI_SCL_DisableOutputPort(MI_SCL_DEV DevId, MI_SCL_CHANNEL ChnId, MI_SCL_PORT PortId);

    MI_S32 MI_SCL_StretchBuf(MI_SCL_DirectBuf_t *pstSrcBuf, MI_SYS_WindowRect_t *pstSrcCrop,
                             MI_SCL_DirectBuf_t *pstDstBuf, MI_SCL_FilterType_e eFilterType);
    MI_S32 MI_SCL_StretchBufOsd(MI_SCL_DirectBuf_t *pstSrcBuf, MI_SYS_WindowRect_t *pstSrcCrop,
                                MI_SCL_DirectBuf_t *pstDstBuf, MI_SCL_DirectOsdBuf_t *pstDstOsdBuf,
                                MI_SCL_FilterType_e eFilterType);
#ifdef __cplusplus
}
#endif

#endif
