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
**	gcoHARDWARE_ClearRect
**
**	Append a command buffer with a CLEAR command.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gctUINT32 Address
**			Base address of surface to clear.
**
**		gctPOINTER Memory
**			Base address of surface to clear.
**
**		gctUINT32 Stride
**			Stride of surface.
**
**		gctINT32 Left
**			Left coordinate of rectangle to clear.
**
**		gctINT32 Top
**			Top coordinate of rectangle to clear.
**
**		gctINT32 Right
**			Right coordinate of rectangle to clear.
**
**		gctINT32 Bottom
**			Bottom coordinate of rectangle to clear.
**
**		gceSURF_FORMAT Format
**			Format of surface to clear.
**
**		gctUINT32 ClearValue
**			Value to be used for clearing the surface.
**
**		gctUINT8 ClearMask
**			Byte-mask to be used for clearing the surface.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoHARDWARE_ClearRect(
	IN gcoHARDWARE Hardware,
	IN gctUINT32 Address,
	IN gctPOINTER Memory,
	IN gctUINT32 Stride,
	IN gctINT32 Left,
	IN gctINT32 Top,
	IN gctINT32 Right,
	IN gctINT32 Bottom,
	IN gceSURF_FORMAT Format,
	IN gctUINT32 ClearValue,
	IN gctUINT8 ClearMask
	)
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Address=%x Memory=0x%x "
                  "Stride=%d Left=%d Top=%d "
                  "Right=%d Bottom=%d Format=%d "
                  "ClearValue=%d ClearMask=%d",
                  Hardware, Address, Memory,
                  Stride, Left, Top,
                  Right, Bottom, Format,
                  ClearValue, ClearMask);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Try hardware clear. */
    status = gcoHARDWARE_Clear(Hardware,
                               Address,
                               Stride,
                               Left,
                               Top,
                               Right,
                               Bottom,
                               Format,
                               ClearValue,
                               ClearMask);

    if (gcmIS_ERROR(status))
    {
        /* Use software clear. */
        status = gcoHARDWARE_ClearSoftware(Hardware,
                                           Memory,
                                           Stride,
                                           Left,
                                           Top,
                                           Right,
                                           Bottom,
                                           Format,
                                           ClearValue,
                                           ClearMask);
    }

    /* Return the staus. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**	gcoHARDWARE_Clear
**
**	Append a command buffer with a CLEAR command.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gctUINT32 Address
**			Base address of surface to clear.
**
**		gctUINT32 Stride
**			Stride of surface.
**
**		gctINT32 Left
**			Left coordinate of rectangle to clear.
**
**		gctINT32 Top
**			Top coordinate of rectangle to clear.
**
**		gctINT32 Right
**			Right coordinate of rectangle to clear.
**
**		gctINT32 Bottom
**			Bottom coordinate of rectangle to clear.
**
**		gceSURF_FORMAT Format
**			Format of surface to clear.
**
**		gctUINT32 ClearValue
**			Value to be used for clearing the surface.
**
**		gctUINT8 ClearMask
**			Byte-mask to be used for clearing the surface.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoHARDWARE_Clear(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctUINT32 Stride,
    IN gctINT32 Left,
    IN gctINT32 Top,
    IN gctINT32 Right,
    IN gctINT32 Bottom,
    IN gceSURF_FORMAT Format,
    IN gctUINT32 ClearValue,
    IN gctUINT8 ClearMask
    )
{
    gceSTATUS status;
    gctUINT32 format, swizzle, isYUVformat;
    gceTILING tiling;
    gctINT32 tileWidth, tileHeight;
    gctUINT32 leftMask, topMask, widthMask, heightMask;
    gctUINT32 stride;

    gcmHEADER_ARG("Hardware=0x%x Address=%x Stride=%d "
                  "Left=%d Top=%d Right=%d Bottom=%d "
                  "Format=%d ClearValue=%d ClearMask=%d",
                  Hardware, Address, Stride,
                  Left, Top, Right, Bottom,
                  Format, ClearValue, ClearMask);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    switch (Format)
    {
    case gcvSURF_X4R4G4B4:
    case gcvSURF_X1R5G5B5:
    case gcvSURF_R5G6B5:
    case gcvSURF_X4B4G4R4:
    case gcvSURF_X1B5G5R5:
        if (ClearMask == 0x7)
        {
            /* When the format has no alpha channel, fake the ClearMask to
            ** include alpha channel clearing.   This will allow us to use
            ** resolve clear. */
            ClearMask = 0xF;
        }
        break;

    default:
        break;
    }

    if ((ClearMask != 0xF)
    &&  (Format != gcvSURF_X8R8G8B8)
    &&  (Format != gcvSURF_A8R8G8B8)
    &&  (Format != gcvSURF_D24S8)
    &&  (Format != gcvSURF_D24X8)
    )
    {
        /* Don't clear with mask when channels are not byte sized. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Supertiled destination? */
    if ((Stride & 0x80000000U) != 0)
    {
        /* Set the tiling mode. */
        tiling = gcvSUPERTILED;

        /* Set the tile size. */
        tileWidth  = 64;
        tileHeight = 64;
    }

    /* Not supertiled. */
    else
    {
        /* Set the tiling mode. */
        tiling = gcvTILED;

        /* Query the tile size. */
        status = gcoHARDWARE_QueryTileSize(gcvNULL, gcvNULL,
                                           &tileWidth, &tileHeight,
                                           gcvNULL);

        if (gcmIS_ERROR(status))
        {
            /* Error. */
            gcmFOOTER();
            return status;
        }
    }

    /* All sides must be tile aligned. */
    leftMask = tileWidth - 1;
    topMask  = tileHeight - 1;

    /* The size has to be 4x1 tile aligned. */
    widthMask  = 4 * 4 - 1;
    heightMask = 4 * 1 - 1;

    /* Convert the format. */
    status = gcoHARDWARE_TranslateDestinationFormat(Hardware,
                                                    Format,
                                                    &format,
                                                    &swizzle,
                                                    &isYUVformat);

    if (status != gcvSTATUS_OK)
    {
        /* Error. */
        gcmFOOTER();
        return status;
    }

    /* For resolve clear, we need to be 4x1 tile aligned. */
    if ((( Left           & leftMask)   == 0)
    &&  (( Top            & topMask)    == 0)
    &&  (((Right  - Left) & widthMask)  == 0)
    &&  (((Bottom - Top)  & heightMask) == 0)
    )
    {
        gctUINT32 config, control, size;
        gctUINT32 dither[2] = { ~0U, ~0U };
        gctUINT32 offset, address, bitsPerPixel;

        /* Set up the starting address of clear rectangle. */
        gcmVERIFY_OK(
            gcoHARDWARE_ConvertFormat(Hardware,
                                      Format,
                                      &bitsPerPixel,
                                      gcvNULL));

        /* Compute the origin offset. */
        gcmVERIFY_OK(
            gcoHARDWARE_ComputeOffset(Left, Top,
                                      Stride,
                                      bitsPerPixel / 8,
                                      tiling, &offset));

        /* Determine the starting address. */
        address = Address + offset;

        /* Build AQRsConfig register. */
        config = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))

               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))

               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 14:14) - (0 ? 14:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ? 14:14))) | (((gctUINT32) ((gctUINT32) (gcvTRUE) & ((gctUINT32) ((((1 ? 14:14) - (0 ? 14:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ? 14:14)));

        /* Build AQRsClearControl register. */
        control = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (ClearMask) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16))) | (((gctUINT32) ((gctUINT32) (gcvTRUE) & ((gctUINT32) ((((1 ? 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16)));

        /* Build AQRsWindowSize register. */
        size = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (Right-Left) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

             | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (Bottom-Top) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

        /* Determine the stride. */
        stride = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 17:0) - (0 ? 17:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:0) - (0 ? 17:0) + 1))))))) << (0 ? 17:0))) | (((gctUINT32) ((gctUINT32) (Stride<<2) & ((gctUINT32) ((((1 ? 17:0) - (0 ? 17:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:0) - (0 ? 17:0) + 1))))))) << (0 ? 17:0)))

               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31))) | (((gctUINT32) ((gctUINT32) ((Stride>>31)) & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)))

               ;

        do
        {
            /* Switch to 3D pipe. */
            gcmERR_BREAK(
                gcoHARDWARE_SelectPipe(Hardware, 0x0));

            /* Flush cache. */
            gcmERR_BREAK(gcoHARDWARE_FlushPipe(Hardware));

            /* Program registers. */
            gcmERR_BREAK(gcoHARDWARE_LoadState32(Hardware,
                                                 0x01604,
                                                 config));

            gcmERR_BREAK(gcoHARDWARE_LoadState(Hardware,
                                               0x01630,
                                               2, dither));

            gcmERR_BREAK(gcoHARDWARE_LoadState32(Hardware,
                                                 0x01610,
                                                 address));

            gcmERR_BREAK(gcoHARDWARE_LoadState32(Hardware,
                                                 0x01614,
                                                 stride));

            gcmERR_BREAK(gcoHARDWARE_LoadState32(Hardware,
                                                 0x01620,
                                                 size));

            gcmERR_BREAK(gcoHARDWARE_LoadState32(Hardware,
                                                 0x01640,
                                                 ClearValue));

            gcmERR_BREAK(gcoHARDWARE_LoadState32(Hardware,
                                                 0x0163C,
                                                 control));

            /* Append new configuration register. */
            gcmERR_BREAK(
                gcoHARDWARE_LoadState32(Hardware,
                                        0x016A0,
                                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))));

            gcmERR_BREAK(gcoHARDWARE_LoadState32(Hardware,
                                                 0x01600,
                                                 0xBEEBBEEB));

            if ((  (Format == gcvSURF_D16)
                || (Format == gcvSURF_D24S8)
                || (Format == gcvSURF_D24X8)
                )
                &&  Hardware->earlyDepth
            )
            {
                /* Make raster wait for clear to be done. */
                gcmERR_BREAK(gcoHARDWARE_Semaphore(Hardware,
                                                   gcvWHERE_RASTER,
                                                   gcvWHERE_PIXEL,
                                                   gcvHOW_SEMAPHORE));
            }
        }
        while (gcvFALSE);

        /* Target has dirty pixels. */
        Hardware->targetDirty = gcvTRUE;

        /* Return the status. */
        gcmFOOTER();
        return status;
    }

    /* Removed 2D clear as it does not work for tiled buffers. */

    /* Error. */
    gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
**
**	gcoHARDWARE_ClearSoftware
**
**	Clear the buffer with software implementation. Buffer is assumed to be
**	tiled.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gctPOINTER LogicalAddress
**			Base address of surface to clear.
**
**		gctUINT32 Stride
**			Stride of surface.
**
**		gctINT32 Left
**			Left coordinate of rectangle to clear.
**
**		gctINT32 Top
**			Top coordinate of rectangle to clear.
**
**		gctINT32 Right
**			Right coordinate of rectangle to clear.
**
**		gctINT32 Bottom
**			Bottom coordinate of rectangle to clear.
**
**		gceSURF_FORMAT Format
**			Format of surface to clear.
**
**		gctUINT32 ClearValue
**			Value to be used for clearing the surface.
**
**		gctUINT8 ClearMask
**			Byte-mask to be used for clearing the surface.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoHARDWARE_ClearSoftware(
	IN gcoHARDWARE Hardware,
	IN gctPOINTER Address,
	IN gctUINT32 Stride,
	IN gctINT32 Left,
	IN gctINT32 Top,
	IN gctINT32 Right,
	IN gctINT32 Bottom,
	IN gceSURF_FORMAT Format,
	IN gctUINT32 ClearValue,
	IN gctUINT8 ClearMask
	)
{
    gceSTATUS status;
    gceTILING tiling;
    gctUINT32 bytesPerTile, bitsPerPixel;
    gctUINT32 offset;
    gctUINT8_PTR address;
    gctINT32 tileWidth, tileHeight, rowStride;
    gctINT32 x, y, tx, ty, bx, by, numPixels;
    gctUINT32 channelMask[4];

    gcmHEADER_ARG("Hardware=0x%x Address=%x Stride=%d "
                  "Left=%d Top=%d Right=%d Bottom=%d "
                  "Format=%d ClearValue=%d ClearMask=%d",
                  Hardware, Address, Stride,
                  Left, Top, Right, Bottom,
                  Format, ClearValue, ClearMask);

    /* For a clear that is not tile aligned, our hardware might not be able to
       do it.  So here is the software implementation. */

    /* Verify the rgeuments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    do
    {
        /* Flush the pipe. */
        gcmERR_BREAK(gcoHARDWARE_FlushPipe(Hardware));

        /* Commit the command queue. */
        gcmERR_BREAK(gcoHARDWARE_Commit(Hardware));

        /* Stall the hardware. */
        gcmERR_BREAK(gcoHARDWARE_Stall(Hardware));

        /* Supertiled destination? */
        if ((Stride & 0x80000000U) != 0)
        {
            /* Set the tiling mode. */
            tiling = gcvSUPERTILED;

            /* Set the tile size. */
            tileWidth  = 64;
            tileHeight = 64;

            /* Remove the supertiled bit. */
            Stride &= ~0x80000000U;
        }

        /* No, query tile size. */
        else
        {
            /* Set the tiling mode. */
            tiling = gcvTILED;

            /* Query the tile size. */
            gcmERR_BREAK(gcoHARDWARE_QueryTileSize(gcvNULL, gcvNULL,
                                                   &tileWidth, &tileHeight,
                                                   gcvNULL));
        }

        /* Query pixel depth. */
        gcmERR_BREAK(gcoHARDWARE_ConvertFormat(Hardware,
                                               Format,
                                               &bitsPerPixel,
                                               gcvNULL));

        /* Compute the number of bytes per tile. */
        bytesPerTile = (tileWidth * tileHeight * bitsPerPixel) / 8;

        switch (Format)
        {
        case gcvSURF_X4R4G4B4: /* 12-bit RGB color without alpha channel. */
            channelMask[0] = 0x000F;
            channelMask[1] = 0x00F0;
            channelMask[2] = 0x0F00;
            channelMask[3] = 0x0;
            break;

        case gcvSURF_D16:      /* 16-bit Depth. */
        case gcvSURF_A4R4G4B4: /* 12-bit RGB color with alpha channel. */
            channelMask[0] = 0x000F;
            channelMask[1] = 0x00F0;
            channelMask[2] = 0x0F00;
            channelMask[3] = 0xF000;
            break;

        case gcvSURF_X1R5G5B5: /* 15-bit RGB color without alpha channel. */
            channelMask[0] = 0x001F;
            channelMask[1] = 0x03E0;
            channelMask[2] = 0x7C00;
            channelMask[3] = 0x0;
            break;

        case gcvSURF_A1R5G5B5: /* 15-bit RGB color with alpha channel. */
            channelMask[0] = 0x001F;
            channelMask[1] = 0x03E0;
            channelMask[2] = 0x7C00;
            channelMask[3] = 0x8000;
            break;

        case gcvSURF_R5G6B5: /* 16-bit RGB color without alpha channel. */
            channelMask[0] = 0x001F;
            channelMask[1] = 0x07E0;
            channelMask[2] = 0xF800;
            channelMask[3] = 0x0;
            break;

        case gcvSURF_D24S8:    /* 24-bit Depth with 8 bit Stencil. */
        case gcvSURF_D24X8:    /* 24-bit Depth with 8 bit Stencil. */
        case gcvSURF_X8R8G8B8: /* 24-bit RGB without alpha channel. */
        case gcvSURF_A8R8G8B8: /* 24-bit RGB with alpha channel. */
            channelMask[0] = 0x000000FF;
            channelMask[1] = 0x0000FF00;
            channelMask[2] = 0x00FF0000;
            channelMask[3] = 0xFF000000;
            break;

        default:
            gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
            return gcvSTATUS_NOT_SUPPORTED;
        }

        /* Expand 16-bit mask into 32-bit mask. */
        if (bitsPerPixel == 16)
        {
            channelMask[0] = channelMask[0] | (channelMask[0] << 16);
            channelMask[1] = channelMask[1] | (channelMask[1] << 16);
            channelMask[2] = channelMask[2] | (channelMask[2] << 16);
            channelMask[3] = channelMask[3] | (channelMask[3] << 16);
        }

        /* Create an enclosing tile boundary of our unaligned rectangle. */
        tx = (Left / tileWidth)  * tileWidth;
        ty = (Top  / tileHeight) * tileHeight;
        bx = gcmALIGN(Right,  tileWidth);
        by = gcmALIGN(Bottom, tileHeight);

        /* Compute the origin offset. */
        gcmERR_BREAK(
            gcoHARDWARE_ComputeOffset(tx, ty,
                                      Stride,
                                      bitsPerPixel / 8,
                                      tiling, &offset));

        address = (gctUINT8_PTR) Address + offset;

        /*
            +---+---+---+---+---+---+
            |   |   |   |   |   |   |
            +---+---+---+---+---+---+
            |   |tx |   |   |   |A  |
            +---+---+---+---+---+---+
            |   |A' |   |   |   |   |
            +---+---+---+---+---+---+
            |   |   |   |   | bx|   |
            +---+---+---+---+---+---+

            A' = A + tileHeight * Stride - ((bx - tx) / tileWidth) * bytesPerTile)
        */
        rowStride = tileHeight * Stride - ((bx - tx) / tileWidth) * bytesPerTile;

        x         = tx;
        y         = ty;
        numPixels = (bx - tx) * (by - ty);
        gcmASSERT(numPixels >= 0);

        /* Move x,y in tiled order and loop through all tiles that enclose
           this rectangle. */
        while (numPixels-- > 0)
        {
            /* Draw only if x,y within clear rectangle. */
            if (bitsPerPixel == 32)
            {
                if ((x >= Left) && (y >= Top) && (x < Right) && (y < Bottom))
                {
                    switch (ClearMask)
                    {
                    case 0x1:
                        /* Common: Clear stencil only. */
                        * address = (gctUINT8) ClearValue;
                        break;

                    case 0xE:
                        /* Common: Clear depth only. */
                                           address[1] = (gctUINT8)  (ClearValue >> 8);
                        * (gctUINT16_PTR) &address[2] = (gctUINT16) (ClearValue >> 16);
                        break;

                    case 0xF:
                        /* Common: Clear everything. */
                        * (gctUINT32_PTR) address = ClearValue;
                        break;

                    default:
                        if (ClearMask & 0x1)
                        {
                            /* Clear byte 0. */
                            address[0] = (gctUINT8) ClearValue;
                        }

                        if (ClearMask & 0x2)
                        {
                            /* Clear byte 1. */
                            address[1] = (gctUINT8) (ClearValue >> 8);
                        }

                        if (ClearMask & 0x4)
                        {
                            /* Clear byte 2. */
                            address[2] = (gctUINT8) (ClearValue >> 16);
                        }

                        if (ClearMask & 0x8)
                        {
                            /* Clear byte 3. */
                            address[3] = (gctUINT8) (ClearValue >> 24);
                        }
                    }
                }
            }

            else if (bitsPerPixel == 16)
            {
                if (((x + 1) >= Left) && (y >= Top) && (x < Right) && (y < Bottom))
                {
                    if ((x + 1) == Right)
                    {
                        /* Dont write on Right pixel. */
                        channelMask[0] = channelMask[0] & 0x0000FFFF;
                        channelMask[1] = channelMask[1] & 0x0000FFFF;
                        channelMask[2] = channelMask[2] & 0x0000FFFF;
                        channelMask[3] = channelMask[3] & 0x0000FFFF;
                    }

                    if ((x + 1) == Left)
                    {
                        /* Dont write on Left pixel. */
                        channelMask[0] = channelMask[0] & 0xFFFF0000;
                        channelMask[1] = channelMask[1] & 0xFFFF0000;
                        channelMask[2] = channelMask[2] & 0xFFFF0000;
                        channelMask[3] = channelMask[3] & 0xFFFF0000;
                    }

                    if (ClearMask & 0x1)
                    {
                        /* Clear byte 0. */
                        *(gctUINT32_PTR) address = (*(gctUINT32_PTR) address & ~channelMask[0])
                                                 | (              ClearValue &  channelMask[0]);
                    }

                    if (ClearMask & 0x2)
                    {
                        /* Clear byte 1. */
                        *(gctUINT32_PTR) address = (*(gctUINT32_PTR) address & ~channelMask[1])
                                                 | (              ClearValue &  channelMask[1]);
                    }

                    if (ClearMask & 0x4)
                    {
                        /* Clear byte 2. */
                        *(gctUINT32_PTR) address = (*(gctUINT32_PTR) address & ~channelMask[2])
                                                 | (              ClearValue &  channelMask[2]);
                    }

                    if (ClearMask & 0x8)
                    {
                        /* Clear byte 3. */
                        *(gctUINT32_PTR) address = (*(gctUINT32_PTR) address & ~channelMask[3])
                                                 | (              ClearValue &  channelMask[3]);
                    }

                    if ((x + 1) == Left)
                    {
                        /* Restore channel mask. */
                        channelMask[0] = channelMask[0] | (channelMask[0] >> 16);
                        channelMask[1] = channelMask[1] | (channelMask[1] >> 16);
                        channelMask[2] = channelMask[2] | (channelMask[2] >> 16);
                        channelMask[3] = channelMask[3] | (channelMask[3] >> 16);
                    }

                    if ((x + 1) == Right)
                    {
                        /* Restore channel mask. */
                        channelMask[0] = channelMask[0] | (channelMask[0] << 16);
                        channelMask[1] = channelMask[1] | (channelMask[1] << 16);
                        channelMask[2] = channelMask[2] | (channelMask[2] << 16);
                        channelMask[3] = channelMask[3] | (channelMask[3] << 16);
                    }
                }
            }

            /* Increment address and x. */
            address += 4;
            x += 32 / bitsPerPixel;

            /* Verify 4-pixel horizontal boundary. */
            if ((x & 0x03) != 0)
            {
                continue;
            }

            /******* HORIZONTAL 4-PIXEL BOUNDARY *************************/

            /* Update to the next line. */
            y += 1;

            /* Verify 4-pixel vertical boundary. */
            if ((y & 0x03) != 0)
            {
                /* Go to the next line. */
                x -= 4;
                continue;
            }

            /******* VERTICAL 4-PIXEL BOUNDARY ***************************/

            /* In supertiled mode? */
            if (tiling == gcvSUPERTILED)
            {
                /* Verify 8x4 horizonal boundary. */
                if ((x & 0x04) != 0)
                {
                    /* Go to the next 4x4 column in 8x16 block. */
                    y -= 4;
                    continue;
                }

                /******* 8x4 BOUNDARY ****************************************/

                /* Verify 8x16 vertical boundary. */
                if ((y & 0x0C) != 0)
                {
                    /* Go back to the first column. */
                    x -= 8;
                    continue;
                }

                /******* 8x16 BOUNDARY ***************************************/

                /* Verify 64x16 boundary. */
                if ((x & 0x38) != 0)
                {
                    /* Go to the next 8x16 block to the right. */
                    y -= 16;
                    continue;
                }

                /******* 64x16 BOUNDARY **************************************/

                /* Verify 64x64 boundary. */
                if ((y & 0x30) != 0)
                {
                    /* Go to the first 8x16 block in the 64x64 tile. */
                    x -= 64;
                    continue;
                }

                /******* 64x64 BOUNDARY **************************************/
            }

            /* At last tile of this row? */
            if (x == bx)
            {
                x = tx;
                address += rowStride;
            }
            else
            {
                /* Goto to start of first line in next tile. */
                y -= tileHeight;
            }
        }

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    /* Return the status. */
    gcmFOOTER();
    return status;
}


