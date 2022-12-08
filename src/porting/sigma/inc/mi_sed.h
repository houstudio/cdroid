/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#ifndef _MI_SED_H_
#define _MI_SED_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "mi_sed_datatype.h"

#define SED_MAJOR_VERSION 2
#define SED_SUB_VERSION 1
#define MACRO_TO_STR(macro) #macro
#define SED_VERSION_STR(major_version,sub_version) ({char *tmp = sub_version/100 ? \
                                    "mi_sed_version_" MACRO_TO_STR(major_version)"." MACRO_TO_STR(sub_version) : sub_version/10 ? \
                                    "mi_sed_version_" MACRO_TO_STR(major_version)".0" MACRO_TO_STR(sub_version) : \
                                    "mi_sed_version_" MACRO_TO_STR(major_version)".00" MACRO_TO_STR(sub_version);tmp;})
#define MI_SED_API_VERSION SED_VERSION_STR(SED_MAJOR_VERSION, SED_SUB_VERSION)

MI_S32 MI_SED_CreateChn(MI_SED_CHN SedChn, MI_SED_DetectorAttr_t* pstAttr);
MI_S32 MI_SED_DestroyChn(MI_SED_CHN SedChn);
MI_S32 MI_SED_StartDetector(MI_SED_CHN SedChn);
MI_S32 MI_SED_StopDetector(MI_SED_CHN SedChn);
MI_S32 MI_SED_AttachToVencChn(MI_SED_CHN SedChn, MI_SED_TARGET_CHN TargetChn);
MI_S32 MI_SED_DetachFromVencChn(MI_SED_CHN SedChn, MI_SED_TARGET_CHN TargetChn);
MI_S32 MI_SED_GetRect(MI_SED_CHN SedChn, MI_SED_RectInfo_t *pstRectInfo);


#ifdef __cplusplus
}
#endif

#endif
