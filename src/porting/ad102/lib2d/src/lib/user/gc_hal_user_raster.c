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
**	2D support functions.
**
*/

#include "gc_hal_user_precomp.h"
#include "gc_hal_user_brush.h"

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE	gcvZONE_2D

/******************************************************************************\
********************************** Structures **********************************
\******************************************************************************/

struct _gco2D
{
	/* Object. */
	gcsOBJECT			object;

	/* Pointer to gcoHAL object. */
	gcoHAL				hal;
};


/******************************************************************************\
********************************* gco2D API Code ********************************
\******************************************************************************/

/*******************************************************************************
**
**	gco2D_Construct
**
**	Construct a new gco2D object.
**
**	INPUT:
**
**		gcoHAL Hal
**			Poniter to an gcoHAL object.
**
**	OUTPUT:
**
**		gco2D * Engine
**			Pointer to a variable that will hold the pointer to the gco2D object.
*/
gceSTATUS
gco2D_Construct(
	IN gcoHAL Hal,
	OUT gco2D * Engine
	)
{
	gceSTATUS status;
	gco2D engine = gcvNULL;

	gcmHEADER_ARG("Hal=0x%x", Hal);

	/* Verify the arguments. */
	gcmVERIFY_ARGUMENT(Hal != gcvNULL);
	gcmVERIFY_ARGUMENT(Engine != gcvNULL);

	do
	{
		/* Is 2D pipe available? */
		if (!gcoHARDWARE_Is2DAvailable(Hal->hardware))
		{
			status = gcvSTATUS_NOT_SUPPORTED;
			break;
		}

		/* Allocate the gco2D object. */
		gcmERR_BREAK(gcoOS_Allocate(
			Hal->os,
			sizeof(struct _gco2D),
			(gctPOINTER *) &engine
			));

		/* Initialize the gco2D object. */
		engine->object.type = gcvOBJ_2D;
		engine->hal = Hal;

		/* Return pointer to the gco2D object. */
		*Engine = engine;

		/* Success. */
		gcmFOOTER_ARG("*Engine=0x%x", *Engine);
		return gcvSTATUS_OK;
	}
	while (gcvFALSE);

	/* Roll back. */
	if (engine != gcvNULL)
	{
		gcmVERIFY_OK(gcoOS_Free(Hal->os, engine));
	}

	/* Success. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_Destroy
**
**	Destroy an gco2D object.
**
**	INPUT:
**
**		gco2D Engine
**			Poniter to an gco2D object to destroy.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_Destroy(
	IN gco2D Engine
	)
{
	gcmHEADER_ARG("Engine=0x%x", Engine);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	/* Mark the gco2D object as unknown. */
	Engine->object.type = gcvOBJ_UNKNOWN;

	/* Free the gco2D object. */
	gcmVERIFY_OK(gcoOS_Free(Engine->hal->os, Engine));

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gco2D_SetBrushLimit
**
**	Sets the maximum number of brushes in the cache.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT MaxCount
**			Maximum number of brushes allowed in the cache at the same time.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetBrushLimit(
	IN gco2D Engine,
	IN gctUINT MaxCount
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x MaxCount=%d", Engine, MaxCount);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	status = gcoHARDWARE_SetBrushLimit(
		Engine->hal->hardware,
		MaxCount
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_GetBrushCache
**
**	Return a pointer to the brush cache.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**	OUTPUT:
**
**		gcoBRUSH_CACHE * BrushCache
**			A pointer to gcoBRUSH_CACHE object.
*/
gceSTATUS
gco2D_GetBrushCache(
	IN gco2D Engine,
	IN OUT gcoBRUSH_CACHE * BrushCache
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x", Engine);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	status = gcoHARDWARE_GetBrushCache(
		Engine->hal->hardware,
		BrushCache
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_FlushBrush
**
**	Flush the brush.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gcoBRUSH Brush
**			A pointer to a valid gcoBRUSH object.
**
**		gceSURF_FORMAT Format
**			Format for destination surface when using color conversion.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_FlushBrush(
	IN gco2D Engine,
	IN gcoBRUSH Brush,
	IN gceSURF_FORMAT Format
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x Brush=0x%x Format=%d", Engine, Brush, Format);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	do
	{
		/* Program the destination format. */
		gcmERR_BREAK(gcoHARDWARE_SetTargetFormat(
			Engine->hal->hardware,
			Format
			));

		gcmERR_BREAK(gcoHARDWARE_FlushBrush(
			Engine->hal->hardware,
			Brush
		));
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_LoadSolidBrush
**
**	Program the specified solid color brush.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gceSURF_FORMAT Format
**			Format for destination surface when using color conversion.
**
**		gctUINT32 ColorConvert
**			The value of the Color parameter is stored directly in internal
**			color register and is used either directly to initialize pattern
**			or is converted to the format of destination before it is used.
**			The later happens if ColorConvert is not zero.
**
**		gctUINT32 Color
**			The color value of the pattern. The value will be used to
**			initialize 8x8 pattern. If the value is in destination format,
**			set ColorConvert to 0. Otherwise, provide the value in ARGB8
**			format and set ColorConvert to 1 to instruct the hardware to
**			convert the value to the destination format before it is
**			actually used.
**
**		gctUINT64 Mask
**			64 bits of mask, where each bit corresponds to one pixel of 8x8
**			pattern. Each bit of the mask is used to determine transparency
**			of the corresponding pixel, in other words, each mask bit is used
**			to select between foreground or background ROPs. If the bit is 0,
**			background ROP is used on the pixel; if 1, the foreground ROP
**			is used. The mapping between Mask parameter bits and actual
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
gceSTATUS
gco2D_LoadSolidBrush(
	IN gco2D Engine,
	IN gceSURF_FORMAT Format,
	IN gctUINT32 ColorConvert,
	IN gctUINT32 Color,
	IN gctUINT64 Mask
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x Format=%d ColorConvert=%d Color=%x Mask=%llx",
					Engine, Format, ColorConvert, Color, Mask);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	do
	{
		/* Program the destination format. */
		gcmERR_BREAK(gcoHARDWARE_SetTargetFormat(
			Engine->hal->hardware,
			Format
			));

		gcmERR_BREAK(gcoHARDWARE_LoadSolidColorPattern(
			Engine->hal->hardware,
			ColorConvert,
			Color,
			Mask
			));
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetMonochromeSource
**
**	Configure color source.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctBOOL ColorConvert
**			The values of FgColor and BgColor parameters are stored directly in
**			internal color registers and are used either directly as the source
**			color or converted to the format of destination before actually
**			used.  The later happens if ColorConvert is gcvTRUE.
**
**		gctUINT8 MonoTransparency
**			This value is used in gcvSURF_SOURCE_MATCH transparency mode.  The
**			value can be either 0 or 1 and is compared against each mono pixel
**          to determine transparency of the pixel.  If the values found are
**			equal, the pixel is transparent; otherwise it is opaque.
**
**		gceSURF_MONOPACK DataPack
**			Determines how many horizontal pixels are there per each 32-bit
**			chunk of monochrome bitmap.  For example, if set to gcvSURF_PACKED8,
**			each 32-bit chunk is 8-pixel wide, which also means that it defines
**			4 vertical lines of pixels.
**
**		gctBOOL CoordRelative
**			If gcvFALSE, the source origin represents absolute pixel coordinate
**			within the source surface. If gcvTRUE, the source origin represents the
**          offset from the destination origin.
**
**		gceSURF_TRANSPARENCY Transparency
**			gcvSURF_OPAQUE - each pixel of the bitmap overwrites the destination.
**			gcvSURF_SOURCE_MATCH - source pixels compared against register value
**				to determine the transparency.  In simple terms, the
**              transaprency comes down to selecting the ROP code to use.
**              Opaque pixels use foreground ROP and transparent ones use
**              background ROP.
**			gcvSURF_SOURCE_MASK - monochrome source mask defines transparency.
**			gcvSURF_PATTERN_MASK - pattern mask defines transparency.
**
**		gctUINT32 FgColor
**		gctUINT32 BgColor
**			The values are used to represent foreground and background colors
**			of the source.  If the values are in destination format, set
**			ColorConvert to gcvFALSE. Otherwise, provide the values in A8R8G8B8
**          format and set ColorConvert to gcvTRUE to instruct the hardware to
**          convert the values to the destination format before they are
**          actually used.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetMonochromeSource(
	IN gco2D Engine,
	IN gctBOOL ColorConvert,
	IN gctUINT8 MonoTransparency,
	IN gceSURF_MONOPACK DataPack,
	IN gctBOOL CoordRelative,
	IN gceSURF_TRANSPARENCY Transparency,
	IN gctUINT32 FgColor,
	IN gctUINT32 BgColor
	)
{
	gctUINT32 srcTransparency;
	gctUINT32 dstTransparency;
	gctUINT32 patTransparency;
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x ColorConvert=%d MonoTransparency=%d DataPack=%lld "
					"CoordRelative=%d Transparency=%d FgColor=%x BgColor=%x",
					Engine, ColorConvert, MonoTransparency, DataPack,
					CoordRelative, Transparency, FgColor, BgColor);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	do
	{
		gcmERR_BREAK(gcoHARDWARE_TranslateSurfTransparency(
			Transparency,
			&srcTransparency,
			&dstTransparency,
			&patTransparency
			));

		/* Set the transparency. */
		gcmERR_BREAK(gcoHARDWARE_SetTransparencyModes(
			Engine->hal->hardware,
			(gce2D_TRANSPARENCY) srcTransparency,
			(gce2D_TRANSPARENCY) dstTransparency,
			(gce2D_TRANSPARENCY) patTransparency
			));

		if ( ColorConvert == gcvFALSE )
		{
			/* Save colors for conversion to ARGB32 format for PE 2.0.
			   Destination format is known only when
			   gcoHARDWARE_StartDE is called. */
			gcmERR_BREAK(gcoHARDWARE_SaveMonoColors(
				Engine->hal->hardware,
				FgColor,
				BgColor
				));
		}

		/* Set the source. */
		gcmERR_BREAK(gcoHARDWARE_SetMonochromeSource(
			Engine->hal->hardware,
			MonoTransparency,
			DataPack,
			CoordRelative,
			FgColor,
			BgColor
			));
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetColorSource
**
**	Configure color source.
**
**  This function is only working with old PE (<2.0).
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT32 Address
**			Source surface base address.
**
**		gctUINT32 Stride
**			Stride of the source surface in bytes.
**
**		gceSURF_FORMAT Format
**			Color format of the source surface.
**
**		gceSURF_ROTATION Rotation
**			Type of rotation.
**
**		gctUINT32 SurfaceWidth
**			Required only if the surface is rotated. Equal to the width
**			of the source surface in pixels.
**
**		gctBOOL CoordRelative
**			If gcvFALSE, the source origin represents absolute pixel coordinate
**			within the source surface.  If gcvTRUE, the source origin represents
**          the offset from the destination origin.
**
**		gceSURF_TRANSPARENCY Transparency
**			gcvSURF_OPAQUE - each pixel of the bitmap overwrites the destination.
**			gcvSURF_SOURCE_MATCH - source pixels compared against register value
**				to determine the transparency.  In simple terms, the
**              transaprency comes down to selecting the ROP code to use.
**              Opaque pixels use foreground ROP and transparent ones use
**              background ROP.
**			gcvSURF_SOURCE_MASK - monochrome source mask defines transparency.
**			gcvSURF_PATTERN_MASK - pattern mask defines transparency.
**
**		gctUINT32 TransparencyColor
**			This value is used in gcvSURF_SOURCE_MATCH transparency mode.  The
**          value is compared against each pixel to determine transparency of
**			the pixel.  If the values found are equal, the pixel is transparent;
**			otherwise it is opaque.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetColorSource(
	IN gco2D Engine,
	IN gctUINT32 Address,
	IN gctUINT32 Stride,
	IN gceSURF_FORMAT Format,
	IN gceSURF_ROTATION Rotation,
	IN gctUINT32 SurfaceWidth,
	IN gctBOOL CoordRelative,
	IN gceSURF_TRANSPARENCY Transparency,
	IN gctUINT32 TransparencyColor
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x Address=%x Stride=%d Format=%d Rotation=%d "
					"SurfaceWidth=%d CoordRelative=%d Transparency=%d "
					"TransparencyColor=%x",
					Engine, Address, Stride, Format, Rotation,
					SurfaceWidth, CoordRelative, Transparency, TransparencyColor);

	if ((Rotation != gcvSURF_0_DEGREE) && (Rotation != gcvSURF_90_DEGREE))
	{
		gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
		return gcvSTATUS_NOT_SUPPORTED;
	}

	/* Forward to gco2D_SetColorSourceEx with the SurfaceHeight set to 0. */
	status = gco2D_SetColorSourceEx(
			Engine,
			Address,
			Stride,
			Format,
			Rotation,
			SurfaceWidth,
			0,
			CoordRelative,
			Transparency,
			TransparencyColor);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetColorSourceEx
**
**	Configure color source.
**
**	This function is only working with old PE (<2.0).
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT32 Address
**			Source surface base address.
**
**		gctUINT32 Stride
**			Stride of the source surface in bytes.
**
**		gceSURF_FORMAT Format
**			Color format of the source surface.
**
**		gceSURF_ROTATION Rotation
**			Type of rotation.
**
**		gctUINT32 SurfaceWidth
**			Required only if the surface is rotated. Equal to the width
**			of the source surface in pixels.
**
**		gctUINT32 SurfaceHeight
**			Required only if the surface is rotated in PE2.0. Equal to the height
**			of the source surface in pixels.
**
**		gctBOOL CoordRelative
**			If gcvFALSE, the source origin represents absolute pixel coordinate
**			within the source surface.  If gcvTRUE, the source origin represents
**          the offset from the destination origin.
**
**		gceSURF_TRANSPARENCY Transparency
**			gcvSURF_OPAQUE - each pixel of the bitmap overwrites the destination.
**			gcvSURF_SOURCE_MATCH - source pixels compared against register value
**				to determine the transparency.  In simple terms, the
**              transaprency comes down to selecting the ROP code to use.
**              Opaque pixels use foreground ROP and transparent ones use
**              background ROP.
**			gcvSURF_SOURCE_MASK - monochrome source mask defines transparency.
**			gcvSURF_PATTERN_MASK - pattern mask defines transparency.
**
**		gctUINT32 TransparencyColor
**			This value is used in gcvSURF_SOURCE_MATCH transparency mode.  The
**          value is compared against each pixel to determine transparency of
**			the pixel.  If the values found are equal, the pixel is transparent;
**			otherwise it is opaque.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetColorSourceEx(
	IN gco2D Engine,
	IN gctUINT32 Address,
	IN gctUINT32 Stride,
	IN gceSURF_FORMAT Format,
	IN gceSURF_ROTATION Rotation,
	IN gctUINT32 SurfaceWidth,
	IN gctUINT32 SurfaceHeight,
	IN gctBOOL CoordRelative,
	IN gceSURF_TRANSPARENCY Transparency,
	IN gctUINT32 TransparencyColor
	)
{
	gcsSURF_INFO surface;
	gctUINT32 srcTransparency;
	gctUINT32 dstTransparency;
	gctUINT32 patTransparency;
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x Address=%x Stride=%d Format=%d Rotation=%d "
					"SurfaceWidth=%d SurfaceHeight=%d CoordRelative=%d "
					"Transparency=%d TransparencyColor=%x",
					Engine, Address, Stride, Format, Rotation,
					SurfaceWidth, SurfaceHeight, CoordRelative,
					Transparency, TransparencyColor);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	/* Fill in the structure. */
	gcoOS_ZeroMemory(&surface, gcmSIZEOF(surface));
	surface.type          = gcvSURF_BITMAP;
	surface.format        = Format;
	surface.alignedWidth  = SurfaceWidth;
	surface.alignedHeight  = SurfaceHeight;
	surface.rotation      = Rotation;
	surface.stride        = Stride;
	surface.node.physical = Address;

	do
	{
		gcmERR_BREAK(gcoHARDWARE_TranslateSurfTransparency(
			Transparency,
			&srcTransparency,
			&dstTransparency,
			&patTransparency
			));

		/* Set the transparency. */
		gcmERR_BREAK(gcoHARDWARE_SetTransparencyModes(
			Engine->hal->hardware,
			(gce2D_TRANSPARENCY) srcTransparency,
			(gce2D_TRANSPARENCY) dstTransparency,
			(gce2D_TRANSPARENCY) patTransparency
			));

		/* Set the transparency color. */
		gcmERR_BREAK(gcoHARDWARE_SetSourceColorKeyRange(
			Engine->hal->hardware,
			TransparencyColor,
			TransparencyColor,
			gcvFALSE
			));

		/* Set the source. */
		gcmERR_BREAK(gcoHARDWARE_SetColorSource(
			Engine->hal->hardware,
			&surface,
			CoordRelative
			));
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetColorSourceAdvanced
**
**	Configure color source.
**
**  This function is only working with PE 2.0 and above.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gcoSURF Source
**			Source surface.
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
gceSTATUS
gco2D_SetColorSourceAdvanced(
	IN gco2D Engine,
	IN gctUINT32 Address,
	IN gctUINT32 Stride,
	IN gceSURF_FORMAT Format,
	IN gceSURF_ROTATION Rotation,
	IN gctUINT32 SurfaceWidth,
	IN gctUINT32 SurfaceHeight,
	IN gctBOOL CoordRelative
	)
{
	gceSTATUS status;
	gcsSURF_INFO surface;

	gcmHEADER_ARG("Engine=0x%x Address=%x Stride=%d Format=%d Rotation=%d "
					"SurfaceWidth=%d SurfaceHeight=%d CoordRelative=%d ",
					Engine, Address, Stride, Format, Rotation,
					SurfaceWidth, SurfaceHeight, CoordRelative);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	/* Fill in the structure. */
	gcoOS_ZeroMemory(&surface, gcmSIZEOF(surface));
	surface.type          = gcvSURF_BITMAP;
	surface.format        = Format;
	surface.alignedWidth  = SurfaceWidth;
	surface.alignedHeight = SurfaceHeight;
	surface.rotation      = Rotation;
	surface.stride        = Stride;
	surface.node.physical = Address;

	status = gcoHARDWARE_SetColorSource(
		Engine->hal->hardware,
		&surface,
		CoordRelative
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetMaskedSource
**
**	Configure masked color source.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT32 Address
**			Source surface base address.
**
**		gctUINT32 Stride
**			Stride of the source surface in bytes.
**
**		gceSURF_FORMAT Format
**			Color format of the source surface.
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
gceSTATUS
gco2D_SetMaskedSource(
	IN gco2D Engine,
	IN gctUINT32 Address,
	IN gctUINT32 Stride,
	IN gceSURF_FORMAT Format,
	IN gctBOOL CoordRelative,
	IN gceSURF_MONOPACK MaskPack
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x Address=%x Stride=%d Format=%d CoordRelative=%d MaskPack=%d",
					Engine, Address, Stride, Format, CoordRelative, MaskPack);

	/* Forward to gco2D_SetMaskedSourceEx with the Rotation set to gcvSURF_0_DEGREE,
	the SurfaceWidth set to 0 and SurfaceHeight set to 0. */
	status = gco2D_SetMaskedSourceEx(
			Engine,
			Address,
			Stride,
			Format,
			CoordRelative,
			MaskPack,
			gcvSURF_0_DEGREE,
			0,
			0
			);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetMaskedSourceEx
**
**	Configure masked color source.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT32 Address
**			Source surface base address.
**
**		gctUINT32 Stride
**			Stride of the source surface in bytes.
**
**		gceSURF_FORMAT Format
**			Color format of the source surface.
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
**		gceSURF_ROTATION Rotation
**			Type of rotation in PE2.0.
**
**		gctUINT32 SurfaceWidth
**			Required only if the surface is rotated in PE2.0. Equal to the width
**			of the source surface in pixels.
**
**		gctUINT32 SurfaceHeight
**			Required only if the surface is rotated in PE2.0. Equal to the height
**			of the source surface in pixels.
**
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetMaskedSourceEx(
	IN gco2D Engine,
	IN gctUINT32 Address,
	IN gctUINT32 Stride,
	IN gceSURF_FORMAT Format,
	IN gctBOOL CoordRelative,
	IN gceSURF_MONOPACK MaskPack,
	IN gceSURF_ROTATION Rotation,
	IN gctUINT32 SurfaceWidth,
	IN gctUINT32 SurfaceHeight
	)
{
	gcsSURF_INFO surface;
	gctUINT32 srcTransparency;
	gctUINT32 dstTransparency;
	gctUINT32 patTransparency;
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x Address=%x Stride=%d Format=%d CoordRelative=%d "
					"MaskPack=%d Rotation=%d SurfaceWidth=%d SurfaceHeight=%d",
					Engine, Address, Stride, Format, CoordRelative,
					MaskPack, Rotation, SurfaceWidth, SurfaceHeight);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	/* Fill in the structure. */
	gcoOS_ZeroMemory(&surface, gcmSIZEOF(surface));
	surface.type          = gcvSURF_BITMAP;
	surface.format        = Format;
	surface.stride        = Stride;
	surface.rotation      = Rotation;
	surface.alignedWidth  = SurfaceWidth;
	surface.alignedHeight  = SurfaceHeight;
	surface.node.physical = Address;

	do
	{
		gcmERR_BREAK(gcoHARDWARE_TranslateSurfTransparency(
			gcvSURF_SOURCE_MASK,
			&srcTransparency,
			&dstTransparency,
			&patTransparency
			));

		/* Set the transparency. */
		gcmERR_BREAK(gcoHARDWARE_SetTransparencyModes(
			Engine->hal->hardware,
			(gce2D_TRANSPARENCY) srcTransparency,
			(gce2D_TRANSPARENCY) dstTransparency,
			(gce2D_TRANSPARENCY) patTransparency
			));

		/* Set the source. */
		gcmERR_BREAK(gcoHARDWARE_SetMaskedSource(
			Engine->hal->hardware,
			&surface,
			CoordRelative,
			MaskPack
			));
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetSource
**
**	Setup the source rectangle.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gcsRECT_PTR SrcRect
**			Pointer to a valid source rectangle.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetSource(
	IN gco2D Engine,
	IN gcsRECT_PTR SrcRect
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x SrcRect=0x%x", Engine, SrcRect);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	status = gcoHARDWARE_SetSource(
		Engine->hal->hardware,
		SrcRect
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetClipping
**
**	Set clipping rectangle.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gcsRECT_PTR Rect
**			Pointer to a valid destination rectangle.
**			The valid range of the coordinates is 0..32768.
**			A pixel is valid if the following is true:
**				(pixelX >= Left) && (pixelX < Right) &&
**				(pixelY >= Top)  && (pixelY < Bottom)
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetClipping(
	IN gco2D Engine,
	IN gcsRECT_PTR Rect
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x Rect=%d", Engine, Rect);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	status = gcoHARDWARE_SetClipping(
		Engine->hal->hardware,
		Rect
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetTarget
**
**	Configure destination.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT32 Address
**			Destination surface base address.
**
**		gctUINT32 Stride
**			Stride of the destination surface in bytes.
**
**		gceSURF_ROTATION Rotation
**			Set to not zero if the destination surface is 90 degree rotated.
**
**		gctUINT32 SurfaceWidth
**			Required only if the surface is rotated. Equal to the width
**			of the destination surface in pixels.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetTarget(
	IN gco2D Engine,
	IN gctUINT32 Address,
	IN gctUINT32 Stride,
	IN gceSURF_ROTATION Rotation,
	IN gctUINT32 SurfaceWidth
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x Address=%x Stride=%d Rotation=%d SurfaceWidth=%d",
					Engine, Address, Stride, Rotation, SurfaceWidth);

	if ((Rotation != gcvSURF_0_DEGREE) && (Rotation != gcvSURF_90_DEGREE))
	{
		gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
		return gcvSTATUS_NOT_SUPPORTED;
	}

	/* Forward to gco2D_SetTargetEx with the SurfaceHeight set to 0. */
	status = gco2D_SetTargetEx(
				Engine,
				Address,
				Stride,
				Rotation,
				SurfaceWidth,
				0);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetTargetEx
**
**	Configure destination.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT32 Address
**			Destination surface base address.
**
**		gctUINT32 Stride
**			Stride of the destination surface in bytes.
**
**		gceSURF_ROTATION Rotation
**			Set to not zero if the destination surface is 90 degree rotated.
**
**		gctUINT32 SurfaceWidth
**			Required only if the surface is rotated. Equal to the width
**			of the destination surface in pixels.
**
**		gctUINT32 SurfaceHeight
**			Required only if the surface is rotated in PE 2.0. Equal to the height
**			of the destination surface in pixels.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetTargetEx(
	IN gco2D Engine,
	IN gctUINT32 Address,
	IN gctUINT32 Stride,
	IN gceSURF_ROTATION Rotation,
	IN gctUINT32 SurfaceWidth,
	IN gctUINT32 SurfaceHeight
	)
{
	gcsSURF_INFO surface;
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x Address=%x Stride=%d Rotation=%d "
					"SurfaceWidth=%d SurfaceHeight=%d",
					Engine, Address, Stride, Rotation, SurfaceWidth, SurfaceHeight);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	/* Fill in the structure. */
	gcoOS_ZeroMemory(&surface, gcmSIZEOF(surface));
	surface.type          = gcvSURF_BITMAP;
	surface.alignedWidth  = SurfaceWidth;
	surface.alignedHeight  = SurfaceHeight;
	surface.rotation      = Rotation;
	surface.stride        = Stride;
	surface.node.physical = Address;

	status = gcoHARDWARE_SetTarget(
		Engine->hal->hardware,
		&surface
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetStretchFactors
**
**	Calculate and program the stretch factors.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT32 HorFactor
**			Horizontal stretch factor.
**
**		gctUINT32 VerFactor
**			Vertical stretch factor.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetStretchFactors(
	IN gco2D Engine,
	IN gctUINT32 HorFactor,
	IN gctUINT32 VerFactor
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x HorFactor=%d VerFactor=%d", Engine, HorFactor, VerFactor);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	status = gcoHARDWARE_SetStretchFactors(
		Engine->hal->hardware,
		HorFactor,
		VerFactor
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetStretchRectFactors
**
**	Calculate and program the stretch factors based on the rectangles.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gcsRECT_PTR SrcRect
**			Pointer to a valid source rectangle.
**
**		gcsRECT_PTR DestRect
**			Pointer to a valid destination rectangle.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetStretchRectFactors(
	IN gco2D Engine,
	IN gcsRECT_PTR SrcRect,
	IN gcsRECT_PTR DestRect
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x SrcRect=0x%x DestRect=0x%x", Engine, SrcRect, DestRect);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	do
	{
		gctUINT32 horFactor, verFactor;

		/* Calculate the stretch factors. */
		gcmERR_BREAK(gcoHARDWARE_GetStretchFactors(
			SrcRect, DestRect,
			&horFactor, &verFactor
			));

		/* Program the stretch factors. */
		gcmERR_BREAK(gcoHARDWARE_SetStretchFactors(
			Engine->hal->hardware,
			horFactor,
			verFactor
			));
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_ConstructSingleColorBrush
**
**	Create a new solid color gcoBRUSH object.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT32 ColorConvert
**			The value of the Color parameter is stored directly in internal
**			color register and is used either directly to initialize pattern
**			or is converted to the format of destination before it is used.
**			The later happens if ColorConvert is not zero.
**
**		gctUINT32 Color
**			The color value of the pattern. The value will be used to
**			initialize 8x8 pattern. If the value is in destination format,
**			set ColorConvert to 0. Otherwise, provide the value in ARGB8
**			format and set ColorConvert to 1 to instruct the hardware to
**			convert the value to the destination format before it is
**			actually used.
**
**		gctUINT64 Mask
**			64 bits of mask, where each bit corresponds to one pixel of 8x8
**			pattern. Each bit of the mask is used to determine transparency
**			of the corresponding pixel, in other words, each mask bit is used
**			to select between foreground or background ROPs. If the bit is 0,
**			background ROP is used on the pixel; if 1, the foreground ROP
**			is used. The mapping between Mask parameter bits and actual
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
**		gcoBRUSH * Brush
**			Pointer to the variable that will hold the gcoBRUSH object pointer.
*/
gceSTATUS
gco2D_ConstructSingleColorBrush(
	IN gco2D Engine,
	IN gctUINT32 ColorConvert,
	IN gctUINT32 Color,
	IN gctUINT64 Mask,
	gcoBRUSH * Brush
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x ColorConvert=%d Color=%x Mask=%llx",
					Engine, ColorConvert, Color, Mask);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	status = gcoBRUSH_ConstructSingleColor(
		Engine->hal,
		ColorConvert,
		Color,
		Mask,
		Brush
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_ConstructMonochromeBrush
**
**	Create a new monochrome gcoBRUSH object.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT32 OriginX
**		gctUINT32 OriginY
**			Specifies the origin of the pattern in 0..7 range.
**
**		gctUINT32 ColorConvert
**			The values of FgColor and BgColor parameters are stored directly in
**			internal color registers and are used either directly to initialize
**			pattern or converted to the format of destination before actually
**			used. The later happens if ColorConvert is not zero.
**
**		gctUINT32 FgColor
**		gctUINT32 BgColor
**			Foreground and background colors of the pattern. The values will be
**			used to initialize 8x8 pattern. If the values are in destination
**			format, set ColorConvert to 0. Otherwise, provide the values in
**			ARGB8 format and set ColorConvert to 1 to instruct the hardware to
**			convert the values to the destination format before they are
**			actually used.
**
**		gctUINT64 Bits
**			64 bits of pixel bits. Each bit represents one pixel and is used
**			to choose between foreground and background colors. If the bit
**			is 0, the background color is used; otherwise the foreground color
**			is used. The mapping between Bits parameter and the actual pattern
**			pixels is the same as of the Mask parameter.
**
**		gctUINT64 Mask
**			64 bits of mask, where each bit corresponds to one pixel of 8x8
**			pattern. Each bit of the mask is used to determine transparency
**			of the corresponding pixel, in other words, each mask bit is used
**			to select between foreground or background ROPs. If the bit is 0,
**			background ROP is used on the pixel; if 1, the foreground ROP
**			is used. The mapping between Mask parameter bits and the actual
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
**		gcoBRUSH * Brush
**			Pointer to the variable that will hold the gcoBRUSH object pointer.
*/
gceSTATUS
gco2D_ConstructMonochromeBrush(
	IN gco2D Engine,
	IN gctUINT32 OriginX,
	IN gctUINT32 OriginY,
	IN gctUINT32 ColorConvert,
	IN gctUINT32 FgColor,
	IN gctUINT32 BgColor,
	IN gctUINT64 Bits,
	IN gctUINT64 Mask,
	gcoBRUSH * Brush
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x OriginX=%d OriginY=%d ColorConvert=%d "
					"FgColor=%x BgColor=%x Bits=%lld Mask=%llx",
					Engine, OriginX, OriginY, ColorConvert,
					FgColor, BgColor, Bits, Mask);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	status = gcoBRUSH_ConstructMonochrome(
		Engine->hal,
		OriginX,
		OriginY,
		ColorConvert,
		FgColor,
		BgColor,
		Bits,
		Mask,
		Brush
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_ConstructColorBrush
**
**	Create a color gcoBRUSH object.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT32 OriginX
**		gctUINT32 OriginY
**			Specifies the origin of the pattern in 0..7 range.
**
**		gctPOINTER Address
**			Location of the pattern bitmap in system memory.
**
**		gceSURF_FORMAT Format
**			Format of the source bitmap.
**
**		gctUINT64 Mask
**			64 bits of mask, where each bit corresponds to one pixel of 8x8
**			pattern. Each bit of the mask is used to determine transparency
**			of the corresponding pixel, in other words, each mask bit is used
**			to select between foreground or background ROPs. If the bit is 0,
**			background ROP is used on the pixel; if 1, the foreground ROP
**			is used. The mapping between Mask parameter bits and the actual
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
**		gcoBRUSH * Brush
**			Pointer to the variable that will hold the gcoBRUSH object pointer.
*/
gceSTATUS
gco2D_ConstructColorBrush(
	IN gco2D Engine,
	IN gctUINT32 OriginX,
	IN gctUINT32 OriginY,
	IN gctPOINTER Address,
	IN gceSURF_FORMAT Format,
	IN gctUINT64 Mask,
	gcoBRUSH * Brush
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x OriginX=%d OriginY=%d Address=0x%x Format=%d Mask=%llx",
					Engine, OriginX, OriginY, Address, Format, Mask);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	status = gcoBRUSH_ConstructColor(
		Engine->hal,
		OriginX,
		OriginY,
		Address,
		Format,
		Mask,
		Brush
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_Clear
**
**	Clear one or more rectangular areas.
**  The color is specified in A8R8G8B8 format.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT32 RectCount
**			The number of rectangles to draw. The array of line positions
**			pointed to by Position parameter must have at least RectCount
**			positions.
**
**		gcsRECT_PTR Rect
**			Points to an array of positions in (x0, y0)-(x1, y1) format.
**
**		gctUINT32 Color32
**			A8R8G8B8 clear color value.
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
gceSTATUS
gco2D_Clear(
	IN gco2D Engine,
	IN gctUINT32 RectCount,
	IN gcsRECT_PTR Rect,
	IN gctUINT32 Color32,
	IN gctUINT8 FgRop,
	IN gctUINT8 BgRop,
	IN gceSURF_FORMAT DestFormat
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x RectCount=%d Rect=0x%x Color32=%x "
					"FgRop=%x BgRop=%x DestFormat=%d",
					Engine, RectCount, Rect, Color32,
					FgRop, BgRop, DestFormat);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	do {
		/* Set the target format. */
		gcmERR_BREAK(gcoHARDWARE_SetTargetFormat(
			Engine->hal->hardware,
			DestFormat
			));

		/* Clear. */
		gcmERR_BREAK(gcoHARDWARE_Clear2D(
			Engine->hal->hardware,
			RectCount,
			Rect,
			Color32,
			gcvTRUE,
			FgRop,
			BgRop
			));
	}
	while (0);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_Line
**
**	Draw one or more Bresenham lines.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT32 LineCount
**			The number of lines to draw. The array of line positions pointed
**			to by Position parameter must have at least LineCount positions.
**
**		gcsRECT_PTR Position
**			Points to an array of positions in (x0, y0)-(x1, y1) format.
**
**		gcoBRUSH Brush
**			Brush to use for drawing.
**
**		gctUINT8 FgRop
**			Foreground ROP to use with opaque pixels.
**
**		gctUINT8 BgRop
**			Background ROP to use with transparent pixels.
**
**		gceSURF_FORMAT DestFormat
**			Format of the destination buffer.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_Line(
	IN gco2D Engine,
	IN gctUINT32 LineCount,
	IN gcsRECT_PTR Position,
	IN gcoBRUSH Brush,
	IN gctUINT8 FgRop,
	IN gctUINT8 BgRop,
	IN gceSURF_FORMAT DestFormat
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x LineCount=%d Position=0x%x Brush=0x%x "
					"FgRop=%x BgRop=%x DestFormat=%d",
					Engine, LineCount, Position, Brush,
					FgRop, BgRop, DestFormat);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	do
	{
		/* Old PE automatically sets the transparency to SOURCE_MASK,
		   when there is no source and pattern is used.
		   For PE 2.0, this has to be done explicitly.
		*/
		gcmERR_BREAK(gcoHARDWARE_SetAutoTransparency(
			Engine->hal->hardware,
			FgRop,
			BgRop
			));

		/* Set the target format. */
		gcmERR_BREAK(gcoHARDWARE_SetTargetFormat(
			Engine->hal->hardware,
			DestFormat
			));

		/* Draw the lines. */
		gcmERR_BREAK(gcoHARDWARE_Line2D(
			Engine->hal->hardware,
			LineCount,
			Position,
			Brush,
			FgRop,
			BgRop
			));
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_ColorLine
**
**	Draw one or more Bresenham lines based on the 32-bit color.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT32 LineCount
**			The number of lines to draw. The array of line positions pointed
**			to by Position parameter must have at least LineCount positions.
**
**		gcsRECT_PTR Position
**			Points to an array of positions in (x0, y0)-(x1, y1) format.
**
**		gctUINT32 Color32
**			Source color in A8R8G8B8 format.
**
**		gctUINT8 FgRop
**			Foreground ROP to use with opaque pixels.
**
**		gctUINT8 BgRop
**			Background ROP to use with transparent pixels.
**
**		gceSURF_FORMAT DestFormat
**			Format of the destination buffer.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_ColorLine(
	IN gco2D Engine,
	IN gctUINT32 LineCount,
	IN gcsRECT_PTR Position,
	IN gctUINT32 Color32,
	IN gctUINT8 FgRop,
	IN gctUINT8 BgRop,
	IN gceSURF_FORMAT DestFormat
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x LineCount=%d Position=0x%x Color32=%x "
					"FgRop=%x BgRop=%x DestFormat=%d",
					Engine, LineCount, Position, Color32,
					FgRop, BgRop, DestFormat);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	do {
		/* Set the target format. */
		gcmERR_BREAK(gcoHARDWARE_SetTargetFormat(
			Engine->hal->hardware,
			DestFormat
			));

		/* Draw the lines. */
		gcmERR_BREAK(gcoHARDWARE_Line2DEx(
			Engine->hal->hardware,
			LineCount,
			Position,
			1,
			&Color32,
			FgRop,
			BgRop
			));
	}
	while (0);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_Blit
**
**	Generic blit.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT32 RectCount
**			The number of rectangles to draw. The array of rectangle positions
**			pointed to by Rect parameter must have at least RectCount
**			positions.
**
**		gcsRECT_PTR Rect
**			Points to an array of positions in (x0, y0)-(x1, y1) format.
**
**		gctUINT8 FgRop
**			Foreground ROP to use with opaque pixels.
**
**		gctUINT8 BgRop
**			Background ROP to use with transparent pixels.
**
**		gceSURF_FORMAT DestFormat
**			Format of the destination buffer.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_Blit(
	IN gco2D Engine,
	IN gctUINT32 RectCount,
	IN gcsRECT_PTR Rect,
	IN gctUINT8 FgRop,
	IN gctUINT8 BgRop,
	IN gceSURF_FORMAT DestFormat
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x RectCount=%d Rect=0x%x FgRop=%x BgRop=%x DestFormat=%d",
					Engine, RectCount, Rect, FgRop, BgRop, DestFormat);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	do
	{
		/* Old PE automatically sets the transparency to SOURCE_MASK,
		   when there is no source and pattern is used.
		   For PE 2.0, this has to be done explicitly.
		*/
		gcmERR_BREAK(gcoHARDWARE_SetAutoTransparency(
			Engine->hal->hardware,
			FgRop,
			BgRop
			));

		/* Set the target format. */
		gcmERR_BREAK(gcoHARDWARE_SetTargetFormat(
			Engine->hal->hardware,
			DestFormat
			));

		/* Set the source. */
		gcmERR_BREAK(gcoHARDWARE_StartDE(
			Engine->hal->hardware,
			gcv2D_BLT,
			1,
			gcvNULL,
			RectCount,
			Rect,
			FgRop,
			BgRop
			));
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_BatchBlit
**
**	Generic blit for a batch of source destination rectangle pairs.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT32 RectCount
**			The number of rectangles to draw. The array of rectangle positions
**			pointed to by SrcRect and DestRect parameters must have at least
**			RectCount positions.
**
**		gcsRECT_PTR SrcRect
**			Points to an array of positions in (x0, y0)-(x1, y1) format.
**
**		gcsRECT_PTR DestRect
**			Points to an array of positions in (x0, y0)-(x1, y1) format.
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
gceSTATUS
gco2D_BatchBlit(
	IN gco2D Engine,
	IN gctUINT32 RectCount,
	IN gcsRECT_PTR SrcRect,
	IN gcsRECT_PTR DestRect,
	IN gctUINT8 FgRop,
	IN gctUINT8 BgRop,
	IN gceSURF_FORMAT DestFormat
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x RectCount=%d SrcRect=0x%x DestRect=0x%x "
					"FgRop=%x BgRop=%x DestFormat=%d",
					Engine, RectCount, SrcRect, DestRect,
					FgRop, BgRop, DestFormat);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	do {
		/* Set the target format. */
		gcmERR_BREAK(gcoHARDWARE_SetTargetFormat(
			Engine->hal->hardware,
			DestFormat
			));

		/* Start the DE engine. */
		gcmERR_BREAK(gcoHARDWARE_StartDE(
			Engine->hal->hardware,
			gcv2D_BLT,
			RectCount,
			SrcRect,
			RectCount,
			DestRect,
			FgRop,
			BgRop
			));
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_StretchBlit
**
**	Stretch blit.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT32 RectCount
**			The number of rectangles to draw. The array of rectangle positions
**			pointed to by Rect parameter must have at least RectCount
**			positions.
**
**		gcsRECT_PTR Rect
**			Points to an array of rectangles. All rectangles are assumed to be
**			of the same size.
**
**		gctUINT8 FgRop
**			Foreground ROP to use with opaque pixels.
**
**		gctUINT8 BgRop
**			Background ROP to use with transparent pixels.
**
**		gceSURF_FORMAT DestFormat
**			Format of the destination buffer.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_StretchBlit(
	IN gco2D Engine,
	IN gctUINT32 RectCount,
	IN gcsRECT_PTR Rect,
	IN gctUINT8 FgRop,
	IN gctUINT8 BgRop,
	IN gceSURF_FORMAT DestFormat
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x RectCount=%d Rect=0x%x FgRop=%x BgRop=%x DestFormat=%d",
					Engine, RectCount, Rect, FgRop, BgRop, DestFormat);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	do
	{
		/* Old PE automatically sets the transparency to SOURCE_MASK,
		   when there is no source and pattern is used.
		   For PE 2.0, this has to be done explicitly.
		*/
		gcmERR_BREAK(gcoHARDWARE_SetAutoTransparency(
			Engine->hal->hardware,
			FgRop,
			BgRop
			));

		/* Set the target format. */
		gcmERR_BREAK(gcoHARDWARE_SetTargetFormat(
			Engine->hal->hardware,
			DestFormat
			));

		/* Set the source. */
		gcmERR_BREAK(gcoHARDWARE_StartDE(
			Engine->hal->hardware,
			gcv2D_STRETCH,
			1,
			gcvNULL,
			RectCount,
			Rect,
			FgRop,
			BgRop
			));
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_MonoBlit
**
**	Monochrome blit.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctPOINTER StreamBits
**			Pointer to the monochrome bitmap.
**
**		gcsPOINT_PTR StreamSize
**			Size of the monochrome bitmap in pixels.
**
**		gcsRECT_PTR StreamRect
**			Bounding rectangle of the area within the bitmap to render.
**
**		gceSURF_MONOPACK SrcStreamPack
**			Source bitmap packing.
**
**		gceSURF_MONOPACK DestStreamPack
**			Packing of the bitmap in the command stream.
**
**		gcsRECT_PTR DestRect
**			Pointer to an array of destination rectangles.
**
**		gctUINT32 FgRop
**			Foreground and background ROP codes.
**
**		gctUINT32 BgRop
**			Background ROP to use with transparent pixels.
**
**		gceSURF_FORMAT DestFormat
**			Destination surface format.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_MonoBlit(
	IN gco2D Engine,
	IN gctPOINTER StreamBits,
	IN gcsPOINT_PTR StreamSize,
	IN gcsRECT_PTR StreamRect,
	IN gceSURF_MONOPACK SrcStreamPack,
	IN gceSURF_MONOPACK DestStreamPack,
	IN gcsRECT_PTR DestRect,
	IN gctUINT32 FgRop,
	IN gctUINT32 BgRop,
	IN gceSURF_FORMAT DestFormat
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x StreamBits=0x%x StreamSize=%d StreamRect=0x%x "
					"SrcStreamPack=%d DestStreamPack=%d DestRect=0x%x "
					"FgRop=%x BgRop=%x DestFormat=%d",
					Engine, StreamBits, StreamSize, StreamRect,
					SrcStreamPack, DestStreamPack, DestRect,
					FgRop, BgRop, DestFormat);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	do
	{
		/* Old PE automatically sets the transparency to SOURCE_MASK,
		   when there is no source and pattern is used.
		   For PE 2.0, this has to be done explicitly.
		*/
		gcmERR_BREAK(gcoHARDWARE_SetAutoTransparency(
			Engine->hal->hardware,
			(gctUINT8) FgRop,
			(gctUINT8) BgRop
			));

		/* Set the target format. */
		gcmERR_BREAK(gcoHARDWARE_SetTargetFormat(
			Engine->hal->hardware,
			DestFormat
			));

		/* Set the source. */
		gcmERR_BREAK(gcoHARDWARE_MonoBlit(
			Engine->hal->hardware,
			StreamBits,
			StreamSize,
			StreamRect,
			SrcStreamPack,
			DestStreamPack,
			DestRect,
			FgRop,
			BgRop
			));
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetKernelSize
**
**	Set kernel size.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT8 HorKernelSize
**			Kernel size for the horizontal pass.
**
**		gctUINT8 VerKernelSize
**			Kernel size for the vertical pass.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetKernelSize(
	IN gco2D Engine,
	IN gctUINT8 HorKernelSize,
	IN gctUINT8 VerKernelSize
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x HorKernelSize=%d VerKernelSize=%d",
					Engine, HorKernelSize, VerKernelSize);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	status = gcoHARDWARE_SetKernelSize(
		Engine->hal->hardware,
		HorKernelSize,
		VerKernelSize
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetFilterType
**
**	Set filter type.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gceFILTER_TYPE FilterType
**			Filter type for the filter blit.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetFilterType(
	IN gco2D Engine,
	IN gceFILTER_TYPE FilterType
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x FilterType=%d", Engine, FilterType);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	status = gcoHARDWARE_SetFilterType(
		Engine->hal->hardware,
		FilterType
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetUserFilterKernel
**
**	Set the user defined filter kernel.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gceFILTER_PASS_TYPE PassType
**			Pass type for the filter blit.
**
**		gctUINT16_PTR KernelArray
**			Pointer to the kernel array from user.
**
**		gctINT ArrayLen
**			Length of the kernel array in bytes.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetUserFilterKernel(
	IN gco2D Engine,
	IN gceFILTER_PASS_TYPE PassType,
	IN gctUINT16_PTR KernelArray
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x PassType=%d KernelArray=0x%x", Engine, PassType, KernelArray);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	status = gcoHARDWARE_SetUserFilterKernel(
		Engine->hal->hardware,
		PassType,
		KernelArray
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_EnableUserFilterPasses
**
**	Select the pass(es) to be done for user defined filter.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctBOOL HorPass
**			Enable horizontal filter pass if HorPass is gcvTRUE.
**			Otherwise disable this pass.
**
**		gctBOOL VerPass
**			Enable vertical filter pass if VerPass is gcvTRUE.
**			Otherwise disable this pass.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_EnableUserFilterPasses(
	IN gco2D Engine,
	IN gctBOOL HorPass,
	IN gctBOOL VerPass
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x HorPass=%d VerPass=%d", Engine, HorPass, VerPass);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	status = gcoHARDWARE_EnableUserFilterPasses(
				Engine->hal->hardware,
				HorPass,
				VerPass
				);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_FreeFilterBuffer
**
**	Frees the temporary buffer allocated by filter blit operation.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_FreeFilterBuffer(
	IN gco2D Engine
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x", Engine);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	status = gcoHARDWARE_FreeFilterBuffer(
		Engine->hal->hardware
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_FilterBlit
**
**	Filter blit.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT32 SrcAddress
**			Base address of the source surface in local memory.
**
**		gctUINT SrcStride
**			Stride of the source surface in bytes.
**
**		gctUINT32 SrcUAddress
**			Base address of U channel of the source surface in local memory for YUV format.
**
**		gctUINT SrcUStride
**			Stride of U channel of the source surface in bytes for YUV format.
**
**		gctUINT32 SrcVAddress
**			Base address of V channel of the source surface in local memory for YUV format.
**
**		gctUINT SrcVStride
**			Stride of of V channel the source surface in bytes for YUV format.
**
**		gceSURF_FORMAT SrcFormat
**			Format of the source surface.
**
**		gceSURF_ROTATION SrcRotation
**			Specifies the source surface rotation angle.
**
**		gctUINT32 SrcSurfaceWidth
**			The width in pixels of the source surface.
**
**		gcsRECT_PTR SrcRect
**			Coordinates of the entire source image.
**
**		gctUINT32 DestAddress
**			Base address of the destination surface in local memory.
**
**		gctUINT DestStride
**			Stride of the destination surface in bytes.
**
**		gceSURF_FORMAT DestFormat
**			Format of the destination surface.
**
**		gceSURF_ROTATION DestRotation
**			Specifies the destination surface rotation angle.
**
**		gctUINT32 DestSurfaceWidth
**			The width in pixels of the destination surface.
**
**		gcsRECT_PTR DestRect
**			Coordinates of the entire destination image.
**
**		gcsRECT_PTR DestSubRect
**			Coordinates of a sub area within the destination to render.
**			If DestSubRect is gcvNULL, the complete image will be rendered
**			using coordinates set by DestRect.
**			If DestSubRect is not gcvNULL and DestSubRect and DestRect are
**			no equal, DestSubRect is assumed to be within DestRect and
**			will be used to render the sub area only.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_FilterBlit(
	IN gco2D Engine,
	IN gctUINT32 SrcAddress,
	IN gctUINT SrcStride,
	IN gctUINT32 SrcUAddress,
	IN gctUINT SrcUStride,
	IN gctUINT32 SrcVAddress,
	IN gctUINT SrcVStride,
	IN gceSURF_FORMAT SrcFormat,
	IN gceSURF_ROTATION SrcRotation,
	IN gctUINT32 SrcSurfaceWidth,
	IN gcsRECT_PTR SrcRect,
	IN gctUINT32 DestAddress,
	IN gctUINT DestStride,
	IN gceSURF_FORMAT DestFormat,
	IN gceSURF_ROTATION DestRotation,
	IN gctUINT32 DestSurfaceWidth,
	IN gcsRECT_PTR DestRect,
	IN gcsRECT_PTR DestSubRect
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x SrcAddress=%x SrcStride=%d SrcUAddress=%x SrcUStride=%d "
					"SrcVAddress=%x SrcVStride=%d SrcFormat=%d SrcRotation=%d "
					"SrcSurfaceWidth=%d SrcRect=0x%x "
					"DestAddress=%x DestStride=%d DestFormat=%d DestRotation=%d "
					"DestSurfaceWidth=%d DestRect=0x%x DestSubRect=0x%x",
					Engine, SrcAddress, SrcStride, SrcUAddress, SrcUStride,
					SrcVAddress, SrcVStride, SrcFormat, SrcRotation,
					SrcSurfaceWidth, SrcRect,
					DestAddress, DestStride, DestFormat, DestRotation,
					DestSurfaceWidth, DestRect, DestSubRect);

	if (((SrcRotation != gcvSURF_0_DEGREE) && (SrcRotation != gcvSURF_90_DEGREE))
		|| ((DestRotation != gcvSURF_0_DEGREE) && (DestRotation != gcvSURF_90_DEGREE)))
	{
		gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
		return gcvSTATUS_NOT_SUPPORTED;
	}

	/* Forward to gco2D_FilterBlitEx with the DstHeight set to 0
		and SrcHeight set to 0. */
	status = gco2D_FilterBlitEx(
							Engine,
							SrcAddress,
							SrcStride,
							SrcUAddress,
							SrcUStride,
							SrcVAddress,
							SrcVStride,
							SrcFormat,
							SrcRotation,
							SrcSurfaceWidth,
							0,
							SrcRect,
							DestAddress,
							DestStride,
							DestFormat,
							DestRotation,
							DestSurfaceWidth,
							0,
							DestRect,
							DestSubRect
							);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_FilterBlitEx
**
**	Filter blit.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT32 SrcAddress
**			Base address of the source surface in local memory.
**
**		gctUINT SrcStride
**			Stride of the source surface in bytes.
**
**		gctUINT32 SrcUAddress
**			Base address of U channel of the source surface in local memory for YUV format.
**
**		gctUINT SrcUStride
**			Stride of U channel of the source surface in bytes for YUV format.
**
**		gctUINT32 SrcVAddress
**			Base address of V channel of the source surface in local memory for YUV format.
**
**		gctUINT SrcVStride
**			Stride of of V channel the source surface in bytes for YUV format.
**
**		gceSURF_FORMAT SrcFormat
**			Format of the source surface.
**
**		gceSURF_ROTATION SrcRotation
**			Specifies the source surface rotation angle.
**
**		gctUINT32 SrcSurfaceWidth
**			The width in pixels of the source surface.
**
**		gctUINT32 SrcSurfaceHeight
**			The height in pixels of the source surface for the rotaion in PE 2.0.
**
**		gcsRECT_PTR SrcRect
**			Coordinates of the entire source image.
**
**		gctUINT32 DestAddress
**			Base address of the destination surface in local memory.
**
**		gctUINT DestStride
**			Stride of the destination surface in bytes.
**
**		gceSURF_FORMAT DestFormat
**			Format of the destination surface.
**
**		gceSURF_ROTATION DestRotation
**			Specifies the destination surface rotation angle.
**
**		gctUINT32 DestSurfaceWidth
**			The width in pixels of the destination surface.
**
**		gctUINT32 DestSurfaceHeight
**			The height in pixels of the destination surface for the rotaion in PE 2.0.
**
**		gcsRECT_PTR DestRect
**			Coordinates of the entire destination image.
**
**		gcsRECT_PTR DestSubRect
**			Coordinates of a sub area within the destination to render.
**			If DestSubRect is gcvNULL, the complete image will be rendered
**			using coordinates set by DestRect.
**			If DestSubRect is not gcvNULL and DestSubRect and DestRect are
**			no equal, DestSubRect is assumed to be within DestRect and
**			will be used to render the sub area only.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_FilterBlitEx(
	IN gco2D Engine,
	IN gctUINT32 SrcAddress,
	IN gctUINT SrcStride,
	IN gctUINT32 SrcUAddress,
	IN gctUINT SrcUStride,
	IN gctUINT32 SrcVAddress,
	IN gctUINT SrcVStride,
	IN gceSURF_FORMAT SrcFormat,
	IN gceSURF_ROTATION SrcRotation,
	IN gctUINT32 SrcSurfaceWidth,
	IN gctUINT32 SrcSurfaceHeight,
	IN gcsRECT_PTR SrcRect,
	IN gctUINT32 DestAddress,
	IN gctUINT DestStride,
	IN gceSURF_FORMAT DestFormat,
	IN gceSURF_ROTATION DestRotation,
	IN gctUINT32 DestSurfaceWidth,
	IN gctUINT32 DestSurfaceHeight,
	IN gcsRECT_PTR DestRect,
	IN gcsRECT_PTR DestSubRect
	)
{
	gcsSURF_INFO srcSurface;
	gcsSURF_INFO destSurface;
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x SrcAddress=%x SrcStride=%d SrcUAddress=%x SrcUStride=%d "
					"SrcVAddress=%x SrcVStride=%d SrcFormat=%d SrcRotation=%d "
					"SrcSurfaceWidth=%d SrcSurfaceHeight=%d SrcRect=0x%x "
					"DestAddress=%x DestStride=%d DestFormat=%d DestRotation=%d "
					"DestSurfaceWidth=%d DestSurfaceHeight=%d DestRect=0x%x DestSubRect=0x%x",
					Engine, SrcAddress, SrcStride, SrcUAddress, SrcUStride,
					SrcVAddress, SrcVStride, SrcFormat, SrcRotation,
					SrcSurfaceWidth, SrcSurfaceHeight, SrcRect,
					DestAddress, DestStride, DestFormat, DestRotation,
					DestSurfaceWidth, DestSurfaceHeight, DestRect, DestSubRect);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	/* Fill in the source structure. */
	gcoOS_ZeroMemory(&srcSurface, gcmSIZEOF(srcSurface));
	srcSurface.type				= gcvSURF_BITMAP;
	srcSurface.format			= SrcFormat;
	srcSurface.alignedWidth		= SrcSurfaceWidth;
	srcSurface.alignedHeight	= SrcSurfaceHeight;
	srcSurface.rotation			= SrcRotation;
	srcSurface.stride			= SrcStride;
	srcSurface.node.physical	= SrcAddress;
	srcSurface.uStride			= SrcUStride;
	srcSurface.node.physical2	= SrcUAddress;
	srcSurface.vStride			= SrcVStride;
	srcSurface.node.physical3	= SrcVAddress;

	/* Fill in the target structure. */
	gcoOS_ZeroMemory(&destSurface, gcmSIZEOF(destSurface));
	destSurface.type          = gcvSURF_BITMAP;
	destSurface.format        = DestFormat;
	destSurface.alignedWidth  = DestSurfaceWidth;
	destSurface.alignedHeight = DestSurfaceHeight;
	destSurface.rotation      = DestRotation;
	destSurface.stride        = DestStride;
	destSurface.node.physical = DestAddress;

	status = gcoHARDWARE_FilterBlit(
		Engine->hal->hardware,
		&srcSurface,
		&destSurface,
		SrcRect,
		DestRect,
		DestSubRect
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_EnableAlphaBlend
**
**	Enable alpha blending engine in the hardware and disengage the ROP engine.
**
**  This function is only working with old PE (<2.0).
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gctUINT8 SrcGlobalAlphaValue
**		gctUINT8 DstGlobalAlphaValue
**			Global alpha value for the color components.
**
**		gceSURF_PIXEL_ALPHA_MODE SrcAlphaMode
**		gceSURF_PIXEL_ALPHA_MODE DstAlphaMode
**			Per-pixel alpha component mode.
**
**		gceSURF_GLOBAL_ALPHA_MODE SrcGlobalAlphaMode
**		gceSURF_GLOBAL_ALPHA_MODE DstGlobalAlphaMode
**			Global/per-pixel alpha values selection.
**
**		gceSURF_BLEND_FACTOR_MODE SrcFactorMode
**		gceSURF_BLEND_FACTOR_MODE DstFactorMode
**			Final blending factor mode.
**
**		gceSURF_PIXEL_COLOR_MODE SrcColorMode
**		gceSURF_PIXEL_COLOR_MODE DstColorMode
**			Per-pixel color component mode.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_EnableAlphaBlend(
	IN gco2D Engine,
	IN gctUINT8 SrcGlobalAlphaValue,
	IN gctUINT8 DstGlobalAlphaValue,
	IN gceSURF_PIXEL_ALPHA_MODE SrcAlphaMode,
	IN gceSURF_PIXEL_ALPHA_MODE DstAlphaMode,
	IN gceSURF_GLOBAL_ALPHA_MODE SrcGlobalAlphaMode,
	IN gceSURF_GLOBAL_ALPHA_MODE DstGlobalAlphaMode,
	IN gceSURF_BLEND_FACTOR_MODE SrcFactorMode,
	IN gceSURF_BLEND_FACTOR_MODE DstFactorMode,
	IN gceSURF_PIXEL_COLOR_MODE SrcColorMode,
	IN gceSURF_PIXEL_COLOR_MODE DstColorMode
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x SrcGlobalAlphaValue=%x DstGlobalAlphaValue=%d "
					"SrcAlphaMode=%x DstAlphaMode=%d "
					"SrcGlobalAlphaMode=%d DstGlobalAlphaMode=%d "
					"SrcFactorMode=%x DstFactorMode=%d SrcColorMode=%d DstColorMode=%d",
					Engine, SrcGlobalAlphaValue, DstGlobalAlphaValue,
					SrcAlphaMode, DstAlphaMode, SrcGlobalAlphaMode, DstGlobalAlphaMode,
					SrcFactorMode, DstFactorMode, SrcColorMode, DstColorMode);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	do
	{
		/* Set the source global color. */
		gcmERR_BREAK(gcoHARDWARE_SetSourceGlobalColor(
			Engine->hal->hardware,
			(gctUINT32)SrcGlobalAlphaValue << 24
			));

		/* Set the target global color. */
		gcmERR_BREAK(gcoHARDWARE_SetTargetGlobalColor(
			Engine->hal->hardware,
			(gctUINT32)DstGlobalAlphaValue << 24
			));

		/* Enable blending. */
		gcmERR_BREAK(gcoHARDWARE_EnableAlphaBlend(
			Engine->hal->hardware,
			SrcAlphaMode,
			DstAlphaMode,
			SrcGlobalAlphaMode,
			DstGlobalAlphaMode,
			SrcFactorMode,
			DstFactorMode,
			SrcColorMode,
			DstColorMode
			));
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_EnableAlphaBlendAdvanced
**
**	Enable alpha blending engine in the hardware.
**
**  This function is only working with PE 2.0 and above.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gceSURF_PIXEL_ALPHA_MODE SrcAlphaMode
**		gceSURF_PIXEL_ALPHA_MODE DstAlphaMode
**			Per-pixel alpha component mode.
**
**		gceSURF_GLOBAL_ALPHA_MODE SrcGlobalAlphaMode
**		gceSURF_GLOBAL_ALPHA_MODE DstGlobalAlphaMode
**			Global/per-pixel alpha values selection.
**
**		gceSURF_BLEND_FACTOR_MODE SrcFactorMode
**		gceSURF_BLEND_FACTOR_MODE DstFactorMode
**			Final blending factor mode.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_EnableAlphaBlendAdvanced(
	IN gco2D Engine,
	IN gceSURF_PIXEL_ALPHA_MODE SrcAlphaMode,
	IN gceSURF_PIXEL_ALPHA_MODE DstAlphaMode,
	IN gceSURF_GLOBAL_ALPHA_MODE SrcGlobalAlphaMode,
	IN gceSURF_GLOBAL_ALPHA_MODE DstGlobalAlphaMode,
	IN gceSURF_BLEND_FACTOR_MODE SrcFactorMode,
	IN gceSURF_BLEND_FACTOR_MODE DstFactorMode
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x SrcAlphaMode=%x DstAlphaMode=%d "
					"SrcGlobalAlphaMode=%x DstGlobalAlphaMode=%d "
					"SrcFactorMode=%x DstFactorMode=%d",
					Engine, SrcAlphaMode, DstAlphaMode,
					SrcGlobalAlphaMode, DstGlobalAlphaMode,
					SrcFactorMode, DstFactorMode);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	status = gcoHARDWARE_EnableAlphaBlend(
		Engine->hal->hardware,
		SrcAlphaMode,
		DstAlphaMode,
		SrcGlobalAlphaMode,
		DstGlobalAlphaMode,
		SrcFactorMode,
		DstFactorMode,
		gcvSURF_COLOR_STRAIGHT,
		gcvSURF_COLOR_STRAIGHT
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetPorterDuffBlending
**
**	Enable alpha blending engine in the hardware and setup the blending modes
**  using the Porter Duff rule defined.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gce2D_PORTER_DUFF_RULE Rule
**			Porter Duff blending rule.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetPorterDuffBlending(
	IN gco2D Engine,
	IN gce2D_PORTER_DUFF_RULE Rule
	)
{
	gceSURF_BLEND_FACTOR_MODE srcFactor, dstFactor;
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x Rule=%d", Engine, Rule);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	switch(Rule)
	{
	case gcvPD_CLEAR:
		srcFactor = gcvSURF_BLEND_ZERO;
		dstFactor = gcvSURF_BLEND_ZERO;
		break;

	case gcvPD_SRC:
		srcFactor = gcvSURF_BLEND_ONE;
		dstFactor = gcvSURF_BLEND_ZERO;
		break;

	case gcvPD_SRC_OVER:
		srcFactor = gcvSURF_BLEND_ONE;
		dstFactor = gcvSURF_BLEND_INVERSED;
		break;

	case gcvPD_DST_OVER:
		srcFactor = gcvSURF_BLEND_INVERSED;
		dstFactor = gcvSURF_BLEND_ONE;
		break;

	case gcvPD_SRC_IN:
		srcFactor = gcvSURF_BLEND_STRAIGHT;
		dstFactor = gcvSURF_BLEND_ZERO;
		break;

	case gcvPD_DST_IN:
		srcFactor = gcvSURF_BLEND_ZERO;
		dstFactor = gcvSURF_BLEND_STRAIGHT;
		break;

	case gcvPD_SRC_OUT:
		srcFactor = gcvSURF_BLEND_INVERSED;
		dstFactor = gcvSURF_BLEND_ZERO;
		break;

	case gcvPD_DST_OUT:
		srcFactor = gcvSURF_BLEND_ZERO;
		dstFactor = gcvSURF_BLEND_INVERSED;
		break;

	case gcvPD_SRC_ATOP:
		srcFactor = gcvSURF_BLEND_STRAIGHT;
		dstFactor = gcvSURF_BLEND_INVERSED;
		break;

	case gcvPD_DST_ATOP:
		srcFactor = gcvSURF_BLEND_INVERSED;
		dstFactor = gcvSURF_BLEND_STRAIGHT;
		break;

	case gcvPD_ADD:
		srcFactor = gcvSURF_BLEND_ONE;
		dstFactor = gcvSURF_BLEND_ONE;
		break;

	case gcvPD_XOR:
		srcFactor = gcvSURF_BLEND_INVERSED;
		dstFactor = gcvSURF_BLEND_INVERSED;
		break;

	default:
		return gcvSTATUS_INVALID_ARGUMENT;
	}

	status = gcoHARDWARE_EnableAlphaBlend(
		Engine->hal->hardware,
		gcvSURF_PIXEL_ALPHA_STRAIGHT,
		gcvSURF_PIXEL_ALPHA_STRAIGHT,
		gcvSURF_GLOBAL_ALPHA_OFF,
		gcvSURF_GLOBAL_ALPHA_OFF,
		srcFactor,
		dstFactor,
		gcvSURF_COLOR_STRAIGHT,
		gcvSURF_COLOR_STRAIGHT
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_DisableAlphaBlend
**
**	Disable alpha blending engine in the hardware and engage the ROP engine.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_DisableAlphaBlend(
	IN gco2D Engine
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x", Engine);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	status = gcoHARDWARE_DisableAlphaBlend(
		Engine->hal->hardware
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_GetPackSize
**
**	Retrieve monochrome stream pack size.
**
**	INPUT:
**
**		gceSURF_MONOPACK StreamPack
**			Stream pack code.
**
**	OUTPUT:
**
**		gctUINT32 * PackWidth
**		gctUINT32 * PackHeight
**			Monochrome stream pack size.
*/
gceSTATUS
gco2D_GetPackSize(
	IN gceSURF_MONOPACK StreamPack,
	OUT gctUINT32 * PackWidth,
	OUT gctUINT32 * PackHeight
	)
{
	gcmHEADER_ARG("StreamPack=0x%x", StreamPack);

	gcmVERIFY_ARGUMENT(PackWidth != gcvNULL);
	gcmVERIFY_ARGUMENT(PackHeight != gcvNULL);

    /* Dispatch on monochrome packing. */
	switch (StreamPack)
	{
	case gcvSURF_PACKED8:
		*PackWidth  = 8;
		*PackHeight = 4;
		break;

	case gcvSURF_PACKED16:
		*PackWidth  = 16;
		*PackHeight = 2;
		break;

	case gcvSURF_PACKED32:
	case gcvSURF_UNPACKED:
		*PackWidth  = 32;
		*PackHeight = 1;
		break;

	default:
        /* Not supprted. */
		gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
		return gcvSTATUS_NOT_SUPPORTED;
	}

    /* Success. */
	gcmFOOTER_ARG("*PackWidth=%d *PackHeight=%d",
					*PackWidth, *PackHeight);
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gco2D_Flush
**
**	Flush the 2D pipeline.
**
**	INPUT:
**		gco2D Engine
**			Pointer to an gco2D object.
**
**	OUTPUT:
**
**		Nothing.
*/

gceSTATUS
gco2D_Flush(
	IN gco2D Engine
	)
{
	gceSTATUS status;
	gcmHEADER_ARG("Engine=0x%x", Engine);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	/* Flush the current pipe. */
	status = gcoHARDWARE_FlushPipe(
		Engine->hal->hardware
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_LoadPalette
**
**	Load 256-entry color table for INDEX8 source surfaces.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to the gco2D object.
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
**			(see ColorConvert).
**
**		gctBOOL ColorConvert
**			If set to gcvTRUE, the 32-bit values in the table are assumed to be
**			in ARGB8 format and will be converted by the hardware to the
**			destination format as needed.
**			If set to gcvFALSE, the 32-bit values in the table are assumed to be
**			preconverted to the destination format.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_LoadPalette(
	IN gco2D Engine,
	IN gctUINT FirstIndex,
	IN gctUINT IndexCount,
	IN gctPOINTER ColorTable,
	IN gctBOOL ColorConvert
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x FirstIndex=%d IndexCount=%d ColorTable=0x%x ColorConvert=%d",
					Engine, FirstIndex, IndexCount, ColorTable, ColorConvert);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	/* Relay the call. */
	status = gcoHARDWARE_LoadPalette(
		Engine->hal->hardware,
		FirstIndex,
		IndexCount,
		ColorTable,
		ColorConvert
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetBitBlitMirror
**
**	Enable/disable 2D BitBlt mirrorring.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to the gco2D object.
**
**		gctBOOL HorizontalMirror
**			Horizontal mirror enable flag.
**
**		gctBOOL VerticalMirror
**			Vertical mirror enable flag.
**
**	OUTPUT:
**
**		gceSTATUS
**			Returns gcvSTATUS_OK if successful.
*/
gceSTATUS
gco2D_SetBitBlitMirror(
	IN gco2D Engine,
	IN gctBOOL HorizontalMirror,
	IN gctBOOL VerticalMirror
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x HorizontalMirror=%d VerticalMirror=%d",
					Engine, HorizontalMirror, VerticalMirror);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	/* Relay the call. */
	status = gcoHARDWARE_SetBitBlitMirror(
		Engine->hal->hardware,
		HorizontalMirror,
		VerticalMirror
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetTransparencyAdvanced
**
**	Set the transparency for source, destination and pattern.
**
**  This function is only working with PE 2.0 and above.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to the gco2D object.
**
**		gce2D_TRANSPARENCY SrcTransparency
**			Source Transparency.
**
**		gce2D_TRANSPARENCY DstTransparency
**			Destination Transparency.
**
**		gce2D_TRANSPARENCY PatTransparency
**			Pattern Transparency.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetTransparencyAdvanced(
	IN gco2D Engine,
	IN gce2D_TRANSPARENCY SrcTransparency,
	IN gce2D_TRANSPARENCY DstTransparency,
	IN gce2D_TRANSPARENCY PatTransparency
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x SrcTransparency=%d DstTransparency=%d PatTransparency=%d",
					Engine, SrcTransparency, DstTransparency, PatTransparency);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	/* Relay the call. */
	status = gcoHARDWARE_SetTransparencyModes(
		Engine->hal->hardware,
		SrcTransparency,
		DstTransparency,
		PatTransparency);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetSourceColorKeyAdvanced
**
**	Set the source color key.
**	Color channel values should specified in the range allowed by the source format.
**	When target format is A8, only Alpha component is used. Otherwise, Alpha component
**  is not used.
**
**  This function is only working with PE 2.0 and above.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to the gco2D object.
**
**		gctUINT32 ColorKey
**			The color key value in A8R8G8B8 format.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetSourceColorKeyAdvanced(
	IN gco2D Engine,
	IN gctUINT32 ColorKey
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x ColorKey=%d", Engine, ColorKey);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	/* Relay the call. */
	status = gcoHARDWARE_SetSourceColorKeyRange(
		Engine->hal->hardware,
		ColorKey,
		ColorKey,
		gcvTRUE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetSourceColorKeyRangeAdvanced
**
**	Set the source color key range.
**	Color channel values should specified in the range allowed by the source format.
**	Lower color key's color channel values should be less than or equal to
**  the corresponding color channel value of ColorKeyHigh.
**	When target format is A8, only Alpha components are used. Otherwise, Alpha
**  components are not used.
**
**  This function is only working with PE 2.0 and above.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to the gco2D object.
**
**		gctUINT32 ColorKeyLow
**			The low color key value in A8R8G8B8 format.
**
**		gctUINT8 ColorKeyHigh
**			The high color key value in A8R8G8B8 format.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetSourceColorKeyRangeAdvanced(
	IN gco2D Engine,
	IN gctUINT32 ColorKeyLow,
	IN gctUINT32 ColorKeyHigh
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x ColorKeyLow=%d ColorKeyHigh=%d",
					Engine, ColorKeyLow, ColorKeyHigh);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	/* Relay the call. */
	status = gcoHARDWARE_SetSourceColorKeyRange(
		Engine->hal->hardware,
		ColorKeyLow,
		ColorKeyHigh,
		gcvTRUE
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetTargetColorKeyAdvanced
**
**	Set the target color key.
**	Color channel values should specified in the range allowed by the target format.
**	When target format is A8, only Alpha component is used. Otherwise, Alpha component
**  is not used.
**
**  This function is only working with PE 2.0 and above.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to the gco2D object.
**
**		gctUINT32 ColorKey
**			The color key value in A8R8G8B8 format.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetTargetColorKeyAdvanced(
	IN gco2D Engine,
	IN gctUINT32 ColorKey
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x ColorKey=%d", Engine, ColorKey);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	/* Relay the call. */
	status = gcoHARDWARE_SetTargetColorKeyRange(
		Engine->hal->hardware,
		ColorKey,
		ColorKey
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetTargetColorKeyRangeAdvanced
**
**	Set the source color key range.
**	Color channel values should specified in the range allowed by the target format.
**	Lower color key's color channel values should be less than or equal to
**  the corresponding color channel value of ColorKeyHigh.
**	When target format is A8, only Alpha components are used. Otherwise, Alpha
**  components are not used.
**
**  This function is only working with PE 2.0 and above.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to the gco2D object.
**
**		gctUINT32 ColorKeyLow
**			The low color key value in A8R8G8B8 format.
**
**		gctUINT32 ColorKeyHigh
**			The high color key value in A8R8G8B8 format.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetTargetColorKeyRangeAdvanced(
	IN gco2D Engine,
	IN gctUINT32 ColorKeyLow,
	IN gctUINT32 ColorKeyHigh
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x ColorKeyLow=%d ColorKeyHigh=%d",
					Engine, ColorKeyLow, ColorKeyHigh);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	/* Relay the call. */
	status = gcoHARDWARE_SetTargetColorKeyRange(
		Engine->hal->hardware,
		ColorKeyLow,
		ColorKeyHigh
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetYUVColorMode
**
**	Set the YUV color space mode.
**
**  This function is only working with PE 2.0 and above.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to the gco2D object.
**
**		gce2D_YUV_COLOR_MODE Mode
**			Mode is either 601 or 709.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_SetYUVColorMode(
	IN gco2D Engine,
	IN gce2D_YUV_COLOR_MODE Mode
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x Mode=%d", Engine, Mode);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	/* Relay the call. */
	status = gcoHARDWARE_YUVColorMode(
		Engine->hal->hardware,
		Mode);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetSourceGlobalColorAdvanced
**
**	Set the source global color value in A8R8G8B8 format.
**
**  This function is only working with PE 2.0 and above.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to the gco2D object.
**
**		gctUINT32 Color32
**			Source color.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gco2D_SetSourceGlobalColorAdvanced(
	IN gco2D Engine,
	IN gctUINT32 Color32
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x Color32=%x", Engine, Color32);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	/* Relay the call. */
	status = gcoHARDWARE_SetSourceGlobalColor(
		Engine->hal->hardware,
		Color32);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetTargetGlobalColor
**
**	Set the source global color value in A8R8G8B8 format.
**
**  This function is only working with PE 2.0 and above.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to the gco2D object.
**
**		gctUINT32 Color32
**			Target color.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gco2D_SetTargetGlobalColorAdvanced(
	IN gco2D Engine,
	IN gctUINT32 Color32
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x Color32=%x", Engine, Color32);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	/* Relay the call. */
	status = gcoHARDWARE_SetTargetGlobalColor(
		Engine->hal->hardware,
		Color32);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetPixelMultiplyModesAdvanced
**
**	Set the source and target pixel multiply modes.
**
**  This function is only working with PE 2.0 and above.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to the gco2D object.
**
**		gce2D_PIXEL_COLOR_MULTIPLY_MODE SrcPremultiplySrcAlpha
**			Source color premultiply with Source Alpha.
**
**		gce2D_PIXEL_COLOR_MULTIPLY_MODE DstPremultiplyDstAlpha
**			Destination color premultiply with Destination Alpha.
**
**		gce2D_GLOBAL_COLOR_MULTIPLY_MODE SrcPremultiplyGlobalMode
**			Source color premultiply with Global color's Alpha or Color.
**
**		gce2D_PIXEL_COLOR_MULTIPLY_MODE DstDemultiplyDstAlpha
**			Destination color demultiply with Destination Alpha.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gco2D_SetPixelMultiplyModeAdvanced(
	IN gco2D Engine,
	IN gce2D_PIXEL_COLOR_MULTIPLY_MODE SrcPremultiplySrcAlpha,
	IN gce2D_PIXEL_COLOR_MULTIPLY_MODE DstPremultiplyDstAlpha,
	IN gce2D_GLOBAL_COLOR_MULTIPLY_MODE SrcPremultiplyGlobalMode,
	IN gce2D_PIXEL_COLOR_MULTIPLY_MODE DstDemultiplyDstAlpha
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x SrcPremultiplySrcAlpha=%d DstPremultiplyDstAlpha=%d "
					"SrcPremultiplyGlobalMode=%d DstDemultiplyDstAlpha=%d",
					Engine, SrcPremultiplySrcAlpha, DstPremultiplyDstAlpha,
					SrcPremultiplyGlobalMode, DstDemultiplyDstAlpha);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	/* Relay the call. */
	status = gcoHARDWARE_SetMultiplyModes(
		Engine->hal->hardware,
		SrcPremultiplySrcAlpha,
		DstPremultiplyDstAlpha,
		SrcPremultiplyGlobalMode,
		DstDemultiplyDstAlpha
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_SetAutoFlushCycles
**
**	Set the GPU clock cycles, after which the idle 2D engine
**  will trigger a flush.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to the gco2D object.
**
**		UINT32 Cycles
**			Source color premultiply with Source Alpha.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gco2D_SetAutoFlushCycles(
	IN gco2D Engine,
	IN gctUINT32 Cycles
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x Cycles=%d", Engine, Cycles);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	/* Relay the call. */
	status = gcoHARDWARE_SetAutoFlushCycles(
		Engine->hal->hardware,
		Cycles
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gco2D_ProfileEngine
**
**	Read the profile registers available in the 2D engine and set them in the profile.
**	pixelsRendered counter is reset to 0 after reading.
**
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to the gco2D object.
**
**		OPTIONAL gcs2D_PROFILE_PTR Profile
**			Pointer to a gcs2D_Profile structure.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gco2D_ProfileEngine(
	IN gco2D Engine,
	OPTIONAL gcs2D_PROFILE_PTR Profile
	)
{
	gcsHAL_INTERFACE iface;
	gceSTATUS status;

	gcmHEADER_ARG("Engine=0x%x Profile=0x%x", Engine, Profile);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Engine, gcvOBJ_2D);

	/* Read all 2D profile registers. */
	iface.command = gcvHAL_PROFILE_REGISTERS_2D;
	iface.u.RegisterProfileData2D.hwProfile2D = Profile;

	/* Call the kernel. */
	status = gcoOS_DeviceControl(
		Engine->hal->os,
		IOCTL_GCHAL_INTERFACE,
		&iface, gcmSIZEOF(iface),
		&iface, gcmSIZEOF(iface)
		);

	/* Return status. */
	gcmFOOTER();
	return status;
}


