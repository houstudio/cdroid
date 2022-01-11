#ifndef __MPI_VPPU_H__
#define __MPI_VPPU_H__

#include "include/fy_comm_video.h"
#include "include/fy_comm_vppu.h"
#include "include/drv/vgs_drv_ioc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

FY_S32 MPI_VPPU_Init();
FY_S32 MPI_VPPU_Exit();

FY_S32 FY_MPI_VPPU_CreateChn(VPPU_CHN VppuChn);

FY_S32 FY_MPI_VPPU_DestroyChn(VPPU_CHN VppuChn);

FY_S32 FY_MPI_VPPU_SetChnMode(VPPU_CHN VppuChn, VPPU_PTH VppuPth, VPPU_CHN_MODE_S *pstVppuMode);

FY_S32 FY_MPI_VPPU_GetChnMode(VPPU_CHN VppuChn, VPPU_PTH VppuPth, VPPU_CHN_MODE_S *pstVppuMode);

FY_S32 FY_MPI_VPPU_SetChnCrop(VPPU_CHN VppuChn,  VPPU_CROP_INFO_S *pstCropInfo);

FY_S32 FY_MPI_VPPU_GetChnCrop(VPPU_CHN VppuChn,  VPPU_CROP_INFO_S *pstCropInfo);

FY_S32 FY_MPI_VPPU_SetChnRotate(VPPU_CHN VppuChn, VPPU_PTH VppuPth, ROTATE_E enRotate);

FY_S32 FY_MPI_VPPU_GetChnRotate(VPPU_CHN VppuChn, VPPU_PTH VppuPth, ROTATE_E *penRotate);

FY_S32 FY_MPI_VPPU_ImageConvert(const VIDEO_FRAME_S  *pstVFrmIn, VIDEO_FRAME_S  *pstVFrmOut);

FY_S32 FY_MPI_VPPU_FormatConvert(VIDEO_FRAME_S* pstFrame, TDE2_SURFACE_S* pstSurface, FY_BOOL bFrmToSur);

FY_S32 FY_MPI_VPPU_SendFrame(VPPU_CHN VppuChn, const VIDEO_FRAME_INFO_S *pstVFrm, FY_BOOL bPlayMode, FY_U32 u32TimeOut);

FY_S32 FY_MPI_VPPU_Query(VPPU_CHN VppuChn, FY_U32 *pSendNum);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MPI_VPPU_H__ */

