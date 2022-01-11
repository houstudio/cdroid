#ifndef _FY_API_TDE_H_
#define _FY_API_TDE_H_

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif /* __cplusplus */
#endif  /* __cplusplus */

#include "include/fy_type.h"
#include "include/fy_comm_tde.h"

#define FY_TDE_Open FY_TDE2_Open
#define FY_TDE_Close FY_TDE2_Close
#define FY_TDE_BeginJob FY_TDE2_BeginJob


/*****************************************************************************
* Function:      FY_TDE2_Open
* Description:   Open the TDE device
* Input:         None
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_Open(FY_VOID);

/*****************************************************************************
* Function:      FY_TDE2_Close
* Description:   Close the TDE device
* Input:         None
* Return:        None
*****************************************************************************/
FY_VOID FY_TDE2_Close(FY_VOID);

/*****************************************************************************
* Function:      FY_TDE2_BeginJob
* Description:   Create a TDE job, get a TDE job handle
* Input:         None
* Return:        tde handle / Error code
*****************************************************************************/
TDE_HANDLE FY_TDE2_BeginJob(FY_VOID);

/*****************************************************************************
* Function:      FY_TDE2_EndJob
* Description:   Submit a TDE job
* Input:         s32Handle:  job handle
*                bSync: if synchronous
*                bBlock: if blocked
*                u32TimeOut: timeout value(in 10ms)
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_EndJob(TDE_HANDLE s32Handle, FY_BOOL bSync, FY_BOOL bBlock, FY_U32 u32TimeOut);

/*****************************************************************************
* Function:      FY_TDE2_CancelJob
* Description:   Cancel a specific TDE job, only successful before calling EndJob
* Input:         s32Handle:  job handle
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_CancelJob(TDE_HANDLE s32Handle);

/*****************************************************************************
* Function:      FY_TDE2_WaitForDone
* Description:   Wait for a submitted job to finish
* Input:         s32Handle:  job handle
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_WaitForDone(TDE_HANDLE s32Handle);

/*****************************************************************************
* Function:      FY_TDE2_WaitAllDone
* Description:   Wait for all submitted jobs to finish
* Input:         None
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_WaitAllDone(FY_VOID);

/*****************************************************************************
* Function:      FY_TDE2_Reset
* Description:   Reset tde
* Input:         None
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_Reset(FY_VOID);

/*****************************************************************************
* Function:      FY_TDE2_SetDeflickerLevel
* Description:   Set the anti-flicker level
* Input:         enDeflickerLevel: anti-flicker level 
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_SetDeflickerLevel(TDE_DEFLICKER_LEVEL_E enDeflickerLevel);

/*****************************************************************************
* Function:      FY_TDE2_GetDeflickerLevel
* Description:   Get the anti-flicker level
* Input:         pDeflickerLevel: to save the anti-flicker level 
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_GetDeflickerLevel(TDE_DEFLICKER_LEVEL_E *pDeflickerLevel);

/*****************************************************************************
* Function:      FY_TDE2_SetAlphaThresholdValue
* Description:   Set the anti-flicker level
* Input:         u8ThresholdValue: Alpha threshold 
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_SetAlphaThresholdValue(FY_U8 u8ThresholdValue);

/*****************************************************************************
* Function:      FY_TDE2_GetAlphaThresholdValue
* Description:   Get the anti-flicker level
* Input:         pu8ThresholdValue: to save the alpha threshold 
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_GetAlphaThresholdValue(FY_U8 *pu8ThresholdValue);

/*****************************************************************************
* Function:      FY_TDE2_GetAlphaThresholdValue
* Description:   Enable or disable alpha judgment
* Input:         bEnAlphaThreshold: whether to enable alpha judgment
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_SetAlphaThresholdState(FY_BOOL bEnAlphaThreshold);

/*****************************************************************************
* Function:      FY_TDE2_GetAlphaThresholdState
* Description:   Get alpha judgment state
* Input:         p_bEnAlphaThreshold: To save the alpha judgment state
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_GetAlphaThresholdState(FY_BOOL * p_bEnAlphaThreshold);

/*****************************************************************************
* Function:      FY_TDE2_QuickCopy
* Description:   Just quick copy, the size of source region and destination region should be the same, so is the color format
* Input:         s32Handle:  job handle
*                pSrc: the source picture information
*                pstSrcRect: the source picture operation region
*                pDst: the destination picture information
*                pstDstRect: the destination picture operation region
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_QuickCopy(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstSrc, TDE2_RECT_S  *pstSrcRect,
                              TDE2_SURFACE_S* pstDst, TDE2_RECT_S *pstDstRect);

/*****************************************************************************
* Function:      FY_TDE2_QuickFill
* Description:   Quick fill
* Input:         s32Handle:  job handle
*                pDst: the destination picture information
*                pstDstRect: the destination picture operation region
*                u32FillData: the color value,its format should be the same to the destination picture
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_QuickFill(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstDst, TDE2_RECT_S *pstDstRect,
                              FY_U32 u32FillData);

/*****************************************************************************
* Function:      FY_TDE2_QuickResize
* Description:   Add the raster bitmap scaling operation to a TDE job
* Input:         s32Handle:  job handle
*                pSrc: the source picture information
*                pstSrcRect: the source picture operation region
*                pDst: the destination picture information
*                pstDstRect: the destination picture operation region
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_QuickResize(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstSrc, TDE2_RECT_S  *pstSrcRect,
                                TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect);

/*****************************************************************************
* Function:      FY_TDE2_QuickFlicker
* Description:   Add the anti-flicker operation to a TDE job
* Input:         s32Handle:  job handle
*                pSrc: the source picture information
*                pstSrcRect: the source picture operation region
*                pDst: the destination picture information
*                pstDstRect: the destination picture operation region
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_QuickDeflicker(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstSrc, TDE2_RECT_S  *pstSrcRect,
                                   TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect);

/*****************************************************************************
* Function:      FY_TDE2_Blit
* Description:   Add the transfer operation with additional functions performed on
                    the raster bitmap to a TDE task
* Input:         s32Handle:  job handle
*                pstBackGround: the background picture information
*                pstBackGroundRect: the background picture operation region
*                pstForeGround: the foreground picture information
*                pstForeGroundRect: the foreground picture operation region
*                pstDst:  the destination picture information
*                pstDstRect:  the destination picture operation region
*                pOpt:  operation parameter settings
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_Bitblit(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstBackGround, TDE2_RECT_S  *pstBackGroundRect,
                            TDE2_SURFACE_S* pstForeGround, TDE2_RECT_S  *pstForeGroundRect, TDE2_SURFACE_S* pstDst,
                            TDE2_RECT_S  *pstDstRect, TDE2_OPT_S* pstOpt);

/*****************************************************************************
* Function:      FY_TDE2_SolidDraw
* Description:   Add the filling operation with additional functions performed on
                    the raster bitmap to a TDE task
* Input:         s32Handle:  job handle
*                pstForeGround:  the foreground picture information
*                pstForeGroundRect: the source picture operation region
*                pstDst:  the background picture information
*                pstDstRect: the destination picture operation region
*                pstFillColor:  the color value
*                pstOpt:  operation parameter settings
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_SolidDraw(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstForeGround, TDE2_RECT_S  *pstForeGroundRect,
                              TDE2_SURFACE_S *pstDst,
                              TDE2_RECT_S  *pstDstRect, TDE2_FILLCOLOR_S *pstFillColor,
                              TDE2_OPT_S *pstOpt);

/*****************************************************************************
* Function:      FY_TDE2_DoCompress
* Description:   Compress the data
* Input:         pstSrc: the picture information needs be compressed
                 pstDst: the picture information compressed
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_DoCompress(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstSrc, TDE2_SURFACE_S* pstDst);

/*****************************************************************************
* Function:      FY_TDE2_DeCompress
* Description:   Decompress the data
* Input:         pstSrc: the picture information compressed
                 pstDst: the original picture information before compressed
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_DeCompress(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstSrc, TDE2_SURFACE_S* pstDst);

/*****************************************************************************
* Function:      FY_TDE2_MbBlit
* Description:   Add the transfer operation with additional functions performed on
                    the macroblock bitmap to a TDE task
* Input:         s32Handle:  job handle
*                pstMB:  Surface of the macroblock
*                pstDst: Operating region of the macroblock
*                pstDstRect:  the destination picture operation region
*                pstMbOpt: operation parameter settings
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_MbBlit(TDE_HANDLE s32Handle, TDE2_MB_S* pstMB, TDE2_RECT_S  *pstMbRect, TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect,
                           TDE2_MBOPT_S* pstMbOpt);

/*****************************************************************************
* Function:      FY_TDE2_BitmapMaskRop
* Description:   Add the mask raster operation (ROP) operation performed
                    on the raster bitmap to a TDE task.
* Input:         s32Handle:  job handle
*                pstBackGround:  the background picture information
*                pstBackGroundRect: the background picture operation region
*                pstForeGround: the foreground picture information
*                pstForeGroundRect: the source picture operation region
*                pstMask: mask picture information
*                pstMaskRect: operating region of the mask picture
*                pstDst:  the destination picture information
*                pstDstRect:  the destination picture operation region
*                enRopCode_Color: ROP operation code of the color component
*                enRopCode_Alpha: ROP operation code of the alpha component
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_BitmapMaskRop(TDE_HANDLE s32Handle, 
                                  TDE2_SURFACE_S* pstBackGround, TDE2_RECT_S  *pstBackGroundRect,
                                  TDE2_SURFACE_S* pstForeGround, TDE2_RECT_S  *pstForeGroundRect,
                                  TDE2_SURFACE_S* pstMask, TDE2_RECT_S  *pstMaskRect, 
                                  TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect,
                                  TDE2_ROP_CODE_E enRopCode_Color, TDE2_ROP_CODE_E enRopCode_Alpha);

/*****************************************************************************
* Function:      FY_TDE2_BitmapMaskBlend
* Description:  Add the mask blending operation performed on the raster
                    bitmap to a TDE task
* Input:         s32Handle:  job handle
*                pstBackGround: the background picture information
*                pstBackGroundRect: the background picture operation region
*                pstForeGround: the foreground picture information
*                pstForeGroundRect: the foreground picture operation region
*                pstMask: mask picture information
*                pstMaskRect: operating region of the mask picture
*                pstDst:  the destination picture information
*                pstDstRect:  the destination picture operation region
*                u8Alpha:  global alpha value during alpha blending
*                enBlendMode: alpha blending mode
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_BitmapMaskBlend(TDE_HANDLE s32Handle, 
                                    TDE2_SURFACE_S* pstBackGround, TDE2_RECT_S  *pstBackGroundRect,
                                    TDE2_SURFACE_S* pstForeGround, TDE2_RECT_S  *pstForeGroundRect,
                                    TDE2_SURFACE_S* pstMask, TDE2_RECT_S  *pstMaskRect,
                                    TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect,
                                    FY_U8 u8Alpha, TDE2_ALUCMD_E enBlendMode);

/*****************************************************************************
* Function:      FY_TDE2_PatternFill
* Description:   Pattern fill
*Input:           s32Handle:  job handle
*                pstBackGround: the background picture information
*                pstBackGroundRect: the background picture operation region
*                pstForeGround: the foreground picture information
*                pstForeGroundRect: the foreground picture operation region
*                pstDst:  the destination picture information
*                pstDstRect:  the destination picture operation region
*                pstOpt:  operation parameter settings
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_PatternFill(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstBackGround, 
                                TDE2_RECT_S *pstBackGroundRect, TDE2_SURFACE_S *pstForeGround,
                                TDE2_RECT_S *pstForeGroundRect, TDE2_SURFACE_S *pstDst,
                                TDE2_RECT_S *pstDstRect, TDE2_PATTERN_FILL_OPT_S *pstOpt);

/*****************************************************************************
* Function:      FY_TDE2_EnableRegionDeflicker
* Description:   To enable or disable the regional anti-flicker function
* Input:         bRegionDeflicker: enable flag
* Return:        Success / Error code
*****************************************************************************/
FY_S32 FY_TDE2_EnableRegionDeflicker(FY_BOOL bRegionDeflicker);



#ifdef __cplusplus
 #if __cplusplus
}
 #endif /* __cplusplus */
#endif  /* __cplusplus */

#endif  /* _FY_API_TDE_H_ */
