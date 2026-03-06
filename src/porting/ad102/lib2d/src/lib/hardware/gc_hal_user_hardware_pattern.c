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

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE	gcvZONE_HARDWARE

/*******************************************************************************
**
**	gcoHARDWARE_LoadSolidColorPattern
**
**	Load solid (single) color pattern.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gctBOOL ColorConvert
**			The value of the Color parameter is stored directly in internal
**			color register and is used either directly to initialize pattern
**			or is converted to the format of destination before it is used.
**			The later happens if ColorConvert is gcvTRUE.
**
**		gctUINT32 Color
**			The color value of the pattern.  The value will be used to
**			initialize the 8x8 pattern.  If the value is in destination format,
**			set ColorConvert to gcvFALSE.  Otherwise, provide the value in A8R8G8B8
**			format and set ColorConvert to gcvTRUE to instruct the hardware to
**			convert the value to the destination format before it is actually
**			used.
**
**		gctUINT64 Mask
**			64 bits of mask, where each bit corresponds to one pixel of the 8x8
**			pattern.  Each bit of the mask is used to determine transparency of
**			the corresponding pixel, in other words, each mask bit is used to
**			select between foreground or background ROPs.  If the bit is 0,
**			background ROP is used on the pixel; if 1, the foreground ROP
**			is used.  The mapping between Mask parameter bits and actual pattern
**			pixels is as follows:
**
**			+----+----+----+----+----+----+----+----+
**			|  7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
**			+----+----+----+----+----+----+----+----+
**			| 15 | 14 | 13 | 12 | 11 | 10 |  9 |  8 |
**			+----+----+----+----+----+----+----+----+
**			| 23 | 22 | 21 | 20 | 19 | 18 | 17 | 16 |
**			+----+----+----+----+----+----+----+----+
**			| 31 | 30 | 29 | 28 | 27 | 26 | 25 | 24 |
**			+----+----+----+----+----+----+----+----+
**			| 39 | 38 | 37 | 36 | 35 | 34 | 33 | 32 |
**			+----+----+----+----+----+----+----+----+
**			| 47 | 46 | 45 | 44 | 43 | 42 | 41 | 40 |
**			+----+----+----+----+----+----+----+----+
**			| 55 | 54 | 53 | 52 | 51 | 50 | 49 | 48 |
**			+----+----+----+----+----+----+----+----+
**			| 63 | 62 | 61 | 60 | 59 | 58 | 57 | 56 |
**			+----+----+----+----+----+----+----+----+
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_LoadSolidColorPattern(
	IN gcoHARDWARE Hardware,
	IN gctBOOL ColorConvert,
	IN gctUINT32 Color,
	IN gctUINT64 Mask
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hardware=0x%x ColorConvert=%d Color=%x Mask=%llx",
					Hardware, ColorConvert, Color, Mask);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
		{
			gctUINT32 config;

			/* SelectPipe(2D). */
			gcmERR_BREAK(gcoHARDWARE_SelectPipe(
				Hardware,
				0x1
				));

			/* LoadState(AQDE_PATTERN_MASK, 2), Mask. */
			gcmERR_BREAK(gcoHARDWARE_LoadState64(
				Hardware,
				0x01248, Mask
				));

			if (!ColorConvert && Hardware->hw2DPE20)
			{
				/* Convert color to ARGB8 if it was specified in target format. */
				gcmERR_BREAK(gcoHARDWARE_ColorConvertToARGB8(
					Hardware->targetSurface.format,
					1,
					&Color,
					&Color
					));
			}

			/* LoadState(AQDE_PATTERN_FG_COLOR, 1), Color. */
			gcmERR_BREAK(gcoHARDWARE_LoadState32(
				Hardware,
				0x01254, Color
				));

			/* Setup pattern configuration. */
			config
				= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))
				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5))) | (((gctUINT32) ((gctUINT32) (ColorConvert) & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:6) - (0 ? 7:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ? 7:6))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 7:6) - (0 ? 7:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ? 7:6)));

			/* LoadState(AQDE_PATTERN_CONFIG, 1), config. */
			gcmERR_BREAK(gcoHARDWARE_LoadState32(
				Hardware,
				0x0123C, config
				));
		}
		else
		{
			/* Pattern is not currently supported by the software renderer. */
			gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
		}
	}
	while (gcvFALSE);

	/* Return the status. */
	gcmFOOTER();
	return status;
}

/******************************************************************************
**
**	gcoHARDWARE_LoadMonochromePattern
**
**	Load monochrome pattern.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gctUINT32 OriginX
**		gctUINT32 OriginY
**			Specifies the origin of the pattern in 0..7 range.
**
**		gctBOOL ColorConvert
**			The values of FgColor and BgColor parameters are stored directly in
**			internal color registers and are used either directly to initialize
**			pattern or converted to the format of destination before actually
**			used.  The later happens if ColorConvert is gcvTRUE.
**
**		gctUINT32 FgColor
**		gctUINT32 BgColor
**			Foreground and background colors of the pattern.  The values will be
**			used to initialize the 8x8 pattern.  If the values are in
**          destination format, set ColorConvert to gcvFALSE.  Otherwise, provide
**          the values in A8R8G8B8 format and set ColorConvert to gcvTRUE to
**          instruct the hardware to convert the values to the destination
**          format before they are actually used.
**
**		gctUINT64 Bits
**			64 bits of pixel bits.  Each bit represents one pixel and is used
**			to choose between foreground and background colors.  If the bit
**			is 0, the background color is used; otherwise the foreground color
**			is used.  The mapping between Bits parameter and the actual pattern
**			pixels is the same as of the Mask parameter.
**
**		gctUINT64 Mask
**			64 bits of mask, where each bit corresponds to one pixel of the 8x8
**			pattern.  Each bit of the mask is used to determine transparency of
**			the corresponding pixel, in other words, each mask bit is used to
**			select between foreground or background ROPs.  If the bit is 0,
**			background ROP is used on the pixel; if 1, the foreground ROP is
**			used.  The mapping between Mask parameter bits and the actual
**			pattern pixels is as follows:
**
**			+----+----+----+----+----+----+----+----+
**			|  7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
**			+----+----+----+----+----+----+----+----+
**			| 15 | 14 | 13 | 12 | 11 | 10 |  9 |  8 |
**			+----+----+----+----+----+----+----+----+
**			| 23 | 22 | 21 | 20 | 19 | 18 | 17 | 16 |
**			+----+----+----+----+----+----+----+----+
**			| 31 | 30 | 29 | 28 | 27 | 26 | 25 | 24 |
**			+----+----+----+----+----+----+----+----+
**			| 39 | 38 | 37 | 36 | 35 | 34 | 33 | 32 |
**			+----+----+----+----+----+----+----+----+
**			| 47 | 46 | 45 | 44 | 43 | 42 | 41 | 40 |
**			+----+----+----+----+----+----+----+----+
**			| 55 | 54 | 53 | 52 | 51 | 50 | 49 | 48 |
**			+----+----+----+----+----+----+----+----+
**			| 63 | 62 | 61 | 60 | 59 | 58 | 57 | 56 |
**			+----+----+----+----+----+----+----+----+
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_LoadMonochromePattern(
	IN gcoHARDWARE Hardware,
	IN gctUINT32 OriginX,
	IN gctUINT32 OriginY,
	IN gctBOOL ColorConvert,
	IN gctUINT32 FgColor,
	IN gctUINT32 BgColor,
	IN gctUINT64 Bits,
	IN gctUINT64 Mask
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hardware=0x%x OriginX=%d OriginY=%d "
					"ColorConvert=%d FgColor=%x BgColor=%x "
					"Bits=%lld Mask=%llx",
					Hardware, OriginX, OriginY,
					ColorConvert, FgColor, BgColor,
					Bits, Mask);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
		{
			gctUINT32 data[6], config;

			/* SelectPipe(2D). */
			gcmERR_BREAK(gcoHARDWARE_SelectPipe(
				Hardware,
				0x1
				));

			/* Pattern data. */
			*(gctUINT64 *) &data[0] = Bits;

			/* Mask data. */
			*(gctUINT64 *) &data[2] = Mask;

			if (!ColorConvert && Hardware->hw2DPE20)
			{
				/* Convert colors to ARGB8 if they were specified in target format. */
				gcmERR_BREAK(gcoHARDWARE_ColorConvertToARGB8(
					Hardware->targetSurface.format,
					1,
					&BgColor,
					&BgColor
					));

				gcmERR_BREAK(gcoHARDWARE_ColorConvertToARGB8(
					Hardware->targetSurface.format,
					1,
					&FgColor,
					&FgColor
					));
			}

			/* Backgroud color. */
			data[4] = BgColor;

			/* Foreground color. */
			data[5] = FgColor;

			/* LoadState(AQDE_PATTERN_LOW, 6), Bits, Mask, BgColor, FgColor. */
			gcmERR_BREAK(gcoHARDWARE_LoadState(
				Hardware,
				0x01240, 6,
				data
				));

			/* Setup pattern configuration. */
			config
				= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 18:16) - (0 ? 18:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16))) | (((gctUINT32) ((gctUINT32) (OriginX) & ((gctUINT32) ((((1 ? 18:16) - (0 ? 18:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 22:20) - (0 ? 22:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20))) | (((gctUINT32) ((gctUINT32) (OriginY) & ((gctUINT32) ((((1 ? 22:20) - (0 ? 22:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5))) | (((gctUINT32) ((gctUINT32) (ColorConvert) & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0))) | (((gctUINT32) (0xA & ((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))
				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 28:24) - (0 ? 28:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:24) - (0 ? 28:24) + 1))))))) << (0 ? 28:24))) | (((gctUINT32) (0x0A & ((gctUINT32) ((((1 ? 28:24) - (0 ? 28:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:24) - (0 ? 28:24) + 1))))))) << (0 ? 28:24)))
				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))
				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:6) - (0 ? 7:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ? 7:6))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 7:6) - (0 ? 7:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ? 7:6)));

			/* LoadState(AQDE_PATTERN_CONFIG, 1), config. */
			gcmERR_BREAK(gcoHARDWARE_LoadState32(
				Hardware,
				0x0123C, config
				));
		}
		else
		{
			/* Pattern is not currently supported by the software renderer. */
			gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
		}
	}
	while (gcvFALSE);

	/* Return the status. */
	gcmFOOTER();
	return status;
}

/******************************************************************************
**
**	gcoHARDWARE_LoadColorPattern
**
**	Load pattern.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gctUINT32 OriginX
**		gctUINT32 OriginY
**			Specifies the origin of the pattern in 0..7 range.
**
**		gctUINT32 Address
**			Location of the pattern bitmap in local memory.
**
**		gceSURF_FORMAT Format
**			Format of the source bitmap.
**
**		gctUINT64 Mask
**			64 bits of mask, where each bit corresponds to one pixel of the 8x8
**			pattern.  Each bit of the mask is used to determine transparency of
**			the corresponding pixel, in other words, each mask bit is used to
**			select between foreground or background ROPs.  If the bit is 0,
**			background ROP is used on the pixel; if 1, the foreground ROP is
**			used.  The mapping between Mask parameter bits and the actual
**			pattern pixels is as follows:
**
**			+----+----+----+----+----+----+----+----+
**			|  7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
**			+----+----+----+----+----+----+----+----+
**			| 15 | 14 | 13 | 12 | 11 | 10 |  9 |  8 |
**			+----+----+----+----+----+----+----+----+
**			| 23 | 22 | 21 | 20 | 19 | 18 | 17 | 16 |
**			+----+----+----+----+----+----+----+----+
**			| 31 | 30 | 29 | 28 | 27 | 26 | 25 | 24 |
**			+----+----+----+----+----+----+----+----+
**			| 39 | 38 | 37 | 36 | 35 | 34 | 33 | 32 |
**			+----+----+----+----+----+----+----+----+
**			| 47 | 46 | 45 | 44 | 43 | 42 | 41 | 40 |
**			+----+----+----+----+----+----+----+----+
**			| 55 | 54 | 53 | 52 | 51 | 50 | 49 | 48 |
**			+----+----+----+----+----+----+----+----+
**			| 63 | 62 | 61 | 60 | 59 | 58 | 57 | 56 |
**			+----+----+----+----+----+----+----+----+
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_LoadColorPattern(
	IN gcoHARDWARE Hardware,
	IN gctUINT32 OriginX,
	IN gctUINT32 OriginY,
	IN gctUINT32 Address,
	IN gceSURF_FORMAT Format,
	IN gctUINT64 Mask
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hardware=0x%x OriginX=%d OriginY=%d "
					"Address=%x Format=%d Mask=%llx",
					Hardware, OriginX, OriginY,
					Address, Format, Mask);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
		{
			gctUINT32 format, swizzle, isYUVformat, config;

			/* Convert the format. */
			gcmERR_BREAK(gcoHARDWARE_TranslatePatternFormat(
				Hardware, Format, &format, &swizzle, &isYUVformat
				));

			/* SelectPipe(2D). */
			gcmERR_BREAK(gcoHARDWARE_SelectPipe(
				Hardware,
				0x1
				));

			/* LoadState(AQDE_PATTERN_MASK_LOW, 2), Mask. */
			gcmERR_BREAK(gcoHARDWARE_LoadState64(
				Hardware,
				0x01248, Mask
				));

			/* LoadState(AQDE_PATTERN_ADDRESS, 1), Address. */
			gcmERR_BREAK(gcoHARDWARE_LoadState32(
				Hardware,
				0x01238, Address
				));

			/* Setup pattern configuration. */
			config
				= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 18:16) - (0 ? 18:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16))) | (((gctUINT32) ((gctUINT32) (OriginX) & ((gctUINT32) ((((1 ? 18:16) - (0 ? 18:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 22:20) - (0 ? 22:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20))) | (((gctUINT32) ((gctUINT32) (OriginY) & ((gctUINT32) ((((1 ? 22:20) - (0 ? 22:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))
				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 28:24) - (0 ? 28:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:24) - (0 ? 28:24) + 1))))))) << (0 ? 28:24))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 28:24) - (0 ? 28:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:24) - (0 ? 28:24) + 1))))))) << (0 ? 28:24)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:6) - (0 ? 7:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ? 7:6))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 7:6) - (0 ? 7:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ? 7:6)));

			/* Set endian control */
			if (Hardware->bigEndian)
			{
				gctUINT32 bpp;

				/* Compute bits per pixel. */
				gcmERR_BREAK(gcoHARDWARE_ConvertFormat(Hardware,
													   Format,
													   &bpp,
													   gcvNULL));

				if (bpp == 16)
				{
					config |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
				}
				else if (bpp == 32)
				{
					config |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
				}
			}

			/* LoadState(AQDE_PATTERN_CONFIG, 1), cofig. */
			gcmERR_BREAK(gcoHARDWARE_LoadState32(
				Hardware,
				0x0123C, config
				));
		}
		else
		{
			/* Pattern is not currently supported by the software renderer. */
			gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
		}
	}
	while (gcvFALSE);

	/* Return the status. */
	gcmFOOTER();
	return status;
}

