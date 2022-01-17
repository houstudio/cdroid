#ifndef  __FY_COMM_VPPU_H__
#define  __FY_COMM_VPPU_H__

#include "fy_type.h"
#include "fy_common.h"
#include "fy_errno.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

/*Define vpss channel's work mode*/
typedef enum fyVPPU_CHN_MODE_E
{
    VPPU_CHN_MODE_AUTO = 0, /*Auto mode*/
    VPPU_CHN_MODE_USER  /*User mode*/
}VPPU_CHN_MODE_E;

typedef struct fyVPPU_CHN_MODE_S
{
    VPPU_CHN_MODE_E  enChnMode;   /*Vppu channel's work mode*/
    FY_U32 u32Width;              /*Width of target image*/
    FY_U32 u32Height;             /*Height of target image*/
    fyPIXEL_FORMAT_E  enPixelFormat;/*Pixel format of target image*/
    //COMPRESS_MODE_E enCompressMode;   /*Compression mode of the output, vppu out must None*/
}VPPU_CHN_MODE_S;

typedef enum fyVPPU_CROP_COORDINATE_E
{
    VPPU_CROP_RATIO_COOR = 0,   /*Ratio coordinate*/
    VPPU_CROP_ABS_COOR          /*Absolute coordinate*/
}VPPU_CROP_COORDINATE_E;

typedef struct fyVPPU_CROP_INFO_S
{
    FY_BOOL bEnable;         /*CROP enable*/
    VPPU_CROP_COORDINATE_E  enCropCoordinate;   /*Coordinate mode of the crop start point*/
    RECT_S  stCropRect;     /*CROP rectangular*/
}VPPU_CROP_INFO_S ;


/* invlalid channel ID */
#define FY_ERR_VPPU_INVALID_CHNID     FY_DEF_ERR(FY_ID_VPPU, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_CHNID)
/* at lease one parameter is illagal ,eg, an illegal enumeration value  */
#define FY_ERR_VPPU_ILLEGAL_PARAM     FY_DEF_ERR(FY_ID_VPPU, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
/* channel exists */
#define FY_ERR_VPPU_EXIST             FY_DEF_ERR(FY_ID_VPPU, EN_ERR_LEVEL_ERROR, EN_ERR_EXIST)
/* using a NULL point */
#define FY_ERR_VPPU_NULL_PTR          FY_DEF_ERR(FY_ID_VPPU, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
/* operation is not supported by NOW */
#define FY_ERR_VPPU_NOT_SUPPORT       FY_DEF_ERR(FY_ID_VPPU, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SUPPORT)
/* operation is not permitted ,eg, try to change static attribute */
#define FY_ERR_VPPU_NOT_PERM          FY_DEF_ERR(FY_ID_VPPU, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)
#define FY_ERR_VPPU_SYS_NOTREADY      FY_DEF_ERR(FY_ID_VPPU, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)
#define FY_ERR_VPPU_NOMEM             FY_DEF_ERR(FY_ID_VPPU, EN_ERR_LEVEL_ERROR, EN_ERR_NOMEM)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif/* End of #ifndef __FY_COMM_VPPU_H__*/

