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
/* SigmaStar 3A extension api */
#ifndef ISP_SIGMA3A_EXT_H
#define ISP_SIGMA3A_EXT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "mi_vpe_datatype.h"

typedef struct
{
    MI_U32 u32HeadSize;    //Size of MIIspApiHeader_t
    MI_U32 u32DataLen;     //Data length;
    MI_U32 u32CtrlID;      //Function ID
    MI_U32 u32Channel;     //Isp channel number
    MI_U32 u32Dir;     //Command direction, 1: Set, 2: Get
    MI_S32 s32Ret;         //Isp api retuen value
} MI_CUS3A_IspApiHeader_t;

typedef struct
{
    MI_CUS3A_IspApiHeader_t stHeader;
    MI_U8 u8Data[0];
}MI_CUS3A_IspApiCmd_t;

typedef struct
{
    MI_U32 u32Dir;  // 0: Set, 1: Get
    //MI_VPE_IspApiData_t stApiData;
    MI_VPE_IspApiHeader_t stApiHeader;
    void* pData;
}MI_CUS3A_CtrlCmd_t;

MI_S32 Cus3A_GetIspApiData(MI_VPE_IspApiHeader_t *pCmd, void *pstData);
MI_S32 Cus3A_SetIspApiData(MI_VPE_IspApiHeader_t *pCmd, void *pstData);

#ifdef __cplusplus
}
#endif

#endif
