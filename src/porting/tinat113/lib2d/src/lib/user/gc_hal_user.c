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
**	gcoHAL object for user HAL layers.
**
*/

#include "gc_hal_user_precomp.h"

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE	gcvZONE_HAL

/******************************************************************************\
******************************** gcoHAL API Code ********************************
\******************************************************************************/

/*******************************************************************************
**
**	_WorkaroundForFilterBlit
**
**	Workaround for the dirty region issue of filter blit.
**  It only exists for old GC300 before 2.0.2 (included).
**
**	INPUT:
**
**		gctHAL Hal
**			Pointer to HAL.
**
**	OUTPUT:
**
**		Nothing.
*/
static gceSTATUS
_WorkaroundForFilterBlit(
	IN gcoHAL Hal
	)
{
	gceSTATUS status;

	gcoSURF    srcSurf = gcvNULL;
	gcsRECT    srcRect;

	gcoSURF    dstSurf = gcvNULL;
	gcsRECT    dstRect;

	do {
		gcmERR_BREAK(gcoSURF_Construct(
			Hal,
			256,
			256,
			1,
			gcvSURF_BITMAP,
			gcvSURF_A8R8G8B8,
			gcvPOOL_DEFAULT,
			&srcSurf
			));

		gcmERR_BREAK(gcoSURF_Construct(
			Hal,
			256,
			256,
			1,
			gcvSURF_BITMAP,
			gcvSURF_A8R8G8B8,
			gcvPOOL_DEFAULT,
			&dstSurf
			));

		srcRect.left   = 0;
		srcRect.top    = 0;
		srcRect.right  = 64;
		srcRect.bottom = 16;

		dstRect.left   = 0;
		dstRect.top    = 0;
		dstRect.right  = 128;
		dstRect.bottom = 32;

		gcmERR_BREAK(gcoSURF_FilterBlit(
			srcSurf,
			dstSurf,
			&srcRect,
			&dstRect,
			gcvNULL
			));

		gcmERR_BREAK(gcoSURF_Destroy(srcSurf));
		srcSurf = gcvNULL;

		gcmERR_BREAK(gcoSURF_Destroy(dstSurf));
		dstSurf = gcvNULL;
	} while(gcvFALSE);

	if (gcmIS_ERROR(status)) {
		gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_HAL,
			"Failed to workarond for GC300.");

		if (srcSurf)
		{
			gcmVERIFY_OK(gcoSURF_Destroy(srcSurf));
		}

		if (dstSurf)
		{
			gcmVERIFY_OK(gcoSURF_Destroy(dstSurf));
		}
	}

	return status;
}

/*******************************************************************************
**
**	gcoHAL_Construct
**
**	Construct a new gcoHAL object.
**
**	INPUT:
**
**		gctPOINTER Context
**			Pointer to a context that can be used by the platform specific
**			functions.
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**	OUTPUT:
**
**		gcoHAL * Hal
**			Pointer to a variable that will hold the gcoHAL object pointer.
*/
gceSTATUS
gcoHAL_Construct(
	IN gctPOINTER Context,
	IN gcoOS Os,
	OUT gcoHAL * Hal
	)
{
	gcoHAL hal = gcvNULL;
	gceSTATUS status;
	gceCHIPMODEL chipModel;
	gctUINT32 chipRevision;

	gcmHEADER_ARG("Context=0x%x Os=0x%x", Context, Os);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
	gcmVERIFY_ARGUMENT(Hal != gcvNULL);

	/* Create the gcoHAL object. */
	gcmONERROR(
		gcoOS_Allocate(Os, gcmSIZEOF(struct _gcoHAL), (gctPOINTER *) &hal));

	/* Initialize the object. */
	hal->object.type = gcvOBJ_HAL;
	hal->context     = Context;
	hal->os          = Os;
    hal->process     = gcoOS_GetCurrentProcessID();

	/* Zero the gco2D, gco3D, and gcoDUMP objects. */
	hal->dump     = gcvNULL;
	hal->engine2D = gcvNULL;
	hal->engineVG = gcvNULL;
	hal->hardware = gcvNULL;

	/* Construct the gcoHARDWARE object. */
	gcmONERROR(
		gcoHARDWARE_Construct(hal, &hal->hardware));

	/* Workaround for the old GC300. */
	gcmONERROR(
		gcoHARDWARE_QueryChipIdentity(hal->hardware,
									  &chipModel,
									  &chipRevision,
									  gcvNULL,
									  gcvNULL,
									  gcvNULL));

	if ((chipModel == gcv300) && (chipRevision <= 0x00002202))
	{
		_WorkaroundForFilterBlit(hal);
	}

	/* Return pointer to the gcoHAL object. */
	*Hal = hal;

	/* Success. */
	gcmFOOTER_ARG("*Hal=0x%x", *Hal);
	return gcvSTATUS_OK;

OnError:
	/* Roll back. */
	if (hal != gcvNULL)
	{
		if (hal->hardware != gcvNULL)
		{
			gcmVERIFY_OK(gcoHARDWARE_Destroy(hal->hardware));
		}

		gcmVERIFY_OK(gcoOS_Free(Os, hal));
	}

	/* Return the status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHAL_Destroy
**
**	Destroy an gcoHAL object.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object that needs to be destroyed.
**
**	OUTPUT:
**
**		Nothing.
**
*******************************************************************************/
gceSTATUS
gcoHAL_Destroy(
	IN gcoHAL Hal
	)
{
	gcmHEADER_ARG("Hal=0x%x", Hal);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);
	gcmVERIFY_OBJECT(Hal->os, gcvOBJ_OS);
	gcmVERIFY_OBJECT(Hal->hardware, gcvOBJ_HARDWARE);

	if (Hal->engine2D != gcvNULL)
	{
		/* Destroy the gco2D object. */
		gcmVERIFY_OK(gco2D_Destroy(Hal->engine2D));
	}

#if gcdUSE_VG
	if (Hal->engineVG != gcvNULL)
	{
		/* Destroy the gcoVG object. */
		gcmVERIFY_OK(gcoVG_Destroy(Hal->engineVG));
	}
#endif


	/* Destroy the gcoHARDWARE object. */
	gcmVERIFY_OK(gcoHARDWARE_Destroy(Hal->hardware));

	/* Destroy the gcoDUMP object if any. */
	if (Hal->dump != gcvNULL)
	{
		gcmVERIFY_OK(gcoDUMP_Destroy(Hal->dump));
	}

	/* Free the gcoHAL object. */
	gcmVERIFY_OK(gcoOS_Free(Hal->os, Hal));

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoHAL_IsFeatureAvailable
**
**  Verifies whether the specified feature is available in hardware.
**
**  INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
**
**		gceFEATURE Feature
**			Feature to be verified.
*/
gceSTATUS
gcoHAL_IsFeatureAvailable(
    IN gcoHAL Hal,
	IN gceFEATURE Feature
    )
{
	gceSTATUS status;

	gcmHEADER_ARG("Hal=0x%x Feature=%d", Hal, Feature);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);

	/* Query support from hardware object. */
	status = gcoHARDWARE_IsFeatureAvailable(Hal->hardware, Feature);
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**  gcoHAL_QueryChipIdentity
**
**  Query the identity of the hardware.
**
**  INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
**
**  OUTPUT:
**
**      gceCHIPMODEL* ChipModel
**          If 'ChipModel' is not gcvNULL, the variable it points to will
**			receive the model of the chip.
**
**      gctUINT32* ChipRevision
**          If 'ChipRevision' is not gcvNULL, the variable it points to will
**			receive the revision of the chip.
**
**      gctUINT32* ChipFeatures
**          If 'ChipFeatures' is not gcvNULL, the variable it points to will
**			receive the feature set of the chip.
**
**      gctUINT32 * ChipMinorFeatures
**          If 'ChipMinorFeatures' is not gcvNULL, the variable it points to
**			will receive the minor feature set of the chip.
**
*/
gceSTATUS gcoHAL_QueryChipIdentity(
    IN gcoHAL Hal,
	OUT gceCHIPMODEL* ChipModel,
	OUT gctUINT32* ChipRevision,
	OUT gctUINT32* ChipFeatures,
	OUT gctUINT32* ChipMinorFeatures
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hal=0x%x", Hal);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);

	/* Query identity from hardware object. */
	status = gcoHARDWARE_QueryChipIdentity(Hal->hardware,
										   ChipModel,
										   ChipRevision,
										   ChipFeatures,
										   ChipMinorFeatures,
										   gcvNULL);

	/* Return status. */
	gcmFOOTER_ARG("status=%d *ChipModel=%d *ChipRevision=%x *ChipFeatures=%08x "
				  "*ChipMinorFeatures=%08x", status, gcmOPT_VALUE(ChipModel),
				  gcmOPT_VALUE(ChipRevision), gcmOPT_VALUE(ChipFeatures),
				  gcmOPT_VALUE(ChipMinorFeatures));
	return status;
}

/*******************************************************************************
**
**	gcoHAL_Call
**
**	Call the kernel HAL layer.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
**
**		gcsHAL_INTERFACE * Interface
**			Pointer to an gcsHAL_INTERFACE structure that defines the command to
**			be executed by the kernel HAL layer.
**
**	OUTPUT:
**
**		gcsHAL_INTERFACE * Interface
**			Pointer to an gcsHAL_INTERFACE structure that will be filled in by
**			the kernel HAL layer.
*/
gceSTATUS
gcoHAL_Call(
	IN gcoHAL Hal,
	IN OUT gcsHAL_INTERFACE * Interface
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hal=0x%x Interface=0x%x", Hal, Interface);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);
	gcmVERIFY_ARGUMENT(Interface != gcvNULL);

	/* Call kernel service. */
	status = gcoOS_DeviceControl(Hal->os, IOCTL_GCHAL_INTERFACE,
								 Interface, gcmSIZEOF(gcsHAL_INTERFACE),
								 Interface, gcmSIZEOF(gcsHAL_INTERFACE));

	if (gcmIS_SUCCESS(status))
	{
		status = Interface->status;
	}

	if (status == gcvSTATUS_OUT_OF_MEMORY)
	{
		/* Commit any command queue to memory. */
		gcmONERROR(gcoHARDWARE_Commit(Hal->hardware));

		/* Stall the hardware. */
		gcmONERROR(gcoHARDWARE_Stall(Hal->hardware));

		/* Retry kernel call again. */
		status = gcoOS_DeviceControl(Hal->os, IOCTL_GCHAL_INTERFACE,
									 Interface, gcmSIZEOF(gcsHAL_INTERFACE),
									 Interface, gcmSIZEOF(gcsHAL_INTERFACE));

		if (gcmIS_SUCCESS(status))
		{
			status = Interface->status;
		}
	}

OnError:
	/* Return the status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHAL_QueryVideoMemory
**
**	Query the amount of video memory.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
**
**	OUTPUT:
**
**		gctPHYS_ADDR * InternalAddress
**			Pointer to a variable that will hold the physical address of the
**			internal memory.  If 'InternalAddress' is gcvNULL, no information about
**			the internal memory will be returned.
**
**		gctSIZE_T * InternalSize
**			Pointer to a variable that will hold the size of the internal
**			memory.  'InternalSize' cannot be gcvNULL if 'InternalAddress' is not
**			gcvNULL.
**
**		gctPHYS_ADDR * ExternalAddress
**			Pointer to a variable that will hold the physical address of the
**			external memory.  If 'ExternalAddress' is gcvNULL, no information about
**			the external memory will be returned.
**
**		gctSIZE_T * ExternalSize
**			Pointer to a variable that will hold the size of the external
**			memory.  'ExternalSize' cannot be gcvNULL if 'ExternalAddress' is not
**			gcvNULL.
**
**		gctPHYS_ADDR * ContiguousAddress
**			Pointer to a variable that will hold the physical address of the
**			contiguous memory.  If 'ContiguousAddress' is gcvNULL, no information
**			about the contiguous memory will be returned.
**
**		gctSIZE_T * ContiguousSize
**			Pointer to a variable that will hold the size of the contiguous
**			memory.  'ContiguousSize' cannot be gcvNULL if 'ContiguousAddress' is
**			not gcvNULL.
*/
gceSTATUS
gcoHAL_QueryVideoMemory(
	IN gcoHAL Hal,
	OUT gctPHYS_ADDR * InternalAddress,
	OUT gctSIZE_T * InternalSize,
	OUT gctPHYS_ADDR * ExternalAddress,
	OUT gctSIZE_T * ExternalSize,
	OUT gctPHYS_ADDR * ContiguousAddress,
	OUT gctSIZE_T * ContiguousSize
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hal=0x%x", Hal);

	status = gcoOS_QueryVideoMemory(Hal->os,
									InternalAddress, InternalSize,
									ExternalAddress, ExternalSize,
									ContiguousAddress, ContiguousSize);
	gcmFOOTER_ARG("status=%d InternalAddress=0x%x InternalSize=%lu "
				  "ExternalAddress=0x%x ExternalSize=%lu ContiguousAddress=0x%x "
				  "ContiguousSize=%lu", status, gcmOPT_POINTER(InternalAddress),
				  gcmOPT_VALUE(InternalSize), gcmOPT_POINTER(ExternalAddress),
				  gcmOPT_VALUE(ExternalSize), gcmOPT_POINTER(ContiguousAddress),
				  gcmOPT_VALUE(ContiguousSize));
	return status;
}

/*******************************************************************************
**
**	gcoHAL_MapMemory
**
**	Map a range of video memory into the process space.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
**
**		gctPHYS_ADDR Physical
**			Physical address of video memory to map.
**
**		gctSIZE_T NumberOfBytes
**			Number of bytes to map.
**
**	OUTPUT:
**
**		gctPOINTER * Logical
**			Pointer to a variable that will hold the logical address of the
**			mapped video memory.
*/
gceSTATUS
gcoHAL_MapMemory(
	IN gcoHAL Hal,
	IN gctPHYS_ADDR Physical,
	IN gctSIZE_T NumberOfBytes,
	OUT gctPOINTER * Logical
	)
{
	gcsHAL_INTERFACE iface;
	gceSTATUS status;

	gcmHEADER_ARG("Hal=0x%x Physical=0x%x NumberOfBytes=%lu", Hal, Physical,
				  NumberOfBytes);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);
	gcmVERIFY_ARGUMENT(Logical != gcvNULL);

	/* Call kernel API to map the memory. */
	iface.command              = gcvHAL_MAP_MEMORY;
	iface.u.MapMemory.physical = Physical;
	iface.u.MapMemory.bytes    = NumberOfBytes;
	gcmONERROR(gcoHAL_Call(Hal, &iface));

	/* Return logical address. */
	*Logical = iface.u.MapMemory.logical;

	/* Success. */
	gcmFOOTER_ARG("*Logical=0x%x", *Logical);
	return gcvSTATUS_OK;

OnError:
	/* Return the status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHAL_UnmapMemory
**
**	Unmap a range of video memory from the process space.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
**
**		gctPHYS_ADDR Physical
**			Physical address of video memory to unmap.
**
**		gctSIZE_T NumberOfBytes
**			Number of bytes to unmap.
**
**		gctPOINTER Logical
**			Logical address of the video memory to unmap.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoHAL_UnmapMemory(
	IN gcoHAL Hal,
	IN gctPHYS_ADDR Physical,
	IN gctSIZE_T NumberOfBytes,
	IN gctPOINTER Logical
	)
{
	gcsHAL_INTERFACE iface;
	gceSTATUS status;

	gcmHEADER_ARG("Hal=0x%x Physical=0x%x NumberOfBytes=%lu Logical=0x%x", Hal,
				  Physical, NumberOfBytes, Logical);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);
	gcmVERIFY_ARGUMENT(Logical != gcvNULL);

	/* Call kernel API to unmap the memory. */
	iface.command                = gcvHAL_UNMAP_MEMORY;
	iface.u.UnmapMemory.physical = Physical;
	iface.u.UnmapMemory.bytes    = NumberOfBytes;
	iface.u.UnmapMemory.logical  = Logical;
	status = gcoHAL_Call(Hal, &iface);

	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHAL_ScheduleUnmapMemory
**
**	Schedule an unmap of a buffer mapped through its physical address.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
**
**		gctPHYS_ADDR Physical
**			Physical address of video memory to unmap.
**
**		gctSIZE_T NumberOfBytes
**			Number of bytes to unmap.
**
**		gctPOINTER Logical
**			Logical address of the video memory to unmap.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoHAL_ScheduleUnmapMemory(
	IN gcoHAL Hal,
	IN gctPHYS_ADDR Physical,
	IN gctSIZE_T NumberOfBytes,
	IN gctPOINTER Logical
	)
{
	gceSTATUS status;
	gcsHAL_INTERFACE iface;

	gcmHEADER_ARG("Hal=0x%x Physical=0x%x NumberOfBytes=%lu Logical=0x%x", Hal,
				  Physical, NumberOfBytes, Logical);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);
	gcmVERIFY_ARGUMENT(NumberOfBytes > 0);
	gcmVERIFY_ARGUMENT(Logical != gcvNULL);

	/* Schedule an event to unmap the user memory. */
	iface.command                = gcvHAL_UNMAP_MEMORY;
	iface.u.UnmapMemory.bytes    = NumberOfBytes;
	iface.u.UnmapMemory.physical = Physical;
	iface.u.UnmapMemory.logical  = Logical;
	status = gcoHAL_ScheduleEvent(Hal, &iface);

	/* Return the status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHAL_ScheduleUnmapUserMemory
**
**	Schedule an unmap of a user buffer using event mechanism.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
**
**		gctPOINTER Info
**			Information record returned by gcoOS_MapUserMemory.
**
**		gctSIZE_T Size
**			Size in bytes of the memory to unlock.
**
**		gctUINT32_PTR Address
**			The address returned by gcoOS_MapUserMemory.
**
**		gctPOINTER Memory
**			Pointer to memory to unlock.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoHAL_ScheduleUnmapUserMemory(
	IN gcoHAL Hal,
	IN gctPOINTER Info,
	IN gctSIZE_T Size,
	IN gctUINT32 Address,
	IN gctPOINTER Memory
	)
{
	gceSTATUS status;
	gcsHAL_INTERFACE iface;

	gcmHEADER_ARG("Hal=0x%x Info=0x%x Size=%lu Address=%08x Memory=0x%x", Hal, Info,
				  Size, Address, Memory);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);
	gcmVERIFY_ARGUMENT(Info != gcvNULL);
	gcmVERIFY_ARGUMENT(Size > 0);
	gcmVERIFY_ARGUMENT(Memory != gcvNULL);

	/* Schedule an event to unmap the user memory. */
	iface.command = gcvHAL_UNMAP_USER_MEMORY;
	iface.u.UnmapUserMemory.info    = Info;
	iface.u.UnmapUserMemory.size    = Size;
	iface.u.UnmapUserMemory.address = Address;
	iface.u.UnmapUserMemory.memory  = Memory;
	status = gcoHAL_ScheduleEvent(Hal, &iface);

	/* Return the status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHAL_Commit
**
**	Commit the current command buffer to hardware and optionally wait until the
**	hardware is finished.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
**
**		gctBOOL Stall
**			gcvTRUE if the thread needs to wait until the hardware has finished
**			executing the committed command buffer.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoHAL_Commit(
	IN gcoHAL Hal,
	IN gctBOOL Stall
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hal=0x%x Stall=%d", Hal, Stall);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);

	/* Commit the command buffer to hardware. */
	gcmONERROR(gcoHARDWARE_Commit(Hal->hardware));

	if (Stall)
	{
		/* Stall the hardware. */
		gcmONERROR(gcoHARDWARE_Stall(Hal->hardware));
	}

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;

OnError:
	/* Return status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHAL_QueryTiled
**
**	Query the tile sizes.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
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
*/
gceSTATUS
gcoHAL_QueryTiled(
	IN gcoHAL Hal,
	OUT gctINT32 * TileWidth2D,
	OUT gctINT32 * TileHeight2D,
	OUT gctINT32 * TileWidth3D,
	OUT gctINT32 * TileHeight3D
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hal=0x%x", Hal);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);

	/* Query the tile sizes through gcoHARDWARE. */
	status = gcoHARDWARE_QueryTileSize(TileWidth2D, TileHeight2D,
									   TileWidth3D, TileHeight3D,
									   gcvNULL);
	gcmFOOTER_ARG("status=%d *TileWidth2D=%d *TileHeight2D=%d *TileWidth3D=%d "
				  "*TileHeight3D=%d", status, gcmOPT_VALUE(TileWidth2D),
				  gcmOPT_VALUE(TileHeight2D), gcmOPT_VALUE(TileWidth3D),
				  gcmOPT_VALUE(TileHeight3D));
	return status;
}

/*******************************************************************************
**
**	gcoHAL_Get2DEngine
**
**	Get the pointer to the gco2D object.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
**
**	OUTPUT:
**
**		gco2D * Engine
**			Pointer to a variable receiving the gco2D object pointer.
*/
gceSTATUS
gcoHAL_Get2DEngine(
	IN gcoHAL Hal,
	OUT gco2D * Engine
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hal=0x%x", Hal);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);
	gcmVERIFY_ARGUMENT(Engine != gcvNULL);

	if (Hal->engine2D == gcvNULL)
	{
		/* Construct the gco2D object. */
		gcmONERROR(gco2D_Construct(Hal, &Hal->engine2D));
	}

	/* Return pointer to the gco2D object. */
	*Engine = Hal->engine2D;

	/* Success. */
	gcmFOOTER_ARG("*Engine=0x%x", *Engine);
	return gcvSTATUS_OK;

OnError:
	/* Return the status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHAL_GetVGEngine
**
**	Get the pointer to the gcoVG object.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
**
**	OUTPUT:
**
**		gcoVG * Engine
**			Pointer to a variable receiving the gcoVG object pointer.
*/
gceSTATUS
gcoHAL_GetVGEngine(
	IN gcoHAL Hal,
	OUT gcoVG * Engine
	)
{
#if gcdUSE_VG
	gceSTATUS status;

	gcmHEADER_ARG("Hal=0x%x", Hal);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);
	gcmVERIFY_ARGUMENT(Engine != gcvNULL);

	if (Hal->engineVG == gcvNULL)
	{
		/* Construct the gcoVG object. */
		gcmONERROR(gcoVG_Construct(Hal, &Hal->engineVG));
	}

	/* Return pointer to the gcoVG object. */
	*Engine = Hal->engineVG;

	/* Success. */
	gcmFOOTER_ARG("*Engine=0x%x", *Engine);
	return gcvSTATUS_OK;

OnError:
	/* Return the status. */
	gcmFOOTER();
	return status;
#else
	return gcvSTATUS_NOT_SUPPORTED;
#endif
}

/*******************************************************************************
**
**	gcoHAL_Get3DEngine
**
**	Get the pointer to the gco3D object.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
**
**	OUTPUT:
**
**		gco3D * Engine
**			Pointer to a variable receiving the gco3D object pointer.
*/
gceSTATUS
gcoHAL_Get3DEngine(
	IN gcoHAL Hal,
	OUT gco3D * Engine
	)
{
	return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
**
**	gcoHAL_GetDump
**
**	Get the pointer to the gcoDUMP object.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
**
**	OUTPUT:
**
**		gcoDUMP * Dump
**			Pointer to a variable receiving the gcoDUMP object pointer.
*/
gceSTATUS
gcoHAL_GetDump(
	IN gcoHAL Hal,
	OUT gcoDUMP * Dump
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hal=0x%x", Hal);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);
	gcmVERIFY_ARGUMENT(Dump != gcvNULL);

	if (Hal->dump == gcvNULL)
	{
		/* Construct the gcoDUMP object. */
		gcmONERROR(gcoDUMP_Construct(Hal->os, Hal, &Hal->dump));
	}

	/* Return pointer to the gcoDUMP object. */
	*Dump = Hal->dump;

	/* Success. */
	gcmFOOTER_ARG("*Dump=0x%x", *Dump);
	return gcvSTATUS_OK;

OnError:
	/* Return the status. */
	gcmFOOTER();
	return status;
}

/* Schedule an event. */
gceSTATUS
gcoHAL_ScheduleEvent(
	IN gcoHAL Hal,
	IN OUT gcsHAL_INTERFACE * Interface
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hal=0x%x Interface=0x%x", Hal, Interface);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);
	gcmVERIFY_ARGUMENT(Interface != gcvNULL);

	/* Send event to hardware layer. */
	status = gcoHARDWARE_CallEvent(Hal->hardware, Interface);
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoHAL_SetPowerManagementState
**
**	Set GPU to a specified power state.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
**
**		gceCHIPPOWERSTATE State
**			Power State.
**
*/
gceSTATUS
gcoHAL_SetPowerManagementState(
	IN gcoHAL Hal,
    IN gceCHIPPOWERSTATE State
    )
{
	gcsHAL_INTERFACE iface;
	gceSTATUS status;

	gcmHEADER_ARG("Hal=0x%x State=%d", Hal, State);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);

	/* Call kernel API to set power management state. */
	iface.command                    = gcvHAL_SET_POWER_MANAGEMENT_STATE;
	iface.u.SetPowerManagement.state = State;
	status = gcoHAL_Call(Hal, &iface);

	/* Return status. */
	gcmFOOTER();
	return status;
}
/*******************************************************************************
**
**	gcoHAL_QueryPowerManagementState
**
**	Query current GPU power state.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
**
**	OUTPUT:
**
**		gceCHIPPOWERSTATE State
**			Power State.
**
*/
gceSTATUS
gcoHAL_QueryPowerManagementState(
	IN gcoHAL Hal,
    OUT gceCHIPPOWERSTATE * State
    )
{
	gcsHAL_INTERFACE iface;
	gceSTATUS status;

	gcmFOOTER_ARG("Hal=0x%x", Hal);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);

	/* Call kernel API to set power management state. */
	iface.command = gcvHAL_QUERY_POWER_MANAGEMENT_STATE;
	gcmONERROR(gcoHAL_Call(Hal, &iface));

	/* Return state to the caller. */
	*State = iface.u.QueryPowerManagement.state;

	/* Success. */
	gcmFOOTER_ARG("*State=%d", *State);
	return gcvSTATUS_OK;

OnError:
	/* Return the status. */
	gcmFOOTER();
	return status;
}

gceSTATUS
gcoHAL_Compact(
	IN gcoHAL Hal
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hal=0x%x", Hal);

	status = gcoOS_Compact(Hal->os);
	gcmFOOTER();
	return status;
}

gceSTATUS
gcoHAL_ProfileStart(
	IN gcoHAL Hal
	)
{
    gceSTATUS status;

	gcmHEADER_ARG("Hal=0x%x", Hal);

	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);

	status = gcoOS_ProfileStart(Hal->os);

	gcmFOOTER();
	return status;
}

gceSTATUS
gcoHAL_ProfileEnd(
	IN gcoHAL Hal,
	IN gctCONST_STRING Title
	)
{
    gceSTATUS status;

	gcmHEADER_ARG("Hal=0x%x Title=%s", Hal, Title);

	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);

	status = gcoOS_ProfileEnd(Hal->os, Title);

	gcmFOOTER();
	return status;
}


gceSTATUS
gcoHAL_DestroySurface(
	IN gcoHAL Hal,
	IN gcoSURF Surface
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hal=0x%0x Surface=0x%0x", Hal, Surface);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);
	gcmVERIFY_OBJECT(Surface, gcvOBJ_SURF);

	/* Destroy the surface using this gcoHAL. */
	Surface->hal = Hal;
	gcmONERROR(gcoSURF_Destroy(Surface));

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;

OnError:
	/* Return the status. */
	gcmFOOTER();
	return status;
}

