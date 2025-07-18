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
#ifndef _MI_VENC_H_
#define _MI_VENC_H_

#include "mi_venc_datatype.h"

#define VENC_MAJOR_VERSION  3
#define VENC_SUB_VERSION    11
#define MACRO_TO_STR(macro) #macro
#define VENC_VERSION_STR(major_version, sub_version)                                                                 \
    (                                                                                                                \
        {                                                                                                            \
            char *tmp =                                                                                              \
                sub_version / 100  ? "mi_venc_version_" MACRO_TO_STR(major_version) "." MACRO_TO_STR(sub_version)    \
                : sub_version / 10 ? "mi_venc_version_" MACRO_TO_STR(major_version) ".0" MACRO_TO_STR(sub_version)   \
                                   : "mi_venc_version_" MACRO_TO_STR(major_version) ".00" MACRO_TO_STR(sub_version); \
            tmp;                                                                                                     \
        })
#define MI_VENC_API_VERSION VENC_VERSION_STR(VENC_MAJOR_VERSION, VENC_SUB_VERSION)

#ifdef __cplusplus
extern "C"
{
#endif
    MI_S32 MI_VENC_CreateDev(MI_VENC_DEV VeDev, MI_VENC_InitParam_t *pstInitParam);
    MI_S32 MI_VENC_DestroyDev(MI_VENC_DEV VeDev);
    MI_S32 MI_VENC_CreateChn(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ChnAttr_t *pstAttr);
    MI_S32 MI_VENC_DestroyChn(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn);
    MI_S32 MI_VENC_ResetChn(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn);
    MI_S32 MI_VENC_SetOutputPortParam(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_U8 PortId,
                                     MI_VENC_OutPortParam_t *pstOutPortParam);
    MI_S32 MI_VENC_GetOutputPortParam(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_U8 PortId,
                                     MI_VENC_OutPortParam_t *pstOutPortParam);
    MI_S32 MI_VENC_EnableOutputPort(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_U8 PortId);
    MI_S32 MI_VENC_DisableOutputPort(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_U8 PortId);

    MI_S32 MI_VENC_StartRecvPic(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn);
    MI_S32 MI_VENC_StartRecvPicEx(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_RecvPicParam_t *pstRecvParam);
    MI_S32 MI_VENC_StopRecvPic(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn);
    MI_S32 MI_VENC_Query(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ChnStat_t *pstStat);
    MI_S32 MI_VENC_SetChnAttr(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ChnAttr_t *pstAttr);
    MI_S32 MI_VENC_GetChnAttr(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ChnAttr_t *pstAttr);
    MI_S32 MI_VENC_GetStream(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_Stream_t *pstStream, MI_S32 s32MilliSec);
    MI_S32 MI_VENC_ReleaseStream(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_Stream_t *pstStream);
    MI_S32 MI_VENC_InsertUserData(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_U8 *pu8Data, MI_U32 u32Len);
    MI_S32 MI_VENC_SetMaxStreamCnt(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_U32 u32MaxStrmCnt);
    MI_S32 MI_VENC_GetMaxStreamCnt(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_U32 *pu32MaxStrmCnt);
    MI_S32 MI_VENC_RequestIdr(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_BOOL bInstant);
    MI_S32 MI_VENC_GetFd(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn);
    MI_S32 MI_VENC_CloseFd(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn);
    MI_S32 MI_VENC_SetRoiCfg(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_RoiCfg_t *pstVencRoiCfg);
    MI_S32 MI_VENC_GetRoiCfg(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_U32 u32Index, MI_VENC_RoiCfg_t *pstVencRoiCfg);
    MI_S32 MI_VENC_SetRoiBgFrameRate(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_RoiBgFrameRate_t *pstRoiBgFrmRate);
    MI_S32 MI_VENC_GetRoiBgFrameRate(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_RoiBgFrameRate_t *pstRoiBgFrmRate);
    MI_S32 MI_VENC_SetH264SliceSplit(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn,
                                     MI_VENC_ParamH264SliceSplit_t *pstSliceSplit);
    MI_S32 MI_VENC_GetH264SliceSplit(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn,
                                     MI_VENC_ParamH264SliceSplit_t *pstSliceSplit);
    MI_S32 MI_VENC_SetH264Trans(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamH264Trans_t *pstH264Trans);
    MI_S32 MI_VENC_GetH264Trans(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamH264Trans_t *pstH264Trans);
    MI_S32 MI_VENC_SetH264Entropy(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamH264Entropy_t *pstH264EntropyEnc);
    MI_S32 MI_VENC_GetH264Entropy(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamH264Entropy_t *pstH264EntropyEnc);
    MI_S32 MI_VENC_SetH264Dblk(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamH264Dblk_t *pstH264Dblk);
    MI_S32 MI_VENC_GetH264Dblk(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamH264Dblk_t *pstH264Dblk);
    MI_S32 MI_VENC_SetH264Vui(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamH264Vui_t *pstH264Vui);
    MI_S32 MI_VENC_GetH264Vui(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamH264Vui_t *pstH264Vui);
    MI_S32 MI_VENC_SetH265SliceSplit(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn,
                                     MI_VENC_ParamH265SliceSplit_t *pstSliceSplit);
    MI_S32 MI_VENC_GetH265SliceSplit(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn,
                                     MI_VENC_ParamH265SliceSplit_t *pstSliceSplit);
    MI_S32 MI_VENC_SetH265Trans(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamH265Trans_t *pstH265Trans);
    MI_S32 MI_VENC_GetH265Trans(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamH265Trans_t *pstH265Trans);
    MI_S32 MI_VENC_SetH265Dblk(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamH265Dblk_t *pstH265Dblk);
    MI_S32 MI_VENC_GetH265Dblk(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamH265Dblk_t *pstH265Dblk);
    MI_S32 MI_VENC_SetH265Vui(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamH265Vui_t *pstH265Vui);
    MI_S32 MI_VENC_GetH265Vui(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamH265Vui_t *pstH265Vui);
    MI_S32 MI_VENC_SetJpegParam(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamJpeg_t *pstJpegParam);
    MI_S32 MI_VENC_GetJpegParam(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamJpeg_t *pstJpegParam);
    MI_S32 MI_VENC_SetRcParam(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_RcParam_t *pstRcParam);
    MI_S32 MI_VENC_GetRcParam(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_RcParam_t *pstRcParam);
    MI_S32 MI_VENC_SetRefParam(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamRef_t *pstRefParam);
    MI_S32 MI_VENC_GetRefParam(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamRef_t *pstRefParam);
    MI_S32 MI_VENC_SetCrop(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_CropCfg_t *pstCropCfg);
    MI_S32 MI_VENC_GetCrop(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_CropCfg_t *pstCropCfg);
    MI_S32 MI_VENC_SetFrameLostStrategy(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn,
                                        MI_VENC_ParamFrameLost_t *pstFrmLostParam);
    MI_S32 MI_VENC_GetFrameLostStrategy(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn,
                                        MI_VENC_ParamFrameLost_t *pstFrmLostParam);
    MI_S32 MI_VENC_SetSuperFrameCfg(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_SuperFrameCfg_t *pstSuperFrmParam);
    MI_S32 MI_VENC_GetSuperFrameCfg(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_SuperFrameCfg_t *pstSuperFrmParam);
    MI_S32 MI_VENC_SetRcPriority(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_RcPriority_e *peRcPriority);
    MI_S32 MI_VENC_GetRcPriority(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_RcPriority_e *peRcPriority);
    MI_S32 MI_VENC_DupChn(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn);
    MI_S32 MI_VENC_SetInputSourceConfig(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn,
                                        MI_VENC_InputSourceConfig_t *pstInputSourceConfig);
    MI_S32 MI_VENC_AllocCustomMap(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_PHY *pPhyAddr, void **ppCpuAddr);
    MI_S32 MI_VENC_ApplyCustomMap(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_PHY PhyAddr);
    MI_S32 MI_VENC_GetLastHistoStaticInfo(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn,
                                          MI_VENC_FrameHistoStaticInfo_t **ppFrmHistoStaticInfo);
    MI_S32 MI_VENC_ReleaseHistoStaticInfo(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn);
    MI_S32 MI_VENC_SetAdvCustRcAttr(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_AdvCustRcAttr_t *pstAdvCustRcAttr);
    MI_S32 MI_VENC_SetIntraRefresh(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_IntraRefresh_t *pstIntraRefresh);
    MI_S32 MI_VENC_GetIntraRefresh(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_IntraRefresh_t *pstIntraRefresh);
    MI_S32 MI_VENC_SetDeBreathCfg(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_DeBreathCfg_t *pstDeBreathCfg);
    MI_S32 MI_VENC_GetDeBreathCfg(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_DeBreathCfg_t *pstDeBreathCfg);
    MI_S32 MI_VENC_SetAv1TileSplit(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamAv1TileSplit_t *pstTileSplit);
    MI_S32 MI_VENC_GetAv1TileSplit(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamAv1TileSplit_t *pstTileSplit);
    MI_S32 MI_VENC_SetAv1Dblk(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamAv1Dblk_t *pstAv1Dblk);
    MI_S32 MI_VENC_GetAv1Dblk(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamAv1Dblk_t *pstAv1Dblk);
    MI_S32 MI_VENC_SetAv1Vui(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamAv1Vui_t *pstAv1Vui);
    MI_S32 MI_VENC_GetAv1Vui(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_ParamAv1Vui_t *pstAv1Vui);
#ifdef __cplusplus
}
#endif

#endif
