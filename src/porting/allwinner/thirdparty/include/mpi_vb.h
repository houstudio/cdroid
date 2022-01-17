#ifndef __MPI_VB_H__
#define __MPI_VB_H__

#include "include/fy_comm_vb.h"
#include "include/fy_comm_sys.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

VB_POOL FY_MPI_VB_CreatePool(FY_U32 u32BlkSize,FY_U32 u32BlkCnt,const FY_CHAR *pcMmzName);
FY_S32 FY_MPI_VB_DestroyPool(VB_POOL Pool);

VB_BLK FY_MPI_VB_GetBlock(VB_POOL Pool, FY_U32 u32BlkSize,const FY_CHAR *pcMmzName);
FY_S32 FY_MPI_VB_ReleaseBlock(VB_BLK Block);

FY_U32 FY_MPI_VB_Handle2PhysAddr(VB_BLK Block);
VB_POOL FY_MPI_VB_Handle2PoolId(VB_BLK Block);
FY_S32 FY_MPI_VB_InquireUserCnt(VB_BLK Block);

FY_S32 FY_MPI_VB_Init(FY_VOID);
FY_S32 FY_MPI_VB_Exit(FY_VOID);
FY_S32 FY_MPI_VB_SetConf(const VB_CONF_S *pstVbConf);
FY_S32 FY_MPI_VB_GetConf(VB_CONF_S *pstVbConf);

FY_S32 FY_MPI_VB_MmapPool(VB_POOL Pool);
FY_S32 FY_MPI_VB_MunmapPool(VB_POOL Pool);

FY_S32 FY_MPI_VB_GetBlkVirAddr(VB_POOL Pool, FY_U32 u32PhyAddr, FY_VOID **ppVirAddr);

FY_S32 FY_MPI_VB_InitModCommPool(VB_UID_E enVbUid);
FY_S32 FY_MPI_VB_ExitModCommPool(VB_UID_E enVbUid);

FY_S32 FY_MPI_VB_SetModPoolConf(VB_UID_E enVbUid, const VB_CONF_S *pstVbConf);
FY_S32 FY_MPI_VB_GetModPoolConf(VB_UID_E enVbUid, VB_CONF_S *pstVbConf);

VB_POOL FY_MPI_VB_CreateVirtualVbPool(FY_U32 u32BlkCnt, const FY_CHAR *pcMmzName);
VB_BLK FY_MPI_VB_AddBlock(VB_POOL Pool , FY_U32 u32PhyAddr, FY_U32 u32Size);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__MPI_VI_H__ */

