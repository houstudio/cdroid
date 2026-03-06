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

/******************************************************************************\
****************************** gcoHARDWARE API Code *****************************
\******************************************************************************/

/*******************************************************************************
**
**	gcoHARDWARE_QueryOpenGL2
**
**	Query the OpenGL ES 2.0 capabilities of the hardware.
**
**	INPUT:
**
**		Nothing.
**
**	OUTPUT:
**
**		gctBOOL * OpenGL2
**			Pointer to a boolean that will specify whether the hardware is
**			OpenGL ES 2.0 compliant or not.
*/
gceSTATUS gcoHARDWARE_QueryOpenGL2(
	OUT gctBOOL * OpenGL2
	)
{
	gcmHEADER();

	/* Verify the arguments. */
	gcmVERIFY_ARGUMENT(OpenGL2 != gcvNULL);

	/* Return OpenGL ES 2.0 capability for XAQ2. */
	*OpenGL2 = gcvTRUE;

	/* Success. */
	gcmFOOTER_ARG("*OpenGL2=%d", *OpenGL2);
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoHARDWARE_QueryStreamCaps
**
**	Query the stream capabilities of the hardware.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to a gcoHARDWARE object.
**
**	OUTPUT:
**
**		gctUINT * MaxAttributes
**			Pointer to a variable that will hold the maximum number of
**			atrtributes for a primitive on success.
**
**		gctUINT * MaxStreamSize
**			Pointer to a variable that will hold the maximum number of bytes of
**			a stream on success.
**
**		gctUINT * NumberOfStreams
**			Pointer to a variable that will hold the number of streams on
**			success.
**
**		gctUINT * Alignment
**			Pointer to a variable that will receive the stream alignment
**			requirement.
*/
gceSTATUS gcoHARDWARE_QueryStreamCaps(
	IN gcoHARDWARE Hardware,
	OUT gctUINT32 * MaxAttributes,
	OUT gctUINT32 * MaxStreamSize,
	OUT gctUINT32 * NumberOfStreams,
	OUT gctUINT32 * Alignment
	)
{
	gcmHEADER_ARG("Hardware=0x%x MaxAttributes=0x%x MaxStreamSize=0x%x "
					"NumberOfStreams=0x%x Alignment=0x%x",
					Hardware, MaxAttributes, MaxStreamSize,
					NumberOfStreams, Alignment);

	if (MaxAttributes != gcvNULL)
	{
		/* Return number of attributes per vertex for XAQ2. */
		*MaxAttributes = 10;
	}

	if (MaxStreamSize != gcvNULL)
	{
		/* Return maximum number of bytes per vertex for XAQ2. */
		*MaxStreamSize = 256;
	}

	if (NumberOfStreams != gcvNULL)
	{
		/* Return number of streams for XAQ2. */
		*NumberOfStreams = Hardware->streamCount;
	}

	if (Alignment != gcvNULL)
	{
		/* Return alignment. */
		*Alignment = (Hardware->chipModel == gcv700) ? 128 : 8;
	}

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoHARDWARE_ConvertFormat
**
**	Convert an API format to hardware parameters.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to the gcoHARDWARE object.
**
**		gceSURF_FORMAT Format
**			API format to convert.
**
**	OUTPUT:
**
**		gctUINT32_PTR BitsPerPixel
**			Pointer to a variable that will hold the number of bits per pixel.
**
**		gctUINT32_PTR BytesPerTile
**			Pointer to a variable that will hold the number of bytes per tile.
*/
gceSTATUS gcoHARDWARE_ConvertFormat(
	IN gcoHARDWARE Hardware,
	IN gceSURF_FORMAT Format,
	OUT gctUINT32_PTR BitsPerPixel,
	OUT gctUINT32_PTR BytesPerTile
	)
{
	gctUINT32 bitsPerPixel;
	gctUINT32 bytesPerTile;

	gcmHEADER_ARG("Hardware=0x%x Format=%d BitsPerPixel=0x%x BytesPerTile=0x%x",
					Hardware, Format, BitsPerPixel, BytesPerTile);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	/* Dispatch on format. */
	switch (Format)
	{
	case gcvSURF_INDEX8:
	case gcvSURF_A8:
	case gcvSURF_L8:
		/* 8-bpp format. */
		bitsPerPixel  = 8;
		bytesPerTile  = (8 * 4 * 4) / 8;
		break;

	case gcvSURF_YV12:
	case gcvSURF_I420:
	case gcvSURF_NV12:
	case gcvSURF_NV21:
		/* 12-bpp planar YUV formats. */
		bitsPerPixel  = 12;
		bytesPerTile  = (12 * 4 * 4) / 8;
		break;

	case gcvSURF_A8L8:
	case gcvSURF_X4R4G4B4:
	case gcvSURF_A4R4G4B4:
	case gcvSURF_X1R5G5B5:
	case gcvSURF_A1R5G5B5:
	case gcvSURF_R5G6B5:
	case gcvSURF_YUY2:
	case gcvSURF_UYVY:
	case gcvSURF_YVYU:
	case gcvSURF_VYUY:
	case gcvSURF_NV16:
	case gcvSURF_NV61:
	case gcvSURF_D16:
		/* 16-bpp format. */
		bitsPerPixel  = 16;
		bytesPerTile  = (16 * 4 * 4) / 8;
		break;

	case gcvSURF_X8R8G8B8:
	case gcvSURF_A8R8G8B8:
	case gcvSURF_X8B8G8R8:
	case gcvSURF_A8B8G8R8:
	case gcvSURF_D24X8:
	case gcvSURF_D24S8:
	case gcvSURF_D32:
		/* 32-bpp format. */
		bitsPerPixel  = 32;
		bytesPerTile  = (32 * 4 * 4) / 8;
		break;

	case gcvSURF_DXT1:
	case gcvSURF_ETC1:
		bitsPerPixel = 4;
		bytesPerTile = (4 * 4 * 4) / 8;
		break;

	case gcvSURF_DXT2:
	case gcvSURF_DXT3:
	case gcvSURF_DXT4:
	case gcvSURF_DXT5:
		bitsPerPixel = 4;
		bytesPerTile = (4 * 4 * 4) / 8;
		break;

	default:
		/* Invalid format. */
		return gcvSTATUS_INVALID_ARGUMENT;
	}

	/* Set the result. */
	if (BitsPerPixel != gcvNULL)
	{
		* BitsPerPixel = bitsPerPixel;
	}

	if (BytesPerTile != gcvNULL)
	{
		* BytesPerTile = bytesPerTile;
	}

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoHARDWARE_QueryCommandBuffer
**
**	Query the command buffer alignment and number of reserved bytes.
**
**	INPUT:
**
**		gcoHARDWARE Harwdare
**			Pointer to an gcoHARDWARE object.
**
**	OUTPUT:
**
**		gctSIZE_T * Alignment
**			Pointer to a variable receiving the alignment for each command.
**
**		gctSIZE_T * ReservedHead
**			Pointer to a variable receiving the number of reserved bytes at the
**          head of each command buffer.
**
**		gctSIZE_T * ReservedTail
**			Pointer to a variable receiving the number of bytes reserved at the
**          tail of each command buffer.
*/
gceSTATUS gcoHARDWARE_QueryCommandBuffer(
    IN gcoHARDWARE Hardware,
    OUT gctSIZE_T * Alignment,
    OUT gctSIZE_T * ReservedHead,
    OUT gctSIZE_T * ReservedTail
    )
{
	gcmHEADER_ARG("Hardware=0x%x Alignment=0x%x ReservedHead=0x%x ReservedTail=0x%x",
					Hardware, Alignment, ReservedHead, ReservedTail);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Alignment != gcvNULL)
    {
        /* Align every 8 bytes. */
        *Alignment = 8;
    }

    if (ReservedHead != gcvNULL)
    {
        /* Reserve space for SelectPipe. */
        *ReservedHead = 32;
    }

    if (ReservedTail != gcvNULL)
    {
        /* Reserve space for Link(). */
        *ReservedTail = 8;
    }

    /* Success. */
	gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoHARDWARE_GetSurfaceTileSize
**
**	Query the tile size of the given surface.
**
**	INPUT:
**
**		Nothing.
**
**	OUTPUT:
**
**		gctINT32 * TileWidth
**			Pointer to a variable receiving the width in pixels per tile.
**
**		gctINT32 * TileHeight
**			Pointer to a variable receiving the height in pixels per tile.
*/
gceSTATUS gcoHARDWARE_GetSurfaceTileSize(
	IN gcsSURF_INFO_PTR Surface,
	OUT gctINT32 * TileWidth,
	OUT gctINT32 * TileHeight
	)
{
	gcmHEADER_ARG("Surface=0x%x TileWidth=0x%x TileHeight=0x%x",
					Surface, TileWidth, TileHeight);

	if (Surface->type == gcvSURF_BITMAP)
	{
		if (TileWidth != gcvNULL)
		{
			/* 1 pixel per 2D tile (linear). */
			*TileWidth = 1;
		}

		if (TileHeight != gcvNULL)
		{
			/* 1 pixel per 2D tile (linear). */
			*TileHeight = 1;
		}
	}
	else
	{
		if (TileWidth != gcvNULL)
		{
			/* 4 pixels per 3D tile for now. */
			*TileWidth = 4;
		}

		if (TileHeight != gcvNULL)
		{
			/* 4 lines per 3D tile. */
			*TileHeight = 4;
		}
	}

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoHARDWARE_QueryTileSize
**
**	Query the tile sizes.
**
**	INPUT:
**
**		Nothing.
**
**	OUTPUT:
**
**		gctINT32 * TileWidth2D
**			Pointer to a variable receiving the width in pixels per 2D tile.  If
**			the 2D is working in linear space, the width will be 1.  If there is
**			no 2D, the width will be 0.
**
**		gctINT32 * TileHeight2D
**			Pointer to a variable receiving the height in pixels per 2D tile.
**			If the 2D is working in linear space, the height will be 1.  If
**			there is no 2D, the height will be 0.
**
**		gctINT32 * TileWidth3D
**			Pointer to a variable receiving the width in pixels per 3D tile.  If
**			the 3D is working in linear space, the width will be 1.  If there is
**			no 3D, the width will be 0.
**
**		gctINT32 * TileHeight3D
**			Pointer to a variable receiving the height in pixels per 3D tile.
**			If the 3D is working in linear space, the height will be 1.  If
**			there is no 3D, the height will be 0.
**
**		gctUINT32 * Stride
**			Pointer to  variable receiving the stride requirement for all modes.
*/
gceSTATUS gcoHARDWARE_QueryTileSize(
	OUT gctINT32 * TileWidth2D,
	OUT gctINT32 * TileHeight2D,
	OUT gctINT32 * TileWidth3D,
	OUT gctINT32 * TileHeight3D,
	OUT gctUINT32 * Stride
	)
{
	gcmHEADER_ARG("TileWidth2D=0x%x TileHeight2D=0x%x TileWidth3D=0x%x "
					"TileHeight3D=0x%x Stride=0x%x",
					TileWidth2D, TileHeight2D, TileWidth3D,
					TileHeight3D, Stride);

	if (TileWidth2D != gcvNULL)
	{
		/* 1 pixel per 2D tile (linear). */
		*TileWidth2D = 1;
	}

	if (TileHeight2D != gcvNULL)
	{
		/* 1 pixel per 2D tile (linear). */
		*TileHeight2D = 1;
	}

	if (TileWidth3D != gcvNULL)
	{
		/* 4 pixels per 3D tile for now. */
		*TileWidth3D = 4;
	}

	if (TileHeight3D != gcvNULL)
	{
		/* 4 lines per 3D tile. */
		*TileHeight3D = 4;
	}

	if (Stride != gcvNULL)
	{
		/* 64-byte stride requirement. */
		*Stride = 64;
	}

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoHARDWARE_QueryTextureCaps
**
**	Query the texture capabilities.
**
**	INPUT:
**
**		Nothing.
**
**	OUTPUT:
**
**		gctUINT * MaxWidth
**			Pointer to a variable receiving the maximum width of a texture.
**
**		gctUINT * MaxHeight
**			Pointer to a variable receiving the maximum height of a texture.
**
**		gctUINT * MaxDepth
**			Pointer to a variable receiving the maximum depth of a texture.  If
**			volume textures are not supported, 0 will be returned.
**
**		gctBOOL * Cubic
**			Pointer to a variable receiving a flag whether the hardware supports
**			cubic textures or not.
**
**		gctBOOL * NonPowerOfTwo
**			Pointer to a variable receiving a flag whether the hardware supports
**			non-power-of-two textures or not.
**
**		gctUINT * VertexSamplers
**			Pointer to a variable receiving the number of sampler units in the
**			vertex shader.
**
**		gctUINT * PixelSamplers
**			Pointer to a variable receiving the number of sampler units in the
**			pxiel shader.
*/
gceSTATUS gcoHARDWARE_QueryTextureCaps(
	IN	gcoHARDWARE Hardware,
	OUT gctUINT * MaxWidth,
	OUT gctUINT * MaxHeight,
	OUT gctUINT * MaxDepth,
	OUT gctBOOL * Cubic,
	OUT gctBOOL * NonPowerOfTwo,
	OUT gctUINT * VertexSamplers,
	OUT gctUINT * PixelSamplers
	)
{
	gcmHEADER_ARG("Hardware=0x%x MaxWidth=0x%x MaxHeight=0x%x MaxDepth=0x%x "
					"Cubic=0x%x NonPowerOfTwo=0x%x VertexSamplers=0x%x "
					"PixelSamplers=0x%x",
					Hardware, MaxWidth, MaxHeight, MaxDepth,
					Cubic, NonPowerOfTwo, VertexSamplers,
					PixelSamplers);

	if (MaxWidth != gcvNULL)
	{
		/* Maximum texture width for XAQ2. */
		*MaxWidth = gcoHARDWARE_IsFeatureAvailable(
			Hardware, gcvFEATURE_TEXTURE_8K) ? 8192 : 2048;
	}

	if (MaxHeight != gcvNULL)
	{
		/* Maximum texture height for XAQ2. */
		*MaxHeight = gcoHARDWARE_IsFeatureAvailable(
			Hardware, gcvFEATURE_TEXTURE_8K) ? 8192 : 2048;
	}

	if (MaxDepth != gcvNULL)
	{
		/* Maximum texture depth for XAQ2. */
		*MaxDepth = 1;
	}

	if (Cubic != gcvNULL)
	{
		/* XAQ2 supports cube maps. */
		*Cubic = gcvTRUE;
	}

	if (NonPowerOfTwo != gcvNULL)
	{
		/* XAQ2 does not support non-power-of-two texture maps. */
		*NonPowerOfTwo = gcvFALSE;
	}

	if (VertexSamplers != gcvNULL)
	{
		/* Return number of texture samples in the vertex shader for XAQ2. */
		*VertexSamplers = 0;
	}

	if (PixelSamplers != gcvNULL)
	{
		/* Return number of texture samples in the pixel shader for XAQ2. */
		*PixelSamplers = 8;
	}

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoHARDWARE_QueryTargetCaps
**
**	Query the render target capabilities.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to gcoHARDWARE object.
**
**	OUTPUT:
**
**		gctUINT * MaxWidth
**			Pointer to a variable receiving the maximum width of a render
**			target.
**
**		gctUINT * MaxHeight
**			Pointer to a variable receiving the maximum height of a render
**			target.
**
**		gctUINT * MultiTargetCount
**			Pointer to a variable receiving the number of render targets.
**
**		gctUINT * MaxSamples
**			Pointer to a variable receiving the maximum number of samples per
**			pixel for MSAA.
*/
gceSTATUS
gcoHARDWARE_QueryTargetCaps(
	IN gcoHARDWARE Hardware,
	OUT gctUINT * MaxWidth,
	OUT gctUINT * MaxHeight,
	OUT gctUINT * MultiTargetCount,
	OUT gctUINT * MaxSamples
	)
{
	gcmHEADER_ARG("Hardware=0x%x MaxWidth=0x%x MaxHeight=0x%x "
					"MultiTargetCount=0x%x MaxSamples=0x%x",
					Hardware, MaxWidth, MaxHeight,
					MultiTargetCount, MaxSamples);

	if (MaxWidth != gcvNULL)
	{
		/* Return maximum width of render target for XAQ2. */
		*MaxWidth = 2048;
	}

	if (MaxHeight != gcvNULL)
	{
		/* Return maximum height of render target for XAQ2. */
		*MaxHeight = 2048;
	}

	if (MultiTargetCount != gcvNULL)
	{
		/* Return number of render targets for XAQ2. */
		*MultiTargetCount = 1;
	}

	if (MaxSamples != gcvNULL)
	{
		/* Return number of samples per pixel for XAQ2. */
		if (((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 7:7) & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))))
		{
			*MaxSamples = 4;
		}
		else
		{
			*MaxSamples = 0;
		}
#if USE_SUPER_SAMPLING
		if (*MaxSamples == 0)
		{
			*MaxSamples = 4;
		}
#endif
	}

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoHARDWARE_QueryIndexCaps
**
**	Query the index capabilities.
**
**	INPUT:
**
**		Nothing.
**
**	OUTPUT:
**
**		gctBOOL * Index8
**			Pointer to a variable receiving the capability for 8-bit indices.
**
**		gctBOOL * Index16
**			Pointer to a variable receiving the capability for 16-bit indices.
**			target.
**
**		gctBOOL * Index32
**			Pointer to a variable receiving the capability for 32-bit indices.
**
**		gctUINT * MaxIndex
**			Maximum number of indices.
*/
gceSTATUS
gcoHARDWARE_QueryIndexCaps(
	OUT gctBOOL * Index8,
	OUT gctBOOL * Index16,
	OUT gctBOOL * Index32,
	OUT gctUINT * MaxIndex
	)
{
	gcmHEADER_ARG("Index8=0x%x Index16=0x%x Index32=0x%x MaxIndex=0x%x",
					Index8, Index16, Index32, MaxIndex);

	if (Index8 != gcvNULL)
	{
		/* XAQ2 supports 8-bit indices. */
		*Index8 = gcvTRUE;
	}

	if (Index16 != gcvNULL)
	{
		/* XAQ2 supports 16-bit indices. */
		*Index16 = gcvTRUE;
	}

	if (Index32 != gcvNULL)
	{
		/* XAQ2 does not support 32-bit indices. */
		*Index32 = gcvFALSE;
	}

	if (MaxIndex != gcvNULL)
	{
		/* XAQ2 supports up to 2**16 indices. */
		*MaxIndex = 1 << 16;
	}

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoHARDWARE_QueryShaderCaps
**
**	Query the shader capabilities.
**
**	INPUT:
**
**		Nothing.
**
**	OUTPUT:
**
**		gctUINT * VertexUniforms
**			Pointer to a variable receiving the number of uniforms in the vertex
**			shader.
**
**		gctUINT * FragmentUniforms
**			Pointer to a variable receiving the number of uniforms in the
**			fragment shader.
**
**		gctUINT * Varyings
**			Pointer to a variable receiving the maimum number of varyings.
*/
gceSTATUS
gcoHARDWARE_QueryShaderCaps(
	OUT gctUINT * VertexUniforms,
	OUT gctUINT * FragmentUniforms,
	OUT gctUINT * Varyings
	)
{
	gcmHEADER_ARG("VertexUniforms=0x%x FragmentUniforms=0x%x Varyings=0x%x",
					VertexUniforms, FragmentUniforms, Varyings);

	if (VertexUniforms != gcvNULL)
	{
		/* XAQ2 has 160 uniforms for the vertex shader. */
		*VertexUniforms = 160;
	}

	if (FragmentUniforms != gcvNULL)
	{
		/* XAQ2 has 64 uniforms for the fragment shader. */
		*FragmentUniforms = 64;
	}

	if (Varyings != gcvNULL)
	{
		/* XAQ2 has up to 8 varyings. */
		*Varyings = 8;
	}

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoHARDWARE_QueryTileStatus
**
**	Query the linear size for a tile size buffer.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to gcoHARDWARE object.
**
**		gctUINT Width, Height
**          Width and height of the surface.
**
**		gctSZIE_T Bytes
**          Size of the surface in bytes.
**
**	OUTPUT:
**
**		gctSIZE_T_PTR Size
**			Pointer to a variable receiving the number of bytes required to
**          store the tile status buffer.
**
**		gctSIZE_T_PTR Alignment
**			Pointer to a variable receiving the alignment requirement.
**
**		gctUINT32_PTR Filler
**			Pointer to a variable receiving the tile status filler for fast
**			clear.
*/
gceSTATUS
gcoHARDWARE_QueryTileStatus(
	IN gcoHARDWARE Hardware,
    IN gctUINT Width,
    IN gctUINT Height,
    IN gctSIZE_T Bytes,
    OUT gctSIZE_T_PTR Size,
    OUT gctUINT_PTR Alignment,
	OUT gctUINT32_PTR Filler
    )
{
    gctUINT width, height;
	gctBOOL is2BitPerTile;

	gcmHEADER_ARG("Hardware=0x%x Width=%d Height=%d "
					"Bytes=%d Size=0x%x Alignment=0x%x "
					"Filler=0x%x",
					Hardware, Width, Height,
					Bytes, Size, Alignment,
					Filler);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmVERIFY_ARGUMENT(Size != gcvNULL);

	/* See if tile status is supported. */
	if (!( ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 0:0)) & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1)))))) ))
	{
		gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
		return gcvSTATUS_NOT_SUPPORTED;
	}

	/* Check tile status size. */
	is2BitPerTile = ((((gctUINT32) (Hardware->chipMinorFeatures)) >> (0 ? 10:10) & ((gctUINT32) ((((1 ? 10:10) - (0 ? 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 10:10) - (0 ? 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1)))))));

	if ((Hardware->chipModel == gcv500)
	&&  (Hardware->chipRevision > 2)
	)
	{
		/* Compute the aligned number of tiles for the surface. */
		width  = gcmALIGN(Width,  4) >> 2;
		height = gcmALIGN(Height, 4) >> 2;

		/* Return the linear size. */
		*Size = gcmALIGN(width * height * 4 / 8, 256);
	}
	else
	{
		if ((Width == 0) && (Height == 0))
		{
			*Size = gcmALIGN(Bytes / 16 / 4, 16 * 4 * 4);
		}
		else
		{
			/* Return the linear size. */
			*Size = is2BitPerTile ? gcmALIGN(Bytes >> 8, 256)
								  : gcmALIGN(Bytes >> 7, 256);
		}
	}

	if (Alignment != gcvNULL)
	{
		/* Set alignment. */
		*Alignment = 64;
	}

	if (Filler != gcvNULL)
	{
		*Filler = (  (Hardware->chipModel == gcv500)
				  && (Hardware->chipRevision > 2)
				  )
				  ? 0xFFFFFFFF
				  : is2BitPerTile ? 0x55555555
								  : 0x11111111;
	}

    /* Success. */
	gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gcoHARDWARE_QuerySamplerBase(
	IN gcoHARDWARE Hardware,
	OUT gctSIZE_T * VertexCount,
	OUT gctINT_PTR VertexBase,
	OUT gctSIZE_T * FragmentCount,
	OUT gctINT_PTR FragmentBase
	)
{
	gcmHEADER_ARG("Hardware=0x%x VertexCount=0x%x VertexBase=0x%x "
					"FragmentCount=0x%x FragmentBase=0x%x",
					Hardware, VertexCount, VertexBase,
					FragmentCount, FragmentBase);

	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	if (VertexCount != gcvNULL)
	{
		if (Hardware->chipModel > gcv500)
		{
			*VertexCount = 4;
		}
		else
		{
			*VertexCount = 0;
		}
	}

	if (VertexBase != gcvNULL)
	{
		*VertexBase = 8;
	}

	if (FragmentCount != gcvNULL)
	{
		*FragmentCount = 8;
	}

	if (FragmentBase != gcvNULL)
	{
		*FragmentBase = 0;
	}

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

