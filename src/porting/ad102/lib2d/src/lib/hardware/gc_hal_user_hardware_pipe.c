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

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE	gcvZONE_HARDWARE

/******************************************************************************\
****************************** gcoHARDWARE API Code *****************************
\******************************************************************************/

/*******************************************************************************
**
**	AQHWARDWARE_SelectPipe
**
**	Select the current pipe for this hardare context.
**
**	INPUT:
**
**		gcoHARDWARE Hardware
**			Pointer to an AQHARWDARE object.
**
**		gctUINT8 Pipe
**			Pipe to select.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoHARDWARE_SelectPipe(
	IN gcoHARDWARE Hardware,
	IN gctUINT8 Pipe
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Hardware=0x%x Pipe=%d", Hardware, Pipe);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

	/* Is 2D pipe present? */
	if ((Pipe == 0x1) && !Hardware->hw2DEngine)
	{
		gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
		return gcvSTATUS_NOT_SUPPORTED;
	}

	/* Don't do anything if the pipe has already been selected. */
	if (Hardware->context->currentPipe == Pipe)
	{
		/* Success. */
		gcmFOOTER_NO();
		return gcvSTATUS_OK;
	}

	/* Flush current pipe. */
	gcmVERIFY_OK(gcoHARDWARE_FlushPipe(Hardware));

	/* Send sempahore and stall. */
	gcmVERIFY_OK(
		gcoHARDWARE_Semaphore(Hardware,
							  gcvWHERE_COMMAND,
							  gcvWHERE_PIXEL,
							  gcvHOW_SEMAPHORE_STALL));

    /* Perform a load state. */
    status = gcoHARDWARE_LoadState32(Hardware,
								   0x03800,
								   ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) | (((gctUINT32) ((gctUINT32) (Pipe) & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))));

	/* Return status. */
	gcmFOOTER();
	return status;
}

