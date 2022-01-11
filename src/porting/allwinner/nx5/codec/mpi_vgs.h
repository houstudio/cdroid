#ifndef __MPI_VGS_H__
#define __MPI_VGS_H__

#include "include/fy_common.h"
#include "include/fy_comm_video.h"
#include "include/fy_comm_vgs.h"
#include "include/fy_comm_tde.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */



/*****************************************************************************
* Function:      FY_MPI_VGS_BeginJob
* Description:   Begin a vgs job,then add task into the job,vgs will finish all the task in the job.
* Input:         VGS_HANDLE *phHandle
* Return:        Success / Error code
*****************************************************************************/
FY_S32  FY_MPI_VGS_BeginJob(VGS_HANDLE *phHandle);

/*****************************************************************************
* Function:      FY_MPI_VGS_EndJob
* Description:   End a job,all tasks in the job will be submmitted to vgs.
* Input:         VGS_HANDLE *phHandle
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_MPI_VGS_EndJob(VGS_HANDLE hHandle);

/*****************************************************************************
* Function:      FY_MPI_VGS_CancelJob
* Description:   Cancel a job ,then all tasks in the job will not be submmitted to vgs.
* Input:         VGS_HANDLE *phHandle
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_MPI_VGS_CancelJob(VGS_HANDLE hHandle);

/*****************************************************************************
* Function:      FY_MPI_VGS_AddScaleTask
* Description:   Add a task to a vgs job.
* Input:         VGS_HANDLE hHandle
* Return:        Success / Error code
*****************************************************************************/
FY_S32  FY_MPI_VGS_AddScaleTask(VGS_HANDLE hHandle, VGS_TASK_ATTR_S *pstTask);

/*****************************************************************************
* Function:      FY_MPI_VGS_AddDrawLineTask
* Description:   Add a draw line task into a job.
* Input:         VGS_HANDLE hHandle
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_MPI_VGS_AddDrawLineTask(VGS_HANDLE hHandle, VGS_TASK_ATTR_S *pstTask, VGS_LINE_S astVgsDrawLine[], FY_U32 u32ArraySize);

FY_S32 FY_MPI_VGS_AddCombineTask(VGS_HANDLE hHandle, VIDEO_FRAME_S *pstVFrmIn, RECT_S *pstInRect, VIDEO_FRAME_S *pstVFrmOut, RECT_S *pstOutRect);

/*****************************************************************************
* Function:      FY_MPI_VGS_DeCompress
* Description:   Add a deCompress task into a job.
* Input:         VGS_HANDLE hHandle
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_MPI_VGS_DeCompress(VGS_HANDLE phHandle, const VIDEO_FRAME_S* pstVFrmIn, VIDEO_FRAME_S  * pstVFrmOut);

/*****************************************************************************
* Function:      FY_MPI_VGS_AddCoverTask
* Description:   Add a cover task into a job.
* Input:         VGS_HANDLE hHandle
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_MPI_VGS_AddCoverTask(VGS_HANDLE hHandle, VGS_TASK_ATTR_S *pstTask, VGS_COVER_S astVgsAddCover[], FY_U32 u32ArraySize);

/*****************************************************************************
* Function:      FY_MPI_VGS_AddOsdTask
* Description:   Add a osd task into a job.
* Input:         VGS_HANDLE hHandle
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_MPI_VGS_AddOsdTask(VGS_HANDLE hHandle, VGS_TASK_ATTR_S *pstTask, VGS_OSD_S astVgsAddOsd[], FY_U32 u32ArraySize);

FY_S32  FY_MPI_VGS_FormatConvert(VGS_HANDLE hHandle, VIDEO_FRAME_S* pstFrame, TDE2_SURFACE_S* pstSurface, FY_BOOL bFrmToSur);

FY_S32 FY_MPI_VGS_Rotate(VGS_TASK_ATTR_S *pstTask, VGS_ROTATE_S *pstRotate);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MPI_VGS_H__ */

