#ifndef __FY_VGS_DRV_H__
#define __FY_VGS_DRV_H__


/******************************************************************************
    Include other header files
 *****************************************************************************/
#include "../fy_comm_vgs.h"
#include "../fy_comm_tde.h"
#include "../fy_comm_vppu.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

/******************************************************************************
    Macros, Enums, Structures definition list
 *****************************************************************************/

typedef struct
{
    FY_S32  handle;
    FY_S32  reserved;
    VGS_TASK_ATTR_S stTaskAttr;
}VGS_TASK_ATTR_S_T;

typedef struct
{
    FY_S32  handle;
    FY_BOOL bFrmToSur;
    VIDEO_FRAME_S stFrame;
    TDE2_SURFACE_S stSurface;
}VGS_FMT_CONVERT_S;

typedef struct
{
    FY_S32 handle;
    FY_S32 reserved1;
    VGS_TASK_ATTR_S stTaskAttr;
    VGS_LINE_S astVgsDrawLine[10];
    FY_U32 count;
    FY_U32 reserved2;
}ADD_DRAW_LINE_S;

typedef struct
{
    FY_S32 handle;
    FY_S32 reserved1;
    VGS_TASK_ATTR_S stTaskAttr;
    VGS_COVER_S astVgsAddCover[100];
    FY_U32 count;
    FY_U32 reserved2;
}ADD_COVER_S;

typedef struct
{
    FY_S32 handle;
    FY_S32 reserved1;
    VGS_TASK_ATTR_S stTaskAttr;
    VGS_OSD_S astVgsAddOsd[100];
    FY_U32 count;
    FY_U32 reserved2;
}ADD_OSD_S;

typedef struct fyVGS_SEND_FRAME_S
{
    VPPU_CHN                vppuChn;
    FY_BOOL                 bPlayMode;
    VIDEO_FRAME_INFO_S      stImgSd;        	/* send picture */
    FY_U32                  u32TimeOut;
}VGS_SEND_FRAME_S;

typedef struct fyVGS_CHN_STAT_S
{
    VPPU_CHN                vppuChn;
    FY_U32                  u32GotFrmNum;
    FY_U32                  u32SkipFrmNum;
    FY_U32                  u32SendFrmOk;
    FY_U32                  u32SendFrmFail;
}VGS_CHN_STAT_S;

typedef struct fyVGS_CHN_MODE_S
{
    VPPU_CHN                vppuChn;
    VPPU_PTH                vppuPth;
    VPPU_CHN_MODE_S         vppuMode;
}VGS_CHN_MODE_S;

typedef struct fyVGS_CROP_INFO_S
{
    VPPU_CHN                vppuChn;
    VPPU_CROP_INFO_S        stVppuCrop;
}VGS_CROP_INFO_S;

typedef struct fyVGS_ROTATE_INFO_S
{
    VPPU_CHN                vppuChn;
    VPPU_PTH                vppuPth;
    ROTATE_E                enRotate;
}VGS_ROTATE_INFO_S;

typedef struct VGS_MULTI_PATH_S
{
    FY_S32                  handle;
    VIDEO_FRAME_INFO_S      stImgIn;        /* input picture */
    VIDEO_FRAME_INFO_S      stImgOut0;      /* output path0 picture */
    VIDEO_FRAME_INFO_S      stImgOut1;      /* output path1 picture */
    VIDEO_FRAME_INFO_S      stImgOut2;      /* output path2 picture */
}VGS_MULTI_PATH_S;

typedef struct fyVGS_IMG_GET_S
{
    FY_U32                  u32TimeOut;
    FY_S32                  NULL_Flag;		/* path1 NULL:0x02, path2 NULL:0x04 */
    VPPU_CHN                vppuChn;
    VIDEO_FRAME_INFO_S      stFrmPth1;
    VIDEO_FRAME_INFO_S      stFrmPth2;
}VGS_IMG_GET_S;

/******************************************************************************
    VGS driver ioctl definition list
 *****************************************************************************/
#define IOC_VGS_BEGIN_JOB               _IOWR('J',0,FY_S32)
#define IOC_VGS_END_JOB                 _IOW('J',2,FY_S32)
#define IOC_VGS_CANCEL_JOB              _IOW('J',1,FY_S32)
#define IOC_VGS_ADD_SCALE_TASK          _IOW('J',3,VGS_TASK_ATTR_S_T)
#define IOC_VGS_ADD_DRAW_LINE_TASK      _IOW('J',5,ADD_DRAW_LINE_S)
#define IOC_VGS_ADD_COVER_TASK          _IOW('J',6,ADD_COVER_S)
#define IOC_VGS_ADD_OSD_TASK            _IOW('J',7,ADD_OSD_S)
#define IOC_VGS_ADD_DECOMPRESS_TASK     _IOW('J',8,VGS_TASK_ATTR_S_T)
#define IOC_VGS_SEND_FRAME              _IOW('J',9,VGS_SEND_FRAME_S)
#define IOC_VGS_ADD_COMBINE_TASK        _IOW('J',10,VGS_COMBINE_S)
#define IOC_VGS_SET_CHN_MODE            _IOW('J',11,VGS_CHN_MODE_S)
#define IOC_VGS_GET_CHN_MODE            _IOWR('J',12,VGS_CHN_MODE_S)
#define IOC_VGS_CREATE_CHN              _IOWR('J',13,VPPU_CHN)
#define IOC_VGS_DESTORY_CHN             _IOWR('J',14,VPPU_CHN)
#define IOC_VGS_SET_CROP_INFO           _IOWR('J',15,VGS_CROP_INFO_S)
#define IOC_VGS_GET_CROP_INFO           _IOWR('J',16,VGS_CROP_INFO_S)
#define IOC_VGS_QUERY_FRAME             _IOWR('J',17,VGS_SEND_FRAME_S)
#define IOC_VGS_ADD_CONVERT_TASK        _IOWR('J',18,VGS_FMT_CONVERT_S)
#define IOC_VGS_ADD_ROTATE_TASK         _IOWR('J',19,VGS_TASK_ATTR_S_T)
#define IOC_VGS_SET_ROTAT_INFO          _IOWR('J',20,VGS_ROTATE_INFO_S)
#define IOC_VGS_GET_ROTAT_INFO          _IOWR('J',21,VGS_ROTATE_INFO_S)
#define IOC_VGS_ADD_MULTIPTH_TASK       _IOWR('J',22,VGS_MULTI_PATH_S)
#define IOC_VGS_GET_IMAGE               _IOWR('J',23,VGS_IMG_GET_S)




#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif//__FY_VGS_DRV_H__
