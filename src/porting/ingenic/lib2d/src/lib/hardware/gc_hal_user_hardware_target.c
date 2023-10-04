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

/******************************************************************************\
****************************** gcoHARDWARE API Code *****************************
\******************************************************************************/

/*******************************************************************************
**
**  gcoHARDWARE_SetClipping
**
**  Set clipping rectangle.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
*       gcsRECT_PTR Rect
**          Pointer to a valid destination rectangle. The valid range of the
**          coordinates is 0..32768.  A pixel is valid if the following is true:
**              (pixelX >= Left) && (pixelX < Right) &&
**              (pixelY >= Top) && (pixelY < Bottom)
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gcoHARDWARE_SetClipping(
    IN gcoHARDWARE Hardware,
    IN gcsRECT_PTR Rect
    )
{
    gceSTATUS status;
    gcsRECT rect;

    gcmHEADER_ARG("Hardware=0x%x Rect=0x%x", Hardware, Rect);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Reset default if rect is not specified. */
    if (Rect == gcvNULL)
    {
        /* Set to the largest rectangle. */
        rect.left   = 0;
        rect.top    = 0;
        rect.right  = 32767;
        rect.bottom = 32767;

        Rect = &rect;
    }

    do
    {
        if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
        {
            gctUINT32 data[2];
            gcsRECT clippedRect;

            /* Clip Rect coordinates in positive range. */
            clippedRect.left = gcmMAX(Rect->left, 0);
            clippedRect.top = gcmMAX(Rect->top, 0);
            clippedRect.right = gcmMAX(Rect->right, 0);
            clippedRect.bottom = gcmMAX(Rect->bottom, 0);

            /* SelectPipe(2D). */
            gcmERR_BREAK(gcoHARDWARE_SelectPipe(
                Hardware,
                0x1
                ));

            /* 0x01260 */
            data[0]
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ? 14:0))) | (((gctUINT32) ((gctUINT32) (clippedRect.left) & ((gctUINT32) ((((1 ? 14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ? 14:0)))

                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 30:16) - (0 ? 30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ? 30:16))) | (((gctUINT32) ((gctUINT32) (clippedRect.top) & ((gctUINT32) ((((1 ? 30:16) - (0 ? 30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ? 30:16)));

            /* 0x01264 */
            data[1]
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ? 14:0))) | (((gctUINT32) ((gctUINT32) (clippedRect.right) & ((gctUINT32) ((((1 ? 14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ? 14:0)))

                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 30:16) - (0 ? 30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ? 30:16))) | (((gctUINT32) ((gctUINT32) (clippedRect.bottom) & ((gctUINT32) ((((1 ? 30:16) - (0 ? 30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ? 30:16)));

            /* Load cllipping states. */
            gcmERR_BREAK(gcoHARDWARE_LoadState(
                Hardware, 0x01260, 2,
                data
                ));
        }
        else
        {
            /* Store clippig rect for software renderer. */
            Hardware->clippingRect = *Rect;

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
**  gcoHARDWARE_SetTarget
**
**  Configure destination.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcsSURF_INFO_PTR Surface
**          Pointer to the destination surface descriptor.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gcoHARDWARE_SetTarget(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_INFO_PTR Surface
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Surface=0x%x", Hardware, Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    do
    {
        if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
        {
            gctUINT32 data[3];
            gctUINT32 rotated = 0;

            /* Store states for programming the target.
               Used later when the command to process is executed.
            */
            Hardware->targetSurface = *Surface;

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

            /* SelectPipe(2D). */
            gcmERR_BREAK(gcoHARDWARE_SelectPipe(
                Hardware,
                0x1
                ));

            /* 0x01228 */
            data[0]
                = Surface->node.physical;

            /* 0x0122C */
            data[1]
                = Surface->stride;

            /* 0x01230 */
            data[2]
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (Surface->alignedWidth) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))) | (((gctUINT32) ((gctUINT32) (rotated) & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));

            /* LoadState(AQDE_DEST_ADDRESS, 3), Address, Stride, rotation. */
            gcmERR_BREAK(gcoHARDWARE_LoadState(
                Hardware,
                0x01228, 3,
                data
                ));

            if (Hardware->fullBitBlitRotation)
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
                    status = gcvSTATUS_NOT_SUPPORTED;
                    break;
                }

                /* Check errors. */
                gcmERR_BREAK(status);

                /* Flush the 2D pipe before writing to the rotation register. */
                gcmERR_BREAK(
                    gcoHARDWARE_FlushPipe(Hardware));

                /* Load target height. */
                gcmERR_BREAK(gcoHARDWARE_LoadState32(
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

                gcmERR_BREAK(gcoHARDWARE_LoadState32(
                            Hardware,
                            0x012BC,
                            value
                            ));
            }
        }
        else
        {
            gceSURF_FORMAT savedFormat;

            /* Rotation is not currently supported by software renderer. */
            if (Surface->rotation != gcvSURF_0_DEGREE)
            {
                gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
            }

            /* Format is set through gcoHARDWARE_SetTargetFormat function. */
            savedFormat = Hardware->targetSurface.format;

            /* Store states for software renderer. */
            Hardware->targetSurface = *Surface;

            /* Restore the format. */
            Hardware->targetSurface.format = savedFormat;

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
**  gcoHARDWARE_SetTargetFormat
**
**  Set the format for the destination surface.
**  For PE 2.0, this mode is not needed, format should be set via SetTarget call.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gceSURF_FORMAT Format
**          Destination surface format.
**
**  OUTPUT:
**
**      gceSTATUS
**          Returns gcvSTATUS_OK if successful.
*/
gceSTATUS
gcoHARDWARE_SetTargetFormat(
    IN gcoHARDWARE Hardware,
    IN gceSURF_FORMAT Format
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Format=%d", Hardware, Format);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    do
    {
        if (Hardware->hw2DEngine && !Hardware->sw2DEngine)
        {
            gctUINT32 format, swizzle, isYUV, endian;

            /* Convert the format. */
            gcmERR_BREAK(gcoHARDWARE_TranslateDestinationFormat(
                Hardware, Format, &format, &swizzle, &isYUV
                ));

            /* Set endian control */
            endian = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));

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
                    endian = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                }
                else if (bpp == 32)
                {
                    endian = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)));
                }
            }

            /* SelectPipe(2D). */
            gcmERR_BREAK(gcoHARDWARE_SelectPipe(
                Hardware,
                0x1
                ));

            /* LoadState(AQDE_DEST_CONFIG, 1), config. */
            gcmERR_BREAK(gcoHARDWARE_LoadState32(
                Hardware,
                0x01234,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))|endian
                ));
        }
        else
        {
            /* Success. */
            status = gcvSTATUS_OK;
        }

        /* Store the format. */
        Hardware->targetSurface.format = Format;
    }
    while (gcvFALSE);

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_SetTargetColorKeyRange
**
**  Setup the destination color key value in A8R8G8B8 format.
**  Write to Destination only if the RGB color channels match to the specified color.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT32 Color
**          Destination color.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gcoHARDWARE_SetTargetColorKeyRange(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 ColorLow,
    IN gctUINT32 ColorHigh
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x ColorLow=%x ColorHigh=%x",
                    Hardware, ColorLow, ColorHigh);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    do
    {
        if (Hardware->hw2DEngine && Hardware->hw2DPE20 && !Hardware->sw2DEngine)
        {
            /* SelectPipe(2D). */
            gcmERR_BREAK(gcoHARDWARE_SelectPipe(
                Hardware,
                0x1
                ));

            /* LoadState global color value. */
            gcmERR_BREAK(gcoHARDWARE_LoadState32(
                Hardware,
                0x012C4,
                ColorLow
                ));

            /* LoadState global color value. */
            gcmERR_BREAK(gcoHARDWARE_LoadState32(
                Hardware,
                0x012E0,
                ColorHigh
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
**  gcoHARDWARE_ColorConvertToARGB8
**
**  Convert Color in target format to A8R8G8B8 color.
**  Only supports destination formats.
**
**  INPUT:
**
**      gceSURF_FORMAT Format
**          Format of the destination surface.
**
**      UINT32 NumColors
**          Number of input color values.
**
**      UINT32_PTR Color
**          Color values in destination format.
**
**  OUTPUT:
**
**      gctUINT32_PTR Color32
**          Color values in ARGB8 format.
*/
gceSTATUS gcoHARDWARE_ColorConvertToARGB8(
    IN gceSURF_FORMAT Format,
    IN gctUINT32 NumColors,
    IN gctUINT32_PTR Color,
    OUT gctUINT32_PTR Color32
    )
{
    gctUINT32 colorR, colorG, colorB, colorA;
    gctUINT32 i;

    gcmHEADER_ARG("Format=%d NumColors=%d Color=0x%x Color32=0x%x",
                    Format, NumColors, Color, Color32);

    for (i = 0; i < NumColors; i++)
    {
        gctUINT32 color = Color[i];
        gctUINT32_PTR color32 = &Color32[i];

        switch(Format)
        {
        case gcvSURF_X8R8G8B8:
        case gcvSURF_A8R8G8B8:
            /* No color conversion needed. */
            *color32 = color;
            continue;

        case gcvSURF_A1R5G5B5:
            /* Extract colors. */
            colorB = (color &   0x1F);
            colorG = (color &  0x3E0) >>  5;
            colorR = (color & 0x7C00) >> 10;
            colorA = (color & 0x8000) >> 15;

            /* Expand colors. */
            colorB = (colorB << 3) | (colorB >> 2);
            colorG = (colorG << 3) | (colorG >> 2);
            colorR = (colorR << 3) | (colorR >> 2);
            colorA = colorA ? 0xFF : 0x00;
            break;

        case gcvSURF_X1R5G5B5:
            /* Extract colors. */
            colorB = (color &   0x1F);
            colorG = (color &  0x3E0) >>  5;
            colorR = (color & 0x7C00) >> 10;

            /* Expand colors. */
            colorB = (colorB << 3) | (colorB >> 2);
            colorG = (colorG << 3) | (colorG >> 2);
            colorR = (colorR << 3) | (colorR >> 2);
            colorA = 0xFF;
            break;

        case gcvSURF_A4R4G4B4:
            /* Extract colors. */
            colorB = (color &    0xF);
            colorG = (color &   0xF0) >>  4;
            colorR = (color &  0xF00) >>  8;
            colorA = (color & 0xF000) >> 12;

            /* Expand colors. */
            colorB = colorB | (colorB << 4);
            colorG = colorG | (colorG << 4);
            colorR = colorR | (colorR << 4);
            colorA = colorA | (colorA << 4);
            break;

        case gcvSURF_X4R4G4B4:
            /* Extract colors. */
            colorB = (color &    0xF);
            colorG = (color &   0xF0) >>  4;
            colorR = (color &  0xF00) >>  8;

            /* Expand colors. */
            colorB = colorB | (colorB << 4);
            colorG = colorG | (colorG << 4);
            colorR = colorR | (colorR << 4);
            colorA = 0xFF;
            break;

        case gcvSURF_R5G6B5:
            /* Extract colors. */
            colorB = (color &   0x1F);
            colorG = (color &  0x7E0) >>  5;
            colorR = (color & 0xF800) >> 11;

            /* Expand colors. */
            colorB = (colorB << 3) | (colorB >> 2);
            colorG = (colorG << 2) | (colorG >> 4);
            colorR = (colorR << 3) | (colorR >> 2);
            colorA = 0xFF;
            break;

        default:
            return gcvSTATUS_NOT_SUPPORTED;
        }

        /* Assemble. */
        *color32 =
           (colorA << 24)
         | (colorR << 16)
         | (colorG <<  8)
         | colorB;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_ColorConvertFromARGB8
**
**  Convert Color in A8R8G8B8 format to target format color.
**  Only supports destination formats.
**
**  INPUT:
**
**      gceSURF_FORMAT Format
**          Format of the destination surface.
**
**      UINT32 NumColors
**          Number of input color values.
**
**      UINT32_PTR Color32
**          Color values in ARGB8 format.
**
**  OUTPUT:
**
**      gctUINT32_PTR Color
**          Color values in destination format.
*/
gceSTATUS gcoHARDWARE_ColorConvertFromARGB8(
    IN gceSURF_FORMAT Format,
    IN gctUINT32 NumColors,
    IN gctUINT32_PTR Color32,
    OUT gctUINT32_PTR Color
    )
{
    gctUINT32 i;

    gcmHEADER_ARG("Format=%d NumColors=%d Color32=0x%x Color=0x%x",
                    Format, NumColors, Color32, Color);

    for (i = 0; i < NumColors; i++)
    {
        gctUINT32 color32 = Color32[i];
        gctUINT32 colorR, colorG, colorB, colorA;
        gctUINT32_PTR color = &Color[i];

        /* Extract colors. */
        colorB = (color32 & 0xFF);
        colorG = (color32 & 0xFF00) >>  8;
        colorR = (color32 & 0xFF0000) >> 16;
        colorA = (color32 & 0xFF000000) >> 24;

        switch(Format)
        {
        case gcvSURF_X8R8G8B8:
        case gcvSURF_A8R8G8B8:
            /* No color conversion needed. */
            *color = color32;
            break;

        case gcvSURF_A1R5G5B5:
        case gcvSURF_X1R5G5B5:
            *color =
                ((colorA >> 7) << 15) |
                ((colorR >> 3) << 10) |
                ((colorG >> 3) <<  5) |
                ((colorB >> 3));
            /* Expand to 32bit. */
            *color = (*color << 16) | *color;
            break;

        case gcvSURF_A4R4G4B4:
        case gcvSURF_X4R4G4B4:
            *color =
                ((colorA >> 4) << 12) |
                ((colorR >> 4) <<  8) |
                ((colorG >> 4) <<  4) |
                ((colorB >> 4));
            /* Expand to 32bit. */
            *color = (*color << 16) | *color;
            break;

        case gcvSURF_R5G6B5:
            *color =
                ((colorR >> 3) << 11) |
                ((colorG >> 2) <<  5) |
                ((colorB >> 3));
            /* Expand to 32bit. */
            *color = (*color << 16) | *color;
            break;

        default:
            return gcvSTATUS_NOT_SUPPORTED;
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_ColorPackFromARGB8
**
**  Pack Color in A8R8G8B8 format to target format color.
**  Needed for backward compatibility.
**  Only supports ARGB source formats existing in old PE.
**
**  INPUT:
**
**      gceSURF_FORMAT Format
**          Format of the source surface.
**
**      UINT32 Color32
**          Color values in ARGB8 format.
**
**  OUTPUT:
**
**      gctUINT32_PTR Color
**          Color values in destination format.
*/
gceSTATUS gcoHARDWARE_ColorPackFromARGB8(
    IN gceSURF_FORMAT Format,
    IN gctUINT32 Color32,
    OUT gctUINT32_PTR Color
    )
{
    gctUINT32 colorR, colorG, colorB, colorA;

    gcmHEADER_ARG("Format=%d Color32=%x Color=0x%x",
                    Format, Color32, Color);

    /* Extract colors. */
    colorB = (Color32 & 0xFF);
    colorG = (Color32 & 0xFF00) >>  8;
    colorR = (Color32 & 0xFF0000) >> 16;
    colorA = (Color32 & 0xFF000000) >> 24;

    switch(Format)
    {
    case gcvSURF_X8R8G8B8:
    case gcvSURF_A8R8G8B8:
        /* No color packing needed. */
        *Color = Color32;
        break;

    case gcvSURF_A1R5G5B5:
    case gcvSURF_X1R5G5B5:
        *Color =
            ((colorA & 0x01) << 15) |
            ((colorR & 0x1F) << 10) |
            ((colorG & 0x1F) <<  5) |
            ((colorB & 0x1F));
        break;

    case gcvSURF_A4R4G4B4:
    case gcvSURF_X4R4G4B4:
        *Color =
            ((colorA & 0xF) << 12) |
            ((colorR & 0xF) <<  8) |
            ((colorG & 0xF) <<  4) |
            ((colorB & 0xF));
        break;

    case gcvSURF_R5G6B5:
        *Color =
            ((colorR & 0x1F) << 11) |
            ((colorG & 0x3F) <<  5) |
            ((colorB & 0x1F));
        break;

    default:
        return gcvSTATUS_NOT_SUPPORTED;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

