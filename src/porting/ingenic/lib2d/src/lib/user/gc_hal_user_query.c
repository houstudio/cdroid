/****************************************************************************
*
*    Copyright (c) 2005 - 2010 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************
*
*    Auto-generated file on 11/10/2010. Do not edit!!!
*
*****************************************************************************/




/**
**	@file
**	Architecture independent capability query functions.
**
*/

#include "gc_hal_user_precomp.h"

#define _GC_OBJ_ZONE		gcvZONE_SURFACE

gcsFORMAT_COMPONENT gcvPIXEL_COMP_XXX8 = {  0, 8 };
gcsFORMAT_COMPONENT gcvPIXEL_COMP_XX8X = {  8, 8 };
gcsFORMAT_COMPONENT gcvPIXEL_COMP_X8XX = { 16, 8 };
gcsFORMAT_COMPONENT gcvPIXEL_COMP_8XXX = { 24, 8 };

static struct _gcsSURF_FORMAT_INFO FormatInfoArray[] =
{
	/*
		Palettized formats (gcsFORMAT_CLASS_TYPE_INDEX).
	*/
	{gcvSURF_INDEX1, gcvFORMAT_CLASS_INDEX, 1, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			{0, 1}
		}}
	},
	{gcvSURF_INDEX4, gcvFORMAT_CLASS_INDEX, 4, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			{0, 4}
		}}
	},
	{gcvSURF_INDEX8, gcvFORMAT_CLASS_INDEX, 8, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			{0, 8}
		}}
	},

	/*
		RGB formats (gcsFORMAT_CLASS_TYPE_RGBA).
	*/
	{gcvSURF_A2R2G2B2, gcvFORMAT_CLASS_RGBA, 8, gcvSURF_ARGB, gcvFALSE,
		{{
			/* A */  {6,  2},
			/* R */  {4,  2},
			/* G */  {2,  2},
			/* B */  {0,  2}
		}}
	},
	{gcvSURF_R3G3B2, gcvFORMAT_CLASS_RGBA, 8, gcvSURF_ARGB, gcvFALSE,
		{{
			/* A */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* R */  {5,  3},
			/* G */  {2,  3},
			/* B */  {0,  2}
		}}
	},
	{gcvSURF_A8R3G3B2, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_ARGB, gcvFALSE,
		{{
			/* A */  {8,  8},
			/* R */  {5,  3},
			/* G */  {2,  3},
			/* B */  {0,  2}
		}}
	},
	{gcvSURF_X4R4G4B4, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_ARGB, gcvFALSE,
		{{
			/* A */ {12,  4 | gcvCOMPONENT_DONTCARE},
			/* R */  {8,  4},
			/* G */  {4,  4},
			/* B */  {0,  4}
		}}
	},
	{gcvSURF_A4R4G4B4, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_ARGB, gcvFALSE,
		{{
			/* A */ {12,  4},
			/* R */  {8,  4},
			/* G */  {4,  4},
			/* B */  {0,  4}
		}}
	},
	{gcvSURF_R4G4B4A4, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_RGBA, gcvFALSE,
		{{
			/* A */  {0,  4},
			/* R */ {12,  4},
			/* G */  {8,  4},
			/* B */  {4,  4}
		}}
	},
	{gcvSURF_X1R5G5B5, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_ARGB, gcvFALSE,
		{{
			/* A */ {15,  1 | gcvCOMPONENT_DONTCARE},
			/* R */ {10,  5},
			/* G */  {5,  5},
			/* B */  {0,  5}
		}}
	},
	{gcvSURF_A1R5G5B5, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_ARGB, gcvFALSE,
		{{
			/* A */ {15,  1},
			/* R */ {10,  5},
			/* G */  {5,  5},
			/* B */  {0,  5}
		}}
	},
	{gcvSURF_R5G5B5A1, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_RGBA, gcvFALSE,
		{{
			/* A */  {0,  1},
			/* R */ {11,  5},
			/* G */  {6,  5},
			/* B */  {1,  5},
		}}
	},
	{gcvSURF_R5G6B5, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_ARGB, gcvFALSE,
		{{
			/* A */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* R */ {11,  5},
			/* G */  {5,  6},
			/* B */  {0,  5}
		}}
	},
	{gcvSURF_R8G8B8, gcvFORMAT_CLASS_RGBA, 24, gcvSURF_ARGB, gcvFALSE,
		{{
			/* A */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* R */ {16,  8},
			/* G */  {8,  8},
			/* B */  {0,  8}
		}}
	},
	{gcvSURF_X8R8G8B8, gcvFORMAT_CLASS_RGBA, 32, gcvSURF_ARGB, gcvFALSE,
		{{
			/* A */ {24,  8 | gcvCOMPONENT_DONTCARE},
			/* R */ {16,  8},
			/* G */  {8,  8},
			/* B */  {0,  8}
		}}
	},
	{gcvSURF_R8G8B8X8, gcvFORMAT_CLASS_RGBA, 32, gcvSURF_RGBA, gcvFALSE,
		{{
			/* A */  {0,  8 | gcvCOMPONENT_DONTCARE},
			/* R */ {24,  8},
			/* G */ {16,  8},
			/* B */  {8,  8}
		}}
	},
	{gcvSURF_A8R8G8B8, gcvFORMAT_CLASS_RGBA, 32, gcvSURF_ARGB, gcvFALSE,
		{{
			/* A */ {24,  8},
			/* R */ {16,  8},
			/* G */  {8,  8},
			/* B */  {0,  8}
		}}
	},
	{gcvSURF_R8G8B8A8, gcvFORMAT_CLASS_RGBA, 32, gcvSURF_RGBA, gcvFALSE,
		{{
			/* A */  {0,  8},
			/* R */ {24,  8},
			/* G */ {16,  8},
			/* B */  {8,  8}
		}}
	},
	{gcvSURF_G8R8G8B8, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_ARGB, gcvTRUE,
		{{
			/* A */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* R */ {16,  8},
			/* G */  {8,  8},
			/* B */  {0,  8}
		}}
	},
	{gcvSURF_G8R8G8B8, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_ARGB, gcvTRUE | gcvCOMPONENT_ODD,
		{{
			/* A */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* R */ {16,  8},
			/* G */ {24,  8},
			/* B */  {0,  8}
		}}
	},
	{gcvSURF_R8G8B8G8, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_RGBA, gcvTRUE,
		{{
			/* A */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* R */ {24,  8},
			/* G */  {0,  8},
			/* B */  {8,  8}
		}}
	},
	{gcvSURF_R8G8B8G8, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_RGBA, gcvTRUE | gcvCOMPONENT_ODD,
		{{
			/* A */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* R */ {24,  8},
			/* G */ {16,  8},
			/* B */  {8,  8}
		}}
	},
	{gcvSURF_X2R10G10B10, gcvFORMAT_CLASS_RGBA, 32, gcvSURF_ARGB, gcvFALSE,
		{{
			/* A */ {30,  2 | gcvCOMPONENT_DONTCARE},
			/* R */ {20, 10},
			/* G */ {10, 10},
			/* B */  {0, 10}
		}}
	},
	{gcvSURF_A2R10G10B10, gcvFORMAT_CLASS_RGBA, 32, gcvSURF_ARGB, gcvFALSE,
		{{
			/* A */ {30,  2},
			/* R */ {20, 10},
			/* G */ {10, 10},
			/* B */  {0, 10}
		}}
	},
	{gcvSURF_X12R12G12B12, gcvFORMAT_CLASS_RGBA, 48, gcvSURF_ARGB, gcvFALSE,
		{{
			/* A */ {36, 12 | gcvCOMPONENT_DONTCARE},
			/* R */ {24, 12},
			/* G */ {12, 12},
			/* B */  {0, 12}
		}}
	},
	{gcvSURF_A12R12G12B12, gcvFORMAT_CLASS_RGBA, 48, gcvSURF_ARGB, gcvFALSE,
		{{
			/* A */ {36, 12},
			/* R */ {24, 12},
			/* G */ {12, 12},
			/* B */  {0, 12}
		}}
	},
	{gcvSURF_X16R16G16B16, gcvFORMAT_CLASS_RGBA, 64, gcvSURF_ARGB, gcvFALSE,
		{{
			/* A */ {48, 16 | gcvCOMPONENT_DONTCARE},
			/* R */ {32, 16},
			/* G */ {16, 16},
			/* B */  {0, 16}
		}}
	},
	{gcvSURF_A16R16G16B16, gcvFORMAT_CLASS_RGBA, 64, gcvSURF_ARGB, gcvFALSE,
		{{
			/* A */ {48, 16},
			/* R */ {32, 16},
			/* G */ {16, 16},
			/* B */  {0, 16}
		}}
	},
	{gcvSURF_R5G5B5X1, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_RGBA, gcvFALSE,
		{{
			/* A */ { 0,  1 | gcvCOMPONENT_DONTCARE},
			/* R */ {11,  5},
			/* G */  {6,  5},
			/* B */  {1,  5}
		}}
	},
	{gcvSURF_R4G4B4X4, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_RGBA, gcvFALSE,
		{{
			/* A */ { 0,  4 | gcvCOMPONENT_DONTCARE},
			/* R */ {12,  4},
			/* G */  {8,  4},
			/* B */  {4,  4}
		}}
	},
	{gcvSURF_R8G8B8X8, gcvFORMAT_CLASS_RGBA, 32, gcvSURF_RGBA, gcvFALSE,
		{{
			/* A */ { 0,  8 | gcvCOMPONENT_DONTCARE},
			/* R */ {24,  8},
			/* G */ {16,  8},
			/* B */  {8,  8}
		}}
	},

	/*
		BGR formats (gcsFORMAT_CLASS_TYPE_RGBA).
	*/
	{gcvSURF_A4B4G4R4, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_ABGR, gcvFALSE,
		{{
			/* A */ {12,  4},
			/* R */  {0,  4},
			/* G */  {4,  4},
			/* B */  {8,  4}
		}}
	},
	{gcvSURF_B4G4R4A4, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_BGRA, gcvFALSE,
		{{
			/* A */  {0,  4},
			/* R */  {4,  4},
			/* G */  {8,  4},
			/* B */ {12,  4}
		}}
	},
	{gcvSURF_A1B5G5R5, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_ABGR, gcvFALSE,
		{{
			/* A */ {15,  1},
			/* R */  {0,  5},
			/* G */  {5,  5},
			/* B */ {10,  5}
		}}
	},
	{gcvSURF_B5G5R5A1, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_BGRA, gcvFALSE,
		{{
			/* A */  {0,  1},
			/* R */  {1,  5},
			/* G */  {6,  5},
			/* B */ {11,  5}
		}}
	},
	{gcvSURF_B5G6R5, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_ABGR, gcvFALSE,
		{{
			/* A */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* R */  {0,  5},
			/* G */  {5,  6},
			/* B */ {11,  5}
		}}
	},
	{gcvSURF_B8G8R8, gcvFORMAT_CLASS_RGBA, 24, gcvSURF_ABGR, gcvFALSE,
		{{
			/* A */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* R */  {0,  8},
			/* G */  {8,  8},
			/* B */ {16,  8}
		}}
	},
	{gcvSURF_X8B8G8R8, gcvFORMAT_CLASS_RGBA, 32, gcvSURF_ABGR, gcvFALSE,
		{{
			/* A */ {24,  8 | gcvCOMPONENT_DONTCARE},
			/* R */  {0,  8},
			/* G */  {8,  8},
			/* B */ {16,  8}
		}}
	},
	{gcvSURF_B8G8R8X8, gcvFORMAT_CLASS_RGBA, 32, gcvSURF_BGRA, gcvFALSE,
		{{
			/* A */  {0,  8 | gcvCOMPONENT_DONTCARE},
			/* R */  {8,  8},
			/* G */ {16,  8},
			/* B */ {24,  8}
		}}
	},
	{gcvSURF_A8B8G8R8, gcvFORMAT_CLASS_RGBA, 32, gcvSURF_ABGR, gcvFALSE,
		{{
			/* A */ {24,  8},
			/* R */  {0,  8},
			/* G */  {8,  8},
			/* B */ {16,  8}
		}}
	},
	{gcvSURF_B8G8R8A8, gcvFORMAT_CLASS_RGBA, 32, gcvSURF_BGRA, gcvFALSE,
		{{
			/* A */  {0,  8},
			/* R */  {8,  8},
			/* G */ {16,  8},
			/* B */ {24,  8}
		}}
	},
	{gcvSURF_A2B10G10R10, gcvFORMAT_CLASS_RGBA, 32, gcvSURF_ABGR, gcvFALSE,
		{{
			/* A */ {30,  2},
			/* R */  {0, 10},
			/* G */ {10, 10},
			/* B */ {20, 10}
		}}
	},
	{gcvSURF_A16B16G16R16, gcvFORMAT_CLASS_RGBA, 64, gcvSURF_ABGR, gcvFALSE,
		{{
			/* A */ {48, 16},
			/* R */  {0, 16},
			/* G */ {16, 16},
			/* B */ {32, 16}
		}}
	},
	{gcvSURF_G16R16, gcvFORMAT_CLASS_RGBA, 32, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* A */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* R */  {0, 16},
			/* G */ {16, 16},
			/* B */  {0,  gcvCOMPONENT_NOTPRESENT}
		}}
	},
	{gcvSURF_B4G4R4A4, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_BGRA, gcvFALSE,
		{{
			/* A */ { 0,  4},
			/* R */ { 4,  4},
			/* G */ { 8,  4},
			/* B */ {12,  4}
		}}
	},
	{gcvSURF_B5G5R5A1, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_BGRA, gcvFALSE,
		{{
			/* A */ { 0,  1},
			/* R */ { 1,  5},
			/* G */ { 6,  5},
			/* B */ {11,  5}
		}}
	},
	{gcvSURF_B8G8R8X8, gcvFORMAT_CLASS_RGBA, 32, gcvSURF_BGRA, gcvFALSE,
		{{
			/* A */ { 0,  8 | gcvCOMPONENT_DONTCARE},
			/* R */  {8,  8},
			/* G */ {16,  8},
			/* B */ {24,  8}
		}}
	},
	{gcvSURF_B8G8R8A8, gcvFORMAT_CLASS_RGBA, 32, gcvSURF_BGRA, gcvFALSE,
		{{
			/* A */ { 0,  8},
			/* R */  {8,  8},
			/* G */ {16,  8},
			/* B */ {24,  8}
		}}
	},
	{gcvSURF_X4B4G4R4, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_ABGR, gcvFALSE,
		{{
			/* A */ {12,  4 | gcvCOMPONENT_DONTCARE},
			/* R */  {0,  4},
			/* G */  {4,  4},
			/* B */  {8,  4}
		}}
	},
	{gcvSURF_X1B5G5R5, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_ABGR, gcvFALSE,
		{{
			/* A */ {15,  1 | gcvCOMPONENT_DONTCARE},
			/* R */  {0,  5},
			/* G */  {5,  5},
			/* B */ {10,  5}
		}}
	},
	{gcvSURF_B4G4R4X4, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_BGRA, gcvFALSE,
		{{
			/* A */  {0,  4 | gcvCOMPONENT_DONTCARE},
			/* R */  {4,  4},
			/* G */  {8,  4},
			/* B */ {12,  4}
		}}
	},
	{gcvSURF_B5G5R5X1, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_BGRA, gcvFALSE,
		{{
			/* A */  {0,  1 | gcvCOMPONENT_DONTCARE},
			/* R */  {1,  5},
			/* G */  {6,  5},
			/* B */ {11,  5}
		}}
	},

	/*
		YUV formats (gcsFORMAT_CLASS_TYPE_YUV).
	*/
	{gcvSURF_YUY2, gcvFORMAT_CLASS_YUV, 16, gcvSURF_NOSWIZZLE, gcvTRUE,
		{{
			/* Y */  {0,  8},
			/* U */  {8,  8},
			/* V */ {24,  8}
		}}
	},
	{gcvSURF_YUY2, gcvFORMAT_CLASS_YUV, 16, gcvSURF_NOSWIZZLE, gcvTRUE | gcvCOMPONENT_ODD,
		{{
			/* Y */ {16,  8},
			/* U */  {8,  8},
			/* V */ {24,  8}
		}}
	},
	{gcvSURF_UYVY, gcvFORMAT_CLASS_YUV, 16, gcvSURF_NOSWIZZLE, gcvTRUE,
		{{
			/* Y */  {8,  8},
			/* U */  {0,  8},
			/* V */ {16,  8}
		}}
	},
	{gcvSURF_UYVY, gcvFORMAT_CLASS_YUV, 16, gcvSURF_NOSWIZZLE, gcvTRUE | gcvCOMPONENT_ODD,
		{{
			/* Y */ {24,  8},
			/* U */  {0,  8},
			/* V */ {16,  8}
		}}
	},
	{gcvSURF_YV12, gcvFORMAT_CLASS_YUV, 12, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Y */ {0,  8},
			/* U */ {0,  8},
			/* V */ {0,  8}
		}}
	},
	{gcvSURF_I420, gcvFORMAT_CLASS_YUV, 12, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Y */ {0,  8},
			/* U */ {0,  8},
			/* V */ {0,  8}
		}}
	},
	{gcvSURF_NV12, gcvFORMAT_CLASS_YUV, 12, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Y */ {0,  8},
			/* U */ {0,  8},
			/* V */ {0,  8}
		}}
	},
	{gcvSURF_NV21, gcvFORMAT_CLASS_YUV, 12, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Y */ {0,  8},
			/* U */ {0,  8},
			/* V */ {0,  8}
		}}
	},
	{gcvSURF_NV16, gcvFORMAT_CLASS_YUV, 12, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Y */ {0,  8},
			/* U */ {0,  8},
			/* V */ {0,  8}
		}}
	},
	{gcvSURF_NV61, gcvFORMAT_CLASS_YUV, 12, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Y */ {0,  8},
			/* U */ {0,  8},
			/* V */ {0,  8}
		}}
	},
	{gcvSURF_YVYU, gcvFORMAT_CLASS_YUV, 16, gcvSURF_NOSWIZZLE, gcvTRUE,
		{{
			/* Y */  {0,  8},
			/* U */	{24,  8},
			/* V */  {8,  8}
		}}
	},
	{gcvSURF_YVYU, gcvFORMAT_CLASS_YUV, 16, gcvSURF_NOSWIZZLE, gcvTRUE | gcvCOMPONENT_ODD,
		{{
			/* Y */ {16,  8},
			/* U */	{24,  8},
			/* V */  {8,  8}
		}}
	},
	{gcvSURF_VYUY, gcvFORMAT_CLASS_YUV, 16, gcvSURF_NOSWIZZLE, gcvTRUE,
		{{
			/* Y */  {8,  8},
			/* U */ {16,  8},
			/* V */  {0,  8}
		}}
	},
	{gcvSURF_VYUY, gcvFORMAT_CLASS_YUV, 16, gcvSURF_NOSWIZZLE, gcvTRUE | gcvCOMPONENT_ODD,
		{{
			/* Y */ {24,  8},
			/* U */ {16,  8},
			/* V */  {0,  8}
		}}
	},

	/*
		Alpha formats (gcsFORMAT_CLASS_TYPE_RGBA).
	*/
	{gcvSURF_A1, gcvFORMAT_CLASS_RGBA, 1, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* A */  {0,  1},
			/* R */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* G */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* B */  {0,  gcvCOMPONENT_NOTPRESENT}
		}}
	},
	{gcvSURF_A4, gcvFORMAT_CLASS_RGBA, 4, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* A */  {0,  4},
			/* R */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* G */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* B */  {0,  gcvCOMPONENT_NOTPRESENT}
		}}
	},
	{gcvSURF_A8, gcvFORMAT_CLASS_RGBA, 8, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* A */  {0,  8},
			/* R */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* G */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* B */  {0,  gcvCOMPONENT_NOTPRESENT}
		}}
	},
	{gcvSURF_A12, gcvFORMAT_CLASS_RGBA, 12, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* A */  {0, 12},
			/* R */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* G */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* B */  {0,  gcvCOMPONENT_NOTPRESENT}
		}}
	},
	{gcvSURF_A16, gcvFORMAT_CLASS_RGBA, 16, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* A */  {0, 16},
			/* R */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* G */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* B */  {0,  gcvCOMPONENT_NOTPRESENT}
		}}
	},
	{gcvSURF_A32, gcvFORMAT_CLASS_RGBA, 32, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* A */  {0, 32},
			/* R */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* G */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* B */  {0,  gcvCOMPONENT_NOTPRESENT}
		}}
	},

	/*
		Luminance formats (gcsFORMAT_CLASS_TYPE_LUMINANCE).
	*/
	{gcvSURF_L1, gcvFORMAT_CLASS_LUMINANCE, 1, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Alpha */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* Lum   */  {0,  1}
		}}
	},
	{gcvSURF_L4, gcvFORMAT_CLASS_LUMINANCE, 4, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Alpha */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* Lum   */  {0,  4}
		}}
	},
	{gcvSURF_L8, gcvFORMAT_CLASS_LUMINANCE, 8, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Alpha */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* Lum   */  {0,  8}
		}}
	},
	{gcvSURF_L12, gcvFORMAT_CLASS_LUMINANCE, 12, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Alpha */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* Lum   */  {0, 12}
		}}
	},
	{gcvSURF_L16, gcvFORMAT_CLASS_LUMINANCE, 16, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Alpha */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* Lum   */  {0, 16}
		}}
	},
	{gcvSURF_L32, gcvFORMAT_CLASS_LUMINANCE, 32, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Alpha */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* Lum   */  {0, 32}
		}}
	},
	{gcvSURF_A4L4, gcvFORMAT_CLASS_LUMINANCE, 8, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Alpha */  {4,  4},
			/* Lum   */  {0,  4}
		}}
	},
	{gcvSURF_A2L6, gcvFORMAT_CLASS_LUMINANCE, 8, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Alpha */  {6,  2},
			/* Lum   */  {0,  6}
		}}
	},
	{gcvSURF_A8L8, gcvFORMAT_CLASS_LUMINANCE, 16, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Alpha */  {8,  8},
			/* Lum   */  {0,  8}
		}}
	},
	{gcvSURF_A4L12, gcvFORMAT_CLASS_LUMINANCE, 16, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Alpha */ {12,  4},
			/* Lum   */  {0, 12}
		}}
	},
	{gcvSURF_A12L12, gcvFORMAT_CLASS_LUMINANCE, 24, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Alpha */ {12, 12},
			/* Lum   */  {0, 12}
		}}
	},
	{gcvSURF_A16L16, gcvFORMAT_CLASS_LUMINANCE, 32, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Alpha */ {16, 16},
			/* Lum   */  {0, 16}
		}}
	},

	/*
		Bump formats (gcsFORMAT_CLASS_TYPE_BUMP).
	*/
	{gcvSURF_L6V5U5, gcvFORMAT_CLASS_LUMINANCE, 16, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Alpha */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* L     */ {10,  6},
			/* V     */  {5,  5},
			/* U     */  {0,  5},
			/* Q     */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* W     */  {0,  gcvCOMPONENT_NOTPRESENT}
		}}
	},
	{gcvSURF_V8U8, gcvFORMAT_CLASS_LUMINANCE, 16, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Alpha */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* L     */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* V     */  {8,  8},
			/* U     */  {0,  8},
			/* Q     */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* W     */  {0,  gcvCOMPONENT_NOTPRESENT}
		}}
	},
	{gcvSURF_X8L8V8U8, gcvFORMAT_CLASS_LUMINANCE, 32, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Alpha */ {24,  8 | gcvCOMPONENT_DONTCARE},
			/* L     */ {16,  8},
			/* V     */  {8,  8},
			/* U     */  {0,  8},
			/* Q     */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* W     */  {0,  gcvCOMPONENT_NOTPRESENT}
		}}
	},
	{gcvSURF_Q8W8V8U8, gcvFORMAT_CLASS_LUMINANCE, 32, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Alpha */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* L     */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* V     */  {8,  8},
			/* U     */  {0,  8},
			/* Q     */ {24,  8},
			/* W     */ {16,  8}
		}}
	},
	{gcvSURF_A2W10V10U10, gcvFORMAT_CLASS_LUMINANCE, 32, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Alpha */ {30,  2},
			/* L     */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* V     */ {10, 10},
			/* U     */  {0, 10},
			/* Q     */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* W     */ {20, 10}
		}}
	},
	{gcvSURF_V16U16, gcvFORMAT_CLASS_LUMINANCE, 32, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Alpha */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* L     */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* V     */ {16, 16},
			/* U     */  {0, 16},
			/* Q     */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* W     */  {0,  gcvCOMPONENT_NOTPRESENT}
		}}
	},
	{gcvSURF_Q16W16V16U16, gcvFORMAT_CLASS_LUMINANCE, 64, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Alpha */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* L     */  {0,  gcvCOMPONENT_NOTPRESENT},
			/* V     */ {16, 16},
			/* U     */  {0, 16},
			/* Q     */ {48, 16},
			/* W     */ {32, 16}
		}}
	},

	/*
		Depth and stencil formats (gcsFORMAT_CLASS_TYPE_DEPTH).
	*/
	{ gcvSURF_D16, gcvFORMAT_CLASS_DEPTH, 16, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Depth   */ { 0, 16                      },
			/* Stencil */ { 0, gcvCOMPONENT_NOTPRESENT },
		}}
	},
	{ gcvSURF_D24S8, gcvFORMAT_CLASS_DEPTH, 32, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Depth   */ { 8, 24 },
			/* Stencil */ { 0,  8 },
		}}
	},
	{ gcvSURF_D32, gcvFORMAT_CLASS_DEPTH, 32, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Depth   */ { 0, 32 },
			/* Stencil */ { 0, gcvCOMPONENT_NOTPRESENT },
		}}
	},
	{ gcvSURF_D24X8, gcvFORMAT_CLASS_DEPTH, 32, gcvSURF_NOSWIZZLE, gcvFALSE,
		{{
			/* Depth   */ { 8, 24 },
			/* Stencil */ { 0,  gcvCOMPONENT_NOTPRESENT },
		}}
	},
};

/*******************************************************************************
**
**	gcoSURF_QueryFormat
**
**	Return pixel format parameters.
**
**	INPUT:
**
**		gceSURF_FORMAT Format
**			API format.
**
**	OUTPUT:
**
**		gcsSURF_FORMAT_INFO_PTR * Info
**			Pointer to a variable that will hold the format description entry.
**			If the format in question is interleaved, two pointers will be
**			returned stored in an array fation.
**
*/
gceSTATUS
gcoSURF_QueryFormat(
	IN gceSURF_FORMAT Format,
	OUT gcsSURF_FORMAT_INFO_PTR * Info
	)
{
	gctUINT i;

	gcmHEADER_ARG("Format=%d", Format);

	/* Verify the arguments. */
	gcmVERIFY_ARGUMENT(Info != gcvNULL);

	/* Find the format in the format table. */
	for (i = 0; i < gcmCOUNTOF(FormatInfoArray); i++)
	{
		if (FormatInfoArray[i].format == Format)
		{
			/* Set the output to the current entry. */
			Info[0] = &FormatInfoArray[i];
			Info[1] = FormatInfoArray[i].interleaved
				? &FormatInfoArray[i + 1]
				: gcvNULL;

			/* Success. */
			gcmFOOTER_ARG("Info=0x%x,0x%x", Info[0], Info[1]);
			return gcvSTATUS_OK;
		}

		if (FormatInfoArray[i].interleaved)
		{
			i++;
		}
	}

	/* Failed to find the entry. */
	gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
	return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
**
**	gcoSURF_ComputeColorMask
**
**	Computes the color pixel mask; for RGB formats alpha channel is not
**	included in the mask.
**
**	INPUT:
**
**		gcsSURF_FORMAT_INFO_PTR Format
**			Pointer to the format information structure.
**
**	OUTPUT:
**
**		gctUINT32_PTR ColorMask
**			Pixel color mask.
*/
gceSTATUS
gcoSURF_ComputeColorMask(
	IN gcsSURF_FORMAT_INFO_PTR Format,
	OUT gctUINT32_PTR ColorMask
	)
{
	gcmHEADER_ARG("Format=0x%x,0x%x", Format[0], Format[1]);

	/* Verify the arguments. */
	gcmVERIFY_ARGUMENT(Format != gcvNULL);
	gcmVERIFY_ARGUMENT(ColorMask != gcvNULL);

	/* Commpute source pixel mask. */
	if (Format[0].fmtClass == gcvFORMAT_CLASS_RGBA)
	{
		/* Reset the mask. */
		*ColorMask = 0;

		/* Add components. */
		if (Format[0].u.rgba.red.width)
		{
			*ColorMask
				|= ((1 << Format[0].u.rgba.red.width) - 1)
				<< Format[0].u.rgba.red.start;
		}

		if (Format[0].u.rgba.green.width)
		{
			*ColorMask
				|= ((1 << Format[0].u.rgba.green.width) - 1)
				<< Format[0].u.rgba.green.start;
		}

		if (Format[0].u.rgba.blue.width)
		{
			*ColorMask
				|= ((1 << Format[0].u.rgba.blue.width) - 1)
				<< Format[0].u.rgba.blue.start;
		}
	}
	else
	{
		*ColorMask = (1 << Format[0].bitsPerPixel) - 1;
	}

	/* Success. */
	gcmFOOTER_ARG("*ColorMask=%08x", *ColorMask);
	return gcvSTATUS_OK;
}


