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
**  @file
**  gcoSURF object for user HAL layers.
**
*/

#include "gc_hal_user_precomp.h"

#define gcmALVM         iface.u.AllocateLinearVideoMemory

#define _GC_OBJ_ZONE    gcvZONE_SURFACE

/******************************************************************************\
**************************** gcoSURF API Support Code **************************
\******************************************************************************/

static gceSTATUS
_Lock(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;

    /* Lock the video memory. */
    gcmONERROR(
        gcoHARDWARE_Lock(Surface->hal->hardware,
                         &Surface->info.node,
                         gcvNULL,
                         gcvNULL));

    gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                  "Locked surface 0x%x: physical=0x%08X logical=0x%x lockCount=%d",
                  &Surface->info.node,
                  Surface->info.node.physical,
                  Surface->info.node.logical,
                  Surface->info.node.lockCount);

    /* Lock the hierarchical Z node. */
    if (Surface->info.hzNode.pool != gcvPOOL_UNKNOWN)
    {
        gcmONERROR(
            gcoHARDWARE_Lock(Surface->hal->hardware,
                             &Surface->info.hzNode,
                             gcvNULL,
                             gcvNULL));

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Locked HZ surface 0x%x: physical=0x%08X logical=0x%x "
                      "lockCount=%d",
                      &Surface->info.hzNode,
                      Surface->info.hzNode.physical,
                      Surface->info.hzNode.logical,
                      Surface->info.hzNode.lockCount);

        /* Only 1 address. */
        Surface->info.hzNode.count = 1;
    }

    /* Lock the tile status buffer. */
    if (Surface->tileStatusNode.pool != gcvPOOL_UNKNOWN)
    {
        /* Lock the tile status buffer. */
        gcmONERROR(
            gcoHARDWARE_Lock(Surface->hal->hardware,
                             &Surface->tileStatusNode,
                             gcvNULL,
                             gcvNULL));

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Locked tile status 0x%x: physical=0x%08X logical=0x%x "
                      "lockedCount=%d",
                      &Surface->tileStatusNode,
                      Surface->tileStatusNode.physical,
                      Surface->tileStatusNode.logical,
                      Surface->tileStatusNode.lockCount);

        /* Only 1 address. */
        Surface->tileStatusNode.count = 1;

        /* Check if this is the forst lock. */
        if (Surface->tileStatusNode.firstLock)
        {
            /* Fill the tile status memory with the filler. */
            gcmONERROR(
                gcoOS_MemFill(Surface->tileStatusNode.logical,
                              (gctUINT8) Surface->tileStatusNode.filler,
                              Surface->tileStatusNode.size));

            /* Dump the memory write. */
            gcmDUMP_BUFFER(Surface->hal->os,
                           "memory",
                           Surface->tileStatusNode.physical,
                           Surface->tileStatusNode.logical,
                           0,
                           Surface->tileStatusNode.size);

            /* No longer first lock. */
            Surface->tileStatusNode.firstLock = gcvFALSE;
        }
    }

    /* Lock the hierarchical Z tile status buffer. */
    if (Surface->hzTileStatusNode.pool != gcvPOOL_UNKNOWN)
    {
        /* Lock the tile status buffer. */
        gcmONERROR(
            gcoHARDWARE_Lock(Surface->hal->hardware,
                             &Surface->hzTileStatusNode,
                             gcvNULL,
                             gcvNULL));

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Locked HZ tile status 0x%x: physical=0x%08X logical=0x%x "
                      "lockedCount=%d",
                      &Surface->hzTileStatusNode,
                      Surface->hzTileStatusNode.physical,
                      Surface->hzTileStatusNode.logical,
                      Surface->hzTileStatusNode.lockCount);

        /* Only 1 address. */
        Surface->hzTileStatusNode.count = 1;

        /* Check if this is the forst lock. */
        if (Surface->hzTileStatusNode.firstLock)
        {
            /* Fill the tile status memory with the filler. */
            gcmONERROR(
                gcoOS_MemFill(Surface->hzTileStatusNode.logical,
                              (gctUINT8) Surface->hzTileStatusNode.filler,
                              Surface->hzTileStatusNode.size));

            /* Dump the memory write. */
            gcmDUMP_BUFFER(Surface->hal->os,
                           "memory",
                           Surface->hzTileStatusNode.physical,
                           Surface->hzTileStatusNode.logical,
                           0,
                           Surface->hzTileStatusNode.size);

            /* No longer first lock. */
            Surface->hzTileStatusNode.firstLock = gcvFALSE;
        }
    }

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_Unlock(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;

    /* Unlock the surface. */
    gcmONERROR(
        gcoHARDWARE_Unlock(Surface->hal->hardware,
                           &Surface->info.node,
                           Surface->info.type));

    gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                  "Unlocked surface 0x%x: lockCount=%d",
                  &Surface->info.node,
                  Surface->info.node.lockCount);

    /* Unlock the hierarchical Z buffer. */
    if (Surface->info.hzNode.pool != gcvPOOL_UNKNOWN)
    {
        gcmONERROR(
            gcoHARDWARE_Unlock(Surface->hal->hardware,
                               &Surface->info.hzNode,
                               gcvSURF_HIERARCHICAL_DEPTH));

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Unlocked HZ surface 0x%x: lockCount=%d",
                      &Surface->info.hzNode,
                      Surface->info.hzNode.lockCount);
    }

    /* Unlock the tile status buffer. */
    if (Surface->tileStatusNode.pool != gcvPOOL_UNKNOWN)
    {
        gcmONERROR(
            gcoHARDWARE_Unlock(Surface->hal->hardware,
                               &Surface->tileStatusNode,
                               gcvSURF_TILE_STATUS));

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Unlocked tile status 0x%x: lockCount=%d",
                      &Surface->info.hzNode,
                      Surface->info.hzNode.lockCount);
    }

    /* Unlock the hierarchical tile status buffer. */
    if (Surface->hzTileStatusNode.pool != gcvPOOL_UNKNOWN)
    {
        gcmONERROR(
            gcoHARDWARE_Unlock(Surface->hal->hardware,
                               &Surface->hzTileStatusNode,
                               gcvSURF_TILE_STATUS));

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Unlocked HZ tile status 0x%x: lockCount=%d",
                      &Surface->info.hzNode,
                      Surface->info.hzNode.lockCount);
    }

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    /* Return the errror. */
    return status;
}

static gceSTATUS
_AllocateTileStatus(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;
    gctSIZE_T bytes;
    gctUINT alignment;
    gcsHAL_INTERFACE iface;

    /* Query the linear size for the tile status buffer. */
    status = gcoHARDWARE_QueryTileStatus(Surface->hal->hardware,
                                         Surface->info.alignedWidth,
                                         Surface->info.alignedHeight,
                                         Surface->info.size,
                                         &bytes,
                                         &alignment,
                                         &Surface->tileStatusNode.filler);

    /* Tile status supported? */
    if (status == gcvSTATUS_NOT_SUPPORTED)
    {
        return gcvSTATUS_OK;
    }

    /* Copy filler. */
    Surface->hzTileStatusNode.filler = Surface->tileStatusNode.filler;

    /* Allocate the tile status buffer. */
    iface.command     = gcvHAL_ALLOCATE_LINEAR_VIDEO_MEMORY;
    gcmALVM.bytes     = bytes;
    gcmALVM.alignment = alignment;
    gcmALVM.type      = gcvSURF_TILE_STATUS;
    gcmALVM.pool      = gcvPOOL_DEFAULT;

    if (gcmIS_ERROR(gcoHAL_Call(Surface->hal, &iface)))
    {
        /* Commit any command buffer and wait for idle hardware. */
        status = gcoHAL_Commit(Surface->hal, gcvTRUE);

        if (gcmIS_SUCCESS(status))
        {
            /* Try allocating again. */
            status = gcoHAL_Call(Surface->hal, &iface);
        }
    }

    if (gcmIS_SUCCESS(status))
    {
        /* Set the node for the tile status buffer. */
        Surface->tileStatusNode.u.normal.node = gcmALVM.node;
        Surface->tileStatusNode.pool          = gcmALVM.pool;
        Surface->tileStatusNode.size          = gcmALVM.bytes;
        Surface->tileStatusNode.firstLock     = gcvTRUE;

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Allocated tile status 0x%x: pool=%d size=%u",
                      &Surface->tileStatusNode,
                      Surface->tileStatusNode.pool,
                      Surface->tileStatusNode.size);

        /* Allocate tile status for hierarchical Z buffer. */
        if (Surface->info.hzNode.pool != gcvPOOL_UNKNOWN)
        {
            /* Query the linear size for the tile status buffer. */
            status = gcoHARDWARE_QueryTileStatus(Surface->hal->hardware,
                                                 0,
                                                 0,
                                                 Surface->info.hzNode.size,
                                                 &bytes,
                                                 &alignment,
                                                 gcvNULL);

            /* Tile status supported? */
            if (status == gcvSTATUS_NOT_SUPPORTED)
            {
                return gcvSTATUS_OK;
            }

            /* Allocate the tile status buffer. */
            iface.command     = gcvHAL_ALLOCATE_LINEAR_VIDEO_MEMORY;
            gcmALVM.bytes     = bytes;
            gcmALVM.alignment = alignment;
            gcmALVM.type      = gcvSURF_TILE_STATUS;
            gcmALVM.pool      = gcvPOOL_DEFAULT;

            if (gcmIS_SUCCESS(gcoHAL_Call(Surface->hal, &iface)))
            {
                /* Set the node for the tile status buffer. */
                Surface->hzTileStatusNode.u.normal.node = gcmALVM.node;
                Surface->hzTileStatusNode.pool          = gcmALVM.pool;
                Surface->hzTileStatusNode.size          = gcmALVM.bytes;
                Surface->hzTileStatusNode.firstLock     = gcvTRUE;

                gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                              "Allocated HZ tile status 0x%x: pool=%d size=%u",
                              &Surface->hzTileStatusNode,
                              Surface->hzTileStatusNode.pool,
                              Surface->hzTileStatusNode.size);
            }
        }
    }

    /* Return the status. */
    return status;
}

static gceSTATUS
_FreeSurface(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;

    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* We only manage valid and non-user pools. */
    if ((Surface->info.node.pool != gcvPOOL_UNKNOWN)
    &&  (Surface->info.node.pool != gcvPOOL_USER)
    )
    {
        /* Unlock the video memory. */
        gcmONERROR(_Unlock(Surface));

        /* Free the video memory. */
        gcmONERROR(
            gcoHARDWARE_ScheduleVideoMemory(Surface->hal->hardware,
                                            &Surface->info.node));

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Freed surface 0x%x",
                      &Surface->info.node);

        /* Mark the memory as freed. */
        Surface->info.node.pool = gcvPOOL_UNKNOWN;
    }

    if (Surface->info.hzNode.pool != gcvPOOL_UNKNOWN)
    {
        /* Free the hierarchical Z video memory. */
        gcmONERROR(
            gcoHARDWARE_ScheduleVideoMemory(Surface->hal->hardware,
                                            &Surface->info.hzNode));

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Freed HZ surface 0x%x",
                      &Surface->info.hzNode);

        /* Mark the memory as freed. */
        Surface->info.hzNode.pool = gcvPOOL_UNKNOWN;
    }

    if (Surface->tileStatusNode.pool != gcvPOOL_UNKNOWN)
    {
        /* Free the tile status memory. */
        gcmONERROR(
            gcoHARDWARE_ScheduleVideoMemory(Surface->hal->hardware,
                                            &Surface->tileStatusNode));

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Freed tile status 0x%x",
                      &Surface->tileStatusNode);

        /* Mark the tile status as freed. */
        Surface->tileStatusNode.pool = gcvPOOL_UNKNOWN;
    }

    if (Surface->hzTileStatusNode.pool != gcvPOOL_UNKNOWN)
    {
        /* Free the hierarchical Z tile status memory. */
        gcmONERROR(
            gcoHARDWARE_ScheduleVideoMemory(Surface->hal->hardware,
                                            &Surface->hzTileStatusNode));

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                      "Freed HZ tile status 0x%x",
                      &Surface->hzTileStatusNode);

        /* Mark the tile status as freed. */
        Surface->hzTileStatusNode.pool = gcvPOOL_UNKNOWN;
    }

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_AllocateSurface(
    IN gcoSURF Surface,
    IN gctUINT Width,
    IN gctUINT Height,
    IN gctUINT Depth,
    IN gceSURF_TYPE Type,
    IN gceSURF_FORMAT Format,
    IN gcePOOL Pool
    )
{
    gceSTATUS status;
    gctUINT32 bitsPerPixel;
    gcsHAL_INTERFACE iface;

    /* Compute bits per pixel. */
    gcmONERROR(
        gcoHARDWARE_ConvertFormat(Surface->hal->hardware,
                                  Format,
                                  &bitsPerPixel,
                                  gcvNULL));

    /* Set dimensions of surface. */
    Surface->info.rect.left   = 0;
    Surface->info.rect.top    = 0;
    Surface->info.rect.right  = Width;
    Surface->info.rect.bottom = Height;

    /* Set the number of planes. */
    Surface->depth = Depth;

    /* Initialize rotation. */
    Surface->info.rotation    = gcvSURF_0_DEGREE;

    /* Set type and format of surface. */
    Surface->info.type   = (gceSURF_TYPE)
                           ((gctUINT32) Type & ~gcvSURF_NO_TILE_STATUS);
    Surface->info.format = Format;

    /* Set aligned surface size. */
    Surface->info.alignedWidth  = Width;
    Surface->info.alignedHeight = Height;
    Surface->info.is16Bit       = (bitsPerPixel == 16);

    /* Init superTiled info. */
    Surface->info.superTiled = gcvFALSE;

    /* Reset the node. */
    Surface->info.node.valid          = gcvFALSE;
    Surface->info.node.lockCount      = 0;
    Surface->info.node.lockedInKernel = 0;
    Surface->info.node.count          = 0;
    Surface->info.node.size           = 0;
    Surface->info.node.firstLock      = gcvTRUE;
    Surface->info.node.pool           = gcvPOOL_UNKNOWN;

    /* User pool? */
    if (Pool == gcvPOOL_USER)
    {
        /* Init the node as the user allocated. */
        Surface->info.node.pool                    = gcvPOOL_USER;
        Surface->info.node.u.wrapped.logicalMapped = gcvFALSE;
        Surface->info.node.u.wrapped.mappingInfo   = gcvNULL;
        Surface->info.node.size                    = Surface->info.alignedWidth
                                                   * (bitsPerPixel / 8)
                                                   * Surface->info.alignedHeight
                                                   * Surface->depth;
    }

    /* No --> allocate video memory. */
    else
    {
        /* Align width and height to tiles. */
        gcmONERROR(
            gcoHARDWARE_AlignToTile(Surface->hal->hardware,
                                    Type,
                                    &Surface->info.alignedWidth,
                                    &Surface->info.alignedHeight,
                                    &Surface->info.superTiled));

        iface.command     = gcvHAL_ALLOCATE_LINEAR_VIDEO_MEMORY;
        gcmALVM.bytes     = Surface->info.alignedWidth * bitsPerPixel / 8
                          * Surface->info.alignedHeight
                          * Depth;
        gcmALVM.alignment = 64;
        gcmALVM.pool      = Pool;
        gcmALVM.type      = (gceSURF_TYPE)
                            ((gctUINT32) Type & ~gcvSURF_NO_TILE_STATUS);

        /* Call kernel API. */
        gcmONERROR(gcoHAL_Call(Surface->hal, &iface));

        /* Get allocated node in video memory. */
        Surface->info.node.u.normal.node = gcmALVM.node;
        Surface->info.node.pool          = gcmALVM.pool;
        Surface->info.node.size          = gcmALVM.bytes;
    }

    /* Determine the surface placement parameters. */
    switch (Format)
    {
    case gcvSURF_YV12:
        Surface->info.vOffset
            = Surface->info.alignedWidth
            * Surface->info.alignedHeight;

        Surface->info.uOffset
            = Surface->info.vOffset
            + Surface->info.vOffset / 4;

        Surface->info.stride
            = Surface->info.alignedWidth;

        Surface->info.uStride
            = Surface->info.vStride
            = Surface->info.alignedWidth / 2;

        Surface->info.size
            = Surface->info.vOffset
            + Surface->info.vOffset / 2;
        break;

    case gcvSURF_I420:
        Surface->info.uOffset
            = Surface->info.alignedWidth
            * Surface->info.alignedHeight;

        Surface->info.vOffset
            = Surface->info.uOffset
            + Surface->info.uOffset / 4;

        Surface->info.stride
            = Surface->info.alignedWidth;

        Surface->info.uStride
            = Surface->info.vStride
            = Surface->info.alignedWidth / 2;

        Surface->info.size
            = Surface->info.uOffset
            + Surface->info.uOffset / 2;
        break;

    case gcvSURF_NV12:
    case gcvSURF_NV21:
        Surface->info.uOffset
            = Surface->info.vOffset
            = Surface->info.alignedWidth
            * Surface->info.alignedHeight;

        Surface->info.stride
            = Surface->info.uStride
            = Surface->info.vStride
            = Surface->info.alignedWidth;

        Surface->info.size
            = Surface->info.uOffset
            + Surface->info.uOffset / 2;
        break;

    case gcvSURF_NV16:
    case gcvSURF_NV61:
        Surface->info.uOffset
            = Surface->info.vOffset
            = Surface->info.alignedWidth
            * Surface->info.alignedHeight;

        Surface->info.stride
            = Surface->info.uStride
            = Surface->info.vStride
            = Surface->info.alignedWidth;

        Surface->info.size
            = Surface->info.uOffset
            + Surface->info.uOffset;
        break;

    default:
        Surface->info.uOffset = Surface->info.vOffset = 0;
        Surface->info.uStride = Surface->info.vStride = 0;

        Surface->info.stride
            = Surface->info.alignedWidth
            * bitsPerPixel / 8;

        Surface->info.size
            = Surface->info.stride
            * Surface->info.alignedHeight;
    }

    gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                  "Allocated surface 0x%x: pool=%d size=%dx%d bytes=%u",
                  &Surface->info.node,
                  Surface->info.node.pool,
                  Surface->info.alignedWidth,
                  Surface->info.alignedHeight,
                  Surface->info.size);

    /* No Hierarchical Z buffer allocated. */
    Surface->info.hzNode.pool           = gcvPOOL_UNKNOWN;
    Surface->info.hzNode.valid          = gcvFALSE;
    Surface->info.hzNode.lockCount      = 0;
    Surface->info.hzNode.lockedInKernel = 0;
    Surface->info.hzNode.count          = 0;
    Surface->info.hzNode.size           = 0;

    /* Check if this is a depth buffer and the GPU supports hierarchical Z. */
    if ((Type == gcvSURF_DEPTH)
    &&  (gcoHAL_IsFeatureAvailable(Surface->hal,
                                   gcvFEATURE_HZ) == gcvSTATUS_TRUE)
    )
    {
        gctSIZE_T bytes;

        /* Compute the hierarchical Z buffer size.  Allocate enough for
        ** 16-bit min/max values. */
        bytes = gcmALIGN((Surface->info.size + 63) / 64 * 4, 256);

        /* Allocate the hierarchical Z buffer. */
        iface.command     = gcvHAL_ALLOCATE_LINEAR_VIDEO_MEMORY;
        gcmALVM.bytes     = bytes;
        gcmALVM.alignment = 64;
        gcmALVM.type      = gcvSURF_HIERARCHICAL_DEPTH;
        gcmALVM.pool      = Pool;

        if (gcmIS_SUCCESS(gcoHAL_Call(Surface->hal, &iface)))
        {
            /* Save hierarchical Z buffer info. */
            Surface->info.hzNode.pool          = gcmALVM.pool;
            Surface->info.hzNode.size          = gcmALVM.bytes;
            Surface->info.hzNode.u.normal.node = gcmALVM.node;
            Surface->info.hzNode.firstLock     = gcvTRUE;

            gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                          "Allocated HZ surface 0x%x: pool=%d size=%u",
                          &Surface->info.hzNode,
                          Surface->info.hzNode.pool,
                          Surface->info.hzNode.size);
        }
    }

    /* No tile status buffer allocated. */
    Surface->tileStatusNode.pool             = gcvPOOL_UNKNOWN;
    Surface->tileStatusNode.valid            = gcvFALSE;
    Surface->tileStatusNode.lockCount        = 0;
    Surface->tileStatusNode.lockedInKernel   = 0;
    Surface->tileStatusNode.count            = 0;
    Surface->hzTileStatusNode.pool           = gcvPOOL_UNKNOWN;
    Surface->hzTileStatusNode.valid          = gcvFALSE;
    Surface->hzTileStatusNode.lockCount      = 0;
    Surface->hzTileStatusNode.lockedInKernel = 0;
    Surface->hzTileStatusNode.count          = 0;

    /* Default state of tileStatusDisabled. */
    Surface->info.tileStatusDisabled = gcvFALSE;

    /* Set default fill color. */
    switch (Format)
    {
    case gcvSURF_D16:
        Surface->info.clearValue = 0xFFFFFFFF;
        break;

    case gcvSURF_D24X8:
    case gcvSURF_D24S8:
        Surface->info.clearValue = 0xFFFFFF00;
        break;

    default:
        Surface->info.clearValue = 0x00000000;
        break;
    }

    /* Verify if the type requires a tile status buffer:
    ** - do not turn on fast clear if the surface is virtual;
    ** - for user pools we don't have the address of the surface yet,
    **   delay tile status determination until we map the surface. */
    if ((Surface->info.node.pool != gcvPOOL_USER)
    &&  (Surface->info.node.pool != gcvPOOL_VIRTUAL)
    &&  (  (Type == gcvSURF_RENDER_TARGET)
        || (Type == gcvSURF_DEPTH)
        )
    )
    {
        _AllocateTileStatus(Surface);
    }

    if (Pool != gcvPOOL_USER)
    {
        /* Lock the surface. */
        gcmONERROR(_Lock(Surface));
    }

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    /* Free the memory allocated to the surface. */
    _FreeSurface(Surface);

    /* Return the status. */
    return status;
}

static gceSTATUS
_UnmapUserBuffer(
    IN gcoSURF Surface,
    IN gctBOOL ForceUnmap
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    do
    {
        /* Cannot be negative. */
        gcmASSERT(Surface->info.node.lockCount >= 0);

        /* Nothing is mapped? */
        if (Surface->info.node.lockCount == 0)
        {
            /* Nothing to do. */
            status = gcvSTATUS_OK;
            break;
        }

        /* Make sure the reference couner is proper. */
        if (Surface->info.node.lockCount > 1)
        {
            /* Forced unmap? */
            if (ForceUnmap)
            {
                /* Invalid reference count. */
                gcmASSERT(gcvFALSE);
            }
            else
            {
                /* Decrement. */
                Surface->info.node.lockCount -= 1;

                /* Done for now. */
                status = gcvSTATUS_OK;
                break;
            }
        }

        /* Unmap the logical memory. */
        if (Surface->info.node.u.wrapped.logicalMapped)
        {
            gcmERR_BREAK(gcoHAL_ScheduleUnmapMemory(
                Surface->hal,
                gcmINT2PTR(Surface->info.node.physical),
                Surface->info.size,
                Surface->info.node.logical
                ));

            Surface->info.node.physical = ~0U;
            Surface->info.node.u.wrapped.logicalMapped = gcvFALSE;
        }

        /* Unmap the physical memory. */
        if (Surface->info.node.u.wrapped.mappingInfo != gcvNULL)
        {
            gcmERR_BREAK(gcoHAL_ScheduleUnmapUserMemory(
                Surface->hal,
                Surface->info.node.u.wrapped.mappingInfo,
                Surface->info.size,
                Surface->info.node.physical,
                Surface->info.node.logical
                ));

            Surface->info.node.logical = gcvNULL;
            Surface->info.node.u.wrapped.mappingInfo = gcvNULL;
        }

        /* Reset the surface. */
        Surface->info.node.lockCount = 0;
        Surface->info.node.valid = gcvFALSE;
    }
    while (gcvFALSE);

    /* Return the status. */
    return status;
}

/******************************************************************************\
******************************** gcoSURF API Code *******************************
\******************************************************************************/

/*******************************************************************************
**
**  gcoSURF_Construct
**
**  Create a new gcoSURF object.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to an gcoHAL object.
**
**      gctUINT Width
**          Width of surface to create in pixels.
**
**      gctUINT Height
**          Height of surface to create in lines.
**
**      gctUINT Depth
**          Depth of surface to create in planes.
**
**      gceSURF_TYPE Type
**          Type of surface to create.
**
**      gceSURF_FORMAT Format
**          Format of surface to create.
**
**      gcePOOL Pool
**          Pool to allocate surface from.
**
**  OUTPUT:
**
**      gcoSURF * Surface
**          Pointer to the variable that will hold the gcoSURF object pointer.
*/
gceSTATUS
gcoSURF_Construct(
    IN gcoHAL Hal,
    IN gctUINT Width,
    IN gctUINT Height,
    IN gctUINT Depth,
    IN gceSURF_TYPE Type,
    IN gceSURF_FORMAT Format,
    IN gcePOOL Pool,
    OUT gcoSURF * Surface
    )
{
    gcoSURF surface = gcvNULL;
    gceSTATUS status;

    gcmHEADER_ARG("Hal=0x%x Width=%u Height=%u Depth=%u Type=%d Format=%d Pool=%d",
              Hal, Width, Height, Depth, Type, Format, Pool);

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(Hal != gcvNULL);
    gcmVERIFY_ARGUMENT(Surface != gcvNULL);

    /* Allocate the gcoSURF object. */
    gcmONERROR(
        gcoOS_Allocate(Hal->os,
                       gcmSIZEOF(struct _gcoSURF),
                       (gctPOINTER *) &surface));

    /* Initialize the gcoSURF object.*/
    surface->object.type = gcvOBJ_SURF;
    surface->hal         = Hal;

    /* 1 sample per pixel, no VAA. */
    surface->info.samples.x = 1;
    surface->info.samples.y = 1;
    surface->info.vaa       = gcvFALSE;
    surface->info.dirty     = gcvTRUE;

    surface->colorType = gcvSURF_COLOR_UNKNOWN;

    surface->info.hzNode.pool = gcvPOOL_UNKNOWN;
    surface->tileStatusNode.pool = gcvPOOL_UNKNOWN;
    surface->hzTileStatusNode.pool = gcvPOOL_UNKNOWN;

    /* Allocate surface. */
    gcmONERROR(
        _AllocateSurface(surface,
                         Width, Height, Depth,
                         Type,
                         Format,
                         Pool));

    surface->referenceCount = 1;

    gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                  "Created gcoSURF 0x%x",
                  surface);

    /* Return pointer to the gcoSURF object. */
    *Surface = surface;

    /* Success. */
    gcmFOOTER_ARG("*Surface=0x%x", *Surface);
    return gcvSTATUS_OK;

OnError:
    /* Free the allocated memory. */
    if (surface != gcvNULL)
    {
        gcoOS_Free(Hal->os, surface);
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_Destroy
**
**  Destroy an gcoSURF object.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object to be destroyed.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_Destroy(
    IN gcoSURF Surface
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Decrement surface reference count. */
    if (--Surface->referenceCount != 0)
    {
        /* Still references to this surface. */
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }


    if (Surface->hal->dump != gcvNULL)
    {
        /* Dump the deletion. */
        gcmVERIFY_OK(
            gcoDUMP_Delete(Surface->hal->dump, Surface->info.node.physical));
    }

    /* User-allocated surface? */
    if (Surface->info.node.pool == gcvPOOL_USER)
    {
        gcmVERIFY_OK(
            _UnmapUserBuffer(Surface, gcvTRUE));
    }

    /* Free the video memory. */
    gcmVERIFY_OK(_FreeSurface(Surface));

    /* Mark gcoSURF object as unknown. */
    Surface->object.type = gcvOBJ_UNKNOWN;

    /* Free the gcoSURF object. */
    gcmVERIFY_OK(gcoOS_Free(Surface->hal->os, Surface));

    gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_SURFACE,
                  "Destroyed gcoSURF 0x%x",
                  Surface);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoSURF_MapUserSurface
**
**  Store the logical and physical pointers to the user-allocated surface in
**  the gcoSURF object and map the pointers as necessary.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object to be destroyed.
**
**      gctUINT Alignment
**          Alignment of each pixel row in bytes.
**
**      gctPOINTER Logical
**          Logical pointer to the user allocated surface or gcvNULL if no
**          logical pointer has been provided.
**
**      gctUINT32 Physical
**          Physical pointer to the user allocated surface or ~0 if no
**          physical pointer has been provided.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_MapUserSurface(
    IN gcoSURF Surface,
    IN gctUINT Alignment,
    IN gctPOINTER Logical,
    IN gctUINT32 Physical
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gctBOOL logicalMapped = gcvFALSE;
    gctPOINTER mappingInfo = gcvNULL;

    gctPOINTER logical = gcvNULL;
    gctUINT32 physical = 0;
    gctUINT32 bitsPerPixel;

    gcmHEADER_ARG("Surface=0x%x Alignment=%u Logical=0x%x Physical=%08x",
              Surface, Alignment, Logical, Physical);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    do
    {
        /* Has to be user-allocated surface. */
        if (Surface->info.node.pool != gcvPOOL_USER)
        {
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Already mapped? */
        if (Surface->info.node.lockCount > 0)
        {
            if ((Logical != gcvNULL) &&
                (Logical != Surface->info.node.logical))
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
                break;
            }

            if ((Physical != ~0) &&
                (Physical != Surface->info.node.physical))
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
                break;
            }

            /* Success. */
            break;
        }

        /* Set new alignment. */
        if (Alignment != 0)
        {
            gctUINT32 stride;

            /* Compute bits per pixel. */
            gcmERR_BREAK(gcoHARDWARE_ConvertFormat(
                Surface->hal->hardware,
                Surface->info.format,
                &bitsPerPixel,
                gcvNULL
                ));

            /* Compute the unaligned stride. */
            stride = Surface->info.alignedWidth * bitsPerPixel / 8;

            /* Align the stide (Alignment can be not a power of number). */
            if ((stride % Alignment) != 0)
            {
                stride = (stride / Alignment) * Alignment + Alignment;
            }

            /* Set the new stride. */
            Surface->info.stride = stride;

            /* Compute the new size. */
            Surface->info.size
                = stride * Surface->info.alignedHeight;
        }

        /* Map logical pointer if not specified. */
        if (Logical == gcvNULL)
        {
            if (Physical == ~0)
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
                break;
            }

            /* Map the logical pointer. */
            gcmERR_BREAK(gcoHAL_MapMemory(
                Surface->hal,
                gcmINT2PTR(Physical),
                Surface->info.size,
                &logical
                ));

            /* Mark as mapped. */
            logicalMapped = gcvTRUE;
        }
        else
        {
            /* Set the logical pointer. */
            logical = Logical;
        }

        /* Map physical pointer if not specified. */
        if (Physical == ~0)
        {
            if (Logical == gcvNULL)
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
                break;
            }

            /* Map the physical pointer. */
            gcmERR_BREAK(gcoOS_MapUserMemory(
                Surface->hal->os,
                Logical,
                Surface->info.size,
                &mappingInfo,
                &physical
                ));
        }
        else
        {
            /* Set the physical pointer. */
            physical = Physical;
        }

        /* Validate the surface. */
        Surface->info.node.valid = gcvTRUE;

        /* Set the lock count. */
        Surface->info.node.lockCount++;

        /* Compute bits per pixel. */
        gcmERR_BREAK(gcoHARDWARE_ConvertFormat(
            Surface->hal->hardware,
            Surface->info.format,
            &bitsPerPixel,
            gcvNULL
            ));

        /* Set the node parameters. */
        Surface->info.node.u.wrapped.logicalMapped = logicalMapped;
        Surface->info.node.u.wrapped.mappingInfo   = mappingInfo;
        Surface->info.node.logical                 = logical;
        Surface->info.node.physical                = physical;
        Surface->info.node.count                   = 1;
    }
    while (gcvFALSE);

    /* Roll back. */
    if (gcmIS_ERROR(status))
    {
        if (logicalMapped)
        {
            gcmVERIFY_OK(gcoHAL_UnmapMemory(
                Surface->hal,
                gcmINT2PTR(physical),
                Surface->info.size,
                logical
                ));
        }

        if (mappingInfo != gcvNULL)
        {
            gcmVERIFY_OK(gcoOS_UnmapUserMemory(
                Surface->hal->os,
                logical,
                Surface->info.size,
                mappingInfo,
                physical
                ));
        }
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}


/*******************************************************************************
**
**  gcoSURF_GetSize
**
**  Get the size of an gcoSURF object.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**  OUTPUT:
**
**      gctUINT * Width
**          Pointer to variable that will receive the width of the gcoSURF
**          object.  If 'Width' is gcvNULL, no width information shall be returned.
**
**      gctUINT * Height
**          Pointer to variable that will receive the height of the gcoSURF
**          object.  If 'Height' is gcvNULL, no height information shall be returned.
**
**      gctUINT * Depth
**          Pointer to variable that will receive the depth of the gcoSURF
**          object.  If 'Depth' is gcvNULL, no depth information shall be returned.
*/
gceSTATUS
gcoSURF_GetSize(
    IN gcoSURF Surface,
    OUT gctUINT * Width,
    OUT gctUINT * Height,
    OUT gctUINT * Depth
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (Width != gcvNULL)
    {
        /* Return the width. */
        *Width = Surface->info.rect.right / Surface->info.samples.x;
    }

    if (Height != gcvNULL)
    {
        /* Return the height. */
        *Height = Surface->info.rect.bottom / Surface->info.samples.y;
    }

    if (Depth != gcvNULL)
    {
        /* Return the depth. */
        *Depth = Surface->depth;
    }

    /* Success. */
    gcmFOOTER_ARG("*Width=%u *Height=%u *Depth=%u",
                  (Width  == gcvNULL) ? 0 : *Width,
                  (Height == gcvNULL) ? 0 : *Height,
                  (Depth  == gcvNULL) ? 0 : *Depth);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoSURF_GetAlignedSize
**
**  Get the aligned size of an gcoSURF object.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**  OUTPUT:
**
**      gctUINT * Width
**          Pointer to variable that receives the aligned width of the gcoSURF
**          object.  If 'Width' is gcvNULL, no width information shall be returned.
**
**      gctUINT * Height
**          Pointer to variable that receives the aligned height of the gcoSURF
**          object.  If 'Height' is gcvNULL, no height information shall be
**          returned.
**
**      gctINT * Stride
**          Pointer to variable that receives the stride of the gcoSURF object.
**          If 'Stride' is gcvNULL, no stride information shall be returned.
*/
gceSTATUS
gcoSURF_GetAlignedSize(
    IN gcoSURF Surface,
    OUT gctUINT * Width,
    OUT gctUINT * Height,
    OUT gctINT * Stride
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (Width != gcvNULL)
    {
        /* Return the aligned width. */
        *Width = Surface->info.alignedWidth;
    }

    if (Height != gcvNULL)
    {
        /* Return the aligned height. */
        *Height = Surface->info.alignedHeight;
    }

    if (Stride != gcvNULL)
    {
        /* Return the stride. */
        *Stride = Surface->info.stride;
    }

    /* Success. */
    gcmFOOTER_ARG("*Width=%u *Height=%u *Stride=%d",
                  (Width  == gcvNULL) ? 0 : *Width,
                  (Height == gcvNULL) ? 0 : *Height,
                  (Stride == gcvNULL) ? 0 : *Stride);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoSURF_GetFormat
**
**  Get surface type and format.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**  OUTPUT:
**
**      gceSURF_TYPE * Type
**          Pointer to variable that receives the type of the gcoSURF object.
**          If 'Type' is gcvNULL, no type information shall be returned.
**
**      gceSURF_FORMAT * Format
**          Pointer to variable that receives the format of the gcoSURF object.
**          If 'Format' is gcvNULL, no format information shall be returned.
*/
gceSTATUS
gcoSURF_GetFormat(
    IN gcoSURF Surface,
    OUT gceSURF_TYPE * Type,
    OUT gceSURF_FORMAT * Format
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    if (Type != gcvNULL)
    {
        /* Return the surface type. */
        *Type = Surface->info.type;
    }

    if (Format != gcvNULL)
    {
        /* Return the surface format. */
        *Format = Surface->info.format;
    }

    /* Success. */
    gcmFOOTER_ARG("*Type=%d *Format=%d",
                  (Type   == gcvNULL) ? 0 : *Type,
                  (Format == gcvNULL) ? 0 : *Format);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoSURF_Lock
**
**  Lock the surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**  OUTPUT:
**
**      gctUINT32 * Address
**          Physical address array of the surface:
**          For YV12, Address[0] is for Y channel,
**                    Address[1] is for V channel and
**                    Address[2] is for U channel;
**          For I420, Address[0] is for Y channel,
**                    Address[1] is for U channel and
**                    Address[2] is for V channel;
**          For NV12, Address[0] is for Y channel and
**                    Address[1] is for UV channel;
**          For all other formats, only Address[0] is used to return the
**          physical address.
**
**      gctPOINTER * Memory
**          Logical address array of the surface:
**          For YV12, Memory[0] is for Y channel,
**                    Memory[1] is for V channel and
**                    Memory[2] is for U channel;
**          For I420, Memory[0] is for Y channel,
**                    Memory[1] is for U channel and
**                    Memory[2] is for V channel;
**          For NV12, Memory[0] is for Y channel and
**                    Memory[1] is for UV channel;
**          For all other formats, only Memory[0] is used to return the logical
**          address.
*/
gceSTATUS
gcoSURF_Lock(
    IN gcoSURF Surface,
    OPTIONAL OUT gctUINT32 * Address,
    OPTIONAL OUT gctPOINTER * Memory
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Lock the surface. */
    gcmONERROR(_Lock(Surface));

    /* Determine surface addresses. */
    switch (Surface->info.format)
    {
    case gcvSURF_YV12:
    case gcvSURF_I420:
        Surface->info.node.count = 3;

        Surface->info.node.logical2  = Surface->info.node.logical
                                     + Surface->info.uOffset;

        Surface->info.node.physical2 = Surface->info.node.physical
                                     + Surface->info.uOffset;

        Surface->info.node.logical3  = Surface->info.node.logical
                                     + Surface->info.vOffset;

        Surface->info.node.physical3 = Surface->info.node.physical
                                     + Surface->info.vOffset;
        break;

    case gcvSURF_NV12:
        Surface->info.node.count = 2;

        Surface->info.node.logical2  = Surface->info.node.logical
                                     + Surface->info.uOffset;

        Surface->info.node.physical2 = Surface->info.node.physical
                                     + Surface->info.uOffset;
        break;

    default:
        Surface->info.node.count = 1;
    }

    /* Set result. */
    if (Address != gcvNULL)
    {
        switch (Surface->info.node.count)
        {
        case 3:
            Address[2] = Surface->info.node.physical3;

        case 2:
            Address[1] = Surface->info.node.physical2;

        case 1:
            Address[0] = Surface->info.node.physical;
        }
    }

    if (Memory != gcvNULL)
    {
        switch (Surface->info.node.count)
        {
        case 3:
            Memory[2] = Surface->info.node.logical3;

        case 2:
            Memory[1] = Surface->info.node.logical2;

        case 1:
            Memory[0] = Surface->info.node.logical;
        }
    }

    /* Success. */
    gcmFOOTER_ARG("*Address=%08X *Memory=0x%x",
                  (Address == gcvNULL) ? 0 : *Address,
                  (Memory  == gcvNULL) ? gcvNULL : *Memory);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_Unlock
**
**  Unlock the surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**      gctPOINTER Memory
**          Pointer to mapped memory.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_Unlock(
    IN gcoSURF Surface,
    IN gctPOINTER Memory
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x Memory=0x%x", Surface, Memory);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Unlock the surface. */
    gcmONERROR(_Unlock(Surface));

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
**  gcoSURF_Fill
**
**  Fill surface with specified value.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**      gcsPOINT_PTR Origin
**          Pointer to the origin of the area to be filled.
**          Assumed to (0, 0) if gcvNULL is given.
**
**      gcsSIZE_PTR Size
**          Pointer to the size of the area to be filled.
**          Assumed to the size of the surface if gcvNULL is given.
**
**      gctUINT32 Value
**          Value to be set.
**
**      gctUINT32 Mask
**          Value mask.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_Fill(
    IN gcoSURF Surface,
    IN gcsPOINT_PTR Origin,
    IN gcsSIZE_PTR Size,
    IN gctUINT32 Value,
    IN gctUINT32 Mask
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
**
**  gcoSURF_Blend
**
**  Alpha blend two surfaces together.
**
**  INPUT:
**
**      gcoSURF SrcSurface
**          Pointer to the source gcoSURF object.
**
**      gcoSURF DestSurface
**          Pointer to the destination gcoSURF object.
**
**      gcsPOINT_PTR SrcOrigin
**          The origin within the source.
**          If gcvNULL is specified, (0, 0) origin is assumed.
**
**      gcsPOINT_PTR DestOrigin
**          The origin within the destination.
**          If gcvNULL is specified, (0, 0) origin is assumed.
**
**      gcsSIZE_PTR Size
**          The size of the area to be blended.
**
**      gceSURF_BLEND_MODE Mode
**          One of the blending modes.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_Blend(
    IN gcoSURF SrcSurface,
    IN gcoSURF DestSurface,
    IN gcsPOINT_PTR SrcOrigin,
    IN gcsPOINT_PTR DestOrigin,
    IN gcsSIZE_PTR Size,
    IN gceSURF_BLEND_MODE Mode
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}


/*******************************************************************************
**
**  gcoSURF_SetClipping
**
**  Set cipping rectangle to the size of the surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_SetClipping(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    status = gcoHARDWARE_SetClipping(
        Surface->hal->hardware,
        &Surface->info.rect
        );
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_Clear2D
**
**  Clear one or more rectangular areas.
**
**  INPUT:
**
**      gcoSURF DestSurface
**          Pointer to the destination surface.
**
**      gctUINT32 RectCount
**          The number of rectangles to draw. The array of rectangles
**          pointed to by Rect parameter must have at least RectCount items.
**          Note, that for masked source blits only one destination rectangle
**          is supported.
**
**      gcsRECT_PTR DestRect
**          Pointer to a list of destination rectangles.
**
**      gctUINT32 LoColor
**          Low 32-bit clear color values.
**
**      gctUINT32 HiColor
**          high 32-bit clear color values.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_Clear2D(
    IN gcoSURF DestSurface,
    IN gctUINT32 RectCount,
    IN gcsRECT_PTR DestRect,
    IN gctUINT32 LoColor,
    IN gctUINT32 HiColor
    )
{
    gceSTATUS status;
    gctPOINTER destMemory = gcvNULL;
    gco2D engine;

    gcmHEADER_ARG("DestSurface=0x%x RectCount=%u DestRect=0x%x LoColor=%u HiColor=%u",
              DestSurface, RectCount, DestRect, LoColor, HiColor);

    do
    {
        /* Validate the object. */
        gcmBADOBJECT_BREAK(DestSurface, gcvOBJ_SURF);

        gcmERR_BREAK(gcoHAL_Get2DEngine(
            DestSurface->hal,
            &engine
            ));

        /* Use surface rect if not specified. */
        if (DestRect == gcvNULL)
        {
            if (RectCount != 1)
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
                break;
            }

            DestRect = &DestSurface->info.rect;
        }

        /* Lock the destination. */
        gcmERR_BREAK(gcoSURF_Lock(
            DestSurface,
            gcvNULL,
            &destMemory
            ));

        /* Program the destination. */
        gcmERR_BREAK(gcoHARDWARE_SetTarget(
            DestSurface->hal->hardware,
            &DestSurface->info
            ));

        /* Program the transparencies as opaque. */
        gcmERR_BREAK(gcoHARDWARE_SetTransparencyModes(
            DestSurface->hal->hardware,
            gcv2D_OPAQUE,
            gcv2D_OPAQUE,
            gcv2D_OPAQUE
            ));

        /* Form a CLEAR command. */
        gcmERR_BREAK(gcoHARDWARE_Clear2D(
            DestSurface->hal->hardware,
            RectCount,
            DestRect,
            LoColor,
            gcvFALSE,
            0xCC,
            0xCC
            ));
    }
    while (gcvFALSE);

    /* Unlock the destination. */
    if (destMemory != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(DestSurface, destMemory));
    }

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_Line
**
**  Draw one or more Bresenham lines.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to an gcoSURF object.
**
**      gctUINT32 LineCount
**          The number of lines to draw. The array of line positions pointed
**          to by Position parameter must have at least LineCount items.
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
gcoSURF_Line(
    IN gcoSURF DestSurface,
    IN gctUINT32 LineCount,
    IN gcsRECT_PTR Position,
    IN gcoBRUSH Brush,
    IN gctUINT8 FgRop,
    IN gctUINT8 BgRop
    )
{
    gceSTATUS status;
    gctPOINTER destMemory = gcvNULL;
    gco2D engine;

    gcmHEADER_ARG("DestSurface=0x%x LineCount=%u Position=0x%x Brush=0x%x FgRop=%02x "
              "BgRop=%02x",
              DestSurface, LineCount, Position, Brush, FgRop, BgRop);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(DestSurface, gcvOBJ_SURF);

    do
    {
        gcmERR_BREAK(gcoHAL_Get2DEngine(
            DestSurface->hal,
            &engine
            ));

        /* Lock the destination. */
        gcmERR_BREAK(gcoSURF_Lock(
            DestSurface,
            gcvNULL,
            &destMemory
            ));

        /* Program the destination. */
        gcmERR_BREAK(gcoHARDWARE_SetTarget(
            DestSurface->hal->hardware,
            &DestSurface->info
            ));

        /* Program the transparencies as opaque. */
        gcmERR_BREAK(gcoHARDWARE_SetTransparencyModes(
            DestSurface->hal->hardware,
            gcv2D_OPAQUE,
            gcv2D_OPAQUE,
            gcv2D_OPAQUE
            ));

        /* Draw a LINE command. */
        gcmERR_BREAK(gco2D_Line(
            engine,
            LineCount,
            Position,
            Brush,
            FgRop,
            BgRop,
            DestSurface->info.format
            ));
    }
    while (gcvFALSE);

    /* Unlock the destination. */
    if (destMemory != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(DestSurface, destMemory));
    }

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_Blit
**
**  Generic rectangular blit.
**
**  INPUT:
**
**      OPTIONAL gcoSURF SrcSurface
**          Pointer to the source surface.
**
**      gcoSURF DestSurface
**          Pointer to the destination surface.
**
**      gctUINT32 RectCount
**          The number of rectangles to draw. The array of rectangles
**          pointed to by Rect parameter must have at least RectCount items.
**          Note, that for masked source blits only one destination rectangle
**          is supported.
**
**      OPTIONAL gcsRECT_PTR SrcRect
**          If RectCount is 1, SrcRect represents an sbsolute rectangle within
**          the source surface.
**          If RectCount is greater then 1, (right,bottom) members of SrcRect
**          are ignored and (left,top) members are used as the offset from
**          the origin of each destination rectangle in DestRect list to
**          determine the corresponding source rectangle. In this case the width
**          and the height of the source are assumed the same as of the
**          corresponding destination rectangle.
**
**      gcsRECT_PTR DestRect
**          Pointer to a list of destination rectangles.
**
**      OPTIONAL gcoBRUSH Brush
**          Brush to use for drawing.
**
**      gctUINT8 FgRop
**          Foreground ROP to use with opaque pixels.
**
**      gctUINT8 BgRop
**          Background ROP to use with transparent pixels.
**
**      OPTIONAL gceSURF_TRANSPARENCY Transparency
**          gcvSURF_OPAQUE - each pixel of the bitmap overwrites the destination.
**          gcvSURF_SOURCE_MATCH - source pixels compared against register value
**              to determine the transparency. In simple terms, the transaprency
**              comes down to selecting the ROP code to use. Opaque pixels use
**              foreground ROP and transparent ones use background ROP.
**          gcvSURF_SOURCE_MASK - monochrome source mask defines transparency.
**          gcvSURF_PATTERN_MASK - pattern mask defines transparency.
**
**      OPTIONAL gctUINT32 TransparencyColor
**          This value is used in gcvSURF_SOURCE_MATCH transparency mode.
**          The value is compared against each pixel to determine transparency
**          of the pixel. If the values found equal, the pixel is transparent;
**          otherwise it is opaque.
**
**      OPTIONAL gctPOINTER Mask
**          A pointer to monochrome mask for masked source blits.
**
**      OPTIONAL gceSURF_MONOPACK MaskPack
**          Determines how many horizontal pixels are there per each 32-bit
**          chunk of monochrome mask. For example, if set to gcvSURF_PACKED8,
**          each 32-bit chunk is 8-pixel wide, which also means that it defines
**          4 vertical lines of pixel mask.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_Blit(
    IN OPTIONAL gcoSURF SrcSurface,
    IN gcoSURF DestSurface,
    IN gctUINT32 RectCount,
    IN OPTIONAL gcsRECT_PTR SrcRect,
    IN gcsRECT_PTR DestRect,
    IN OPTIONAL gcoBRUSH Brush,
    IN gctUINT8 FgRop,
    IN gctUINT8 BgRop,
    IN OPTIONAL gceSURF_TRANSPARENCY Transparency,
    IN OPTIONAL gctUINT32 TransparencyColor,
    IN OPTIONAL gctPOINTER Mask,
    IN OPTIONAL gceSURF_MONOPACK MaskPack
    )
{
    gceSTATUS status;

    gcoHARDWARE hardware = gcvNULL;
    gco2D engine;

    gctUINT32 srcTransparency;
    gctUINT32 dstTransparency;
    gctUINT32 patTransparency;

    gctBOOL useBrush;
    gctBOOL useSource;

    gctBOOL stretchBlt = gcvFALSE;
    gctBOOL relativeSource = gcvFALSE;

    gctPOINTER srcMemory  = gcvNULL;
    gctPOINTER destMemory = gcvNULL;

    gcmHEADER_ARG("SrcSurface=0x%x DestSurface=0x%x RectCount=%u SrcRect=0x%x "
              "DestRect=0x%x Brush=0x%x FgRop=%02x BgRop=%02x Transparency=%d "
              "TransparencyColor=%08x Mask=0x%x MaskPack=%d",
              SrcSurface, DestSurface, RectCount, SrcRect, DestRect, Brush,
              FgRop, BgRop, Transparency, TransparencyColor, Mask, MaskPack);

    do
    {
        gctUINT32 destFormat, destFormatSwizzle, destIsYUV;

        /* Validate the object. */
        gcmBADOBJECT_BREAK(DestSurface, gcvOBJ_SURF);

        /* Make a shortcut to the gcoHARDWARE object. */
        hardware = DestSurface->hal->hardware;

        /* Is 2D Engine preset? */
        if (gcmIS_ERROR(gcoHAL_Get2DEngine(DestSurface->hal, &engine)))
        {
            /* No, use software renderer. */
            gcmERR_BREAK(gcoHARDWARE_UseSoftware2D(hardware, gcvTRUE));
        }

        /* Is the destination format supported? */
        if (gcmIS_ERROR(gcoHARDWARE_TranslateDestinationFormat(
                hardware, DestSurface->info.format,
                &destFormat, &destFormatSwizzle, &destIsYUV)))
        {
            /* No, use software renderer. */
            gcmERR_BREAK(gcoHARDWARE_UseSoftware2D(hardware, gcvTRUE));
        }

        /* Translate the specified transparency mode. */
        gcmERR_BREAK(gcoHARDWARE_TranslateSurfTransparency(
            Transparency,
            &srcTransparency,
            &dstTransparency,
            &patTransparency
            ));

        /* Determine the resource usage. */
        gcoHARDWARE_Get2DResourceUsage(
            FgRop, BgRop,
            srcTransparency,
            &useSource, &useBrush, gcvNULL
            );

        /* Use surface rect if not specified. */
        if (DestRect == gcvNULL)
        {
            if (RectCount != 1)
            {
                status = gcvSTATUS_INVALID_ARGUMENT;
                break;
            }

            DestRect = &DestSurface->info.rect;
        }

        /* Setup the brush if needed. */
        if (useBrush)
        {
            /* Program the destination format. */
            gcmERR_BREAK(gcoHARDWARE_SetTargetFormat(
                hardware,
                DestSurface->info.format
                ));

            /* Flush the brush. */
            gcmERR_BREAK(gcoHARDWARE_FlushBrush(
                hardware,
                Brush
            ));
        }

        /* Setup the source if needed. */
        if (useSource)
        {
            /* Validate the object. */
            gcmBADOBJECT_BREAK(SrcSurface, gcvOBJ_SURF);

            /* Use surface rect if not specified. */
            if (SrcRect == gcvNULL)
            {
                SrcRect = &SrcSurface->info.rect;
            }

            /* Lock the source. */
            gcmERR_BREAK(gcoSURF_Lock(
                SrcSurface,
                gcvNULL,
                &srcMemory
                ));

            /* Determine the relative flag. */
            relativeSource = (RectCount > 1) ? gcvTRUE : gcvFALSE;

            /* Program the source. */
            if (Mask == gcvNULL)
            {
                gctBOOL equal;

                /* Check whether this should be a stretch/shrink blit. */
                if ( (gcsRECT_IsOfEqualSize(SrcRect, DestRect, &equal) ==
                          gcvSTATUS_OK) &&
                     !equal )
                {
                    gctUINT32 horFactor, verFactor;

                    /* Calculate the stretch factors. */
                    gcoHARDWARE_GetStretchFactors(
                        SrcRect, DestRect,
                        &horFactor, &verFactor
                        );

                    /* Program the stretch factors. */
                    gcmERR_BREAK(gcoHARDWARE_SetStretchFactors(
                        hardware,
                        horFactor,
                        verFactor
                        ));

                    /* Mark as stretch blit. */
                    stretchBlt = gcvTRUE;
                }

                /* Configure color source. */
                /* Cannot use gco2D calls here as 2D engine may not be present. */
                gcmERR_BREAK(gcoHARDWARE_TranslateSurfTransparency(
                    Transparency,
                    &srcTransparency,
                    &dstTransparency,
                    &patTransparency
                    ));

                /* Set the transparency. */
                gcmERR_BREAK(gcoHARDWARE_SetTransparencyModes(
                    hardware,
                    (gce2D_TRANSPARENCY) srcTransparency,
                    (gce2D_TRANSPARENCY) dstTransparency,
                    (gce2D_TRANSPARENCY) patTransparency
                    ));

                /* Set the transparency color. */
                gcmERR_BREAK(gcoHARDWARE_SetSourceColorKeyRange(
                    hardware,
                    TransparencyColor,
                    TransparencyColor,
                    gcvFALSE
                    ));

                gcmERR_BREAK(gcoHARDWARE_SetColorSource(
                    hardware,
                    &SrcSurface->info,
                    relativeSource
                    ));

                /* Set source rectangle size. */
                gcmERR_BREAK(gcoHARDWARE_SetSource(
                    hardware,
                    SrcRect
                    ));
            }
        }

        /* Lock the destination. */
        gcmERR_BREAK(gcoSURF_Lock(
            DestSurface,
            gcvNULL,
            &destMemory
            ));

        /* Program the destination. */
        gcmERR_BREAK(gcoHARDWARE_SetTarget(
            hardware,
            &DestSurface->info
            ));

        /* Masked sources need to be handled differently. */
        if (useSource && (Mask != gcvNULL))
        {
            gctUINT32 streamPackHeightMask;
            gcsSURF_FORMAT_INFO_PTR srcFormat[2];
            gctUINT32 srcAlignedLeft, srcAlignedTop;
            gctINT32 tileWidth, tileHeight;
            gctUINT32 tileHeightMask;
            gctUINT32 maxHeight;
            gctUINT32 srcBaseAddress;
            gcsRECT srcSubRect;
            gcsRECT destSubRect;
            gcsRECT maskRect;
            gcsPOINT maskSize;
            gctUINT32 lines2render;
            gctUINT32 streamWidth;
            gceSURF_MONOPACK streamPack;

            /* Compute the destination size. */
            gctUINT32 destWidth  = DestRect->right  - DestRect->left;
            gctUINT32 destHeight = DestRect->bottom - DestRect->top;

            /* Query tile size. */
            gcmASSERT(SrcSurface->info.type == gcvSURF_BITMAP);
            gcoHARDWARE_QueryTileSize(
                &tileWidth, &tileHeight,
                gcvNULL, gcvNULL,
                gcvNULL
                );

            tileHeightMask = tileHeight - 1;

            /* Determine left source coordinate. */
            srcSubRect.left = SrcRect->left & 7;

            /* Assume 8-pixel packed stream. */
            streamWidth = gcmALIGN(srcSubRect.left + destWidth, 8);

            /* Do we fit? */
            if (streamWidth == 8)
            {
                streamPack = gcvSURF_PACKED8;
                streamPackHeightMask = ~3U;
            }

            /* Nope, don't fit. */
            else
            {
                /* Assume 16-pixel packed stream. */
                streamWidth = gcmALIGN(srcSubRect.left + destWidth, 16);

                /* Do we fit now? */
                if (streamWidth == 16)
                {
                    streamPack = gcvSURF_PACKED16;
                    streamPackHeightMask = ~1U;
                }

                /* Still don't. */
                else
                {
                    /* Assume unpacked stream. */
                    streamWidth = gcmALIGN(srcSubRect.left + destWidth, 32);
                    streamPack = gcvSURF_UNPACKED;
                    streamPackHeightMask = ~0U;
                }
            }

            /* Determine the maxumum stream height. */
            maxHeight  = (gco2D_GetMaximumDataCount() << 5) / streamWidth;
            maxHeight &= streamPackHeightMask;

            /* Determine the sub source rectangle. */
            srcSubRect.top    = SrcRect->top & tileHeightMask;
            srcSubRect.right  = srcSubRect.left + destWidth;
            srcSubRect.bottom = srcSubRect.top;

            /* Init destination subrectangle. */
            destSubRect.left   = DestRect->left;
            destSubRect.top    = DestRect->top;
            destSubRect.right  = DestRect->right;
            destSubRect.bottom = destSubRect.top;

            /* Determine the number of lines to render. */
            lines2render = srcSubRect.top + destHeight;

            /* Determine the aligned source coordinates. */
            srcAlignedLeft = SrcRect->left - srcSubRect.left;
            srcAlignedTop  = SrcRect->top  - srcSubRect.top;
            gcmASSERT((srcAlignedLeft % tileWidth) == 0);

            /* Get format characteristics. */
            gcmERR_BREAK(gcoSURF_QueryFormat(SrcSurface->info.format, srcFormat));

            /* Determine the initial source address. */
            srcBaseAddress
                = SrcSurface->info.node.physical
                +   srcAlignedTop  * SrcSurface->info.stride
                + ((srcAlignedLeft * srcFormat[0]->bitsPerPixel) >> 3);

            /* Set initial mask coordinates. */
            maskRect.left   = srcAlignedLeft;
            maskRect.top    = srcAlignedTop;
            maskRect.right  = maskRect.left + streamWidth;
            maskRect.bottom = maskRect.top;

            /* Set mask size. */
            maskSize.x = SrcSurface->info.rect.right;
            maskSize.y = SrcSurface->info.rect.bottom;

            do
            {
                /* Determine the area to render in this pass. */
                srcSubRect.top = srcSubRect.bottom & tileHeightMask;
                srcSubRect.bottom = srcSubRect.top + lines2render;
                if (srcSubRect.bottom > (gctINT32) maxHeight)
                    srcSubRect.bottom = maxHeight & ~tileHeightMask;

                destSubRect.top = destSubRect.bottom;
                destSubRect.bottom
                    = destSubRect.top
                    + (srcSubRect.bottom - srcSubRect.top);

                maskRect.top = maskRect.bottom;
                maskRect.bottom = maskRect.top + srcSubRect.bottom;

                /* Set source rectangle size. */
                gcmERR_BREAK(gcoHARDWARE_SetSource(
                    hardware,
                    &srcSubRect
                    ));

                /* Configure masked source. */
                gcmERR_BREAK(gco2D_SetMaskedSource(
                    engine,
                    srcBaseAddress,
                    SrcSurface->info.stride,
                    SrcSurface->info.format,
                    relativeSource,
                    streamPack
                    ));

                /* Do the blit. */
                gcmERR_BREAK(gco2D_MonoBlit(
                    engine,
                    Mask,
                    &maskSize,
                    &maskRect,
                    MaskPack,
                    streamPack,
                    &destSubRect,
                    FgRop,
                    BgRop,
                    DestSurface->info.format
                    ));

                /* Update the source address. */
                srcBaseAddress += srcSubRect.bottom * SrcSurface->info.stride;

                /* Update the line counter. */
                lines2render -= srcSubRect.bottom;
            }
            while (lines2render);
        }
        else
        {
            gce2D_COMMAND command = stretchBlt ? gcv2D_STRETCH : gcv2D_BLT;

            /* Old PE automatically sets the transparency to SOURCE_MASK,
               when there is no source and pattern is used.
               For PE 2.0, this has to be done explicitly.
            */
            gcmERR_BREAK(gcoHARDWARE_SetAutoTransparency(
                hardware,
                FgRop,
                BgRop
                ));

            /* Set the target format. */
            gcmERR_BREAK(gcoHARDWARE_SetTargetFormat(
                hardware,
                DestSurface->info.format
                ));

            /* Set the source. */
            gcmERR_BREAK(gcoHARDWARE_StartDE(
                hardware,
                command,
                1,
                gcvNULL,
                RectCount,
                DestRect,
                FgRop,
                BgRop
                ));
        }
    }
    while (gcvFALSE);

    /* Unlock the source. */
    if (srcMemory != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(SrcSurface, srcMemory));
    }

    /* Unlock the destination. */
    if (destMemory != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(DestSurface, destMemory));
    }

    if (hardware != gcvNULL)
    {
        /* Disable software renderer. */
        gcmVERIFY_OK(gcoHARDWARE_UseSoftware2D(hardware, gcvFALSE));
    }

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_MonoBlit
**
**  Monochrome blit.
**
**  INPUT:
**
**      gcoSURF DestSurface
**          Pointer to the destination surface.
**
**      gctPOINTER Source
**          A pointer to the monochrome bitmap.
**
**      gceSURF_MONOPACK SourcePack
**          Determines how many horizontal pixels are there per each 32-bit
**          chunk of monochrome bitmap. For example, if set to gcvSURF_PACKED8,
**          each 32-bit chunk is 8-pixel wide, which also means that it defines
**          4 vertical lines of pixels.
**
**      gcsPOINT_PTR SourceSize
**          Size of the source monochrome bitmap in pixels.
**
**      gcsPOINT_PTR SourceOrigin
**          Top left coordinate of the source within the bitmap.
**
**      gcsRECT_PTR DestRect
**          Pointer to a list of destination rectangles.
**
**      OPTIONAL gcoBRUSH Brush
**          Brush to use for drawing.
**
**      gctUINT8 FgRop
**          Foreground ROP to use with opaque pixels.
**
**      gctUINT8 BgRop
**          Background ROP to use with transparent pixels.
**
**      gctBOOL ColorConvert
**          The values of FgColor and BgColor parameters are stored directly in
**          internal color registers and are used either directly as the source
**          color or converted to the format of destination before actually
**          used. The later happens if ColorConvert is not zero.
**
**      gctUINT8 MonoTransparency
**          This value is used in gcvSURF_SOURCE_MATCH transparency mode.
**          The value can be either 0 or 1 and is compared against each mono
**          pixel to determine transparency of the pixel. If the values found
**          equal, the pixel is transparent; otherwise it is opaque.
**
**      gceSURF_TRANSPARENCY Transparency
**          gcvSURF_OPAQUE - each pixel of the bitmap overwrites the destination.
**          gcvSURF_SOURCE_MATCH - source pixels compared against register value
**              to determine the transparency. In simple terms, the transaprency
**              comes down to selecting the ROP code to use. Opaque pixels use
**              foreground ROP and transparent ones use background ROP.
**          gcvSURF_SOURCE_MASK - monochrome source mask defines transparency.
**          gcvSURF_PATTERN_MASK - pattern mask defines transparency.
**
**      gctUINT32 FgColor
**          The values are used to represent foreground color
**          of the source. If the values are in destination format, set
**          ColorConvert to 0. Otherwise, provide the values in ARGB8 format
**          and set ColorConvert to 1 to instruct the hardware to convert the
**          values to the destination format before they are actually used.
**
**      gctUINT32 BgColor
**          The values are used to represent background color
**          of the source. If the values are in destination format, set
**          ColorConvert to 0. Otherwise, provide the values in ARGB8 format
**          and set ColorConvert to 1 to instruct the hardware to convert the
**          values to the destination format before they are actually used.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_MonoBlit(
    IN gcoSURF DestSurface,
    IN gctPOINTER Source,
    IN gceSURF_MONOPACK SourcePack,
    IN gcsPOINT_PTR SourceSize,
    IN gcsPOINT_PTR SourceOrigin,
    IN gcsRECT_PTR DestRect,
    IN OPTIONAL gcoBRUSH Brush,
    IN gctUINT8 FgRop,
    IN gctUINT8 BgRop,
    IN gctBOOL ColorConvert,
    IN gctUINT8 MonoTransparency,
    IN gceSURF_TRANSPARENCY Transparency,
    IN gctUINT32 FgColor,
    IN gctUINT32 BgColor
    )
{
    gceSTATUS status;

    gcoHARDWARE hardware;
    gco2D engine;

    gctUINT32 srcTransparency;
    gctUINT32 dstTransparency;
    gctUINT32 patTransparency;

    gctBOOL useBrush;
    gctBOOL useSource;

    gctUINT32 destWidth;
    gctUINT32 destHeight;

    gctUINT32 maxHeight;
    gctUINT32 streamPackHeightMask;
    gcsPOINT sourceOrigin;
    gcsRECT srcSubRect;
    gcsRECT destSubRect;
    gcsRECT streamRect;
    gctUINT32 lines2render;
    gctUINT32 streamWidth;
    gceSURF_MONOPACK streamPack;

    gctPOINTER destMemory = gcvNULL;

    gcmHEADER_ARG("DestSurface=0x%x Source=0x%x SourceSize=0x%x SourceOrigin=0x%x "
              "DestRect=0x%x Brush=0x%x FgRop=%02x BgRop=%02x ColorConvert=%d "
              "MonoTransparency=%u Transparency=%d FgColor=%08x BgColor=%08x",
              DestSurface, Source, SourceSize, SourceOrigin, DestRect, Brush,
              FgRop, BgRop, ColorConvert, MonoTransparency, Transparency,
              FgColor, BgColor);

    do
    {
        gctUINT32 destFormat, destFormatSwizzle, destIsYUV;

        /* Validate the object. */
        gcmBADOBJECT_BREAK(DestSurface, gcvOBJ_SURF);

        /* Make a shortcut to the gcoHARDWARE object. */
        hardware = DestSurface->hal->hardware;
        gcmERR_BREAK(gcoHAL_Get2DEngine(DestSurface->hal, &engine));

        /* Is the destination format supported? */
        if (gcmIS_ERROR(gcoHARDWARE_TranslateDestinationFormat(
                hardware, DestSurface->info.format,
                &destFormat, &destFormatSwizzle, &destIsYUV)))
        {
            /* No, use software renderer. */
            gcmERR_BREAK(gcoHARDWARE_UseSoftware2D(hardware, gcvTRUE));
        }

        /* Translate the specified transparency mode. */
        gcmERR_BREAK(gcoHARDWARE_TranslateSurfTransparency(
            Transparency,
            &srcTransparency,
            &dstTransparency,
            &patTransparency
            ));

        /* Determine the resource usage. */
        gcoHARDWARE_Get2DResourceUsage(
            FgRop, BgRop,
            srcTransparency,
            &useSource, &useBrush, gcvNULL
            );

        /* Source must be used. */
        if (!useSource)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Use surface rect if not specified. */
        if (DestRect == gcvNULL)
        {
            DestRect = &DestSurface->info.rect;
        }

        /* Default to 0 origin. */
        if (SourceOrigin == gcvNULL)
        {
            SourceOrigin = &sourceOrigin;
            SourceOrigin->x = 0;
            SourceOrigin->y = 0;
        }

        /* Lock the destination. */
        gcmERR_BREAK(gcoSURF_Lock(
            DestSurface,
            gcvNULL,
            &destMemory
            ));

        /* Program the destination. */
        gcmERR_BREAK(gcoHARDWARE_SetTarget(
            hardware,
            &DestSurface->info
            ));

        /* Setup the brush if needed. */
        if (useBrush)
        {
            /* Program the destination format. */
            gcmERR_BREAK(gcoHARDWARE_SetTargetFormat(
                hardware,
                DestSurface->info.format
                ));

            /* Flush the brush. */
            gcmERR_BREAK(gcoHARDWARE_FlushBrush(
                hardware,
                Brush
                ));
        }

        /* Compute the destination size. */
        destWidth  = DestRect->right  - DestRect->left;
        destHeight = DestRect->bottom - DestRect->top;

        /* Determine the number of lines to render. */
        lines2render = destHeight;

        /* Determine left source coordinate. */
        srcSubRect.left = SourceOrigin->x & 7;

        /* Assume 8-pixel packed stream. */
        streamWidth = gcmALIGN(srcSubRect.left + destWidth, 8);

        /* Do we fit? */
        if (streamWidth == 8)
        {
            streamPack = gcvSURF_PACKED8;
            streamPackHeightMask = ~3U;
        }

        /* Nope, don't fit. */
        else
        {
            /* Assume 16-pixel packed stream. */
            streamWidth = gcmALIGN(srcSubRect.left + destWidth, 16);

            /* Do we fit now? */
            if (streamWidth == 16)
            {
                streamPack = gcvSURF_PACKED16;
                streamPackHeightMask = ~1U;
            }

            /* Still don't. */
            else
            {
                /* Assume unpacked stream. */
                streamWidth = gcmALIGN(srcSubRect.left + destWidth, 32);
                streamPack = gcvSURF_UNPACKED;
                streamPackHeightMask = ~0U;
            }
        }

        /* Set the rectangle value. */
        srcSubRect.top = srcSubRect.right = srcSubRect.bottom = 0;

        /* Set source rectangle size. */
        gcmERR_BREAK(gco2D_SetSource(
            engine,
            &srcSubRect
            ));

        /* Program the source. */
        gcmERR_BREAK(gco2D_SetMonochromeSource(
            engine,
            ColorConvert,
            MonoTransparency,
            streamPack,
            gcvFALSE,
            Transparency,
            FgColor,
            BgColor
            ));

        /* Determine the maxumum stream height. */
        maxHeight  = (gco2D_GetMaximumDataCount() << 5) / streamWidth;
        maxHeight &= streamPackHeightMask;

        /* Set the stream rectangle. */
        streamRect.left   = SourceOrigin->x - srcSubRect.left;
        streamRect.top    = SourceOrigin->y;
        streamRect.right  = streamRect.left + streamWidth;
        streamRect.bottom = streamRect.top;

        /* Init destination subrectangle. */
        destSubRect.left   = DestRect->left;
        destSubRect.top    = DestRect->top;
        destSubRect.right  = DestRect->right;
        destSubRect.bottom = destSubRect.top;

        do
        {
            /* Determine the area to render in this pass. */
            gctUINT32 currLines = (lines2render > maxHeight)
                ? maxHeight
                : lines2render;

            streamRect.top     = streamRect.bottom;
            streamRect.bottom += currLines;

            destSubRect.top     = destSubRect.bottom;
            destSubRect.bottom += currLines;

            /* Do the blit. */
            gcmERR_BREAK(gco2D_MonoBlit(
                engine,
                Source,
                SourceSize,
                &streamRect,
                SourcePack,
                streamPack,
                &destSubRect,
                FgRop, BgRop,
                DestSurface->info.format
                ));

            /* Update the line counter. */
            lines2render -= currLines;
        }
        while (lines2render);
    }
    while (gcvFALSE);

    /* Unlock the destination. */
    if (destMemory != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(DestSurface, destMemory));
    }

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_FilterBlit
**
**  Filter blit.
**
**  INPUT:
**
**      gcoSURF SrcSurface
**          Pointer to the source surface.
**
**      gcoSURF DestSurface
**          Pointer to the destination surface.
**
**      gcsRECT_PTR SrcRect
**          Coordinates of the entire source image.
**
**      gcsRECT_PTR DestRect
**          Coordinates of the entire destination image.
**
**      gcsRECT_PTR DestSubRect
**          Coordinates of a sub area within the destination to render.
**          If DestSubRect is gcvNULL, the complete image will be rendered
**          using coordinates set by DestRect.
**          If DestSubRect is not gcvNULL and DestSubRect and DestRect are
**          no equal, DestSubRect is assumed to be within DestRect and
**          will be used to reneder the sub area only.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_FilterBlit(
    IN gcoSURF SrcSurface,
    IN gcoSURF DestSurface,
    IN gcsRECT_PTR SrcRect,
    IN gcsRECT_PTR DestRect,
    IN gcsRECT_PTR DestSubRect
    )
{
    gceSTATUS status;
    gcsRECT destSubRect;

    gctPOINTER srcMemory[3];
    gctPOINTER destMemory[3];

    gcmHEADER_ARG("SrcSurface=0x%x DestSurface=0x%x SrcRect=0x%x DestRect=0x%x "
              "DestSubRect=0x%x",
              SrcSurface, DestSurface, SrcRect, DestRect, DestSubRect);

    do
    {
        /* Reset surface pointers. */
        srcMemory[0]  = gcvNULL;
        destMemory[0] = gcvNULL;

        /* Verify the surfaces. */
        gcmBADOBJECT_BREAK(SrcSurface, gcvOBJ_SURF);
        gcmBADOBJECT_BREAK(DestSurface, gcvOBJ_SURF);

        /* Use surface rect if not specified. */
        if (SrcRect == gcvNULL)
        {
            SrcRect = &SrcSurface->info.rect;
        }

        /* Use surface rect if not specified. */
        if (DestRect == gcvNULL)
        {
            DestRect = &DestSurface->info.rect;
        }

        /* Make sure the destination sub rectangle is set. */
        if (DestSubRect == gcvNULL)
        {
            destSubRect.left   = 0;
            destSubRect.top    = 0;
            destSubRect.right  = DestRect->right  - DestRect->left;
            destSubRect.bottom = DestRect->bottom - DestRect->top;

            DestSubRect = &destSubRect;
        }

        /* Lock the destination. */
        gcmERR_BREAK(gcoSURF_Lock(
            DestSurface,
            gcvNULL,
            destMemory
            ));

        /* Lock the source. */
        gcmERR_BREAK(gcoSURF_Lock(
            SrcSurface,
            gcvNULL,
            srcMemory
            ));

        /* Call gco2D object to complete the blit. */
        gcmERR_BREAK(gcoHARDWARE_FilterBlit(
            DestSurface->hal->hardware,
            &SrcSurface->info,
            &DestSurface->info,
            SrcRect,
            DestRect,
            DestSubRect
            ));
    }
    while (gcvFALSE);

    /* Unlock the source. */
    if (srcMemory != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(SrcSurface, srcMemory));
    }

    /* Unlock the destination. */
    if (destMemory != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(DestSurface, destMemory));
    }

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_EnableAlphaBlend
**
**  Enable alpha blending engine in the hardware and disengage the ROP engine.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**      gctUINT8 SrcGlobalAlphaValue
**          Global alpha value for the color components.
**
**      gctUINT8 DstGlobalAlphaValue
**          Global alpha value for the color components.
**
**      gceSURF_PIXEL_ALPHA_MODE SrcAlphaMode
**          Per-pixel alpha component mode.
**
**      gceSURF_PIXEL_ALPHA_MODE DstAlphaMode
**          Per-pixel alpha component mode.
**
**      gceSURF_GLOBAL_ALPHA_MODE SrcGlobalAlphaMode
**          Global/per-pixel alpha values selection.
**
**      gceSURF_GLOBAL_ALPHA_MODE DstGlobalAlphaMode
**          Global/per-pixel alpha values selection.
**
**      gceSURF_BLEND_FACTOR_MODE SrcFactorMode
**          Final blending factor mode.
**
**      gceSURF_BLEND_FACTOR_MODE DstFactorMode
**          Final blending factor mode.
**
**      gceSURF_PIXEL_COLOR_MODE SrcColorMode
**          Per-pixel color component mode.
**
**      gceSURF_PIXEL_COLOR_MODE DstColorMode
**          Per-pixel color component mode.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_EnableAlphaBlend(
    IN gcoSURF Surface,
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

    gcmHEADER_ARG("Surface=0x%x SrcGlobalAlphaValue=%u DstGlobalAlphaValue=%u "
              "SrcAlphaMode=%d DstAlphaMode=%d SrcGlobalAlphaMode=%d "
              "DstGlobalAlphaMode=%d SrcFactorMode=%d DstFactorMode=%d "
              "SrcColorMode=%d DstColorMode=%d",
              Surface, SrcGlobalAlphaValue, DstGlobalAlphaValue, SrcAlphaMode,
              DstAlphaMode, SrcGlobalAlphaMode, DstGlobalAlphaMode,
              SrcFactorMode, DstFactorMode, SrcColorMode, DstColorMode);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    do
    {
        /* Set the source global color. */
        gcmERR_BREAK(gcoHARDWARE_SetSourceGlobalColor(
            Surface->hal->hardware,
            (gctUINT32)SrcGlobalAlphaValue << 24
            ));

        /* Set the target global color. */
        gcmERR_BREAK(gcoHARDWARE_SetTargetGlobalColor(
            Surface->hal->hardware,
            (gctUINT32)DstGlobalAlphaValue << 24
            ));

        /* Enable blending. */
        gcmERR_BREAK(gcoHARDWARE_EnableAlphaBlend(
            Surface->hal->hardware,
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
**  gcoSURF_DisableAlphaBlend
**
**  Disable alpha blending engine in the hardware and engage the ROP engine.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_DisableAlphaBlend(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    status = gcoHARDWARE_DisableAlphaBlend(Surface->hal->hardware);
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_CopyPixels
**
**  Copy a rectangular area from one surface to another with format conversion.
**
**  INPUT:
**
**      gcoSURF Source
**          Pointer to the source surface.
**
**      gcoSURF Target
**          Pointer to the target surface.
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
gcoSURF_CopyPixels(
    IN gcoSURF Source,
    IN gcoSURF Target,
    IN gctINT SourceX,
    IN gctINT SourceY,
    IN gctINT TargetX,
    IN gctINT TargetY,
    IN gctINT Width,
    IN gctINT Height
    )
{
    gceSTATUS status, last;
    gctPOINTER srcMemory = gcvNULL;
    gctPOINTER trgMemory = gcvNULL;

    gcmHEADER_ARG("Source=0x%x Target=0x%x SourceX=%d SourceY=%d TargetX=%d TargetY=%d "
              "Width=%d Height=%d",
              Source, Target, SourceX, SourceY, TargetX, TargetY, Width,
              Height);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Source, gcvOBJ_SURF);
    gcmVERIFY_OBJECT(Target, gcvOBJ_SURF);

    do
    {
        /* Lock the surfaces. */
        gcmERR_BREAK(
            gcoSURF_Lock(Source, gcvNULL, &srcMemory));
        gcmERR_BREAK(
            gcoSURF_Lock(Target, gcvNULL, &trgMemory));

        if (Source->info.node.pool != gcvPOOL_USER)
        {
            gcmERR_BREAK(gcoOS_CacheInvalidate(Source->hal->os,
                                               srcMemory,
                                               Source->info.size));
        }
        if (Target->info.node.pool != gcvPOOL_USER)
        {
            gcmERR_BREAK(gcoOS_CacheInvalidate(Target->hal->os,
                                               trgMemory,
                                               Target->info.size));
        }

        /* Flush the surfaces. */
        gcmERR_BREAK(gcoSURF_Flush(Source));
        gcmERR_BREAK(gcoSURF_Flush(Target));


        /* Sycnhronize with the GPU. */
        gcmERR_BREAK(
            gcoHAL_Commit(Source->hal, gcvTRUE));

        /* Read the pixel. */
        gcmERR_BREAK(
            gcoHARDWARE_CopyPixels(Source->hal->hardware,
                                   &Source->info,
                                   &Target->info,
                                   SourceX, SourceY,
                                   TargetX, TargetY,
                                   Width, Height));
    }
    while (gcvFALSE);

    /* Unlock the surfaces. */
    if (srcMemory != gcvNULL)
    {
        gcmCHECK_STATUS(
            gcoSURF_Unlock(Source, srcMemory));
    }

    if (trgMemory != gcvNULL)
    {
        gcmCHECK_STATUS(
            gcoSURF_Unlock(Target, trgMemory));
    }

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_ReadPixel
**
**  gcoSURF_ReadPixel reads and returns the current value of the pixel from
**  the specified surface. The pixel value is returned in the specified format.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**      gctPOINTER Memory
**          Pointer to the actual surface bits returned by gcoSURF_Lock.
**
**      gctINT X, Y
**          Coordinates of the pixel.
**
**      gceSURF_FORMAT Format
**          Format of the pixel value to be returned.
**
**  OUTPUT:
**
**      gctPOINTER PixelValue
**          Pointer to the placeholder for the result.
*/
gceSTATUS
gcoSURF_ReadPixel(
    IN gcoSURF Surface,
    IN gctPOINTER Memory,
    IN gctINT X,
    IN gctINT Y,
    IN gceSURF_FORMAT Format,
    OUT gctPOINTER PixelValue
    )
{
    gceSTATUS status, last;
    gctPOINTER srcMemory = gcvNULL;
    gcsSURF_INFO target;

    gcmHEADER_ARG("Surface=0x%x Memory=0x%x X=%d Y=%d Format=%d",
                  Surface, Memory, X, Y, Format);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    do
    {
        /* Flush the surface. */
        gcmERR_BREAK(
            gcoSURF_Flush(Surface));


        /* Sycnhronize with the GPU. */
        gcmERR_BREAK(
            gcoHAL_Commit(Surface->hal, gcvTRUE));

        /* Lock the source surface. */
        gcmERR_BREAK(
            gcoSURF_Lock(Surface, gcvNULL, &srcMemory));

        /* Fill in the target structure. */
        target.type          = gcvSURF_BITMAP;
        target.format        = Format;

        target.rect.left     = 0;
        target.rect.top      = 0;
        target.rect.right    = 1;
        target.rect.bottom   = 1;

        target.alignedWidth  = 1;
        target.alignedHeight = 1;

        target.rotation      = gcvSURF_0_DEGREE;

        target.stride        = 0;
        target.size          = 0;

        target.node.valid    = gcvTRUE;
        target.node.logical  = (gctUINT8_PTR) PixelValue;

        target.samples.x     = 1;
        target.samples.y     = 1;

        /* Read the pixel. */
        gcmERR_BREAK(
            gcoHARDWARE_CopyPixels(Surface->hal->hardware,
                                   &Surface->info,
                                   &target,
                                   X, Y,
                                   0, 0,
                                   1, 1));
    }
    while (gcvFALSE);

    /* Unlock the source surface. */
    if (srcMemory != gcvNULL)
    {
        gcmCHECK_STATUS(
            gcoSURF_Unlock(Surface, srcMemory));
    }

    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_WritePixel
**
**  gcoSURF_WritePixel writes a color value to a pixel of the specified surface.
**  The pixel value is specified in the specified format.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**      gctPOINTER Memory
**          Pointer to the actual surface bits returned by gcoSURF_Lock.
**
**      gctINT X, Y
**          Coordinates of the pixel.
**
**      gceSURF_FORMAT Format
**          Format of the pixel value to be returned.
**
**      gctPOINTER PixelValue
**          Pointer to the pixel value.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_WritePixel(
    IN gcoSURF Surface,
    IN gctPOINTER Memory,
    IN gctINT X,
    IN gctINT Y,
    IN gceSURF_FORMAT Format,
    IN gctPOINTER PixelValue
    )
{
    gceSTATUS status, last;
    gctPOINTER trgMemory = gcvNULL;
    gcsSURF_INFO source;

    gcmHEADER_ARG("Surface=0x%x Memory=0x%x X=%d Y=%d Format=%d PixelValue=0x%x",
              Surface, Memory, X, Y, Format, PixelValue);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    do
    {
        /* Flush the surface. */
        gcmERR_BREAK(
            gcoSURF_Flush(Surface));


        /* Sycnhronize with the GPU. */
        gcmERR_BREAK(
            gcoHAL_Commit(Surface->hal, gcvTRUE));

        /* Lock the source surface. */
        gcmERR_BREAK(
            gcoSURF_Lock(Surface, gcvNULL, &trgMemory));

        /* Fill in the source structure. */
        source.type          = gcvSURF_BITMAP;
        source.format        = Format;

        source.rect.left     = 0;
        source.rect.top      = 0;
        source.rect.right    = 1;
        source.rect.bottom   = 1;

        source.alignedWidth  = 1;
        source.alignedHeight = 1;

        source.rotation      = gcvSURF_0_DEGREE;

        source.stride        = 0;
        source.size          = 0;

        source.node.valid    = gcvTRUE;
        source.node.logical  = (gctUINT8_PTR) PixelValue;

        source.samples.x     = 1;
        source.samples.y     = 1;

        /* Read the pixel. */
        gcmERR_BREAK(
            gcoHARDWARE_CopyPixels(Surface->hal->hardware,
                                   &source,
                                   &Surface->info,
                                   0, 0,
                                   X, Y,
                                   1, 1));
    }
    while (gcvFALSE);

    /* Unlock the source surface. */
    if (trgMemory != gcvNULL)
    {
        gcmCHECK_STATUS(gcoSURF_Unlock(Surface, trgMemory));
    }

    /* Return status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoSURF_NODE_Cache(
    IN gcoSURF Surface,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes,
    IN gceCACHEOPERATION Operation
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsSURF_NODE_PTR Node = &Surface->info.node;

    gcmHEADER_ARG("Node=0x%x, Operation=%d, Bytes=%u", Node, Operation, Bytes);

    if (Node->pool == gcvPOOL_USER)
    {
        gcmFOOTER();
        return gcvSTATUS_OK;
    }
    gcoOS Os = Surface->hal->os;

    switch (Operation)
    {
        case gcvCACHE_CLEAN:
            gcmONERROR(gcoOS_CacheClean(Os, Logical, Bytes));
            break;

        case gcvCACHE_INVALIDATE:
            gcmONERROR(gcoOS_CacheInvalidate(Os, Logical, Bytes));
            break;

        case gcvCACHE_FLUSH:
            gcmONERROR(gcoOS_CacheFlush(Os, Logical, Bytes));
            break;

        default:
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            break;
    }

    gcmFOOTER();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_CPUCacheOperation
**
**  Perform the specified CPU cache operation on the surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**      gceSURF_CPU_CACHE_OP_TYPE Operation
**          Cache operation to be performed.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_CPUCacheOperation(
    IN gcoSURF Surface,
    IN gceCACHEOPERATION Operation
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER source[3] = {0};
    gctBOOL locked = gcvFALSE;

    gcmHEADER_ARG("Surface=0x%x, Operation=%d", Surface, Operation);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Lock the surfaces. */
    gcmONERROR(gcoSURF_Lock(Surface, gcvNULL, source));
    locked = gcvTRUE;

    gcmONERROR(gcoSURF_NODE_Cache(Surface,
                                  source[0],
                                  Surface->info.node.size,
                                  Operation));

    /* Unlock the surfaces. */
    gcmONERROR(gcoSURF_Unlock(Surface, source[0]));
    locked = gcvFALSE;

    gcmFOOTER();
    return gcvSTATUS_OK;

OnError:
    if (locked)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(Surface, source[0]));
    }

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_Flush
**
**  Flush the caches to make sure the surface has all pixels.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_Flush(
    IN gcoSURF Surface
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Flush the current pipe. */
    status = gcoHARDWARE_FlushPipe(Surface->hal->hardware);
    gcmFOOTER();
    return status;
}


/*******************************************************************************
**
**  gcoSURF_SetOrientation
**
**  Set the orientation of a surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**      gceORIENTATION Orientation
**          The requested surface orientation.  Orientation can be one of the
**          following values:
**
**              gcvORIENTATION_TOP_BOTTOM - Surface is from top to bottom.
**              gcvORIENTATION_BOTTOM_TOP - Surface is from bottom to top.
**
**  OUTPUT:
**
*/
gceSTATUS
gcoSURF_SetOrientation(
    IN gcoSURF Surface,
    IN gceORIENTATION Orientation
    )
{
    gcmHEADER_ARG("Surface=0x%x Orientation=%d", Surface, Orientation);

    /* Verif the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Set the orientation. */
    Surface->info.orientation = Orientation;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoSURF_QueryOrientation
**
**  Query the orientation of a surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**  OUTPUT:
**
**      gceORIENTATION * Orientation
**          Pointer to a variable receiving the surface orientation.
**
*/
gceSTATUS
gcoSURF_QueryOrientation(
    IN gcoSURF Surface,
    OUT gceORIENTATION * Orientation
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);
    gcmVERIFY_ARGUMENT(Orientation != gcvNULL);

    /* Return the orientation. */
    *Orientation = Surface->info.orientation;

    /* Success. */
    gcmFOOTER_ARG("*Orientation=%d", *Orientation);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoSURF_SetColorType
**
**  Set the color type of the surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**      gceSURF_COLOR_TYPE colorType
**          color type of the surface.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_SetColorType(
    IN gcoSURF Surface,
    IN gceSURF_COLOR_TYPE ColorType
    )
{
    gcmHEADER_ARG("Surface=0x%x ColorType=%d", Surface, ColorType);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Set the color type. */
    Surface->colorType = ColorType;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoSURF_GetColorType
**
**  Get the color type of the surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**  OUTPUT:
**
**      gceSURF_COLOR_TYPE *colorType
**          pointer to the variable receiving color type of the surface.
**
*/
gceSTATUS
gcoSURF_GetColorType(
    IN gcoSURF Surface,
    OUT gceSURF_COLOR_TYPE *ColorType
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);
    gcmVERIFY_ARGUMENT(ColorType != gcvNULL);

    /* Return the color type. */
    *ColorType = Surface->colorType;

    /* Success. */
    gcmFOOTER_ARG("*ColorType=%d", *ColorType);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoSURF_SetRotation
**
**  Set the surface ration angle.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to the surface.
**
**      gceSURF_ROTATION Rotation
**          Rotation angle.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_SetRotation(
    IN gcoSURF Surface,
    IN gceSURF_ROTATION Rotation
    )
{
    gcmHEADER_ARG("Surface=0x%x Rotation=%d", Surface, Rotation);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Support only 2D surfaces. */
    if (Surface->info.type != gcvSURF_BITMAP)
    {
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Set new rotation. */
    Surface->info.rotation = Rotation;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**  gcoSURF_ConstructWrapper
**
**  Create a new gcoSURF wrapper object.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to an gcoHAL object.
**
**  OUTPUT:
**
**      gcoSURF * Surface
**          Pointer to the variable that will hold the gcoSURF object pointer.
*/
gceSTATUS
gcoSURF_ConstructWrapper(
    IN gcoHAL Hal,
    OUT gcoSURF * Surface
    )
{
    gcoSURF surface;
    gceSTATUS status;

    gcmHEADER_ARG("Hal=0x%x", Hal);

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(Hal != gcvNULL);
    gcmVERIFY_ARGUMENT(Surface != gcvNULL);

    do
    {
        /* Allocate the gcoSURF object. */
        gcmERR_BREAK(gcoOS_Allocate(
            Hal->os,
            gcmSIZEOF(struct _gcoSURF),
            (gctPOINTER *) &surface
            ));

        /* Reset the object. */
        gcmVERIFY_OK(gcoOS_ZeroMemory(
            surface, gcmSIZEOF(struct _gcoSURF)
            ));

        /* Initialize the gcoSURF object.*/
        surface->object.type = gcvOBJ_SURF;
        surface->hal         = Hal;

        /* 1 sample per pixel. */
        surface->info.samples.x = 1;
        surface->info.samples.y = 1;

        /* One plane. */
        surface->depth = 1;

        /* Initialize the node. */
        surface->info.node.pool      = gcvPOOL_USER;
        surface->info.node.physical  = ~0U;
        surface->info.node.physical2 = ~0U;
        surface->info.node.physical3 = ~0U;
        surface->info.node.count     = 1;
        surface->referenceCount = 1;

        /* Return pointer to the gcoSURF object. */
        *Surface = surface;

        /* Success. */
        gcmFOOTER_ARG("*Surface=0x%x", *Surface);
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoSURF_SetBuffer
**
**  Set the underlying buffer for the surface wrapper.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to gcoSURF object.
**
**      gceSURF_TYPE Type
**          Type of surface to create.
**
**      gceSURF_FORMAT Format
**          Format of surface to create.
**
**      gctINT Stride
**          Surface stride. Is set to ~0 the stride will be autocomputed.
**
**      gctPOINTER Logical
**          Logical pointer to the user allocated surface or gcvNULL if no
**          logical pointer has been provided.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_SetBuffer(
    IN gcoSURF Surface,
    IN gceSURF_TYPE Type,
    IN gceSURF_FORMAT Format,
    IN gctUINT Stride,
    IN gctPOINTER Logical,
    IN gctUINT32 Physical
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x Type=%d Format=%d Stride=%u Logical=0x%x "
                  "Physical=%08x",
                  Surface, Type, Format, Stride, Logical, Physical);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Has to be user-allocated surface. */
    if (Surface->info.node.pool != gcvPOOL_USER)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Unmap the current buffer if any. */
    gcmONERROR(_UnmapUserBuffer(Surface, gcvTRUE));

    /* Determine the stride parameters. */
    Surface->autoStride = (Stride == ~0U);

    /* Set surface parameters. */
    Surface->info.type   = Type;
    Surface->info.format = Format;
    Surface->info.stride = Stride;

    /* Set node pointers. */
    Surface->logical  = (gctUINT8_PTR) Logical;
    Surface->physical = Physical;

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
**  gcoSURF_SetVideoBuffer
**
**  Set the underlying video buffer for the surface wrapper.
**  The video plane addresses should be specified invidually.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to gcoSURF object.
**
**      gceSURF_TYPE Type
**          Type of surface to create.
**
**      gceSURF_FORMAT Format
**          Format of surface to create.
**
**      gctINT Stride
**          Surface stride. Is set to ~0 the stride will be autocomputed.
**
**      gctPOINTER LogicalPlane1
**          Logical pointer to the first plane of the user allocated surface
**          or gcvNULL if no logical pointer has been provided.
**
**      gctUINT32 PhysicalPlane1
**          Physical pointer to the user allocated surface or ~0 if no
**          physical pointer has been provided.
**
**  OUTPUT:
**
**      Nothing.
*/
#if 0
gceSTATUS
gcoSURF_SetVideoBuffer(
    IN gcoSURF Surface,
    IN gceSURF_TYPE Type,
    IN gceSURF_FORMAT Format,
    IN gctUINT StridePlane1,
    IN gctPOINTER LogicalPlane1,
    IN gctUINT32 PhysicalPlane1,
    IN gctUINT StridePlane2,
    IN gctPOINTER LogicalPlane2,
    IN gctUINT32 PhysicalPlane2,
    IN gctUINT StridePlane3,
    IN gctPOINTER LogicalPlane3,
    IN gctUINT32 PhysicalPlane3
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x Type=%d Format=%d StridePlane1=%u LogicalPlane1=0x%x "
              "PhysicalPlane1=%08x StridePlane2=%u LogicalPlane2=0x%x "
              "PhysicalPlane2=%08x StridePlane3=%u LogicalPlane3=0x%x "
              "PhysicalPlane3=%08x",
              Surface, Type, Format, StridePlane1, LogicalPlane1,
              PhysicalPlane1, StridePlane2, LogicalPlane2, PhysicalPlane2,
              StridePlane3, LogicalPlane3, PhysicalPlane3);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    do
    {
        /* Has to be user-allocated surface. */
        if (Surface->info.node.pool != gcvPOOL_USER)
        {
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Unmap the current buffer if any. */
        gcmERR_BREAK(_UnmapUserBuffer(
            Surface, gcvTRUE
            ));

        /* Determine the stride parameters. */
        Surface->autoStride = (Stride == ~0U);

        /* Set surface parameters. */
        Surface->info.type   = Type;
        Surface->info.format = Format;
        Surface->info.stride = Stride;

        /* Set node pointers. */
        Surface->logical  = (gctUINT8_PTR) Logical;
        Surface->physical = Physical;
    }
    while (gcvFALSE);

    /* Return the status. */
    gcmFOOTER();
    return status;
}
#endif

/*******************************************************************************
**
**  gcoSURF_SetWindow
**
**  Set the size of the surface in pixels and map the underlying buffer set by
**  gcoSURF_SetBuffer if necessary.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to gcoSURF object.
**
**      gctINT X, Y
**          The origin of the surface.
**
**      gctINT Width, Height
**          Size of the surface in pixels.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_SetWindow(
    IN gcoSURF Surface,
    IN gctUINT X,
    IN gctUINT Y,
    IN gctUINT Width,
    IN gctUINT Height
    )
{
    gceSTATUS status;
    gctUINT32 offset;
    gctUINT32 bitsPerPixel;

    gcmHEADER_ARG("Surface=0x%x X=%u Y=%u Width=%u Height=%u",
                  Surface, X, Y, Width, Height);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    /* Unmap the current buffer if any. */
    gcmONERROR(
        _UnmapUserBuffer(Surface, gcvTRUE));

    /* Make sure at least one of the surface pointers is set. */
    if ((Surface->logical == gcvNULL) && (Surface->physical == ~0))
    {
        gcmONERROR(gcvSTATUS_INVALID_ADDRESS);
    }

    /* Set initial aligned width and height. */
    Surface->info.alignedWidth  = Width;
    Surface->info.alignedHeight = Height;

    /* Compute bits per pixel. */
    gcmONERROR(
        gcoHARDWARE_ConvertFormat(Surface->hal->hardware,
                                  Surface->info.format,
                                  &bitsPerPixel,
                                  gcvNULL));

    /* Stride is the same as the width? */
    if (Surface->autoStride)
    {
        /* Compute the stride. */
        Surface->info.stride = Width * bitsPerPixel / 8;
    }
    else
    {
        /* Align the surface size. */
        gcmONERROR(
            gcoHARDWARE_AlignToTile(Surface->hal->hardware,
                                    Surface->info.type,
                                    &Surface->info.alignedWidth,
                                    &Surface->info.alignedHeight,
                                    &Surface->info.superTiled));
    }

    /* Set the rectangle. */
    Surface->info.rect.left   = X;
    Surface->info.rect.top    = Y;
    Surface->info.rect.right  = X + Width;
    Surface->info.rect.bottom = Y + Height;

    offset = X * bitsPerPixel / 8 + Y * Surface->info.stride;

    /* Compute the surface size. */
    Surface->info.size
        = Surface->info.stride
        * Surface->info.rect.bottom;

    /* Need to map physical pointer? */
    if (Surface->physical == ~0)
    {
        /* Map the physical pointer. */
        gcmONERROR(
            gcoOS_MapUserMemory(Surface->hal->os,
                                (gctUINT8_PTR) Surface->logical + offset,
                                Surface->info.size,
                                &Surface->info.node.u.wrapped.mappingInfo,
                                &Surface->info.node.physical));
    }
    else
    {
        Surface->info.node.physical = Surface->physical + offset;
    }

    /* Need to map logical pointer? */
    if (Surface->logical == gcvNULL)
    {
        /* Map the logical pointer. */
        gcmONERROR(
            gcoHAL_MapMemory(Surface->hal,
                             gcmINT2PTR(Surface->physical + offset),
                             Surface->info.size,
                             (gctPOINTER *) &Surface->info.node.logical));

        Surface->info.node.u.wrapped.logicalMapped = gcvTRUE;
    }
    else
    {
        Surface->info.node.logical = (gctUINT8_PTR) Surface->logical + offset;
    }

    /* Validate the node. */
    Surface->info.node.lockCount = 1;
    Surface->info.node.valid     = gcvTRUE;

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
**  gcoSURF_ReferenceSurface
**
**  Increase reference count of a surface.
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to gcoSURF object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoSURF_ReferenceSurface(
    IN gcoSURF Surface
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    Surface->referenceCount++;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**  gcoSURF_QueryReferenceCount
**
**  Query reference count of a surface
**
**  INPUT:
**
**      gcoSURF Surface
**          Pointer to gcoSURF object.
**
**  OUTPUT:
**
**      gctINT32 ReferenceCount
**          Reference count of a surface
*/
gceSTATUS
gcoSURF_QueryReferenceCount(
    IN gcoSURF Surface,
    OUT gctINT32 * ReferenceCount
    )
{
    gcmHEADER_ARG("Surface=0x%x", Surface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

    *ReferenceCount = Surface->referenceCount;

    gcmFOOTER_ARG("*ReferenceCount=%d", *ReferenceCount);
    return gcvSTATUS_OK;
}

