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
**	gcoBRUSH object for user HAL layers.
**
*/

#include "gc_hal_user_precomp.h"
#include "gc_hal_user_brush.h"

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE	gcvZONE_2D

/******************************************************************************\
********************************** Structures **********************************
\******************************************************************************/

enum
{
	BRUSH_MAX_DATA_SIZE = (10 + 64) * 4
};

struct _gcoBRUSH
{
	/* Object. */
	gcsOBJECT			object;

	/* Pointer to an gcoHAL object. */
	gcoHAL				hal;

	/* Pointer to an gco2D object. */
	gco2D				engine;

	/* Pattern attributes. */
	gceSURF_FORMAT		format;

	/* Pattern origin. */
	gctUINT32			originX;
	gctUINT32			originY;

	/* Color attributes. */
	gctUINT32			colorConvert;
	gctUINT32			fgColor;
	gctUINT32			bgColor;

	/* Mono pattern value and mask. */
	gctUINT64			monoBits;
	gctPOINTER			colorBits;
	gctUINT32			colorSize;
	gctUINT64			mask;
};

/******************************************************************************\
********************************* Support Code *********************************
\******************************************************************************/

/*******************************************************************************
**
**	_BuildBrushBuffer
**
**	Returns a buffer filled with complete set of brush parameters.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
**
**		gceSURF_FORMAT Format
**		gctUINT32 OriginX
**		gctUINT32 OriginY
**		gctUINT32 ColorConvert
**		gctUINT32 FgColor
**		gctUINT32 BgColor
**		gctUINT64 MonoBits
**		gctPOINTER ColorBits
**		gctUINT64 Mask
**			Brush parameters described in the ConstructXXX functions below.
**
**	OUTPUT:
**
**		gctUINT8 * BrushData
**			Brush data buffer.
**
**		gctUINT32 * DataCount
**			Number of bytes in the brush data array.
*/
static gceSTATUS _BuildBrushBuffer(
	IN gcoHARDWARE Hardware,
	IN gceSURF_FORMAT Format,
	IN gctUINT32 OriginX,
	IN gctUINT32 OriginY,
	IN gctUINT32 ColorConvert,
	IN gctUINT32 FgColor,
	IN gctUINT32 BgColor,
	IN gctUINT64 MonoBits,
	IN gctPOINTER ColorBits,
	IN gctUINT64 Mask,
	IN OUT gctUINT8 * BrushData,
	IN OUT gctUINT32 * DataCount
	)
{
	gceSTATUS status;
	gctUINT32 i, index;
	gctUINT32 * brushData;
	gctUINT32 bitsPerPixel;
	gctUINT32 size8, size32;
	gctUINT64 tempBits;

	/* Check parameters. */
	if ((BrushData == gcvNULL) ||
		(*DataCount < BRUSH_MAX_DATA_SIZE))
	{
		*DataCount = BRUSH_MAX_DATA_SIZE;
		return gcvSTATUS_MORE_DATA;
	}

	/* Init the buffer parameters. */
	index = 0;
	brushData = (gctUINT32*) BrushData;

	/* Fill in the buffer with brush parameters. */
	brushData[index++] = Format;						/* 1  */
	brushData[index++] = OriginX;						/* 2  */
	brushData[index++] = OriginY;						/* 3  */
	brushData[index++] = ColorConvert;					/* 4  */
	brushData[index++] = FgColor;						/* 5  */
	brushData[index++] = BgColor;						/* 6  */
	brushData[index++] = (gctUINT32)(MonoBits >> 32);	/* 7  */
	brushData[index++] = (gctUINT32)(MonoBits);			/* 8  */
	brushData[index++] = (gctUINT32)(Mask >> 32);		/* 9  */
	brushData[index++] = (gctUINT32)(Mask);				/* 10 */

	/* Fill in the brush color data based on the format. */
	if (ColorBits != gcvNULL)
	{
		/*
		   Color brush.
		*/

		/* Compute bits per pixel. */
		status = gcoHARDWARE_ConvertFormat(Hardware,
										   Format,
										   &bitsPerPixel,
										   gcvNULL);

		if (status != gcvSTATUS_OK)
		{
			/* Error. */
			return status;
		}

		/* Determine the size of the brush bitmap. */
		size8  = bitsPerPixel * 8;
		size32 = size8 / 4;

		/* Copy brush bitmap. */
		gcmVERIFY_OK(gcoOS_MemCopy(brushData + index, ColorBits, size8));
		index += size32;
	}
	else if ((OriginX != ~0) && (OriginY != ~0))
	{
		/*
		   Monochrome brush.
		*/

		tempBits = MonoBits;
		for (i = 0; i < 64; i++)
		{
			brushData[index++] = ((tempBits & 1) != 0)
				? FgColor
				: BgColor;

			tempBits >>= 1;
		}
	}
	else
	{
		/*
		  Solid color brush.
		*/

		for (i = 0; i < 64; i++)
		{
			brushData[index++] = FgColor;
		}
	}

	/* Set the data size. */
	*DataCount = index * 4;

	/* Success. */
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	_Construct
**
**	Common brush constructor.
**
**	INPUT:
**
**		gcoHARDWARE Hardware,
**			Pointer to an gcoHARDWARE object.
**
**		gceSURF_FORMAT Format
**		gctUINT32 OriginX
**		gctUINT32 OriginY
**		gctUINT32 ColorConvert
**		gctUINT32 FgColor
**		gctUINT32 BgColor
**		gctUINT64 MonoBits
**		gctPOINTER ColorBits
**		gctUINT64 Mask
**			Brush parameters described in the ConstructXXX functions below.
**
**	OUTPUT:
**
**		gcoBRUSH * Brush
**			Initialized gcoBRUSH object if successful.
*/
static gceSTATUS _Construct(
	IN gcoHAL Hal,
	IN gceSURF_FORMAT Format,
	IN gctUINT32 OriginX,
	IN gctUINT32 OriginY,
	IN gctUINT32 ColorConvert,
	IN gctUINT32 FgColor,
	IN gctUINT32 BgColor,
	IN gctUINT64 MonoBits,
	IN gctUINT32 * ColorBits,
	IN gctUINT64 Mask,
	gcoBRUSH * Brush
	)
{
	gcoHARDWARE hardware;
	gcoOS os;
	gco2D engine;
	gceSTATUS status;
	gcoBRUSH_CACHE brushCache;
	gcoBRUSH brush;
	gctUINT8 brushData[BRUSH_MAX_DATA_SIZE];
	gctUINT32 dataSize;
	gctUINT32 * bitmap;
	gctUINT32 bitsPerPixel;
	gctUINT32 brushID;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);

	/* Extract the gcoHARDWARE object pointer. */
	hardware = Hal->hardware;
	gcmVERIFY_OBJECT(hardware, gcvOBJ_HARDWARE);

	/* Extract the gcoOS object pointer. */
	os = Hal->os;
	gcmVERIFY_OBJECT(os, gcvOBJ_OS);

	/* Extract the gco2D object pointer. */
	status = gcoHAL_Get2DEngine(Hal, &engine);

	if (status < 0)
	{
		/* Error. */
		return status;
	}

	/* Query the brush cache pointer. */
	status = gco2D_GetBrushCache(engine, &brushCache);

	if (status != gcvSTATUS_OK)
	{
		/* Error. */
		return status;
	}

	gcmVERIFY_OBJECT(brushCache, gcvOBJ_BRUSHCACHE);

	/* Build the data buffer. */
	dataSize = sizeof(brushData);
	status = _BuildBrushBuffer(hardware,
							   Format,
							   OriginX,
							   OriginY,
							   ColorConvert,
							   FgColor,
							   BgColor,
							   MonoBits,
							   ColorBits,
							   Mask,
							   brushData,
							   &dataSize);

	if (status != gcvSTATUS_OK)
	{
		/* Error. */
		return status;
	}

	/* Compute the brush ID. */
	status = gcoBRUSH_CACHE_GetBrushID(brushCache,
									 brushData,
									 dataSize,
									 &brushID);

	if (status != gcvSTATUS_OK)
	{
		/* Error. */
		return status;
	}

	/* Attempt to find the matching brush. */
	status = gcoBRUSH_CACHE_GetBrush(brushCache,
								   brushID,
								   brushData,
								   dataSize,
								   &brush);

	if (status != gcvSTATUS_OK)
	{
		/* Error. */
		return status;
	}

	/* If no matching brush found, create a new one. */
	if (brush == gcvNULL)
	{
		/* Allocate the gcoBRUSH object. */
		status = gcoOS_Allocate(os, sizeof(struct _gcoBRUSH), (gctPOINTER *) &brush);

		if (status != gcvSTATUS_OK)
		{
			/* Error. */
			return status;
		}

		/* Copy color bitmap if present. */
		if (ColorBits == gcvNULL)
		{
			dataSize = 0;
			bitmap = gcvNULL;
		}
		else
		{
			/* Compute bits per pixel. */
			status = gcoHARDWARE_ConvertFormat(hardware,
											   Format,
											   &bitsPerPixel,
											   gcvNULL);

			if (status != gcvSTATUS_OK)
			{
				/* Free the gcoBRUSH object. */
				gcmVERIFY_OK(gcoOS_Free(os, brush));

				/* Error. */
				return status;
			}

			/* Determine the data size. */
			dataSize = bitsPerPixel * 8;

			/* Allocate the bitmap buffer. */
			status = gcoOS_Allocate(os, dataSize, (gctPOINTER *) &bitmap);

			if (status != gcvSTATUS_OK)
			{
				/* Free the gcoBRUSH object. */
				gcmVERIFY_OK(gcoOS_Free(os, brush));

				/* Error. */
				return status;
			}

			/* Copy the bitmap. */
			gcmVERIFY_OK(gcoOS_MemCopy(bitmap, ColorBits, dataSize));
		}

		/* Initialize the gcoBRUSH object.*/
		brush->object.type = gcvOBJ_BRUSH;
		brush->hal         = Hal;
		brush->engine      = engine;

		/* Set members. */
		brush->colorSize    = dataSize;
		brush->format       = Format;
		brush->originX      = OriginX;
		brush->originY      = OriginY;
		brush->colorConvert = ColorConvert;
		brush->fgColor      = FgColor;
		brush->bgColor      = BgColor;
		brush->monoBits     = MonoBits;
		brush->colorBits    = bitmap;
		brush->mask         = Mask;

		/* Add the brush to the cache. */
		status = gcoBRUSH_CACHE_AddBrush(brushCache,
									   brush,
									   brushID,
									   (bitmap == gcvNULL) ? gcvFALSE : gcvTRUE);

		if (status != gcvSTATUS_OK)
		{
			/* Free the brush. */
			gcmVERIFY_OK(gcoBRUSH_Delete(brush));

			/* Error. */
			return status;
		}
	}

	/* Return pointer to the gcoBRUSH object. */
	*Brush = brush;

	/* Success. */
	return gcvSTATUS_OK;
}

/******************************************************************************\
******************************* gcoBRUSH API Code ******************************
\******************************************************************************/

/*******************************************************************************
**
**	gcoBRUSH_ConstructSingleColor
**
**	Create a new solid color gcoBRUSH object.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
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
gcoBRUSH_ConstructSingleColor(
	IN gcoHAL Hal,
	IN gctUINT32 ColorConvert,
	IN gctUINT32 Color,
	IN gctUINT64 Mask,
	gcoBRUSH * Brush
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hal=0x%x ColorConvert=%d Color=%x Mask=%llx Brush=0x%x",
					Hal, ColorConvert, Color, Mask, Brush);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);
	gcmVERIFY_ARGUMENT(ColorConvert <= 1);
	gcmVERIFY_ARGUMENT(Brush != gcvNULL);

	/* Create the brush. */
	status = _Construct(Hal,
						gcvSURF_A8R8G8B8,
						~0U,
						~0U,
						ColorConvert,
						Color,
						Color,
						0,
						gcvNULL,
						Mask,
						Brush);

	/* Return status. */
	gcmFOOTER_ARG("*Brush=0x%x status=%d", *Brush, status);
	return status;
}

/*******************************************************************************
**
**	gcoBRUSH_ConstructMonochrome
**
**	Create a new monochrome gcoBRUSH object.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
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
gcoBRUSH_ConstructMonochrome(
	IN gcoHAL Hal,
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

	gcmHEADER_ARG("Hal=0x%x OriginX=%d OriginY=%d ColorConvert=%d FgColor=%x BgColor=%x"
					"Bits=%lld Mask=%llx Brush=0x%x",
					Hal, OriginX, OriginY, ColorConvert, FgColor, BgColor,
					Bits, Mask, Brush);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);
	gcmVERIFY_ARGUMENT(OriginX < 8);
	gcmVERIFY_ARGUMENT(OriginY < 8);
	gcmVERIFY_ARGUMENT(ColorConvert <= 1);
	gcmVERIFY_ARGUMENT(Brush != gcvNULL);

	/* Create the brush. */
	status = _Construct(Hal,
						gcvSURF_A8R8G8B8,
						OriginX,
						OriginY,
						ColorConvert,
						FgColor,
						BgColor,
						Bits,
						gcvNULL,
						Mask,
						Brush);

	/* Return status. */
	gcmFOOTER_ARG("*Brush=0x%x status=%d", *Brush, status);
	return status;
}

/*******************************************************************************
**
**	gcoBRUSH_ConstructColor
**
**	Create a color gcoBRUSH object.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
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
gcoBRUSH_ConstructColor(
	IN gcoHAL Hal,
	IN gctUINT32 OriginX,
	IN gctUINT32 OriginY,
	IN gctPOINTER Address,
	IN gceSURF_FORMAT Format,
	IN gctUINT64 Mask,
	gcoBRUSH * Brush
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hal=0x%x OriginX=%d OriginY=%d Address=0x%x Format=%d Mask=%llx Brush=0x%x",
					Hal, OriginX, OriginY, Address, Format, Mask, Brush);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);
	gcmVERIFY_ARGUMENT(OriginX < 8);
	gcmVERIFY_ARGUMENT(OriginY < 8);
	gcmVERIFY_ARGUMENT(Address != gcvNULL);
	gcmVERIFY_ARGUMENT(Brush != gcvNULL);

	/* Create the brush. */
	status = _Construct(Hal,
						Format,
						OriginX,
						OriginY,
						0,
						0,
						0,
						0,
						Address,
						Mask,
						Brush);

	/* Return status. */
	gcmFOOTER_ARG("*Brush=0x%x status=%d", *Brush, status);
	return status;
}

/*******************************************************************************
**
**	gcoBRUSH_Destroy
**
**	Destroy an gcoBRUSH object.
**
**	INPUT:
**
**		gcoBRUSH Brush
**			Pointer to an gcoBRUSH object to be destroyed.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoBRUSH_Destroy(
	IN gcoBRUSH Brush
	)
{
	gceSTATUS status;
	gcoBRUSH_CACHE brushCache;

	gcmHEADER_ARG("Brush=0x%x", Brush);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Brush, gcvOBJ_BRUSH);

	/* Query the brush cache pointer. */
	status = gco2D_GetBrushCache(Brush->engine, &brushCache);

	if (status < 0)
	{
		/* Error. */
		gcmFOOTER();
		return status;
	}

	/* Call gcoBRUSH_CACHE object to complete the call. */
	status = gcoBRUSH_CACHE_DeleteBrush(brushCache, Brush);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoBRUSH_Delete
**
**	Frees all resources held up the specified gcoBRUSH object.
**
**	INPUT:
**
**		gcoBRUSH Brush
**			Pointer to an gcoBRUSH object to be deleted.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoBRUSH_Delete(
	IN gcoBRUSH Brush
	)
{
	gcmHEADER_ARG("Brush=0x%x", Brush);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Brush, gcvOBJ_BRUSH);

	/* Free the bitmap. */
	if (Brush->colorBits != gcvNULL)
	{
		gcmVERIFY_OK(gcoOS_Free(Brush->hal->os, Brush->colorBits));
	}

	/* Mark gcoBRUSH object as unknown. */
	Brush->object.type = gcvOBJ_UNKNOWN;

	/* Free the gcoBRUSH object. */
	gcmVERIFY_OK(gcoOS_Free(Brush->hal->os, Brush));

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoBRUSH_GetBrushData
**
**	Return a buffer filled with complete set of brush data.
**
**	INPUT:
**
**		gcoBRUSH Brush
**			Pointer to an gcoBRUSH object to be deleted.
**
**	OUTPUT:
**
**		gctPOINTER BrushData
**			Brush data buffer.
**
**		gctUINT32 * DataCount
**			Number of bytes in the brush data array.
*/
gceSTATUS
gcoBRUSH_GetBrushData(
	IN gcoBRUSH Brush,
	IN OUT gctPOINTER BrushData,
	IN OUT gctUINT32 * DataCount
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Brush=0x%x BrushData=0x%x DataCount=0x%x", Brush, BrushData, DataCount);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Brush, gcvOBJ_BRUSH);

	/* Build the buffer from the new brush. */
	status = _BuildBrushBuffer(Brush->hal->hardware,
							   Brush->format,
							   Brush->originX,
							   Brush->originY,
							   Brush->colorConvert,
							   Brush->fgColor,
							   Brush->bgColor,
							   Brush->monoBits,
							   Brush->colorBits,
							   Brush->mask,
							   (gctUINT8 *) BrushData,
							   DataCount);

	/* Return status. */
	gcmFOOTER_ARG("*DataCount=%d status=%d", *DataCount, status);
	return status;
}

/*******************************************************************************
**
**	gcoBRUSH_FlushBrush
**
**	Flush the brush.
**
**	INPUT:
**
**		gcoBRUSH Brush
**			Pointer to an gcoBRUSH object to be deleted.
**
*		gctBOOL Upload
**			If not zero, the flush function will upload the pattern data
**			to the video memory block.
**
**		gcuVIDMEM_NODE_PTR Node
**			Pointer to video memory node object.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoBRUSH_FlushBrush(
	IN gcoBRUSH Brush,
	IN gctBOOL Upload,
	IN gcsSURF_NODE_PTR Node
	)
{
	gceSTATUS status;
	gcoHARDWARE hardware;

	gcmHEADER_ARG("Brush=0x%x Upload=%d Node=0x%x", Brush, Upload, Node);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Brush, gcvOBJ_BRUSH);

	/* Extract the gcoHARDWARE object pointer. */
	hardware = Brush->hal->hardware;
	gcmVERIFY_OBJECT(hardware, gcvOBJ_HARDWARE);

	/* Program the brush. */
	do
	{
		if (Brush->colorBits != gcvNULL)
		{
			/*
			   Color brush.
			*/

			/* Verify that the surface is locked. */
			gcmVERIFY_NODE_LOCK(Node);

			/* Copy the pattern to the video memory. */
			if (Upload)
			{
				gcmVERIFY_OK(gcoOS_MemCopy(Node->logical,
										   Brush->colorBits,
										   Brush->colorSize));

                gcmVERIFY_OK(gcoOS_CacheFlush(Brush->hal->os,
                                              Node->logical,
                                              Brush->colorSize));

			}

			/* Load the pattern. */
			status = gcoHARDWARE_LoadColorPattern(hardware,
												  Brush->originX,
												  Brush->originY,
												  Node->physical,
												  Brush->format,
												  Brush->mask);
		}
		else if ((Brush->originX != ~0) && (Brush->originY != ~0))
		{
			/*
			   Monochrome brush.
			*/

			status = gcoHARDWARE_LoadMonochromePattern(hardware,
													   Brush->originX,
													   Brush->originY,
													   Brush->colorConvert,
													   Brush->fgColor,
													   Brush->bgColor,
													   Brush->monoBits,
													   Brush->mask);
		}
		else
		{
			/*
			  Solid color brush.
			*/

			status = gcoHARDWARE_LoadSolidColorPattern(hardware,
													   Brush->colorConvert,
													   Brush->fgColor,
													   Brush->mask);
		}
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

