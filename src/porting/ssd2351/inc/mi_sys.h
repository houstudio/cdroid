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
#ifndef _MI_SYS_H_
#define _MI_SYS_H_

#include "mi_common.h"
#include "mi_sys_datatype.h"

#define SYS_MAJOR_VERSION   3
#define SYS_SUB_VERSION 14
#define MACRO_TO_STR(macro) #macro
#define SYS_VERSION_STR(major_version, sub_version)                                                                 \
    (                                                                                                               \
        {                                                                                                           \
            char *tmp =                                                                                             \
                sub_version / 100  ? "mi_sys_version_" MACRO_TO_STR(major_version) "." MACRO_TO_STR(sub_version)    \
                : sub_version / 10 ? "mi_sys_version_" MACRO_TO_STR(major_version) ".0" MACRO_TO_STR(sub_version)   \
                                   : "mi_sys_version_" MACRO_TO_STR(major_version) ".00" MACRO_TO_STR(sub_version); \
            tmp;                                                                                                    \
        })
#define MI_SYS_API_VERSION SYS_VERSION_STR(SYS_MAJOR_VERSION, SYS_SUB_VERSION)

#define MI_SYS_BindChnPort2 MI_SYS_BindChnPort

#ifdef __cplusplus
extern "C"
{
#endif

    MI_S32 MI_SYS_Init(MI_U16 u16SocId);

    MI_S32 MI_SYS_Exit(MI_U16 u16SocId);

    MI_S32 MI_SYS_BindChnPort(MI_U16 u16SocId, MI_SYS_ChnPort_t *pstSrcChnPort, MI_SYS_ChnPort_t *pstDstChnPort,
                               MI_U32 u32SrcFrmrate, MI_U32 u32DstFrmrate, MI_SYS_BindType_e eBindType,
                               MI_U32 u32BindParam);

    MI_S32 MI_SYS_UnBindChnPort(MI_U16 u16SocId, MI_SYS_ChnPort_t *pstSrcChnPort, MI_SYS_ChnPort_t *pstDstChnPort);

    MI_S32 MI_SYS_GetBindbyDest(MI_U16 u16SocId, MI_SYS_ChnPort_t *pstDstChnPort, MI_SYS_ChnPort_t *pstSrcChnPort,
                                MI_SYS_BindType_e *peBindType);

    MI_S32 MI_SYS_GetVersion(MI_U16 u16SocId, MI_SYS_Version_t *pstVersion);

    MI_S32 MI_SYS_GetCurPts(MI_U16 u16SocId, MI_U64 *pu64Pts);

    MI_S32 MI_SYS_InitPtsBase(MI_U16 u16SocId, MI_U64 u64PtsBase);

    MI_S32 MI_SYS_SyncPts(MI_U16 u16SocId, MI_U64 u64Pts);

    MI_S32 MI_SYS_SetReg(MI_U16 u16SocId, MI_U32 u32RegAddr, MI_U16 u16Value, MI_U16 u16Mask);

    MI_S32 MI_SYS_GetReg(MI_U16 u16SocId, MI_U32 u32RegAddr, MI_U16 *pu16Value);

    MI_S32 MI_SYS_ReadUuid(MI_U16 u16SocId, MI_U64 *u64Uuid);

    MI_S32 MI_SYS_SetChnMMAConf(MI_U16 u16SocId, MI_ModuleId_e eModId, MI_U32 u32DevId, MI_U32 u32ChnId,
                                MI_U8 *pu8MMAHeapName);

    MI_S32 MI_SYS_GetChnMMAConf(MI_U16 u16SocId, MI_ModuleId_e eModId, MI_U32 u32DevId, MI_U32 u32ChnId, void *data,
                                MI_U32 u32Length);

    MI_S32 MI_SYS_ChnInputPortGetBuf(MI_SYS_ChnPort_t *pstChnPort, MI_SYS_BufConf_t *pstBufConf,
                                     MI_SYS_BufInfo_t *pstBufInfo, MI_SYS_BUF_HANDLE *bufHandle, MI_S32 s32TimeOutMs);

    MI_S32 MI_SYS_ChnInputPortPutBuf(MI_SYS_BUF_HANDLE bufHandle, MI_SYS_BufInfo_t *pstBufInfo, MI_BOOL bDropBuf);

    MI_S32 MI_SYS_ChnOutputPortGetBuf(MI_SYS_ChnPort_t *pstChnPort, MI_SYS_BufInfo_t *pstBufInfo,
                                      MI_SYS_BUF_HANDLE *bufHandle);

    MI_S32 MI_SYS_ChnOutputPortPutBuf(MI_SYS_BUF_HANDLE hBufHandle);

    MI_S32 MI_SYS_ChnInputPortGetBufPa(MI_SYS_ChnPort_t *pstChnPort, MI_SYS_BufConf_t *pstBufConf,
                                       MI_SYS_BufInfo_t *pstBufInfo, MI_SYS_BUF_HANDLE *pBufHandle,
                                       MI_S32 s32TimeOutMs);

    MI_S32 MI_SYS_ChnInputPortPutBufPa(MI_SYS_BUF_HANDLE bufHandle, MI_SYS_BufInfo_t *pstBufInfo, MI_BOOL bDropBuf);

    MI_S32 MI_SYS_ChnOutputPortGetBufPa(MI_SYS_ChnPort_t *pstChnPort, MI_SYS_BufInfo_t *pstBufInfo,
                                        MI_SYS_BUF_HANDLE *pBufHandle);

    MI_S32 MI_SYS_ChnOutputPortPutBufPa(MI_SYS_BUF_HANDLE bufHandle);

    MI_S32 MI_SYS_SetChnOutputPortDepth(MI_U16 u16SocId, MI_SYS_ChnPort_t *pstChnPort, MI_U32 u32UserFrameDepth,
                                        MI_U32 u32BufQueueDepth);

    MI_S32 MI_SYS_ChnPortInjectBuf(MI_SYS_BUF_HANDLE handle, MI_SYS_ChnPort_t *pstChnInputPort);

    MI_S32 MI_SYS_GetFd(MI_SYS_ChnPort_t *pstChnPort, MI_S32 *s32Fd);

    MI_S32 MI_SYS_CloseFd(MI_S32 s32ChnPortFd);

    MI_S32 MI_SYS_ConfigPrivateMMAPool(MI_U16 u16SocId, MI_SYS_GlobalPrivPoolConfig_t *pstGlobalPrivPoolConf);

    MI_S32 MI_SYS_MemsetPa(MI_U16 u16SocId, MI_PHY phyPa, MI_U32 u32Val, MI_U32 u32Lenth);

    MI_S32 MI_SYS_MemcpyPa(MI_U16 u16SocId, MI_PHY phyDst, MI_PHY phySrc, MI_U32 u32Lenth);

    MI_S32 MI_SYS_MemcpyPaEx(MI_U16 u16SocId, MI_SYS_MemcpyDirect_e eMemcpyDirect, MI_PHY phyDst, MI_PHY phySrc, MI_U32 u32Lenth);

    MI_S32 MI_SYS_BufFillPa(MI_U16 u16SocId, MI_SYS_FrameData_t *pstBuf, MI_U32 u32Val, MI_SYS_WindowRect_t *pstRect);

    MI_S32 MI_SYS_BufBlitPa(MI_U16 u16SocId, MI_SYS_FrameData_t *pstDstBuf, MI_SYS_WindowRect_t *pstDstRect,
                            MI_SYS_FrameData_t *pstSrcBuf, MI_SYS_WindowRect_t *pstSrcRect);

    MI_S32 MI_SYS_PrivateDevChnHeapAlloc(MI_U16 u16SocId, MI_ModuleId_e eModule, MI_U32 u32Devid, MI_S32 s32ChnId,
                                         MI_U8 *pu8BufName, MI_U32 u32blkSize, MI_PHY *pphyAddr, MI_BOOL bTailAlloc);

    MI_S32 MI_SYS_PrivateDevChnHeapFree(MI_U16 u16SocId, MI_ModuleId_e eModule, MI_U32 u32Devid, MI_S32 s32ChnId,
                                        MI_PHY phyAddr);

    MI_S32 MI_SYS_EnableChnOutputPortLowLatency(MI_U16 u16SocId, MI_SYS_ChnPort_t *pstChnPort, MI_BOOL bEnable,
                                                MI_U32 u32Param);

    MI_S32 MI_SYS_DupBuf(MI_SYS_BUF_HANDLE srcBufHandle, MI_SYS_BUF_HANDLE *pDupTargetBufHandle);

    MI_S32 MI_SYS_Va2Pa(void *pVirtualAddress, MI_PHY *pPhyAddr);

    /*
    N.B.
    below MMAHeapName can only be NULL or real mma heap name, do not set it with random character string.
    you can get mma heap name xxx from "mma_heap=xxx," of cat /proc/cmdline.
    */
    /* NOLINTNEXTLINE */
    MI_S32 MI_SYS_MMA_Alloc(MI_U16 u16SocId, MI_U8 *pstMMAHeapName, MI_U32 u32BlkSize, MI_PHY *phyAddr);
    /* NOLINTNEXTLINE */
    MI_S32 MI_SYS_MMA_Free(MI_U16 u16SocId, MI_PHY phyAddr);
    MI_S32 MI_SYS_Mmap(MI_U64 phyAddr, MI_U32 u32Size, void **ppVirtualAddress, MI_BOOL bCache);

    MI_S32 MI_SYS_Munmap(void *pVirtualAddress, MI_U32 u32Size);
    MI_S32 MI_SYS_FlushInvCache(void *pVirtualAddress, MI_U32 u32Length);

    MI_S32 MI_SYS_ChnInputPortSetUserPicture(MI_SYS_BUF_HANDLE BufHandle, MI_SYS_BufInfo_t *pstBufInfo,
                                             MI_SYS_UserPictureInfo_t *pstUserPictureInfo);

    MI_S32 MI_SYS_EnableUserPicture(MI_SYS_BUF_HANDLE BufHandle);

    MI_S32 MI_SYS_DisableUserPicture(MI_SYS_BUF_HANDLE BufHandle);

    MI_S32 MI_SYS_SetChnOutputPortUserFrc(MI_SYS_ChnPort_t *pstChnPort, MI_U32 u32SrcFrmrate, MI_U32 u32DstFrmrate);

    MI_S32 MI_SYS_SetChnOutputPortBufExtConf(MI_SYS_ChnPort_t *pstChnPort, MI_SYS_FrameBufExtraConfig_t *pstBufExtraConf);

    MI_S32 MI_SYS_SetChnInputPortFrc(MI_SYS_ChnPort_t *pstChnPort, MI_SYS_ChnPortFrcAttr_t *pstFrcAttr);

    MI_S32 MI_SYS_QueryDevChnPortState(MI_SYS_ChnPort_t *pstChnPort, MI_BOOL bIsInputPort, MI_SYS_ChnPortState_t *pstState);

    MI_S32 MI_SYS_CreateChnInputPortDmabufCusAllocator(MI_SYS_ChnPort_t *pstChnPort);
    MI_S32 MI_SYS_DestroyChnInputPortDmabufCusAllocator(MI_SYS_ChnPort_t *pstChnPort);
    MI_S32 MI_SYS_ChnInputPortEnqueueDmabuf(MI_SYS_ChnPort_t *pstChnPort, MI_SYS_DmaBufInfo_t *pstDmaBufInfo);
    MI_S32 MI_SYS_ChnInputPortDequeueDmabuf(MI_SYS_ChnPort_t *pstChnPort, MI_SYS_DmaBufInfo_t *pstDmaBufInfo);

    MI_S32 MI_SYS_CreateChnOutputPortDmabufCusAllocator(MI_SYS_ChnPort_t *pstChnPort);
    MI_S32 MI_SYS_DestroyChnOutputPortDmabufCusAllocator(MI_SYS_ChnPort_t *pstChnPort);
    MI_S32 MI_SYS_ChnOutputPortEnqueueDmabuf(MI_SYS_ChnPort_t *pstChnPort, MI_SYS_DmaBufInfo_t *pstDmaBufInfo);
    MI_S32 MI_SYS_ChnOutputPortDequeueDmabuf(MI_SYS_ChnPort_t *pstChnPort, MI_SYS_DmaBufInfo_t *pstDmaBufInfo);
    MI_S32 MI_SYS_ChnOutputPortDropDmabuf(MI_SYS_ChnPort_t *pstChnPort, MI_SYS_DmaBufInfo_t *pstDmaBufInfo);
    MI_S32 MI_SYS_GetDmaBuf(MI_S32 s32Fd, MI_PHY *pPhyAddr);
    MI_S32 MI_SYS_PutDmaBuf(MI_PHY phyAddr);

#ifdef __cplusplus
}
#endif

#endif ///_MI_SYS_H_
