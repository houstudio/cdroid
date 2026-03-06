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
#include "gc_hal_user_context.h"
#include "gc_hal_user.h"
#include "gc_hal_user_brush.h"

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcvZONE_HARDWARE

/******************************************************************************\
********************************* Support Code *********************************
\******************************************************************************/

static gctUINT32 _SetBitWidth(
    gctUINT32 Value,
    gctINT8 CurWidth,
    gctINT8 NewWidth
    )
{
    gctUINT32 result;
    gctINT8 widthDiff;

    /* Mask source bits. */
    Value &= ((gctUINT64) 1 << CurWidth) - 1;

    /* Init the result. */
    result = Value;

    /* Determine the difference in width. */
    widthDiff = NewWidth - CurWidth;

    /* Until the difference is not zero... */
    while (widthDiff)
    {
        /* New value is thiner then current? */
        if (widthDiff < 0)
        {
            result >>= -widthDiff;
            widthDiff = 0;
        }

        /* Full source replication? */
        else if (widthDiff >= CurWidth)
        {
            result = (CurWidth == 32) ? Value
                                      : ((result << CurWidth) | Value);
            widthDiff -= CurWidth;
        }

        /* Partial source replication. */
        else
        {
            result = (result << widthDiff) | (Value >> (CurWidth - widthDiff));
            widthDiff = 0;
        }
    }

    /* Return result. */
    return result;
}

static void _ConvertComponent(
    gctUINT8* SrcPixel,
    gctUINT8* TrgPixel,
    gctUINT SrcBit,
    gctUINT TrgBit,
    gcsFORMAT_COMPONENT* SrcComponent,
    gcsFORMAT_COMPONENT* TrgComponent,
    gcsBOUNDARY_PTR SrcBoundary,
    gcsBOUNDARY_PTR TrgBoundary,
    gctUINT32 Default
    )
{
    gctUINT32 srcValue;
    gctUINT8 srcWidth;
    gctUINT8 trgWidth;
    gctUINT32 trgMask;
    gctUINT32 bits;

    /* Exit if target is beyond the boundary. */
    if ((TrgBoundary != gcvNULL) &&
        ((TrgBoundary->x < 0) || (TrgBoundary->x >= TrgBoundary->width) ||
         (TrgBoundary->y < 0) || (TrgBoundary->y >= TrgBoundary->height)))
    {
        return;
    }

    /* Exit if target component is not present. */
    if (TrgComponent->width == gcvCOMPONENT_NOTPRESENT)
    {
        return;
    }

    /* Extract target width. */
    trgWidth = TrgComponent->width & gcvCOMPONENT_WIDTHMASK;

    /* Extract the source. */
    if ((SrcComponent == gcvNULL) ||
        (SrcComponent->width == gcvCOMPONENT_NOTPRESENT) ||
        (SrcComponent->width &  gcvCOMPONENT_DONTCARE)   ||
        ((SrcBoundary != gcvNULL) &&
         ((SrcBoundary->x < 0) || (SrcBoundary->x >= SrcBoundary->width) ||
          (SrcBoundary->y < 0) || (SrcBoundary->y >= SrcBoundary->height))))
    {
        srcValue = Default;
        srcWidth = 32;
    }
    else
    {
        /* Extract source width. */
        srcWidth = SrcComponent->width & gcvCOMPONENT_WIDTHMASK;

        /* Compute source position. */
        SrcBit += SrcComponent->start;
        SrcPixel += SrcBit >> 3;
        SrcBit &= 7;

        /* Compute number of bits to read from source. */
        bits = SrcBit + srcWidth;

        /* Read the value. */
        srcValue = SrcPixel[0] >> SrcBit;

        if (bits > 8)
        {
            /* Read up to 16 bits. */
            srcValue |= SrcPixel[1] << (8 - SrcBit);
        }

        if (bits > 16)
        {
            /* Read up to 24 bits. */
            srcValue |= SrcPixel[2] << (16 - SrcBit);
        }

        if (bits > 24)
        {
            /* Read up to 32 bits. */
            srcValue |= SrcPixel[3] << (24 - SrcBit);
        }
    }

    /* Make the source component the same width as the target. */
    srcValue = _SetBitWidth(srcValue, srcWidth, trgWidth);

    /* Compute destination position. */
    TrgBit += TrgComponent->start;
    TrgPixel += TrgBit >> 3;
    TrgBit &= 7;

    /* Determine the target mask. */
    trgMask = (gctUINT32) (((gctUINT64) 1 << trgWidth) - 1);
    trgMask <<= TrgBit;

    /* Align the source value. */
    srcValue <<= TrgBit;

    /* Loop while there are bits to set. */
    while (trgMask != 0)
    {
        /* Set 8 bits of the pixel value. */
        if ((trgMask & 0xFF) == 0xFF)
        {
            /* Set all 8 bits. */
            *TrgPixel = (gctUINT8) srcValue;
        }
        else
        {
            /* Set the required bits. */
            *TrgPixel = (gctUINT8) ((*TrgPixel & ~trgMask) | srcValue);
        }

        /* Next 8 bits. */
        TrgPixel ++;
        trgMask  >>= 8;
        srcValue >>= 8;
    }
}

static gctUINT32 _Average2Colors(
    gctUINT32 Color1,
    gctUINT32 Color2
    )
{
    gctUINT32 byte102 =  Color1 & 0x00FF00FF;
    gctUINT32 byte113 =  (Color1 & 0xFF00FF00) >> 1;

    gctUINT32 byte202 =  Color2 & 0x00FF00FF;
    gctUINT32 byte213 =  (Color2 & 0xFF00FF00) >> 1;

    gctUINT32 sum02 = (byte102 + byte202) >> 1;
    gctUINT32 sum13 = (byte113 + byte213);

    gctUINT32 average
        = (sum02 & 0x00FF00FF)
        | (sum13 & 0xFF00FF00);

    return average;
}

static gctUINT32 _Average4Colors(
    gctUINT32 Color1,
    gctUINT32 Color2,
    gctUINT32 Color3,
    gctUINT32 Color4
    )
{
    gctUINT32 byte102 =  Color1 & 0x00FF00FF;
    gctUINT32 byte113 =  (Color1 & 0xFF00FF00) >> 2;

    gctUINT32 byte202 =  Color2 & 0x00FF00FF;
    gctUINT32 byte213 =  (Color2 & 0xFF00FF00) >> 2;

    gctUINT32 byte302 =  Color3 & 0x00FF00FF;
    gctUINT32 byte313 =  (Color3 & 0xFF00FF00) >> 2;

    gctUINT32 byte402 =  Color4 & 0x00FF00FF;
    gctUINT32 byte413 =  (Color4 & 0xFF00FF00) >> 2;

    gctUINT32 sum02 = (byte102 + byte202 + byte302 + byte402) >> 2;
    gctUINT32 sum13 = (byte113 + byte213 + byte313 + byte413);

    gctUINT32 average
        = (sum02 & 0x00FF00FF)
        | (sum13 & 0xFF00FF00);

    return average;
}

/******************************************************************************\
****************************** gcoHARDWARE API code *****************************
\******************************************************************************/

/*******************************************************************************
**
**  gcoHARDWARE_Construct
**
**  Construct a new gcoHARDWARE object.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to an gcoHAL object.
**
**  OUTPUT:
**
**      gcoHARDWARE * Hardware
**          Pointer to a variable that will hold the gcoHARDWARE object.
*/
gceSTATUS
gcoHARDWARE_Construct(
    IN gcoHAL Hal,
    OUT gcoHARDWARE * Hardware
    )
{
    gceSTATUS status;
    gcoOS os;
    gcoHARDWARE hardware = gcvNULL;
    gcsHAL_INTERFACE iface;
    gctUINT16 data = 0xff00;

    gcmHEADER_ARG("Hal=0x%x", Hal);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);
    gcmVERIFY_ARGUMENT(Hardware != gcvNULL);

    os = Hal->os;

    do
    {
        /* Allocate the gcoHARDWARE object. */
        gcmERR_BREAK(gcoOS_Allocate(os,
                                    gcmSIZEOF(struct _gcoHARDWARE),
                                    (gctPOINTER *) &hardware));

        /* Reset the object. */
        gcmVERIFY_OK(gcoOS_ZeroMemory(hardware,
                                      gcmSIZEOF(struct _gcoHARDWARE)));

        /* Initialize the gcoHARDWARE object. */
        hardware->object.type = gcvOBJ_HARDWARE;
        hardware->os  = os;
        hardware->hal = Hal;

        hardware->buffer  = gcvNULL;
        hardware->context = gcvNULL;
        hardware->queue   = gcvNULL;

        /* Query chip identity. */
#ifdef NO_KERNEL
        gcoOS_ZeroMemory(&iface.u.QueryChipIdentity,
                         gcmSIZEOF(iface.u.QueryChipIdentity));
        iface.u.QueryChipIdentity.chipModel = gcv600;
#else
        iface.command = gcvHAL_QUERY_CHIP_IDENTITY;

        gcmERR_BREAK(gcoOS_DeviceControl(os,
                                         IOCTL_GCHAL_INTERFACE,
                                         &iface, gcmSIZEOF(iface),
                                         &iface, gcmSIZEOF(iface)));

        gcmERR_BREAK(iface.status);
#endif

        hardware->chipModel              = iface.u.QueryChipIdentity.chipModel;
        hardware->chipRevision           = iface.u.QueryChipIdentity.chipRevision;
        hardware->chipFeatures           = iface.u.QueryChipIdentity.chipFeatures;
        hardware->chipMinorFeatures      = iface.u.QueryChipIdentity.chipMinorFeatures;
        hardware->chipMinorFeatures1     = iface.u.QueryChipIdentity.chipMinorFeatures1;
        hardware->streamCount            = iface.u.QueryChipIdentity.streamCount;
        hardware->registerMax            = iface.u.QueryChipIdentity.registerMax;
        hardware->threadCount            = iface.u.QueryChipIdentity.threadCount;
        hardware->shaderCoreCount        = iface.u.QueryChipIdentity.shaderCoreCount;
        hardware->vertexCacheSize        = iface.u.QueryChipIdentity.vertexCacheSize;
        hardware->vertexOutputBufferSize = iface.u.QueryChipIdentity.vertexOutputBufferSize;

        /* Check if big endian */
        hardware->bigEndian = (*(gctUINT8 *)&data == 0xff);

        /* Set default API. */
        hardware->api = gcvAPI_OPENGL;

#ifndef NO_KERNEL
        /* Construct the gcoCONTEXT object. */
        gcmERR_BREAK(gcoCONTEXT_Construct(os, hardware, &hardware->context));

        /* Construct the command buffer. */
        gcmERR_BREAK(gcoBUFFER_Construct(Hal,
                                         hardware,
                                         32 << 10,
                                         &hardware->buffer));
#endif

        /* Construct the event queue. */
        gcmERR_BREAK(gcoQUEUE_Construct(os, &hardware->queue));

        /* Sync filter variables. */
        gcmVERIFY_OK(gcoOS_ZeroMemory(&hardware->horSyncFilterKernel,
                                      gcmSIZEOF(hardware->horSyncFilterKernel)));

        gcmVERIFY_OK(gcoOS_ZeroMemory(&hardware->verSyncFilterKernel,
                                      gcmSIZEOF(hardware->verSyncFilterKernel)));

        hardware->horSyncFilterKernel.filterType = gcvFILTER_SYNC;
        hardware->verSyncFilterKernel.filterType = gcvFILTER_SYNC;

        hardware->horSyncFilterKernel.kernelChanged = gcvTRUE;
        hardware->verSyncFilterKernel.kernelChanged = gcvTRUE;

        /* Blur filter variables. */
        gcmVERIFY_OK(gcoOS_ZeroMemory(&hardware->horBlurFilterKernel,
                                      gcmSIZEOF(hardware->horBlurFilterKernel)));

        gcmVERIFY_OK(gcoOS_ZeroMemory(&hardware->verBlurFilterKernel,
                                      gcmSIZEOF(hardware->verBlurFilterKernel)));

        hardware->horBlurFilterKernel.filterType = gcvFILTER_BLUR;
        hardware->verBlurFilterKernel.filterType = gcvFILTER_BLUR;

        hardware->horBlurFilterKernel.kernelChanged = gcvTRUE;
        hardware->verBlurFilterKernel.kernelChanged = gcvTRUE;

        /* User defined filter variables. */
        hardware->horUserFilterKernel.filterType = gcvFILTER_USER;
        hardware->verUserFilterKernel.filterType = gcvFILTER_USER;

        hardware->horUserFilterKernel.kernelChanged = gcvTRUE;
        hardware->verUserFilterKernel.kernelChanged = gcvTRUE;

        /* Filter blit variables. */
        hardware->horUserFilterPass = gcvTRUE;
        hardware->verUserFilterPass = gcvTRUE;

        hardware->loadedFilterType  = gcvFILTER_SYNC;
        hardware->loadedKernelSize  = 0;
        hardware->loadedScaleFactor = 0;

        hardware->newFilterType     = gcvFILTER_SYNC;
        hardware->newHorKernelSize  = 9;
        hardware->newVerKernelSize  = 9;

        /* Reset the temporary surface. */
        gcoOS_ZeroMemory(&hardware->tempBuffer, sizeof(hardware->tempBuffer));
        hardware->tempBuffer.node.pool = gcvPOOL_UNKNOWN;

        hardware->stencilMode    = 0x0;
        hardware->stencilEnabled = gcvFALSE;

        /* Don't stall before primitive. */
        hardware->stallPrimitive = gcvFALSE;
        hardware->earlyDepth     = gcvFALSE;

        /* Disable tile status. */
        hardware->memoryConfig  = 0;
        hardware->paused        = gcvFALSE;
        hardware->cacheDirty    = gcvFALSE;
        hardware->targetDirty   = gcvFALSE;
        hardware->currentTarget = gcvNULL;
        hardware->currentDepth  = gcvNULL;
        hardware->inFlush       = gcvFALSE;

        /* Disable anti-alias. */
        hardware->sampleMask   = 0xF;
        hardware->sampleEnable = 0xF;
        hardware->samples.x    = 1;
        hardware->samples.y    = 1;
        hardware->vaa          = gcvVAA_NONE;

        /* Disable dithering. */
        hardware->dither[0] = hardware->dither[1] = ~0U;

        /* Determine whether 2D hardware is present. */
        hardware->hw2DEngine = ((((gctUINT32) (hardware->chipFeatures)) >> (0 ? 9:9) & ((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1)))))));
        /* Don't force software by default. */
        hardware->sw2DEngine = gcvFALSE;

        /* Determine whether PE 2.0 is present. */
        hardware->hw2DPE20 = ((((gctUINT32) (hardware->chipMinorFeatures)) >> (0 ? 7:7) & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1)))))));

        /* Determine whether byte write feature is present in the chip. */
        hardware->byteWrite = ((((gctUINT32) (hardware->chipFeatures)) >> (0 ? 19:19) & ((gctUINT32) ((((1 ? 19:19) - (0 ? 19:19) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:19) - (0 ? 19:19) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 19:19) - (0 ? 19:19) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:19) - (0 ? 19:19) + 1)))))));

        /* Determine whether full rotation is present in the chip. */
        if (hardware->chipRevision < 0x4310)
        {
            hardware->fullBitBlitRotation    = gcvFALSE;
            hardware->fullFilterBlitRotation = gcvFALSE;
        }
        else if (hardware->chipRevision == 0x4310)
        {
            hardware->fullBitBlitRotation    = gcvTRUE;
            hardware->fullFilterBlitRotation = gcvFALSE;
        }
        else if (hardware->chipRevision >= 0x4400)
        {
            hardware->fullBitBlitRotation    = gcvTRUE;
            hardware->fullFilterBlitRotation = gcvTRUE;
        }

        /* MASK register is missing on 4.3.1_rc0. */
        if (hardware->chipRevision == 0x4310)
        {
            hardware->shadowRotAngleReg = gcvTRUE;
            hardware->rotAngleRegShadow = 0x00000000;
        }
        else
        {
            hardware->shadowRotAngleReg = gcvFALSE;
        }

        /* Construct brush cache object. */
        gcmERR_BREAK(gcoBRUSH_CACHE_Construct(Hal, &hardware->brushCache));

        /* Reset temporary pattern table. */
        hardware->patternTable = gcvNULL;
        hardware->patternTableProgram = gcvFALSE;
        hardware->patternTableFirstIndex = 0;
        hardware->patternTableIndexCount = 0;

        /* Reset programming mono color. */
        hardware->monoColorProgram = gcvFALSE;

        /* Reset programming transparency color. */
        hardware->transparencyColorProgram = gcvFALSE;

        /* Initialize variables for bandwidth optimization. */
        hardware->alphaBlendEnable = gcvFALSE;
        hardware->colorWrite       = 0xF;
        hardware->colorCompression = gcvFALSE;
        hardware->destinationRead  = ~0U;

        /* Determine striping. */
        if ((hardware->chipModel >= gcv860)
        &&  !((((gctUINT32) (hardware->chipMinorFeatures1)) >> (0 ? 4:4) & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1)))))))
        )
        {
            /* Get number of cache lines. */
            hardware->needStriping = hardware->hw2DEngine ? 32 : 16;
        }
        else
        {
            /* No need for striping. */
            hardware->needStriping = 0;
        }

        gcmERR_BREAK(
            gcoOS_CreateSignal(os, gcvFALSE, &hardware->stallSignal));

        /* Return pointer to the gcoHARDWARE object. */
        *Hardware = hardware;

        /* Success. */
        gcmFOOTER_ARG("*Hardware=0x%x", *Hardware);
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    if (hardware != gcvNULL)
    {
        /* Roll back. */
        if (hardware->brushCache != gcvNULL)
        {
            gcmVERIFY_OK(gcoBRUSH_CACHE_Destroy(hardware->brushCache));
        }

        if (hardware->buffer != gcvNULL)
        {
            gcmVERIFY_OK(gcoBUFFER_Destroy(hardware->buffer));
        }

        if (hardware->context != gcvNULL)
        {
            gcmVERIFY_OK(gcoCONTEXT_Destroy(hardware->context));
        }

        if (hardware->queue != gcvNULL)
        {
            gcmVERIFY_OK(gcoQUEUE_Destroy(hardware->queue));
        }

        gcmVERIFY_OK(gcoOS_Free(os, hardware));
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_Destroy
**
**  Destroy an gcoHARDWARE object.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object that needs to be destroyed.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gcoHARDWARE_Destroy(
    IN gcoHARDWARE Hardware
    )
{
    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Destroy brush cache. */
    gcmVERIFY_OK(gcoBRUSH_CACHE_Destroy(Hardware->brushCache));

    /* Destroy temporary surface */
    if (Hardware->tempSurface != gcvNULL)
    {
        Hardware->tempSurface->hal->engine3D = gcvNULL;
        gcmVERIFY_OK(gcoSURF_Destroy(Hardware->tempSurface));
    }

    /* Destroy the command buffer. */
    if (Hardware->buffer != gcvNULL)
    {
        gcmVERIFY_OK(gcoBUFFER_Destroy(Hardware->buffer));
    }

    /* Destroy the context buffer. */
    if (Hardware->context != gcvNULL)
    {
        gcmVERIFY_OK(gcoCONTEXT_Destroy(Hardware->context));
    }

    /* Commit the event queue. */
    gcmVERIFY_OK(gcoQUEUE_Commit(Hardware->queue));

    /* Destroy the event queue. */
    gcmVERIFY_OK(gcoQUEUE_Destroy(Hardware->queue));

    /* 2D pattern cleanup. */
    if (Hardware->patternTable != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_Free(Hardware->os, Hardware->patternTable));
    }

    /* Free kernel array. */
    gcmVERIFY_OK(gcoHARDWARE_FreeKernelArray(Hardware));

    /* Free temporary buffer allocated by filter blit operation. */
    gcmVERIFY_OK(gcoHARDWARE_FreeTemporarySurface(Hardware, gcvFALSE));

    /* Destroy the stall signal. */
    gcmVERIFY_OK(
        gcoOS_DestroySignal(Hardware->os, Hardware->stallSignal));

    /* Mark the gcoHARDWARE object as unknown. */
    Hardware->object.type = gcvOBJ_UNKNOWN;

    /* Free the gcoHARDWARE object. */
    gcmVERIFY_OK(gcoOS_Free(Hardware->os, Hardware));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_QueryChipIdentity
**
**  Query the identity of the hardware.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**  OUTPUT:
**
**      gceCHIPMODEL* ChipModel
**          If 'ChipModel' is not gcvNULL, the variable it points to will
**          receive the model of the chip.
**
**      gctUINT32* ChipRevision
**          If 'ChipRevision' is not gcvNULL, the variable it points to will
**          receive the revision of the chip.
**
**      gctUINT32* ChipFeatures
**          If 'ChipFeatures' is not gcvNULL, the variable it points to will
**          receive the feature set of the chip.
**
**      gctUINT32 * ChipMinorFeatures
**          If 'ChipMinorFeatures' is not gcvNULL, the variable it points to
**          will receive the minor feature set of the chip.
**
**      gctUINT32 * ChipMinorFeatures1
**          If 'ChipMinorFeatures1' is not gcvNULL, the variable it points to
**          will receive the minor feature set 1 of the chip.
**
*/
gceSTATUS gcoHARDWARE_QueryChipIdentity(
    IN gcoHARDWARE Hardware,
    OUT gceCHIPMODEL* ChipModel,
    OUT gctUINT32* ChipRevision,
    OUT gctUINT32* ChipFeatures,
    OUT gctUINT32* ChipMinorFeatures,
    OUT gctUINT32* ChipMinorFeatures1
    )
{
    gcmHEADER_ARG("Hardware=0x%x ChipModel=0x%x ChipRevision=0x%x "
                    "ChipFeatures=0x%x ChipMinorFeatures=0x%x ChipMinorFeatures1=0x%x",
                    Hardware, ChipModel, ChipRevision,
                    ChipFeatures, ChipMinorFeatures, ChipMinorFeatures1);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Return chip model. */
    if (ChipModel != gcvNULL)
    {
        *ChipModel = Hardware->chipModel;
    }

    /* Return revision number. */
    if (ChipRevision != gcvNULL)
    {
        *ChipRevision = Hardware->chipRevision;
    }

    /* Return feature set. */
    if (ChipFeatures != gcvNULL)
    {
        *ChipFeatures = Hardware->chipFeatures;
    }

    /* Return minor feature set. */
    if (ChipMinorFeatures != gcvNULL)
    {
        *ChipMinorFeatures = Hardware->chipMinorFeatures;
    }

    /* Return minor feature set 1. */
    if (ChipMinorFeatures1 != gcvNULL)
    {
        *ChipMinorFeatures1 = Hardware->chipMinorFeatures1;
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_TranslateSourceFormat
**
**  Translate API source color format to its hardware value.
**  Checks PE2D feature to determine if the format is supported or not.
**
**  INPUT:
**
**      gceSURF_FORMAT APIValue
**          API value.
**
**  OUTPUT:
**
**      gctUINT32 * HwValue
**          Corresponding hardware value.
*/
gceSTATUS gcoHARDWARE_TranslateSourceFormat(
    IN gcoHARDWARE Hardware,
    IN gceSURF_FORMAT APIValue,
    OUT gctUINT32* HwValue,
    OUT gctUINT32* HwSwizzleValue,
    OUT gctUINT32* HwIsYUVValue
    )
{
    gctUINT32 swizzle_argb, swizzle_rgba, swizzle_abgr, swizzle_bgra;
    gctUINT32 swizzle_uv, swizzle_vu;

    gcmHEADER_ARG("Hardware=0x%x APIValue=%d HwValue=0x%x "
                    "HwSwizzleValue=0x%x HwIsYUVValue=0x%x",
                    Hardware, APIValue, HwValue,
                    HwSwizzleValue, HwIsYUVValue);

    swizzle_argb = 0x0;
    swizzle_rgba = 0x1;
    swizzle_abgr = 0x2;
    swizzle_bgra = 0x3;

    swizzle_uv = 0x0;
    swizzle_vu = 0x1;


    /* Default values. */
    *HwIsYUVValue = gcvFALSE;
    *HwSwizzleValue = swizzle_argb;

    /* Dispatch on format. */
    switch (APIValue)
    {
    case gcvSURF_INDEX8:
        *HwValue = 0x9;
        break;

    case gcvSURF_X4R4G4B4:
        *HwValue = 0x0;
        break;

    case gcvSURF_A4R4G4B4:
        *HwValue = 0x1;
        break;

    case gcvSURF_X1R5G5B5:
        *HwValue = 0x2;
        break;

    case gcvSURF_A1R5G5B5:
        *HwValue = 0x3;
        break;

    case gcvSURF_R4G4B4X4:
        *HwValue = 0x0;
        *HwSwizzleValue = swizzle_rgba;
        break;

    case gcvSURF_X4B4G4R4:
        *HwValue = 0x0;
        *HwSwizzleValue = swizzle_abgr;
        break;

    case gcvSURF_B4G4R4X4:
        *HwValue = 0x0;
        *HwSwizzleValue = swizzle_bgra;
        break;

    case gcvSURF_R4G4B4A4:
        *HwValue = 0x1;
        *HwSwizzleValue = swizzle_rgba;
        break;

    case gcvSURF_B4G4R4A4:
        *HwValue = 0x1;
        *HwSwizzleValue = swizzle_bgra;
        break;

    case gcvSURF_A4B4G4R4:
        *HwValue = 0x1;
        *HwSwizzleValue = swizzle_abgr;
        break;

    case gcvSURF_R5G5B5X1:
        *HwValue = 0x2;
        *HwSwizzleValue = swizzle_rgba;
        break;

    case gcvSURF_B5G5R5X1:
        *HwValue = 0x2;
        *HwSwizzleValue = swizzle_bgra;
        break;

    case gcvSURF_X1B5G5R5:
        *HwValue = 0x2;
        *HwSwizzleValue = swizzle_abgr;
        break;

    case gcvSURF_R5G5B5A1:
        *HwValue = 0x3;
        *HwSwizzleValue = swizzle_rgba;
        break;

    case gcvSURF_B5G5R5A1:
        *HwValue = 0x3;
        *HwSwizzleValue = swizzle_bgra;
        break;

    case gcvSURF_A1B5G5R5:
        *HwValue = 0x3;
        *HwSwizzleValue = swizzle_abgr;
        break;

    case gcvSURF_R5G6B5:
    case gcvSURF_D16:
        *HwValue = 0x4;
        break;

    case gcvSURF_B5G6R5:
        *HwValue = 0x4;
        *HwSwizzleValue = swizzle_abgr;
        break;

    case gcvSURF_X8R8G8B8:
        *HwValue = 0x5;
        break;

    case gcvSURF_R8G8B8X8:
        *HwValue = 0x5;
        *HwSwizzleValue = swizzle_rgba;
        break;

    case gcvSURF_B8G8R8X8:
        *HwValue = 0x5;
        *HwSwizzleValue = swizzle_bgra;
        break;

    case gcvSURF_X8B8G8R8:
        *HwValue = 0x5;
        *HwSwizzleValue = swizzle_abgr;
        break;

    case gcvSURF_A8R8G8B8:
    case gcvSURF_D24S8:
    case gcvSURF_D24X8:
        *HwValue = 0x6;
        break;

    case gcvSURF_R8G8B8A8:
        *HwValue = 0x6;
        *HwSwizzleValue = swizzle_rgba;
        break;

    case gcvSURF_B8G8R8A8:
        *HwValue = 0x6;
        *HwSwizzleValue = swizzle_bgra;
        break;

    case gcvSURF_A8B8G8R8:
        *HwValue = 0x6;
        *HwSwizzleValue = swizzle_abgr;
        break;

    case gcvSURF_YUY2:
        *HwValue = 0x7;
        *HwSwizzleValue = swizzle_uv;
        *HwIsYUVValue = gcvTRUE;
        break;

    case gcvSURF_YVYU:
        *HwValue = 0x7;
        *HwSwizzleValue = swizzle_vu;
        *HwIsYUVValue = gcvTRUE;
        break;

    case gcvSURF_UYVY:
        *HwValue = 0x8;
        *HwSwizzleValue = swizzle_uv;
        *HwIsYUVValue = gcvTRUE;
        break;

    case gcvSURF_VYUY:
        *HwValue = 0x8;
        *HwSwizzleValue = swizzle_vu;
        *HwIsYUVValue = gcvTRUE;
        break;

    case gcvSURF_YV12:
    case gcvSURF_I420:
        *HwValue = 0xF;
        *HwSwizzleValue = swizzle_uv;
        *HwIsYUVValue = gcvTRUE;
        break;

    case gcvSURF_A8:
        *HwValue = 0x10;
        break;

    case gcvSURF_NV12:
        *HwValue = 0x11;
        *HwSwizzleValue = swizzle_uv;
        *HwIsYUVValue = gcvTRUE;
        break;

    case gcvSURF_NV21:
        *HwValue = 0x11;
        *HwSwizzleValue = swizzle_vu;
        *HwIsYUVValue = gcvTRUE;
        break;

    case gcvSURF_NV16:
        *HwValue = 0x12;
        *HwSwizzleValue = swizzle_uv;
        *HwIsYUVValue = gcvTRUE;
        break;

    case gcvSURF_NV61:
        *HwValue = 0x12;
        *HwSwizzleValue = swizzle_vu;
        *HwIsYUVValue = gcvTRUE;
        break;

#if 0
    /* FIXME: remove HDR support for now. */
    case gcvSURF_HDR7E3:
        *HwValue = AQDE_DEST_CONFIG_FORMAT_HDR7E3;
        break;

    case gcvSURF_HDR6E4:
        *HwValue = AQDE_DEST_CONFIG_FORMAT_HDR6E4;
        break;

    case gcvSURF_HDR5E5:
        *HwValue = AQDE_DEST_CONFIG_FORMAT_HDR5E5;
        break;

    case gcvSURF_HDR6E5:
        *HwValue = AQDE_DEST_CONFIG_FORMAT_HDR6E5;
        break;
#endif

    default:
        /* Not supported. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* For PE 1.0, return error for formats not supported. */
    if (!Hardware->hw2DPE20)
    {
        /* Swizzled formats are not supported. */
        if ( (  *HwIsYUVValue && (*HwSwizzleValue != swizzle_uv) )
          || ( !*HwIsYUVValue && (*HwSwizzleValue != swizzle_argb) ))
        {
            gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
            return gcvSTATUS_NOT_SUPPORTED;
        }

        /* Newer added formats are not supported. */
        if ( 0
        || (*HwValue == 0x11)
        || (*HwValue == 0x12)
        || (*HwValue == 0x10)
        )
        {
            gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
            return gcvSTATUS_NOT_SUPPORTED;
        }

        /* YUV420 format are supported where available. */
        if(0
        || (*HwValue == 0xF)
        )
        {
            if (gcoHARDWARE_IsFeatureAvailable(Hardware,
                                               gcvFEATURE_YUV420_SCALER)
                != gcvSTATUS_TRUE)
            {
                gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
                return gcvSTATUS_NOT_SUPPORTED;
            }
        }
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_TranslateDestinationFormat
**
**  Translate API destination color format to its hardware value.
**
**  INPUT:
**
**      gceSURF_FORMAT APIValue
**          API value.
**
**  OUTPUT:
**
**      gctUINT32 * HwValue
**          Corresponding hardware value.
*/
gceSTATUS gcoHARDWARE_TranslateDestinationFormat(
    IN gcoHARDWARE Hardware,
    IN gceSURF_FORMAT APIValue,
    OUT gctUINT32* HwValue,
    OUT gctUINT32* HwSwizzleValue,
    OUT gctUINT32* HwIsYUVValue
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x APIValue=%d HwValue=0x%x "
                    "HwSwizzleValue=0x%x HwIsYUVValue=0x%x",
                    Hardware, APIValue, HwValue,
                    HwSwizzleValue, HwIsYUVValue);

    do
    {
        gcmERR_BREAK(gcoHARDWARE_TranslateSourceFormat(
            Hardware,
            APIValue,
            HwValue,
            HwSwizzleValue,
            HwIsYUVValue
            ));

        /* Check if format is supported as destination. */
        switch (*HwValue)
        {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x10:
            break;

        default:
            /* Not supported. */
            *HwValue = *HwSwizzleValue = *HwIsYUVValue = 0;
            gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
            return gcvSTATUS_NOT_SUPPORTED;
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_TranslatePatternFormat
**
**  Translate API pattern color format to its hardware value.
**
**  INPUT:
**
**      gceSURF_FORMAT APIValue
**          API value.
**
**  OUTPUT:
**
**      gctUINT32 * HwValue
**          Corresponding hardware value.
*/
gceSTATUS gcoHARDWARE_TranslatePatternFormat(
    IN gcoHARDWARE Hardware,
    IN gceSURF_FORMAT APIValue,
    OUT gctUINT32* HwValue,
    OUT gctUINT32* HwSwizzleValue,
    OUT gctUINT32* HwIsYUVValue
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x APIValue=%d HwValue=0x%x "
                    "HwSwizzleValue=0x%x HwIsYUVValue=0x%x",
                    Hardware, APIValue, HwValue,
                    HwSwizzleValue, HwIsYUVValue);

    do
    {
        gcmERR_BREAK(gcoHARDWARE_TranslateSourceFormat(
            Hardware,
            APIValue,
            HwValue,
            HwSwizzleValue,
            HwIsYUVValue
            ));

        /* Check if format is supported as pattern. */
        switch (*HwValue)
        {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4:
        case 0x5:
        case 0x6:
            break;

        default:
            /* Not supported. */
            *HwValue = *HwSwizzleValue = *HwIsYUVValue = 0;
            gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
            return gcvSTATUS_NOT_SUPPORTED;
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_TranslateTransparency
**
**  Translate API transparency mode to its hardware value.
**  Obsolete function for PE 2.0
**
**  INPUT:
**
**      gceSURF_TRANSPARENCY APIValue
**          API value.
**
**  OUTPUT:
**
**      gctUINT32 * HwValue
**          Corresponding hardware value.
*/
gceSTATUS gcoHARDWARE_TranslateTransparency(
    IN gceSURF_TRANSPARENCY APIValue,
    OUT gctUINT32* HwValue
    )
{
    gcmHEADER_ARG("APIValue=%d HwValue=0x%x", APIValue, HwValue);

    /* Dispatch on transparency. */
    switch (APIValue)
    {
    case gcvSURF_OPAQUE:
        *HwValue = 0x0;
        break;

    case gcvSURF_SOURCE_MATCH:
        *HwValue = 0x1;
        break;

    case gcvSURF_SOURCE_MASK:
        *HwValue = 0x2;
        break;

    case gcvSURF_PATTERN_MASK:
        *HwValue = 0x3;
        break;

    default:
        /* Not supported. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_TranslateSurfTransparency
**
**  Translate SURF API transparency mode to PE 2.0 transparency values.
**
**  INPUT:
**
**      gceSURF_TRANSPARENCY APIValue
**          API value.
**
**  OUTPUT:
**
**      gctUINT32 * HwValue
**          Corresponding hardware value.
*/
gceSTATUS gcoHARDWARE_TranslateSurfTransparency(
    IN gceSURF_TRANSPARENCY APIValue,
    OUT gctUINT32* SrcTransparency,
    OUT gctUINT32* DstTransparency,
    OUT gctUINT32* PatTransparency
    )
{
    gctUINT32 srcTransparency, patTransparency;

    gcmHEADER_ARG("APIValue=%d SrcTransparency=0x%x DstTransparency=0x%x PatTransparency=0x%x",
                    APIValue, SrcTransparency, DstTransparency, PatTransparency);

    /* Dispatch on transparency. */
    switch (APIValue)
    {
    case gcvSURF_OPAQUE:
        srcTransparency = gcv2D_OPAQUE;
        patTransparency = gcv2D_OPAQUE;
        break;

    case gcvSURF_SOURCE_MATCH:
        srcTransparency = gcv2D_KEYED;
        patTransparency = gcv2D_OPAQUE;
        break;

    case gcvSURF_SOURCE_MASK:
        srcTransparency = gcv2D_MASKED;
        patTransparency = gcv2D_OPAQUE;
        break;

    case gcvSURF_PATTERN_MASK:
        srcTransparency = gcv2D_OPAQUE;
        patTransparency = gcv2D_MASKED;
        break;

    default:
        /* Not supported. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    if ( SrcTransparency != gcvNULL )
    {
        *SrcTransparency = srcTransparency;
    }

    if ( DstTransparency != gcvNULL )
    {
        *DstTransparency = gcv2D_OPAQUE;
    }

    if ( PatTransparency != gcvNULL )
    {
        *PatTransparency = patTransparency;
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_TranslateTransparencies
**
**  Translate API transparency mode to its PE 1.0 hardware value.
**
**  INPUT:
**
**      gceSURF_TRANSPARENCY APIValue
**          API value.
**
**  OUTPUT:
**
**      gctUINT32 * HwValue
**          Corresponding hardware value.
*/
gceSTATUS gcoHARDWARE_TranslateTransparencies(
    IN gcoHARDWARE  Hardware,
    IN gctUINT32    srcTransparency,
    IN gctUINT32    dstTransparency,
    IN gctUINT32    patTransparency,
    OUT gctUINT32*  HwValue
    )
{
    gcmHEADER_ARG("Hardware=0x%x srcTransparency=%d dstTransparency=%d "
                    "patTransparency=%d HwValue=0x%x",
                    Hardware, srcTransparency, dstTransparency,
                    patTransparency, HwValue);

    /* Dispatch on transparency. */
    if (!Hardware->hw2DPE20)
    {
        if ((srcTransparency == gcv2D_OPAQUE)
        &&  (dstTransparency == gcv2D_OPAQUE)
        &&  (patTransparency == gcv2D_OPAQUE))
        {
            *HwValue = 0x0;
        }
        else
        if ((srcTransparency == gcv2D_KEYED)
        &&  (dstTransparency == gcv2D_OPAQUE)
        &&  (patTransparency == gcv2D_OPAQUE))
        {
            *HwValue = 0x1;
        }
        else
        if ((srcTransparency == gcv2D_MASKED)
        &&  (dstTransparency == gcv2D_OPAQUE)
        &&  (patTransparency == gcv2D_OPAQUE))
        {
            *HwValue = 0x2;
        }
        else
        if ((srcTransparency == gcv2D_OPAQUE)
        &&  (dstTransparency == gcv2D_OPAQUE)
        &&  (patTransparency == gcv2D_MASKED))
        {
            *HwValue = 0x3;
        }
        else
        {
            /* PE 2.0 feature requested on PE 1.0 hardware. */
            *HwValue = 0x0;
        }
    }
    else
    {
        *HwValue = 0x0;
    }

    gcmFOOTER_ARG("*HwValue=%d", *HwValue);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_TranslateSourceTransparency
**
**  Translate API transparency mode to its hardware value.
**
**  INPUT:
**
**      gce2D_TRANSPARENCY APIValue
**          API value.
**
**  OUTPUT:
**
**      gctUINT32 * HwValue
**          Corresponding hardware value.
*/
gceSTATUS gcoHARDWARE_TranslateSourceTransparency(
    IN gce2D_TRANSPARENCY APIValue,
    OUT gctUINT32 * HwValue
    )
{
    gcmHEADER_ARG("APIValue=%d HwValue=0x%x", APIValue, HwValue);

    /* Dispatch on transparency. */
    switch (APIValue)
    {
    case gcv2D_OPAQUE:
        *HwValue = 0x0;
        break;

    case gcv2D_KEYED:
        *HwValue = 0x2;
        break;

    case gcv2D_MASKED:
        *HwValue = 0x1;
        break;

    default:
        /* Not supported. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Success. */
    gcmFOOTER_ARG("*HwValue=%d", *HwValue);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_TranslateDestinationTransparency
**
**  Translate API transparency mode to its hardware value.
**  MASK transparency mode is reserved.
**
**  INPUT:
**
**      gce2D_TRANSPARENCY APIValue
**          API value.
**
**  OUTPUT:
**
**      gctUINT32 * HwValue
**          Corresponding hardware value.
*/
gceSTATUS gcoHARDWARE_TranslateDestinationTransparency(
    IN gce2D_TRANSPARENCY APIValue,
    OUT gctUINT32 * HwValue
    )
{
    gcmHEADER_ARG("APIValue=%d HwValue=0x%x", APIValue, HwValue);

    /* Dispatch on transparency. */
    switch (APIValue)
    {
    case gcv2D_OPAQUE:
        *HwValue = 0x0;
        break;

    case gcv2D_KEYED:
        *HwValue = 0x2;
        break;

    default:
        /* Not supported. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Success. */
    gcmFOOTER_ARG("*HwValue=%d", *HwValue);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_TranslatePatternTransparency
**
**  Translate API transparency mode to its hardware value.
**  KEY transparency mode is reserved.
**
**  INPUT:
**
**      gce2D_TRANSPARENCY APIValue
**          API value.
**
**  OUTPUT:
**
**      gctUINT32 * HwValue
**          Corresponding hardware value.
*/
gceSTATUS gcoHARDWARE_TranslatePatternTransparency(
    IN gce2D_TRANSPARENCY APIValue,
    OUT gctUINT32 * HwValue
    )
{
    gcmHEADER_ARG("APIValue=%d HwValue=0x%x", APIValue, HwValue);

    /* Dispatch on transparency. */
    switch (APIValue)
    {
    case gcv2D_OPAQUE:
        *HwValue = 0x0;
        break;

    case gcv2D_MASKED:
        *HwValue = 0x1;
        break;

    default:
        /* Not supported. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Success. */
    gcmFOOTER_ARG("*HwValue=%d", *HwValue);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_TranslateYUVColorMode
**
**  Translate API YUV Color mode to its hardware value.
**
**  INPUT:
**
**      gce2D_YUV_COLOR_MODE APIValue
**          API value.
**
**  OUTPUT:
**
**      gctUINT32 * HwValue
**          Corresponding hardware value.
*/
gceSTATUS gcoHARDWARE_TranslateYUVColorMode(
    IN  gce2D_YUV_COLOR_MODE APIValue,
    OUT gctUINT32 * HwValue
    )
{
    gcmHEADER_ARG("APIValue=%d HwValue=0x%x", APIValue, HwValue);

    /* Dispatch on transparency. */
    switch (APIValue)
    {
    case gcv2D_YUV_601:
        *HwValue = 0x0;
        break;

    case gcv2D_YUV_709:
        *HwValue = 0x1;
        break;

    default:
        /* Not supported. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Success. */
    gcmFOOTER_ARG("*HwValue=%d", *HwValue);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_PixelColorMultiplyMode
**
**  Translate API pixel color multiply mode to its hardware value.
**
**  INPUT:
**
**      gce2D_PIXEL_COLOR_MULTIPLY_MODE APIValue
**          API value.
**
**  OUTPUT:
**
**      gctUINT32 * HwValue
**          Corresponding hardware value.
*/
gceSTATUS gcoHARDWARE_PixelColorMultiplyMode(
    IN  gce2D_PIXEL_COLOR_MULTIPLY_MODE APIValue,
    OUT gctUINT32 * HwValue
    )
{
    gcmHEADER_ARG("APIValue=%d HwValue=0x%x", APIValue, HwValue);

    /* Dispatch on transparency. */
    switch (APIValue)
    {
    case gcv2D_COLOR_MULTIPLY_DISABLE:
        *HwValue = 0x0;
        break;

    case gcv2D_COLOR_MULTIPLY_ENABLE:
        *HwValue = 0x1;
        break;

    default:
        /* Not supported. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Success. */
    gcmFOOTER_ARG("*HwValue=%d", *HwValue);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_GlobalColorMultiplyMode
**
**  Translate API global color multiply mode to its hardware value.
**
**  INPUT:
**
**      gce2D_GLOBAL_COLOR_MULTIPLY_MODE APIValue
**          API value.
**
**  OUTPUT:
**
**      gctUINT32 * HwValue
**          Corresponding hardware value.
*/
gceSTATUS gcoHARDWARE_GlobalColorMultiplyMode(
    IN  gce2D_GLOBAL_COLOR_MULTIPLY_MODE APIValue,
    OUT gctUINT32 * HwValue
    )
{
    gcmHEADER_ARG("APIValue=%d HwValue=0x%x", APIValue, HwValue);

    /* Dispatch on transparency. */
    switch (APIValue)
    {
    case gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE:
        *HwValue = 0x0;
        break;

    case gcv2D_GLOBAL_COLOR_MULTIPLY_ALPHA:
        *HwValue = 0x1;
        break;

    case gcv2D_GLOBAL_COLOR_MULTIPLY_COLOR:
        *HwValue = 0x2;
        break;

    default:
        /* Not supported. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Success. */
    gcmFOOTER_ARG("*HwValue=%d", *HwValue);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_TranslateMonoPack
**
**  Translate API mono packing mode to its hardware value.
**
**  INPUT:
**
**      gceSURF_MONOPACK APIValue
**          API value.
**
**  OUTPUT:
**
**      gctUINT32 * HwValue
**          Corresponding hardware value.
*/
gceSTATUS gcoHARDWARE_TranslateMonoPack(
    IN gceSURF_MONOPACK APIValue,
    OUT gctUINT32 * HwValue
    )
{
    gcmHEADER_ARG("APIValue=%d HwValue=0x%x", APIValue, HwValue);

    /* Dispatch on monochrome packing. */
    switch (APIValue)
    {
    case gcvSURF_PACKED8:
        *HwValue = 0x0;
        break;

    case gcvSURF_PACKED16:
        *HwValue = 0x1;
        break;

    case gcvSURF_PACKED32:
        *HwValue = 0x2;
        break;

    case gcvSURF_UNPACKED:
        *HwValue = 0x3;
        break;

    default:
        /* Not supprted. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Success. */
    gcmFOOTER_ARG("*HwValue=%d", *HwValue);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_TranslateCommand
**
**  Translate API 2D command to its hardware value.
**
**  INPUT:
**
**      gce2D_COMMAND APIValue
**          API value.
**
**  OUTPUT:
**
**      gctUINT32 * HwValue
**          Corresponding hardware value.
*/
gceSTATUS gcoHARDWARE_TranslateCommand(
    IN gce2D_COMMAND APIValue,
    OUT gctUINT32 * HwValue
    )
{
    gcmHEADER_ARG("APIValue=%d HwValue=0x%x", APIValue, HwValue);

    /* Dispatch on command. */
    switch (APIValue)
    {
    case gcv2D_CLEAR:
        *HwValue = 0x0;
        break;

    case gcv2D_LINE:
        *HwValue = 0x1;
        break;

    case gcv2D_BLT:
        *HwValue = 0x2;
        break;

    case gcv2D_STRETCH:
        *HwValue = 0x4;
        break;

    case gcv2D_HOR_FILTER:
        *HwValue = 0x5;
        break;

    case gcv2D_VER_FILTER:
        *HwValue = 0x6;
        break;

    default:
        /* Not supported. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Success. */
    gcmFOOTER_ARG("*HwValue=%d", *HwValue);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_TranslatePixelAlphaMode
**
**  Translate API per-pixel alpha mode to its hardware value.
**
**  INPUT:
**
**      gceSURF_PIXEL_ALPHA_MODE APIValue
**          API value.
**
**  OUTPUT:
**
**      gctUINT32 * HwValue
**          Corresponding hardware value.
*/
gceSTATUS gcoHARDWARE_TranslatePixelAlphaMode(
    IN gceSURF_PIXEL_ALPHA_MODE APIValue,
    OUT gctUINT32* HwValue
    )
{
    gcmHEADER_ARG("APIValue=%d HwValue=0x%x", APIValue, HwValue);

    /* Dispatch on command. */
    switch (APIValue)
    {
    case gcvSURF_PIXEL_ALPHA_STRAIGHT:
        *HwValue = 0x0;
        break;

    case gcvSURF_PIXEL_ALPHA_INVERSED:
        *HwValue = 0x1;
        break;

    default:
        /* Not supported. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Success. */
    gcmFOOTER_ARG("*HwValue=%d", *HwValue);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_TranslateGlobalAlphaMode
**
**  Translate API global alpha mode to its hardware value.
**
**  INPUT:
**
**      gceSURF_GLOBAL_ALPHA_MODE APIValue
**          API value.
**
**  OUTPUT:
**
**      gctUINT32 * HwValue
**          Corresponding hardware value.
*/
gceSTATUS gcoHARDWARE_TranslateGlobalAlphaMode(
    IN gceSURF_GLOBAL_ALPHA_MODE APIValue,
    OUT gctUINT32* HwValue
    )
{
    gcmHEADER_ARG("APIValue=%d HwValue=0x%x", APIValue, HwValue);

    /* Dispatch on command. */
    switch (APIValue)
    {
    case gcvSURF_GLOBAL_ALPHA_OFF:
        *HwValue = 0x0;
        break;

    case gcvSURF_GLOBAL_ALPHA_ON:
        *HwValue = 0x1;
        break;

    case gcvSURF_GLOBAL_ALPHA_SCALE:
        *HwValue = 0x2;
        break;

    default:
        /* Not supported. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Success. */
    gcmFOOTER_ARG("*HwValue=%d", *HwValue);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_TranslatePixelColorMode
**
**  Translate API per-pixel color mode to its hardware value.
**
**  INPUT:
**
**      gceSURF_PIXEL_COLOR_MODE APIValue
**          API value.
**
**  OUTPUT:
**
**      gctUINT32 * HwValue
**          Corresponding hardware value.
*/
gceSTATUS gcoHARDWARE_TranslatePixelColorMode(
    IN gceSURF_PIXEL_COLOR_MODE APIValue,
    OUT gctUINT32* HwValue
    )
{
    gcmHEADER_ARG("APIValue=%d HwValue=0x%x", APIValue, HwValue);

    /* Dispatch on command. */
    switch (APIValue)
    {
    case gcvSURF_COLOR_STRAIGHT:
        *HwValue = 0x0;
        break;

    case gcvSURF_COLOR_MULTIPLY:
        *HwValue = 0x1;
        break;

    default:
        /* Not supported. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Success. */
    gcmFOOTER_ARG("*HwValue=%d", *HwValue);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_TranslatePixelMultiplyMode
**
**  Translate API per-pixel multiply mode to its hardware value.
**
**  INPUT:
**
**      gce2D_PIXEL_COLOR_MULTIPLY_MODE APIValue
**          API value.
**
**  OUTPUT:
**
**      gctUINT32 * HwValue
**          Corresponding hardware value.
*/
gceSTATUS gcoHARDWARE_TranslatePixelMultiplyMode(
    IN gce2D_PIXEL_COLOR_MULTIPLY_MODE APIValue,
    OUT gctUINT32* HwValue
    )
{
    gcmHEADER_ARG("APIValue=%d HwValue=0x%x", APIValue, HwValue);

    /* Dispatch on command. */
    switch (APIValue)
    {
    case gcv2D_COLOR_MULTIPLY_DISABLE:
        *HwValue = 0x0;
        break;

    case gcv2D_COLOR_MULTIPLY_ENABLE:
        *HwValue = 0x1;
        break;

    default:
        /* Not supported. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Success. */
    gcmFOOTER_ARG("*HwValue=%d", *HwValue);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_TranslateAlphaFactorMode
**
**  Translate API alpha factor source mode to its hardware value.
**
**  INPUT:
**
**      gceSURF_BLEND_FACTOR_MODE APIValue
**          API value.
**
**  OUTPUT:
**
**      gctUINT32 * HwValue
**          Corresponding hardware value.
*/
gceSTATUS gcoHARDWARE_TranslateAlphaFactorMode(
    IN  gcoHARDWARE               Hardware,
    IN  gceSURF_BLEND_FACTOR_MODE APIValue,
    OUT gctUINT32*                HwValue
    )
{
    gcmHEADER_ARG("Hardware=0x%x APIValue=%d HwValue=0x%x", Hardware, APIValue, HwValue);

    /* Dispatch on command. */
    switch (APIValue)
    {
    case gcvSURF_BLEND_ZERO:
        *HwValue = 0x0;
        break;

    case gcvSURF_BLEND_ONE:
        *HwValue = 0x1;
        break;

    case gcvSURF_BLEND_STRAIGHT:
        *HwValue = 0x2;
        break;

    case gcvSURF_BLEND_INVERSED:
        *HwValue = 0x3;
        break;

    case gcvSURF_BLEND_COLOR:
        if ( !Hardware->hw2DPE20 )
        {
            gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
            return gcvSTATUS_NOT_SUPPORTED;
        }

        *HwValue = 0x4;
        break;

    case gcvSURF_BLEND_COLOR_INVERSED:
        if ( !Hardware->hw2DPE20 )
        {
            gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
            return gcvSTATUS_NOT_SUPPORTED;
        }

        *HwValue = 0x5;
        break;

    case gcvSURF_BLEND_SRC_ALPHA_SATURATED:
        if ( !Hardware->hw2DPE20 )
        {
            gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
            return gcvSTATUS_NOT_SUPPORTED;
        }

        *HwValue = 0x6;
        break;

    default:
        /* Not supported. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Success. */
    gcmFOOTER_ARG("*HwValue=%d", *HwValue);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_GetStretchFactor
**
**  Calculate stretch factor.
**
**  INPUT:
**
**      gctINT32 SrcSize
**          The size in pixels of a source dimension (width or height).
**
**      gctINT32 DestSize
**          The size in pixels of a destination dimension (width or height).
**
**  OUTPUT:
**
**      Nothing.
**
**  RETURN:
**
**      gctUINT32
**          Stretch factor in 16.16 fixed point format.
*/
gctUINT32 gcoHARDWARE_GetStretchFactor(
    IN gctINT32 SrcSize,
    IN gctINT32 DestSize
    )
{
    gctUINT stretchFactor;

    gcmHEADER_ARG("SrcSize=%d DestSize=%d", SrcSize, DestSize);

    if ( (SrcSize > 0) && (DestSize > 1) )
    {
        stretchFactor = ((SrcSize - 1) << 16) / (DestSize - 1);
    }
    else
    {
        stretchFactor = 0;
    }

    gcmFOOTER_ARG("return=%d", stretchFactor);
    return stretchFactor;
}

/*******************************************************************************
**
**  gcoHARDWARE_GetStretchFactors
**
**  Calculate the stretch factors.
**
**  INPUT:
**
**      gcsRECT_PTR SrcRect
**          Pointer to a valid source rectangles.
**
**      gcsRECT_PTR DestRect
**          Pointer to a valid destination rectangles.
**
**  OUTPUT:
**
**      gctUINT32 * HorFactor
**          Pointer to a variable that will receive the horizontal stretch
**          factor.
**
**      gctUINT32 * VerFactor
**          Pointer to a variable that will receive the vertical stretch factor.
*/
gceSTATUS gcoHARDWARE_GetStretchFactors(
    IN gcsRECT_PTR SrcRect,
    IN gcsRECT_PTR DestRect,
    OUT gctUINT32 * HorFactor,
    OUT gctUINT32 * VerFactor
    )
{
    gcmHEADER_ARG("SrcRect=0x%x DestRect=0x%x HorFactor=0x%x VerFactor=0x%x",
                    SrcRect, DestRect, HorFactor, VerFactor);

    if (HorFactor != gcvNULL)
    {
        gctINT32 src, dest;

        /* Compute width of rectangles. */
        gcmVERIFY_OK(gcsRECT_Width(SrcRect, &src));
        gcmVERIFY_OK(gcsRECT_Width(DestRect, &dest));

        /* Compute and return horizontal stretch factor. */
        *HorFactor = gcoHARDWARE_GetStretchFactor(src, dest);
    }

    if (VerFactor != gcvNULL)
    {
        gctINT32 src, dest;

        /* Compute height of rectangles. */
        gcmVERIFY_OK(gcsRECT_Height(SrcRect, &src));
        gcmVERIFY_OK(gcsRECT_Height(DestRect, &dest));

        /* Compute and return vertical stretch factor. */
        *VerFactor = gcoHARDWARE_GetStretchFactor(src, dest);
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_SetStretchFactors
**
**  Calculate and program the stretch factors.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT32 HorFactor
**          Horizontal stretch factor.
**
**      gctUINT32 VerFactor
**          Vertical stretch factor.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gcoHARDWARE_SetStretchFactors(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 HorFactor,
    IN gctUINT32 VerFactor
    )
{
    gceSTATUS status;
    gctUINT32 memory[2];

    gcmHEADER_ARG("Hardware=0x%x HorFactor=%d VerFactor=%d",
                    Hardware, HorFactor, VerFactor);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* SelectPipe(2D) */
    status = gcoHARDWARE_SelectPipe(Hardware, 0x1);

    if (status != gcvSTATUS_OK)
    {
        /* Error. */
        gcmFOOTER();
        return status;
    }

    /* Set states into temporary buffer. */
    memory[0] = HorFactor;
    memory[1] = VerFactor;

    /* Through load state command. */
    status = gcoHARDWARE_LoadState(Hardware, 0x01220, 2,
                                memory);

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_EnableAlphaBlend
**
**  Enable alpha blending engine in the hardware and disengage the ROP engine.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT8 SrcGlobalAlphaValue
**      gctUINT8 DstGlobalAlphaValue
**          Global alpha value for the color components.
**
**      gceSURF_PIXEL_ALPHA_MODE SrcAlphaMode
**      gceSURF_PIXEL_ALPHA_MODE DstAlphaMode
**          Per-pixel alpha component mode.
**
**      gceSURF_GLOBAL_ALPHA_MODE SrcGlobalAlphaMode
**      gceSURF_GLOBAL_ALPHA_MODE DstGlobalAlphaMode
**          Global/per-pixel alpha values selection.
**
**      gceSURF_BLEND_FACTOR_MODE SrcFactorMode
**      gceSURF_BLEND_FACTOR_MODE DstFactorMode
**          Final blending factor mode.
**
**      gceSURF_PIXEL_COLOR_MODE SrcColorMode
**      gceSURF_PIXEL_COLOR_MODE DstColorMode
**          Per-pixel color component mode.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gcoHARDWARE_EnableAlphaBlend(
    IN gcoHARDWARE Hardware,
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

    /* Define hardware components. */
    gctUINT32 srcAlphaMode, srcGlobalAlphaMode, srcFactorMode, srcColorMode;
    gctUINT32 dstAlphaMode, dstGlobalAlphaMode, dstFactorMode, dstColorMode;
    gctUINT8 srcGlobalAlphaValue, dstGlobalAlphaValue;

    /* State array. */
    gctUINT32 states[2];

    gcmHEADER_ARG("Hardware=0x%x SrcAlphaMode=%d DstAlphaMode=%d SrcGlobalAlphaMode=%d "
              "DstGlobalAlphaMode=%d SrcFactorMode=%d DstFactorMode=%d "
              "SrcColorMode=%d DstColorMode=%d",
              Hardware, SrcAlphaMode, DstAlphaMode, SrcGlobalAlphaMode,
              DstGlobalAlphaMode, SrcFactorMode, DstFactorMode,
              SrcColorMode, DstColorMode);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    do
    {
        /* Translate inputs. */
        gcmERR_BREAK(gcoHARDWARE_TranslatePixelAlphaMode(
            SrcAlphaMode, &srcAlphaMode
            ));

        gcmERR_BREAK(gcoHARDWARE_TranslatePixelAlphaMode(
            DstAlphaMode, &dstAlphaMode
            ));

        gcmERR_BREAK(gcoHARDWARE_TranslateGlobalAlphaMode(
            SrcGlobalAlphaMode, &srcGlobalAlphaMode
            ));

        gcmERR_BREAK(gcoHARDWARE_TranslateGlobalAlphaMode(
            DstGlobalAlphaMode, &dstGlobalAlphaMode
            ));

        gcmERR_BREAK(gcoHARDWARE_TranslateAlphaFactorMode(
            Hardware, SrcFactorMode, &srcFactorMode
            ));

        gcmERR_BREAK(gcoHARDWARE_TranslateAlphaFactorMode(
            Hardware, DstFactorMode, &dstFactorMode
            ));

        gcmERR_BREAK(gcoHARDWARE_TranslatePixelColorMode(
            SrcColorMode, &srcColorMode
            ));

        gcmERR_BREAK(gcoHARDWARE_TranslatePixelColorMode(
            DstColorMode, &dstColorMode
            ));

        /* SelectPipe(2D). */
        gcmERR_BREAK(gcoHARDWARE_SelectPipe(Hardware, 0x1));

        if (Hardware->hw2DPE20)
        {
            /* Color modes are not directly supported in new PE.
               User should use Premultiply modes instead.
               Driver using premultiply modes for old PE where possible. */
            if ((srcColorMode == gcvSURF_COLOR_MULTIPLY)
              ||(dstColorMode == gcvSURF_COLOR_MULTIPLY))
            {
                gce2D_PIXEL_COLOR_MULTIPLY_MODE srcPremultiply = gcv2D_COLOR_MULTIPLY_DISABLE;
                gce2D_PIXEL_COLOR_MULTIPLY_MODE dstPremultiply = gcv2D_COLOR_MULTIPLY_DISABLE;
                gce2D_GLOBAL_COLOR_MULTIPLY_MODE srcPremultiplyGlobal = gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE;

                if (srcColorMode == gcvSURF_COLOR_MULTIPLY)
                {
                    if (srcAlphaMode != gcvSURF_PIXEL_ALPHA_STRAIGHT)
                    {
                        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
                        return gcvSTATUS_NOT_SUPPORTED;
                    }

                    if ((srcGlobalAlphaMode == gcvSURF_GLOBAL_ALPHA_OFF)
                      ||(srcGlobalAlphaMode == gcvSURF_GLOBAL_ALPHA_SCALE))
                    {
                        srcPremultiply = gcv2D_COLOR_MULTIPLY_ENABLE;
                    }

                    if ((srcGlobalAlphaMode == gcvSURF_GLOBAL_ALPHA_ON)
                      ||(srcGlobalAlphaMode == gcvSURF_GLOBAL_ALPHA_SCALE))
                    {
                        srcPremultiplyGlobal = gcv2D_GLOBAL_COLOR_MULTIPLY_ALPHA;
                    }
                }
                if (dstColorMode == gcvSURF_COLOR_MULTIPLY)
                {
                    if (dstAlphaMode != gcvSURF_PIXEL_ALPHA_STRAIGHT)
                    {
                        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
                        return gcvSTATUS_NOT_SUPPORTED;
                    }

                    if (dstGlobalAlphaMode == gcvSURF_GLOBAL_ALPHA_OFF)
                    {
                        dstPremultiply = gcv2D_COLOR_MULTIPLY_ENABLE;
                    }
                    else
                    {
                        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
                        return gcvSTATUS_NOT_SUPPORTED;
                    }
                }

                /* Program equivalent premultiply modes. */
                gcmERR_BREAK(gcoHARDWARE_SetMultiplyModes(Hardware,
                    srcPremultiply,
                    dstPremultiply,
                    srcPremultiplyGlobal,
                    gcv2D_COLOR_MULTIPLY_DISABLE));
            }
        }

        srcGlobalAlphaValue = (gctUINT8)(Hardware->globalSrcColor >> 24);
        dstGlobalAlphaValue = (gctUINT8)(Hardware->globalTargetColor >> 24);

        /*
            Fill in the states.
        */

        /* Enable alpha blending and set global alpha values. */
        states[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:16) - (0 ? 23:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:16) - (0 ? 23:16) + 1))))))) << (0 ? 23:16))) | (((gctUINT32) ((gctUINT32) (srcGlobalAlphaValue) & ((gctUINT32) ((((1 ? 23:16) - (0 ? 23:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:16) - (0 ? 23:16) + 1))))))) << (0 ? 23:16)))

                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:24) - (0 ? 31:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ? 31:24))) | (((gctUINT32) ((gctUINT32) (dstGlobalAlphaValue) & ((gctUINT32) ((((1 ? 31:24) - (0 ? 31:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ? 31:24)));

        /* Set alpha blending modes. */
        states[1] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) | (((gctUINT32) ((gctUINT32) (srcAlphaMode) & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))

                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) | (((gctUINT32) ((gctUINT32) (dstAlphaMode) & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))

                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8))) | (((gctUINT32) ((gctUINT32) (srcGlobalAlphaMode) & ((gctUINT32) ((((1 ? 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8)))

                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 13:12) - (0 ? 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12))) | (((gctUINT32) ((gctUINT32) (dstGlobalAlphaMode) & ((gctUINT32) ((((1 ? 13:12) - (0 ? 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12)))

                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))) | (((gctUINT32) ((gctUINT32) (srcColorMode) & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)))

                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ? 20:20))) | (((gctUINT32) ((gctUINT32) (dstColorMode) & ((gctUINT32) ((((1 ? 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ? 20:20)))

                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 26:24) - (0 ? 26:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24))) | (((gctUINT32) ((gctUINT32) (srcFactorMode) & ((gctUINT32) ((((1 ? 26:24) - (0 ? 26:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))

                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28))) | (((gctUINT32) ((gctUINT32) (dstFactorMode) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));

        /* LoadState(AQDE_ALPHA_CONTROL, 2), states. */
        gcmERR_BREAK(gcoHARDWARE_LoadState(Hardware, 0x0127C,
                                       2, states));
    }
    while (gcvFALSE);

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_DisableAlphaBlend
**
**  Disable alpha blending engine in the hardware and engage the ROP engine.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gcoHARDWARE_DisableAlphaBlend(
    IN gcoHARDWARE Hardware
    )
{
    gceSTATUS status;
    gctUINT32 data;

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    do
    {
        /* SelectPipe(2D). */
        gcmERR_BREAK(gcoHARDWARE_SelectPipe(Hardware, 0x1));

        /* Disable alpha blending. */
        data = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        /* LoadState(AQDE_ALPHA_CONTROL, AlphaOff). */
        gcmERR_BREAK(gcoHARDWARE_LoadState32(Hardware, 0x0127C,
                                         data));
    }
    while (gcvFALSE);

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_LoadState
**
**  Load a state buffer.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT32 Address
**          Starting register address of the state buffer.
**
**      gctUINT32 Count
**          Number of states in state buffer.
**
**      gctPOINTER Data
**          Pointer to state buffer.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_LoadState(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctSIZE_T Count,
    IN gctPOINTER Data
    )
{
    gctUINT32_PTR memory;
    gceSTATUS status;
    gctBOOL_PTR hints = gcvNULL;

    gcmHEADER_ARG("Hardware=0x%x Address=%x Count=%d Data=0x%x",
                    Hardware, Address, Count, Data);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

#if gcdSECURE_USER
    /* Allocate memory for the hints. */
    gcmONERROR(
        gcoOS_Allocate(Hardware->os,
                       (1 + Count) * gcmSIZEOF(gctBOOL),
                       (gctPOINTER *) &hints));
#endif

    /* Buffer the states. */
    gcmONERROR(
        gcoCONTEXT_Buffer(Hardware->context,
                          Address,
                          Count,
                          Data,
                          hints));

    /* Reserve space in the command buffer. */
    gcmONERROR(
        gcoBUFFER_Reserve(Hardware->buffer,
                          (1 + Count) * 4,
                          gcvTRUE,
                          hints,
                          (gctPOINTER *) &memory));

    /* LOAD_STATE(Count,Address>>2)*/
    memory[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (Count) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (Address>>2) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

    /* Copy the state buffer. */
    gcmVERIFY_OK(
        gcoOS_MemCopy(&memory[1], Data, Count * 4));

#if gcdSECURE_USER
    /* Free the hints. */
    gcmVERIFY_OK(gcoOS_Free(Hardware->os, hints));
#endif

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (hints != gcvNULL)
    {
        /* Roll back the hint allocation. */
        gcmVERIFY_OK(gcoOS_Free(Hardware->os, hints));
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_LoadStateF
**
**  Load a state buffer with floating point states.  The states are meant for
**  the 3D pipe.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT32 Address
**          Starting register address of the state buffer.
**
**      gctUINT32 Count
**          Number of states in state buffer.
**
**      gctPOINTER Data
**          Pointer to state buffer.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_LoadStateF(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctSIZE_T Count,
    IN gctPOINTER Data
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Address=%x Count=%d Data=0x%x",
                    Hardware, Address, Count, Data);

    /* Switch to the 3D pipe. */
    gcmONERROR(
        gcoHARDWARE_SelectPipe(Hardware, 0x0));

    /* Call LoadState function. */
    gcmONERROR
        (gcoHARDWARE_LoadState(Hardware, Address, Count, Data));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_LoadStateX
**
**  Load a state buffer with fixed point states.  The states are meant for the
**  3D pipe.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT32 Address
**          Starting register address of the state buffer.
**
**      gctUINT32 Count
**          Number of states in state buffer.
**
**      gctPOINTER Data
**          Pointer to state buffer.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_LoadStateX(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctSIZE_T Count,
    IN gctPOINTER Data
    )
{
    gctUINT32_PTR memory;
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Address=%x Count=%d Data=0x%x",
                    Hardware, Address, Count, Data);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Switch to 3D pipe. */
    gcmONERROR(
        gcoHARDWARE_SelectPipe(Hardware, 0x0));

    /* Buffer the states. */
    gcmONERROR(
        gcoCONTEXT_BufferX(Hardware->context,
                           Address,
                           Count,
                           Data));

    /* Reserve space in the command buffer. */
    gcmONERROR(
        gcoBUFFER_Reserve(Hardware->buffer,
                          (1 + Count) * 4,
                          gcvTRUE,
                          gcvNULL,
                          (gctPOINTER *) &memory));

    /* LOAD_STATE(Count,Address>>2)*/
    memory[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (Count) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (Address>>2) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

    /* Copy the state buffer. */
    gcmVERIFY_OK(gcoOS_MemCopy(&memory[1], Data, Count * 4));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_LoadState32
**
**  Load one 32-bit state.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT32 Address
**          Register address of the state.
**
**      gctUINT32 Data
**          Value of the state.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_LoadState32(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctUINT32 Data
    )
{
    gceSTATUS status;
    gctUINT32_PTR memory;
    gctBOOL hints[2];

    gcmHEADER_ARG("Hardware=0x%x Address=0x%05x Data=0x%08x",
                  Hardware, Address, Data);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Buffer the state. */
    gcmONERROR(
        gcoCONTEXT_Buffer(Hardware->context,
                          Address,
                          1,
                          &Data,
                          hints));

    /* Reserve 8 bytes in the command buffer. */
    gcmONERROR(
        gcoBUFFER_Reserve(Hardware->buffer,
                          8,
                          gcvTRUE,
                          hints,
                          (gctPOINTER *) &memory));

    /* LOAD_STATE(1,Address>>2),Data*/
    memory[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (Address>>2) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

    memory[1] = Data;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_LoadState32x
**
**  Load one 32-bit state, which is represented in 16.16 fixed point.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT32 Address
**          Register address of the state.
**
**      gctFIXED_POINT Data
**          Value of the state in 16.16 fixed point format.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_LoadState32x(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctFIXED_POINT Data
    )
{
    gceSTATUS status;
    gctUINT32_PTR memory;

    gcmHEADER_ARG("Hardware=0x%x Address=0x%05x Data=%f",
                  Hardware, Address, Data / 65536.0f);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Buffer the state. */
    gcmONERROR(
        gcoCONTEXT_BufferX(Hardware->context,
                           Address,
                           1,
                           &Data));

    /* Reserve 8 bytes in the command buffer. */
    gcmONERROR(
        gcoBUFFER_Reserve(Hardware->buffer,
                          8,
                          gcvTRUE,
                          gcvNULL,
                          (gctPOINTER *) &memory));

    /* LOAD_STATE(1,Address>>2),Data*/
    memory[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (Address>>2) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

    memory[1] = Data;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_LoadState64
**
**  Load one 64-bit state.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctUINT32 Address
**          Register address of the state.
**
**      gctUINT64 Data
**          Value of the state.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_LoadState64(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctUINT64 Data
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Address=0x%05x Data=%016llx",
                  Hardware, Address, Data);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Call buffered load state to do it. */
    status = gcoHARDWARE_LoadState(Hardware, Address, 2, &Data);

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_Commit
**
**  Commit the current command buffer to the hardware.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_Commit(
    IN gcoHARDWARE Hardware
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Dump the commit. */
    gcmDUMP(Hardware->os, "@[commit]");

    do
    {
        /* Precommit the gcoCONTEXT. */
        gcmERR_BREAK(gcoCONTEXT_PreCommit(Hardware->context));

        /* Commit command buffer and return status. */
        gcmERR_BREAK(gcoBUFFER_Commit(Hardware->buffer,
                                      Hardware->context,
                                      Hardware->queue));
    }
    while (gcvFALSE);

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_Stall
**
**  Stall the thread until the hardware is finished.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to a gcoHARDWARE object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_Stall(
    IN gcoHARDWARE Hardware
    )
{
    gcsHAL_INTERFACE iface;
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Dump the stall. */
    gcmDUMP(Hardware->os, "@[stall]");

    /* Create a signal event. */
    iface.command            = gcvHAL_SIGNAL;
    iface.u.Signal.signal    = Hardware->stallSignal;
    iface.u.Signal.auxSignal = gcvNULL;
    iface.u.Signal.process   = Hardware->hal->process;
    iface.u.Signal.fromWhere = gcvKERNEL_PIXEL;

    /* Send the event. */
    gcmONERROR(gcoHARDWARE_CallEvent(Hardware, &iface));

    /* Commit the event queue. */
    gcmONERROR(gcoQUEUE_Commit(Hardware->queue));

    /* Wait for the signal. */
    gcmONERROR(
        gcoOS_WaitSignal(Hardware->os, Hardware->stallSignal, gcvINFINITE));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  _ConvertResolveFormat
**
**  Converts HAL resolve format into its hardware equivalent.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to a gcoHARDWARE object.
**
**      gceSURF_FORMAT Format
**          HAL format value.
**
**  OUTPUT:
**
**      gctUINT32 * HardwareFormat
**          Hardware format value.
**
**      gctBOOL * Flip
**          RGB component flip flag.
*/
static gceSTATUS _ConvertResolveFormat(
    IN gcoHARDWARE Hardware,
    IN gceSURF_FORMAT Format,
    OUT gctUINT32 * HardwareFormat,
    OUT gctBOOL * Flip
    )
{
    gctUINT32 format;
    gctBOOL flip;

    switch (Format)
    {
    case gcvSURF_X4R4G4B4:
        format = 0x0;
        flip   = gcvFALSE;
        break;

    case gcvSURF_A4R4G4B4:
        format = 0x1;
        flip   = gcvFALSE;
        break;

    case gcvSURF_X1R5G5B5:
        format = 0x2;
        flip   = gcvFALSE;
        break;

    case gcvSURF_A1R5G5B5:
        format = 0x3;
        flip   = gcvFALSE;
        break;

    case gcvSURF_R5G6B5:
        format = 0x4;
        flip   = gcvFALSE;
        break;

    case gcvSURF_X8R8G8B8:
        format = 0x5;
        flip   = gcvFALSE;
        break;

    case gcvSURF_A8R8G8B8:
        format = 0x6;
        flip   = gcvFALSE;
        break;

    case gcvSURF_X8B8G8R8:
        format = 0x5;
        flip   = gcvTRUE;
        break;

    case gcvSURF_A8B8G8R8:
        format = 0x6;
        flip   = gcvTRUE;
        break;

    case gcvSURF_YUY2:
        if (((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 21:21) & ((gctUINT32) ((((1 ? 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1))))))))
        {
            format = 0x7;
            flip   = gcvFALSE;
            break;
        }

    /* Fake 16-bit formats. */
    case gcvSURF_D16:
    case gcvSURF_UYVY:
        format = 0x1;
        flip   = gcvFALSE;
        break;

    /* Fake 32-bit formats. */
    case gcvSURF_D24S8:
    case gcvSURF_D24X8:
    case gcvSURF_D32:
        format = 0x6;
        flip   = gcvFALSE;
        break;

    default:
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    *HardwareFormat = format;

    if (Flip != gcvNULL)
    {
        *Flip = flip;
    }

    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  _AlignResolveRect
**
**  Align the specified rectangle to the resolve block requirements.
**
**  INPUT:
**
**      gcsPOINT_PTR RectOrigin
**          Unaligned origin of the rectangle.
**
**      gcsPOINT_PTR RectSize
**          Unaligned size of the rectangle.
**
**  OUTPUT:
**
**      gcsPOINT_PTR AlignedOrigin
**          Resolve-aligned origin of the rectangle.
**
**      gcsPOINT_PTR AlignedSize
**          Resolve-aligned size of the rectangle.
*/
static void _AlignResolveRect(
    IN gcsPOINT_PTR RectOrigin,
    IN gcsPOINT_PTR RectSize,
    OUT gcsPOINT_PTR AlignedOrigin,
    OUT gcsPOINT_PTR AlignedSize
    )
{
    /* Determine region's right and bottom coordinates. */
    gctINT32 right  = RectOrigin->x + RectSize->x;
    gctINT32 bottom = RectOrigin->y + RectSize->y;

    /* Determine the outside or "external" coordinates aligned for resolve
       to completely cover the requested rectangle. */
    AlignedOrigin->x = RectOrigin->x & ~3;
    AlignedOrigin->y = RectOrigin->y & ~3;
    AlignedSize->x   = gcmALIGN(right  - AlignedOrigin->x, 16);
    AlignedSize->y   = gcmALIGN(bottom - AlignedOrigin->y,  4);
}

/*******************************************************************************
**
**  _ComputePixelLocation
**
**  Compute the offset of the specified pixel and determine its format.
**
**  INPUT:
**
**      gctUINT X, Y
**          Pixel coordinates.
**
**      gctUINT Stride
**          Surface stride.
**
**      gcsSURF_FORMAT_INFO_PTR * Format
**          Pointer to the pixel format (even/odd pair).
**
**      gctBOOL Tiled
**          Surface tiled vs. linear flag.
**
**  OUTPUT:
**
**      gctUINT32 PixelOffset
**          Offset of the pixel from the beginning of the surface.
**
**      gcsSURF_FORMAT_INFO_PTR * PixelFormat
**          Specific pixel format of this pixel.
*/
static void _ComputePixelLocation(
    IN gctUINT X,
    IN gctUINT Y,
    IN gctUINT Stride,
    IN gcsSURF_FORMAT_INFO_PTR * Format,
    IN gctBOOL Tiled,
    OUT gctUINT32_PTR PixelOffset,
    OUT gcsSURF_FORMAT_INFO_PTR * PixelFormat
    )
{
    gctUINT8 bitsPerPixel = Format[0]->bitsPerPixel;

    if (Format[0]->interleaved)
    {
        /* Determine whether the pixel is at even or odd location. */
        gctUINT oddPixel = X & 1;

        /* Force to the even location for interleaved pixels. */
        X &= ~1;

        /* Pick the proper format. */
        *PixelFormat = Format[oddPixel];
    }
    else
    {
        *PixelFormat = Format[0];
    }

    if (Tiled)
    {
        *PixelOffset
            = (Y & ~3) * Stride
            + (X & ~3) * bitsPerPixel / 2
            + (Y &  3) * bitsPerPixel / 2
            + (X &  3) * bitsPerPixel / 8;
    }
    else
    {
        *PixelOffset
            = Y * Stride
            + X * bitsPerPixel / 8;
    }
}

/*******************************************************************************
**
**  _BitBlitCopy
**
**  Make a copy of the specified rectangular area using 2D hardware.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcsSURF_INFO_PTR SrcInfo
**          Pointer to the source surface descriptor.
**
**      gcsSURF_INFO_PTR DestInfo
**          Pointer to the destination surface descriptor.
**
**      gcsPOINT_PTR SrcOrigin
**          The origin of the source area to be copied.
**
**      gcsPOINT_PTR DestOrigin
**          The origin of the destination area to be copied.
**
**      gcsPOINT_PTR RectSize
**          The size of the rectangular area to be copied.
**
**  OUTPUT:
**
**      Nothing.
*/
static gceSTATUS
_BitBlitCopy(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_INFO_PTR SrcInfo,
    IN gcsSURF_INFO_PTR DestInfo,
    IN gcsPOINT_PTR SrcOrigin,
    IN gcsPOINT_PTR DestOrigin,
    IN gcsPOINT_PTR RectSize
    )
{
    gceSTATUS status;

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    do
    {
        gctUINT32 format, swizzle, isYUVformat, data[6];
        gctINT destRight, destBottom;
        gctUINT32* memory;

        /* Verify that the surfaces are locked. */
        gcmVERIFY_LOCK(SrcInfo);
        gcmVERIFY_LOCK(DestInfo);

        /* Select 2D pipe. */
        gcmERR_BREAK(gcoHARDWARE_SelectPipe(
            Hardware,
            0x1
            ));

        /***********************************************************************
        ** Setup source.
        */

        /* AQDESrcAddress. */
        data[0] = SrcInfo->node.physical;

        /* AQDESrcStride. */
        data[1] = SrcInfo->stride;

        /* AQDESrcRotationConfig. */
        data[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));

        /* Convert source format. */
        gcmERR_BREAK(gcoHARDWARE_TranslateSourceFormat(
            Hardware, SrcInfo->format, &format, &swizzle, &isYUVformat
            ));

        /* AQDESrcConfig. */
        data[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)))|
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)))|
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))|
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 28:24) - (0 ? 28:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:24) - (0 ? 28:24) + 1))))))) << (0 ? 28:24))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 28:24) - (0 ? 28:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:24) - (0 ? 28:24) + 1))))))) << (0 ? 28:24)))|
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));

        /* AQDESrcOrigin. */
        data[4] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (SrcOrigin->x) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))|
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (SrcOrigin->y) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

        /* AQDESrcSize. */
        data[5] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (RectSize->x) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))|
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (RectSize->y) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

        /* Program the states. */
        gcmERR_BREAK(gcoHARDWARE_LoadState(
            Hardware, 0x01200, 6, data
            ));

        /***********************************************************************
        ** Setup destination.
        */

        /* AQDEDestAddress. */
        data[0] = DestInfo->node.physical;

        /* AQDEDestStride. */
        data[1] = DestInfo->stride;

        /* AQDEDestRotationConfig. */
        data[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));

        /* Convert destination format. */
        gcmERR_BREAK(
            gcoHARDWARE_TranslateDestinationFormat(Hardware,
                                                   DestInfo->format,
                                                   &format,
                                                   &swizzle,
                                                   &isYUVformat));

        /* AQDEDestConfig. */
        data[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))

                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12)));

        /* Program the states. */
        gcmERR_BREAK(
            gcoHARDWARE_LoadState(Hardware,
                                  0x01228,
                                  4,
                                  data));

        destRight  = DestOrigin->x + RectSize->x;
        destBottom = DestOrigin->y + RectSize->y;

        /***********************************************************************
        ** Setup clipping window.
        */
        /* 0x01260 */
        data[0]
            = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ? 14:0))) | (((gctUINT32) ((gctUINT32) (DestOrigin->x) & ((gctUINT32) ((((1 ? 14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ? 14:0)))

            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 30:16) - (0 ? 30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ? 30:16))) | (((gctUINT32) ((gctUINT32) (DestOrigin->y) & ((gctUINT32) ((((1 ? 30:16) - (0 ? 30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ? 30:16)));

        /* 0x01264 */
        data[1]
            = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ? 14:0))) | (((gctUINT32) ((gctUINT32) (destRight) & ((gctUINT32) ((((1 ? 14:0) - (0 ? 14:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ? 14:0)))

            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 30:16) - (0 ? 30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ? 30:16))) | (((gctUINT32) ((gctUINT32) (destBottom) & ((gctUINT32) ((((1 ? 30:16) - (0 ? 30:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ? 30:16)));

        /* Load cllipping states. */
        gcmERR_BREAK(gcoHARDWARE_LoadState(
            Hardware, 0x01260, 2,
            data
            ));

        /***********************************************************************
        ** Blit the data.
        */

        /* AQDERop. */
        data[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 21:20) - (0 ? 21:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0))) | (((gctUINT32) ((gctUINT32) (0xCC) & ((gctUINT32) ((((1 ? 7:0) - (0 ? 7:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0)));

        gcmERR_BREAK(
            gcoHARDWARE_LoadState32(Hardware, 0x0125C, data[0]));

        /* Reserve space for the command. */
        gcmERR_BREAK(
            gcoBUFFER_Reserve(Hardware->buffer,
                              4 * gcmSIZEOF(gctUINT32),
                              gcvTRUE,
                              gcvNULL,
                              (gctPOINTER *) &memory));

        /* StartDE(1, 0). */
        memory[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x04 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8)))

                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 26:16) - (0 ? 26:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:16) - (0 ? 26:16) + 1))))))) << (0 ? 26:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 26:16) - (0 ? 26:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:16) - (0 ? 26:16) + 1))))))) << (0 ? 26:16)));

        /* DestRectangle. */
        memory[2] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (DestOrigin->x) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (DestOrigin->y) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

        memory[3] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (destRight) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (destBottom) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

        gcmDUMP(Hardware->os, "@[prim2d 1 0x00000000");
        gcmDUMP(Hardware->os,
                "  %d,%d %d,%d",
                DestOrigin->x, DestOrigin->y, destRight, destBottom);
        gcmDUMP(Hardware->os, "] -- prim2d");

        /* Flush the 2D cache. */
        data[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3))) | (((gctUINT32) (0x1&((gctUINT32)((((1?3:3)-(0?3:3)+1)==32)?~0:(~(~0<<((1?3:3)-(0?3:3)+1)))))))<<(0?3:3)));

        gcmERR_BREAK(gcoHARDWARE_LoadState32(Hardware, 0x0380C, data[0]));

        /* Commit the command buffer. */
        gcmERR_BREAK(gcoHARDWARE_Commit(Hardware));
    }
    while (gcvFALSE);

    /* Return the status. */
    return status;
}

/*******************************************************************************
**
**  _SoftwareCopy
**
**  Make a copy of the specified rectangular area using CPU.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcsSURF_INFO_PTR SrcInfo
**          Pointer to the source surface descriptor.
**
**      gcsSURF_INFO_PTR DestInfo
**          Pointer to the destination surface descriptor.
**
**      gcsPOINT_PTR SrcOrigin
**          The origin of the source area to be copied.
**
**      gcsPOINT_PTR DestOrigin
**          The origin of the destination area to be copied.
**
**      gcsPOINT_PTR RectSize
**          The size of the rectangular area to be copied.
**
**  OUTPUT:
**
**      Nothing.
*/
static gceSTATUS _SoftwareCopy(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_INFO_PTR SrcInfo,
    IN gcsSURF_INFO_PTR DestInfo,
    IN gcsPOINT_PTR SrcOrigin,
    IN gcsPOINT_PTR DestOrigin,
    IN gcsPOINT_PTR RectSize
    )
{
    gceSTATUS status;

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmVERIFY_ARGUMENT(RectSize->x > 0);
    gcmVERIFY_ARGUMENT(RectSize->y > 0);

    do
    {
        gctBOOL srcTiled, dstTiled;
        gcsSURF_FORMAT_INFO_PTR srcFormatInfo[2];
        gcsSURF_FORMAT_INFO_PTR dstFormatInfo[2];
        gctUINT srcX, srcY;
        gctUINT dstX, dstY;
        gctUINT srcRight, srcBottom;
        gctUINT32 srcOffset, dstOffset;
        gcsSURF_FORMAT_INFO_PTR srcFormat, dstFormat;

        /* Verify that the surfaces are locked. */
        gcmVERIFY_LOCK(SrcInfo);
        gcmVERIFY_LOCK(DestInfo);

        /* Flush and stall the pipe. */
        gcmERR_BREAK(gcoHARDWARE_FlushPipe(Hardware));
        gcmERR_BREAK(gcoHARDWARE_Commit(Hardware));
        gcmERR_BREAK(gcoHARDWARE_Stall(Hardware));

        /* Query format specifics. */
        gcmERR_BREAK(gcoSURF_QueryFormat(SrcInfo->format,  srcFormatInfo));
        gcmERR_BREAK(gcoSURF_QueryFormat(DestInfo->format, dstFormatInfo));

        /* Determine whether the destination is tiled. */
        srcTiled = (SrcInfo->type  != gcvSURF_BITMAP);
        dstTiled = (DestInfo->type != gcvSURF_BITMAP);

        /* Test for fast copy. */
        if (srcTiled
        &&  dstTiled
        &&  (SrcInfo->format == DestInfo->format)
        &&  (SrcOrigin->x == 0)
        &&  (SrcOrigin->y == 0)
        &&  (RectSize->x == (gctINT) DestInfo->alignedWidth)
        &&  (RectSize->y == (gctINT) DestInfo->alignedHeight)
        )
        {
            gctUINT32 sourceOffset = 0;
            gctUINT32 targetOffset = 0;
            gctINT y;

            for (y = 0; y < RectSize->y; y += 4)
            {
                gcoOS_MemCopy(DestInfo->node.logical + targetOffset,
                              SrcInfo->node.logical  + sourceOffset,
                              DestInfo->stride * 4);

                sourceOffset += SrcInfo->stride  * 4;
                targetOffset += DestInfo->stride * 4;
            }

            return gcvSTATUS_OK;
        }

        /* Set initial coordinates. */
        srcX = SrcOrigin->x;
        srcY = SrcOrigin->y;
        dstX = DestOrigin->x;
        dstY = DestOrigin->y;

        /* Compute limits. */
        srcRight  = SrcOrigin->x + RectSize->x - 1;
        srcBottom = SrcOrigin->y + RectSize->y - 1;

        /* Loop through the rectangle. */
        while (gcvTRUE)
        {
            gctUINT8_PTR srcPixel, dstPixel;

            _ComputePixelLocation(
                srcX, srcY, SrcInfo->stride,
                srcFormatInfo, srcTiled,
                &srcOffset, &srcFormat
                );

            _ComputePixelLocation(
                dstX, dstY, DestInfo->stride,
                dstFormatInfo, dstTiled,
                &dstOffset, &dstFormat
                );

            srcPixel = SrcInfo->node.logical  + srcOffset;
            dstPixel = DestInfo->node.logical + dstOffset;

            gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                Hardware,
                srcPixel,
                dstPixel,
                0, 0,
                srcFormat,
                dstFormat,
                gcvNULL, gcvNULL
                ));

            /* End of line? */
            if (srcX == srcRight)
            {
                /* Last row? */
                if (srcY == srcBottom)
                {
                    break;
                }

                /* Reset to the beginning of the line. */
                srcX = SrcOrigin->x;
                dstX = DestOrigin->x;

                /* Advance to the next line. */
                srcY++;
                dstY++;
            }
            else
            {
                /* Advance to the next pixel. */
                srcX++;
                dstX++;
            }
        }
    }
    while (gcvFALSE);

    /* Return the status. */
    return status;
}

/*******************************************************************************
**
**  _SourceCopy
**
**  Make a copy of the specified rectangular area.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcsSURF_INFO_PTR SrcInfo
**          Pointer to the source surface descriptor.
**
**      gcsSURF_INFO_PTR DestInfo
**          Pointer to the destination surface descriptor.
**
**      gcsPOINT_PTR SrcOrigin
**          The origin of the source area to be copied.
**
**      gcsPOINT_PTR DestOrigin
**          The origin of the destination area to be copied.
**
**      gcsPOINT_PTR RectSize
**          The size of the rectangular area to be copied.
**
**  OUTPUT:
**
**      Nothing.
*/
static gceSTATUS _SourceCopy(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_INFO_PTR SrcInfo,
    IN gcsSURF_INFO_PTR DestInfo,
    IN gcsPOINT_PTR SrcOrigin,
    IN gcsPOINT_PTR DestOrigin,
    IN gcsPOINT_PTR RectSize
    )
{
    gceSTATUS status;

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    do
    {
        /* Limit to 2D surfaces only for now. */
        if ((SrcInfo->type != gcvSURF_BITMAP)
        ||  (DestInfo->type != gcvSURF_BITMAP)
        )
        {
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Is 2D pipe present? */
        if (Hardware->hw2DEngine
        &&  !Hardware->sw2DEngine
        /* GC500 needs properly aligned surfaces. */
        &&  (  (Hardware->chipModel != gcv500)
            || ((DestInfo->rect.right & 7) == 0)
            )
        )
        {
            status = _BitBlitCopy(
                Hardware,
                SrcInfo,
                DestInfo,
                SrcOrigin,
                DestOrigin,
                RectSize
                );
        }
        else
        {
            status = _SoftwareCopy(
                Hardware,
                SrcInfo,
                DestInfo,
                SrcOrigin,
                DestOrigin,
                RectSize
                );
        }
    }
    while (gcvFALSE);

    /* Return the status. */
    return status;
}

/*******************************************************************************
**
**  _Tile420Surface
**
**  Tile linear 4:2:0 source surface.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcsSURF_INFO_PTR SrcInfo
**          Pointer to the source surface descriptor.
**
**      gcsSURF_INFO_PTR DestInfo
**          Pointer to the destination surface descriptor.
**
**      gcsPOINT_PTR SrcOrigin
**          The origin of the source area to be copied.
**
**      gcsPOINT_PTR DestOrigin
**          The origin of the destination area to be copied.
**
**      gcsPOINT_PTR RectSize
**          The size of the rectangular area to be copied.
**
**  OUTPUT:
**
**      Nothing.
*/
static gceSTATUS _Tile420Surface(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_INFO_PTR SrcInfo,
    IN gcsSURF_INFO_PTR DestInfo,
    IN gcsPOINT_PTR SrcOrigin,
    IN gcsPOINT_PTR DestOrigin,
    IN gcsPOINT_PTR RectSize
    )
{
    gceSTATUS status;

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    do
    {
        gctUINT32 srcFormat;
        gctBOOL tilerAvailable;

        /* Verify that the surfaces are locked. */
        gcmVERIFY_LOCK(SrcInfo);
        gcmVERIFY_LOCK(DestInfo);

        /* Determine hardware support for 4:2:0 tiler. */
        tilerAvailable = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 13:13) & ((gctUINT32) ((((1 ? 13:13) - (0 ? 13:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 13:13) - (0 ? 13:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1)))))));

        /* Available? */
        if (!tilerAvailable)
        {
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Input limitations until more support is required. */
        if ((SrcOrigin->x  != 0) || (SrcOrigin->y  != 0) ||
            (DestOrigin->x != 0) || (DestOrigin->y != 0) ||
            (RectSize->x != (gctINT) SrcInfo->alignedWidth)  ||
            (RectSize->y != (gctINT) SrcInfo->alignedHeight) ||
            (RectSize->x != (gctINT) DestInfo->alignedWidth) ||
            (RectSize->y != (gctINT) DestInfo->alignedHeight))
        {
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Append FLUSH to the command buffer. */
        gcmERR_BREAK(gcoHARDWARE_FlushPipe(Hardware));

        /* Determine the format. */
        if ((SrcInfo->format == gcvSURF_YV12) ||
            (SrcInfo->format == gcvSURF_I420))
        {
            srcFormat = 0x0;
        }
        else
        {
            srcFormat = 0x1;
        }

        /* Set tiler configuration. */
        gcmERR_BREAK(gcoHARDWARE_LoadState32(
            Hardware,
            0x01678,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) | (((gctUINT32) (0x1&((gctUINT32)((((1?0:0)-(0?0:0)+1)==32)?~0:(~(~0<<((1?0:0)-(0?0:0)+1)))))))<<(0?0:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) | (((gctUINT32) ((gctUINT32) (srcFormat) & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))

            ));

        /* Set window size. */
        gcmERR_BREAK(gcoHARDWARE_LoadState32(
            Hardware,
            0x0167C,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (RectSize->x) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (RectSize->y) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))

            ));

        /* Set Y plane. */
        gcmERR_BREAK(gcoHARDWARE_LoadState32(
            Hardware,
            0x01680,
            SrcInfo->node.physical
            ));

        gcmERR_BREAK(gcoHARDWARE_LoadState32(
            Hardware,
            0x01684,
            SrcInfo->stride
            ));

        /* Set U plane. */
        gcmERR_BREAK(gcoHARDWARE_LoadState32(
            Hardware,
            0x01688,
            SrcInfo->node.physical2
            ));

        gcmERR_BREAK(gcoHARDWARE_LoadState32(
            Hardware,
            0x0168C,
            SrcInfo->uStride
            ));

        /* Set V plane. */
        gcmERR_BREAK(gcoHARDWARE_LoadState32(
            Hardware,
            0x01690,
            SrcInfo->node.physical3
            ));

        gcmERR_BREAK(gcoHARDWARE_LoadState32(
            Hardware,
            0x01694,
            SrcInfo->vStride
            ));

        /* Set destination. */
        gcmERR_BREAK(gcoHARDWARE_LoadState32(
            Hardware,
            0x01698,
            DestInfo->node.physical
            ));

        gcmERR_BREAK(gcoHARDWARE_LoadState32(
            Hardware,
            0x0169C,
            DestInfo->stride
            ));

        /* Disable clear. */
        gcmERR_BREAK(gcoHARDWARE_LoadState32(
            Hardware,
            0x0163C,
            0
            ));

        /* Trigger resolve. */
        gcmERR_BREAK(gcoHARDWARE_LoadState32(
            Hardware,
            0x01600,
            1
            ));

        /* Disable tiler. */
        gcmERR_BREAK(gcoHARDWARE_LoadState32(
            Hardware,
            0x01678,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            ));

        /* Commit the command buffer. */
        gcmERR_BREAK(gcoHARDWARE_Commit(Hardware));
    }
    while (gcvFALSE);

    /* Return the status. */
    return status;
}

typedef struct
{
    gctUINT32 mode;
    gctUINT32 horFactor;
    gctUINT32 verFactor;
}
gcsSUPERENTRY, *gcsSUPERENTRY_PTR;

typedef struct
{
    /* Source information. */
    gcsSURF_FORMAT_INFO_PTR srcFormatInfo[2];
    gceTILING srcTiling;

    /* Destination information. */
    gcsSURF_FORMAT_INFO_PTR dstFormatInfo[2];
    gceTILING dstTiling;

    /* Resolve information. */
    gctBOOL flipY;
    gcsSUPERENTRY_PTR superSampling;
}
gcsRESOLVE_VARS,
* gcsRESOLVE_VARS_PTR;

static gceSTATUS
_StripeResolve(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_INFO_PTR SrcInfo,
    IN gcsSURF_INFO_PTR DestInfo,
    IN gcsPOINT_PTR SrcOrigin,
    IN gcsPOINT_PTR DestOrigin,
    IN gcsPOINT_PTR RectSize,
    IN gcsRESOLVE_VARS_PTR Vars
    )
{
    gceSTATUS status;
    gcsRECT srcRect, dstRect;
    gctINT32 x, xStep, y, yStep;
    gctINT32 width, height;
    gctUINT32 srcOffset, dstOffset;
    gctINT hShift, vShift;
    gctINT32 dstY;

    /* Copy super-sampling factor. */
    hShift = (Vars->superSampling->horFactor == 1) ? 0 : 1;
    vShift = (Vars->superSampling->verFactor == 1) ? 0 : 1;

    /* Compute source bounding box. */
    srcRect.left   = SrcOrigin->x & ~15;
    srcRect.right  = gcmALIGN(SrcOrigin->x + (RectSize->x << hShift), 16);
    srcRect.top    = SrcOrigin->y & ~3;
    srcRect.bottom = gcmALIGN(SrcOrigin->y + (RectSize->y << vShift), 4);

    /* Compute destination bounding box. */
    dstRect.left   = DestOrigin->x & ~15;
    dstRect.right  = gcmALIGN(DestOrigin->x + RectSize->x, 16);
    dstRect.top    = DestOrigin->y & ~3;
    dstRect.bottom = gcmALIGN(DestOrigin->y + RectSize->y, 4);

    /* Walk all stripes horizontally. */
    for (x = srcRect.left; x < srcRect.right; x += xStep)
    {
        /* Compute horizontal step. */
        xStep = (x & 63) ? (x & 31) ? 16 : 32 : 64;
        gcmASSERT((x & ~63) == ((x + xStep - 1) & ~63));
        yStep = 16 * Hardware->needStriping / xStep;

        /* Compute width. */
        width = gcmMIN(srcRect.right - x, xStep);

        /* Walk the stripe vertically. */
        for (y = srcRect.top; y < srcRect.bottom; y += yStep)
        {
            /* Compute vertical step. */
            yStep = 16 * Hardware->needStriping / xStep;
            if ((y & ~63) != ((y + yStep - 1) & ~63))
            {
                /* Don't overflow a super tile. */
                yStep = gcmALIGN(y, 64) - y;
            }

            /* Compute height. */
            height = gcmMIN(srcRect.bottom - y, yStep);

            /* Compute destination y. */
            dstY = Vars->flipY ? (dstRect.bottom - (y >> vShift) - height)
                               : (y >> vShift);

            /* Compute offsets. */
            gcmONERROR(
                gcoHARDWARE_ComputeOffset(x, y,
                                          SrcInfo->stride,
                                          Vars->srcFormatInfo[0]->bitsPerPixel / 8,
                                          Vars->srcTiling, &srcOffset));
            gcmONERROR(
                gcoHARDWARE_ComputeOffset(x >> hShift, dstY,
                                          DestInfo->stride,
                                          Vars->dstFormatInfo[0]->bitsPerPixel / 8,
                                          Vars->dstTiling, &dstOffset));

            /* Resolve one part of the stripe. */
            gcmONERROR(
                gcoHARDWARE_LoadState32(Hardware,
                                        0x01608,
                                        SrcInfo->node.physical + srcOffset));

            gcmONERROR(
                gcoHARDWARE_LoadState32(Hardware,
                                        0x01610,
                                        DestInfo->node.physical + dstOffset));

            gcmONERROR(
                gcoHARDWARE_LoadState32(Hardware,
                                        0x01620,
                                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (width) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

                                        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (height) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))));

            gcmONERROR(
                gcoHARDWARE_LoadState32(Hardware,
                                        0x01600,
                                        0xBEEBBEEB));
        }
    }

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    /* Return the error. */
    return status;
}

/*******************************************************************************
**
**  _ResolveRect
**
**  Perform a resolve on a surface.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcsSURF_INFO_PTR SrcInfo
**          Pointer to the source surface descriptor.
**
**      gcsSURF_INFO_PTR DestInfo
**          Pointer to the destination surface descriptor.
**
**      gcsPOINT_PTR SrcOrigin
**          The origin of the source area to be resolved.
**
**      gcsPOINT_PTR DestOrigin
**          The origin of the destination area to be resolved.
**
**      gcsPOINT_PTR RectSize
**          The size of the rectangular area to be resolved.
**
**  OUTPUT:
**
**      Nothing.
*/
static gceSTATUS _ResolveRect(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_INFO_PTR SrcInfo,
    IN gcsSURF_INFO_PTR DestInfo,
    IN gcsPOINT_PTR SrcOrigin,
    IN gcsPOINT_PTR DestOrigin,
    IN gcsPOINT_PTR RectSize
    )
{
#   define _AA(HorFactor, VerFactor) \
    { \
        AQ_RS_CONFIG_RS_SRC_SUPER_SAMPLE_ENABLE \
            ## HorFactor ## X ## VerFactor, \
        HorFactor, \
        VerFactor \
    }

#   define _VAA \
    { \
        0x1, \
        2, \
        1 \
    }

#   define _NOAA \
    { \
        0x0, \
        1, \
        1 \
    }

#   define _INVALIDAA \
    { \
        ~0U, \
        ~0U, \
        ~0U \
    }

    static gcsSUPERENTRY superSamplingTable[17] =
    {
        /*  SOURCE 1x1                 SOURCE 2x1 */

            /* DEST 1x1  DEST 2x1      DEST 1x1  DEST 2x1 */
            _NOAA, _INVALIDAA, { 0x1,2,1},_NOAA,

            /* DEST 1x2  DEST 2x2      DEST 1x2  DEST 2x2 */
            _INVALIDAA,  _INVALIDAA,   _INVALIDAA, _INVALIDAA,

        /*  SOURCE 1x2                 SOURCE 2x2 */

            /* DEST 1x1  DEST 2x1      DEST 1x1  DEST 2x1 */
            { 0x2, 1, 2}, _INVALIDAA, { 0x3, 2, 2}, { 0x2,1,2},

            /* DEST 1x2  DEST 2x2      DEST 1x2  DEST 2x2 */
            _NOAA, _INVALIDAA, { 0x1,2,1},_NOAA,

            /* VAA */
            _VAA,
    };

    gceSTATUS status;
    gcsRESOLVE_VARS vars;
    gcsPOINT srcOrigin;
    gcsPOINT dstOrigin;
    gcsPOINT srcSize;
    gctUINT32 config, srcWndSize;
    gctUINT32 srcAddress, dstAddress;
    gctUINT32 srcStride, dstStride;
    gctUINT32 srcOffset, dstOffset;
    gctUINT32 srcFormat, dstFormat;
    gctUINT superSamplingIndex;
    gctBOOL dstFlip;
    gctUINT32 vaa, endian;
    gctUINT32 dither[2];
    gctUINT32 states[22];
    gctUINT index = 0;

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT((SrcOrigin->x & 3) == 0);
    gcmVERIFY_ARGUMENT((SrcOrigin->y & 3) == 0);

    gcmVERIFY_ARGUMENT((DestOrigin->x & 3) == 0);
    gcmVERIFY_ARGUMENT((DestOrigin->y & 3) == 0);

    gcmVERIFY_ARGUMENT((RectSize->x & 15) == 0);
    gcmVERIFY_ARGUMENT((RectSize->y &  3) == 0);

    /* Verify that the surfaces are locked. */
    if (!SrcInfo->node.valid || !DestInfo->node.valid)
    {
        gcmONERROR(gcvSTATUS_MEMORY_UNLOCKED);
    }

    /* Convert source and destination formats. */
    gcmONERROR(
        _ConvertResolveFormat(Hardware,
                              SrcInfo->format,
                              &srcFormat,
                              gcvNULL));

    gcmONERROR(
        gcoSURF_QueryFormat(SrcInfo->format, vars.srcFormatInfo));

    gcmONERROR(
        _ConvertResolveFormat(Hardware,
                              DestInfo->format,
                              &dstFormat,
                              &dstFlip));

    gcmONERROR(
        gcoSURF_QueryFormat(DestInfo->format, vars.dstFormatInfo));

    /* Determine if y flipping is required. */
    vars.flipY = SrcInfo->orientation != DestInfo->orientation;

    if (vars.flipY
    &&  !((((gctUINT32) (Hardware->chipMinorFeatures)) >> (0 ? 0:0) & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1)))))))
    )
    {
        DestInfo->orientation = SrcInfo->orientation;
        vars.flipY            = gcvFALSE;
    }

    /* Determine source tiling. */
    vars.srcTiling =

        /* Super tiled? */
        SrcInfo->superTiled ? gcvSUPERTILED

        /* Tiled? */
        : (SrcInfo->type != gcvSURF_BITMAP) ? gcvTILED

        /* Linear. */
        : gcvLINEAR;

    /* Determine destination tiling. */
    vars.dstTiling =

        /* Super tiled? */
        DestInfo->superTiled ? gcvSUPERTILED

        /* Tiled? */
        : (DestInfo->type != gcvSURF_BITMAP) ? gcvTILED

        /* Linear. */
        : gcvLINEAR;

    /* Determine vaa value. */
    vaa = SrcInfo->vaa
        ? (srcFormat == 0x06)
          ? 0x2
          : 0x1
        : 0x0;

    /* Determine the supersampling mode. */
    if (SrcInfo->vaa && !DestInfo->vaa)
    {
        superSamplingIndex = 16;
    }
    else
    {
        superSamplingIndex = (SrcInfo->samples.y  << 3)
                           + (DestInfo->samples.y << 2)
                           + (SrcInfo->samples.x  << 1)
                           +  DestInfo->samples.x
                           -  15;

        if (SrcInfo->vaa && DestInfo->vaa)
        {
            srcFormat = 0x06;
            dstFormat = 0x06;
            vaa       = 0x0;
        }
    }

    vars.superSampling = &superSamplingTable[superSamplingIndex];

    /* Supported mode? */
    if (vars.superSampling->mode == ~0)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Determine dithering. */
    if (vars.srcFormatInfo[0]->bitsPerPixel > vars.dstFormatInfo[0]->bitsPerPixel)
    {
        dither[0] = Hardware->dither[0];
        dither[1] = Hardware->dither[1];
    }
    else
    {
        dither[0] = dither[1] = ~0U;
    }

    /* Flush the pipe. */
    gcmONERROR(gcoHARDWARE_FlushPipe(Hardware));

    /* Switch to 3D pipe. */
    gcmONERROR(gcoHARDWARE_SelectPipe(Hardware, 0x0));

    /* Construct configuration state. */
    config

        /* Configure source. */
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | (((gctUINT32) ((gctUINT32) (srcFormat) & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))

        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7))) | (((gctUINT32) ((gctUINT32) (vars.srcTiling!=gcvLINEAR) & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)))

        /* Configure destination. */
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))) | (((gctUINT32) ((gctUINT32) (dstFormat) & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))

        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 14:14) - (0 ? 14:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ? 14:14))) | (((gctUINT32) ((gctUINT32) (vars.dstTiling!=gcvLINEAR) & ((gctUINT32) ((((1 ? 14:14) - (0 ? 14:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ? 14:14)))

        /* Configure flipping. */
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 29:29) - (0 ? 29:29) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29))) | (((gctUINT32) ((gctUINT32) (dstFlip) & ((gctUINT32) ((((1 ? 29:29) - (0 ? 29:29) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)))

        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30))) | (((gctUINT32) ((gctUINT32) (vars.flipY) & ((gctUINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)))


        /* Configure supersampling enable. */
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 6:5) - (0 ? 6:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ? 6:5))) | (((gctUINT32) ((gctUINT32) (vars.superSampling->mode) & ((gctUINT32) ((((1 ? 6:5) - (0 ? 6:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ? 6:5)));

    /* Determine the source stride. */
    srcStride = (vars.srcTiling == gcvLINEAR)

              /* Linear. */
              ? SrcInfo->stride

              /* Tiled. */
              : ((((gctUINT32) (SrcInfo->stride*4)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31))) | (((gctUINT32) ((gctUINT32) (vars.srcTiling==gcvSUPERTILED) & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));

    /* Determine the destination stride. */
    dstStride = (vars.dstTiling == gcvLINEAR)

              /* Linear. */
              ? DestInfo->stride

              /* Tiled. */
              : ((((gctUINT32) (DestInfo->stride*4)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31))) | (((gctUINT32) ((gctUINT32) (vars.dstTiling==gcvSUPERTILED) & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));

    /* Append RESOLVE_CONFIG state. */
    states[index++] = 0x01604;
    states[index++] = config;

    /* Set source and destination stride. */
    states[index++] = 0x0160C;
    states[index++] = srcStride;

    states[index++] = 0x01614;
    states[index++] = dstStride;

    /* Append RESOLVE_DITHER commands. */
    states[index++] = 0x01630;
    states[index++] = dither[0];
    states[index++] = 0x01630 + 4;
    states[index++] = dither[1];

    /* Append RESOLVE_CLEAR_CONTROL state. */
    states[index++] = 0x0163C;
    states[index++] = 0;

    /* Set endian control */
    endian = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8)));

    if (Hardware->bigEndian &&
		(SrcInfo->type != gcvSURF_TEXTURE) &&
		(DestInfo->type == gcvSURF_BITMAP))
    {
        gctUINT32 destBPP;

        /* Compute bits per pixel. */
        gcmONERROR(gcoHARDWARE_ConvertFormat(Hardware,
                                             DestInfo->format,
                                             &destBPP,
                                             gcvNULL));

        if (destBPP == 16)
        {
            endian = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8)));
        }
        else if (destBPP == 32)
        {
            endian = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8)));
        }
    }

    /* Append new configuration register. */
    states[index++] = 0x016A0;
    states[index++] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))) | (((gctUINT32) ((gctUINT32) (vaa) & ((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))|endian;

    if (Hardware->needStriping
    &&  ( ((((gctUINT32) (Hardware->memoryConfig)) >> (0 ? 1:1)) & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1)))))) )
    )
    {
        gcmASSERT(index <= gcmCOUNTOF(states));
        gcmONERROR(gcoHARDWARE_LoadStateBlock(Hardware, states, index));

        /* Stripe the resolve. */
        gcmONERROR(_StripeResolve(Hardware,
                                  SrcInfo,
                                  DestInfo,
                                  SrcOrigin,
                                  DestOrigin,
                                  RectSize,
                                  &vars));
    }

    else
    {
        /* Determine the origins. */
        srcOrigin.x = SrcOrigin->x  * SrcInfo->samples.x;
        srcOrigin.y = SrcOrigin->y  * SrcInfo->samples.y;
        dstOrigin.x = DestOrigin->x * DestInfo->samples.x;
        dstOrigin.y = DestOrigin->y * DestInfo->samples.y;

        /* Determine the source base address offset. */
        gcmONERROR(
            gcoHARDWARE_ComputeOffset(srcOrigin.x, srcOrigin.y,
                                      SrcInfo->stride,
                                      vars.srcFormatInfo[0]->bitsPerPixel / 8,
                                      vars.srcTiling, &srcOffset));

        /* Determine the destination base address offset. */
        gcmONERROR(
            gcoHARDWARE_ComputeOffset(dstOrigin.x, dstOrigin.y,
                                      DestInfo->stride,
                                      vars.dstFormatInfo[0]->bitsPerPixel / 8,
                                      vars.dstTiling, &dstOffset));

        /* Determine base addresses. */
        srcAddress = SrcInfo->node.physical  + srcOffset;
        dstAddress = DestInfo->node.physical + dstOffset;

        /* Determine source rectangle size. */
        srcSize.x = RectSize->x * vars.superSampling->horFactor;
        srcSize.y = RectSize->y * vars.superSampling->verFactor;

        srcWndSize = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (srcSize.x) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))

                   | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (srcSize.y) & ((gctUINT32) ((((1 ? 31:16) - (0 ? 31:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

        /* Append RESOLVE_SOURCE commands. */
        states[index++] = 0x01608;
        states[index++] = srcAddress;

        /* Append RESOLVE_DESTINATION commands. */
        states[index++] = 0x01610;
        states[index++] = dstAddress;

        /* Append RESOLVE_WINDOW commands. */
        states[index++] = 0x01620;
        states[index++] = srcWndSize;

        /* Trigger resolve. */
        states[index++] = 0x01600;
        states[index++] = 0xBEEBBEEB;

        gcmASSERT(index <= gcmCOUNTOF(states));
        gcmONERROR(gcoHARDWARE_LoadStateBlock(Hardware, states, index));
    }


    /* Commit the command buffer. */
    gcmONERROR(gcoHARDWARE_Commit(Hardware));

    if ((DestInfo == Hardware->currentTarget)
    &&  (DestOrigin->x == 0)
    &&  (DestOrigin->y == 0)
    &&  (RectSize->x >= DestInfo->rect.right)
    &&  (RectSize->y >= DestInfo->rect.bottom)
    )
    {
        /* All pixels have been resolved. */
        Hardware->targetDirty = gcvFALSE;
    }

    /* Tile status cache is dirty. */
    Hardware->cacheDirty = gcvTRUE;

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_ComputeOffset
**
**  Compute the offset of the specified pixel location.
**
**  INPUT:
**
**      gctINT32 X, Y
**          Coordinates of the pixel.
**
**      gctUINT Stride
**          Surface stride.
**
**      gctINT BytesPerPixel
**          The number of bytes per pixel in the surface.
**
**      gctINT Tiling
**          Tiling type.
**
**  OUTPUT:
**
**      Computed pixel offset.
*/
gceSTATUS gcoHARDWARE_ComputeOffset(
    IN gctINT32 X,
    IN gctINT32 Y,
    IN gctUINT Stride,
    IN gctINT BytesPerPixel,
    IN gceTILING Tiling,
    OUT gctUINT32_PTR Offset
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    switch (Tiling)
    {
    case gcvLINEAR:
        /* Linear. */
        * Offset

            /* Skip full rows of pixels. */
            = Y * Stride

            /* Skip pixels to the left. */
            + X * BytesPerPixel;
        break;

    case gcvTILED:
        /* Tiled. */
        * Offset

            /* Skip full rows of tiles 4x4 to the top. */
            = (Y & ~3) * Stride

            + BytesPerPixel
            * (
                  /* Skip full 4x4 tiles to the left. */
                  ((X & ~0x03) << 2)

                  /* Skip rows/pixels inside the target 4x4 tile. */
                + ((Y &  0x03) << 2)
                + ((X &  0x03)     )
            );
        break;

    case gcvSUPERTILED:
        /* Super tiled. */
        * Offset

            /* Skip full rows of 64x64 tiles to the top. */
            = (Y & ~63) * Stride

            + BytesPerPixel
            * (
                  /* Skip full 64x64 tiles to the left. */
                  ((X & ~0x3F) << 6)

                  /* Skip full 16x16 tiles to the left and to the top. */
                + ((Y &  0x30) << 6)
                + ((X &  0x38) << 4)

                  /* Skip full 4x4 tiles to the left and to the top. */
                + ((Y &  0x0C) << 3)
                + ((X &  0x04) << 2)

                  /* Skip rows/pixels inside the target 4x4 tile. */
                + ((Y &  0x03) << 2)
                + ((X &  0x03)     )
            );
        break;

    default:
        status = gcvSTATUS_NOT_SUPPORTED;
    }

    /* Return status. */
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_ResolveRect
**
**  Resolve a rectangluar area of a surface to another surface.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcsSURF_INFO_PTR SrcInfo
**          Pointer to the source surface descriptor.
**
**      gcsSURF_INFO_PTR DestInfo
**          Pointer to the destination surface descriptor.
**
**      gcsPOINT_PTR SrcOrigin
**          The origin of the source area to be resolved.
**
**      gcsPOINT_PTR DestOrigin
**          The origin of the destination area to be resolved.
**
**      gcsPOINT_PTR RectSize
**          The size of the rectangular area to be resolved.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_ResolveRect(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_INFO_PTR SrcInfo,
    IN gcsSURF_INFO_PTR DestInfo,
    IN gcsPOINT_PTR SrcOrigin,
    IN gcsPOINT_PTR DestOrigin,
    IN gcsPOINT_PTR RectSize
    )
{
    gceSTATUS status;
    gctBOOL locked = gcvFALSE;
    gctBOOL srcLocked = gcvFALSE;
    gctBOOL destLocked = gcvFALSE;

    gcmHEADER_ARG("Hardware=0x%x SrcInfo=0x%x DestInfo=0x%x "
                    "SrcOrigin=0x%x DestOrigin=0x%x RectSize=0x%x",
                    Hardware, SrcInfo, DestInfo,
                    SrcOrigin, DestOrigin, RectSize);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    do
    {
        gctBOOL resampling;
        gctBOOL dithering;
        gctBOOL srcTiled;
        gctBOOL destTiled;
        gcsPOINT alignedSrcOrigin, alignedSrcSize;
        gcsPOINT alignedDestOrigin, alignedDestSize;
        gcsPOINT alignedRectSize;
        gcsSURF_FORMAT_INFO_PTR srcFormatInfo[2];
        gcsSURF_FORMAT_INFO_PTR dstFormatInfo[2];
        gcsSURF_FORMAT_INFO_PTR tmpFormatInfo;
        gcsPOINT tempOrigin;
        gctBOOL flip = gcvFALSE;


        /***********************************************************************
        ** Determine special functions.
        */

        srcTiled  = (SrcInfo->type  != gcvSURF_BITMAP);
        destTiled = (DestInfo->type != gcvSURF_BITMAP);

        resampling =  (SrcInfo->samples.x  != 1)
                   || (SrcInfo->samples.y  != 1)
                   || (DestInfo->samples.x != 1)
                   || (DestInfo->samples.y != 1);

        dithering = Hardware->dither[0] != Hardware->dither[1];

        /***********************************************************************
        ** Since 2D bitmaps live in linear space, we don't need to resolve them.
        ** We can just copy them using the 2D engine.  However, we need to flush
        ** and stall after we are done with the copy since we have to make sure
        ** the bits are there before we blit them to the screen.
        */

        if (!srcTiled && !destTiled && !resampling && !dithering && !flip)
        {
            status = _SourceCopy(
                Hardware,
                SrcInfo,
                DestInfo,
                SrcOrigin,
                DestOrigin,
                RectSize
                );

            /* Done. */
            break;
        }


        /***********************************************************************
        ** YUV 4:2:0 tiling case.
        */

        if ((SrcInfo->format == gcvSURF_YV12) ||
            (SrcInfo->format == gcvSURF_I420) ||
            (SrcInfo->format == gcvSURF_NV12))
        {
            if (!srcTiled && destTiled && (DestInfo->format == gcvSURF_YUY2))
            {
                status = _Tile420Surface(
                    Hardware,
                    SrcInfo,
                    DestInfo,
                    SrcOrigin,
                    DestOrigin,
                    RectSize
                    );
            }
            else
            {
                status = gcvSTATUS_NOT_SUPPORTED;
            }

            /* Done. */
            break;
        }


        /***********************************************************************
        ** Surfaces smaller then 16 pixels wide are not supported by the hw.
        */

        if ((SrcInfo->rect.right < 16) || (DestInfo->rect.right < 16))
        {
            status = gcoHARDWARE_CopyPixels(
                Hardware,
                SrcInfo,
                DestInfo,
                SrcOrigin->x,
                SrcOrigin->y,
                DestOrigin->x,
                DestOrigin->y,
                RectSize->x,
                RectSize->y
                );

            /* Done. */
            break;
        }

        /***********************************************************************
        ** Calculate the aligned source and destination rectangles; aligned to
        ** completely cover the specified source and destination areas.
        */

        _AlignResolveRect(
            SrcOrigin, RectSize, &alignedSrcOrigin, &alignedSrcSize
            );

        _AlignResolveRect(
            DestOrigin, RectSize, &alignedDestOrigin, &alignedDestSize
            );

        /* Use the maximum rectangle. */
        alignedRectSize.x = gcmMAX(alignedSrcSize.x, alignedDestSize.x);
        alignedRectSize.y = gcmMAX(alignedSrcSize.y, alignedDestSize.y);

        /***********************************************************************
        ** If specified and aligned rectangles are the same, then the requested
        ** rectangle is prefectly aligned and we can do it in one shot.
        */

        if ((alignedSrcOrigin.x  == SrcOrigin->x)  &&
            (alignedSrcOrigin.y  == SrcOrigin->y)  &&
            (alignedDestOrigin.x == DestOrigin->x) &&
            (alignedDestOrigin.y == DestOrigin->y) &&
            (alignedRectSize.x   == RectSize->x)   &&
            (alignedRectSize.y   == RectSize->y))
        {
            status = _ResolveRect(
                Hardware,
                SrcInfo,
                DestInfo,
                SrcOrigin,
                DestOrigin,
                RectSize
                );

            /* Done. */
            break;
        }

        /***********************************************************************
        ** Special case when source and destination are both tiled and the
        ** request is for the entire surface.
        */
        if ((SrcInfo->type != gcvSURF_BITMAP)
        &&  (alignedSrcOrigin.x == 0)
        &&  (alignedSrcOrigin.y == 0)
        &&  (DestInfo->type != gcvSURF_BITMAP)
        &&  (RectSize->x == (gctINT) DestInfo->alignedWidth)
        &&  (RectSize->y == (gctINT) DestInfo->alignedHeight)
        )
        {
            /* Flush the pipe. */
            gcmERR_BREAK(
                gcoHARDWARE_FlushPipe(Hardware));


            /* Lock the source surface. */
            gcmERR_BREAK(
                gcoHARDWARE_Lock(Hardware,
                                 &SrcInfo->node,
                                 gcvNULL,
                                 gcvNULL));

            srcLocked = gcvTRUE;

            /* Lock the destination surface. */
            gcmERR_BREAK(
                gcoHARDWARE_Lock(Hardware,
                                 &DestInfo->node,
                                 gcvNULL,
                                 gcvNULL));

            destLocked = gcvTRUE;

            /* Perform software copy. */
            gcmERR_BREAK(
                _SoftwareCopy(Hardware,
                              SrcInfo,
                              DestInfo,
                              SrcOrigin,
                              DestOrigin,
                              RectSize));

            /* Done. */
            break;
        }

        /***********************************************************************
        ** At least one side of the rectangle is not aligned. In this case we
        ** will allocate a temporary buffer to resolve the aligned rectangle
        ** to and then use a source copy to complete the operation.
        */

        /* Query format specifics. */
        gcmERR_BREAK(gcoSURF_QueryFormat(SrcInfo->format,  srcFormatInfo));
        gcmERR_BREAK(gcoSURF_QueryFormat(DestInfo->format, dstFormatInfo));

        /* Pick the most compact format for the temporary surface. */
        tmpFormatInfo
            = (srcFormatInfo[0]->bitsPerPixel < dstFormatInfo[0]->bitsPerPixel)
                ? srcFormatInfo[0]
                : dstFormatInfo[0];

        /* Allocate the temporary surface. */
        gcmERR_BREAK(gcoHARDWARE_AllocateTemporarySurface(
            Hardware,
            alignedRectSize.x,
            alignedRectSize.y,
            tmpFormatInfo,
            gcvSURF_BITMAP
            ));

        /* Lock the temporary surface. */
        gcmERR_BREAK(gcoHARDWARE_Lock(
            Hardware,
            &Hardware->tempBuffer.node,
            gcvNULL,
            gcvNULL
            ));

        /* Mark as locked. */
        locked = gcvTRUE;

        /* Set the temporary buffer origin. */
        tempOrigin.x = 0;
        tempOrigin.y = 0;

        /* Resolve the aligned rectangle into the temporary surface. */
        gcmERR_BREAK(_ResolveRect(
            Hardware,
            SrcInfo,
            &Hardware->tempBuffer,
            &alignedSrcOrigin,
            &tempOrigin,
            &alignedRectSize
            ));

        /* Compute the temporary buffer origin. */
        tempOrigin.x = SrcOrigin->x - alignedSrcOrigin.x;
        tempOrigin.y = SrcOrigin->y - alignedSrcOrigin.y;

        /* Copy the unaligned area to the final destination. */
        gcmERR_BREAK(_SourceCopy(
            Hardware,
            &Hardware->tempBuffer,
            DestInfo,
            &tempOrigin,
            DestOrigin,
            RectSize
            ));
    }
    while (gcvFALSE);


    /* Unlock. */
    if (locked)
    {
        /* Unlock the temporary surface. */
        gcmVERIFY_OK(gcoHARDWARE_Unlock(Hardware,
                                        &Hardware->tempBuffer.node,
                                        Hardware->tempBuffer.type));
    }

    if (srcLocked)
    {
        /* Unlock the source. */
        gcmVERIFY_OK(gcoHARDWARE_Unlock(Hardware,
                                        &SrcInfo->node,
                                        SrcInfo->type));
    }

    if (destLocked)
    {
        /* Unlock the source. */
        gcmVERIFY_OK(gcoHARDWARE_Unlock(Hardware,
                                        &DestInfo->node,
                                        DestInfo->type));
    }

    /* Return result. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_Lock
**
**  Lock video memory.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to a gcoHARDWARE object.
**
**      gcsSURF_NODE_PTR Node
**          Pointer to a gcsSURF_NODE structure that describes the video
**          memory to lock.
**
**  OUTPUT:
**
**      gctUINT32 * Address
**          Physical address of the surface.
**
**      gctPOINTER * Memory
**          Logical address of the surface.
*/
gceSTATUS
gcoHARDWARE_Lock(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_NODE_PTR Node,
    OUT gctUINT32 * Address,
    OUT gctPOINTER * Memory
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Node=0x%x Address=0x%x Memory=0x%x",
                    Hardware, Node, Address, Memory);

    do
    {
        if (Node->valid)
        {
            status = gcvSTATUS_OK;
        }
        else
        {
            gcsHAL_INTERFACE iface;

            /* User pools have to be mapped first. */
            if (Node->pool == gcvPOOL_USER)
            {
                status = gcvSTATUS_MEMORY_UNLOCKED;
                break;
            }

            /* Fill in the kernel call structure. */
            iface.command = gcvHAL_LOCK_VIDEO_MEMORY;
            iface.u.LockVideoMemory.node = Node->u.normal.node;

            /* Call the kernel. */
            gcmERR_BREAK(gcoOS_DeviceControl(
                Hardware->os,
                IOCTL_GCHAL_INTERFACE,
                &iface, sizeof(iface),
                &iface, sizeof(iface)
                ));

            /* Success? */
            gcmERR_BREAK(iface.status);

            /* Validate the node. */
            Node->valid = gcvTRUE;

            /* Store pointers. */
            Node->physical = iface.u.LockVideoMemory.address;
            Node->logical  = iface.u.LockVideoMemory.memory;

            /* Set locked in the kernel flag. */
            Node->lockedInKernel = gcvTRUE;
        }

        /* Increment the lock count. */
        Node->lockCount++;

        /* Set the result. */
        if (Address != gcvNULL)
        {
            *Address = Node->physical;
        }

        if (Memory != gcvNULL)
        {
            *Memory = Node->logical;
        }
    }
    while (gcvFALSE);

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_Unlock
**
**  Unlock video memory.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcsSURF_NODE_PTR Node
**          Pointer to a gcsSURF_NODE structure that describes the video
**          memory to unlock.
**
**      gceSURF_TYPE Type
**          Type of surface to unlock.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_Unlock(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_NODE_PTR Node,
    IN gceSURF_TYPE Type
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Node=0x%x Type=%d",
                    Hardware, Node, Type);

    do
    {
        gcsHAL_INTERFACE iface;

        /* Verify whether the node is valid. */
        if (!Node->valid || (Node->lockCount == 0))
        {
            gcmTRACE_ZONE(
                gcvLEVEL_WARNING, gcvZONE_SURFACE,
                "gcoHARDWARE_Unlock: Node=0x%x; unlock called on an unlocked surface.",
                Node
                );

            status = gcvSTATUS_OK;
            break;
        }

        /* Locked more then once? */
        if (Node->lockCount > 1)
        {
            /* Decrement the lock count. */
            Node->lockCount--;

            /* Success. */
            status = gcvSTATUS_OK;
            break;
        }

        /* User mapped surfaces don't need to be unlocked. */
        if (Node->pool == gcvPOOL_USER)
        {
            /* Reset the count and leave the node as valid. */
            Node->lockCount = 0;

            /* Success. */
            status = gcvSTATUS_OK;
            break;
        }

        if (Node->lockedInKernel)
        {
            /* Unlock the video memory node. */
            iface.command = gcvHAL_UNLOCK_VIDEO_MEMORY;
            iface.u.UnlockVideoMemory.node = Node->u.normal.node;
            iface.u.UnlockVideoMemory.type = Type;
            iface.u.UnlockVideoMemory.asynchroneous = gcvTRUE;

            /* Call the kernel. */
            status = gcoOS_DeviceControl(
                Hardware->os,
                IOCTL_GCHAL_INTERFACE,
                &iface, gcmSIZEOF(iface),
                &iface, gcmSIZEOF(iface)
                );

            /* Verify result. */
            if (gcmNO_ERROR(status))
            {
                /* Get kernel status. */
                status = iface.status;
            }

            if (gcmIS_ERROR(status))
            {
                /* Error. */
                break;
            }

            /* Do we need to schedule an event for the unlock? */
            if (iface.u.UnlockVideoMemory.asynchroneous)
            {
                iface.u.UnlockVideoMemory.asynchroneous = gcvFALSE;
                status = gcoHARDWARE_CallEvent(Hardware, &iface);
            }

            if (gcmIS_ERROR(status))
            {
                /* Error. */
                break;
            }

            /* Reset locked in the kernel flag. */
            Node->lockedInKernel = gcvFALSE;
        }
        else
        {
            /* Success. */
            status = gcvSTATUS_OK;
        }

        /* Reset the valid flag. */
        if (Node->pool == gcvPOOL_CONTIGUOUS || Node->pool == gcvPOOL_VIRTUAL)
        {
            Node->valid = gcvFALSE;
        }

        /* Reset the lock count. */
        Node->lockCount = 0;
    }
    while (gcvFALSE);

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_CallEvent
**
**  Send an event to the kernel and append the required synchronization code to
**  the command buffer..
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcsHAL_INTERFACE * Interface
**          Pointer to an gcsHAL_INTERFACE structure the defines the event to
**          send.
**
**  OUTPUT:
**
**      gcsHAL_INTERFACE * Interface
**          Pointer to an gcsHAL_INTERFACE structure the received information
**          from the kernel.
*/
gceSTATUS
gcoHARDWARE_CallEvent(
    IN gcoHARDWARE Hardware,
    IN OUT gcsHAL_INTERFACE * Interface
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Interface=0x%x", Hardware, Interface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmVERIFY_ARGUMENT(Interface != gcvNULL);

    /* Append the event to the event queue. */
    status = gcoQUEUE_AppendEvent(Hardware->queue, Interface);


    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_ScheduleVideoMemory
**
**  Schedule destruction for the specified video memory node.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to a gcoHARDWARE object.
**
**      gcsSURF_NODE_PTR Node
**          Pointer to the video momory node to be destroyed.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_ScheduleVideoMemory(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_NODE_PTR Node
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Node=0x%x", Hardware, Node);

    if (Node->pool == gcvPOOL_USER)
    {
        /* User-allocated memory, don't touch. */
        status = gcvSTATUS_OK;
    }
    else
    {
        gcsHAL_INTERFACE iface;

        /* Verify the arguments. */
        gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

        /* Free the allocated video memory asynchronously. */
        iface.command = gcvHAL_FREE_VIDEO_MEMORY;
        iface.u.FreeVideoMemory.node = Node->u.normal.node;

        /* Call kernel HAL. */
        status = gcoHARDWARE_CallEvent(Hardware, &iface);
    }

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_AllocateTemporarySurface
**
**  Allocates a temporary surface with specified parameters.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to a gcoHARDWARE object.
**
**      gctUINT Width, Height
**          The aligned size of the surface to be allocated.
**
**      gcsSURF_FORMAT_INFO_PTR Format
**          The format of the surface to be allocated.
**
**      gceSURF_TYPE Type
**          The type of the surface to be allocated.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_AllocateTemporarySurface(
    IN gcoHARDWARE Hardware,
    IN gctUINT Width,
    IN gctUINT Height,
    IN gcsSURF_FORMAT_INFO_PTR Format,
    IN gceSURF_TYPE Type
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Width=%d Height=%d "
                    "Format=%d Type=%d",
                    Hardware, Width, Height,
                    Format, Type);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    do
    {
        gcsHAL_INTERFACE iface;

        /* Do we have a compatible surface? */
        if ((Hardware->tempBuffer.type == Type) &&
            (Hardware->tempBuffer.format == Format->format) &&
            (Hardware->tempBuffer.rect.right == (gctINT) Width) &&
            (Hardware->tempBuffer.rect.bottom == (gctINT) Height))
        {
            status = gcvSTATUS_OK;
            break;
        }

        /* Delete existing buffer. */
        gcmERR_BREAK(gcoHARDWARE_FreeTemporarySurface(Hardware, gcvTRUE));

        Hardware->tempBuffer.alignedWidth  = Width;
        Hardware->tempBuffer.alignedHeight = Height;

        /* Align the width and height. */
        gcmERR_BREAK(
            gcoHARDWARE_AlignToTile(Hardware,
                                    Type,
                                    &Hardware->tempBuffer.alignedWidth,
                                    &Hardware->tempBuffer.alignedHeight,
                                    gcvNULL));

        /* Init the interface structure. */
        iface.command = gcvHAL_ALLOCATE_LINEAR_VIDEO_MEMORY;
        iface.u.AllocateLinearVideoMemory.bytes     = Hardware->tempBuffer.alignedWidth
                                                    * Format->bitsPerPixel / 8
                                                    * Hardware->tempBuffer.alignedHeight;
        iface.u.AllocateLinearVideoMemory.alignment = 64;
        iface.u.AllocateLinearVideoMemory.pool      = gcvPOOL_DEFAULT;
        iface.u.AllocateLinearVideoMemory.type      = Type;

        /* Call kernel service. */
        gcmERR_BREAK(gcoOS_DeviceControl(
            Hardware->os, IOCTL_GCHAL_INTERFACE,
            &iface, sizeof(gcsHAL_INTERFACE),
            &iface, sizeof(gcsHAL_INTERFACE)
            ));

        /* Validate the return value. */
        gcmERR_BREAK(iface.status);

        /* Set the new parameters. */
        Hardware->tempBuffer.type                = Type;
        Hardware->tempBuffer.format              = Format->format;
        Hardware->tempBuffer.stride              = Width * Format->bitsPerPixel / 8;
        Hardware->tempBuffer.node.valid          = gcvFALSE;
        Hardware->tempBuffer.node.lockCount      = 0;
        Hardware->tempBuffer.node.lockedInKernel = gcvFALSE;
        Hardware->tempBuffer.node.logical        = gcvNULL;
        Hardware->tempBuffer.node.physical       = ~0U;

        Hardware->tempBuffer.node.pool
            = iface.u.AllocateLinearVideoMemory.pool;
        Hardware->tempBuffer.node.u.normal.node
            = iface.u.AllocateLinearVideoMemory.node;

        Hardware->tempBuffer.samples.x = 1;
        Hardware->tempBuffer.samples.y = 1;

        Hardware->tempBuffer.rect.left   = 0;
        Hardware->tempBuffer.rect.top    = 0;
        Hardware->tempBuffer.rect.right  = Width;
        Hardware->tempBuffer.rect.bottom = Height;
    }
    while (gcvFALSE);

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_FreeTemporarySurface
**
**  Free the temporary surface.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to a gcoHARDWARE object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_FreeTemporarySurface(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Synchronized
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Synchronized=%d", Hardware, Synchronized);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    do
    {
        /* Is there a surface allocated? */
        if (Hardware->tempBuffer.node.pool == gcvPOOL_UNKNOWN)
        {
            status = gcvSTATUS_OK;
            break;
        }

        /* Schedule deletion. */
        if (Synchronized)
        {
            gcmERR_BREAK(gcoHARDWARE_ScheduleVideoMemory(
                Hardware,
                &Hardware->tempBuffer.node
                ));
        }

        /* Not synchronized --> delete immediately. */
        else
        {
            gcsHAL_INTERFACE iface;

            /* Free the video memory. */
            iface.command = gcvHAL_FREE_VIDEO_MEMORY;
            iface.u.FreeVideoMemory.node
                = Hardware->tempBuffer.node.u.normal.node;

            /* Call kernel API. */
            gcmERR_BREAK(gcoHAL_Call(Hardware->hal, &iface));
        }

        /* Reset the temporary surface. */
        gcoOS_ZeroMemory(&Hardware->tempBuffer, sizeof(Hardware->tempBuffer));
        Hardware->tempBuffer.node.pool = gcvPOOL_UNKNOWN;
    }
    while (gcvFALSE);

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_ConvertPixel
**
**  Convert source pixel from source format to target pixel' target format.
**  The source format class should be either identical or convertible to
**  the target format class.
**
**  INPUT:
**
**      gctPOINTER SrcPixel, TrgPixel,
**          Pointers to source and target pixels.
**
**      gctUINT SrcBitOffset, TrgBitOffset
**          Bit offsets of the source and target pixels relative to their
**          respective pointers.
**
**      gcsSURF_FORMAT_INFO_PTR SrcFormat, TrgFormat
**          Pointers to source and target pixel format descriptors.
**
**      gcsBOUNDARY_PTR SrcBoundary, TrgBoundary
**          Pointers to optional boundary structures to verify the source
**          and target position. If the source is found to be beyond the
**          defined boundary, it will be assumed to be 0. If the target
**          is found to be beyond the defined boundary, the write will
**          be ignored. If boundary checking is not needed, gcvNULL can be
**          passed.
**
**  OUTPUT:
**
**      gctPOINTER TrgPixel + TrgBitOffset
**          Converted pixel.
*/
gceSTATUS gcoHARDWARE_ConvertPixel(
    IN gcoHARDWARE Hardware,
    IN gctPOINTER SrcPixel,
    OUT gctPOINTER TrgPixel,
    IN gctUINT SrcBitOffset,
    IN gctUINT TrgBitOffset,
    IN gcsSURF_FORMAT_INFO_PTR SrcFormat,
    IN gcsSURF_FORMAT_INFO_PTR TrgFormat,
    IN gcsBOUNDARY_PTR SrcBoundary,
    IN gcsBOUNDARY_PTR TrgBoundary
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x SrcPixel=0x%x TrgPixel=0x%x "
                    "SrcBitOffset=%d TrgBitOffset=%d SrcFormat=0x%x "
                    "TrgFormat=%d SrcBoundary=0x%x TrgBoundary=0x%x",
                    Hardware, SrcPixel, TrgPixel,
                    SrcBitOffset, TrgBitOffset, SrcFormat,
                    TrgFormat, SrcBoundary, TrgBoundary);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (SrcFormat->fmtClass == gcvFORMAT_CLASS_RGBA)
    {
        if (TrgFormat->fmtClass == gcvFORMAT_CLASS_RGBA)
        {
            _ConvertComponent(
                SrcPixel, TrgPixel,
                SrcBitOffset, TrgBitOffset,
                &SrcFormat->u.rgba.alpha,
                &TrgFormat->u.rgba.alpha,
                SrcBoundary, TrgBoundary,
                ~0U
                );

            _ConvertComponent(
                SrcPixel, TrgPixel,
                SrcBitOffset, TrgBitOffset,
                &SrcFormat->u.rgba.red,
                &TrgFormat->u.rgba.red,
                SrcBoundary, TrgBoundary,
                0
                );

            _ConvertComponent(
                SrcPixel, TrgPixel,
                SrcBitOffset, TrgBitOffset,
                &SrcFormat->u.rgba.green,
                &TrgFormat->u.rgba.green,
                SrcBoundary, TrgBoundary,
                0
                );

            _ConvertComponent(
                SrcPixel, TrgPixel,
                SrcBitOffset, TrgBitOffset,
                &SrcFormat->u.rgba.blue,
                &TrgFormat->u.rgba.blue,
                SrcBoundary, TrgBoundary,
                0
                );

            /* Success. */
            status = gcvSTATUS_OK;
        }

        else if (TrgFormat->fmtClass == gcvFORMAT_CLASS_LUMINANCE)
        {
            _ConvertComponent(
                SrcPixel, TrgPixel,
                SrcBitOffset, TrgBitOffset,
                &SrcFormat->u.rgba.red,
                &TrgFormat->u.lum.value,
                SrcBoundary, TrgBoundary,
                0
                );

            _ConvertComponent(
                SrcPixel, TrgPixel,
                SrcBitOffset, TrgBitOffset,
                &SrcFormat->u.rgba.alpha,
                &TrgFormat->u.lum.alpha,
                SrcBoundary, TrgBoundary,
                ~0U
                );

            /* Success. */
            status = gcvSTATUS_OK;
        }

        else if (TrgFormat->fmtClass == gcvFORMAT_CLASS_YUV)
        {
            gctUINT8 r, g, b;
            gctUINT8 y, u, v;

            /*
                Get RGB value.
            */

            _ConvertComponent(
                SrcPixel, &r, SrcBitOffset, 0,
                &SrcFormat->u.rgba.red, &gcvPIXEL_COMP_XXX8,
                SrcBoundary, gcvNULL, 0
                );

            _ConvertComponent(
                SrcPixel, &g, SrcBitOffset, 0,
                &SrcFormat->u.rgba.green, &gcvPIXEL_COMP_XXX8,
                SrcBoundary, gcvNULL, 0
                );

            _ConvertComponent(
                SrcPixel, &b, SrcBitOffset, 0,
                &SrcFormat->u.rgba.blue, &gcvPIXEL_COMP_XXX8,
                SrcBoundary, gcvNULL, 0
                );

            /*
                Convert to YUV.
            */

            gcoHARDWARE_RGB2YUV(
                 r,  g,  b,
                &y, &u, &v
                );

            /*
                Average Us and Vs for odd pixels.
            */
            if ((TrgFormat->interleaved & gcvCOMPONENT_ODD) != 0)
            {
                gctUINT8 curU, curV;

                _ConvertComponent(
                    TrgPixel, &curU, TrgBitOffset, 0,
                    &TrgFormat->u.yuv.u, &gcvPIXEL_COMP_XXX8,
                    TrgBoundary, gcvNULL, 0
                    );

                _ConvertComponent(
                    TrgPixel, &curV, TrgBitOffset, 0,
                    &TrgFormat->u.yuv.v, &gcvPIXEL_COMP_XXX8,
                    TrgBoundary, gcvNULL, 0
                    );


                u = (gctUINT8) (((gctUINT16) u + (gctUINT16) curU) >> 1);
                v = (gctUINT8) (((gctUINT16) v + (gctUINT16) curV) >> 1);
            }

            /*
                Convert to the final format.
            */

            _ConvertComponent(
                &y, TrgPixel, 0, TrgBitOffset,
                &gcvPIXEL_COMP_XXX8, &TrgFormat->u.yuv.y,
                gcvNULL, TrgBoundary, 0
                );

            _ConvertComponent(
                &u, TrgPixel, 0, TrgBitOffset,
                &gcvPIXEL_COMP_XXX8, &TrgFormat->u.yuv.u,
                gcvNULL, TrgBoundary, 0
                );

            _ConvertComponent(
                &v, TrgPixel, 0, TrgBitOffset,
                &gcvPIXEL_COMP_XXX8, &TrgFormat->u.yuv.v,
                gcvNULL, TrgBoundary, 0
                );

            /* Success. */
            status = gcvSTATUS_OK;
        }

        else
        {
            /* Not supported combination. */
            status = gcvSTATUS_NOT_SUPPORTED;
        }
    }

    else if (SrcFormat->fmtClass == gcvFORMAT_CLASS_YUV)
    {
        if (TrgFormat->fmtClass == gcvFORMAT_CLASS_YUV)
        {
            _ConvertComponent(
                SrcPixel, TrgPixel,
                SrcBitOffset, TrgBitOffset,
                &SrcFormat->u.yuv.y,
                &TrgFormat->u.yuv.y,
                SrcBoundary, TrgBoundary,
                0
                );

            _ConvertComponent(
                SrcPixel, TrgPixel,
                SrcBitOffset, TrgBitOffset,
                &SrcFormat->u.yuv.u,
                &TrgFormat->u.yuv.u,
                SrcBoundary, TrgBoundary,
                0
                );

            _ConvertComponent(
                SrcPixel, TrgPixel,
                SrcBitOffset, TrgBitOffset,
                &SrcFormat->u.yuv.v,
                &TrgFormat->u.yuv.v,
                SrcBoundary, TrgBoundary,
                0
                );

            /* Success. */
            status = gcvSTATUS_OK;
        }

        else if (TrgFormat->fmtClass == gcvFORMAT_CLASS_RGBA)
        {
            gctUINT8 y, u, v;
            gctUINT8 r, g, b;

            /*
                Get YUV value.
            */

            _ConvertComponent(
                SrcPixel, &y, SrcBitOffset, 0,
                &SrcFormat->u.yuv.y, &gcvPIXEL_COMP_XXX8,
                SrcBoundary, gcvNULL, 0
                );

            _ConvertComponent(
                SrcPixel, &u, SrcBitOffset, 0,
                &SrcFormat->u.yuv.u, &gcvPIXEL_COMP_XXX8,
                SrcBoundary, gcvNULL, 0
                );

            _ConvertComponent(
                SrcPixel, &v, SrcBitOffset, 0,
                &SrcFormat->u.yuv.v, &gcvPIXEL_COMP_XXX8,
                SrcBoundary, gcvNULL, 0
                );

            /*
                Convert to RGB.
            */

            gcoHARDWARE_YUV2RGB(
                 y,  u,  v,
                &r, &g, &b
                );

            /*
                Convert to the final format.
            */

            _ConvertComponent(
                gcvNULL, TrgPixel, 0, TrgBitOffset,
                gcvNULL, &TrgFormat->u.rgba.alpha,
                gcvNULL, TrgBoundary, ~0U
                );

            _ConvertComponent(
                &r, TrgPixel, 0, TrgBitOffset,
                &gcvPIXEL_COMP_XXX8, &TrgFormat->u.rgba.red,
                gcvNULL, TrgBoundary, 0
                );

            _ConvertComponent(
                &g, TrgPixel, 0, TrgBitOffset,
                &gcvPIXEL_COMP_XXX8, &TrgFormat->u.rgba.green,
                gcvNULL, TrgBoundary, 0
                );

            _ConvertComponent(
                &b, TrgPixel, 0, TrgBitOffset,
                &gcvPIXEL_COMP_XXX8, &TrgFormat->u.rgba.blue,
                gcvNULL, TrgBoundary, 0
                );

            /* Success. */
            status = gcvSTATUS_OK;
        }

        else
        {
            /* Not supported combination. */
            status = gcvSTATUS_NOT_SUPPORTED;
        }
    }

    else if (SrcFormat->fmtClass == gcvFORMAT_CLASS_INDEX)
    {
        if (TrgFormat->fmtClass == gcvFORMAT_CLASS_INDEX)
        {
            _ConvertComponent(
                SrcPixel, TrgPixel,
                SrcBitOffset, TrgBitOffset,
                &SrcFormat->u.index.value,
                &TrgFormat->u.index.value,
                SrcBoundary, TrgBoundary,
                0
                );

            /* Success. */
            status = gcvSTATUS_OK;
        }

        else
        {
            /* Not supported combination. */
            status = gcvSTATUS_NOT_SUPPORTED;
        }
    }

    else if (SrcFormat->fmtClass == gcvFORMAT_CLASS_LUMINANCE)
    {
        if (TrgFormat->fmtClass == gcvFORMAT_CLASS_LUMINANCE)
        {
            _ConvertComponent(
                SrcPixel, TrgPixel,
                SrcBitOffset, TrgBitOffset,
                &SrcFormat->u.lum.alpha,
                &TrgFormat->u.lum.alpha,
                SrcBoundary, TrgBoundary,
                ~0U
                );

            _ConvertComponent(
                SrcPixel, TrgPixel,
                SrcBitOffset, TrgBitOffset,
                &SrcFormat->u.lum.value,
                &TrgFormat->u.lum.value,
                SrcBoundary, TrgBoundary,
                0
                );

            /* Success. */
            status = gcvSTATUS_OK;
        }

        else
        {
            /* Not supported combination. */
            status = gcvSTATUS_NOT_SUPPORTED;
        }
    }

    else if (SrcFormat->fmtClass == gcvFORMAT_CLASS_BUMP)
    {
        if (TrgFormat->fmtClass == gcvFORMAT_CLASS_BUMP)
        {
            _ConvertComponent(
                SrcPixel, TrgPixel,
                SrcBitOffset, TrgBitOffset,
                &SrcFormat->u.bump.alpha,
                &TrgFormat->u.bump.alpha,
                SrcBoundary, TrgBoundary,
                ~0U
                );

            _ConvertComponent(
                SrcPixel, TrgPixel,
                SrcBitOffset, TrgBitOffset,
                &SrcFormat->u.bump.l,
                &TrgFormat->u.bump.l,
                SrcBoundary, TrgBoundary,
                0
                );

            _ConvertComponent(
                SrcPixel, TrgPixel,
                SrcBitOffset, TrgBitOffset,
                &SrcFormat->u.bump.v,
                &TrgFormat->u.bump.v,
                SrcBoundary, TrgBoundary,
                0
                );

            _ConvertComponent(
                SrcPixel, TrgPixel,
                SrcBitOffset, TrgBitOffset,
                &SrcFormat->u.bump.u,
                &TrgFormat->u.bump.u,
                SrcBoundary, TrgBoundary,
                0
                );

            _ConvertComponent(
                SrcPixel, TrgPixel,
                SrcBitOffset, TrgBitOffset,
                &SrcFormat->u.bump.q,
                &TrgFormat->u.bump.q,
                SrcBoundary, TrgBoundary,
                0
                );

            _ConvertComponent(
                SrcPixel, TrgPixel,
                SrcBitOffset, TrgBitOffset,
                &SrcFormat->u.bump.w,
                &TrgFormat->u.bump.w,
                SrcBoundary, TrgBoundary,
                0
                );

            /* Success. */
            status = gcvSTATUS_OK;
        }

        else
        {
            /* Not supported combination. */
            status = gcvSTATUS_NOT_SUPPORTED;
        }
    }

    else if (SrcFormat->fmtClass == gcvFORMAT_CLASS_DEPTH)
    {
        if (TrgFormat->fmtClass == gcvFORMAT_CLASS_DEPTH)
        {
            _ConvertComponent(
                SrcPixel, TrgPixel,
                SrcBitOffset, TrgBitOffset,
                &SrcFormat->u.depth.depth,
                &TrgFormat->u.depth.depth,
                SrcBoundary, TrgBoundary,
                ~0U
                );

            _ConvertComponent(
                SrcPixel, TrgPixel,
                SrcBitOffset, TrgBitOffset,
                &SrcFormat->u.depth.stencil,
                &TrgFormat->u.depth.stencil,
                SrcBoundary, TrgBoundary,
                0
                );

            /* Success. */
            status = gcvSTATUS_OK;
        }

        else
        {
            /* Not supported combination. */
            status = gcvSTATUS_NOT_SUPPORTED;
        }
    }

    else
    {
        /* Not supported class. */
        status = gcvSTATUS_NOT_SUPPORTED;
    }

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_CopyPixels
**
**  Copy a rectangular area from one surface to another with format conversion.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcsSURF_INFO_PTR Source
**          Pointer to the source surface descriptor.
**
**      gcsSURF_INFO_PTR Target
**          Pointer to the destination surface descriptor.
**
**      gctINT SourceX, SourceY
**          Source surface origin.
**
**      gctINT TargetX, TargetY
**          Target surface origin.
**
**      gctINT Width, Height
**          The size of the area. If Height is negative, the area will
**          be copied with a vertical flip.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_CopyPixels(
    IN gcoHARDWARE Hardware,
    IN gcsSURF_INFO_PTR Source,
    IN gcsSURF_INFO_PTR Target,
    IN gctINT SourceX,
    IN gctINT SourceY,
    IN gctINT TargetX,
    IN gctINT TargetY,
    IN gctINT Width,
    IN gctINT Height
    )
{
    gceSTATUS status;
    gcsSURF_INFO_PTR source;
    gctPOINTER memory = gcvNULL;
    gcsSURF_FORMAT_INFO_PTR srcFormat[2];
    gcsSURF_FORMAT_INFO_PTR trgFormat[2];

    gcmHEADER_ARG("Hardware=0x%x Source=0x%x Target=0x%x "
                    "SourceX=%d SourceY=%d TargetX=%d "
                    "TargetY=%d Width=%d Height=%d",
                    Hardware, Source, Target,
                    SourceX, SourceY, TargetX,
                    TargetY, Width, Height);

    /* Get surface formats. */
    gcmONERROR(
        gcoSURF_QueryFormat(Source->format, srcFormat));

    gcmONERROR(
        gcoSURF_QueryFormat(Target->format, trgFormat));

    /* Check if the source has multi-sampling or super-tiling. */
    if ((Source->samples.x > 1)
    ||  (Source->samples.y > 1)
    ||  Source->superTiled
    )
    {
        gcsPOINT zero, size;
        gceORIENTATION orientation;

        /* Create a temporary surface. */
        gcmONERROR(
            gcoHARDWARE_AllocateTemporarySurface(Hardware,
                                                 gcmALIGN(Source->rect.right  / Source->samples.x, 16),
                                                 gcmALIGN(Source->rect.bottom / Source->samples.y,  4),
                                                 srcFormat[0],
                                                 gcvSURF_BITMAP));

        /* Use same orientation as source. */
        orientation = Hardware->tempBuffer.orientation;
        Hardware->tempBuffer.orientation = Source->orientation;

        /* Lock it. */
        gcmONERROR(
            gcoHARDWARE_Lock(Hardware,
                             &Hardware->tempBuffer.node,
                             gcvNULL,
                             &memory));

        /* Resolve the source into the temporary surface. */
        zero.x = 0;
        zero.y = 0;
        size.x = Hardware->tempBuffer.rect.right;
        size.y = Hardware->tempBuffer.rect.bottom;

        gcmONERROR(
            _ResolveRect(Hardware,
                         Source,
                         &Hardware->tempBuffer,
                         &zero,
                         &zero,
                         &size));

        /* Stall the hardware.  Note that _ResolveRect already performed a
        ** commit. */
        gcmONERROR(
            gcoHARDWARE_Stall(Hardware));

        /* Restore orientation. */
        Hardware->tempBuffer.orientation = orientation;


        /* Use temporary buffer as source. */
        source = &Hardware->tempBuffer;
    }
    else
    {
        /* Use source as-is. */
        source = Source;
    }

    do
    {
        /***********************************************************************
        ** Local variable definitions.
        */

        /* Tile dimensions. */
        gctINT32 srcTileWidth, srcTileHeight, srcTileSize;
        gctINT32 trgTileWidth, trgTileHeight, trgTileSize;

        /* Pixel sizes. */
        gctINT srcPixelSize, trgPixelSize;

        /* Walking boundaries. */
        gctINT trgX1, trgY1, trgX2, trgY2;
        gctINT srcX1, srcY1, srcX2, srcY2;

        /* Source step. */
        gctINT srcStepX, srcStepY;

        /* Coordinate alignment. */
        gctUINT srcAlignX, srcAlignY;
        gctUINT trgAlignX, trgAlignY;

        /* Pixel group sizes. */
        gctUINT srcGroupX, srcGroupY;
        gctUINT trgGroupX, trgGroupY;

        /* Line surface offsets. */
        gctUINT srcLineOffset, trgLineOffset;

        /* Counts to keep track of when to add long vs. short offsets. */
        gctINT srcLeftPixelCount, srcMidPixelCount;
        gctINT srcTopLineCount, srcMidLineCount;
        gctINT trgLeftPixelCount, trgMidPixelCount;
        gctINT trgTopLineCount, trgMidLineCount;
        gctINT srcPixelCount, trgPixelCount;
        gctINT srcLineCount, trgLineCount;

        /* Long and short offsets. */
        gctINT srcShortStride, srcLongStride;
        gctINT srcShortOffset, srcLongOffset;
        gctINT trgShortStride, trgLongStride;
        gctINT trgShortOffset, trgLongOffset;

        /* Direct copy flag and the intermediate format. */
        gctBOOL directCopy;
        gcsSURF_FORMAT_INFO_PTR intFormat[2];

        /* Boundary checking. */
        gcsBOUNDARY srcBoundary;
        gcsBOUNDARY trgBoundary;
        gcsBOUNDARY_PTR srcBoundaryPtr;
        gcsBOUNDARY_PTR trgBoundaryPtr;

        /***********************************************************************
        ** Get surface format details.
        */

        /* Compute pixel sizes. */
        srcPixelSize = srcFormat[0]->bitsPerPixel / 8;
        trgPixelSize = trgFormat[0]->bitsPerPixel / 8;

        /***********************************************************************
        ** Validate inputs.
        */

        /* Verify that the surfaces are locked. */
        gcmVERIFY_LOCK(source);
        gcmVERIFY_LOCK(Target);

        /* Check support. */
        if (Width < 0)
        {
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        if ((srcFormat[0]->interleaved) && (source->samples.x != 1))
        {
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        if ((trgFormat[0]->interleaved) && (Target->samples.x != 1))
        {
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /***********************************************************************
        ** Determine the type of operation and the intermediate format.
        */

        directCopy
            =  (source->samples.x == Target->samples.x)
            && (source->samples.y == Target->samples.y);

        if (!directCopy)
        {
            gceSURF_FORMAT intermediateFormat
                = (srcFormat[0]->fmtClass == gcvFORMAT_CLASS_RGBA)
                    ? gcvSURF_A8R8G8B8
                    : srcFormat[0]->format;

            gcmERR_BREAK(gcoSURF_QueryFormat(intermediateFormat, intFormat));
        }

        /***********************************************************************
        ** Determine the size of the pixel groups.
        */

        srcGroupX = (srcFormat[0]->interleaved || (source->samples.x != 1))
            ? 2
            : 1;

        trgGroupX = (trgFormat[0]->interleaved || (Target->samples.x != 1))
            ? 2
            : 1;

        srcGroupY = source->samples.y;
        trgGroupY = Target->samples.y;

        /* Determine coordinate alignments. */
        srcAlignX = ~(srcGroupX - 1);
        srcAlignY = ~(srcGroupY - 1);
        trgAlignX = ~(trgGroupX - 1);
        trgAlignY = ~(trgGroupY - 1);

        /***********************************************************************
        ** Determine tile parameters.
        */

        gcmERR_BREAK(gcoHARDWARE_GetSurfaceTileSize(
            source, &srcTileWidth, &srcTileHeight
            ));

        gcmERR_BREAK(gcoHARDWARE_GetSurfaceTileSize(
            Target, &trgTileWidth, &trgTileHeight
            ));

        srcTileSize = srcTileWidth * srcTileHeight * srcPixelSize;
        trgTileSize = trgTileWidth * trgTileHeight * trgPixelSize;

        /* Determine pixel and line counts per tile. */
        srcMidPixelCount = (srcTileWidth  == 1) ? 1 : srcTileWidth  / srcGroupX;
        srcMidLineCount  = (srcTileHeight == 1) ? 1 : srcTileHeight / srcGroupY;
        trgMidPixelCount = (trgTileWidth  == 1) ? 1 : trgTileWidth  / trgGroupX;
        trgMidLineCount  = (trgTileHeight == 1) ? 1 : trgTileHeight / trgGroupY;

        /***********************************************************************
        ** Determine the initial horizontal source coordinates.
        */

        srcX1    = source->samples.x *  SourceX;
        srcX2    = source->samples.x * (SourceX + Width);
        srcStepX = source->samples.x;

        /* Pixels left to go before using the long offset. */
        srcLeftPixelCount
            = ((~((gctUINT) srcX1)) & (srcTileWidth - 1)) / srcGroupX + 1;

        srcShortOffset
            = srcPixelSize * srcGroupX;

        srcLongOffset
            = srcTileSize - srcPixelSize * (srcTileWidth - srcGroupX);

        /***********************************************************************
        ** Determine the initial vertical source coordinates.
        */

        if (Height < 0)
        {
            srcY1    =            source->samples.y * (SourceY - Height - 1);
            srcY2    =            source->samples.y * (SourceY          - 1);
            srcStepY = - (gctINT) source->samples.y;

            /* Lines left to go before using the long stride. */
            srcTopLineCount
                = (((gctUINT) srcY1) & (srcTileHeight - 1)) / srcGroupY + 1;

            srcLongStride = - (gctINT) ((srcTileHeight == 1)
                ? source->stride * srcGroupY
                : source->stride * srcTileHeight
                  - srcTileWidth * srcPixelSize * (srcTileHeight - srcGroupY));

            srcShortStride = (srcTileHeight == 1)
                ? srcLongStride
                : - (gctINT) srcTileWidth * srcPixelSize * srcGroupY;

            /* Determine the vertical target range. */
            trgY1 = Target->samples.y *  TargetY;
            trgY2 = Target->samples.y * (TargetY - Height);
        }
        else
        {
            srcY1    = source->samples.y *  SourceY;
            srcY2    = source->samples.y * (SourceY + Height);
            srcStepY = source->samples.y;

            /* Lines left to go before using the long stride. */
            srcTopLineCount
                = ((~((gctUINT) srcY1)) & (srcTileHeight - 1)) / srcGroupY + 1;

            srcLongStride = (srcTileHeight == 1)
                ? source->stride * srcGroupY
                : source->stride * srcTileHeight
                  - srcTileWidth * srcPixelSize * (srcTileHeight - srcGroupY);

            srcShortStride = (srcTileHeight == 1)
                ? srcLongStride
                : srcTileWidth * srcPixelSize * srcGroupY;

            /* Determine the vertical target range. */
            trgY1 = Target->samples.y *  TargetY;
            trgY2 = Target->samples.y * (TargetY + Height);
        }

        /***********************************************************************
        ** Determine the initial target coordinates.
        */

        trgX1 = Target->samples.x *  TargetX;
        trgX2 = Target->samples.x * (TargetX + Width);

        /* Pixels left to go before using the long offset. */
        trgLeftPixelCount
            = ((~((gctUINT) trgX1)) & (trgTileWidth - 1)) / trgGroupX + 1;

        /* Lines left to go before using the long stride. */
        trgTopLineCount
            = ((~((gctUINT) trgY1)) & (trgTileHeight - 1)) / trgGroupY + 1;

        trgShortOffset = trgPixelSize * trgGroupX;
        trgLongOffset = trgTileSize - trgPixelSize * (trgTileWidth - trgGroupX);

        trgLongStride = (trgTileHeight == 1)
            ? Target->stride * trgGroupY
            : Target->stride * trgTileHeight
              - trgTileWidth * trgPixelSize * (trgTileHeight - trgGroupY);

        trgShortStride = (trgTileHeight == 1)
            ? trgLongStride
            : trgTileWidth * trgPixelSize * trgGroupY;

        /***********************************************************************
        ** Setup the boundary checking.
        */

        if ((srcX1 < 0) || (srcX2 >= source->rect.right) ||
            (srcY1 < 0) || (srcY2 >= source->rect.bottom))
        {
            srcBoundaryPtr = &srcBoundary;
            srcBoundary.width  = source->rect.right;
            srcBoundary.height = source->rect.bottom;
        }
        else
        {
            srcBoundaryPtr = gcvNULL;
        }

        if ((trgX1 < 0) || (trgX2 > Target->rect.right) ||
            (trgY1 < 0) || (trgY2 > Target->rect.bottom))
        {
            trgBoundaryPtr = &trgBoundary;
            trgBoundary.width  = Target->rect.right;
            trgBoundary.height = Target->rect.bottom;
        }
        else
        {
            trgBoundaryPtr = gcvNULL;
        }

        /***********************************************************************
        ** Compute the source and target initial offsets.
        */

        srcLineOffset

            /* Skip full tile lines. */
            = ((gctINT) (srcY1 & srcAlignY & ~(srcTileHeight - 1)))
                * source->stride

            /* Skip full tiles in the same line. */
            + ((((gctINT) (srcX1 & srcAlignX & ~(srcTileWidth - 1)))
                * srcTileHeight * srcFormat[0]->bitsPerPixel) >> 3)

            /* Skip full rows within the target tile. */
            + ((((gctINT) (srcY1 & srcAlignY & (srcTileHeight - 1)))
                * srcTileWidth * srcFormat[0]->bitsPerPixel) >> 3)

            /* Skip pixels on the target row. */
            + (((gctINT) (srcX1 & srcAlignX & (srcTileWidth - 1)))
                * srcFormat[0]->bitsPerPixel >> 3);

        trgLineOffset

            /* Skip full tile lines. */
            = ((gctINT) (trgY1 & trgAlignY & ~(trgTileHeight - 1)))
                * Target->stride

            /* Skip full tiles in the same line. */
            + ((((gctINT) (trgX1 & trgAlignX & ~(trgTileWidth - 1)))
                * trgTileHeight * trgFormat[0]->bitsPerPixel) >> 3)

            /* Skip full rows within the target tile. */
            + ((((gctINT) (trgY1 & trgAlignY & (trgTileHeight - 1)))
                * trgTileWidth * trgFormat[0]->bitsPerPixel) >> 3)

            /* Skip pixels on the target row. */
            + (((gctINT) (trgX1 & trgAlignX & (trgTileWidth - 1)))
                * trgFormat[0]->bitsPerPixel >> 3);

        /* Initialize line counts. */
        srcLineCount = srcTopLineCount;
        trgLineCount = trgTopLineCount;

        /* Initialize the vertical coordinates. */
        srcBoundary.y = srcY1;
        trgBoundary.y = trgY1;

        /* Go through all lines. */
        while (srcBoundary.y != srcY2)
        {
            /* Initialize the initial pixel addresses. */
            gctUINT8_PTR srcPixelAddress = source->node.logical + srcLineOffset;
            gctUINT8_PTR trgPixelAddress = Target->node.logical + trgLineOffset;

            /* Initialize pixel counts. */
            srcPixelCount = srcLeftPixelCount;
            trgPixelCount = trgLeftPixelCount;

            /* Initialize the horizontal coordinates. */
            srcBoundary.x = srcX1;
            trgBoundary.x = trgX1;

            /* Go through all columns. */
            while (srcBoundary.x != srcX2)
            {
                /* Determine oddity of the pixels. */
                gctINT srcOdd = srcBoundary.x & srcFormat[0]->interleaved;
                gctINT trgOdd = trgBoundary.x & trgFormat[0]->interleaved;

                /* Direct copy without resampling. */
                if (directCopy)
                {
                    gctUINT8 x, y;

                    for (y = 0; y < source->samples.y; y++)
                    {
                        /* Determine the vertical pixel offsets. */
                        gctUINT srcVerOffset = y * srcTileWidth * srcPixelSize;
                        gctUINT trgVerOffset = y * trgTileWidth * trgPixelSize;

                        for (x = 0; x < source->samples.x; x++)
                        {
                            /* Determine the horizontal pixel offsets. */
                            gctUINT srcHorOffset = x * srcPixelSize;
                            gctUINT trgHorOffset = x * trgPixelSize;

                            /* Convert pixel. */
                            gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                                Hardware,
                                srcPixelAddress + srcVerOffset + srcHorOffset,
                                trgPixelAddress + trgVerOffset + trgHorOffset,
                                0, 0,
                                srcFormat[srcOdd],
                                trgFormat[trgOdd],
                                srcBoundaryPtr,
                                trgBoundaryPtr
                                ));
                        }

                        /* Error? */
                        if (gcmIS_ERROR(status))
                        {
                            break;
                        }
                    }

                    /* Error? */
                    if (gcmIS_ERROR(status))
                    {
                        break;
                    }
                }

                /* Need to resample. */
                else
                {
                    gctUINT32 data[4];

                    /* Read the top left pixel. */
                    gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                        Hardware,
                        srcPixelAddress,
                        &data[0],
                        0, 0,
                        srcFormat[0],
                        intFormat[0],
                        srcBoundaryPtr,
                        trgBoundaryPtr
                        ));

                    /* Replicate horizotnally? */
                    if (source->samples.x == 1)
                    {
                        data[1] = data[0];

                        /* Replicate vertically as well? */
                        if (source->samples.y == 1)
                        {
                            data[2] = data[0];
                        }
                        else
                        {
                            /* Read the bottom left pixel. */
                            gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                                Hardware,
                                srcPixelAddress + srcTileWidth * srcPixelSize,
                                &data[2],
                                0, 0,
                                srcFormat[0],
                                intFormat[0],
                                srcBoundaryPtr,
                                trgBoundaryPtr
                                ));
                        }

                        /* Replicate the bottom right. */
                        data[3] = data[2];
                    }

                    else
                    {
                        /* Read the top right pixel. */
                        gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                            Hardware,
                            srcPixelAddress + srcPixelSize,
                            &data[1],
                            0, 0,
                            srcFormat[0],
                            intFormat[0],
                            srcBoundaryPtr,
                            trgBoundaryPtr
                            ));

                        /* Replicate vertically as well? */
                        if (source->samples.y == 1)
                        {
                            data[2] = data[0];
                            data[3] = data[1];
                        }
                        else
                        {
                            /* Read the bottom left pixel. */
                            gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                                Hardware,
                                srcPixelAddress + srcTileWidth * srcPixelSize,
                                &data[2],
                                0, 0,
                                srcFormat[0],
                                intFormat[0],
                                srcBoundaryPtr,
                                trgBoundaryPtr
                                ));

                            /* Read the bottom right pixel. */
                            gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                                Hardware,
                                srcPixelAddress + (srcTileWidth + 1) * srcPixelSize,
                                &data[3],
                                0, 0,
                                srcFormat[0],
                                intFormat[0],
                                srcBoundaryPtr,
                                trgBoundaryPtr
                                ));
                        }
                    }

                    /* Compute the destination. */
                    if (Target->samples.x == 1)
                    {
                        if (Target->samples.y == 1)
                        {
                            /* Average four sources. */
                            gctUINT32 dstPixel = _Average4Colors(
                                data[0], data[1], data[2], data[3]
                                );

                            /* Convert pixel. */
                            gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                                Hardware,
                                &dstPixel,
                                trgPixelAddress,
                                0, 0,
                                intFormat[0],
                                trgFormat[0],
                                srcBoundaryPtr,
                                trgBoundaryPtr
                                ));
                        }
                        else
                        {
                            /* Average two horizontal pairs of sources. */
                            gctUINT32 dstTop = _Average2Colors(
                                data[0], data[1]
                                );

                            gctUINT32 dstBottom = _Average2Colors(
                                data[2], data[3]
                                );

                            /* Convert the top pixel. */
                            gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                                Hardware,
                                &dstTop,
                                trgPixelAddress,
                                0, 0,
                                intFormat[0],
                                trgFormat[0],
                                srcBoundaryPtr,
                                trgBoundaryPtr
                                ));

                            /* Convert the bottom pixel. */
                            gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                                Hardware,
                                &dstBottom,
                                trgPixelAddress + trgTileWidth * trgPixelSize,
                                0, 0,
                                intFormat[0],
                                trgFormat[0],
                                srcBoundaryPtr,
                                trgBoundaryPtr
                                ));
                        }
                    }
                    else
                    {
                        if (Target->samples.y == 1)
                        {
                            /* Average two vertical pairs of sources. */
                            gctUINT32 dstLeft = _Average2Colors(
                                data[0], data[2]
                                );

                            gctUINT32 dstRight = _Average2Colors(
                                data[1], data[3]
                                );

                            /* Convert the left pixel. */
                            gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                                Hardware,
                                &dstLeft,
                                trgPixelAddress,
                                0, 0,
                                intFormat[0],
                                trgFormat[0],
                                srcBoundaryPtr,
                                trgBoundaryPtr
                                ));

                            /* Convert the right pixel. */
                            gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                                Hardware,
                                &dstRight,
                                trgPixelAddress + trgPixelSize,
                                0, 0,
                                intFormat[0],
                                trgFormat[0],
                                srcBoundaryPtr,
                                trgBoundaryPtr
                                ));
                        }
                        else
                        {
                            /* Copy four sources as they are. */
                            gctUINT8 x, y;

                            for (y = 0; y < 2; y++)
                            {
                                /* Determine the vertical pixel offset. */
                                gctUINT trgVerOffset = y * trgTileWidth * trgPixelSize;

                                for (x = 0; x < 2; x++)
                                {
                                    /* Determine the horizontal pixel offset. */
                                    gctUINT trgHorOffset = x * trgPixelSize;

                                    /* Convert pixel. */
                                    gcmERR_BREAK(gcoHARDWARE_ConvertPixel(
                                        Hardware,
                                        &data[x + y * 2],
                                        trgPixelAddress + trgVerOffset + trgHorOffset,
                                        0, 0,
                                        intFormat[0],
                                        trgFormat[0],
                                        srcBoundaryPtr,
                                        trgBoundaryPtr
                                        ));
                                }

                                /* Error? */
                                if (gcmIS_ERROR(status))
                                {
                                    break;
                                }
                            }

                            /* Error? */
                            if (gcmIS_ERROR(status))
                            {
                                break;
                            }
                        }
                    }
                }

                /* Advance to the next column. */
                if (srcOdd || !srcFormat[0]->interleaved)
                {
                    if (--srcPixelCount == 0)
                    {
                        srcPixelAddress += srcLongOffset;
                        srcPixelCount    = srcMidPixelCount;
                    }
                    else
                    {
                        srcPixelAddress += srcShortOffset;
                    }
                }

                if (trgOdd || !trgFormat[0]->interleaved)
                {
                    if (--trgPixelCount == 0)
                    {
                        trgPixelAddress += trgLongOffset;
                        trgPixelCount    = trgMidPixelCount;
                    }
                    else
                    {
                        trgPixelAddress += trgShortOffset;
                    }
                }

                srcBoundary.x += srcStepX;
                trgBoundary.x += Target->samples.x;
            }

            /* Error? */
            if (gcmIS_ERROR(status))
            {
                break;
            }

            /* Advance to the next line. */
            if (--srcLineCount == 0)
            {
                srcLineOffset += srcLongStride;
                srcLineCount   = srcMidLineCount;
            }
            else
            {
                srcLineOffset += srcShortStride;
            }

            if (--trgLineCount == 0)
            {
                trgLineOffset += trgLongStride;
                trgLineCount   = trgMidLineCount;
            }
            else
            {
                trgLineOffset += trgShortStride;
            }

            srcBoundary.y += srcStepY;
            trgBoundary.y += Target->samples.y;
        }
    }
    while (gcvFALSE);

    if (Target->node.pool == gcvPOOL_USER)
    {
        gctUINT offset, bytes, size;
        gctUINT step;

        if (source->type == gcvSURF_BITMAP)
        {
            int bpp = srcFormat[0]->bitsPerPixel / 8;
            offset  = SourceY * source->stride + SourceX * bpp;
            size    = Width * gcmABS(Height) * bpp;
            bytes   = Width * bpp;
            step    = source->stride;

            if (bytes == step)
            {
                bytes *= gcmABS(Height);
                step   = 0;
            }
        }
        else
        {
            offset = 0;
            size   = source->stride * source->alignedHeight;
            bytes  = size;
            step   = 0;
        }

        while (size > 0)
        {
            gcmDUMP_BUFFER(Hardware->os,
                           "verify",
                           source->node.physical,
                           source->node.logical,
                           offset,
                           bytes);

            offset += step;
            size   -= bytes;
        }
    }
    else
    {
        gcmDUMP_BUFFER(Hardware->os,
                       "memory",
                       Target->node.physical,
                       Target->node.logical,
                       0,
                       Target->size);
    }

OnError:
    if (memory != gcvNULL)
    {
        /* Unlock the temporay buffer. */
        gcmVERIFY_OK(
            gcoHARDWARE_Unlock(Hardware,
                               &Hardware->tempBuffer.node,
                               Hardware->tempBuffer.type));
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_FlushPipe
**
**  Flush the current graphics pipe.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_FlushPipe(
    IN gcoHARDWARE Hardware
    )
{
    gceSTATUS status;
    gctUINT32 flush;

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    do
    {
        if (Hardware->context->currentPipe == 0x1)
        {
            /* Flush 2D cache. */
            flush = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3))) | (((gctUINT32) (0x1&((gctUINT32)((((1?3:3)-(0?3:3)+1)==32)?~0:(~(~0<<((1?3:3)-(0?3:3)+1)))))))<<(0?3:3)));
        }
        else
        {
            /* Flush Z and Color caches. */
            flush = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) | (((gctUINT32) (0x1&((gctUINT32)((((1?0:0)-(0?0:0)+1)==32)?~0:(~(~0<<((1?0:0)-(0?0:0)+1)))))))<<(0?0:0)))|
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))) | (((gctUINT32) (0x1&((gctUINT32)((((1?1:1)-(0?1:1)+1)==32)?~0:(~(~0<<((1?1:1)-(0?1:1)+1)))))))<<(0?1:1)));
        }

        /* Append flush state in command buffer. */
        gcmERR_BREAK(
            gcoHARDWARE_LoadState32(Hardware, 0x0380C, flush));

        if (Hardware->chipModel == gcv700)
        {
            /* Flush the L2 cache. */
            gcmERR_BREAK(gcoHARDWARE_FlushL2Cache(Hardware));
        }
    }
    while (gcvFALSE);

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_Semaphore
**
**  Send sempahore and stall until sempahore is signalled.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gceWHERE From
**          Semaphore source.
**
**      GCHWERE To
**          Sempahore destination.
**
**      gceHOW How
**          What to do.  Can be a one of the following values:
**
**              gcvHOW_SEMAPHORE            Send sempahore.
**              gcvHOW_STALL                Stall.
**              gcvHOW_SEMAPHORE_STALL  Send semaphore and stall.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_Semaphore(
    IN gcoHARDWARE Hardware,
    IN gceWHERE From,
    IN gceWHERE To,
    IN gceHOW How
    )
{
    gceSTATUS status;
    gctUINT32 * memory;
    gctBOOL semaphore = (How == gcvHOW_SEMAPHORE) || (How == gcvHOW_SEMAPHORE_STALL);
    gctBOOL stall = (How == gcvHOW_STALL) || (How == gcvHOW_SEMAPHORE_STALL);
    gctUINT32 source, destination;

    gcmHEADER_ARG("Hardware=0x%x From=%d To=%d How=%d",
                    Hardware, From, To, How);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Special case for RASTER to PIEL_ENGINE semaphores. */
    if ((From == gcvWHERE_RASTER) && (To == gcvWHERE_PIXEL))
    {
        if (How == gcvHOW_SEMAPHORE)
        {
            /* Set flag so we can issue a semaphore/stall when required. */
            Hardware->stallPrimitive = gcvTRUE;

            /* Success. */
            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }

        else if (How == gcvHOW_STALL)
        {
            /* Make sure we do a semaphore/stall here. */
            semaphore = gcvTRUE;
            stall     = gcvTRUE;
        }
    }

    switch (From)
    {
    case gcvWHERE_COMMAND:
        source = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)));
        break;

    case gcvWHERE_RASTER:
        source = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | (((gctUINT32) (0x05 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)));

        /* We need to stall on the next primitive if this is only a
           semaphore. */
        Hardware->stallPrimitive = (How == gcvHOW_SEMAPHORE);
        break;

    default:
        gcmASSERT(gcvFALSE);
        gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_ARGUMENT);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    gcmASSERT(To == gcvWHERE_PIXEL);
    destination = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));

    /* Reserve command buffer space. */
    status = gcoBUFFER_Reserve(Hardware->buffer,
                               ((semaphore & stall) ? 2 : 1) * 8,
                               gcvTRUE,
                               gcvNULL,
                               (gctPOINTER *) &memory);

    if (gcmIS_ERROR(status))
    {
        /* Error. */
        gcmFOOTER();
        return status;
    }

    if (semaphore)
    {
        /* Send sempahore token. */
        memory[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))|
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))|
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

        memory[1] = source | destination;

        /* Dump the semaphore. */
        gcmDUMP(Hardware->os,
                "@[semaphore 0x%08X 0x%08X]",
                source, destination);

        /* Point to stall command is required. */
        memory += 2;
    }

    if (stall)
    {
        if (From == gcvWHERE_COMMAND)
        {
            /* Stall command processor until semaphore is signalled. */
            memory[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));
        }
        else
        {
            /* Send stall token. */
            memory[0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))|
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))|
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (0x0F00) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));
        }

        memory[1] = source | destination;

        /* Dump the stall. */
        gcmDUMP(Hardware->os, "@[stall 0x%08X 0x%08X]", source, destination);
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**  gcoHARDWARE_SetAntiAlias
**
**  Enable or disable anti-aliasing.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctBOOL Enable
**          Enable anti-aliasing when gcvTRUE and disable anti-aliasing when
**          gcvFALSE.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_SetAntiAlias(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gctUINT32 value;
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Store value. */
    Hardware->sampleMask = Enable ? 0xF : 0x0;

    if (Enable)
    {
        /* Enable anti-aliasing. */
        value = (    ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4))) | (((gctUINT32) ((gctUINT32) ((Hardware->sampleEnable)) & ((gctUINT32) ((((1 ? 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8))));
    }
    else
    {
        /* Disable anti-aliasng. */
        value = (    ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4))) | (((gctUINT32) ((gctUINT32) ((0)) & ((gctUINT32) ((((1 ? 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4))) &((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8))));
    }

    /* Set the anti-alias enable bits. */
    status = gcoHARDWARE_LoadState32(Hardware,
                                   0x03818,
                                   value);

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_SetDither
**
**  Enable or disable dithering.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gctBOOL Enable
**          gcvTRUE to enable dithering or gcvFALSE to disable it.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_SetDither(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Set dithering values. */
    Hardware->dither[0] = Enable ? 0x6E4CA280 : ~0;
    Hardware->dither[1] = Enable ? 0x5D7F91B3 : ~0;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_AlignToTile
**
**  Align the specified width and height to tile boundaries.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gceSURF_TYPE Type
**          Type of alignment.
**
**      gctUINT32 * Width
**          Pointer to the width to be aligned.  If 'Width' is gcvNULL, no width
**          will be aligned.
**
**      gctUINT32 * Height
**          Pointer to the height to be aligned.  If 'Height' is gcvNULL, no height
**          will be aligned.
**
**  OUTPUT:
**
**      gctUINT32 * Width
**          Pointer to a variable that will receive the aligned width.
**
**      gctUINT32 * Height
**          Pointer to a variable that will receive the aligned height.
**
**      gctBOOL_PTR SuperTiled
**          Pointer to a variable that receives the super-tiling flag for the
**          surface.
*/
gceSTATUS
gcoHARDWARE_AlignToTile(
    IN gcoHARDWARE Hardware,
    IN gceSURF_TYPE Type,
    IN OUT gctUINT32_PTR Width,
    IN OUT gctUINT32_PTR Height,
    OUT gctBOOL_PTR SuperTiled
    )
{
    gctBOOL superTiled = gcvFALSE;
    gctUINT32 xAlignment, yAlignment;

    gcmHEADER_ARG("Hardware=0x%x Type=%d Width=0x%x Height=0x%x",
                  Hardware, Type, Width, Height);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Super tiling can be enabled for render targets and depth buffers. */
    superTiled =
        (  (Type == gcvSURF_RENDER_TARGET)
        || (Type == gcvSURF_DEPTH)
        )
        &&
        /* Of course, hardware needs to support super tiles. */
        ((((gctUINT32) (Hardware->chipMinorFeatures)) >> (0 ? 12:12) & ((gctUINT32) ((((1 ? 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1)))))));

    /* Compute alignment factors. */
    xAlignment = superTiled ? 64
               : (Type == gcvSURF_TEXTURE) ? 4
               : 16;
    yAlignment = superTiled ? 64 : 4;

    if (Width != gcvNULL)
    {
        /* Align the width. */
        *Width = gcmALIGN(*Width, xAlignment);
    }

    if (Height != gcvNULL)
    {
        /* Align the height. */
        *Height = gcmALIGN(*Height, yAlignment);
    }

    if (SuperTiled != gcvNULL)
    {
        /* Copy the super tiling. */
        *SuperTiled = superTiled;
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_IsVirtualAddress
**
**  Verifies whether the specified physical address is virtual.
**
**  INPUT:
**
**      gctUINT32 Address
**          Address to be verified.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_IsVirtualAddress(
    IN gctUINT32 Address
    )
{
    gctBOOL isVirtual;

    gcmHEADER_ARG("Address=%x", Address);

    isVirtual = ((((gctUINT32) (Address)) >> (0 ? 31:31) & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1)))))));
    /* Return result. */
    gcmFOOTER_ARG("return=%d", isVirtual
                                ? gcvSTATUS_TRUE
                                : gcvSTATUS_FALSE);
    return isVirtual
        ? gcvSTATUS_TRUE
        : gcvSTATUS_FALSE;
}

/*******************************************************************************
**
**  gcoHARDWARE_IsFeatureAvailable
**
**  Verifies whether the specified feature is available in hardware.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gceFEATURE Feature
**          Feature to be verified.
*/
gceSTATUS
gcoHARDWARE_IsFeatureAvailable(
    IN gcoHARDWARE Hardware,
    IN gceFEATURE Feature
    )
{
    gctBOOL available;

    gcmHEADER_ARG("Hardware=0x%x Feature=%d", Hardware, Feature);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    switch (Feature)
    {
    /* Generic features. */
    case gcvFEATURE_PIPE_2D:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 9:9) & ((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 9:9) - (0 ? 9:9) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:9) - (0 ? 9:9) + 1)))))));
        break;

    case gcvFEATURE_PIPE_3D:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 2:2) & ((gctUINT32) ((((1 ? 2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1)))))));
        break;

    /* Generic features. */
    case gcvFEATURE_PIPE_VG:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 26:26) & ((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1)))))));
        break;

    case gcvFEATURE_DC:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 8:8) & ((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1)))))));
        break;

    case gcvFEATURE_HIGH_DYNAMIC_RANGE:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 12:12) & ((gctUINT32) ((((1 ? 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1)))))));
        break;

    case gcvFEATURE_MODULE_CG:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 14:14) & ((gctUINT32) ((((1 ? 14:14) - (0 ? 14:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:14) - (0 ? 14:14) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 14:14) - (0 ? 14:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:14) - (0 ? 14:14) + 1)))))));
        break;

    case gcvFEATURE_MIN_AREA:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 15:15) & ((gctUINT32) ((((1 ? 15:15) - (0 ? 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 15:15) - (0 ? 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1)))))));
        break;

    case gcvFEATURE_BUFFER_INTERLEAVING:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 18:18) & ((gctUINT32) ((((1 ? 18:18) - (0 ? 18:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 18:18) - (0 ? 18:18) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 18:18) - (0 ? 18:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 18:18) - (0 ? 18:18) + 1)))))));
        break;

    case gcvFEATURE_BYTE_WRITE_2D:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 19:19) & ((gctUINT32) ((((1 ? 19:19) - (0 ? 19:19) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:19) - (0 ? 19:19) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 19:19) - (0 ? 19:19) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:19) - (0 ? 19:19) + 1)))))));
        break;

    case gcvFEATURE_ENDIANNESS_CONFIG:
        available = ((((gctUINT32) (Hardware->chipMinorFeatures)) >> (0 ? 2:2) & ((gctUINT32) ((((1 ? 2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1)))))));
        break;

    case gcvFEATURE_DUAL_RETURN_BUS:
        available = ((((gctUINT32) (Hardware->chipMinorFeatures)) >> (0 ? 1:1) & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1)))))));
        break;

    case gcvFEATURE_DEBUG_MODE:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 4:4) & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1)))))));
        break;

    /* Resolve. */
    case gcvFEATURE_FAST_CLEAR:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 0:0) & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1)))))));
        break;

    case gcvFEATURE_YUV420_TILER:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 13:13) & ((gctUINT32) ((((1 ? 13:13) - (0 ? 13:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 13:13) - (0 ? 13:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1)))))));
        break;

    case gcvFEATURE_YUY2_AVERAGING:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 21:21) & ((gctUINT32) ((((1 ? 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1)))))));
        break;

    case gcvFEATURE_FLIP_Y:
        available = ((((gctUINT32) (Hardware->chipMinorFeatures)) >> (0 ? 0:0) & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1)))))));
        break;

    /* Depth. */
    case gcvFEATURE_EARLY_Z:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 16:16) & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1)))))));
        break;

    case gcvFEATURE_Z_COMPRESSION:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 5:5) & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1)))))));
        break;

    /* MSAA. */
    case gcvFEATURE_MSAA:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 7:7) & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1)))))));
        break;

    case gcvFEATURE_SPECIAL_ANTI_ALIASING:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 1:1) & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1)))))));
        break;

    case gcvFEATURE_SPECIAL_MSAA_LOD:
        available = ((((gctUINT32) (Hardware->chipMinorFeatures)) >> (0 ? 5:5) & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1)))))));
        break;

    case gcvFEATURE_VAA:
        available = ((((gctUINT32) (Hardware->chipMinorFeatures)) >> (0 ? 25:25) & ((gctUINT32) ((((1 ? 25:25) - (0 ? 25:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 25:25) - (0 ? 25:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1)))))));
        break;

    /* Texture. */
    case gcvFEATURE_422_TEXTURE_COMPRESSION:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 17:17) & ((gctUINT32) ((((1 ? 17:17) - (0 ? 17:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:17) - (0 ? 17:17) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 17:17) - (0 ? 17:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:17) - (0 ? 17:17) + 1)))))));
        break;

    case gcvFEATURE_DXT_TEXTURE_COMPRESSION:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 3:3) & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1)))))));
        break;

    case gcvFEATURE_ETC1_TEXTURE_COMPRESSION:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 10:10) & ((gctUINT32) ((((1 ? 10:10) - (0 ? 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 10:10) - (0 ? 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1)))))));
        break;

    case gcvFEATURE_CORRECT_TEXTURE_CONVERTER:
#ifdef GC_FEATURES_CORRECT_TEXTURE_CONVERTER
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? GC_FEATURES_CORRECT_TEXTURE_CONVERTER) & ((gctUINT32) ((((1 ? GC_FEATURES_CORRECT_TEXTURE_CONVERTER) - (0 ? GC_FEATURES_CORRECT_TEXTURE_CONVERTER) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GC_FEATURES_CORRECT_TEXTURE_CONVERTER) - (0 ? GC_FEATURES_CORRECT_TEXTURE_CONVERTER) + 1)))))) == (GC_FEATURES_CORRECT_TEXTURE_CONVERTER_AVAILABLE & ((gctUINT32) ((((1 ? GC_FEATURES_CORRECT_TEXTURE_CONVERTER) - (0 ? GC_FEATURES_CORRECT_TEXTURE_CONVERTER) + 1) == 32) ? ~0 : (~(~0 << ((1 ? GC_FEATURES_CORRECT_TEXTURE_CONVERTER) - (0 ? GC_FEATURES_CORRECT_TEXTURE_CONVERTER) + 1)))))));
#else
        available = gcvFALSE;
#endif
        break;

    case gcvFEATURE_TEXTURE_8K:
        available = ((((gctUINT32) (Hardware->chipMinorFeatures)) >> (0 ? 3:3) & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1)))))));
        break;

    /* Filter Blit. */
    case gcvFEATURE_SCALER:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 20:20) & ((gctUINT32) ((((1 ? 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1)))))));
        break;

    case gcvFEATURE_YUV420_SCALER:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 6:6) & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1)))))));
        break;

    case gcvFEATURE_YUY2_RENDER_TARGET:
        available = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 24:24) & ((gctUINT32) ((((1 ? 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1)))))));
        break;

    case gcvFEATURE_FRAGMENT_PROCESSOR:
        available = gcvFALSE;
        break;

    case gcvFEATURE_SHADER_HAS_W:
        available = ((((gctUINT32) (Hardware->chipMinorFeatures)) >> (0 ? 19:19) & ((gctUINT32) ((((1 ? 19:19) - (0 ? 19:19) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:19) - (0 ? 19:19) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 19:19) - (0 ? 19:19) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:19) - (0 ? 19:19) + 1)))))));
        break;

    case gcvFEATURE_2DPE20:
        available = ((((gctUINT32) (Hardware->chipMinorFeatures)) >> (0 ? 7:7) & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1)))))));
        break;

    case gcvFEATURE_HZ:
        available = ((((gctUINT32) (Hardware->chipMinorFeatures)) >> (0 ? 27:27) & ((gctUINT32) ((((1 ? 27:27) - (0 ? 27:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 27:27) - (0 ? 27:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1)))))));
        break;

    case gcvFEATURE_CORRECT_STENCIL:
        available = ((((gctUINT32) (Hardware->chipMinorFeatures)) >> (0 ? 30:30) & ((gctUINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1)))))));
        break;

    case gcvFEATURE_MC20:
        available = ((((gctUINT32) (Hardware->chipMinorFeatures)) >> (0 ? 22:22) & ((gctUINT32) ((((1 ? 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1)))))));
        break;

	case gcvFEATURE_SUPER_TILED:
        available = ((((gctUINT32) (Hardware->chipMinorFeatures)) >> (0 ? 12:12) & ((gctUINT32) ((((1 ? 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1)))))));
		break;

    default:
        gcmFATAL("Invalid feature has been requested.");
        available = gcvFALSE;
    }

    /* Return result. */
    gcmFOOTER_ARG("%d", available ? gcvSTATUS_TRUE : gcvSTATUS_OK);
    return available ? gcvSTATUS_TRUE : gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_Is2DAvailable
**
**  Verifies whether 2D engine is available.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_Is2DAvailable(
    IN gcoHARDWARE Hardware
    )
{
    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    gcmFOOTER_ARG("%d", Hardware->hw2DEngine ? gcvSTATUS_TRUE : gcvSTATUS_OK);
    return Hardware->hw2DEngine ? gcvSTATUS_TRUE : gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_UseSoftware2D
**
**  Sets the software 2D renderer force flag.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_UseSoftware2D(
    IN gcoHARDWARE Hardware,
    IN gctBOOL Enable
    )
{
    gcmHEADER_ARG("Hardware=0x%x Enable=%d", Hardware, Enable);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Set the force flag. */
    Hardware->sw2DEngine = Enable;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_SetBrushLimit
**
**  Sets the maximum number of brushes in the cache.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**      gctUINT MaxCount
**          Maximum number of brushes allowed in the cache at the same time.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_SetBrushLimit(
    IN gcoHARDWARE Hardware,
    IN gctUINT MaxCount
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x MaxCount=%d", Hardware, MaxCount);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    status = gcoBRUSH_CACHE_SetBrushLimit(
        Hardware->brushCache,
        MaxCount
        );

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_GetBrushCache
**
**  Return a pointer to the brush cache.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**  OUTPUT:
**
**      gcoBRUSH_CACHE * BrushCache
**          A pointer to gcoBRUSH_CACHE object.
*/
gceSTATUS
gcoHARDWARE_GetBrushCache(
    IN gcoHARDWARE Hardware,
    IN OUT gcoBRUSH_CACHE * BrushCache
    )
{
    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmVERIFY_ARGUMENT(BrushCache != gcvNULL);

    /* Set the result. */
    *BrushCache = Hardware->brushCache;

    /* Success. */
    gcmFOOTER_ARG("*BrushCache=0x%x", *BrushCache);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHARDWARE_FlushBrush
**
**  Program the brush.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**      gcoBRUSH Brush
**          A pointer to a valid gcoBRUSH object.
**
**      gceSURF_FORMAT Format
**          Format for destination surface when using color conversion.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_FlushBrush(
    IN gcoHARDWARE Hardware,
    IN gcoBRUSH Brush
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Brush=0x%x", Hardware, Brush);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    do
    {
        /* Flush the brush. */
        gcmERR_BREAK(gcoBRUSH_CACHE_FlushBrush(
            Hardware->brushCache,
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
**  gcoHARDWARE_Clear2D
**
**  Clear one or more rectangular areas.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**      gctUINT32 RectCount
**          The number of rectangles to draw. The array of line positions
**          pointed to by Position parameter must have at least RectCount
**          positions.
**
**      gcsRECT_PTR Rect
**          Points to an array of positions in (x0, y0)-(x1, y1) format.
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
**      gctUINT8 FgRop
**          Foreground ROP to use with opaque pixels.
**
**      gctUINT8 BgRop
**          Background ROP to use with transparent pixels.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_Clear2D(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 RectCount,
    IN gcsRECT_PTR Rect,
    IN gctUINT32 Color,
    IN gctBOOL ColorConvert,
    IN gctUINT8 FgRop,
    IN gctUINT8 BgRop
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x RectCount=%d Rect=0x%x "
                    "Color=%x ColorConvert=%d FgRop=%x "
                    "BgRop=%x",
                    Hardware, RectCount, Rect,
                    Color, ColorConvert, FgRop,
                    BgRop);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    do
    {
        /* Set the clear color. */
        gcmERR_BREAK(gcoHARDWARE_Set2DClearColor(
            Hardware,
            Color,
            ColorConvert
            ));

        if (Hardware->hw2DPE20)
        {
            gctBOOL useSource;

            /* Determine the resource usage. */
            gcoHARDWARE_Get2DResourceUsage(
                FgRop, BgRop,
                Hardware->srcTransparency,
                &useSource, gcvNULL, gcvNULL
                );

            /* If source is used, then setup dummy source for DE. */
            if (useSource)
            {
                static gcsSURF_INFO zeroSurfaceInfo;

                /* Needs some format specified other than gcvSURF_UNKNOWN. */
                zeroSurfaceInfo.format = gcvSURF_A8R8G8B8;

                gcmERR_BREAK(gcoHARDWARE_SetColorSource(
                    Hardware,
                    &zeroSurfaceInfo,
                    0));

                gcmERR_BREAK(gcoHARDWARE_SetSource(
                    Hardware,
                    &zeroSurfaceInfo.rect
                    ));
            }
        }
        else
        {
            FgRop = 0x0;
            BgRop = 0x0;
        }

        if (Rect == gcvNULL)
        {
            /* Has target surface been set?. */
            if (Hardware->targetSurface.type == gcvSURF_TYPE_UNKNOWN)
            {
                gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_OBJECT);
                return gcvSTATUS_INVALID_OBJECT;
            }

            /* Use target rectangle as clear rectangle. */
            Rect = &Hardware->targetSurface.rect;
        }

        /* Kick off 2D engine. */
        gcmERR_BREAK(gcoHARDWARE_StartDE(
            Hardware,
            gcv2D_CLEAR,
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
**  gcoHARDWARE_Line2D
**
**  Draw one or more Bresenham lines using a brush.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**      gctUINT32 LineCount
**          The number of lines to draw. The array of line positions pointed
**          to by Position parameter must have at least LineCount positions.
**
**      gcsRECT_PTR Position
**          Points to an array of positions in (x0, y0)-(x1, y1) format.
**
**      gcoBRUSH Brush
**          Brush to use for drawing.
**
**      gctUINT8 FgRop
**          Foreground ROP to use with opaque pixels.
**
**      gctUINT8 BgRop
**          Background ROP to use with transparent pixels.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_Line2D(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 LineCount,
    IN gcsRECT_PTR Position,
    IN gcoBRUSH Brush,
    IN gctUINT8 FgRop,
    IN gctUINT8 BgRop
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x LineCount=%d Position=0x%x "
                    "Brush=0x%x FgRop=%x BgRop=%x",
                    Hardware, LineCount, Position,
                    Brush, FgRop, BgRop);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    do
    {
        /* Flush the brush. */
        gcmERR_BREAK(gcoHARDWARE_FlushBrush(
            Hardware,
            Brush
            ));

        /* Kick off 2D engine. */
        gcmERR_BREAK(gcoHARDWARE_StartDELine(
            Hardware,
            gcv2D_LINE,
            LineCount,
            Position,
            0,
            gcvNULL,
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
**  gcoHARDWARE_Line2DEx
**
**  Draw one or more Bresenham lines using solid color(s).
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**      gctUINT32 LineCount
**          The number of lines to draw. The array of line positions pointed
**          to by Position parameter must have at least LineCount positions.
**
**      gcsRECT_PTR Position
**          Points to an array of positions in (x0, y0)-(x1, y1) format.
**
**      gctUINT32 ColorCount
**          Set as 1, if single color for all lines.
**          Set as LineCount, if each line has its own color.
**
**      gctUINT32_PTR Color32
**          Source color array in A8R8G8B8 format.
**
**      gctUINT8 FgRop
**          Foreground ROP to use with opaque pixels.
**
**      gctUINT8 BgRop
**          Background ROP to use with transparent pixels.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_Line2DEx(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 LineCount,
    IN gcsRECT_PTR Position,
    IN gctUINT32 ColorCount,
    IN gctUINT32_PTR Color32,
    IN gctUINT8 FgRop,
    IN gctUINT8 BgRop
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x LineCount=%d Position=0x%x "
                    "ColorCount=%d Color32=0x%x FgRop=%x BgRop=%x",
                    Hardware, LineCount, Position,
                    ColorCount, Color32, FgRop, BgRop);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmASSERT((ColorCount == 1) || (ColorCount == LineCount));

    do
    {
        /* Set the solid color using a monochrome stream. */
        gctUINT32 data[3];

        /* SelectPipe(2D). */
        gcmERR_BREAK(gcoHARDWARE_SelectPipe(
            Hardware,
            0x1
            ));

        /* Configure the source as monochrome stream. */
        data[0]
            = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 5:4) - (0 ? 5:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0))) | (((gctUINT32) (0xA & ((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 28:24) - (0 ? 28:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:24) - (0 ? 28:24) + 1))))))) << (0 ? 28:24))) | (((gctUINT32) (0x0A & ((gctUINT32) ((((1 ? 28:24) - (0 ? 28:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:24) - (0 ? 28:24) + 1))))))) << (0 ? 28:24)))
            ;

        /* Src origin. */
        data[1] = 0;

        /* Src size. */
        data[2] = 0;

        /* LoadState(AQDE_SRC_CONFIG, 1), config. */
        gcmERR_BREAK(gcoHARDWARE_LoadState(
            Hardware,
            0x0120C, 3,
            data
            ));

        /* Kick off 2D engine. */
        gcmERR_BREAK(gcoHARDWARE_StartDELine(
            Hardware,
            gcv2D_LINE,
            LineCount,
            Position,
            ColorCount,
            Color32,
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
**  gcoHARDWARE_MonoBlit
**
**  Monochrome blit.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**      gctPOINTER StreamBits
**          Pointer to the monochrome bitmap.
**
**      gcsPOINT_PTR StreamSize
**          Size of the monochrome bitmap in pixels.
**
**      gcsRECT_PTR StreamRect
**          Bounding rectangle of the area within the bitmap to render.
**
**      gceSURF_MONOPACK SrcStreamPack
**          Source bitmap packing.
**
**      gceSURF_MONOPACK DestStreamPack
**          Packing of the bitmap in the command stream.
**
**      gcsRECT_PTR DestRect
**          Pointer to an array of destination rectangles.
**
**      gctUINT32 FgRop
**          Foreground and background ROP codes.
**
**      gctUINT32 BgRop
**          Background ROP to use with transparent pixels.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_MonoBlit(
    IN gcoHARDWARE Hardware,
    IN gctPOINTER StreamBits,
    IN gcsPOINT_PTR StreamSize,
    IN gcsRECT_PTR StreamRect,
    IN gceSURF_MONOPACK SrcStreamPack,
    IN gceSURF_MONOPACK DestStreamPack,
    IN gcsRECT_PTR DestRect,
    IN gctUINT32 FgRop,
    IN gctUINT32 BgRop
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x StreamBits=0x%x StreamSize=%d StreamRect=0x%x "
                    "SrcStreamPack=%d DestStreamPack=%d DestRect=0x%x "
                    "FgRop=%x BgRop=%x",
                    Hardware, StreamBits, StreamSize, StreamRect,
                    SrcStreamPack, DestStreamPack, DestRect,
                    FgRop, BgRop);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(StreamBits != gcvNULL);
    gcmVERIFY_ARGUMENT(StreamRect != gcvNULL);
    gcmVERIFY_ARGUMENT(StreamSize != gcvNULL);
    gcmVERIFY_ARGUMENT(DestRect != gcvNULL);

    do
    {
        gctUINT32 srcStreamWidth, srcStreamHeight;
        gctUINT32 destStreamWidth, destStreamHeight;
        gctUINT32 srcPackWidth, srcPackHeight;
        gctUINT32 destPackWidth, destPackHeight;
        gctUINT32 destStreamSize;
        gctUINT32_PTR buffer;

        /* Get stream pack sizes. */
        gcmERR_BREAK(gco2D_GetPackSize(
            SrcStreamPack,
            &srcPackWidth,
            &srcPackHeight
            ));

        gcmERR_BREAK(gco2D_GetPackSize(
            DestStreamPack,
            &destPackWidth,
            &destPackHeight
            ));

        /* Determine the size of the stream. */
        destStreamWidth  = StreamRect->right  - StreamRect->left;
        destStreamHeight = StreamRect->bottom - StreamRect->top;

        /* Align the height, verify alignment of the width. */
        gcmASSERT((destStreamWidth & (destPackWidth - 1)) == 0);
        destStreamHeight = gcmALIGN(destStreamHeight, destPackHeight);

        /* Determine the size of the stream in bytes and in 32-bitters. */
        destStreamSize = (destStreamWidth * destStreamHeight) >> 3;
        gcmASSERT((destStreamSize & 3) == 0);

        /* Determine the size of the source stream in pixels. */
        srcStreamWidth  = gcmALIGN(StreamSize->x, srcPackWidth);
        srcStreamHeight = gcmALIGN(StreamSize->y, srcPackHeight);

        /* Call lower layer to form a StartDE command. */
        gcmERR_BREAK(gcoHARDWARE_StartDEStream(
            Hardware,
            DestRect,
            FgRop,
            BgRop,
            destStreamSize,
            (gctPOINTER *) &buffer
            ));

        /* Same packing and entire stream? */
        if ((SrcStreamPack   == DestStreamPack) &&
            (srcStreamWidth  == destStreamWidth) &&
            (srcStreamHeight == destStreamHeight) &&
            (StreamRect->left == 0) &&
            (StreamRect->top == 0))
        {
            gcmASSERT(
                (StreamRect->left == 0) && (StreamRect->top == 0)
                );

            gcmERR_BREAK(gcoOS_MemCopy(
                buffer,
                StreamBits,
                destStreamSize
                ));
        }
        else
        {
            gctUINT8_PTR srcStream;
            gctUINT8_PTR dstStream;

            /* Compute the offset into the source stream in bits/pixels. */
            gctUINT32 srcOffset = (SrcStreamPack == gcvSURF_UNPACKED)
                ?  StreamRect->top * srcStreamWidth + StreamRect->left
                : (StreamRect->left & ~(srcPackWidth - 1)) * srcStreamHeight +
                  (StreamRect->left &  (srcPackWidth - 1)) +
                   StreamRect->top  *   srcPackWidth;

            /* Adjust to get the offset in bytes. */
            gcmASSERT((srcOffset & 7) == 0);
            srcOffset >>= 3;

            /* Set stream bases. */
            srcStream = (gctUINT8_PTR) StreamBits + srcOffset;
            dstStream = (gctUINT8_PTR) buffer;

            /* Same packing? */
            if ((SrcStreamPack == DestStreamPack) &&
                ((StreamRect->left & (srcPackWidth - 1)) == 0))
            {
                gctUINT32 count;
                gctUINT32 passCount;
                gctUINT32 step;
                gctUINT32 copySize;
                gctUINT32 srcStride;

                /* Must be pack aligned. */
                gcmASSERT((srcOffset       & ((srcPackWidth >> 3) - 1)) == 0);
                gcmASSERT((srcStreamWidth  & ( srcPackWidth       - 1)) == 0);
                gcmASSERT((destStreamWidth & ( destPackWidth      - 1)) == 0);

                if (DestStreamPack == gcvSURF_UNPACKED)
                {
                    srcStride = srcStreamWidth  >> 3;
                    copySize  = destStreamWidth >> 3;

                    /* One line at a time. */
                    step      = 1;
                    passCount = destStreamHeight;
                }
                else
                {
                    srcStride = (srcPackWidth  * srcStreamHeight)  >> 3;
                    copySize  = (destPackWidth * destStreamHeight) >> 3;

                    /* One packed column at a time. */
                    step      = destPackWidth;
                    passCount = destStreamWidth;
                }

                for (count = 0; count < passCount; count += step)
                {
                    gcmERR_BREAK(gcoOS_MemCopy(dstStream, srcStream, copySize));
                    srcStream += srcStride;
                    dstStream += copySize;
                }
            }
            else
            {
                gctUINT32 destX, destY;

                gctUINT8_PTR srcLine = srcStream;
                gctUINT8_PTR destLine = dstStream;

                gctUINT32 srcLineStep, destLineStep;
                gctUINT32 srcByteIntStep, srcByteExtStep;
                gctUINT32 destByteIntStep, destByteExtStep;

                if (SrcStreamPack == gcvSURF_UNPACKED)
                {
                    srcByteIntStep = srcByteExtStep = 1;
                    srcLineStep    = srcStreamWidth >> 3;
                }
                else
                {
                    srcByteIntStep = 1;
                    srcByteExtStep = ((srcPackWidth * srcStreamHeight) - srcPackWidth + 8) >> 3;
                    srcLineStep    = srcPackWidth >> 3;
                }

                if (DestStreamPack == gcvSURF_UNPACKED)
                {
                    destByteIntStep = destByteExtStep = 1;
                    destLineStep    = destStreamWidth >> 3;
                }
                else
                {
                    destByteIntStep = 1;
                    destByteExtStep = ((destPackWidth * destStreamHeight) - destPackWidth + 8) >> 3;
                    destLineStep    = destPackWidth >> 3;
                }

                for (destY = 0; destY < destStreamHeight; destY++)
                {
                    gctUINT32 srcX;
                    gctUINT8_PTR srcByte  = srcLine;
                    gctUINT8_PTR destByte = destLine;

                    for (srcX = StreamRect->left, destX = 0;
                         destX < destStreamWidth;)
                    {
                        /* Copy the current byte. */
                        *destByte = *srcByte;

                        /* Advance the coordinates. */
                        srcX  += 8;
                        destX += 8;

                        /* Advance the pointers. */
                        srcByte += ((srcX % srcPackWidth) == 0)
                            ? srcByteExtStep
                            : srcByteIntStep;

                        destByte += ((destX % destPackWidth) == 0)
                            ? destByteExtStep
                            : destByteIntStep;
                    }

                    /* Advance the pointers. */
                    srcLine  += srcLineStep;
                    destLine += destLineStep;
                }
            }

            /* Reverse the bit order for big Endian */
            if (Hardware->bigEndian)
            {
                gctUINT i;
                gctUINT32_PTR p = (gctUINT32_PTR)buffer;

                for (i = 0; i < destStreamSize / sizeof(gctUINT32); i++)
                {
                    gctUINT8_PTR p2 = (gctUINT8_PTR)&p[i];

                    p[i] = ((gctUINT32)p2[3] << 24)
                            + ((gctUINT32)p2[2] << 16)
                            + ((gctUINT32)p2[1] << 8)
                            + (gctUINT32)p2[0];
                }
            }
        }

        gcmDUMP_DATA(Hardware->os, "prim2D", buffer, destStreamSize);
    }
    while (gcvFALSE);

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_WriteBuffer
**
**  Write data into the command buffer.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to the gcoHARDWARE object.
**
**      gctCONST_POINTER Data
**          Pointer to a buffer that contains the data to be copied.
**
**      IN gctSIZE_T Bytes
**          Number of bytes to copy.
**
**      IN gctBOOL Aligned
**          gcvTRUE if the data needs to be aligned to 64-bit.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_WriteBuffer(
    IN gcoHARDWARE Hardware,
    IN gctCONST_POINTER Data,
    IN gctSIZE_T Bytes,
    IN gctBOOL Aligned
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Data=0x%x Bytes=%d Aligned=%d",
                    Hardware, Data, Bytes, Aligned);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Call hardware. */
    status = gcoBUFFER_Write(
        Hardware->buffer,
        Data,
        Bytes,
        Aligned
        );

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_RGB2YUV
**
**  Convert RGB8 color value to YUV color space.
**
**  INPUT:
**
**      gctUINT8 R, G, B
**          RGB color value.
**
**  OUTPUT:
**
**      gctUINT8_PTR Y, U, V
**          YUV color value.
*/
void gcoHARDWARE_RGB2YUV(
    gctUINT8 R,
    gctUINT8 G,
    gctUINT8 B,
    gctUINT8_PTR Y,
    gctUINT8_PTR U,
    gctUINT8_PTR V
    )
{
    gcmHEADER_ARG("R=%d G=%d B=%d",
                    R, G, B);

    *Y = (gctUINT8) ((( 66 * R + 129 * G +  25 * B + 128) >> 8) +  16);
    *U = (gctUINT8) (((-38 * R -  74 * G + 112 * B + 128) >> 8) + 128);
    *V = (gctUINT8) (((112 * R -  94 * G -  18 * B + 128) >> 8) + 128);

    gcmFOOTER_ARG("*Y=%d *U=%d *V=%d",
                    *Y, *U, *V);
}

/*******************************************************************************
**
**  gcoHARDWARE_YUV2RGB
**
**  Convert YUV color value to RGB8 color space.
**
**  INPUT:
**
**      gctUINT8 Y, U, V
**          YUV color value.
**
**  OUTPUT:
**
**      gctUINT8_PTR R, G, B
**          RGB color value.
*/
void gcoHARDWARE_YUV2RGB(
    gctUINT8 Y,
    gctUINT8 U,
    gctUINT8 V,
    gctUINT8_PTR R,
    gctUINT8_PTR G,
    gctUINT8_PTR B
    )
{
    /* Clamp the input values to the legal ranges. */
    gctINT y = (Y < 16) ? 16 : ((Y > 235) ? 235 : Y);
    gctINT u = (U < 16) ? 16 : ((U > 240) ? 240 : U);
    gctINT v = (V < 16) ? 16 : ((V > 240) ? 240 : V);

    /* Shift ranges. */
    gctINT _y = y - 16;
    gctINT _u = u - 128;
    gctINT _v = v - 128;

    /* Convert to RGB. */
    gctINT r = (298 * _y            + 409 * _v + 128) >> 8;
    gctINT g = (298 * _y - 100 * _u - 208 * _v + 128) >> 8;
    gctINT b = (298 * _y + 516 * _u            + 128) >> 8;

    gcmHEADER_ARG("Y=%d U=%d V=%d",
                    Y, U, V);

    /* Clamp the result. */
    *R = (r < 0)? 0 : (r > 255)? 255 : (gctUINT8) r;
    *G = (g < 0)? 0 : (g > 255)? 255 : (gctUINT8) g;
    *B = (b < 0)? 0 : (b > 255)? 255 : (gctUINT8) b;

    gcmFOOTER_ARG("*R=%d *G=%d *B=%d",
                    *R, *G, *B);
}

gceSTATUS
gcoHARDWARE_OptimizeBandwidth(
    IN gcoHARDWARE Hardware
    )
{
    gctUINT32 control;
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    do
    {
        /* Determine destination read control. */
        control = (  Hardware->colorCompression
                  || Hardware->alphaBlendEnable
                  || (Hardware->colorWrite != 0xF)
                  || ((((gctUINT32) (Hardware->memoryConfig)) >> (0 ? 1:1) & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1)))))))
                  )
                  ? 0x0
                  : 0x1;

        gcmTRACE(gcvLEVEL_INFO,
                 "Read control: compression=%d alpha=%d writeEnable=%X ==> %d",
                 Hardware->colorCompression,
                 Hardware->alphaBlendEnable,
                 Hardware->colorWrite,
                 control);

        /* Only process when it is different. */
        if (control != Hardware->destinationRead)
        {
            /* Switch to 3D pipe. */
            gcmERR_BREAK(
                gcoHARDWARE_SelectPipe(Hardware, 0x0));

            /* Flush the color cache. */
            gcmERR_BREAK(
                gcoHARDWARE_LoadState32(Hardware,
                                        0x0380C,
                                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))) | (((gctUINT32) (0x1&((gctUINT32)((((1?1:1)-(0?1:1)+1)==32)?~0:(~(~0<<((1?1:1)-(0?1:1)+1)))))))<<(0?1:1)))));

            /* Write the destination read control. */
            gcmERR_BREAK(
                gcoHARDWARE_LoadState32(Hardware,
                                        0x0142C,
                                        ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 17:17) - (0 ? 17:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ? 17:17))) | (((gctUINT32) (0x0&((gctUINT32)((((1?17:17)-(0?17:17)+1)==32)?~0:(~(~0<<((1?17:17)-(0?17:17)+1)))))))<<(0?17:17)))&
                                        ((((gctUINT32) (~0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))) | (((gctUINT32) ((gctUINT32) (control) & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)))));

            /* Save value. */
            Hardware->destinationRead = control;
        }

        /* Suucess. */
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    /* Return the error. */
    gcmFOOTER();
    return status;
}

gceSTATUS gcoHARDWARE_FlushL2Cache(
    IN gcoHARDWARE Hardware
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x", Hardware);

    do
    {
        /* Idle the pipe. */
        gcmERR_BREAK(
            gcoHARDWARE_Semaphore(Hardware,
                                  gcvWHERE_COMMAND,
                                  gcvWHERE_PIXEL,
                                  gcvHOW_SEMAPHORE_STALL));

        /* Flush the L2 cache. */
        gcmERR_BREAK(
            gcoHARDWARE_LoadState32(Hardware,
                                    0x01650,
                                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6))) | (((gctUINT32) (0x1&((gctUINT32)((((1?6:6)-(0?6:6)+1)==32)?~0:(~(~0<<((1?6:6)-(0?6:6)+1)))))))<<(0?6:6)))));


        /* Idle the pipe (again). */
        gcmERR_BREAK(
            gcoHARDWARE_Semaphore(Hardware,
                                  gcvWHERE_COMMAND,
                                  gcvWHERE_PIXEL,
                                  gcvHOW_SEMAPHORE_STALL));

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHARDWARE_SetAutoFlushCycles
**
**  Set the GPU clock cycles after which the idle 2D engine will keep auto-flushing.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      UINT32 Cycles
**          Source color premultiply with Source Alpha.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gcoHARDWARE_SetAutoFlushCycles(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Cycles
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Hardware=0x%x Cycles=%d", Hardware, Cycles);

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

            /* LoadState timeout value. */
            gcmERR_BREAK(gcoHARDWARE_LoadState32(
                Hardware,
                0x00670,
                Cycles
                ));
        }
        else
        {
            /* Auto-flush is not supported by the software renderer. */
            gcmERR_BREAK(gcvSTATUS_NOT_SUPPORTED);
        }
    }
    while (gcvFALSE);

    /* Return status. */
    gcmFOOTER();
    return status;
}

/* Resolve depth buffer. */
gceSTATUS
gcoHARDWARE_ResolveDepth(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 SrcTileStatusAddress,
    IN gcsSURF_INFO_PTR SrcInfo,
    IN gcsSURF_INFO_PTR DestInfo,
    IN gcsPOINT_PTR SrcOrigin,
    IN gcsPOINT_PTR DestOrigin,
    IN gcsPOINT_PTR RectSize
    )
{
    gceSTATUS status;
    gctUINT32 physicalBaseAddress;
    gctUINT32 config;
    gctBOOL hasCompression;
    gctUINT32 format;

    gcmHEADER_ARG("Hardware=0x%x SrcTileStatusAddress=%x SrcInfo=0x%x "
                    "DestInfo=0x%x SrcOrigin=0x%x DestOrigin=0x%x "
                    "RectSize=0x%x",
                    Hardware, SrcTileStatusAddress, SrcInfo,
                    DestInfo, SrcOrigin, DestOrigin,
                    RectSize);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmVERIFY_ARGUMENT(SrcInfo != gcvNULL);


#if gcdSECURE_USER
        physicalBaseAddress = 0;
#else
    /* Get physical base address. */
    if (gcoHARDWARE_IsFeatureAvailable(Hardware,
                                       gcvFEATURE_MC20) == gcvSTATUS_TRUE)
    {
        physicalBaseAddress = 0;
    }
    else
    {
        gcmVERIFY_OK(
            gcoOS_GetBaseAddress(Hardware->os, &physicalBaseAddress));
    }
#endif

    /* Determine compression. */
    hasCompression = ((((gctUINT32) (Hardware->chipFeatures)) >> (0 ? 5:5) & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1)))))));

    /* Determine color format. */
    switch (SrcInfo->format)
    {
    case gcvSURF_D24X8:
        format = 0x6;
        break;

    case gcvSURF_D24S8:
        format = 0x5;
        break;

    case gcvSURF_D16:
    case gcvSURF_D32:
    default:
        return gcvSTATUS_NOT_SUPPORTED;
    }


    /* Program depth tile status into color fast clear. */
    gcmONERROR(
        gcoHARDWARE_LoadState32(Hardware,
                                0x01658,
                                SrcTileStatusAddress + physicalBaseAddress));

    gcmONERROR(
        gcoHARDWARE_LoadState32(Hardware,
                                0x0165C,
                                SrcInfo->node.physical + physicalBaseAddress));

    gcmONERROR(
        gcoHARDWARE_LoadState32(Hardware,
                                0x01660,
                                SrcInfo->clearValue));

    /* Build configuration register. */
    config = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))) | (((gctUINT32) (0x1&((gctUINT32)((((1?1:1)-(0?1:1)+1)==32)?~0:(~(~0<<((1?1:1)-(0?1:1)+1)))))))<<(0?1:1)))
           | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)))
           | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))
           | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7))) | (((gctUINT32) ((gctUINT32) (hasCompression) & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)))

           | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ? 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)))

           ;

    gcmONERROR(
        gcoHARDWARE_LoadState32(Hardware,
                                0x01654,
                                config));

    /* Perform the resolve. */
    status = gcoHARDWARE_ResolveRect(Hardware,
                                     SrcInfo,
                                     DestInfo,
                                     SrcOrigin,
                                     DestOrigin,
                                     RectSize);

OnError:
    if (Hardware->currentTarget != gcvNULL)
    {
        /* Reset tile status. */
        gcmVERIFY_OK(
            gcoHARDWARE_LoadState32(Hardware,
                                    0x01658,
                                    Hardware->physicalTileColor +
                                    physicalBaseAddress));

        gcmVERIFY_OK(
            gcoHARDWARE_LoadState32(Hardware,
                                    0x0165C,
                                    Hardware->currentTarget->node.physical +
                                    physicalBaseAddress));

        gcmVERIFY_OK(
            gcoHARDWARE_LoadState32(Hardware,
                                    0x01660,
                                    Hardware->currentTarget->clearValue));
    }

    /* Reset memory configuration. */
    gcmVERIFY_OK(
        gcoHARDWARE_LoadState32(Hardware,
                                0x01654,
                                Hardware->memoryConfig));

    /* Reset the auto disable counters. */
    gcmVERIFY_OK(
        gcoHARDWARE_LoadState32(Hardware,
                                0x01650,
                                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5))) | (((gctUINT32) (0x1&((gctUINT32)((((1?5:5)-(0?5:5)+1)==32)?~0:(~(~0<<((1?5:5)-(0?5:5)+1)))))))<<(0?5:5)))));

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**  gcoHARDWARE_LoadStateBlock
**
**  Load a number of states in the form of address/data.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to a gcoHARDWARE object.
**
**      gctUINT32_PTR States
**          Pointer to the address/state values.  All addresses should be byte
**          addresses, like AQ_FLUSH_Address.
**
**      gctSIZE_T Count
**          Number of 32-bit entries in the array pointed to by States.  This
**          means that there are Count / 2 states to be loaded.
*/
gceSTATUS
gcoHARDWARE_LoadStateBlock(
    IN gcoHARDWARE Hardware,
    IN gctUINT32_PTR States,
    IN gctSIZE_T Count
    )
{
    gceSTATUS status;
    gctBOOL_PTR hints = gcvNULL;
    gctUINT32_PTR memory;
    gctSIZE_T i;

    gcmHEADER_ARG("Hardware=0x%x States=0x%x Count=%lu",
                  Hardware, States, Count);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmVERIFY_ARGUMENT(States != gcvNULL);
    gcmVERIFY_ARGUMENT(Count > 0);

#if gcdSECURE_USER
    /* Allocate memory for the hints. */
    gcmONERROR(gcoOS_Allocate(Hardware->os,
                              Count * gcmSIZEOF(gctBOOL),
                              (gctPOINTER *) &hints));
#endif

    for (i = 0; i < Count; i += 2)
    {
        /* Buffer the states. */
        gcmONERROR(gcoCONTEXT_Buffer(Hardware->context,
                                     States[i + 0],
                                     1,
                                     &States[i + 1],
                                     (hints != gcvNULL) ? hints + i
                                                        : gcvNULL));
    }

    /* Reserve space in the command buffer. */
    gcmONERROR(gcoBUFFER_Reserve(Hardware->buffer,
                                 Count * 4,
                                 gcvTRUE,
                                 hints,
                                 (gctPOINTER *) &memory));

    /* Copy all the states. */
    for (i = 0; i < Count; i += 2)
    {
        memory[i + 0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (States[i+0]>>2) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
        memory[i + 1] = States[i + 1];
    }

#if gcdSECURE_USER
    /* Free the hints. */
    gcmVERIFY_OK(gcoOS_Free(Hardware->os, hints));
#endif

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
#if gcdSECURE_USER
    /* Free the hits if allocated. */
    if (hints != gcvNULL)
    {
        gcmVERIFY_OK(
            gcoOS_Free(Hardware->os, hints));
    }
#endif

    /* Return the status. */
    gcmFOOTER();
    return status;
}


