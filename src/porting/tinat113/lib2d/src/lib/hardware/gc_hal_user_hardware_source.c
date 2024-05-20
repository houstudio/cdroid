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
**	gcoHARDWARE_SetMonochromeSource
**
**	Configure color source for the PE 2.0 engine.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gctUINT8 MonoTransparency
**			This value is used in gcvSURF_SOURCE_MATCH transparency mode.  The
**			value can be either 0 or 1 and is compared against each mono pixel
**			to determine transparency of the pixel.  If the values found are
**          equal, the pixel is transparent; otherwise it is opaque.
**
**		gceSURF_MONOPACK DataPack
**			Determines how many horizontal pixels are there per each 32-bit
**			chunk of monochrome bitmap.  For example, if set to gcvSURF_PACKED8,
**			each 32-bit chunk is 8-pixel wide, which also means that it defines
**			4 vertical lines of pixels.
**
**		gctBOOL CoordRelative
**			If gcvFALSE, the source origin represents absolute pixel coordinate
**			within the source surface.  If gcvTRUE, the source origin represents
**          the offset from the destination origin.
**
**		gctUINT32 FgColor32
**		gctUINT32 BgColor32
**			The values are used to represent foreground and background colors
**			of the source.  The values should be specified in A8R8G8B8 format.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_SetMonochromeSource(
	IN gcoHARDWARE Hardware,
	IN gctUINT8 MonoTransparency,
	IN gceSURF_MONOPACK DataPack,
	IN gctBOOL CoordRelative,
	IN gctUINT32 FgColor32,
	IN gctUINT32 BgColor32
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hardware=0x%x MonoTransparency=%d DataPack=%d "
					"CoordRelative=%d FgColor32=%x BgColor32=%x",
					Hardware, MonoTransparency, DataPack,
					CoordRelative, FgColor32, BgColor32);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		gctUINT32 datapack;

		/* Convert the data packing. */
		gcmERR_BREAK(gcoHARDWARE_TranslateMonoPack(
			DataPack, &datapack
			));

		if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
		{
			gctUINT32 config;
			gctUINT32 transparency;

			/* SelectPipe(2D). */
			gcmERR_BREAK(gcoHARDWARE_SelectPipe(
				Hardware,
				0x1
				));

			/* Get PE 1.0 transparency from new transparency modes. */
			gcmERR_BREAK(gcoHARDWARE_TranslateTransparencies(
				Hardware,
				Hardware->srcTransparency,
				Hardware->dstTransparency,
				Hardware->patTransparency,
				&transparency
				));

            /* LoadState(0x01200, 1), 0. */
			gcmERR_BREAK(gcoHARDWARE_LoadState32(
				Hardware,
				0x01200, 0
				));

			/* Setup source configuration. transparency field is obsolete for PE 2.0. */
			config
				= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)))
				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0))) | (((gctUINT32) (0xA & ((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))
				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 28:24) - (0 ? 28:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:24) - (0 ? 28:24) + 1))))))) << (0 ? 28:24))) | (((gctUINT32) (0x0A & ((gctUINT32) ((((1 ? 28:24) - (0 ? 28:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:24) - (0 ? 28:24) + 1))))))) << (0 ? 28:24)))
				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 13:12) - (0 ? 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12))) | (((gctUINT32) ((gctUINT32) (datapack) & ((gctUINT32) ((((1 ? 13:12) - (0 ? 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)))
				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4))) | (((gctUINT32) ((gctUINT32) (transparency) & ((gctUINT32) ((((1 ? 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6))) | (((gctUINT32) ((gctUINT32) (CoordRelative) & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)))

				| (MonoTransparency
					? ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:15) - (0 ? 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ? 15:15))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 15:15) - (0 ? 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ? 15:15)))
					: ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:15) - (0 ? 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ? 15:15))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 15:15) - (0 ? 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ? 15:15))));

			/* User can set transparency separately after setting source.
			   Save the source configuration for later use.  */
			Hardware->srcConfig = config;

			/* LoadState(AQDE_SRC_CONFIG, 1), config. */
			gcmERR_BREAK(gcoHARDWARE_LoadState32(
				Hardware,
				0x0120C, config
				));

			/* LoadState(AQDE_SRC_COLOR_BG, 1), BgColor. */
			gcmERR_BREAK(gcoHARDWARE_LoadState32(
				Hardware,
				0x01218, BgColor32
				));

			/* LoadState(AQDE_SRC_COLOR_FG, 1), FgColor. */
			gcmERR_BREAK(gcoHARDWARE_LoadState32(
				Hardware,
				0x0121C, FgColor32
				));
		}
		else
		{
			/* Monochrome operations are not currently supported. */
			gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
		}
	}
	while (gcvFALSE);

	/* Return the status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHARDWARE_SetColorSource
**
**	Configure color source.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**      gcsSURF_INFO_PTR Surface
**          Pointer to the source surface descriptor.
**
**		gctBOOL CoordRelative
**			If gcvFALSE, the source origin represents absolute pixel coordinate
**			within the source surface.  If gcvTRUE, the source origin represents
**          the offset from the destination origin.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_SetColorSource(
	IN gcoHARDWARE Hardware,
	IN gcsSURF_INFO_PTR Surface,
	IN gctBOOL CoordRelative
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hardware=0x%x Surface=0x%x CoordRelative=%d",
					Hardware, Surface, CoordRelative);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		gctUINT32 format, transparency, swizzle, isYUV;

		/* Convert the format. */
		gcmERR_BREAK(gcoHARDWARE_TranslateSourceFormat(
			Hardware, Surface->format, &format, &swizzle, &isYUV
			));

		/* Get PE 1.0 transparency from new transparency modes. */
		gcmERR_BREAK(gcoHARDWARE_TranslateTransparencies(
			Hardware,
			Hardware->srcTransparency,
			Hardware->dstTransparency,
			Hardware->patTransparency,
			&transparency
			));

		if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
		{
		    gctUINT32 data[4];
			gctUINT32 rotated = 0;

			/* Determine color swizzle. */
			gctUINT32 rgbaSwizzle, uvSwizzle;

			if (isYUV)
			{
				rgbaSwizzle = 0x0;
				uvSwizzle   = swizzle;
			}
			else
			{
				rgbaSwizzle = swizzle;
				uvSwizzle   = 0x0;
			}

			if (Hardware->fullBitBlitRotation)
			{
				rotated = gcvFALSE;
			}
			else
			{
				/* Determine 90 degree rotation enable field. */
				if (Surface->rotation == gcvSURF_0_DEGREE)
				{
					rotated = gcvFALSE;
				}
				else if (Surface->rotation == gcvSURF_90_DEGREE)
				{
					rotated = gcvTRUE;
				}
				else
				{
					gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
				}
			}

			/* 0x01200 */
			data[0]
				= Surface->node.physical;

			/* 0x01204 */
			data[1]
				= Surface->stride;

			/* 0x01208 */
			data[2]
				= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (Surface->alignedWidth) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))) | (((gctUINT32) ((gctUINT32) (rotated) & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));

			/* 0x0120C; transparency field is obsolete for PE 2.0. */
			data[3]
				= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)))
				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4))) | (((gctUINT32) ((gctUINT32) (transparency) & ((gctUINT32) ((((1 ? 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 28:24) - (0 ? 28:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:24) - (0 ? 28:24) + 1))))))) << (0 ? 28:24))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 28:24) - (0 ? 28:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:24) - (0 ? 28:24) + 1))))))) << (0 ? 28:24)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) | (((gctUINT32) ((gctUINT32) (rgbaSwizzle) & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6))) | (((gctUINT32) ((gctUINT32) (CoordRelative) & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));

			/* Set endian control */
			if (Hardware->bigEndian)
			{
				gctUINT32 bpp;

				/* Compute bits per pixel. */
				gcmERR_BREAK(gcoHARDWARE_ConvertFormat(Hardware,
													   Surface->format,
													   &bpp,
													   gcvNULL));

				if (bpp == 16)
				{
					data[3] |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
				}
				else if (bpp == 32)
				{
					data[3] |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
				}
			}

			/* User can set transparency separately after setting source.
			   Save the source configuration for later use.  */
			Hardware->srcConfig = data[3];

			/* SelectPipe(2D). */
			gcmERR_BREAK(gcoHARDWARE_SelectPipe(
				Hardware,
				0x1
				));

			/* Load source states. */
			gcmERR_BREAK(gcoHARDWARE_LoadState(
				Hardware,
				0x01200, 4,
				data
				));

		if (Hardware->fullBitBlitRotation)
		{
			gctUINT32 srcRot = 0;
			gctUINT32 value;

			switch (Surface->rotation)
			{
			case gcvSURF_0_DEGREE:
				srcRot = 0x0;
				break;

			case gcvSURF_90_DEGREE:
				srcRot = 0x4;
				break;

			case gcvSURF_180_DEGREE:
				srcRot = 0x5;
				break;

			case gcvSURF_270_DEGREE:
				srcRot = 0x6;
				break;

			default:
				status = gcvSTATUS_NOT_SUPPORTED;
				break;
			}

			/* Check errors. */
			gcmERR_BREAK(status);

			/* Flush the 2D pipe before writing to the rotation register. */
			gcmERR_BREAK(
				gcoHARDWARE_FlushPipe(Hardware));

			/* Load source height. */
			gcmERR_BREAK(gcoHARDWARE_LoadState32(
				Hardware,
				0x012B8,
				((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (Surface->alignedHeight) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

				));

			/* 0x012BC */
			if (Hardware->shadowRotAngleReg)
			{
				value = ((((gctUINT32) (Hardware->rotAngleRegShadow)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))) | (((gctUINT32) ((gctUINT32) (srcRot) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)));

				/* Save the shadow value. */
				Hardware->rotAngleRegShadow = value;
			}
			else
			{
				value =	((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))) | (((gctUINT32) ((gctUINT32) (srcRot) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))

							| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8))) | (((gctUINT32) (0x0&((gctUINT32)((((1?8:8)-(0?8:8)+1)==32)?~0:(~(~0<<((1?8:8)-(0?8:8)+1)))))))<<(0?8:8)))
							| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9)));
			}

			gcmERR_BREAK(gcoHARDWARE_LoadState32(
						Hardware,
						0x012BC,
						value
						));
		}

			/* Load source UV swizzle state. */
			gcmERR_BREAK(gcoHARDWARE_LoadState32(
				Hardware,
				0x012D8,
				(    ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) | (((gctUINT32) ((gctUINT32) ((uvSwizzle)) & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7))) | (((gctUINT32) (0x0&((gctUINT32)((((1?7:7)-(0?7:7)+1)==32)?~0:(~(~0<<((1?7:7)-(0?7:7)+1)))))))<<(0?7:7))))
				));
		}
		else
		{
			/* Current software renderer support is limited. */
			if (CoordRelative ||
				(Surface->rotation != gcvSURF_0_DEGREE) ||
				((transparency != gcvSURF_OPAQUE) &&
				 (transparency != gcvSURF_SOURCE_MATCH)))
			{
				gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
			}
		}

		/* Save states. */
		Hardware->sourceSurface = *Surface;
	}
	while (gcvFALSE);

	/* Return the status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHARDWARE_SetMaskedSource
**
**	Configure masked color source.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**      gcsSURF_INFO_PTR Surface
**          Pointer to the source surface descriptor.
**
**		gctBOOL CoordRelative
**			If gcvFALSE, the source origin represents absolute pixel coordinate
**			within the source surface.  If gcvTRUE, the source origin represents
**          the offset from the destination origin.
**
**		gceSURF_MONOPACK MaskPack
**			Determines how many horizontal pixels are there per each 32-bit
**			chunk of monochrome mask.  For example, if set to gcvSURF_PACKED8,
**			each 32-bit chunk is 8-pixel wide, which also means that it defines
**			4 vertical lines of pixel mask.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_SetMaskedSource(
	IN gcoHARDWARE Hardware,
	IN gcsSURF_INFO_PTR Surface,
	IN gctBOOL CoordRelative,
	IN gceSURF_MONOPACK MaskPack
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hardware=0x%x Surface=0x%x CoordRelative=%d MaskPack=%d",
					Hardware, Surface, CoordRelative, MaskPack);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		gctUINT32 format, transparency, swizzle, isYUV, maskpack;

		/* Convert the format. */
		gcmERR_BREAK(gcoHARDWARE_TranslateSourceFormat(
			Hardware, Surface->format, &format, &swizzle, &isYUV
			));

		/* Convert the data packing. */
		gcmERR_BREAK(gcoHARDWARE_TranslateMonoPack(MaskPack, &maskpack));

		/* Get PE 1.0 transparency from new transparency modes. */
		gcmERR_BREAK(gcoHARDWARE_TranslateTransparencies(
			Hardware,
			Hardware->srcTransparency,
			Hardware->dstTransparency,
			Hardware->patTransparency,
			&transparency
			));

		if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
		{
			gctUINT32 data[4];

			/* Determine color swizzle. */
			gctUINT32 rgbaSwizzle;

			if (isYUV)
			{
				rgbaSwizzle = 0x0;
			}
			else
			{
				rgbaSwizzle = swizzle;
			}

			if (!Hardware->fullBitBlitRotation &&
				Surface->rotation != gcvSURF_0_DEGREE)
			{
				gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
			}

			/* SelectPipe(2D). */
			gcmERR_BREAK(gcoHARDWARE_SelectPipe(
				Hardware,
				0x1
				));

			/* 0x01200 */
			data[0]
				= Surface->node.physical;

			/* 0x01204 */
			data[1]
				= Surface->stride;

			/* 0x01208 */
			data[2]
				= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (Surface->alignedWidth) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));

			/* AQDE_SRC_CONFIG_Address. transparency field is obsolete for PE 2.0. */
			data[3]
				= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)))
				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4))) | (((gctUINT32) ((gctUINT32) (transparency) & ((gctUINT32) ((((1 ? 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 28:24) - (0 ? 28:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:24) - (0 ? 28:24) + 1))))))) << (0 ? 28:24))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 28:24) - (0 ? 28:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:24) - (0 ? 28:24) + 1))))))) << (0 ? 28:24)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) | (((gctUINT32) ((gctUINT32) (rgbaSwizzle) & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 13:12) - (0 ? 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12))) | (((gctUINT32) ((gctUINT32) (maskpack) & ((gctUINT32) ((((1 ? 13:12) - (0 ? 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6))) | (((gctUINT32) ((gctUINT32) (CoordRelative) & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));

			/* Set endian control */
			if (Hardware->bigEndian)
			{
				gctUINT32 bpp;

				/* Compute bits per pixel. */
				gcmERR_BREAK(gcoHARDWARE_ConvertFormat(Hardware,
													   Surface->format,
													   &bpp,
													   gcvNULL));

				if (bpp == 16)
				{
					data[3] |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
				}
				else if (bpp == 32)
				{
					data[3] |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
				}
			}

			/* User can set transparency separately after setting source.
			   Save the source configuration for later use.  */
			Hardware->srcConfig = data[3];

			/* SelectPipe(2D). */
			gcmERR_BREAK(gcoHARDWARE_SelectPipe(
				Hardware,
				0x1
				));

			/* Load source states. */
			gcmERR_BREAK(gcoHARDWARE_LoadState(
				Hardware,
				0x01200, 4,
				data
				));

			if (Hardware->fullBitBlitRotation)
			{
				gctUINT32 srcRot = 0;
				gctUINT32 value;

				switch (Surface->rotation)
				{
				case gcvSURF_0_DEGREE:
					srcRot = 0x0;
					break;

				case gcvSURF_90_DEGREE:
					srcRot = 0x4;
					break;

				case gcvSURF_180_DEGREE:
					srcRot = 0x5;
					break;

				case gcvSURF_270_DEGREE:
					srcRot = 0x6;
					break;

				default:
					status = gcvSTATUS_NOT_SUPPORTED;
					break;
				}

				/* Check errors. */
				gcmERR_BREAK(status);

				/* Flush the 2D pipe before writing to the rotation register. */
				gcmERR_BREAK(
					gcoHARDWARE_FlushPipe(Hardware));

				/* Load source height. */
				gcmERR_BREAK(gcoHARDWARE_LoadState32(
					Hardware,
					0x012B8,
					((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (Surface->alignedHeight) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

					));

				/* 0x012BC */
				if (Hardware->shadowRotAngleReg)
				{
					value = ((((gctUINT32) (Hardware->rotAngleRegShadow)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))) | (((gctUINT32) ((gctUINT32) (srcRot) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)));

					/* Save the shadow value. */
					Hardware->rotAngleRegShadow = value;
				}
				else
				{
					value =	((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))) | (((gctUINT32) ((gctUINT32) (srcRot) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))

								| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8))) | (((gctUINT32) (0x0&((gctUINT32)((((1?8:8)-(0?8:8)+1)==32)?~0:(~(~0<<((1?8:8)-(0?8:8)+1)))))))<<(0?8:8)))
								| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9)));
				}

				gcmERR_BREAK(gcoHARDWARE_LoadState32(
							Hardware,
							0x012BC,
							value
							));
			}
		}
		else
		{
			/* Masked source is not currently supported by
			   the software renderer. */
			gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
		}
	}
	while (gcvFALSE);

	/* Return the status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHARDWARE_SetSource
**
**	Setup the source rectangle.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gcsRECT_PTR SrcRect
**			Pointer to a valid source rectangle.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_SetSource(
	IN gcoHARDWARE Hardware,
	IN gcsRECT_PTR SrcRect
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hardware=0x%x SrcRect=0x%x", Hardware, SrcRect);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
	gcmVERIFY_ARGUMENT(SrcRect != gcvNULL);

	do
	{
		if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
		{
		    gctUINT32 data[2];

			/* SelectPipe(2D). */
			gcmERR_BREAK(gcoHARDWARE_SelectPipe(
				Hardware,
				0x1
				));

			/* 0x01210 */
			data[0]
				= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (SrcRect->left) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (SrcRect->top) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

			/* 0x01214 */
			data[1]
				= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (SrcRect->right-SrcRect->left) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (SrcRect->bottom-SrcRect->top) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

			/* LoadState(AQDE_SRC_ORIGIN, 2), origin, size. */
			gcmERR_BREAK(gcoHARDWARE_LoadState(
				Hardware,
				0x01210, 2,
				data
				));
		}
		else
		{
			/* Store source rectangle for the software renderer. */
			Hardware->sourceRect = *SrcRect;

			/* Success. */
			status = gcvSTATUS_OK;
		}
	}
	while (gcvFALSE);

	/* Return the status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHARDWARE_SetOriginFraction
**
**	Setup the fraction of the source origin for filter blit.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gctUINT16 HorFraction
**		gctUINT16 VerFraction
**			Source origin fractions.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_SetOriginFraction(
	IN gcoHARDWARE Hardware,
	IN gctUINT16 HorFraction,
	IN gctUINT16 VerFraction
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hardware=0x%x HorFraction=%d VerFraction=%d",
					Hardware, HorFraction, VerFraction);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
		{
			/* SelectPipe(2D). */
			gcmERR_BREAK(gcoHARDWARE_SelectPipe(
				Hardware,
				0x1
				));

			/* LoadState(AQDE_SRC_ORIGIN_FRACTION, HorFraction, VerFraction. */
			gcmERR_BREAK(gcoHARDWARE_LoadState32(
				Hardware,
				0x01278,
				  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (HorFraction) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (VerFraction) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))

				));
		}
		else
		{
			/* Not supported by the software renderer. */
			gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
		}
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHARDWARE_LoadPalette
**
**	Load 256-entry color table for INDEX8 source surfaces.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to the gcoHARDWARE object.
**
**		gctUINT FirstIndex
**			The index to start loading from (0..255).
**
**		gctUINT IndexCount
**			The number of indices to load (FirstIndex + IndexCount <= 256).
**
**		gctPOINTER ColorTable
**			Pointer to the color table to load. The value of the pointer should
**			be set to the first value to load no matter what the value of
**			FirstIndex is. The table must consist of 32-bit entries that contain
**			color values in either ARGB8 or the destination color format
**			(see ColorConvert). For PE 2.0, ColorTable values need to be in ARGB8.
**
**		gctBOOL ColorConvert
**			If set to gcvTRUE, the 32-bit values in the table are assumed to be
**			in ARGB8 format.
**			If set to gcvFALSE, the 32-bit values in the table are assumed to be
**			in destination format.
**			For old PE, the palette is assumed to be in destination format.
**			For new PE, the palette is assumed to be in ARGB8 format.
**          Thus, it is recommended to pass the palette accordingly, to avoid
**			internal color conversion.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_LoadPalette(
	IN gcoHARDWARE Hardware,
	IN gctUINT FirstIndex,
	IN gctUINT IndexCount,
	IN gctPOINTER ColorTable,
	IN gctBOOL ColorConvert
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hardware=0x%x FirstIndex=%d IndexCount=%d "
					"ColorTable=0x%x ColorConvert=%d",
					Hardware, FirstIndex, IndexCount,
					ColorTable, ColorConvert);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
		{
			gctUINT32 address;
			gctUINT32_PTR memory;

			if ((Hardware->hw2DPE20 && (ColorConvert == gcvFALSE)) ||
				(!Hardware->hw2DPE20 && (ColorConvert == gcvTRUE)))
			{
				/* Save the palette and related states.
				   They will be used along with destination
				   format when a 2D operation is performed. */
				if (Hardware->patternTable == gcvNULL)
				{
					gcmERR_BREAK(gcoOS_Allocate(
						Hardware->os,
						sizeof(gctUINT32)*256,
						(gctPOINTER*) &Hardware->patternTable
						));
				}

				gcmVERIFY_OK(gcoOS_MemCopy(Hardware->patternTable, ColorTable, IndexCount * 4));
				Hardware->patternTableIndexCount = IndexCount;
				Hardware->patternTableFirstIndex = FirstIndex;
				Hardware->patternTableProgram = gcvTRUE;

				gcmFOOTER_NO();
				return gcvSTATUS_OK;
			}

			/* SelectPipe(2D). */
			gcmERR_BREAK(gcoHARDWARE_SelectPipe(
				Hardware,
				0x1
				));

			/* Reserve space in the command buffer. */
			gcmERR_BREAK(gcoBUFFER_Reserve(
				Hardware->buffer,
				4 + IndexCount * 4,
				gcvTRUE,
				gcvNULL,
				(gctPOINTER *) &memory
				));

			/* Determine first address. */
			if (Hardware->hw2DPE20)
			{
				address = 0x0D00 + FirstIndex;
			}
			else
			{
				address = 0x0700 + FirstIndex;
			}

			/* Assemble the load state command. */
			memory[0]
				= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (IndexCount) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

			/* Copy the color table. */
			gcmVERIFY_OK(gcoOS_MemCopy(&memory[1], ColorTable, IndexCount * 4));
		}
		else
		{
			/* Not supported by the software renderer. */
			gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
		}
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHARDWARE_SetSourceGlobalColor
**
**	Setup the source global color value in ARGB8 format.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gctUINT32 Color
**			Source color.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_SetSourceGlobalColor(
	IN gcoHARDWARE Hardware,
	IN gctUINT32 Color
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hardware=0x%x Color=%x", Hardware, Color);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
		{
			if (Hardware->hw2DPE20)
			{
				/* SelectPipe(2D). */
				gcmERR_BREAK(gcoHARDWARE_SelectPipe(
					Hardware,
					0x1
					));

				/* LoadState global color value. */
				gcmERR_BREAK(gcoHARDWARE_LoadState32(
					Hardware,
					0x012C8,
					Color
					));
			}
			else
			{
				Hardware->globalSrcColor = Color;
				status = gcvSTATUS_OK;
			}
		}
		else
		{
			/* Not supported by the software renderer. */
			gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
		}
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHARDWARE_SetTargetGlobalColor
**
**	Setup the target global color value in ARGB8 format.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gctUINT32 Color
**			Target color.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_SetTargetGlobalColor(
	IN gcoHARDWARE Hardware,
	IN gctUINT32 Color
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hardware=0x%x Color=%x", Hardware, Color);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
		{
			if (Hardware->hw2DPE20)
			{
				/* SelectPipe(2D). */
				gcmERR_BREAK(gcoHARDWARE_SelectPipe(
					Hardware,
					0x1
					));

				/* LoadState global color value. */
				gcmERR_BREAK(gcoHARDWARE_LoadState32(
					Hardware,
					0x012CC,
					Color
					));
			}
			else
			{
				Hardware->globalTargetColor = Color;
				status = gcvSTATUS_OK;
			}
		}
		else
		{
			/* Not supported by the software renderer. */
			gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
		}
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;

}

/*******************************************************************************
**
**	gcoHARDWARE_SetMultiplyModes
**
**	Setup the source and target pixel multiply modes.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gce2D_PIXEL_COLOR_MULTIPLY_MODE SrcPremultiplySrcAlpha
**			Source color premultiply with Source Alpha.
**
**		gce2D_PIXEL_COLOR_MULTIPLY_MODE DstPremultiplyDstAlpha
**			Destination color premultiply with Destination Alpha.
**
**		gce2D_GLOBAL_COLOR_MULTIPLY_MODE SrcPremultiplyGlobalMode
**			Source color premultiply with Global color's Alpha.
**
**		gce2D_PIXEL_COLOR_MULTIPLY_MODE DstDemultiplyDstAlpha
**			Destination color demultiply with Destination Alpha.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_SetMultiplyModes(
	IN gcoHARDWARE Hardware,
	IN gce2D_PIXEL_COLOR_MULTIPLY_MODE SrcPremultiplySrcAlpha,
	IN gce2D_PIXEL_COLOR_MULTIPLY_MODE DstPremultiplyDstAlpha,
	IN gce2D_GLOBAL_COLOR_MULTIPLY_MODE SrcPremultiplyGlobalMode,
	IN gce2D_PIXEL_COLOR_MULTIPLY_MODE DstDemultiplyDstAlpha
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hardware=0x%x SrcPremultiplySrcAlpha=%d DstPremultiplyDstAlpha=%d "
					"SrcPremultiplyGlobalMode=%d DstDemultiplyDstAlpha=%d",
					Hardware, SrcPremultiplySrcAlpha, DstPremultiplyDstAlpha,
					SrcPremultiplyGlobalMode, DstDemultiplyDstAlpha);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		if (Hardware->hw2DEngine && Hardware->hw2DPE20 && !Hardware->sw2DEngine)
		{
			gctUINT32 srcPremultiplySrcAlpha;
			gctUINT32 dstPremultiplyDstAlpha;
			gctUINT32 srcPremultiplyGlobalMode;
			gctUINT32 dstDemultiplyDstAlpha;

			/* Convert the multiply modes. */
			gcmERR_BREAK(gcoHARDWARE_PixelColorMultiplyMode(
				SrcPremultiplySrcAlpha, &srcPremultiplySrcAlpha
				));

			gcmERR_BREAK(gcoHARDWARE_PixelColorMultiplyMode(
				DstPremultiplyDstAlpha, &dstPremultiplyDstAlpha
				));

			gcmERR_BREAK(gcoHARDWARE_GlobalColorMultiplyMode(
				SrcPremultiplyGlobalMode, &srcPremultiplyGlobalMode
				));

			gcmERR_BREAK(gcoHARDWARE_PixelColorMultiplyMode(
				DstDemultiplyDstAlpha, &dstDemultiplyDstAlpha
				));

			/* SelectPipe(2D). */
			gcmERR_BREAK(gcoHARDWARE_SelectPipe(
				Hardware,
				0x1
				));

			/* LoadState pixel multiply modes. */
			gcmERR_BREAK(gcoHARDWARE_LoadState32(
				Hardware,
				0x012D0,
				  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) | (((gctUINT32) ((gctUINT32) (srcPremultiplySrcAlpha) & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) | (((gctUINT32) ((gctUINT32) (dstPremultiplyDstAlpha) & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8))) | (((gctUINT32) ((gctUINT32) (srcPremultiplyGlobalMode) & ((gctUINT32) ((((1 ? 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ? 20:20))) | (((gctUINT32) ((gctUINT32) (dstDemultiplyDstAlpha) & ((gctUINT32) ((((1 ? 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ? 20:20)))

				));
		}
		else
		{
			/* Not supported by the PE1.0 hardware and software renderer. */
			gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
		}
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHARDWARE_SetTransparencyModes
**
**	Setup the source, target and pattern transparency modes.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gceSURF_TRANSPARENCY SrcTransparency
**			Source Transparency.
**
**		gceSURF_TRANSPARENCY DstTransparency
**			Destination Transparency.
**
**		gceSURF_TRANSPARENCY PatTransparency
**			Pattern Transparency.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_SetTransparencyModes(
	IN gcoHARDWARE Hardware,
	IN gce2D_TRANSPARENCY SrcTransparency,
	IN gce2D_TRANSPARENCY DstTransparency,
	IN gce2D_TRANSPARENCY PatTransparency
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hardware=0x%x SrcTransparency=%d DstTransparency=%d PatTransparency=%d",
					Hardware, SrcTransparency, DstTransparency, PatTransparency);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
		{
			if (Hardware->hw2DPE20)
			{
				gctUINT32 srcTransparency;
				gctUINT32 dstTransparency;
				gctUINT32 patTransparency;

				/* Convert the transparency modes. */
				gcmERR_BREAK(gcoHARDWARE_TranslateSourceTransparency(
					SrcTransparency, &srcTransparency
					));

				gcmERR_BREAK(gcoHARDWARE_TranslateDestinationTransparency(
					DstTransparency, &dstTransparency
					));

				gcmERR_BREAK(gcoHARDWARE_TranslatePatternTransparency(
					PatTransparency, &patTransparency
					));

				/* SelectPipe(2D). */
				gcmERR_BREAK(gcoHARDWARE_SelectPipe(
					Hardware,
					0x1
					));

				/* LoadState transparency modes.
				   Enable Source or Destination read when
				   respective Color key is turned on. */
				gcmERR_BREAK(gcoHARDWARE_LoadState32(
					Hardware,
					0x012D4,
					  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) | (((gctUINT32) ((gctUINT32) (srcTransparency) & ((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))

					| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8))) | (((gctUINT32) ((gctUINT32) (dstTransparency) & ((gctUINT32) ((((1 ? 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8)))

					| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4))) | (((gctUINT32) ((gctUINT32) (patTransparency) & ((gctUINT32) ((((1 ? 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)))

					|  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16))) | (((gctUINT32) ((gctUINT32) ((srcTransparency==0x2)) & ((gctUINT32) ((((1 ? 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16)))

					| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:24) - (0 ? 25:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ? 25:24))) | (((gctUINT32) ((gctUINT32) ((dstTransparency==0x2)) & ((gctUINT32) ((((1 ? 25:24) - (0 ? 25:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ? 25:24)))

					));
			}
			else
			{
				gctUINT32 transparency;
				gctUINT32 config;

				/* Save the transparency modes. */
				Hardware->srcTransparency = SrcTransparency;
				Hardware->dstTransparency = DstTransparency;
				Hardware->patTransparency = PatTransparency;

				/* Get PE 1.0 transparency from new transparency modes. */
				gcmERR_BREAK(gcoHARDWARE_TranslateTransparencies(
					Hardware,
					Hardware->srcTransparency,
					Hardware->dstTransparency,
					Hardware->patTransparency,
					&transparency
					));

				/* If user has already set source, then this code adds the
				   transparency setting onto it. Otherwise, the saved
				   transparencies above will be used later while setting source.
				*/
				config = ((((gctUINT32) (Hardware->srcConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4))) | (((gctUINT32) ((gctUINT32) (transparency) & ((gctUINT32) ((((1 ? 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));

				/* SelectPipe(2D). */
				gcmERR_BREAK(gcoHARDWARE_SelectPipe(
					Hardware,
					0x1
					));

				/* LoadState(AQDE_SRC_CONFIG, 1), config. */
				gcmERR_BREAK(gcoHARDWARE_LoadState32(
					Hardware,
					0x0120C, config
					));
			}
		}
		else
		{
			/* Store transparency states. */
			Hardware->srcTransparency = SrcTransparency;
			Hardware->dstTransparency = DstTransparency;
			Hardware->patTransparency = PatTransparency;
			status = gcvSTATUS_OK;
		}
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHARDWARE_SetAutoTransparency
**
**  Used for backward compatibility only.
**	Setup the source, target and pattern transparency modes from rop codes.
**  Old PE does this automatically. PE 2.0 needs to set the transparency
**  modes automatically when the user uses old API calls.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gctUINT8 FgRop
**			Foreground ROP to use with opaque pixels.
**
**		gctUINT8 BgRop
**			Background ROP to use with transparent pixels.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_SetAutoTransparency(
	IN gcoHARDWARE Hardware,
	IN gctUINT8 FgRop,
	IN gctUINT8 BgRop
	)
{
	gceSTATUS status = gcvSTATUS_OK;

	gcmHEADER_ARG("Hardware=0x%x FgRop=%x BgRop=%x",
					Hardware, FgRop, BgRop);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		if (Hardware->hw2DEngine && !Hardware->sw2DEngine && Hardware->hw2DPE20)
		{
			gctBOOL usePattern;

			/* Determine the resource usage. */
			gcoHARDWARE_Get2DResourceUsage(
				FgRop, BgRop,
				gcv2D_OPAQUE,
				gcvNULL, &usePattern, gcvNULL
				);

			if (usePattern)
			{
				/* Set transparency to pattern masked. */
				gcmERR_BREAK(gcoHARDWARE_SetTransparencyModes(
					Hardware,
					gcv2D_OPAQUE,
					gcv2D_OPAQUE,
					gcv2D_MASKED
					));
			}
		}
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHARDWARE_SetSourceColorKeyRange
**
**	Setup the source color key value in source format.
**  Source pixels matching specified color range become transparent.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gctUINT32 ColorLow
**		gctUINT32 ColorHigh
**			Transparency low and high color.
**
**		gctBOOL ColorPack
**			If set to gcvTRUE, the 32-bit values in the table are assumed to be
**			in ARGB8 format.
**			If set to gcvFALSE, the 32-bit values in the table are assumed to be
**			in source format.
**			For old API calls, the color key is assumed to be in source format.
**			For new PE, the color is in ARGB8 format and needs to be packed.
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_SetSourceColorKeyRange(
	IN gcoHARDWARE Hardware,
	IN gctUINT32 ColorLow,
	IN gctUINT32 ColorHigh,
	IN gctBOOL ColorPack
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hardware=0x%x ColorLow=%x ColorHigh=%x ColorPack=%d",
					Hardware, ColorLow, ColorHigh, ColorPack);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
		{
			if (!Hardware->hw2DPE20 && ColorPack)
			{
				/* Save color key for packing into source format for PE 1.0.
				   Source format is known only when gcoHARDWARE_StartDE is called. */
				gcmERR_BREAK(gcoHARDWARE_SaveTransparencyColor(
					Hardware,
					ColorLow
					));

				gcmFOOTER_NO();
				return gcvSTATUS_OK;
			}

			/* Disable the pending transparency settings. */
			Hardware->transparencyColorProgram = gcvFALSE;

			/* SelectPipe(2D). */
			gcmERR_BREAK(gcoHARDWARE_SelectPipe(
				Hardware,
				0x1
				));

			/* LoadState source color key. */
			gcmERR_BREAK(gcoHARDWARE_LoadState32(
				Hardware,
				0x01218,
				ColorLow
				));

			/* LoadState source color key. */
			gcmERR_BREAK(gcoHARDWARE_LoadState32(
				Hardware,
				0x012DC,
				ColorHigh
				));
		}
		else
		{
			if (ColorLow == ColorHigh)
			{
				/* Store transparency states. */
				Hardware->transparencyColor = ColorLow;
				gcmFOOTER_NO();
				return gcvSTATUS_OK;
			}

			/* Not supported by the software renderer. */
			gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
			return gcvSTATUS_NOT_SUPPORTED;
		}
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHARDWARE_YUVColorMode
**
**	Setup the YUV color space mode.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gceSURF_YUV_COLOR_SPACE Mode
**			Mode is either 601 or 709.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_YUVColorMode(
	IN gcoHARDWARE Hardware,
	IN gce2D_YUV_COLOR_MODE Mode
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hardware=0x%x Mode=%d", Hardware, Mode);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		if (Hardware->hw2DEngine && Hardware->hw2DPE20 && !Hardware->sw2DEngine)
		{
			gctUINT32 mode;

			/* Convert the YUV color mode. */
			gcmERR_BREAK(gcoHARDWARE_TranslateYUVColorMode(
				Mode, &mode
				));

			/* SelectPipe(2D). */
			gcmERR_BREAK(gcoHARDWARE_SelectPipe(
				Hardware,
				0x1
				));

			/* LoadState YUV color mode. */
			gcmERR_BREAK(gcoHARDWARE_LoadState32(
				Hardware,
				0x012D8,
				(    ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) | (((gctUINT32) ((gctUINT32) ((mode)) & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3))) | (((gctUINT32) (0x0&((gctUINT32)((((1?3:3)-(0?3:3)+1)==32)?~0:(~(~0<<((1?3:3)-(0?3:3)+1)))))))<<(0?3:3))))
				));
		}
		else
		{
			/* Not supported by the software renderer. */
			gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
		}
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHARDWARE_SaveMonoColors
**
**	Save colors for conversion to ARGB32 format.
**	Destination format is known only when gcoHARDWARE_StartDE is called.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gctUINT32 FgColor
**		gctUINT32 BgColor
**			The values are used to represent foreground and background
**			colors of the source.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_SaveMonoColors(
	IN gcoHARDWARE Hardware,
	IN gctUINT32 FgColor,
	IN gctUINT32 BgColor
	)
{
	gcmHEADER_ARG("Hardware=0x%x FgColor=%x BgColor=%x",
					Hardware, FgColor, BgColor);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	Hardware->fgColor = FgColor;
	Hardware->bgColor = BgColor;
	Hardware->monoColorProgram = gcvTRUE;

	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoHARDWARE_SaveTransparencyColor
**
**	Save colors for packing Color32 into source format.
**	Source format is known only when gcoHARDWARE_StartDE is called.
**  No color conversion is done in packing.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gctUINT32 Color32
**			The values are used to represent transparency color
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_SaveTransparencyColor(
	IN gcoHARDWARE Hardware,
	IN gctUINT32 Color32
	)
{
	gcmHEADER_ARG("Hardware=0x%x Color32=%x", Hardware, Color32);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	Hardware->transparencyColor = Color32;
	Hardware->transparencyColorProgram = gcvTRUE;

	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

