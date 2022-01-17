#ifndef __FY_COMM_VB_H__
#define __FY_COMM_VB_H__

#include "fy_type.h"
#include "fy_errno.h"
#include "fy_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define VB_MAX_POOLS 256
#define VB_MAX_COMM_POOLS 16
#define VB_MAX_MOD_COMM_POOLS 16


/* user ID for VB */

typedef enum fyVB_UID_E
{
    VB_UID_VIU			= 0,
    VB_UID_VOU			= 1,
    VB_UID_VGS			= 2,
    VB_UID_VENC 		= 3,
    VB_UID_VDEC 		= 4,
    VB_UID_VDA			= 5,
    VB_UID_H264E		= 6,
    VB_UID_JPEGED		= 7,
    VB_UID_MPEG4E		= 8,
    VB_UID_H264D		= 9,
    VB_UID_JPEGD		= 10,
    VB_UID_MPEG4D		= 11,
    VB_UID_VPSS 		= 12,
    VB_UID_GRP			= 13,
    VB_UID_MPI			= 14,
    VB_UID_PCIV 		= 15,
    VB_UID_AI			= 16,
    VB_UID_AENC 		= 17,
    VB_UID_RC			= 18,
    VB_UID_VFMW 		= 19,
    VB_UID_USER 		= 20,
    VB_UID_H265E		= 21,
//fh module   
    VB_UID_BGM 	        = 22,
    VB_UID_BGMX         = 23,
    VB_UID_BGMSW        = 24,
    
    VB_UID_VPPU         = 25,
    
    VB_UID_BUTT    
} VB_UID_E;

#define VB_MAX_USER   VB_UID_BUTT

#define VB_INVALID_POOLID (-1UL)
#define VB_INVALID_HANDLE (-1UL)

/* Generall common pool use this owner id, module common pool use VB_UID as owner id */
#define POOL_OWNER_COMMON	-1

/* Private pool use this owner id */
#define POOL_OWNER_PRIVATE	-2

typedef enum fyPOOL_TYPE_E
{
    POOL_TYPE_COMMON			= 0,
    POOL_TYPE_PRIVATE			= 1,
    POOL_TYPE_MODULE_COMMON		= 2,
    POOL_TYPE_BUTT
} POOL_TYPE_E;

typedef FY_U32 VB_POOL;
typedef FY_U32 VB_BLK;

#define RESERVE_MMZ_NAME "window"

typedef struct fyVB_CONF_S
{
    FY_U32 u32MaxPoolCnt;     /* max count of pools, (0,VB_MAX_POOLS]  */    
    struct fyVB_CPOOL_S
    {
        FY_U32 u32BlkSize;
        FY_U32 u32BlkCnt;
        FY_CHAR acMmzName[MAX_MMZ_NAME_LEN];
    }astCommPool[VB_MAX_COMM_POOLS];
} VB_CONF_S;

typedef struct fyVB_POOL_STATUS_S
{
    FY_U32 bIsCommPool;
    FY_U32 u32BlkCnt;
    FY_U32 u32FreeBlkCnt;
}VB_POOL_STATUS_S;

#define VB_SUPPLEMENT_JPEG_MASK 0x1

typedef struct fyVB_SUPPLEMENT_CONF_S
{
    FY_U32 u32SupplementConf;
}VB_SUPPLEMENT_CONF_S;


#define FY_ERR_VB_NULL_PTR  FY_DEF_ERR(FY_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
#define FY_ERR_VB_NOMEM     FY_DEF_ERR(FY_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_NOMEM)
#define FY_ERR_VB_NOBUF     FY_DEF_ERR(FY_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_NOBUF)
#define FY_ERR_VB_UNEXIST   FY_DEF_ERR(FY_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_UNEXIST)
#define FY_ERR_VB_ILLEGAL_PARAM FY_DEF_ERR(FY_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
#define FY_ERR_VB_NOTREADY  FY_DEF_ERR(FY_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)
#define FY_ERR_VB_BUSY      FY_DEF_ERR(FY_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_BUSY)
#define FY_ERR_VB_NOT_PERM  FY_DEF_ERR(FY_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)

#define FY_ERR_VB_2MPOOLS FY_DEF_ERR(FY_ID_VB, EN_ERR_LEVEL_ERROR, EN_ERR_BUTT + 1)

#define FY_TRACE_VB(level,fmt...) FY_TRACE(level, FY_ID_VB,##fmt)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  /* __FY_COMM_VB_H_ */

