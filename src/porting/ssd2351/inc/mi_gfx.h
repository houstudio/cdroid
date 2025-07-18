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
#ifndef _MI_GFX_H_
#define _MI_GFX_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "mi_common.h"
#include "mi_gfx_datatype.h"

#define GFX_MAJOR_VERSION 3
#define GFX_SUB_VERSION 4
#define MACRO_TO_STR(macro) #macro
#define GFX_VERSION_STR(major_version,sub_version) ({char *tmp = sub_version/100 ? \
                                    "mi_gfx_version_" MACRO_TO_STR(major_version)"." MACRO_TO_STR(sub_version) : sub_version/10 ? \
                                    "mi_gfx_version_" MACRO_TO_STR(major_version)".0" MACRO_TO_STR(sub_version) : \
                                    "mi_gfx_version_" MACRO_TO_STR(major_version)".00" MACRO_TO_STR(sub_version);tmp;})
#define MI_GFX_API_VERSION GFX_VERSION_STR(GFX_MAJOR_VERSION,GFX_SUB_VERSION)

/*-------------------------------------------------------------------------------------------
 * Global Functions
-------------------------------------------------------------------------------------------*/

//-------------------------------------------------------------------------------------------------
/// Open Gfx
/// @param  GfxDevId        \b IN: Gfx device id
/// @return MI_SUCCESS         - Success
/// @return OTHER              - Failure
//------------------------------------------------------------------------------------------------
MI_S32 MI_GFX_Open(MI_GFX_DEV GfxDevId);

//-------------------------------------------------------------------------------------------------
/// Close Gfx
/// @param  GfxDevId        \b IN: Gfx device id
/// @return MI_SUCCESS         - Success
/// @return OTHER              - Failure
//------------------------------------------------------------------------------------------------
MI_S32 MI_GFX_Close(MI_GFX_DEV GfxDevId);

//-------------------------------------------------------------------------------------------------
/// Wait for processing to complete
/// @param  GfxDevId        \b IN: Gfx device id
/// @param  bWaitAllDone    \b IN: Whether to wait for all processing to complete
/// @param  u16TargetFence  \b IN: Wait fence
/// @return MI_SUCCESS         - Success
/// @return OTHER              - Failure
//------------------------------------------------------------------------------------------------
MI_S32 MI_GFX_WaitAllDone(MI_GFX_DEV GfxDevId, MI_BOOL bWaitAllDone, MI_U16 u16TargetFence);

//-------------------------------------------------------------------------------------------------
/// Fill Rect
/// @param GfxDevId         \b IN: Gfx device id
/// @param  pstDst          \b IN: Target surface info
/// @param  pstDstRect      \b IN: Target rect info
/// @param  u32ColorVal     \b IN: Color to fill
///     For all RGB color, the color set as the ARGB8888 format.\n
///     Each color component need to shift to high bit.\n
///     Use ARGB1555 as the example, the source color key as the following:\n
///     ARGB1555  --> ARRRRRGGGGGBBBBB                   (every character represents one bit)\n
///     For I8 format, the index set to b component\n
/// @param  pu16Fence       \b OUT: wait fence
/// @return MI_SUCCESS         - Success
/// @return OTHER              - Failure
//------------------------------------------------------------------------------------------------
MI_S32 MI_GFX_QuickFill(MI_GFX_DEV GfxDevId, MI_GFX_Surface_t *pstDst, MI_GFX_Rect_t *pstDstRect,
    MI_U32 u32ColorVal, MI_U16 *pu16Fence);

//-------------------------------------------------------------------------------------------------
/// Draw line
/// @param  GfxDevId        \b IN: Gfx device id
/// @param  pstDst          \b IN: Target surface info
/// @param  pstLine         \b IN: Line pattern info
/// @param  pu16Fence       \b OUT: wait fence
/// @return MI_SUCCESS         - Success
/// @return OTHER              - Failure
//-------------------------------------------------------------------------------------------------
MI_S32 MI_GFX_DrawLine(MI_GFX_DEV GfxDevId, MI_GFX_Surface_t *pstDst, MI_GFX_Line_t *pstLine, MI_U16 *pu16Fence);

//-------------------------------------------------------------------------------------------------
/// Bit Blit
/// @param  GfxDevId        \b IN: Gfx device id
/// @param  pstSrc          \b IN: Source surface info
/// @param  pstSrcRect      \b IN: Source rect info
/// @param  pstDst          \b IN: Target surface info
/// @param  pstDstRect      \b IN: Target rect info
/// @param  pstOpt          \b IN: Bit blit options
/// @param  pu16Fence       \b OUT: wait fence
/// @return MI_SUCCESS         - Success
/// @return OTHER              - Failure
//-------------------------------------------------------------------------------------------------
MI_S32 MI_GFX_BitBlit(MI_GFX_DEV GfxDevId, MI_GFX_Surface_t *pstSrc, MI_GFX_Rect_t *pstSrcRect,
    MI_GFX_Surface_t *pstDst,  MI_GFX_Rect_t *pstDstRect, MI_GFX_Opt_t *pstOpt, MI_U16 *pu16Fence);

//-------------------------------------------------------------------------------------------------
/// Set Palette for Index Color format(I2/I4/I8)
/// @param  GfxDevId        \b IN: Gfx device id
/// @param  eColorFmt       \b IN: Index color format(I2/I4/I8)
/// @param  pstPalette      \b IN: RGB color data array for corresponding Index Color
/// @return MI_SUCCESS         - Success
/// @return OTHER              - Failure,refer to error code
//------------------------------------------------------------------------------------------------
MI_S32 MI_GFX_SetPalette(MI_GFX_DEV GfxDevId, MI_GFX_ColorFmt_e eColorFmt, MI_GFX_Palette_t* pstPalette);

MI_S32 MI_GFX_CreateDev(MI_GFX_DEV GfxDevId, MI_GFX_DevAttr_t *pstDevAttr);
MI_S32 MI_GFX_DestroyDev(MI_GFX_DEV GfxDevId);

//-------------------------------------------------------------------------------------------------
/// Get ARGB8888 to ARGB1555 alpha threshold
/// @param  GfxDevId            \b IN: Gfx device id
/// @param  pu8ThresholdValue   \b OUT: ARGB1555 alpha threshold
/// @return MI_SUCCESS             - Success
/// @return OTHER                  - Failure,refer to error code
//------------------------------------------------------------------------------------------------
MI_S32 MI_GFX_GetARGB8888To1555AlphaThreshold(MI_GFX_DEV GfxDevId, MI_U8 *pu8ThresholdValue);

//-------------------------------------------------------------------------------------------------
/// Set ARGB8888 to ARGB1555 alpha threshold
/// This alpha threshold is used when ARGB8888 is converted to ARGB1555.
/// When ARGB8888's alpha is greater than or equal to the alpha threshold, ARGB1555's alpha is set to 1, otherwise 0.
/// @param  GfxDevId            \b IN: Gfx device id
/// @param  u8ThresholdValue    \b IN: ARGB1555 alpha threshold
/// @return MI_SUCCESS             - Success
/// @return OTHER                  - Failure,refer to error code
//------------------------------------------------------------------------------------------------
MI_S32 MI_GFX_SetARGB8888To1555AlphaThreshold(MI_GFX_DEV GfxDevId, MI_U8 u8ThresholdValue);

//-------------------------------------------------------------------------------------------------
/// Get ARGB1555 to ARGB8888 alpha
/// @param  GfxDevId            \b IN: Gfx device id
/// @param  pu8AlphaValue       \b OUT: ARGB1555 alpha
/// @return MI_SUCCESS             - Success
/// @return OTHER                  - Failure,refer to error code
//------------------------------------------------------------------------------------------------
MI_S32 MI_GFX_GetARGB1555To8888AlphaValue(MI_GFX_DEV GfxDevId, MI_U8 *pu8AlphaValue);

//-------------------------------------------------------------------------------------------------
/// Set ARGB1555 to ARGB8888 alpha
/// This alpha value is used when ARGB1555 is converted to ARGB8888.
/// When ARGB1555's alpha is 1, ARGB8888's alpha is set to this value, otherwise 0.
/// @param  GfxDevId            \b IN: Gfx device id
/// @param  u8AlphaValue        \b IN: ARGB1555 alpha
/// @return MI_SUCCESS             - Success
/// @return OTHER                  - Failure,refer to error code
//------------------------------------------------------------------------------------------------
MI_S32 MI_GFX_SetARGB1555To8888AlphaValue(MI_GFX_DEV GfxDevId, MI_U8 u8AlphaValue);

//-------------------------------------------------------------------------------------------------
/// Get scaling coefficient
/// @param  GfxDevId                        \b IN: Gfx device id
/// @param  pu16ScalingCoefficient          \b OUT: scaling coefficient = *pfScalingCoefficient / 1000
/// @return MI_SUCCESS                         - Success
/// @return OTHER                              - Failure,refer to error code
//------------------------------------------------------------------------------------------------
MI_S32 MI_GFX_GetInitialScalingCoeff(MI_GFX_DEV GfxDevId, MI_U16 *pu16ScalingCoefficient);

//-------------------------------------------------------------------------------------------------
/// Set scaling coefficient
/// @param  GfxDevId                        \b IN: Gfx device id
/// @param  u16ScalingCoefficient           \b IN: scaling coefficient = u16ScalingCoefficient / 1000
/// @return MI_SUCCESS                         - Success
/// @return OTHER                              - Failure,refer to error code
//------------------------------------------------------------------------------------------------
MI_S32 MI_GFX_SetInitialScalingCoeff(MI_GFX_DEV GfxDevId, MI_U16 u16ScalingCoefficient);

#ifdef __cplusplus
}
#endif

#endif //_MI_GFX_H_
