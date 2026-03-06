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


#include "gc_hal_user.h"

/******************************************************************************\
***************************** Filter Blit Defines *****************************
\******************************************************************************/

#define gcvMAXKERNELSIZE		9
#define gcvSUBPIXELINDEXBITS	5

#define gcvSUBPIXELCOUNT \
	(1 << gcvSUBPIXELINDEXBITS)

#define gcvSUBPIXELLOADCOUNT \
	(gcvSUBPIXELCOUNT / 2 + 1)

#define gcvWEIGHTSTATECOUNT \
	(((gcvSUBPIXELLOADCOUNT * gcvMAXKERNELSIZE + 1) & ~1) / 2)

#define gcvKERNELTABLESIZE \
	(gcvSUBPIXELLOADCOUNT * gcvMAXKERNELSIZE * sizeof(gctUINT16))

/******************************************************************************\
********************************* Support Code *********************************
\******************************************************************************/

/*******************************************************************************
**
**	_SincFilter
**
**	Sinc filter function.
**
**	INPUT:
**
**		gctFLOAT x
**			x coordinate.
**
**		gctINT32 radius
**			Radius of the filter.
**
**	OUTPUT:
**
**		Nothing.
**
**	RETURN:
**
**		gctFLOAT
**			Function value at x.
*/
static gctFLOAT _SincFilter(
	gctFLOAT x,
	gctINT32 radius
	)
{
	gctFLOAT pit, pitd, f1, f2, result;

	if (x == 0.0f)
	{
		result = 1.0f;
	}
	else if ((x < -radius) || (x > radius))
	{
		result = 0.0f;
	}
	else
	{
		pit  = gcdPI * x;
		pitd = pit / radius;

		f1 = gcoMATH_Sine(pit)  / pit;
		f2 = gcoMATH_Sine(pitd) / pitd;

		result = f1 * f2;
	}

	return result;
}

/*******************************************************************************
**
**	_CalculateSyncTable
**
**	Calculate weight array for sync filter.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to a gcoHARDWARE object.
**
**		gctUINT8 KernelSize
**			New kernel size.
**
**		gctUINT32 SrcSize
**			The size in pixels of a source dimension (width or height).
**
**		gctUINT32 DestSize
**			The size in pixels of a destination dimension (width or height).
**
**	OUTPUT:
**
**		gcsFILTER_BLIT_ARRAY_PTR KernelInfo
**			Updated kernel structure and table.
*/
static gceSTATUS _CalculateSyncTable(
	IN gcoHARDWARE Hardware,
	IN gctUINT8 KernelSize,
	IN gctUINT32 SrcSize,
	IN gctUINT32 DestSize,
	OUT gcsFILTER_BLIT_ARRAY_PTR KernelInfo
	)
{
	gceSTATUS status = gcvSTATUS_OK;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
	gcmVERIFY_ARGUMENT(KernelInfo != gcvNULL);
	gcmVERIFY_ARGUMENT(KernelInfo->filterType == gcvFILTER_SYNC);
	gcmVERIFY_ARGUMENT(SrcSize != 0);
	gcmVERIFY_ARGUMENT(DestSize != 0);

	do
	{
		gctUINT32 scaleFactor;
		gctFLOAT fScale;
		gctUINT32 kernelHalf;
		gctFLOAT fSubpixelStep;
		gctFLOAT fSubpixelOffset;
		gctUINT32 subpixelPos;
		gctINT kernelPos;
		gctINT padding;
		gctUINT16_PTR kernelArray;

		/* Compute the scale factor. */
		scaleFactor = gcoHARDWARE_GetStretchFactor(SrcSize, DestSize);

		/* Same kernel size and ratio as before? */
		if ((KernelInfo->kernelSize  == KernelSize) &&
			(KernelInfo->scaleFactor == scaleFactor))
		{
			break;
		}

		/* Allocate the array if not allocated yet. */
		if (KernelInfo->kernelArray == gcvNULL)
		{
			/* Allocate the array. */
			gcmERR_BREAK(gcoOS_Allocate(
				Hardware->os,
				gcvKERNELTABLESIZE,
				(gctPOINTER)&KernelInfo->kernelArray
				));
		}

		/* Store new parameters. */
		KernelInfo->kernelSize  = KernelSize;
		KernelInfo->scaleFactor = scaleFactor;

		/* Compute the scale factor. */
		fScale = (gctFLOAT) DestSize / (gctFLOAT) SrcSize;

		/* Adjust the factor for magnification. */
		if (fScale > 1.0f)
		{
			fScale = 1.0f;
		}

		/* Calculate the kernel half. */
		kernelHalf = KernelInfo->kernelSize >> 1;

		/* Calculate the subpixel step. */
		fSubpixelStep = 1.0f / gcvSUBPIXELCOUNT;

		/* Init the subpixel offset. */
		fSubpixelOffset = 0.5f;

		/* Determine kernel padding size. */
		padding = (gcvMAXKERNELSIZE - KernelInfo->kernelSize) / 2;

		/* Set initial kernel array pointer. */
		kernelArray = KernelInfo->kernelArray;

		/* Loop through each subpixel. */
		for (subpixelPos = 0; subpixelPos < gcvSUBPIXELLOADCOUNT; subpixelPos++)
		{
			/* Define a temporary set of weights. */
			gctFLOAT fSubpixelSet[gcvMAXKERNELSIZE];

			/* Init the sum of all weights for the current subpixel. */
			gctFLOAT fWeightSum = 0;

			/* Compute weights. */
			for (kernelPos = 0; kernelPos < gcvMAXKERNELSIZE; kernelPos++)
			{
				/* Determine the current index. */
				gctINT index = kernelPos - padding;

				/* Pad with zeros. */
				if ((index < 0) || (index >= KernelInfo->kernelSize))
				{
					fSubpixelSet[kernelPos] = 0.0f;
				}
				else
				{
					if (KernelInfo->kernelSize == 1)
					{
						fSubpixelSet[kernelPos] = 1.0f;
					}
					else
					{
						/* Compute the x position for filter function. */
						gctFLOAT fX
							= (index - ((gctINT) kernelHalf) + fSubpixelOffset)
							* fScale;

						/* Compute the weight. */
						fSubpixelSet[kernelPos] = _SincFilter(fX, kernelHalf);
					}

					/* Update the sum of weights. */
					fWeightSum += fSubpixelSet[kernelPos];
				}
			}

			/* Adjust weights so that the sum will be 1.0. */
			for (kernelPos = 0; kernelPos < gcvMAXKERNELSIZE; kernelPos++)
			{
				/* Normalize the current weight. */
				gctFLOAT fWeight = fSubpixelSet[kernelPos] / fWeightSum;

				/* Convert the weight to fixed point and store in the table. */
				if (!fWeight)
				{
					*kernelArray++ = 0x0000;
				}
				else if (fWeight >= 1.0f)
				{
					*kernelArray++ = 0x4000;
				}
				else if (fWeight <= -1.0f)
				{
					*kernelArray++ = 0xC000;
				}
				else
				{
					*kernelArray++ = (gctINT16) (gctINT) (fWeight * (1 << 14));
				}
			}

			/* Advance to the next subpixel. */
			fSubpixelOffset -= fSubpixelStep;
		}

		KernelInfo->kernelChanged = gcvTRUE;
	}
	while (gcvFALSE);

	return status;
}

/*******************************************************************************
**
**	_CalculateBlurTable
**
**	Calculate weight array for blur filter.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to a gcoHARDWARE object.
**
**		gctUINT8 KernelSize
**			New kernel size.
**
**		gctUINT32 SrcSize
**			The size in pixels of a source dimension (width or height).
**
**		gctUINT32 DestSize
**			The size in pixels of a destination dimension (width or height).
**
**	OUTPUT:
**
**		gcsFILTER_BLIT_ARRAY_PTR KernelInfo
**			Updated kernel structure and table.
*/
static gceSTATUS _CalculateBlurTable(
	IN gcoHARDWARE Hardware,
	IN gctUINT8 KernelSize,
	IN gctUINT32 SrcSize,
	IN gctUINT32 DestSize,
	OUT gcsFILTER_BLIT_ARRAY_PTR KernelInfo
	)
{
	gceSTATUS status = gcvSTATUS_OK;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
	gcmVERIFY_ARGUMENT(KernelInfo != gcvNULL);
	gcmVERIFY_ARGUMENT(KernelInfo->filterType == gcvFILTER_BLUR);
	gcmVERIFY_ARGUMENT(SrcSize != 0);
	gcmVERIFY_ARGUMENT(DestSize != 0);

	do
	{
		gctUINT32 scaleFactor;
		gctUINT32 subpixelPos;
		gctINT kernelPos;
		gctINT padding;
		gctUINT16_PTR kernelArray;

		/* Compute the scale factor. */
		scaleFactor = gcoHARDWARE_GetStretchFactor(SrcSize, DestSize);

		/* Same kernel size and ratio as before? */
		if ((KernelInfo->kernelSize  == KernelSize) &&
			(KernelInfo->scaleFactor == scaleFactor))
		{
			break;
		}

		/* Allocate the array if not allocated yet. */
		if (KernelInfo->kernelArray == gcvNULL)
		{
			/* Allocate the array. */
			gcmERR_BREAK(gcoOS_Allocate(
				Hardware->os,
				gcvKERNELTABLESIZE,
				(gctPOINTER)&KernelInfo->kernelArray
				));
		}

		/* Store new parameters. */
		KernelInfo->kernelSize  = KernelSize;
		KernelInfo->scaleFactor = scaleFactor;

		/* Determine kernel padding size. */
		padding = (gcvMAXKERNELSIZE - KernelInfo->kernelSize) / 2;

		/* Set initial kernel array pointer. */
		kernelArray = KernelInfo->kernelArray;

		/* Loop through each subpixel. */
		for (subpixelPos = 0; subpixelPos < gcvSUBPIXELLOADCOUNT; subpixelPos++)
		{
			/* Compute weights. */
			for (kernelPos = 0; kernelPos < gcvMAXKERNELSIZE; kernelPos++)
			{
				/* Determine the current index. */
				gctINT index = kernelPos - padding;

				/* Pad with zeros. */
				if ((index < 0) || (index >= KernelInfo->kernelSize))
				{
					*kernelArray++ = 0x0000;
				}
				else
				{
					if (KernelInfo->kernelSize == 1)
					{
						*kernelArray++ = 0x4000;
					}
					else
					{
						gctFLOAT fWeight;

						/* Compute the weight. */
						fWeight = 1 / (gctFLOAT)KernelInfo->kernelSize;
						*kernelArray++ = (gctINT16) (gctINT) (fWeight * (1 << 14));
					}
				}
			}
		}

		KernelInfo->kernelChanged = gcvTRUE;
	}
	while (gcvFALSE);

	return status;
}

/*******************************************************************************
**
**	_LoadKernel
**
**	Program kernel size and kernel weight table.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gcsFILTER_BLIT_ARRAY_PTR Kernel
**			Pointer to kernel array info structure.
**
**	OUTPUT:
**
**		Nothing.
*/
static gceSTATUS _LoadKernel(
	IN gcoHARDWARE Hardware,
	IN gcsFILTER_BLIT_ARRAY_PTR Kernel
	)
{
	gceSTATUS status;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		/* Different? */
		if ((Hardware->loadedFilterType  == Kernel->filterType) &&
			(Hardware->loadedKernelSize  == Kernel->kernelSize) &&
			(Hardware->loadedScaleFactor == Kernel->scaleFactor) &&
			(Kernel->kernelChanged		 == gcvFALSE))
		{
			/* Nope, still the same. */
			status = gcvSTATUS_OK;
			break;
		}

		/* SelectPipe(2D). */
		gcmERR_BREAK(gcoHARDWARE_SelectPipe(Hardware, 0x1));

		/* LoadState(AQDE_FILTER_KERNEL) */
		gcmERR_BREAK(gcoHARDWARE_LoadState(
			Hardware,
			0x01800, gcvWEIGHTSTATECOUNT,
			Kernel->kernelArray
			));

		/* Update current values. */
		Hardware->loadedFilterType  = Kernel->filterType;
		Hardware->loadedKernelSize  = Kernel->kernelSize;
		Hardware->loadedScaleFactor = Kernel->scaleFactor;

		Kernel->kernelChanged 		= gcvFALSE;
	}
	while (gcvFALSE);

	/* Return status. */
	return status;
}

/*******************************************************************************
**
**	_SetVideoSource
**
**	Program video source registers.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**      gcsSURF_INFO_PTR Surface
**          Pointer to the source surface descriptor.
**
**		gctUINT32 Address
**		gctUINT32 Stride
**		gctUINT32 UAddress
**		gctUINT32 UStride
**		gctUINT32 VAddress
**		gctUINT32 VStride
**			Addresses and strides of the source.
**
**		gceSURF_FORMAT Format
**			Source surface format.
**
**		gcsRECT_PTR Rect
**			Source image rectangle.
**
**		gcsPOINT_PTR Origin
**			Source origin within the image rectangle.
**
**	OUTPUT:
**
**		Nothing.
*/
static gceSTATUS _SetVideoSource(
	IN gcoHARDWARE Hardware,
	IN gcsSURF_INFO_PTR Surface,
	IN gcsRECT_PTR Rect,
	IN gcsPOINT_PTR Origin
	)
{
	gceSTATUS status;
	gctUINT32 rotated;
	gctUINT32 memory[4];
	gctUINT32 format, swizzle, isYUVformat;
	gctUINT32 rgbaSwizzle, uvSwizzle;
	gctUINT32 endian;

	/* Check the rotation capability. */
	if (!Hardware->fullFilterBlitRotation &&
		Surface->rotation != gcvSURF_0_DEGREE)
	{
		gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
	}

	rotated = gcvFALSE;

	/* Load AQDE_SRC_ROTATION_CONFIG_Address. */
	gcmONERROR(gcoHARDWARE_LoadState32(
			Hardware,
			0x01208,
			((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (Surface->alignedWidth) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))) | (((gctUINT32) ((gctUINT32) (rotated) & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)))

			));

	/*--------------------------------------------------------------------*/
	memory[0] = Surface->node.physical;
	memory[1] = Surface->stride;

	gcmONERROR(gcoHARDWARE_LoadState(
		Hardware,
		0x01200,
		2,
		memory
		));

	/*--------------------------------------------------------------------*/
	gcmONERROR(gcoHARDWARE_TranslateSourceFormat(
		Hardware, Surface->format, &format, &swizzle, &isYUVformat
		));

	/* Set endian control */
	endian = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));

	if (Hardware->bigEndian)
	{
		gctUINT32 bpp;

		/* Compute bits per pixel. */
		gcmONERROR(gcoHARDWARE_ConvertFormat(Hardware,
											 Surface->format,
											 &bpp,
											 gcvNULL));

		if (bpp == 16)
		{
			endian = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
		}
		else if (bpp == 32)
		{
			endian = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
		}
	}

	/* Determine color swizzle. */
	if (isYUVformat)
	{
		rgbaSwizzle = 0x0;
		uvSwizzle   = swizzle;
	}
	else
	{
		rgbaSwizzle = swizzle;
		uvSwizzle   = 0x0;
	}

	gcmONERROR(gcoHARDWARE_LoadState32(
		Hardware,
		0x0120C,
		  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))

		| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 28:24) - (0 ? 28:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:24) - (0 ? 28:24) + 1))))))) << (0 ? 28:24))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 28:24) - (0 ? 28:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:24) - (0 ? 28:24) + 1))))))) << (0 ? 28:24)))

		| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) | (((gctUINT32) ((gctUINT32) (rgbaSwizzle) & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)))

		| endian
		));

	/* Load source UV swizzle state. */
	gcmONERROR(gcoHARDWARE_LoadState32(
		Hardware,
		0x012D8,
		(    ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) | (((gctUINT32) ((gctUINT32) ((uvSwizzle)) & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7))) | (((gctUINT32) (0x0&((gctUINT32)((((1?7:7)-(0?7:7)+1)==32)?~0:(~(~0<<((1?7:7)-(0?7:7)+1)))))))<<(0?7:7))))
		));

	/*--------------------------------------------------------------------*/
	memory[0] = Surface->node.physical2;
	memory[1] = Surface->uStride;
	memory[2] = Surface->node.physical3;
	memory[3] = Surface->vStride;

	gcmONERROR(gcoHARDWARE_LoadState(
		Hardware,
		0x01284,
		4,
		memory
		));

	/*--------------------------------------------------------------------*/
	memory[0]
		= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (Rect->left) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

		| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (Rect->top) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));
	memory[1]
		= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (Rect->right) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

		| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (Rect->bottom) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));
	memory[2] = Origin->x;
	memory[3] = Origin->y;

	gcmONERROR(gcoHARDWARE_LoadState(
		Hardware,
		0x01298,
		4,
		memory
		));

	if (Hardware->fullFilterBlitRotation)
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
			gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
		}

		/* Load source height. */
		gcmONERROR(gcoHARDWARE_LoadState32(
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
			value = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0))) | (((gctUINT32) ((gctUINT32) (srcRot) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))

						| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8))) | (((gctUINT32) (0x0&((gctUINT32)((((1?8:8)-(0?8:8)+1)==32)?~0:(~(~0<<((1?8:8)-(0?8:8)+1)))))))<<(0?8:8)))
						| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9)));
		}

		gcmONERROR(gcoHARDWARE_LoadState32(
					Hardware,
					0x012BC,
					value
					));
	}

	/* Success. */
	return gcvSTATUS_OK;

OnError:
	/* Return the error. */
	return status;
}

/*******************************************************************************
**
**	_SetVideoDestination
**
**	Program video destination registers.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**      gcsSURF_INFO_PTR Surface
**          Pointer to the destination surface descriptor.
**
**		gcsRECT_PTR Rect
**			Destination image rectangle.
**
**	OUTPUT:
**
**		Nothing.
*/
static gceSTATUS _SetVideoDestination(
	IN gcoHARDWARE Hardware,
	IN gcsSURF_INFO_PTR Surface,
	IN gcsRECT_PTR Rect
	)
{
	gceSTATUS status;
	gctUINT32 rotated;
	gctUINT32 memory[2];
	gctUINT32 format, swizzle, isYUVformat;
	gctUINT32 endian;

	/* Check the rotation capability. */
	if (!Hardware->fullFilterBlitRotation &&
		Surface->rotation != gcvSURF_0_DEGREE)
	{
		gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
	}

	rotated = gcvFALSE;

	/* Load AQDE_DEST_ROTATION_CONFIG_Address. */
	gcmONERROR(gcoHARDWARE_LoadState32(
			Hardware,
			0x01230,
			((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (Surface->alignedWidth) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

				| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))) | (((gctUINT32) ((gctUINT32) (rotated) & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)))

			));

	/*--------------------------------------------------------------------*/
	memory[0] = Surface->node.physical;
	memory[1] = Surface->stride;

	gcmONERROR(gcoHARDWARE_LoadState(
		Hardware,
		0x01228,
		2,
		memory
		));

	/*--------------------------------------------------------------------*/
	gcmONERROR(gcoHARDWARE_TranslateDestinationFormat(
		Hardware, Surface->format, &format, &swizzle, &isYUVformat
		));

	/* Set endian control */
	endian = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));

	if (Hardware->bigEndian)
	{
		gctUINT32 bpp;

		/* Flush the current pipe. */
		gcmONERROR(gcoHARDWARE_FlushPipe(Hardware));

		/* Compute bits per pixel. */
		gcmONERROR(gcoHARDWARE_ConvertFormat(Hardware,
											 Surface->format,
											 &bpp,
											 gcvNULL));

		if (bpp == 16)
		{
			endian = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
		}
		else if (bpp == 32)
		{
			endian = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
		}
	}

	gcmONERROR(gcoHARDWARE_LoadState32(
		Hardware,
		0x01234,
		((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))|endian
		));

	/*--------------------------------------------------------------------*/
	memory[0]
		= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (Rect->left) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

		| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (Rect->top) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));
	memory[1]
		= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (Rect->right) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

		| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (Rect->bottom) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

	gcmONERROR(gcoHARDWARE_LoadState(
		Hardware,
		0x012A8,
		2,
		memory
		));

	if (Hardware->fullFilterBlitRotation)
	{
		gctUINT32 dstRot = 0;
		gctUINT32 value;

		switch (Surface->rotation)
		{
		case gcvSURF_0_DEGREE:
			dstRot = 0x0;
			break;

		case gcvSURF_90_DEGREE:
			dstRot = 0x4;
			break;

		case gcvSURF_180_DEGREE:
			dstRot = 0x5;
			break;

		case gcvSURF_270_DEGREE:
			dstRot = 0x6;
			break;

		default:
			gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
		}

		/* Load target height. */
		gcmONERROR(gcoHARDWARE_LoadState32(
			Hardware,
			0x012B4,
			((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (Surface->alignedHeight) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

			));

		/* 0x012BC */
		if (Hardware->shadowRotAngleReg)
		{
			value = ((((gctUINT32) (Hardware->rotAngleRegShadow)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3))) | (((gctUINT32) ((gctUINT32) (dstRot) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)));

			/* Save the shadow value. */
			Hardware->rotAngleRegShadow = value;
		}
		else
		{
			value = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3))) | (((gctUINT32) ((gctUINT32) (dstRot) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))

						| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)))
						| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9))) | (((gctUINT32) (0x0&((gctUINT32)((((1?9:9)-(0?9:9)+1)==32)?~0:(~(~0<<((1?9:9)-(0?9:9)+1)))))))<<(0?9:9)));
		}

		gcmONERROR(gcoHARDWARE_LoadState32(
			Hardware,
			0x012BC,
			value
			));
	}

	/* Success. */
	return gcvSTATUS_OK;

OnError:
	/* Return status. */
	return status;
}

/*******************************************************************************
**
**	_StartVR
**
**	Start video raster engine.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gctBOOL Horizontal
**			Set to gcvTRUE to start horizontal blit.
**			Set to gcvFALSE to start vertical blit.
**
**	OUTPUT:
**
**		Nothing.
*/
static gceSTATUS _StartVR(
	IN gcoHARDWARE Hardware,
	IN gctBOOL Horizontal
	)
{
	gceSTATUS status;

	do
	{
		gctUINT32 blitType = Horizontal
			? 0x0
			: 0x1;

		/*******************************************************************
		** Setup ROP.
		*/

		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			0x0125C,
			  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)))
			| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8))) | (((gctUINT32) ((gctUINT32) (0xCC) & ((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8)))

			| ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0))) | (((gctUINT32) ((gctUINT32) (0xCC) & ((gctUINT32) ((((1 ? 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0)))

			));

		gcmERR_BREAK(gcoHARDWARE_LoadState32(
			Hardware,
			0x01294,
			((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3))) | (((gctUINT32) (0x0&((gctUINT32)((((1?3:3)-(0?3:3)+1)==32)?~0:(~(~0<<((1?3:3)-(0?3:3)+1)))))))<<(0?3:3)))&
			((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) | (((gctUINT32) ((gctUINT32) (blitType) & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))

			));
	}
	while (gcvFALSE);

	/* Return status. */
	return status;
}

/*******************************************************************************
**
**	_DestroyKernelArray
**
**	Destroy the kernel array.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gcsFILTER_BLIT_ARRAY_PTR KernelInfo
**			Pointer to kernel array info structure.
**
**	OUTPUT:
**
**		Nothing.
*/
static gceSTATUS _DestroyKernelArray(
	IN gcoHARDWARE Hardware,
	IN gcsFILTER_BLIT_ARRAY_PTR KernelInfo
	)
{
	gceSTATUS status = gcvSTATUS_OK;

	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
	gcmVERIFY_ARGUMENT(KernelInfo != gcvNULL);

	if (KernelInfo->kernelArray != gcvNULL)
	{
		/* Free the array. */
		status = gcoOS_Free(
			Hardware->os,
			KernelInfo->kernelArray
			);

		/* Reset the pointer. */
		KernelInfo->kernelArray = gcvNULL;
	}

	return status;
}

/******************************************************************************\
****************************** gcoHARDWARE API Code *****************************
\******************************************************************************/

/*******************************************************************************
**
**	gcoHARDWARE_SetKernelSize
**
**	Set kernel size.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
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
gceSTATUS gcoHARDWARE_SetKernelSize(
	IN gcoHARDWARE Hardware,
	IN gctUINT8 HorKernelSize,
	IN gctUINT8 VerKernelSize
	)
{
	gcmHEADER_ARG("Hardware=0x%x HorKernelSize=%d VerKernelSize=%d",
					Hardware, HorKernelSize, VerKernelSize);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
	gcmVERIFY_ARGUMENT(HorKernelSize <= gcvMAXKERNELSIZE);
	gcmVERIFY_ARGUMENT(VerKernelSize <= gcvMAXKERNELSIZE);
	gcmVERIFY_ARGUMENT((HorKernelSize & 1) == 1);
	gcmVERIFY_ARGUMENT((VerKernelSize & 1) == 1);

	/* Set sizes. */
	Hardware->newHorKernelSize = HorKernelSize;
	Hardware->newVerKernelSize = VerKernelSize;

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoHARDWARE_SetFilterType
**
**	Set filter type.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gceFILTER_TYPE FilterType
**			Filter type for the filter blit.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_SetFilterType(
	IN gcoHARDWARE Hardware,
	IN gceFILTER_TYPE FilterType
	)
{
	gcmHEADER_ARG("Hardware=0x%x FilterType=%d", Hardware, FilterType);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	/* Set the new filter type. */
	switch (FilterType)
	{
	case gcvFILTER_SYNC:
		Hardware->newFilterType = gcvFILTER_SYNC;
		break;

	case gcvFILTER_BLUR:
		Hardware->newFilterType = gcvFILTER_BLUR;
		break;

	case gcvFILTER_USER:
		Hardware->newFilterType = gcvFILTER_USER;
		break;

	default:
		gcmASSERT(gcvFALSE);
		gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
		return gcvSTATUS_NOT_SUPPORTED;
	}

	/* Return status. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoHARDWARE_SetUserFilterKernel
**
**	Set the user defined filter kernel.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gceFILTER_PASS_TYPE PassType
**			Pass type for the filter blit.
**
**		gctUINT16_PTR KernelArray
**			Pointer to the kernel array from user.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_SetUserFilterKernel(
	IN gcoHARDWARE Hardware,
	IN gceFILTER_PASS_TYPE PassType,
	IN gctUINT16_PTR KernelArray
	)
{
	gceSTATUS status = gcvSTATUS_OK;

	gcmHEADER_ARG("Hardware=0x%x PassType=%d KernelArray=0x%x",
					Hardware, PassType, KernelArray);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
	gcmVERIFY_ARGUMENT(KernelArray != gcvNULL);

	do
	{
		gcsFILTER_BLIT_ARRAY_PTR kernelInfo = gcvNULL;

		if (PassType == gcvFILTER_HOR_PASS)
		{
			kernelInfo = &Hardware->horUserFilterKernel;
		}
		else if (PassType == gcvFILTER_VER_PASS)
		{
			kernelInfo = &Hardware->verUserFilterKernel;
		}
		else
		{
			gcmTRACE_ZONE(gcvLEVEL_ERROR,
						  gcvZONE_HARDWARE,
						  "Unknown filter pass type.");

			status = gcvSTATUS_NOT_SUPPORTED;
			break;
		}

		/* Allocate the array if not allocated yet. */
		if (kernelInfo->kernelArray == gcvNULL)
		{
			/* Allocate the array. */
			gcmERR_BREAK(gcoOS_Allocate(
				Hardware->os,
				gcvKERNELTABLESIZE,
				(gctPOINTER)&kernelInfo->kernelArray
				));
		}

		gcmERR_BREAK(gcoOS_MemCopy(
				kernelInfo->kernelArray,
				KernelArray,
				gcvKERNELTABLESIZE
				));

		kernelInfo->kernelChanged = gcvTRUE;
	}
	while (gcvFALSE);

	if (gcmIS_ERROR(status))
	{
		gcmTRACE_ZONE(gcvLEVEL_INFO,
				gcvZONE_HARDWARE,
				"Failed to set the user filter array."
				);
	}

	/* Success. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHARDWARE_EnableUserFilterPasses
**
**	Select the pass(es) to be done for user defined filter.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
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
gceSTATUS gcoHARDWARE_EnableUserFilterPasses(
	IN gcoHARDWARE Hardware,
	IN gctBOOL HorPass,
	IN gctBOOL VerPass
	)
{
	gcmHEADER_ARG("Hardware=0x%x HorPass=%d VerPass=%d",
					Hardware, HorPass, VerPass);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	Hardware->horUserFilterPass = HorPass;
	Hardware->verUserFilterPass = VerPass;

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoHARDWARE_FreeKernelArray
**
**	Frees the kernel weight array.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object that needs to be destroyed.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_FreeKernelArray(
	IN gcoHARDWARE Hardware
	)
{
	gceSTATUS status = gcvSTATUS_OK;

	gcmHEADER_ARG("Hardware=0x%x", Hardware);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	do
	{
		/* Free the horizontal array of the sync filter. */
		gcmERR_BREAK(_DestroyKernelArray(
			Hardware,
			&Hardware->horSyncFilterKernel
			));

		/* Free the vertical array of the sync filter. */
		gcmERR_BREAK(_DestroyKernelArray(
			Hardware,
			&Hardware->verSyncFilterKernel
			));

		/* Free the horizontal array of the blur filter. */
		gcmERR_BREAK(_DestroyKernelArray(
			Hardware,
			&Hardware->horBlurFilterKernel
			));

		/* Free the vertical array of the blur filter. */
		gcmERR_BREAK(_DestroyKernelArray(
			Hardware,
			&Hardware->verBlurFilterKernel
			));

		/* Free the horizontal array of the user filter. */
		gcmERR_BREAK(_DestroyKernelArray(
			Hardware,
			&Hardware->horUserFilterKernel
			));

		/* Free the vertical array of the user filter. */
		gcmERR_BREAK(_DestroyKernelArray(
			Hardware,
			&Hardware->verUserFilterKernel
			));
	}
	while (gcvFALSE);

	/* Return the status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHARDWARE_FreeFilterBuffer
**
**	Frees the temporary buffer allocated by filter blit operation.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_FreeFilterBuffer(
	IN gcoHARDWARE Hardware
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hardware=0x%x", Hardware);

	status = gcoHARDWARE_FreeTemporarySurface(Hardware, gcvTRUE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHARDWARE_FilterBlit
**
**	Filter blit.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**      gcsSURF_INFO_PTR SrcSurface
**          Pointer to the source surface descriptor.
**
**      gcsSURF_INFO_PTR DestSurface
**          Pointer to the destination surface descriptor.
**
**		gcsRECT_PTR SrcRect
**			Coorinates of the entire source image.
**
**		gcsRECT_PTR DestRect
**			Coorinates of the entire destination image.
**
**		gcsRECT_PTR DestSubRect
**			Coordinates of a sub area within the destination to render.
**			The coordinates are relative to the coordinates represented
**			by DestRect. If DestSubRect is gcvNULL, the complete image will
**			be rendered based on DestRect.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoHARDWARE_FilterBlit(
	IN gcoHARDWARE Hardware,
	IN gcsSURF_INFO_PTR SrcSurface,
	IN gcsSURF_INFO_PTR DestSurface,
	IN gcsRECT_PTR SrcRect,
	IN gcsRECT_PTR DestRect,
	IN gcsRECT_PTR DestSubRect
	)
{
	gceSTATUS status = gcvSTATUS_OK;
	gctPOINTER tmpMemory = gcvNULL;

	gcmHEADER_ARG("Hardware=0x%x SrcSurface=0x%x DestSurface=0x%x "
					"SrcRect=0x%x DestRect=0x%x DestSubRect=0x%x",
					Hardware, SrcSurface, DestSurface,
					SrcRect, DestRect, DestSubRect);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
	gcmVERIFY_ARGUMENT(SrcSurface != gcvNULL);
	gcmVERIFY_ARGUMENT(DestSurface != gcvNULL);
	gcmVERIFY_ARGUMENT(SrcRect != gcvNULL);
	gcmVERIFY_ARGUMENT(DestRect != gcvNULL);
	gcmVERIFY_ARGUMENT(DestSubRect != gcvNULL);

	do
	{
		gcsPOINT srcRectSize;
		gcsPOINT destRectSize;

		gctBOOL horPass, verPass;

		gcsSURF_FORMAT_INFO_PTR srcFormat[2];
		gcsSURF_FORMAT_INFO_PTR destFormat[2];

		gcsRECT srcSubRect;
		gcsRECT destRect;

		gcsFILTER_BLIT_ARRAY_PTR horKernel = gcvNULL;
		gcsFILTER_BLIT_ARRAY_PTR verKernel = gcvNULL;

/*----------------------------------------------------------------------------*/
/*------------------- Verify the presence of 2D hardware. --------------------*/

		/* Only supported with hardware 2D engine. */
		if (!Hardware->hw2DEngine || Hardware->sw2DEngine)
		{
			gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
		}

/*----------------------------------------------------------------------------*/
/*------------------------- Compute rectangle sizes. -------------------------*/

		gcmERR_BREAK(gcsRECT_Width(SrcRect, &srcRectSize.x));
		gcmERR_BREAK(gcsRECT_Height(SrcRect, &srcRectSize.y));
		gcmERR_BREAK(gcsRECT_Width(DestRect, &destRectSize.x));
		gcmERR_BREAK(gcsRECT_Height(DestRect, &destRectSize.y));

/*----------------------------------------------------------------------------*/
/*--------------------------- Update kernel arrays. --------------------------*/

		if (Hardware->newFilterType == gcvFILTER_SYNC)
		{
			horPass = gcvTRUE;

			/* Do we need the vertical pass? */
			verPass = (srcRectSize.y != destRectSize.y);

			/* Set the proper kernel array for sync filter. */
			horKernel = &Hardware->horSyncFilterKernel;
			verKernel = &Hardware->verSyncFilterKernel;

			/* Recompute the table if necessary. */
			gcmERR_BREAK(_CalculateSyncTable(
				Hardware,
				Hardware->newHorKernelSize,
				srcRectSize.x,
				destRectSize.x,
				horKernel
				));

			gcmERR_BREAK(_CalculateSyncTable(
				Hardware,
				Hardware->newVerKernelSize,
				srcRectSize.y,
				destRectSize.y,
				verKernel
				));
		}
		else if (Hardware->newFilterType == gcvFILTER_BLUR)
		{
			/* Always do both passes for blur. */
			horPass = verPass = gcvTRUE;

			/* Set the proper kernel array for blur filter. */
			horKernel = &Hardware->horBlurFilterKernel;
			verKernel = &Hardware->verBlurFilterKernel;

			/* Recompute the table if necessary. */
			gcmERR_BREAK(_CalculateBlurTable(
				Hardware,
				Hardware->newHorKernelSize,
				srcRectSize.x,
				destRectSize.x,
				horKernel
				));

			gcmERR_BREAK(_CalculateBlurTable(
				Hardware,
				Hardware->newVerKernelSize,
				srcRectSize.y,
				destRectSize.y,
				verKernel
				));
		}
		else if (Hardware->newFilterType == gcvFILTER_USER)
		{
			gctUINT32 scaleFactor;

			/* Do the pass(es) according to user settings. */
			horPass = Hardware->horUserFilterPass;
			verPass = Hardware->verUserFilterPass;

			/* Set the proper kernel array for user defined filter. */
			horKernel = &Hardware->horUserFilterKernel;
			verKernel = &Hardware->verUserFilterKernel;

			/* Set the kernel size and scale factors. */
			scaleFactor = gcoHARDWARE_GetStretchFactor(srcRectSize.x, destRectSize.x);
			horKernel->kernelSize  = Hardware->newHorKernelSize;
			horKernel->scaleFactor = scaleFactor;

			scaleFactor = gcoHARDWARE_GetStretchFactor(srcRectSize.y, destRectSize.y);
			verKernel->kernelSize  = Hardware->newVerKernelSize;
			verKernel->scaleFactor = scaleFactor;
		}
		else
		{
			gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_HARDWARE, "Unknown filter type");
 			status = gcvSTATUS_NOT_SUPPORTED;

			break;
		}

/*----------------------------------------------------------------------------*/
/*---------------------- Program the stretch factors. ------------------------*/

		/* Program the stretch factors. */
		gcmERR_BREAK(gcoHARDWARE_SetStretchFactors(
			Hardware,
			horKernel->scaleFactor,
			verKernel->scaleFactor
			));

/*----------------------------------------------------------------------------*/
/*------------------- Determine the source sub rectangle. --------------------*/

		/* Compute the source sub rectangle that exactly represents
		   the destination sub rectangle. */
		srcSubRect.left
			= DestSubRect->left
			* horKernel->scaleFactor;
		srcSubRect.top
			= DestSubRect->top
			* verKernel->scaleFactor;
		srcSubRect.right
			= (DestSubRect->right - 1)
			* horKernel->scaleFactor + (1 << 16);
		srcSubRect.bottom
			= (DestSubRect->bottom - 1)
			* verKernel->scaleFactor + (1 << 16);

		/*  Before rendering each destination pixel, the HW will select the
		    corresponding source center pixel to apply the kernel around.
		    To make this process precise we need to add 0.5 to source initial
		    coordinates here; this will make HW pick the next source pixel if
		    the fraction is equal or greater then 0.5. */
		srcSubRect.left   += 0x00008000;
		srcSubRect.top    += 0x00008000;
		srcSubRect.right  += 0x00008000;
		srcSubRect.bottom += 0x00008000;

/*----------------------------------------------------------------------------*/
/*------------------- Compute the destination coordinates. -------------------*/

		/* Determine final destination subrectangle. */
		destRect.left   = DestRect->left + DestSubRect->left;
		destRect.top    = DestRect->top  + DestSubRect->top;
		destRect.right  = DestRect->left + DestSubRect->right;
		destRect.bottom = DestRect->top  + DestSubRect->bottom;

/*----------------------------------------------------------------------------*/
/*------------------ Do the blit with the temporary buffer. ------------------*/

		if (horPass && verPass)
		{
			gctUINT32 horKernelHalf;
			gctUINT32 leftExtra;
			gctUINT32 rightExtra;
			gcsPOINT srcOrigin;
			gcsPOINT tmpRectSize;
			gcsSURF_FORMAT_INFO_PTR tempFormat[2];
			gcsPOINT tempAlignment;
			gctUINT32 tempHorCoordMask;
			gctUINT32 tempVerCoordMask;
			gcsPOINT tempOrigin;
			gcsRECT tempRect;
			gcsPOINT tmpBufRectSize;

			/* In partial filter blit cases, the vertical pass has to render
			   more pixel information to the left and to the right of the
			   temporary image so that the horizontal pass has its necessary
			   kernel information on the edges of the image. */
			horKernelHalf = horKernel->kernelSize >> 1;

			leftExtra  = srcSubRect.left >> 16;
			rightExtra = srcRectSize.x - (srcSubRect.right >> 16);

			if (leftExtra > horKernelHalf)
				leftExtra = horKernelHalf;

			if (rightExtra > horKernelHalf)
				rightExtra = horKernelHalf;

			/* Determine the source origin. */
			srcOrigin.x = ((SrcRect->left - leftExtra) << 16) + srcSubRect.left;
			srcOrigin.y = (SrcRect->top << 16) + srcSubRect.top;

			/* Determine temporary surface format. */
			gcmERR_BREAK(gcoSURF_QueryFormat(SrcSurface->format, srcFormat));
			gcmERR_BREAK(gcoSURF_QueryFormat(DestSurface->format, destFormat));

			if (srcFormat[0]->bitsPerPixel > destFormat[0]->bitsPerPixel)
			{
				tempFormat[0] = srcFormat[0];
				tempFormat[1] = srcFormat[1];
			}
			else
			{
				tempFormat[0] = destFormat[0];
				tempFormat[1] = destFormat[1];
			}

			gcmERR_BREAK(gco2D_GetPixelAlignment(
				tempFormat[0]->format,
				&tempAlignment
				));

			tempHorCoordMask = tempAlignment.x - 1;
			tempVerCoordMask = tempAlignment.y - 1;

			/* Determine the size of the temporary image. */
			tmpRectSize.x
				= leftExtra
				+ ((srcSubRect.right >> 16) - (srcSubRect.left >> 16))
				+ rightExtra;

			tmpRectSize.y
				= DestSubRect->bottom - DestSubRect->top;

			/* Determine the destination origin. */
			tempRect.left = srcOrigin.x >> 16;
			tempRect.top  = DestRect->top + DestSubRect->top;

			/* Align the temporary destination. */
			tempRect.left &= tempHorCoordMask;
			tempRect.top  &= tempVerCoordMask;

			/* Determine the bottom right corner of the destination. */
			tempRect.right  = tempRect.left + tmpRectSize.x;
			tempRect.bottom = tempRect.top  + tmpRectSize.y;

			/* Determine the source origin. */
			tempOrigin.x
				= ((leftExtra + tempRect.left) << 16)
				+ (srcSubRect.left & 0xFFFF);
			tempOrigin.y
				= (tempRect.top << 16)
				+ (srcSubRect.top & 0xFFFF);

			/* Determine the size of the temporaty surface. */
			tmpBufRectSize.x = gcmALIGN(tempRect.right,  tempAlignment.x);
			tmpBufRectSize.y = gcmALIGN(tempRect.bottom, tempAlignment.y);

			/* Allocate the temporary buffer. */
			gcmERR_BREAK(gcoHARDWARE_AllocateTemporarySurface(
				Hardware,
				tmpBufRectSize.x,
				tmpBufRectSize.y,
				tempFormat[0],
				gcvSURF_BITMAP
				));

			/* Lock the temporary surface. */
			gcmERR_BREAK(gcoHARDWARE_Lock(
				Hardware,
				&Hardware->tempBuffer.node,
				gcvNULL,
				gcvNULL
				));

			/* Set logical pointer. */
			tmpMemory = Hardware->tempBuffer.node.logical;

			/*******************************************************************
			** Program the vertical pass.
			*/

			/* Program the kernel if different from the current. */
			gcmERR_BREAK(_LoadKernel(Hardware, verKernel));

			/* Set source. */
			gcmERR_BREAK(_SetVideoSource(
				Hardware,
				SrcSurface,
				SrcRect,
				&srcOrigin
				));

			/* Set destination. */
			gcmERR_BREAK(_SetVideoDestination(
				Hardware,
				&Hardware->tempBuffer,
				&tempRect
				));

			/* Start the vertical blit. */
			gcmERR_BREAK(_StartVR(Hardware, gcvFALSE));

			/*******************************************************************
			** Program the horizontal pass.
			*/

			/* Program the kernel if different from the current. */
			gcmERR_BREAK(_LoadKernel(Hardware, horKernel));

			/* Set source. */
			gcmERR_BREAK(_SetVideoSource(
				Hardware,
				&Hardware->tempBuffer,
				&tempRect,
				&tempOrigin
				));

			/* Set destination. */
			gcmERR_BREAK(_SetVideoDestination(
				Hardware,
				DestSurface,
				&destRect
				));

			/* Start the horizontal blit. */
			gcmERR_BREAK(_StartVR(Hardware, gcvTRUE));
		}

/*----------------------------------------------------------------------------*/
/*---------------------------- One pass only blit. -------------------------*/

		else if (horPass || verPass)
		{
			/* Determine the source origin. */
			gcsPOINT srcOrigin;
			gcsFILTER_BLIT_ARRAY_PTR kernelInfo = gcvNULL;

			srcOrigin.x = (SrcRect->left << 16) + srcSubRect.left;
			srcOrigin.y = (SrcRect->top  << 16) + srcSubRect.top;

			kernelInfo = horPass ? horKernel : verKernel;

			/* Program the kernel if different from the current. */
			gcmERR_BREAK(_LoadKernel(
				Hardware,
				kernelInfo
				));

			/* Set source. */
			gcmERR_BREAK(_SetVideoSource(
				Hardware,
				SrcSurface,
				SrcRect,
				&srcOrigin
				));

			/* Set destination. */
			gcmERR_BREAK(_SetVideoDestination(
				Hardware,
				DestSurface,
				&destRect
				));

			/* Start the blit. */
			gcmERR_BREAK(_StartVR(Hardware, horPass));
		}
/*----------------------------------------------------------------------------*/
/*---------------------------- Should no be here. ----------------------------*/
		else
		{
			gcmTRACE_ZONE(gcvLEVEL_ERROR,
						  gcvZONE_HARDWARE,
						  "None of the passes is set."
						  );

			status = gcvSTATUS_NOT_SUPPORTED;

			break;
		}
	}
	while (gcvFALSE);

	/* Unlock temporary. */
	if (tmpMemory != gcvNULL)
	{
		/* Lock the temporary surface. */
		status = gcoHARDWARE_Unlock(
			Hardware,
			&Hardware->tempBuffer.node,
			Hardware->tempBuffer.type
			);
	}

	/* Return status. */
	gcmFOOTER();
	return status;
}


