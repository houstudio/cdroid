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
#define _GC_OBJ_ZONE    gcvZONE_HARDWARE

/******************************************************************************
 *
 *  deRop3
 *
 *  Perform the specified raster operation on 32-bit data.
 *
 *  Input parameters:
 *
 *      gctUINT8 Rop3
 *          Raster operation code.
 *
 *      gctUINT32 Destination
 *          Destination pixel value.
 *
 *      gctUINT32 Source
 *          Source pixel values.
 *
 *      gctUINT32 Pattern
 *          Pattern value.
 *
 *  Return value:
 *
 *      gctUINT32
 *          32-bit result.
 *
 */
static gctUINT32 _Rop3(
    gctUINT8 Rop3,
    gctUINT32 Source,
    gctUINT32 Pattern,
    gctUINT32 Destination
    )
{
    gctUINT32 i;
    gctUINT32 result = 0;

    for (i = 0; i < 32; i++)
    {
        /* Extract data bits. */
        gctUINT32 sourceBit      = Source      & 1;
        gctUINT32 patternBit     = Pattern     & 1;
        gctUINT32 destinationBit = Destination & 1;

        /* Construct rop bit index. */
        gctUINT32 index
            = (sourceBit  << 1)
            | (patternBit << 2)
            | destinationBit;

        /* Determine the result bit. */
        gctUINT32 resultBit = (Rop3 >> index) & 1;

        /* Set the result. */
        result |= (resultBit << i);

        /* Advance to the next bit. */
        Source      >>= 1;
        Pattern     >>= 1;
        Destination >>= 1;
    }

    return result;
}

/*******************************************************************************
**
**  _RenderRectangle
**
**  2D software renderer.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gce2D_COMMAND Command
**          2D engine command to be executed.
**
**      gctUINT32 RectCount
**          Number of destination rectangles to be operated on.
**
**      gcsRECT_PTR DestRect
**          Pointer to an array of destination rectangles.
**
**      gctUINT32 FgRop
**      gctUINT32 BgRop
**          Foreground and background ROP codes.
**
**  OUTPUT:
**
**      Nothing .
*/
static gceSTATUS _RenderRectangle(
    IN gcoHARDWARE Hardware,
    IN gce2D_COMMAND Command,
    IN gctUINT32 RectCount,
    IN gcsRECT_PTR DestRect,
    IN gctUINT32 FgRop,
    IN gctUINT32 BgRop
    )
{
    gceSTATUS status;

    /* Verify arguments. */
    gcmVERIFY_ARGUMENT(DestRect != gcvNULL);

    do
    {
        gctINT32 initSrcLeft, initTrgLeft;
        gctUINT srcPixelSize, trgPixelSize;
        gctUINT srcStep, trgStep;
        gctINT32 x, y;
        gctINT32 width, height;
        gctUINT8_PTR srcLinePtr;
        gctUINT8_PTR trgLinePtr;
        gcsSURF_FORMAT_INFO_PTR srcFormat[2];
        gcsSURF_FORMAT_INFO_PTR intFormat[2];
        gcsSURF_FORMAT_INFO_PTR trgFormat[2];
        gctUINT32 srcColorMask;
        gctUINT32 transparentColor;
        gctBOOL srcOddStart, trgOddStart;
        gctUINT32 srcPixel[2];
        gctUINT32 trgPixel[2];

        /* Only limited support for now. */
        if ((Command != gcv2D_BLT) ||
            (RectCount != 1))
        {
            gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
        }

        /* Commit any commands buffer and stall. */
        gcmERR_BREAK(gcoHARDWARE_Commit(Hardware));
        gcmERR_BREAK(gcoHARDWARE_Stall(Hardware));

        /* Get format specifics. */
        gcmERR_BREAK(gcoSURF_QueryFormat(
            Hardware->sourceSurface.format, srcFormat
            ));

        gcmERR_BREAK(gcoSURF_QueryFormat(
            gcvSURF_A8R8G8B8, intFormat
            ));

        gcmERR_BREAK(gcoSURF_QueryFormat(
            Hardware->targetSurface.format, trgFormat
            ));

        /* Determine the initial source and target left coordinates. */
        initSrcLeft = srcFormat[0]->interleaved
            ? ~1 & Hardware->sourceRect.left
            :      Hardware->sourceRect.left;

        initTrgLeft = trgFormat[0]->interleaved
            ? ~1 & DestRect->left
            :      DestRect->left;

        /* Determine odd start flags. */
        srcOddStart = 1 & Hardware->sourceRect.left;
        trgOddStart = 1 & DestRect->left;

        /* Determine pixel sizes. */
        srcPixelSize = srcFormat[0]->bitsPerPixel / 8;
        trgPixelSize = trgFormat[0]->bitsPerPixel / 8;

        /* Determine horizontal steps. */
        srcStep = srcFormat[0]->interleaved
            ? srcPixelSize * 2
            : srcPixelSize;

        trgStep = trgFormat[0]->interleaved
            ? trgPixelSize * 2
            : trgPixelSize;

        /* Compute the rectangle size. */
        width  = DestRect->right  - DestRect->left;
        height = DestRect->bottom - DestRect->top;

        /* Compute initial position. */
        srcLinePtr
            = ((gctUINT8_PTR) Hardware->sourceSurface.node.logical)
            + Hardware->sourceRect.top * Hardware->sourceSurface.stride
            + initSrcLeft * srcPixelSize;

        trgLinePtr
            = ((gctUINT8_PTR) Hardware->targetSurface.node.logical)
            + DestRect->top * Hardware->targetSurface.stride
            + initTrgLeft * trgPixelSize;

        /* Commpute source pixel mask. */
        gcmERR_BREAK(gcoSURF_ComputeColorMask(srcFormat[0], &srcColorMask));

        /* Determine the transparency color. */
        transparentColor = Hardware->transparencyColor & srcColorMask;

        /* Loop through pixels one at a time. */
        for (y = 0; (y < height) && gcmIS_SUCCESS(status); y++)
        {
            gctUINT8_PTR srcPixelPtr = srcLinePtr;
            gctUINT8_PTR trgPixelPtr = trgLinePtr;

            /* Determine initial even/odd. */
            gctBOOL srcOdd = srcFormat[0]->interleaved && srcOddStart;
            gctBOOL trgOdd = trgFormat[0]->interleaved && trgOddStart;

            for (x = 0; x < width; x++)
            {
                gctBOOL transparent = gcvFALSE;

                gctUINT32 convSrcPixel = 0;
                gctUINT32 resultPixel;
                gctUINT8 rop;

                /* Read the source and destination pixels. */
                if (!srcOdd || srcOddStart)
                {
                    gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                        Hardware,
                        srcPixelPtr, &srcPixel[0], 0, 0,
                        srcFormat[0], srcFormat[0],
                        gcvNULL, gcvNULL
                        ));

                    if (srcFormat[0]->interleaved)
                    {
                        gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                            Hardware,
                            srcPixelPtr, &srcPixel[1], 0, 0,
                            srcFormat[1], srcFormat[1],
                            gcvNULL, gcvNULL
                            ));
                    }
                }

                if (!trgOdd || trgOddStart)
                {
                    gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                        Hardware,
                        trgPixelPtr, &trgPixel[0], 0, 0,
                        trgFormat[0], intFormat[0],
                        gcvNULL, gcvNULL
                        ));

                    if (trgFormat[0]->interleaved)
                    {
                        gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                            Hardware,
                            trgPixelPtr, &trgPixel[1], 0, 0,
                            trgFormat[1], intFormat[0],
                            gcvNULL, gcvNULL
                            ));
                    }
                }

                /* Determine transparency. */
                if (Hardware->srcTransparency == gcv2D_KEYED)
                {
                    transparent = ((srcPixel[srcOdd] & srcColorMask) == transparentColor)
                        ? gcvTRUE
                        : gcvFALSE;
                }

                /* Determine the ROP code to be used. */
                rop = transparent ? (gctUINT8) BgRop : (gctUINT8) FgRop;

                /* Convert the source pixel to the intermediate format. */
                gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                    Hardware,
                    &srcPixel[srcOdd],
                    &convSrcPixel,
                    0, 0,
                    srcFormat[srcOdd],
                    intFormat[0],
                    gcvNULL, gcvNULL
                    ));

                /* Perform ROP. */
                resultPixel = _Rop3(rop, convSrcPixel, 0, trgPixel[trgOdd]);

                /* Write the result back. */
                gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                    Hardware,
                    &resultPixel,
                    trgPixelPtr,
                    0, 0,
                    intFormat[0],
                    trgFormat[trgOdd],
                    gcvNULL, gcvNULL
                    ));

                /* Advance to the next pixel. */
                if (!srcFormat[0]->interleaved || srcOdd)
                    srcPixelPtr += srcStep;

                if (!trgFormat[0]->interleaved || trgOdd)
                    trgPixelPtr += trgStep;

                /* Update the odd flags. */
                srcOdd = (srcOdd + srcFormat[0]->interleaved) & 1;
                trgOdd = (trgOdd + trgFormat[0]->interleaved) & 1;
            }

            /* Advance to the next line. */
            srcLinePtr += Hardware->sourceSurface.stride;
            trgLinePtr += Hardware->targetSurface.stride;
        }

        /* Dump the results. */
        gcmDUMP_BUFFER(Hardware->os,
                       "memory",
                       Hardware->targetSurface.node.physical,
                       Hardware->targetSurface.node.logical,
                       0,
                       Hardware->targetSurface.node.size);
    }
    while (gcvFALSE);

    /* Return status. */
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_Get2DResourceUsage
**
**  Determines the usage of 2D resources (source/pattern/destination).
**
**  INPUT:
**
**      gctUINT8 FgRop
**      gctUINT8 BgRop
**          Foreground and background ROP codes.
**
**      gctUINT32 SrcTransparency
**          Current source transparency mode in hardware encoding.
**
**  OUTPUT:
**
**      gctBOOL_PTR UseSource
**      gctBOOL_PTR UsePattern
**      gctBOOL_PTR UseDestination
**          Resource usage flags to be determined and returned.
**          gcvNULL is allowed for the unwanted flags.
**
*/
void gcoHARDWARE_Get2DResourceUsage(
    IN gctUINT8 FgRop,
    IN gctUINT8 BgRop,
    IN gctUINT32 SrcTransparency,
    OUT gctBOOL_PTR UseSource,
    OUT gctBOOL_PTR UsePattern,
    OUT gctBOOL_PTR UseDestination
    )
{
    gcmHEADER_ARG("FgRop=%x BgRop=%x SrcTransparency=%d "
                    "UseSource=0x%x UsePattern=0x%x UseDestination=0x%x",
                    FgRop, BgRop, SrcTransparency,
                    UseSource, UsePattern, UseDestination);

    /* Determine whether we need the source for the operation. */
    if (UseSource != gcvNULL)
    {
        if (SrcTransparency == gcv2D_KEYED)
        {
            *UseSource = gcvTRUE;
        }
        else
        {
            /* Determine whether this is target only operation. */
            gctBOOL targetOnly
                =  ((FgRop == 0x00) && (BgRop == 0x00))     /* Blackness.    */
                || ((FgRop == 0x55) && (BgRop == 0x55))     /* Invert.       */
                || ((FgRop == 0xAA) && (BgRop == 0xAA))     /* No operation. */
                || ((FgRop == 0xFF) && (BgRop == 0xFF));    /* Whiteness.    */

            *UseSource
                = !targetOnly
                && ((((FgRop >> 2) & 0x33) != (FgRop & 0x33))
                ||  (((BgRop >> 2) & 0x33) != (BgRop & 0x33)));
        }
    }

    /* Determine whether we need the pattern for the operation. */
    if (UsePattern != gcvNULL)
    {
        *UsePattern
            =  (((FgRop >> 4) & 0x0F) != (FgRop & 0x0F))
            || (((BgRop >> 4) & 0x0F) != (BgRop & 0x0F));
    }

    /* Determine whether we need the destiantion for the operation. */
    if (UseDestination != gcvNULL)
    {
        *UseDestination
            =  (((FgRop >> 1) & 0x55) != (FgRop & 0x55))
            || (((BgRop >> 1) & 0x55) != (BgRop & 0x55));
    }

    gcmFOOTER_NO();
}

/*******************************************************************************
**
**  gco2D_GetMaximumDataCount
**
**  Retrieve the maximum number of 32-bit data chunks for a single DE command.
**
**  INPUT:
**
**      Nothing
**
**  OUTPUT:
**
**      gctUINT32
**          Data count value.
*/
gctUINT32 gco2D_GetMaximumDataCount(
    void
    )
{
    gctUINT32 result;

    gcmHEADER();

    result = __gcmMASK(26:16);

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

/*******************************************************************************
**
**  gco2D_GetMaximumRectCount
**
**  Retrieve the maximum number of rectangles, that can be passed in a single DE command.
**
**  INPUT:
**
**      Nothing
**
**  OUTPUT:
**
**      gctUINT32
**          Rectangle count value.
*/
gctUINT32 gco2D_GetMaximumRectCount(
    void
    )
{
    gctUINT32 result;

    gcmHEADER();

    result = __gcmMASK(15:8);

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

/*******************************************************************************
**
**  gco2D_GetPixelAlignment
**
**  Returns the pixel alignment of the surface.
**
**  INPUT:
**
**      gceSURF_FORMAT Format
**          Pixel format.
**
**  OUTPUT:
**
**      gcsPOINT_PTR Alignment
**          Pointer to the pixel alignment values.
*/
gceSTATUS gco2D_GetPixelAlignment(
    gceSURF_FORMAT Format,
    gcsPOINT_PTR Alignment
    )
{
    gceSTATUS status;
    const gctUINT32 BITS_PER_CACHELINE = 64 * 8;

    gcmHEADER_ARG("Format=%d Alignment=0x%x", Format, Alignment);

    /* Verify the argument. */
    gcmVERIFY_ARGUMENT(Alignment != gcvNULL);

    do
    {
        /* Get format's specifics. */
        gcsSURF_FORMAT_INFO_PTR format[2];
        gcmERR_BREAK(gcoSURF_QueryFormat(Format, format));

        /* Determine horizontal alignment. */
        Alignment->x = BITS_PER_CACHELINE / format[0]->bitsPerPixel;

        /* Vertical alignment for GC600 is simple. */
        Alignment->y = 1;
    }
    while (gcvFALSE);

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_Set2DClearColor
**
**  Set 2D clear color.
**  For PE 2.0, the color is specified in A8R8G8B8 format.
**  For older PE, the color is specified in destination format.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT32 Color
**          32-bit clear color value.
**
**      gctBOOL ColorConvert
**          If set to gcvTRUE, the 32-bit values in the table are assumed to be
**          in ARGB8 format and will be converted by the hardware to the
**          destination format as needed.
**          If set to gcvFALSE, the 32-bit values in the table are assumed to be
**          preconverted to the destination format.
**
**  OUTPUT:
**
**      gceSTATUS
**          Returns gcvSTATUS_OK if successful.
*/
gceSTATUS gcoHARDWARE_Set2DClearColor(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Color,
    IN gctBOOL ColorConvert
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Color=%x ColorConvert=%d",
                    Hardware, Color, ColorConvert);

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

            if (Hardware->hw2DPE20)
            {
                /* LoadState(AQDE_CLEAR_PIXEL_VALUE_LOW, 1), LoColor. */
                gcmERR_BREAK(gcoHARDWARE_LoadState32(
                    Hardware,
                    0x012C0,
                    Color
                    ));
            }
            else
            {
                if (ColorConvert)
                {
                    /* Convert color in to destination format. */
                    gcmERR_BREAK(gcoHARDWARE_ColorConvertFromARGB8(
                        Hardware->targetSurface.format,
                        1,
                        &Color,
                        &Color
                        ));
                }

                /* LoadState(AQDE_CLEAR_BYTE_MASK, 1), 0xFF. */
                gcmERR_BREAK(gcoHARDWARE_LoadState32(
                    Hardware,
                    0x01268,
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0)))

                    ));

                /* LoadState(AQDE_CLEAR_PIXEL_VALUE_LOW, 1), LoColor. */
                gcmERR_BREAK(gcoHARDWARE_LoadState32(
                    Hardware,
                    0x01270,
                    Color
                    ));

                /* LoadState(AQDE_CLEAR_PIXEL_VALUE_HIGH, 1), HiColor. */
                gcmERR_BREAK(gcoHARDWARE_LoadState32(
                    Hardware,
                    0x01274,
                    Color
                    ));
            }
        }
        else
        {
            /* 2D clear is not currently supported by the software renderer. */
            gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
        }
    }
    while (gcvFALSE);

    /* Return result. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_SetBitBlitMirror
**
**  Enable/disable 2D BitBlt mirrorring.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctBOOL HorizontalMirror
**          Horizontal mirror enable flag.
**
**      gctBOOL VerticalMirror
**          Vertical mirror enable flag.
**
**  OUTPUT:
**
**      gceSTATUS
**          Returns gcvSTATUS_OK if successful.
*/
gceSTATUS gcoHARDWARE_SetBitBlitMirror(
    IN gcoHARDWARE Hardware,
    IN gctBOOL HorizontalMirror,
    IN gctBOOL VerticalMirror
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x HorizontalMirror=%d VerticalMirror=%d",
                    Hardware, HorizontalMirror, VerticalMirror);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    do
    {
        if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
        {
            gctUINT32 mirror;
            gctUINT32 enable;
            gctUINT32 config;

            /* Determine the mirror value. */
            if (HorizontalMirror)
            {
                if (VerticalMirror)
                {
                    mirror = 0x3;
                }
                else
                {
                    mirror = 0x1;
                }
            }
            else
            {
                if (VerticalMirror)
                {
                    mirror = 0x2;
                }
                else
                {
                    mirror = 0x0;
                }
            }

            /* Determine mirror enable flag. */
            enable = (mirror == 0x0)
                ? 0x0
                : 0x1;

            /* SelectPipe(2D). */
            gcmERR_BREAK(gcoHARDWARE_SelectPipe(
                Hardware,
                0x1
                ));

            /* Configure the mirror. */
            config
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) | (((gctUINT32) ((gctUINT32) (enable) & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4))) | (((gctUINT32) ((gctUINT32) (mirror) & ((gctUINT32) ((((1 ? 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));

            /* Set mirror configuration. */
            gcmERR_BREAK(gcoHARDWARE_LoadState32(
                Hardware,
                0x0126C,
                config
                ));
        }
        else
        {
            /* 2D clear is not currently supported by the software renderer. */
            gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
        }
    }
    while (gcvFALSE);

    /* Return result. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_StartDE
**
**  Start a DE command for one or more source and destination rectangles.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gce2D_COMMAND Command
**          2D engine command to be executed.
**
**      gctUINT32 SrcRectCount
**          Set as 1, for single source rectangle.
**          Set as DestRectCount, if each blit has its own source rectangle.
**
**      gcsRECT_PTR SrcRect
**          Pointer to an array of source rectangles.
**
**      gctUINT32 DestRectCount
**          Number of destination rectangles to be operated on.
**
**      gcsRECT_PTR DestRect
**          Pointer to an array of destination rectangles.
**
**      gctUINT32 FgRop
**      gctUINT32 BgRop
**          Foreground and background ROP codes.
**
**  OUTPUT:
**
**      gceSTATUS
**          Returns gcvSTATUS_OK if successful.
*/
gceSTATUS gcoHARDWARE_StartDE(
    IN gcoHARDWARE Hardware,
    IN gce2D_COMMAND Command,
    IN gctUINT32 SrcRectCount,
    IN gcsRECT_PTR SrcRect,
    IN gctUINT32 DestRectCount,
    IN gcsRECT_PTR DestRect,
    IN gctUINT32 FgRop,
    IN gctUINT32 BgRop
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Command=0x%x SrcRectCount=%d "
                    "SrcRect=0x%x DestRectCount=%d DestRect=0x%x "
                    "FgRop=%x BgRop=%x",
                    Hardware, Command, SrcRectCount,
                    SrcRect, DestRectCount, DestRect,
                    FgRop, BgRop);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmASSERT((SrcRectCount == 1) || (SrcRectCount == DestRectCount));
    gcmASSERT(DestRectCount > 0);
    gcmASSERT(DestRect);

    do
    {
        if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
        {
            gctUINT32 i, destConfig;
            gctUINT32_PTR memory;
            gctUINT32 command, format, swizzle, isYUVformat, endian;

            /* Convert the command. */
            gcmERR_BREAK(gcoHARDWARE_TranslateCommand(
                Command, &command
                ));

            /* Convert the format. */
            gcmERR_BREAK(gcoHARDWARE_TranslateDestinationFormat(
                Hardware, Hardware->targetSurface.format, &format, &swizzle, &isYUVformat
                ));

            /* Set endian control */
            endian = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));

            if (Hardware->bigEndian)
            {
                gctUINT32 bpp;

                /* Compute bits per pixel. */
                gcmERR_BREAK(gcoHARDWARE_ConvertFormat(Hardware,
                                                       Hardware->targetSurface.format,
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

            /*******************************************************************
            ** Select 2D pipe.
            */

            gcmERR_BREAK(gcoHARDWARE_SelectPipe(
                Hardware,
                0x1
                ));

            /*******************************************************************
            ** Chips with byte write capability don't fetch the destiantion
            ** if it's not needed for the current operation. If the primitive(s)
            ** that follow overlap with the previous primitive causing a cache
            ** hit and they happen to use the destination to ROP with, it will
            ** cause corruption since the destination was not fetched in the
            ** first place.
            **
            ** Currently the hardware does not track this kind of case so we
            ** have to flush whenever we see a use of source or destination.
            */

            /* Flush 2D cache if needed. */
            if (Hardware->byteWrite)
            {
                gctBOOL useSource;
                gctBOOL useDest;

                /* Determine the resource usage. */
                gcoHARDWARE_Get2DResourceUsage(
                    (gctUINT8) FgRop, (gctUINT8) BgRop,
                    Hardware->srcTransparency,
                    &useSource, gcvNULL, &useDest
                    );

                if (useSource || useDest)
                {
                    gcmERR_BREAK(gcoHARDWARE_FlushPipe(Hardware));
                }
            }

            /*******************************************************************
            ** Pattern table needs to be in destination format for old PE,
            ** and in ARGB8 format for PE 2.0. Convert the saved pattern table
            ** to respective format and program it.
            */

            /* Program pattern table if needed. */
            if (Hardware->patternTableProgram)
            {
                gctBOOL colorConvert;

                if (Hardware->hw2DPE20)
                {
                    colorConvert = gcvTRUE;

                    /* Pattern table is in destination format,
                       convert it into ARGB8 format. */
                    gcmERR_BREAK(gcoHARDWARE_ColorConvertToARGB8(
                        Hardware->targetSurface.format,
                        Hardware->patternTableIndexCount,
                        Hardware->patternTable,
                        Hardware->patternTable
                        ));
                }
                else
                {
                    colorConvert = gcvFALSE;

                    /* Pattern table is in ARGB8 format,
                       convert it into destination format. */
                    gcmERR_BREAK(gcoHARDWARE_ColorConvertFromARGB8(
                        Hardware->targetSurface.format,
                        Hardware->patternTableIndexCount,
                        Hardware->patternTable,
                        Hardware->patternTable
                        ));
                }

                /* Load the palette. */
                gcmERR_BREAK(gcoHARDWARE_LoadPalette(
                    Hardware,
                    Hardware->patternTableFirstIndex,
                    Hardware->patternTableIndexCount,
                    (gctPOINTER)Hardware->patternTable,
                    colorConvert
                    ));

                Hardware->patternTableProgram = gcvFALSE;
            }

            /*******************************************************************
            ** Monochrome foreground and background color can be specified in
            ** destination format. Convert them to ARGB8 format and program them.
            */

            /* Program mono colors if needed. */
            if (Hardware->monoColorProgram)
            {
                gcmERR_BREAK(gcoHARDWARE_ColorConvertToARGB8(
                    Hardware->targetSurface.format,
                    1,
                    &Hardware->fgColor,
                    &Hardware->fgColor
                    ));

                gcmERR_BREAK(gcoHARDWARE_ColorConvertToARGB8(
                    Hardware->targetSurface.format,
                    1,
                    &Hardware->bgColor,
                    &Hardware->bgColor
                    ));

                /* LoadState(AQDE_SRC_COLOR_BG, 1), BgColor. */
                gcmERR_BREAK(gcoHARDWARE_LoadState32(
                    Hardware,
                    0x01218, Hardware->bgColor
                    ));

                /* LoadState(AQDE_SRC_COLOR_FG, 1), FgColor. */
                gcmERR_BREAK(gcoHARDWARE_LoadState32(
                    Hardware,
                    0x0121C, Hardware->fgColor
                    ));

                Hardware->monoColorProgram = gcvFALSE;
            }

            /*******************************************************************
            ** Transparency color can be specified in 32bits using gco2D_SetColorKey.
            ** For old PE, the color has to be packed into the source format.
            */

            /* Program transparency color if needed. */
            if (Hardware->transparencyColorProgram)
            {
                gcmERR_BREAK(gcoHARDWARE_ColorPackFromARGB8(
                    Hardware->sourceSurface.format,
                    Hardware->transparencyColor,
                    &Hardware->transparencyColor
                    ));

                /* LoadState(AQDE_SRC_COLOR_BG, 1), BgColor. */
                gcmERR_BREAK(gcoHARDWARE_LoadState32(
                    Hardware,
                    0x01218, Hardware->transparencyColor
                    ));

                Hardware->transparencyColorProgram = gcvFALSE;
            }

            /*******************************************************************
            ** Set the destination configuration register.
            */

            if (Hardware->bigEndian)
            {
                /* Flush the current pipe. */
                gcmERR_BREAK(gcoHARDWARE_FlushPipe(Hardware));
            }

            destConfig =
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12))) | (((gctUINT32) ((gctUINT32) (command) & ((gctUINT32) ((((1 ? 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12)))

                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ? 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16)))

                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))

                | endian;

            gcmERR_BREAK(gcoHARDWARE_LoadState32(
                Hardware,
                0x01234,
                destConfig
                ));


            /*******************************************************************
            ** Setup ROP.
            */
            gcmERR_BREAK(gcoHARDWARE_LoadState32(
                Hardware,
                0x0125C,
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8))) | (((gctUINT32) ((gctUINT32) (BgRop) & ((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8)))

                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0))) | (((gctUINT32) ((gctUINT32) (FgRop) & ((gctUINT32) ((((1 ? 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0)))

                ));


            /*******************************************************************
            ** Allocate and configure StartDE command buffer.
            ** Subdivide into multiple commands if RectCount exceeds maxRectCount.
            */

            if (SrcRect == gcvNULL)
            {
                gctUINT32 maxRectCount = gco2D_GetMaximumRectCount();
                gctUINT32 roundNum = (DestRectCount + maxRectCount - 1) / maxRectCount;

                gcmERR_BREAK(
                    gcoBUFFER_Reserve(
                        Hardware->buffer,
                        (roundNum * 2 + DestRectCount * 2) * sizeof(gctUINT32),
                        gcvTRUE,
                        gcvNULL,
                        (gctPOINTER *) &memory));

                do
                {
                    /* Render upto maxRectCount rectangles. */
                    gctUINT32 destRectCount = (DestRectCount < maxRectCount) ? DestRectCount : maxRectCount;

                    /* StartDE(RectCount). */
                    *memory++
                        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x04 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8))) | (((gctUINT32) ((gctUINT32) (destRectCount) & ((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8)));
                    *memory++
                        = 0;

                    gcmDUMP(Hardware->os,
                            "@[draw2d %u 0x00000000",
                            destRectCount);

                    /* Append the rectangles. */
                    for (i = 0; i < destRectCount; i++)
                    {
                        *memory++
                            = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (DestRect[i].left) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

                            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (DestRect[i].top) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));
                        *memory++
                            = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (DestRect[i].right) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

                            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (DestRect[i].bottom) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

                        gcmDUMP(Hardware->os,
                                "  %d,%d %d,%d",
                                DestRect[i].left, DestRect[i].top,
                                DestRect[i].right, DestRect[i].bottom);
                    }

                    gcmDUMP(Hardware->os, "] -- draw2d");

                    DestRectCount -= destRectCount;
                    DestRect += destRectCount;
                }
                while (DestRectCount);
            }
            else
            {
                /* Force to draw one rectange at a time. */

                /* Reserve space for all the draw commands at one go. */
                gcmERR_BREAK(gcoBUFFER_Reserve(
                    Hardware->buffer,
                    8 * DestRectCount * sizeof(gctUINT32),
                    gcvTRUE,
                    gcvNULL,
                    (gctPOINTER *) &memory
                    ));

                do
                {
                    /* Program source rectangle for each destination rectangle. */

                    /* 0x01210 */
                    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (0x01210>>2) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

                    /* 0x01210 */
                    *memory++
                        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (SrcRect->left) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

                        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (SrcRect->top) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

                    gcmDUMP(Hardware->os,
                            "@[state 0x%04X 0x%08X]",
                            0x0484, memory[-1]);

                    /* 0x01214 */
                    *memory++
                        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (SrcRect->right-SrcRect->left) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

                        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (SrcRect->bottom-SrcRect->top) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

                    gcmDUMP(Hardware->os,
                            "@[state 0x%04X 0x%08X]",
                            0x0485, memory[-1]);

                    memory++;

                    /* StartDE(RectCount). */
                    *memory++
                        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x04 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8)));
                    *memory++
                        = 0;

                    /* Append the rectangle. */
                    *memory++
                        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (DestRect->left) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

                        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (DestRect->top) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));
                    *memory++
                        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (DestRect->right) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

                        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (DestRect->bottom) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

                    gcmDUMP(Hardware->os, "@[draw2d 1 0x00000000");
                    gcmDUMP(Hardware->os,
                            "  %d,%d %d,%d",
                            DestRect->left, DestRect->top,
                            DestRect->right, DestRect->bottom);
                    gcmDUMP(Hardware->os, "] -- draw2d");

                    /* Move to next SrcRect and DestRect. */
                    SrcRect++;
                    DestRect++;
                }
                while (--DestRectCount);
            }

            /*******************************************************************
            ** Program a dummy state load at addr 0xFFFF.
            */

            gcmERR_BREAK(gcoHARDWARE_LoadState32(
                    Hardware,
                    0x00004,
                    0x0
                    ));
        }
        else
        {
            /* Program the destination format. */
            gcmERR_BREAK(gcoHARDWARE_SetTargetFormat(
                Hardware,
                Hardware->targetSurface.format
                ));

            /* Call software renderer. */
            gcmERR_BREAK(_RenderRectangle(
                Hardware,
                Command,
                DestRectCount,
                DestRect,
                FgRop,
                BgRop
                ));
        }
    }
    while (gcvFALSE);

    /* Return result. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_StartDELine
**
**  Start a DE command to draw one or more Lines, with a common or
**  individual color.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gce2D_COMMAND Command
**          2D engine command to be executed.
**
**      gctUINT32 RectCount
**          Number of destination rectangles to be operated on.
**
**      gcsRECT_PTR DestRect
**          Pointer to an array of destination rectangles.
**
**      gctUINT32 ColorCount
**          Set as 0, if using brush.
**          Set as 1, if single color for all lines.
**          Set as LineCount, if each line has its own color.
**
**      gctUINT32_PTR Color32
**          Source color array in A8R8G8B8 format.
**
**      gctUINT32 FgRop
**      gctUINT32 BgRop
**          Foreground and background ROP codes.
**
**  OUTPUT:
**
**      gceSTATUS
**          Returns gcvSTATUS_OK if successful.
*/
gceSTATUS gcoHARDWARE_StartDELine(
    IN gcoHARDWARE Hardware,
    IN gce2D_COMMAND Command,
    IN gctUINT32 LineCount,
    IN gcsRECT_PTR DestRect,
    IN gctUINT32 ColorCount,
    IN gctUINT32_PTR Color32,
    IN gctUINT32 FgRop,
    IN gctUINT32 BgRop
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Command=0x%x LineCount=%d "
                    "DestRect=0x%x ColorCount=%d Color32=%x "
                    "FgRop=%x BgRop=%x",
                    Hardware, Command, LineCount,
                    DestRect, ColorCount, Color32,
                    FgRop, BgRop);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmASSERT((ColorCount == 1) || (ColorCount == 0) || (ColorCount == LineCount));
    gcmASSERT(LineCount);

    do
    {
        if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
        {
            gctUINT32 i, destConfig, maxLineCount;
            gctUINT32 colorConfig[2], lastProgrammedColor = 0;
            gctUINT32_PTR memory;
            gctUINT32 command, format, swizzle, isYUVformat, endian;

            /* Convert the command. */
            gcmERR_BREAK(gcoHARDWARE_TranslateCommand(
                Command, &command
                ));

            /* Convert the format. */
            gcmERR_BREAK(gcoHARDWARE_TranslateDestinationFormat(
                Hardware, Hardware->targetSurface.format, &format, &swizzle, &isYUVformat
                ));

            /* Set endian control */
            endian = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));

            if (Hardware->bigEndian)
            {
                gctUINT32 bpp;

                /* Compute bits per pixel. */
                gcmERR_BREAK(gcoHARDWARE_ConvertFormat(Hardware,
                                                       Hardware->targetSurface.format,
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

            /*******************************************************************
            ** Select 2D pipe.
            */

            gcmERR_BREAK(gcoHARDWARE_SelectPipe(
                Hardware,
                0x1
                ));

            /*******************************************************************
            ** Chips with byte write capability don't fetch the destiantion
            ** if it's not needed for the current operation. If the primitive(s)
            ** that follow overlap with the previous primitive causing a cache
            ** hit and they happen to use the destination to ROP with, it will
            ** cause corruption since the destination was not fetched in the
            ** first place.
            **
            ** Currently the hardware does not track this kind of case so we
            ** have to flush whenever we see a use of source or destination.
            */

            /* Flush 2D cache if needed. */
            if (Hardware->byteWrite)
            {
                gctBOOL useSource;
                gctBOOL useDest;

                /* Determine the resource usage. */
                gcoHARDWARE_Get2DResourceUsage(
                    (gctUINT8) FgRop, (gctUINT8) BgRop,
                    Hardware->srcTransparency,
                    &useSource, gcvNULL, &useDest
                    );

                if (useSource || useDest)
                {
                    gcmERR_BREAK(gcoHARDWARE_FlushPipe(Hardware));
                }
            }

            /*******************************************************************
            ** Pattern table needs to be in destination format for old PE,
            ** and in ARGB8 format for PE 2.0. Convert the saved pattern table
            ** to respective format and program it.
            */

            /* Program pattern table if needed. */
            if (Hardware->patternTableProgram)
            {
                gctBOOL colorConvert;

                if (Hardware->hw2DPE20)
                {
                    colorConvert = gcvTRUE;

                    /* Pattern table is in destination format,
                       convert it into ARGB8 format. */
                    gcmERR_BREAK(gcoHARDWARE_ColorConvertToARGB8(
                        Hardware->targetSurface.format,
                        Hardware->patternTableIndexCount,
                        Hardware->patternTable,
                        Hardware->patternTable
                        ));
                }
                else
                {
                    colorConvert = gcvFALSE;

                    /* Pattern table is in ARGB8 format,
                       convert it into destination format. */
                    gcmERR_BREAK(gcoHARDWARE_ColorConvertFromARGB8(
                        Hardware->targetSurface.format,
                        Hardware->patternTableIndexCount,
                        Hardware->patternTable,
                        Hardware->patternTable
                        ));
                }

                /* Load the palette. */
                gcmERR_BREAK(gcoHARDWARE_LoadPalette(
                    Hardware,
                    Hardware->patternTableFirstIndex,
                    Hardware->patternTableIndexCount,
                    (gctPOINTER)Hardware->patternTable,
                    colorConvert
                    ));

                Hardware->patternTableProgram = gcvFALSE;
            }

            /*******************************************************************
            ** Monochrome foreground and background color can be specified in
            ** destination format. Convert them to ARGB8 format and program
            ** them.
            */

            /* Program mono colors if needed. */
            if (Hardware->monoColorProgram)
            {
                gcmERR_BREAK(gcoHARDWARE_ColorConvertToARGB8(
                    Hardware->targetSurface.format,
                    1,
                    &Hardware->fgColor,
                    &Hardware->fgColor
                    ));

                gcmERR_BREAK(gcoHARDWARE_ColorConvertToARGB8(
                    Hardware->targetSurface.format,
                    1,
                    &Hardware->bgColor,
                    &Hardware->bgColor
                    ));

                /* LoadState(AQDE_SRC_COLOR_BG, 1), BgColor. */
                gcmERR_BREAK(gcoHARDWARE_LoadState32(
                    Hardware,
                    0x01218, Hardware->bgColor
                    ));

                /* LoadState(AQDE_SRC_COLOR_FG, 1), FgColor. */
                gcmERR_BREAK(gcoHARDWARE_LoadState32(
                    Hardware,
                    0x0121C, Hardware->fgColor
                    ));

                Hardware->monoColorProgram = gcvFALSE;
            }

            /*******************************************************************
            ** Transparency color can be specified in 32bits using
            ** gco2D_SetColorKey.  For old PE, the color has to be packed into
            ** the source format.
            */

            /* Program transparency color if needed. */
            if (Hardware->transparencyColorProgram)
            {
                gcmERR_BREAK(gcoHARDWARE_ColorPackFromARGB8(
                    Hardware->sourceSurface.format,
                    Hardware->transparencyColor,
                    &Hardware->transparencyColor
                    ));

                /* LoadState(AQDE_SRC_COLOR_BG, 1), BgColor. */
                gcmERR_BREAK(gcoHARDWARE_LoadState32(
                    Hardware,
                    0x01218, Hardware->transparencyColor
                    ));

                Hardware->transparencyColorProgram = gcvFALSE;
            }

            /*******************************************************************
            ** Set the destination configuration register.
            */

            if (Hardware->bigEndian)
            {
                /* Flush the current pipe. */
                gcmERR_BREAK(gcoHARDWARE_FlushPipe(Hardware));
            }

            destConfig =
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12))) | (((gctUINT32) ((gctUINT32) (command) & ((gctUINT32) ((((1 ? 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12)))

                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ? 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16)))

                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))

                | endian;

            gcmERR_BREAK(gcoHARDWARE_LoadState32(
                Hardware,
                0x01234,
                destConfig
                ));


            /*******************************************************************
            ** Setup ROP.
            */

            gcmERR_BREAK(gcoHARDWARE_LoadState32(
                Hardware,
                0x0125C,
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8))) | (((gctUINT32) ((gctUINT32) (BgRop) & ((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8)))

                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0))) | (((gctUINT32) ((gctUINT32) (FgRop) & ((gctUINT32) ((((1 ? 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0)))

                ));


            /*******************************************************************
            ** Allocate and configure StartDE command buffer.  Subdivide into
            ** multiple commands if LineCount exceeds maxLineCount.
            */

            maxLineCount = gco2D_GetMaximumRectCount();

            if (ColorCount)
            {
                /* Set last programmed color different from *Color32,
                   so that the first color always gets programmed. */
                lastProgrammedColor = *Color32 + 1;
            }

            do
            {
                /* Render upto maxRectCount rectangles. */
                gctUINT32 lineCount = (LineCount < maxLineCount)
                                    ? LineCount
                                    : maxLineCount;

                /* Program color for each line. */
                if (ColorCount && (lastProgrammedColor != *Color32))
                {
                    /* Backgroud color. */
                    colorConfig[0] = *Color32;

                    /* Foreground color. */
                    colorConfig[1] = *Color32;

                    /* Save last programmed color. */
                    lastProgrammedColor = *Color32;

                    /* LoadState(AQDE_SRC_COLOR_BG, 2), BgColor, FgColor. */
                    gcmERR_BREAK(gcoHARDWARE_LoadState(
                        Hardware,
                        0x01218, 2,
                        colorConfig
                        ));
                }

                /* Find the number of following lines with same color. */
                if (ColorCount > 1)
                {
                    gctUINT32 sameColoredLines = 1;

                    Color32++;

                    while (sameColoredLines < lineCount)
                    {
                        if (lastProgrammedColor != *Color32)
                        {
                            break;
                        }

                        Color32++;
                        sameColoredLines++;
                    }

                    lineCount = sameColoredLines;
                }

                gcmERR_BREAK(gcoBUFFER_Reserve(
                    Hardware->buffer,
                    (2 + lineCount * 2) * sizeof(gctUINT32),
                    gcvTRUE,
                    gcvNULL,
                    (gctPOINTER *) &memory
                    ));

                /* StartDE(RectCount). */
                *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x04 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8))) | (((gctUINT32) ((gctUINT32) (lineCount) & ((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8)));
                *memory++
                    = 0;

                gcmDUMP(Hardware->os, "@[draw2d %u 0x00000000", lineCount);

                /* Append the rectangles. */
                for (i = 0; i < lineCount; i++)
                {
                    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (DestRect[i].left) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (DestRect[i].top) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));
                    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (DestRect[i].right) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (DestRect[i].bottom) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

                    gcmDUMP(Hardware->os,
                            "  %d,%d %d,%d",
                            DestRect[i].left, DestRect[i].top,
                            DestRect[i].right, DestRect[i].bottom);
                }

                gcmDUMP(Hardware->os, "] -- draw2d");

                LineCount -= lineCount;
                DestRect += lineCount;
            }
            while (LineCount);

            /*******************************************************************
            ** Program a dummy state load at addr 0xFFFF.
            */

            gcmERR_BREAK(gcoHARDWARE_LoadState32(
                    Hardware,
                    0x00004,
                    0x0
                    ));
        }
        else
        {
            /* Program the destination format. */
            gcmERR_BREAK(gcoHARDWARE_SetTargetFormat(
                Hardware,
                Hardware->targetSurface.format
                ));

            /* Call software renderer. */
            gcmERR_BREAK(_RenderRectangle(
                Hardware,
                Command,
                LineCount,
                DestRect,
                FgRop,
                BgRop
                ));
        }
    }
    while (gcvFALSE);

    /* Return result. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_StartDEStream
**
**  Start a DE command with a monochrome stream source.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcsRECT_PTR DestRect
**          Pointer to the destination rectangles.
**
**      gctUINT32 FgRop
**      gctUINT32 BgRop
**          Foreground and background ROP codes.
**
**      gctUINT32 StreamSize
**          Size of the stream in bytes.
**
**  OUTPUT:
**
**      gctPOINTER * StreamBits
**          Pointer to an allocated buffer for monochrome data.
*/
gceSTATUS gcoHARDWARE_StartDEStream(
    IN gcoHARDWARE Hardware,
    IN gcsRECT_PTR DestRect,
    IN gctUINT32 FgRop,
    IN gctUINT32 BgRop,
    IN gctUINT32 StreamSize,
    OUT gctPOINTER * StreamBits
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x DestRect=0x%x FgRop=%x "
                    "BgRop=%x StreamSize=%d StreamBits=0x%x",
                    Hardware, DestRect, FgRop,
                    BgRop, StreamSize, StreamBits);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmVERIFY_ARGUMENT(DestRect != gcvNULL);
    gcmVERIFY_ARGUMENT(StreamBits != gcvNULL);

    do
    {
        gctUINT32 format, swizzle, isYUVformat, endian;

        /* Convert the format. */
        gcmERR_BREAK(gcoHARDWARE_TranslateDestinationFormat(
            Hardware, Hardware->targetSurface.format, &format, &swizzle, &isYUVformat
            ));

        /* Set endian control */
        endian = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));

        if (Hardware->bigEndian)
        {
            gctUINT32 bpp;

            /* Compute bits per pixel. */
            gcmERR_BREAK(gcoHARDWARE_ConvertFormat(Hardware,
                                                   Hardware->targetSurface.format,
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

        if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
        {
            gctUINT32 dataCount, destConfig;
            gctUINT32_PTR memory;

            /*******************************************************************
            ** Select 2D pipe.
            */

            gcmERR_BREAK(gcoHARDWARE_SelectPipe(
                Hardware,
                0x1
                ));

            /*******************************************************************
            ** Chips with byte write capability don't fetch the destiantion
            ** if it's not needed for the current operation. If the primitive(s)
            ** that follow overlap with the previous primitive causing a cache
            ** hit and they happen to use the destination to ROP with, it will
            ** cause corruption since the destination was not fetched in the
            ** first place.
            **
            ** Currently the hardware does not track this kind of case so we
            ** have to flush whenever we see a use of source or destination.
            */

            /* Flush 2D cache if needed. */
            if (Hardware->byteWrite)
            {
                gctBOOL useDest;

                /* Determine the resource usage. */
                gcoHARDWARE_Get2DResourceUsage(
                    (gctUINT8) FgRop, (gctUINT8) BgRop,
                    Hardware->srcTransparency,
                    gcvNULL, gcvNULL, &useDest
                    );

                if (useDest)
                {
                    gcmERR_BREAK(gcoHARDWARE_FlushPipe(Hardware));
                }
            }

            /*******************************************************************
            ** Monochrome foreground and background color can be specified in
            ** destination format. Convert them to ARGB8 format and program them.
            */

            /* Program mono colors if needed. */
            if (Hardware->monoColorProgram)
            {
                gcmERR_BREAK(gcoHARDWARE_ColorConvertToARGB8(
                    Hardware->targetSurface.format,
                    1,
                    &Hardware->fgColor,
                    &Hardware->fgColor
                    ));

                gcmERR_BREAK(gcoHARDWARE_ColorConvertToARGB8(
                    Hardware->targetSurface.format,
                    1,
                    &Hardware->bgColor,
                    &Hardware->bgColor
                    ));

                /* LoadState(AQDE_SRC_COLOR_BG, 1), BgColor. */
                gcmERR_BREAK(gcoHARDWARE_LoadState32(
                    Hardware,
                    0x01218, Hardware->bgColor
                    ));

                /* LoadState(AQDE_SRC_COLOR_FG, 1), FgColor. */
                gcmERR_BREAK(gcoHARDWARE_LoadState32(
                    Hardware,
                    0x0121C, Hardware->fgColor
                    ));

                Hardware->monoColorProgram = gcvFALSE;
            }

            /*******************************************************************
            ** Set the destination configuration register.
            */
            if (Hardware->bigEndian)
            {
                /* Flush the current pipe. */
                gcmERR_BREAK(gcoHARDWARE_FlushPipe(Hardware));
            }

            destConfig =
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16))) | (((gctUINT32) ((gctUINT32) (swizzle) & ((gctUINT32) ((((1 ? 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16)))

                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))

                | endian;

            gcmERR_BREAK(gcoHARDWARE_LoadState32(
                Hardware,
                0x01234,
                destConfig
                ));


            /*******************************************************************
            ** Setup ROP.
            */

            gcmERR_BREAK(gcoHARDWARE_LoadState32(
                Hardware,
                0x0125C,
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8))) | (((gctUINT32) ((gctUINT32) (BgRop) & ((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8)))

                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0))) | (((gctUINT32) ((gctUINT32) (FgRop) & ((gctUINT32) ((((1 ? 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0)))

                ));


            /*******************************************************************
            ** Allocate and configure StartDE command buffer.
            */

            gcmERR_BREAK(gcoBUFFER_Reserve(
                Hardware->buffer,
                (2 + 2) * sizeof(gctUINT32) + StreamSize,
                gcvTRUE,
                gcvFALSE,
                (gctPOINTER *) &memory
                ));

            /* Determine the data count. */
            dataCount = StreamSize >> 2;

            /* StartDE(DataCount). */
            *memory++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x04 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8)))

                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 26:16) - (0 ? 26:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:16) - (0 ? 26:16) + 1))))))) << (0 ? 26:16))) | (((gctUINT32) ((gctUINT32) (dataCount) & ((gctUINT32) ((((1 ? 26:16) - (0 ? 26:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:16) - (0 ? 26:16) + 1))))))) << (0 ? 26:16)));
            *memory++
                = 0;

            gcmDUMP(Hardware->os, "@[prim2d 1 0x%08X", dataCount);

            /* Append the rectangle. */
            *memory++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (DestRect->left) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (DestRect->top) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));
            *memory++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (DestRect->right) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (DestRect->bottom) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

            gcmDUMP(Hardware->os, "  %d,%d %d,%d",
                    DestRect->left, DestRect->top,
                    DestRect->right, DestRect->bottom);

            /* Set the stream location. */
            *StreamBits = memory;

            /*******************************************************************
            ** Program a dummy state load at addr 0xFFFF.
            */

            gcmERR_BREAK(gcoHARDWARE_LoadState32(
                    Hardware,
                    0x00004,
                    0x0
                    ));
        }
        else
        {
            /* Monochrome operations are not currently supported. */
            gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
        }
    }
    while (gcvFALSE);

    /* Return status. */
    gcmFOOTER();
    return status;
}


