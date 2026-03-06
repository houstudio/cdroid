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




#include "gc_hal_user_hardware_precomp.h"

#if defined (FP_PRESENT)

/*******************************************************************************
** Conversion macros.
*/

#define gcmCONVERTFLOAT(Value) \
	(* (gctUINT32_PTR) &Value)

#define gcmCONVERTFIXED(Value) \
	Value


/*******************************************************************************
** Texture function codes.
*/

static gctUINT32 _TextureFunction[] =
{
	FP_STAGE_FUNCTION_REPLACE,			/* gcvTEXTURE_REPLACE     */
	FP_STAGE_FUNCTION_MODULATE,			/* gcvTEXTURE_MODULATE    */
	FP_STAGE_FUNCTION_ADD,				/* gcvTEXTURE_ADD         */
	FP_STAGE_FUNCTION_ADD_SIGNED,		/* gcvTEXTURE_ADD_SIGNED  */
	FP_STAGE_FUNCTION_INTERPOLATE,		/* gcvTEXTURE_INTERPOLATE */
	FP_STAGE_FUNCTION_SUBTRACT,			/* gcvTEXTURE_SUBTRACT    */
	FP_STAGE_FUNCTION_DOT3				/* gcvTEXTURE_DOT3        */
};

static gctUINT32 _TextureSource[] =
{
	FP_STAGE_SOURCE_TEXTURE,			/* gcvCOLOR_FROM_TEXTURE        */
	FP_STAGE_SOURCE_CONSTANT,			/* gcvCOLOR_FROM_CONSTANT_COLOR */
	FP_STAGE_SOURCE_COLOR,				/* gcvCOLOR_FROM_PRIMARY_COLOR  */
	FP_STAGE_SOURCE_PREVIOUS			/* gcvCOLOR_FROM_PREVIOUS_COLOR */
};

static gctUINT32 _TextureChannel[] =
{
	/* gcvFROM_COLOR */
	FP_STAGE_SOURCE_COLOR_CHANNELS,

	/* gcvFROM_ONE_MINUS_COLOR */
	FP_STAGE_SOURCE_COLOR_CHANNELS | FP_STAGE_SOURCE_INVERSED,

	/* gcvFROM_ALPHA */
	FP_STAGE_SOURCE_ALPHA_CHANNEL,

	/* gcvFROM_ONE_MINUS_ALPHA */
	FP_STAGE_SOURCE_ALPHA_CHANNEL  | FP_STAGE_SOURCE_INVERSED
};


/*******************************************************************************
**
**	_ConvertColor
**
**	Color conversion functions.
**
**	INPUT:
**
**      Red, Green, Blue, Alpha
**          Color components to convert.
**
**	OUTPUT:
**
**		RGBA8 value.
*/
static gctUINT32
_GetSource(
	IN gceTEXTURE_SOURCE Source,
	IN gceTEXTURE_CHANNEL Channel
	)
{
	gctUINT32 source = FP_STAGE_SOURCE_OFF;

	if ((Source  >= 0) && (Source  < gcmCOUNTOF(_TextureSource)) &&
		(Channel >= 0) && (Channel < gcmCOUNTOF(_TextureChannel)))
	{
		source
			= _TextureSource[Source]
			| _TextureChannel[Channel]
			| FP_STAGE_SOURCE_ON;
	}

	return source;
}


/*******************************************************************************
**
**	_SetTextureFunction
**
**	Configure texture function.
**
**	INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**		gctUINT32 Address
**			Register address.
**
**		gceTEXTURE_FUNCTION Function
**			Texture function.
**
**		gceTEXTURE_SOURCE Source0, Source1, Source2
**			The source of the value for the function.
**
**		gceTEXTURE_CHANNEL Channel0, Channel1, Channel2
**			Determines whether the value comes from the color, alpha channel;
**			straight or inversed.
**
**		gctINT Scale
**			Result scale value.
**
**	OUTPUT:
**
**		Nothing.
*/
static gceSTATUS
_SetTextureFunction(
	IN gcoHARDWARE Hardware,
	IN gctUINT32 Address,
	IN gceTEXTURE_FUNCTION Function,
	IN gceTEXTURE_SOURCE Source0,
	IN gceTEXTURE_CHANNEL Channel0,
	IN gceTEXTURE_SOURCE Source1,
	IN gceTEXTURE_CHANNEL Channel1,
	IN gceTEXTURE_SOURCE Source2,
	IN gceTEXTURE_CHANNEL Channel2,
	IN gctINT Scale
	)
{
	gctUINT32 config;
	gctUINT32 function;
	gctUINT32 source0;
	gctUINT32 source1;
	gctUINT32 source2;
	gctUINT32 shift;

	/* Translate the texture function. */
	if ((Function < 0) || (Function >= gcmCOUNTOF(_TextureFunction)))
	{
		return gcvSTATUS_INVALID_ARGUMENT;
	}

	function = _TextureFunction[Function];

	/* Determine the sources. */
	source0 = _GetSource(Source0, Channel0);
	source1 = _GetSource(Source1, Channel1);
	source2 = _GetSource(Source2, Channel2);

	/* Determine the shift. */
	switch (Scale)
	{
	case 1:
		shift = 0;
		break;

	case 2:
		shift = 1;
		break;

	case 4:
		shift = 2;
		break;

	default:
		return gcvSTATUS_INVALID_ARGUMENT;
	}

	/* Determine the register value. */
	config
		= (    ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? FP_STAGE_FUNCTION) - (0 ? FP_STAGE_FUNCTION) + 1) == 32) ? ~0 : (~(~0 << ((1 ? FP_STAGE_FUNCTION) - (0 ? FP_STAGE_FUNCTION) + 1))))))) << (0 ? FP_STAGE_FUNCTION))) | (((gctUINT32) ((gctUINT32) ((function)) & ((gctUINT32) ((((1 ? FP_STAGE_FUNCTION) - (0 ? FP_STAGE_FUNCTION) + 1) == 32) ? ~0 : (~(~0 << ((1 ? FP_STAGE_FUNCTION) - (0 ? FP_STAGE_FUNCTION) + 1))))))) << (0 ? FP_STAGE_FUNCTION))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? FP_STAGE_MASK_FUNCTION) - (0 ? FP_STAGE_MASK_FUNCTION) + 1) == 32) ? ~0 : (~(~0 << ((1 ? FP_STAGE_MASK_FUNCTION) - (0 ? FP_STAGE_MASK_FUNCTION) + 1))))))) << (0 ? FP_STAGE_MASK_FUNCTION))) | (((gctUINT32) (FP_STAGE_MASK_FUNCTION_ENABLED&((gctUINT32)((((1?FP_STAGE_MASK_FUNCTION)-(0?FP_STAGE_MASK_FUNCTION)+1)==32)?~0:(~(~0<<((1?FP_STAGE_MASK_FUNCTION)-(0?FP_STAGE_MASK_FUNCTION)+1)))))))<<(0?FP_STAGE_MASK_FUNCTION))))
		& (    ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? FP_STAGE_SOURCE0) - (0 ? FP_STAGE_SOURCE0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? FP_STAGE_SOURCE0) - (0 ? FP_STAGE_SOURCE0) + 1))))))) << (0 ? FP_STAGE_SOURCE0))) | (((gctUINT32) ((gctUINT32) ((source0)) & ((gctUINT32) ((((1 ? FP_STAGE_SOURCE0) - (0 ? FP_STAGE_SOURCE0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? FP_STAGE_SOURCE0) - (0 ? FP_STAGE_SOURCE0) + 1))))))) << (0 ? FP_STAGE_SOURCE0))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? FP_STAGE_MASK_SOURCE0) - (0 ? FP_STAGE_MASK_SOURCE0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? FP_STAGE_MASK_SOURCE0) - (0 ? FP_STAGE_MASK_SOURCE0) + 1))))))) << (0 ? FP_STAGE_MASK_SOURCE0))) | (((gctUINT32) (FP_STAGE_MASK_SOURCE0_ENABLED&((gctUINT32)((((1?FP_STAGE_MASK_SOURCE0)-(0?FP_STAGE_MASK_SOURCE0)+1)==32)?~0:(~(~0<<((1?FP_STAGE_MASK_SOURCE0)-(0?FP_STAGE_MASK_SOURCE0)+1)))))))<<(0?FP_STAGE_MASK_SOURCE0))))
		& (    ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? FP_STAGE_SOURCE1) - (0 ? FP_STAGE_SOURCE1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? FP_STAGE_SOURCE1) - (0 ? FP_STAGE_SOURCE1) + 1))))))) << (0 ? FP_STAGE_SOURCE1))) | (((gctUINT32) ((gctUINT32) ((source1)) & ((gctUINT32) ((((1 ? FP_STAGE_SOURCE1) - (0 ? FP_STAGE_SOURCE1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? FP_STAGE_SOURCE1) - (0 ? FP_STAGE_SOURCE1) + 1))))))) << (0 ? FP_STAGE_SOURCE1))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? FP_STAGE_MASK_SOURCE1) - (0 ? FP_STAGE_MASK_SOURCE1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? FP_STAGE_MASK_SOURCE1) - (0 ? FP_STAGE_MASK_SOURCE1) + 1))))))) << (0 ? FP_STAGE_MASK_SOURCE1))) | (((gctUINT32) (FP_STAGE_MASK_SOURCE1_ENABLED&((gctUINT32)((((1?FP_STAGE_MASK_SOURCE1)-(0?FP_STAGE_MASK_SOURCE1)+1)==32)?~0:(~(~0<<((1?FP_STAGE_MASK_SOURCE1)-(0?FP_STAGE_MASK_SOURCE1)+1)))))))<<(0?FP_STAGE_MASK_SOURCE1))))
		& (    ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? FP_STAGE_SOURCE2) - (0 ? FP_STAGE_SOURCE2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? FP_STAGE_SOURCE2) - (0 ? FP_STAGE_SOURCE2) + 1))))))) << (0 ? FP_STAGE_SOURCE2))) | (((gctUINT32) ((gctUINT32) ((source2)) & ((gctUINT32) ((((1 ? FP_STAGE_SOURCE2) - (0 ? FP_STAGE_SOURCE2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? FP_STAGE_SOURCE2) - (0 ? FP_STAGE_SOURCE2) + 1))))))) << (0 ? FP_STAGE_SOURCE2))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? FP_STAGE_MASK_SOURCE2) - (0 ? FP_STAGE_MASK_SOURCE2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? FP_STAGE_MASK_SOURCE2) - (0 ? FP_STAGE_MASK_SOURCE2) + 1))))))) << (0 ? FP_STAGE_MASK_SOURCE2))) | (((gctUINT32) (FP_STAGE_MASK_SOURCE2_ENABLED&((gctUINT32)((((1?FP_STAGE_MASK_SOURCE2)-(0?FP_STAGE_MASK_SOURCE2)+1)==32)?~0:(~(~0<<((1?FP_STAGE_MASK_SOURCE2)-(0?FP_STAGE_MASK_SOURCE2)+1)))))))<<(0?FP_STAGE_MASK_SOURCE2))))
		& (    ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? FP_STAGE_SHIFT) - (0 ? FP_STAGE_SHIFT) + 1) == 32) ? ~0 : (~(~0 << ((1 ? FP_STAGE_SHIFT) - (0 ? FP_STAGE_SHIFT) + 1))))))) << (0 ? FP_STAGE_SHIFT))) | (((gctUINT32) ((gctUINT32) ((shift)) & ((gctUINT32) ((((1 ? FP_STAGE_SHIFT) - (0 ? FP_STAGE_SHIFT) + 1) == 32) ? ~0 : (~(~0 << ((1 ? FP_STAGE_SHIFT) - (0 ? FP_STAGE_SHIFT) + 1))))))) << (0 ? FP_STAGE_SHIFT))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? FP_STAGE_MASK_SHIFT) - (0 ? FP_STAGE_MASK_SHIFT) + 1) == 32) ? ~0 : (~(~0 << ((1 ? FP_STAGE_MASK_SHIFT) - (0 ? FP_STAGE_MASK_SHIFT) + 1))))))) << (0 ? FP_STAGE_MASK_SHIFT))) | (((gctUINT32) (FP_STAGE_MASK_SHIFT_ENABLED&((gctUINT32)((((1?FP_STAGE_MASK_SHIFT)-(0?FP_STAGE_MASK_SHIFT)+1)==32)?~0:(~(~0<<((1?FP_STAGE_MASK_SHIFT)-(0?FP_STAGE_MASK_SHIFT)+1)))))))<<(0?FP_STAGE_MASK_SHIFT))));

	/* Set texture function configuration. */
	return gcoHARDWARE_LoadState32(Hardware, Address, config);
}

#endif


/*******************************************************************************
**
**	gcoHARDWARE_SetFragmentConfiguration
**
**	Set the fragment processor configuration.
**
**	INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**		gctBOOL ColorFromStream
**			Selects whether the fragment color comes from the color stream or
**			from the constant.
**
**		gctBOOL EnableFog
**			Fog enable flag.
**
**		gctBOOL EnableSmoothPoint
**			Antialiased point enable flag.
**
**		gctUINT32 ClipPlanes
**			Clip plane enable bits:
**				[0] for plane 0;
**				[1] for plane 1;
**				[2] for plane 2;
**				[3] for plane 3;
**				[4] for plane 4;
**				[5] for plane 5.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoHARDWARE_SetFragmentConfiguration(
	IN gcoHARDWARE Hardware,
	IN gctBOOL ColorFromStream,
	IN gctBOOL EnableFog,
	IN gctBOOL EnableSmoothPoint,
	IN gctUINT32 ClipPlanes
	)
{
#if defined (FP_PRESENT)
	gctUINT32 config;
	gctBOOL userClipping;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	/* Determine whether user clipping is enabled. */
	userClipping = ((ClipPlanes & 0x3F) != 0);

	/* Determine the value. */
	config
		= (    ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? GCREG_FP_CONFIGURATION_COLOR_FROM_STREAM) - (0 ? GCREG_FP_CONFIGURATION_COLOR_FROM_STREAM) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_CONFIGURATION_COLOR_FROM_STREAM) - (0 ? GCREG_FP_CONFIGURATION_COLOR_FROM_STREAM) + 1))))))) << (0 ? GCREG_FP_CONFIGURATION_COLOR_FROM_STREAM))) | (((gctUINT32) ((gctUINT32) ((ColorFromStream)) & ((gctUINT32) ((((1 ? GCREG_FP_CONFIGURATION_COLOR_FROM_STREAM) - (0 ? GCREG_FP_CONFIGURATION_COLOR_FROM_STREAM) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_CONFIGURATION_COLOR_FROM_STREAM) - (0 ? GCREG_FP_CONFIGURATION_COLOR_FROM_STREAM) + 1))))))) << (0 ? GCREG_FP_CONFIGURATION_COLOR_FROM_STREAM))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? GCREG_FP_CONFIGURATION_MASK_COLOR_FROM_STREAM) - (0 ? GCREG_FP_CONFIGURATION_MASK_COLOR_FROM_STREAM) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_CONFIGURATION_MASK_COLOR_FROM_STREAM) - (0 ? GCREG_FP_CONFIGURATION_MASK_COLOR_FROM_STREAM) + 1))))))) << (0 ? GCREG_FP_CONFIGURATION_MASK_COLOR_FROM_STREAM))) | (((gctUINT32) (GCREG_FP_CONFIGURATION_MASK_COLOR_FROM_STREAM_ENABLED&((gctUINT32)((((1?GCREG_FP_CONFIGURATION_MASK_COLOR_FROM_STREAM)-(0?GCREG_FP_CONFIGURATION_MASK_COLOR_FROM_STREAM)+1)==32)?~0:(~(~0<<((1?GCREG_FP_CONFIGURATION_MASK_COLOR_FROM_STREAM)-(0?GCREG_FP_CONFIGURATION_MASK_COLOR_FROM_STREAM)+1)))))))<<(0?GCREG_FP_CONFIGURATION_MASK_COLOR_FROM_STREAM))))
		& (    ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? GCREG_FP_CONFIGURATION_FOG) - (0 ? GCREG_FP_CONFIGURATION_FOG) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_CONFIGURATION_FOG) - (0 ? GCREG_FP_CONFIGURATION_FOG) + 1))))))) << (0 ? GCREG_FP_CONFIGURATION_FOG))) | (((gctUINT32) ((gctUINT32) ((EnableFog)) & ((gctUINT32) ((((1 ? GCREG_FP_CONFIGURATION_FOG) - (0 ? GCREG_FP_CONFIGURATION_FOG) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_CONFIGURATION_FOG) - (0 ? GCREG_FP_CONFIGURATION_FOG) + 1))))))) << (0 ? GCREG_FP_CONFIGURATION_FOG))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? GCREG_FP_CONFIGURATION_MASK_FOG) - (0 ? GCREG_FP_CONFIGURATION_MASK_FOG) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_CONFIGURATION_MASK_FOG) - (0 ? GCREG_FP_CONFIGURATION_MASK_FOG) + 1))))))) << (0 ? GCREG_FP_CONFIGURATION_MASK_FOG))) | (((gctUINT32) (GCREG_FP_CONFIGURATION_MASK_FOG_ENABLED&((gctUINT32)((((1?GCREG_FP_CONFIGURATION_MASK_FOG)-(0?GCREG_FP_CONFIGURATION_MASK_FOG)+1)==32)?~0:(~(~0<<((1?GCREG_FP_CONFIGURATION_MASK_FOG)-(0?GCREG_FP_CONFIGURATION_MASK_FOG)+1)))))))<<(0?GCREG_FP_CONFIGURATION_MASK_FOG))))
		& (    ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? GCREG_FP_CONFIGURATION_SMOOTH_POINT) - (0 ? GCREG_FP_CONFIGURATION_SMOOTH_POINT) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_CONFIGURATION_SMOOTH_POINT) - (0 ? GCREG_FP_CONFIGURATION_SMOOTH_POINT) + 1))))))) << (0 ? GCREG_FP_CONFIGURATION_SMOOTH_POINT))) | (((gctUINT32) ((gctUINT32) ((EnableSmoothPoint)) & ((gctUINT32) ((((1 ? GCREG_FP_CONFIGURATION_SMOOTH_POINT) - (0 ? GCREG_FP_CONFIGURATION_SMOOTH_POINT) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_CONFIGURATION_SMOOTH_POINT) - (0 ? GCREG_FP_CONFIGURATION_SMOOTH_POINT) + 1))))))) << (0 ? GCREG_FP_CONFIGURATION_SMOOTH_POINT))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? GCREG_FP_CONFIGURATION_MASK_SMOOTH_POINT) - (0 ? GCREG_FP_CONFIGURATION_MASK_SMOOTH_POINT) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_CONFIGURATION_MASK_SMOOTH_POINT) - (0 ? GCREG_FP_CONFIGURATION_MASK_SMOOTH_POINT) + 1))))))) << (0 ? GCREG_FP_CONFIGURATION_MASK_SMOOTH_POINT))) | (((gctUINT32) (GCREG_FP_CONFIGURATION_MASK_SMOOTH_POINT_ENABLED&((gctUINT32)((((1?GCREG_FP_CONFIGURATION_MASK_SMOOTH_POINT)-(0?GCREG_FP_CONFIGURATION_MASK_SMOOTH_POINT)+1)==32)?~0:(~(~0<<((1?GCREG_FP_CONFIGURATION_MASK_SMOOTH_POINT)-(0?GCREG_FP_CONFIGURATION_MASK_SMOOTH_POINT)+1)))))))<<(0?GCREG_FP_CONFIGURATION_MASK_SMOOTH_POINT))))
		& (    ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? GCREG_FP_CONFIGURATION_USER_CLIPPING) - (0 ? GCREG_FP_CONFIGURATION_USER_CLIPPING) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_CONFIGURATION_USER_CLIPPING) - (0 ? GCREG_FP_CONFIGURATION_USER_CLIPPING) + 1))))))) << (0 ? GCREG_FP_CONFIGURATION_USER_CLIPPING))) | (((gctUINT32) ((gctUINT32) ((userClipping)) & ((gctUINT32) ((((1 ? GCREG_FP_CONFIGURATION_USER_CLIPPING) - (0 ? GCREG_FP_CONFIGURATION_USER_CLIPPING) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_CONFIGURATION_USER_CLIPPING) - (0 ? GCREG_FP_CONFIGURATION_USER_CLIPPING) + 1))))))) << (0 ? GCREG_FP_CONFIGURATION_USER_CLIPPING))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? GCREG_FP_CONFIGURATION_MASK_USER_CLIPPING) - (0 ? GCREG_FP_CONFIGURATION_MASK_USER_CLIPPING) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_CONFIGURATION_MASK_USER_CLIPPING) - (0 ? GCREG_FP_CONFIGURATION_MASK_USER_CLIPPING) + 1))))))) << (0 ? GCREG_FP_CONFIGURATION_MASK_USER_CLIPPING))) | (((gctUINT32) (GCREG_FP_CONFIGURATION_MASK_USER_CLIPPING_ENABLED&((gctUINT32)((((1?GCREG_FP_CONFIGURATION_MASK_USER_CLIPPING)-(0?GCREG_FP_CONFIGURATION_MASK_USER_CLIPPING)+1)==32)?~0:(~(~0<<((1?GCREG_FP_CONFIGURATION_MASK_USER_CLIPPING)-(0?GCREG_FP_CONFIGURATION_MASK_USER_CLIPPING)+1)))))))<<(0?GCREG_FP_CONFIGURATION_MASK_USER_CLIPPING))))
		& (    ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? GCREG_FP_CONFIGURATION_CLIP_PLANES) - (0 ? GCREG_FP_CONFIGURATION_CLIP_PLANES) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_CONFIGURATION_CLIP_PLANES) - (0 ? GCREG_FP_CONFIGURATION_CLIP_PLANES) + 1))))))) << (0 ? GCREG_FP_CONFIGURATION_CLIP_PLANES))) | (((gctUINT32) ((gctUINT32) ((ClipPlanes)) & ((gctUINT32) ((((1 ? GCREG_FP_CONFIGURATION_CLIP_PLANES) - (0 ? GCREG_FP_CONFIGURATION_CLIP_PLANES) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_CONFIGURATION_CLIP_PLANES) - (0 ? GCREG_FP_CONFIGURATION_CLIP_PLANES) + 1))))))) << (0 ? GCREG_FP_CONFIGURATION_CLIP_PLANES))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? GCREG_FP_CONFIGURATION_MASK_CLIP_PLANES) - (0 ? GCREG_FP_CONFIGURATION_MASK_CLIP_PLANES) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_CONFIGURATION_MASK_CLIP_PLANES) - (0 ? GCREG_FP_CONFIGURATION_MASK_CLIP_PLANES) + 1))))))) << (0 ? GCREG_FP_CONFIGURATION_MASK_CLIP_PLANES))) | (((gctUINT32) (GCREG_FP_CONFIGURATION_MASK_CLIP_PLANES_ENABLED&((gctUINT32)((((1?GCREG_FP_CONFIGURATION_MASK_CLIP_PLANES)-(0?GCREG_FP_CONFIGURATION_MASK_CLIP_PLANES)+1)==32)?~0:(~(~0<<((1?GCREG_FP_CONFIGURATION_MASK_CLIP_PLANES)-(0?GCREG_FP_CONFIGURATION_MASK_CLIP_PLANES)+1)))))))<<(0?GCREG_FP_CONFIGURATION_MASK_CLIP_PLANES))));

	/* Set configuration. */
	return gcoHARDWARE_LoadState32(
		Hardware,
		GCREG_FP_CONFIGURATION_Address,
		config
		);
#else
	return gcvSTATUS_NOT_SUPPORTED;
#endif
}


/*******************************************************************************
**
**	gcoHARDWARE_EnableTextureStage
**
**	Enable/disable texture stage operation.
**
**	INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**		gctINT Stage
**			Target stage number.
**
**		gctBOOL Enable
**			Stage enable flag.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoHARDWARE_EnableTextureStage(
	IN gcoHARDWARE Hardware,
	IN gctINT Stage,
	IN gctBOOL Enable
	)
{
#if defined (FP_PRESENT)
	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	/* Verify the stage number. */
	if ((Stage < 0) || (Stage >= 4))
	{
		return gcvSTATUS_INVALID_ARGUMENT;
	}

	/* Set configuration. */
	return gcoHARDWARE_LoadState32(
		Hardware,
		GCREG_FP_TEXTURE_Address + Stage * 4,
		(    ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? GCREG_FP_TEXTURE_STAGE) - (0 ? GCREG_FP_TEXTURE_STAGE) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_TEXTURE_STAGE) - (0 ? GCREG_FP_TEXTURE_STAGE) + 1))))))) << (0 ? GCREG_FP_TEXTURE_STAGE))) | (((gctUINT32) ((gctUINT32) ((Enable)) & ((gctUINT32) ((((1 ? GCREG_FP_TEXTURE_STAGE) - (0 ? GCREG_FP_TEXTURE_STAGE) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_TEXTURE_STAGE) - (0 ? GCREG_FP_TEXTURE_STAGE) + 1))))))) << (0 ? GCREG_FP_TEXTURE_STAGE))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? GCREG_FP_TEXTURE_MASK_STAGE) - (0 ? GCREG_FP_TEXTURE_MASK_STAGE) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_TEXTURE_MASK_STAGE) - (0 ? GCREG_FP_TEXTURE_MASK_STAGE) + 1))))))) << (0 ? GCREG_FP_TEXTURE_MASK_STAGE))) | (((gctUINT32) (GCREG_FP_TEXTURE_MASK_STAGE_ENABLED&((gctUINT32)((((1?GCREG_FP_TEXTURE_MASK_STAGE)-(0?GCREG_FP_TEXTURE_MASK_STAGE)+1)==32)?~0:(~(~0<<((1?GCREG_FP_TEXTURE_MASK_STAGE)-(0?GCREG_FP_TEXTURE_MASK_STAGE)+1)))))))<<(0?GCREG_FP_TEXTURE_MASK_STAGE))))
		);
#else
	return gcvSTATUS_NOT_SUPPORTED;
#endif
}


/*******************************************************************************
**
**	gcoHARDWARE_SetTextureColorMask
**
**	Program the channel enable masks for the color texture function.
**
**	INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**		gctINT Stage
**			Target stage number.
**
**		gctBOOL ColorEnabled
**			Determines whether RGB color result from the color texture
**			function affects the overall result or should be ignored.
**
**		gctBOOL AlphaEnabled
**			Determines whether A color result from the color texture
**			function affects the overall result or should be ignored.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoHARDWARE_SetTextureColorMask(
	IN gcoHARDWARE Hardware,
	IN gctINT Stage,
	IN gctBOOL ColorEnabled,
	IN gctBOOL AlphaEnabled
	)
{
#if defined (FP_PRESENT)
	gctUINT32 mask = GCREG_FP_TEXTURE_COLOR_ENABLE_NONE;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	/* Verify the stage number. */
	if ((Stage < 0) || (Stage >= 4))
	{
		return gcvSTATUS_INVALID_ARGUMENT;
	}

	/* Determine the mask. */
	if (ColorEnabled)
	{
		mask |= GCREG_FP_TEXTURE_COLOR_ENABLE_RGB;
	}

	if (AlphaEnabled)
	{
		mask |= GCREG_FP_TEXTURE_COLOR_ENABLE_A;
	}

	/* Set configuration. */
	return gcoHARDWARE_LoadState32(
		Hardware,
		GCREG_FP_TEXTURE_Address + Stage * 4,
		(    ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? GCREG_FP_TEXTURE_COLOR_ENABLE) - (0 ? GCREG_FP_TEXTURE_COLOR_ENABLE) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_TEXTURE_COLOR_ENABLE) - (0 ? GCREG_FP_TEXTURE_COLOR_ENABLE) + 1))))))) << (0 ? GCREG_FP_TEXTURE_COLOR_ENABLE))) | (((gctUINT32) ((gctUINT32) ((mask)) & ((gctUINT32) ((((1 ? GCREG_FP_TEXTURE_COLOR_ENABLE) - (0 ? GCREG_FP_TEXTURE_COLOR_ENABLE) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_TEXTURE_COLOR_ENABLE) - (0 ? GCREG_FP_TEXTURE_COLOR_ENABLE) + 1))))))) << (0 ? GCREG_FP_TEXTURE_COLOR_ENABLE))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? GCREG_FP_TEXTURE_MASK_COLOR_ENABLE) - (0 ? GCREG_FP_TEXTURE_MASK_COLOR_ENABLE) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_TEXTURE_MASK_COLOR_ENABLE) - (0 ? GCREG_FP_TEXTURE_MASK_COLOR_ENABLE) + 1))))))) << (0 ? GCREG_FP_TEXTURE_MASK_COLOR_ENABLE))) | (((gctUINT32) (GCREG_FP_TEXTURE_MASK_COLOR_ENABLE_ENABLED & ((gctUINT32) ((((1 ? GCREG_FP_TEXTURE_MASK_COLOR_ENABLE) - (0 ? GCREG_FP_TEXTURE_MASK_COLOR_ENABLE) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_TEXTURE_MASK_COLOR_ENABLE) - (0 ? GCREG_FP_TEXTURE_MASK_COLOR_ENABLE) + 1))))))) << (0 ? GCREG_FP_TEXTURE_MASK_COLOR_ENABLE))))
		);
#else
	return gcvSTATUS_NOT_SUPPORTED;
#endif
}


/*******************************************************************************
**
**	gcoHARDWARE_SetTextureAlphaMask
**
**	Program the channel enable masks for the alpha texture function.
**
**	INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**		gctINT Stage
**			Target stage number.
**
**		gctBOOL ColorEnabled
**			Determines whether RGB color result from the alpha texture
**			function affects the overall result or should be ignored.
**
**		gctBOOL AlphaEnabled
**			Determines whether A color result from the alpha texture
**			function affects the overall result or should be ignored.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoHARDWARE_SetTextureAlphaMask(
	IN gcoHARDWARE Hardware,
	IN gctINT Stage,
	IN gctBOOL ColorEnabled,
	IN gctBOOL AlphaEnabled
	)
{
#if defined (FP_PRESENT)
	gctUINT32 mask = GCREG_FP_TEXTURE_ALPHA_ENABLE_NONE;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	/* Verify the stage number. */
	if ((Stage < 0) || (Stage >= 4))
	{
		return gcvSTATUS_INVALID_ARGUMENT;
	}

	/* Determine the mask. */
	if (ColorEnabled)
	{
		mask |= GCREG_FP_TEXTURE_ALPHA_ENABLE_RGB;
	}

	if (AlphaEnabled)
	{
		mask |= GCREG_FP_TEXTURE_ALPHA_ENABLE_A;
	}

	/* Set configuration. */
	return gcoHARDWARE_LoadState32(
		Hardware,
		GCREG_FP_TEXTURE_Address + Stage * 4,
		(    ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? GCREG_FP_TEXTURE_ALPHA_ENABLE) - (0 ? GCREG_FP_TEXTURE_ALPHA_ENABLE) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_TEXTURE_ALPHA_ENABLE) - (0 ? GCREG_FP_TEXTURE_ALPHA_ENABLE) + 1))))))) << (0 ? GCREG_FP_TEXTURE_ALPHA_ENABLE))) | (((gctUINT32) ((gctUINT32) ((mask)) & ((gctUINT32) ((((1 ? GCREG_FP_TEXTURE_ALPHA_ENABLE) - (0 ? GCREG_FP_TEXTURE_ALPHA_ENABLE) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_TEXTURE_ALPHA_ENABLE) - (0 ? GCREG_FP_TEXTURE_ALPHA_ENABLE) + 1))))))) << (0 ? GCREG_FP_TEXTURE_ALPHA_ENABLE))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? GCREG_FP_TEXTURE_MASK_ALPHA_ENABLE) - (0 ? GCREG_FP_TEXTURE_MASK_ALPHA_ENABLE) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_TEXTURE_MASK_ALPHA_ENABLE) - (0 ? GCREG_FP_TEXTURE_MASK_ALPHA_ENABLE) + 1))))))) << (0 ? GCREG_FP_TEXTURE_MASK_ALPHA_ENABLE))) | (((gctUINT32) (GCREG_FP_TEXTURE_MASK_ALPHA_ENABLE_ENABLED & ((gctUINT32) ((((1 ? GCREG_FP_TEXTURE_MASK_ALPHA_ENABLE) - (0 ? GCREG_FP_TEXTURE_MASK_ALPHA_ENABLE) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GCREG_FP_TEXTURE_MASK_ALPHA_ENABLE) - (0 ? GCREG_FP_TEXTURE_MASK_ALPHA_ENABLE) + 1))))))) << (0 ? GCREG_FP_TEXTURE_MASK_ALPHA_ENABLE))))
		);
#else
	return gcvSTATUS_NOT_SUPPORTED;
#endif
}


/*******************************************************************************
**
**	gcoHARDWARE_SetFragmentColor
**
**	Program the constant fragment color to be used when there is no color
**	defined stream.
**
**	INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**		Red, Green, Blue, Alpha
**			Color value to be set.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoHARDWARE_SetFragmentColorX(
	IN gcoHARDWARE Hardware,
	IN gctFIXED_POINT Red,
	IN gctFIXED_POINT Green,
	IN gctFIXED_POINT Blue,
	IN gctFIXED_POINT Alpha
	)
{
#if defined (FP_PRESENT)
	gceSTATUS status;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		/* Set the color. */
		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_FRAGMENT_RED_X_Address,
			gcmCONVERTFIXED(Red)
			));

		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_FRAGMENT_GREEN_X_Address,
			gcmCONVERTFIXED(Green)
			));

		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_FRAGMENT_BLUE_X_Address,
			gcmCONVERTFIXED(Blue)
			));

		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_FRAGMENT_ALPHA_X_Address,
			gcmCONVERTFIXED(Alpha)
			));
	}
	while (gcvFALSE);

	/* Return status. */
	return status;
#else
	return gcvSTATUS_NOT_SUPPORTED;
#endif
}

gceSTATUS
gcoHARDWARE_SetFragmentColorF(
	IN gcoHARDWARE Hardware,
	IN gctFLOAT Red,
	IN gctFLOAT Green,
	IN gctFLOAT Blue,
	IN gctFLOAT Alpha
	)
{
#if defined (FP_PRESENT)
	gceSTATUS status;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		/* Set the color. */
		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_FRAGMENT_RED_F_Address,
			gcmCONVERTFLOAT(Red)
			));

		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_FRAGMENT_GREEN_F_Address,
			gcmCONVERTFLOAT(Green)
			));

		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_FRAGMENT_BLUE_F_Address,
			gcmCONVERTFLOAT(Blue)
			));

		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_FRAGMENT_ALPHA_F_Address,
			gcmCONVERTFLOAT(Alpha)
			));
	}
	while (gcvFALSE);

	/* Return status. */
	return status;
#else
	return gcvSTATUS_NOT_SUPPORTED;
#endif
}


/*******************************************************************************
**
**	gcoHARDWARE_SetFogColor
**
**	Program the constant fog color to be used in the fog equation when fog
**	is enabled.
**
**	INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**		Red, Green, Blue, Alpha
**			Color value to be set.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoHARDWARE_SetFogColorX(
	IN gcoHARDWARE Hardware,
	IN gctFIXED_POINT Red,
	IN gctFIXED_POINT Green,
	IN gctFIXED_POINT Blue,
	IN gctFIXED_POINT Alpha
	)
{
#if defined (FP_PRESENT)
	gceSTATUS status;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		/* Set the color. */
		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_FOG_RED_X_Address,
			gcmCONVERTFIXED(Red)
			));

		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_FOG_GREEN_X_Address,
			gcmCONVERTFIXED(Green)
			));

		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_FOG_BLUE_X_Address,
			gcmCONVERTFIXED(Blue)
			));

		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_FOG_ALPHA_X_Address,
			gcmCONVERTFIXED(Alpha)
			));
	}
	while (gcvFALSE);

	/* Return status. */
	return status;
#else
	return gcvSTATUS_NOT_SUPPORTED;
#endif
}

gceSTATUS
gcoHARDWARE_SetFogColorF(
	IN gcoHARDWARE Hardware,
	IN gctFLOAT Red,
	IN gctFLOAT Green,
	IN gctFLOAT Blue,
	IN gctFLOAT Alpha
	)
{
#if defined (FP_PRESENT)
	gceSTATUS status;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		/* Set the color. */
		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_FOG_RED_F_Address,
			gcmCONVERTFLOAT(Red)
			));

		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_FOG_GREEN_F_Address,
			gcmCONVERTFLOAT(Green)
			));

		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_FOG_BLUE_F_Address,
			gcmCONVERTFLOAT(Blue)
			));

		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_FOG_ALPHA_F_Address,
			gcmCONVERTFLOAT(Alpha)
			));
	}
	while (gcvFALSE);

	/* Return status. */
	return status;
#else
	return gcvSTATUS_NOT_SUPPORTED;
#endif
}


/*******************************************************************************
**
**	gcoHARDWARE_SetTetxureColor
**
**	Program the constant texture color to be used when selected by the tetxure
**	function.
**
**	INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**		gctINT Stage
**			Target stage number.
**
**		Red, Green, Blue, Alpha
**			Color value to be set.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoHARDWARE_SetTetxureColorX(
	IN gcoHARDWARE Hardware,
	IN gctINT Stage,
	IN gctFIXED_POINT Red,
	IN gctFIXED_POINT Green,
	IN gctFIXED_POINT Blue,
	IN gctFIXED_POINT Alpha
	)
{
#if defined (FP_PRESENT)
	gceSTATUS status;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		/* Determine stage offset. */
		gctUINT32 offset = Stage * 4;

		/* Set the color. */
		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_TEXTURE_RED_X_Address + offset,
			gcmCONVERTFIXED(Red)
			));

		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_TEXTURE_GREEN_X_Address + offset,
			gcmCONVERTFIXED(Green)
			));

		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_TEXTURE_BLUE_X_Address + offset,
			gcmCONVERTFIXED(Blue)
			));

		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_TEXTURE_ALPHA_X_Address + offset,
			gcmCONVERTFIXED(Alpha)
			));
	}
	while (gcvFALSE);

	/* Return status. */
	return status;
#else
	return gcvSTATUS_NOT_SUPPORTED;
#endif
}

gceSTATUS
gcoHARDWARE_SetTetxureColorF(
	IN gcoHARDWARE Hardware,
	IN gctINT Stage,
	IN gctFLOAT Red,
	IN gctFLOAT Green,
	IN gctFLOAT Blue,
	IN gctFLOAT Alpha
	)
{
#if defined (FP_PRESENT)
	gceSTATUS status;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		/* Determine stage offset. */
		gctUINT32 offset = Stage * 4;

		/* Set the color. */
		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_TEXTURE_RED_F_Address + offset,
			gcmCONVERTFLOAT(Red)
			));

		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_TEXTURE_GREEN_F_Address + offset,
			gcmCONVERTFLOAT(Green)
			));

		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_TEXTURE_BLUE_F_Address + offset,
			gcmCONVERTFLOAT(Blue)
			));

		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			GCREG_FP_TEXTURE_ALPHA_F_Address + offset,
			gcmCONVERTFLOAT(Alpha)
			));
	}
	while (gcvFALSE);

	/* Return status. */
	return status;
#else
	return gcvSTATUS_NOT_SUPPORTED;
#endif
}


/*******************************************************************************
**
**	gcoHARDWARE_SetColorTextureFunction
**
**	Configure color texture function.
**
**	INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**		gctINT Stage
**			Target stage number.
**
**		gceTEXTURE_FUNCTION Function
**			Texture function.
**
**		gceTEXTURE_SOURCE Source0, Source1, Source2
**			The source of the value for the function.
**
**		gceTEXTURE_CHANNEL Channel0, Channel1, Channel2
**			Determines whether the value comes from the color, alpha channel;
**			straight or inversed.
**
**		gctINT Scale
**			Result scale value.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoHARDWARE_SetColorTextureFunction(
	IN gcoHARDWARE Hardware,
	IN gctINT Stage,
	IN gceTEXTURE_FUNCTION Function,
	IN gceTEXTURE_SOURCE Source0,
	IN gceTEXTURE_CHANNEL Channel0,
	IN gceTEXTURE_SOURCE Source1,
	IN gceTEXTURE_CHANNEL Channel1,
	IN gceTEXTURE_SOURCE Source2,
	IN gceTEXTURE_CHANNEL Channel2,
	IN gctINT Scale
	)
{
#if defined (FP_PRESENT)
	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	/* Verify the stage number. */
	if ((Stage < 0) || (Stage >= 4))
	{
		return gcvSTATUS_INVALID_ARGUMENT;
	}

	/* Set the texture function. */
	return _SetTextureFunction(
		Hardware,
		GCREG_FP_COLOR_Address + Stage * 4,
		Function,
		Source0, Channel0,
		Source1, Channel1,
		Source2, Channel2,
		Scale
		);
#else
	return gcvSTATUS_NOT_SUPPORTED;
#endif
}


/*******************************************************************************
**
**	gcoHARDWARE_SetAlphaTextureFunction
**
**	Configure alpha texture function.
**
**	INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**		gctINT Stage
**			Target stage number.
**
**		gceTEXTURE_FUNCTION Function
**			Texture function.
**
**		gceTEXTURE_SOURCE Source0, Source1, Source2
**			The source of the value for the function.
**
**		gceTEXTURE_CHANNEL Channel0, Channel1, Channel2
**			Determines whether the value comes from the color, alpha channel;
**			straight or inversed.
**
**		gctINT Scale
**			Result scale value.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoHARDWARE_SetAlphaTextureFunction(
	IN gcoHARDWARE Hardware,
	IN gctINT Stage,
	IN gceTEXTURE_FUNCTION Function,
	IN gceTEXTURE_SOURCE Source0,
	IN gceTEXTURE_CHANNEL Channel0,
	IN gceTEXTURE_SOURCE Source1,
	IN gceTEXTURE_CHANNEL Channel1,
	IN gceTEXTURE_SOURCE Source2,
	IN gceTEXTURE_CHANNEL Channel2,
	IN gctINT Scale
	)
{
#if defined (FP_PRESENT)
	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	/* Verify the stage number. */
	if ((Stage < 0) || (Stage >= 4))
	{
		return gcvSTATUS_INVALID_ARGUMENT;
	}

	/* Set the texture function. */
	return _SetTextureFunction(
		Hardware,
		GCREG_FP_ALPHA_Address + Stage * 4,
		Function,
		Source0, Channel0,
		Source1, Channel1,
		Source2, Channel2,
		Scale
		);
#else
	return gcvSTATUS_NOT_SUPPORTED;
#endif
}

