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

static gctSIZE_T
_SwitchPipe(
	IN gctUINT32_PTR Buffer,
	IN gctSIZE_T Index,
	IN gctBOOL Pipe2D
	)
{
	if (Buffer != gcvNULL)
	{
		/* Address correct index. */
		Buffer += Index;

		/* Flush the current pipe. */
		*Buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
				  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
				  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));
		*Buffer++ = Pipe2D
				  ? ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) | (((gctUINT32) (0x1&((gctUINT32)((((1?0:0)-(0?0:0)+1)==32)?~0:(~(~0<<((1?0:0)-(0?0:0)+1)))))))<<(0?0:0)))
				  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))) | (((gctUINT32) (0x1&((gctUINT32)((((1?1:1)-(0?1:1)+1)==32)?~0:(~(~0<<((1?1:1)-(0?1:1)+1)))))))<<(0?1:1)))
				  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2))) | (((gctUINT32) (0x1&((gctUINT32)((((1?2:2)-(0?2:2)+1)==32)?~0:(~(~0<<((1?2:2)-(0?2:2)+1)))))))<<(0?2:2)))
				  : ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3))) | (((gctUINT32) (0x1&((gctUINT32)((((1?3:3)-(0?3:3)+1)==32)?~0:(~(~0<<((1?3:3)-(0?3:3)+1)))))))<<(0?3:3)));

		/* Semaphore from FE to PE. */
		*Buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
				  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
				  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));
		*Buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
				  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));

		/* Stall from FE to PE. */
		*Buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));
		*Buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
				  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));

		/* Switch to the requested pipe. */
		*Buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
				  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
				  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E00) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));
		*Buffer++ = Pipe2D
				  ? ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
				  : ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));
	}

	/* Switching pipe takes 8 slots. */
	return 8;
}

static gctSIZE_T
_Flush3DPipe(
	IN gctUINT32_PTR Buffer,
	IN gctSIZE_T Index
	)
{
	if (Buffer != gcvNULL)
	{
		/* Address correct index. */
		Buffer += Index;

		/* Flush the current pipe. */
		*Buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
				  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
				  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

		*Buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) | (((gctUINT32) (0x1&((gctUINT32)((((1?0:0)-(0?0:0)+1)==32)?~0:(~(~0<<((1?0:0)-(0?0:0)+1)))))))<<(0?0:0)))
				  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))) | (((gctUINT32) (0x1&((gctUINT32)((((1?1:1)-(0?1:1)+1)==32)?~0:(~(~0<<((1?1:1)-(0?1:1)+1)))))))<<(0?1:1)))
				  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2))) | (((gctUINT32) (0x1&((gctUINT32)((((1?2:2)-(0?2:2)+1)==32)?~0:(~(~0<<((1?2:2)-(0?2:2)+1)))))))<<(0?2:2)));

		/* Semaphore from FE to PE. */
		*Buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
				  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
				  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));
		*Buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
				  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));

		/* Stall from FE to PE. */
		*Buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));
		*Buffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 4:0) - (0 ? 4:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
				  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ? 12:8) - (0 ? 12:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));
	}

	/* Flushing 3D pipe takes 6 slots. */
	return 6;
}

static gctSIZE_T
_State(
	IN gcoCONTEXT Context,
	IN gctSIZE_T Index,
	IN gctUINT32 Address,
	IN gctUINT32 Value,
	IN gctSIZE_T Size,
	IN gctBOOL FixedPoint,
	IN gctBOOL Hinted
	)
{
	gctUINT32_PTR buffer = Context->buffer;
	gctSIZE_T align, i;

	/* Determine if we need alignment. */
	align = (Index & 1) ? 1 : 0;

	/* Test for flush/break. */
	if (Size == 0)
	{
		/* Align context buffer. */
		if (align && (buffer != gcvNULL))
		{
			buffer[Index] = 0xDEADDEAD;
		}

		/* Reset last address. */
		Context->lastAddress = ~0U;

		/* Return alignment requirement. */
		return align;
	}

	if ((buffer == gcvNULL)
	&&  (Address + Size > Context->stateCount)
	)
	{
		/* Determine maximum state. */
		Context->stateCount = Address + Size;
	}

	if (Hinted && (buffer == gcvNULL))
	{
		/* Increment number of hinted states. */
		Context->hintCount += Size;
	}

	/* See if we can append this state to the previous one. */
	if ((Address == Context->lastAddress)
	&&  (FixedPoint == Context->lastFixed)
	)
	{
		if (buffer != gcvNULL)
		{
			/* Update last load state. */
			buffer[Context->lastIndex] =
				((((gctUINT32) (buffer[Context->lastIndex])) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (Context->lastSize+Size) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

			/* Walk all the states. */
			for (i = 0; i < Size; ++i)
			{
				/* Set state to uninitialized value. */
				buffer[Index + i] = Value;

				/* Set index in state mapping table. */
				Context->map[Address + i] = Index + i;

				/* Save hint. */
				if (Context->hint != gcvNULL)
				{
					Context->hint[Address + i] = Hinted ? 1 : 0;
				}
			}
		}

		/* Update last address and size. */
		Context->lastAddress += Size;
		Context->lastSize    += Size;

		/* Return number of slots required. */
		return Size;
	}

	if (buffer != gcvNULL)
	{
		if (align)
		{
			/* Add filler. */
			buffer[Index++] = 0xDEADDEAD;
		}

		/* LoadState(Address, Count). */
		gcmASSERT((Index & 1) == 0);
		buffer[Index] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
					  | (FixedPoint
						 ? ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)))
						 : ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)))
					  )
					  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (Size) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
					  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (Address) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

		/* Walk all the states. */
		for (i = 0; i < Size; ++i)
		{
			/* Set state to uninitialized value. */
			buffer[Index + 1 + i] = Value;

			/* Set index in state mapping table. */
			Context->map[Address + i] = Index + 1 + i;

			/* Save hint. */
			if (Context->hint != gcvNULL)
			{
				Context->hint[Address + i] = Hinted ? 1 : 0;
			}
		}
	}

	/* Save information for this LoadState. */
	Context->lastIndex   = Index;
	Context->lastAddress = Address + Size;
	Context->lastSize    = Size;
	Context->lastFixed   = FixedPoint;

	/* Return size for load state. */
	return align + 1 + Size;
}

static gceSTATUS
_InitializeContextBuffer(
	IN gcoCONTEXT Context
	)
{
	gctUINT32_PTR buffer = Context->buffer;
	gctSIZE_T index = 0;

	Context->lastAddress = ~0U;

	if (gcoHARDWARE_IsFeatureAvailable(Context->hardware,
	                                   gcvFEATURE_PIPE_2D) == gcvSTATUS_TRUE)
	{
		/**********************************************************************/
		/* Switch to 2D pipe. *************************************************/

		index += _SwitchPipe(buffer, index, gcvTRUE);
	}
	else
	{
		/**********************************************************************/
		/* Flush 3D pipe. *****************************************************/

		index += _Flush3DPipe(buffer, index);
	}

#define _STATE(reg) \
	index += _State(Context, \
					index, \
					reg ## _Address >> 2, \
					reg ## _ResetValue, \
					reg ## _Count, \
					gcvFALSE, \
					gcvFALSE)
#define _STATE_COUNT(reg, count) \
	index += _State(Context, \
					index, \
					reg ## _Address >> 2, \
					reg ## _ResetValue, \
					count, \
					gcvFALSE, \
					gcvFALSE)
#define _STATE_HINT(reg) \
	index += _State(Context, \
					index, \
					reg ## _Address >> 2, \
					reg ## _ResetValue, \
					reg ## _Count, \
					gcvFALSE, \
					gcvTRUE)
#define _STATE_X(reg) \
	index += _State(Context, \
					index, \
					reg ## _Address >> 2, \
					reg ## _ResetValue, \
					reg ## _Count, \
					gcvTRUE, \
					gcvFALSE)
#define _FLUSH() \
	index += _State(Context, index, 0, 0, 0, gcvFALSE, gcvFALSE)

	if (gcoHARDWARE_IsFeatureAvailable(Context->hardware,
	                                   gcvFEATURE_PIPE_2D) == gcvSTATUS_TRUE)
	{
		/**********************************************************************/
		/* Build 2D states. ***************************************************/

		/* Source states. */
		index += _State(Context, index, 0x01200 >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
		index += _State(Context, index, 0x01204 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x01208 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x0120C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x01210 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x01214 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x01218 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x0121C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);

		/* Stretch factor states. */
		index += _State(Context, index, 0x01220 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x01224 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);

		/* Destination states. */
		index += _State(Context, index, 0x01228 >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
		index += _State(Context, index, 0x0122C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x01230 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x01234 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);

		/* Pattern states. */
		index += _State(Context, index, 0x01238 >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
		index += _State(Context, index, 0x01240 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x01244 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x01248 >> 2, 0xFFFFFFFF, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x0124C >> 2, 0xFFFFFFFF, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x01250 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x01254 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x0123C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);

		/* FilterBlt states. */
		index += _State(Context, index, 0x01258 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x01800 >> 2, 0x00000000, 128, gcvFALSE, gcvFALSE);

		/* Color lookup table. */
		index += _State(Context, index, 0x01C00 >> 2, 0x00000000, 256, gcvFALSE, gcvFALSE);

		/* 2D states. */
		index += _State(Context, index, 0x0125C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x01260 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x01264 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x01268 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x01270 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x01274 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x0126C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x01278 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);

		/* Alpha states. */
		index += _State(Context, index, 0x0127C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		index += _State(Context, index, 0x01280 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);

		/* YUV states. */
		if (gcoHARDWARE_IsFeatureAvailable(Context->hardware,
										   gcvFEATURE_YUV420_SCALER)
			== gcvSTATUS_TRUE)
		{
			index += _State(Context, index, 0x01284 >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
			index += _State(Context, index, 0x01288 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
			index += _State(Context, index, 0x0128C >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
			index += _State(Context, index, 0x01290 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
			index += _State(Context, index, 0x01298 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
			index += _State(Context, index, 0x0129C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
			index += _State(Context, index, 0x012A0 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
			index += _State(Context, index, 0x012A4 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
			index += _State(Context, index, 0x012A8 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
			index += _State(Context, index, 0x012AC >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
			index += _State(Context, index, 0x012E4 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
		}

		/* 2.0 states. */
		if (gcoHARDWARE_IsFeatureAvailable(Context->hardware,
										   gcvFEATURE_2DPE20) == gcvSTATUS_TRUE)
		{
			index += _State(Context, index, 0x012B0 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
			index += _State(Context, index, 0x012B4 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
			index += _State(Context, index, 0x012B8 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
			index += _State(Context, index, 0x012BC >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
			index += _State(Context, index, 0x012C0 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
			index += _State(Context, index, 0x012C4 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
			index += _State(Context, index, 0x012C8 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
			index += _State(Context, index, 0x012CC >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
			index += _State(Context, index, 0x012D0 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
			index += _State(Context, index, 0x012D4 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
			index += _State(Context, index, 0x012D8 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
			index += _State(Context, index, 0x012DC >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
			index += _State(Context, index, 0x012E0 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);

			index += _State(Context, index, 0x03400 >> 2, 0x00000000, 256, gcvFALSE, gcvFALSE);
		}
		_FLUSH();
	}

	if (gcoHARDWARE_IsFeatureAvailable(Context->hardware,
	                                   gcvFEATURE_PIPE_2D) == gcvSTATUS_TRUE)
	{
		/**********************************************************************/
		/* Switch to 3D pipe. *************************************************/

		Context->pipe3DIndex = index;

		index += _SwitchPipe(buffer, index, gcvFALSE);
	}


	/* Global states. */
	index += _State(Context, index, 0x03814 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x03818 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x0381C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x03820 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x03828 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x0382C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);

	/* Front End states. */
	index += _State(Context, index, 0x00600 >> 2, 0x00000000, 16, gcvFALSE, gcvFALSE);
	_FLUSH();
	index += _State(Context, index, 0x00644 >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x00648 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x0064C >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x00650 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00680 >> 2, 0x00000000, 8, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x006A0 >> 2, 0x00000000, 8, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00670 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);

	/* Vertex Shader states. */
	index += _State(Context, index, 0x00800 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00804 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00808 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x0080C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00810 >> 2, 0x00000000, 4, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00820 >> 2, 0x00000000, 4, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00830 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00838 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x04000 >> 2, 0x00000000, 1024, gcvFALSE, gcvFALSE);
	_FLUSH();
	index += _State(Context, index, 0x00850 >> 2, 0x000003E8, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00854 >> 2, 0x00000100, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00858 >> 2, 0x00001005, 1, gcvFALSE, gcvFALSE);

	/* Primitive Assembly states. */
	index += _State(Context, index, 0x00A00 >> 2, 0x00000000, 1, gcvTRUE, gcvFALSE);
	index += _State(Context, index, 0x00A04 >> 2, 0x00000000, 1, gcvTRUE, gcvFALSE);
	index += _State(Context, index, 0x00A08 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00A0C >> 2, 0x00000000, 1, gcvTRUE, gcvFALSE);
	index += _State(Context, index, 0x00A10 >> 2, 0x00000000, 1, gcvTRUE, gcvFALSE);
	index += _State(Context, index, 0x00A14 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00A18 >> 2, 0x00000000, 1,gcvFALSE,gcvFALSE);
	index += _State(Context, index, 0x00A1C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00A28 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00A2C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00A30 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00A40 >> 2, 0x00000000, 10, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00A34 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);

	/* Setup states. */
	index += _State(Context, index, 0x00C00 >> 2, 0x00000000, 1, gcvTRUE, gcvFALSE);
	index += _State(Context, index, 0x00C04 >> 2, 0x00000000, 1, gcvTRUE, gcvFALSE);
	index += _State(Context, index, 0x00C08 >> 2, 0x45000000, 1, gcvTRUE, gcvFALSE);
	index += _State(Context, index, 0x00C0C >> 2, 0x45000000, 1, gcvTRUE, gcvFALSE);
	index += _State(Context, index, 0x00C10 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00C14 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00C18 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);

	/* Raster states. */
	index += _State(Context, index, 0x00E00 >> 2, 0x00000001, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00E10 >> 2, 0x00000000, 4, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00E04 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00E40 >> 2, 0x00000000, 16, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x00E08 >> 2, 0x00000001, 1, gcvFALSE, gcvFALSE);

	/* Pixel Shader states. */
	index += _State(Context, index, 0x01000 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01004 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01008 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x0100C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01010 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01018 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x06000 >> 2, 0x00000000, 1024, gcvFALSE, gcvFALSE);
	_FLUSH();

	/* Texture states. */
	index += _State(Context, index, 0x02000 >> 2, 0x00000000, 16, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x02040 >> 2, 0x00000000, 16, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x02080 >> 2, 0x00000000, 12, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x020C0 >> 2, 0x00000000, 12, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x02100 >> 2, 0x00000000, 12, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x02140 >> 2, 0x00000000, 16, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x02400 >> 2, 0x00000000, 16, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x02440 >> 2, 0x00000000, 12, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x02480 >> 2, 0x00000000, 12, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x024C0 >> 2, 0x00000000, 12, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x02500 >> 2, 0x00000000, 12, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x02540 >> 2, 0x00000000, 12, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x02580 >> 2, 0x00000000, 12, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x025C0 >> 2, 0x00000000, 12, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x02600 >> 2, 0x00000000, 12, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x02640 >> 2, 0x00000000, 12, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x02680 >> 2, 0x00000000, 12, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x026C0 >> 2, 0x00000000, 12, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x02700 >> 2, 0x00000000, 12, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x02740 >> 2, 0x00000000, 12, gcvFALSE, gcvTRUE);

    _FLUSH();
    index += _Flush3DPipe(buffer, index);

	/* Pixel Engine states. */
	index += _State(Context, index, 0x01400 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01404 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01408 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x0140C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01410 >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x01414 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01418 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x0141C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01420 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01424 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01428 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x0142C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01430 >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x01434 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01454 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01458 >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x0145C >> 2, 0x00000010, 1, gcvFALSE, gcvFALSE);

	/* Resolve states. */
	index += _State(Context, index, 0x01604 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01608 >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x0160C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01610 >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x01614 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01620 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01630 >> 2, 0x00000000, 2, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01640 >> 2, 0x00000000, 4, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x0163C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01654 >> 2, 0x00200000, 1, gcvFALSE, gcvFALSE);
    _FLUSH();
	index += _State(Context, index, 0x01658 >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
    _FLUSH();
	index += _State(Context, index, 0x0165C >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x01660 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
    _FLUSH();
	index += _State(Context, index, 0x01664 >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
    _FLUSH();
	index += _State(Context, index, 0x01668 >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x0166C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01678 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x0167C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01680 >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x01684 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01688 >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x0168C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01690 >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x01694 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x01698 >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x0169C >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	index += _State(Context, index, 0x016A0 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
    _FLUSH();
	index += _State(Context, index, 0x016A4 >> 2, 0x00000000, 1, gcvFALSE, gcvTRUE);
	index += _State(Context, index, 0x016A8 >> 2, 0x00000000, 1, gcvFALSE, gcvFALSE);
	_FLUSH();

	if (gcoHARDWARE_IsFeatureAvailable(Context->hardware,
	                                   gcvFEATURE_PIPE_2D) == gcvSTATUS_TRUE)
	{
		/**********************************************************************/
		/* Switch to 2D pipe. *************************************************/

		Context->pipe2DIndex = index;
		index += _SwitchPipe(buffer, index, gcvTRUE);
	}
	else
	{
		Context->pipe2DIndex = 0;
	}

	/**************************************************************************/
	/* Link to another address. ***********************************************/

	Context->linkIndex = index;
	if (buffer != gcvNULL)
	{
		buffer[index + 0] = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x08 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
						  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

		buffer[index + 1] = 0;
	}
	index += 2;

	/* Save index to inUse flag. */
	Context->inUseIndex = index;

	/* Save size for buffer. */
	Context->bufferSize = (index + 1) * gcmSIZEOF(gctUINT32);

	/* Success. */
	return gcvSTATUS_OK;
}

/******************************************************************************\
**
**  gcoCONTEXT_Construct
**
**  Construct a new gcoCONTEXT object.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to a gcoOS object.
**
**      gcoHARDWARE Hardware
**          Pointer to a gcoHARDWARE object.
**
**  OUTPUT:
**
**      gcoCONTEXT * Context
**          Pointer to a variable thet will receive the gcoCONTEXT object
**          pointer.
*/
gceSTATUS
gcoCONTEXT_Construct(
    IN gcoOS Os,
	IN gcoHARDWARE Hardware,
    OUT gcoCONTEXT * Context
    )
{
    gceSTATUS status;
    gcoCONTEXT context = gcvNULL;
#ifdef __QNXNTO__
	gctPHYS_ADDR physical;
	gctSIZE_T bytes;
#endif

	gcmHEADER_ARG("Os=0x%x Hardware=0x%x", Os, Hardware);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmVERIFY_ARGUMENT(Context != gcvNULL);

    /* Allocate the AQCONSTRUCT object. */
#ifdef __QNXNTO__
	bytes = sizeof(struct _gcoCONTEXT);
	gcmONERROR(
		gcoOS_AllocateContiguous(Os,
								 gcvTRUE,
								 &bytes,
								 &physical,
								 (gctPOINTER *) &context));
#else
	gcmONERROR(
		gcoOS_Allocate(Os,
	    			   gcmSIZEOF(struct _gcoCONTEXT),
   					   (gctPOINTER *) &context));
#endif

	/* Initialize the gcoCONTEXT object. */
	context->object.type = gcvOBJ_CONTEXT;
	context->os          = Os;
	context->hardware    = Hardware;
	context->id          = 0;
	context->map         = gcvNULL;
	context->buffer      = gcvNULL;
	context->hint        = gcvNULL;
	context->hintArray   = gcvNULL;
	context->hintValue   = 2;

	if (gcoHARDWARE_IsFeatureAvailable(context->hardware,
	                                   gcvFEATURE_PIPE_2D) == gcvSTATUS_TRUE)
	{
		context->initialPipe = 0x1;
	}
	else
	{
		context->initialPipe = 0x0;
	}
	context->currentPipe = 0x0;
	context->entryPipe   = 0x0;

	context->stateCount = 0;
	context->hintCount  = 1;

	/* Get the size of the context buffer. */
	gcmONERROR(
		_InitializeContextBuffer(context));

	/* Allocate the state mapping table. */
	gcmONERROR(
		gcoOS_Allocate(Os,
					   gcmSIZEOF(gctUINT32) * context->stateCount,
					   (gctPOINTER *) &context->map));

	/* Zero the state mapping table. */
	gcmONERROR(
		gcoOS_ZeroMemory(context->map,
						 gcmSIZEOF(gctUINT32) * context->stateCount));

#if gcdSECURE_USER
	/* Allocate the hint table. */
	gcmONERROR(
		gcoOS_Allocate(Os,
					   gcmSIZEOF(gctUINT8) * context->stateCount,
					   (gctPOINTER *) &context->hint));

	/* Zero the hint table. */
	gcmONERROR(
		gcoOS_ZeroMemory(context->hint,
						 gcmSIZEOF(gctUINT8) * context->stateCount));
#endif

	/* Allocate the context buffer. */
	gcmONERROR(
		gcoOS_Allocate(Os,
					   context->bufferSize,
					   (gctPOINTER *) &context->buffer));

#if gcdSECURE_USER
	/* Allocate the hint array. */
	gcmONERROR(
		gcoOS_Allocate(Os,
					   context->hintCount * gcmSIZEOF(gctUINT32),
					   (gctPOINTER *) &context->hintArray));

	/* Initialize hinting. */
	context->hintIndex = context->hintArray;
#endif

	/* Initialize the context buffer. */
	gcmONERROR(
		_InitializeContextBuffer(context));

	/* No memory for physical context buffer yet. */
	context->logical = gcvNULL;
	context->inUse   = gcvNULL;

	/* Not yet committed. */
	context->postCommit = gcvFALSE;

	/* Return pointer to the gcoCONTEXT object. */
	*Context = context;

	/* Success. */
	gcmFOOTER_ARG("*Context=0x%x", *Context);
	return gcvSTATUS_OK;

OnError:
	/* Roll back on error. */
	if (context != gcvNULL)
	{
		/* Free the gcsCONTEXTBUFFER structure if allocated. */
		if (context->buffer != gcvNULL)
		{
			gcmVERIFY_OK(gcoOS_Free(Os, context->buffer));
		}

		/* Free the hint array if allocates. */
		if (context->hintArray != gcvNULL)
		{
			gcmVERIFY_OK(gcoOS_Free(Os, context->hintArray));
		}

		/* Free the hint table if allocated. */
		if (context->hint != gcvNULL)
		{
			gcmVERIFY_OK(gcoOS_Free(Os, context->hint));
		}

		/* Free the state map if allocated. */
		if (context->map != gcvNULL)
		{
			gcmVERIFY_OK(gcoOS_Free(Os, context->map));
		}

		/* Free the gcoCONTEXT structure. */
#ifdef __QNXNTO__
        gcmVERIFY_OK(gcoOS_FreeContiguous(Os,
			gcvNULL,
			context,
    		sizeof(struct _gcoCONTEXT)
			));
#else
        gcmVERIFY_OK(gcoOS_Free(Os, context));
#endif
	}

	/* Return the status. */
	gcmFOOTER();
	return status;
}

/******************************************************************************\
**
**  gcoCONTEXT_Destroy
**
**  Destroy a gcoCONTEXT object.
**
**  INPUT:
**
**      gcoCONTEXT Context
**          Pointer to an gcoCONTEXT object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoCONTEXT_Destroy(
    IN gcoCONTEXT Context
    )
{
	gcmHEADER_ARG("Context=0x%x", Context);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Context, gcvOBJ_CONTEXT);

    /* Free the context buffer. */
    gcmVERIFY_OK(gcoOS_Free(Context->os, Context->buffer));

    /* Free the physical context buffer if allocated. */
    if (Context->logical != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_FreeContiguous(Context->os,
										  Context->physical,
										  Context->logical,
										  Context->bytes));
    }

	/* Free the state mapping. */
	if (Context->map != gcvNULL)
	{
		gcmVERIFY_OK(gcoOS_Free(Context->os, Context->map));
	}

	/* Free the state hinting. */
	if (Context->hint != gcvNULL)
	{
		gcmVERIFY_OK(gcoOS_Free(Context->os, Context->hint));
	}

	/* Free the hint array. */
	if (Context->hintArray != gcvNULL)
	{
		gcmVERIFY_OK(gcoOS_Free(Context->os, Context->hintArray));
	}

    /* Mark the gcoCONTEXT object as unknown. */
    Context->object.type = gcvOBJ_UNKNOWN;

    /* Free the gcoCONTEXT object. */
#ifdef __QNXNTO__
        gcmVERIFY_OK(gcoOS_FreeContiguous(
			Context->os,
			gcvNULL,
			Context,
    		sizeof(struct _gcoCONTEXT)
			));
#else
	gcmVERIFY_OK(gcoOS_Free(Context->os, Context));
#endif


    /* Success. */
	gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

#define _ENABLE(reg, field) \
	do { \
		if (gcmVERIFYFIELDVALUE(data, reg, MASK_ ## field, ENABLED)) { \
			enable |= gcmFIELDMASK(reg, field); \
		} \
	} while (gcvFALSE)

/******************************************************************************\
**
**  gcoCONTEXT_Buffer
**
**  Buffer a number of states.
**
**  INPUT:
**
**      gcoCONTEXT Context
**          Pointer to an gcoCONTEXT object.
**
**      gctUINT32 Address
**          Address of first state to buffer.
**
**		gctSIZE_T Count
**			Number of states to buffer.
**
**      gctUINT32_PTR Data
**          Pointer to state data to buffer.
**
**  OUTPUT:
**
**      gctBOOL_PTR Hints
**			Optional pointer that will receive an array whether or not each
**			buffered state has a physical address or not.
*/
gceSTATUS
gcoCONTEXT_Buffer(
    IN gcoCONTEXT Context,
    IN gctUINT32 Address,
	IN gctSIZE_T Count,
    IN gctUINT32_PTR Data,
	OUT gctBOOL_PTR Hints
    )
{
    gceSTATUS status;
	gctUINT32 address = Address >> 2;
	gctSIZE_T i;
	gctUINT32 enable;
	gctBOOL_PTR hints;

	gcmHEADER_ARG("Context=0x%x Address=%x Count=%d Data=0x%x Hints=0x%x",
					Context, Address, Count, Data, Hints);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Context, gcvOBJ_CONTEXT);

	/* Get the pointer to the hints. */
	hints = (Context->hint == gcvNULL) ? gcvNULL : Hints;

	do
	{
	    /* See if PostCommit needs to be called. */
	    if (Context->postCommit)
		{
	        /* Call PostCommit. */
		    gcmERR_BREAK(gcoCONTEXT_PostCommit(Context));
		}

		if (hints != gcvNULL)
		{
			*hints++ = gcvFALSE;
		}

		/* Process all state addresses. */
		for (i = 0; i < Count; ++i, ++address, ++Data)
		{
			/* Lookup index into context buffer. */
			gctUINT32 index = Context->map[address];
			gctUINT32 data  = *Data;

			/* Dump the state. */
			gcmDUMP(Context->os, "@[state 0x%04X 0x%08X]", address, data);

			if (hints != gcvNULL)
			{
				*hints++ = (Context->hint[address] != 0);
			}

			if ((Context->hint          != gcvNULL)
			&&  (Context->hint[address] != 0)
			&&  (Context->hint[address]  < Context->hintValue)
			)
			{
				gcmASSERT(Context->hintIndex != gcvNULL);
				*Context->hintIndex++  = index * gcmSIZEOF(gctUINT32);
				Context->hint[address] = Context->hintValue;
			}

		    /* Test for special states. */
			switch (address)
			{
			case 0x0E00:
			    /* Save new pipe. */
				Context->currentPipe = data;
				break;

			case 0x048F:
				gcmASSERT(index != 0);

				/* Initialize all stuff for patterns. */
				Context->buffer[index] = ((((gctUINT32) (data)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:6) - (0 ? 7:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ? 7:6))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ? 7:6) - (0 ? 7:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ? 7:6)));
				break;

			case 0x028D:
				gcmASSERT(index != 0);

				/* Determine write enable. */
				enable = 0;
				do { if (((((gctUINT32) (data)) >> (0 ? 3:3) & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 5:5) & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 10:10) & ((gctUINT32) ((((1 ? 10:10) - (0 ? 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 10:10) - (0 ? 10:10) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:10) - (0 ? 10:10) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 9:8) - (0 ? 9:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 14:14) & ((gctUINT32) ((((1 ? 14:14) - (0 ? 14:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:14) - (0 ? 14:14) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 14:14) - (0 ? 14:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:14) - (0 ? 14:14) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 13:12) - (0 ? 13:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 18:18) & ((gctUINT32) ((((1 ? 18:18) - (0 ? 18:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 18:18) - (0 ? 18:18) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 18:18) - (0 ? 18:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 18:18) - (0 ? 18:18) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 21:21) & ((gctUINT32) ((((1 ? 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ? 20:20)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 23:23) & ((gctUINT32) ((((1 ? 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ? 22:22)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 28:28) & ((gctUINT32) ((((1 ? 28:28) - (0 ? 28:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:28) - (0 ? 28:28) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 28:28) - (0 ? 28:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 28:28) - (0 ? 28:28) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ? 27:24)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 30:30) & ((gctUINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 29:29) - (0 ? 29:29) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)); } } while(gcvFALSE);

				/* Copy in enabled bits. */
				Context->buffer[index] = (Context->buffer[index] & ~enable)
									   | (data & enable);
				break;

			case 0x0500:
				gcmASSERT(index != 0);

				/* Determine write enable. */
				enable = 0;
				do { if (((((gctUINT32) (data)) >> (0 ? 31:31) & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 27:27) & ((gctUINT32) ((((1 ? 27:27) - (0 ? 27:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 27:27) - (0 ? 27:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 25:25) & ((gctUINT32) ((((1 ? 25:25) - (0 ? 25:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 25:25) - (0 ? 25:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 21:21) & ((gctUINT32) ((((1 ? 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ? 20:20)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 17:17) & ((gctUINT32) ((((1 ? 17:17) - (0 ? 17:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:17) - (0 ? 17:17) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 17:17) - (0 ? 17:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:17) - (0 ? 17:17) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 13:13) & ((gctUINT32) ((((1 ? 13:13) - (0 ? 13:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 13:13) - (0 ? 13:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 11:11) & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 10:8) - (0 ? 10:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ? 10:8)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 5:5) & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 3:3) & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)); } } while(gcvFALSE);

				/* Copy in enabled bits. */
				Context->buffer[index] = (Context->buffer[index] & ~enable)
									   | (data & enable);
				break;

			case 0x0506:
				gcmASSERT(index != 0);

				/* Determine write enable. */
				enable = 0;
				do { if (((((gctUINT32) (data)) >> (0 ? 31:31) & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 27:27) & ((gctUINT32) ((((1 ? 27:27) - (0 ? 27:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 27:27) - (0 ? 27:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:27) - (0 ? 27:27) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 26:24) - (0 ? 26:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 23:23) & ((gctUINT32) ((((1 ? 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 22:20) - (0 ? 22:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 19:19) & ((gctUINT32) ((((1 ? 19:19) - (0 ? 19:19) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:19) - (0 ? 19:19) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 19:19) - (0 ? 19:19) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:19) - (0 ? 19:19) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 18:16) - (0 ? 18:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 15:15) & ((gctUINT32) ((((1 ? 15:15) - (0 ? 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 15:15) - (0 ? 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 11:11) & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 10:8) - (0 ? 10:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ? 10:8)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 7:7) & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 6:4) - (0 ? 6:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 3:3) & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)); } } while(gcvFALSE);

				/* Copy in enabled bits. */
				Context->buffer[index] = (Context->buffer[index] & ~enable)
									   | (data & enable);
				break;

			case 0x0507:
				gcmASSERT(index != 0);

				/* Determine write enable. */
				enable = 0;
				do { if (((((gctUINT32) (data)) >> (0 ? 7:7) & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 31:24) - (0 ? 31:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ? 31:24)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 6:6) & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 23:16) - (0 ? 23:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:16) - (0 ? 23:16) + 1))))))) << (0 ? 23:16)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 5:5) & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 4:4) & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)); } } while(gcvFALSE);

				/* Copy in enabled bits. */
				Context->buffer[index] = (Context->buffer[index] & ~enable)
									   | (data & enable);
				break;

			case 0x0508:
				gcmASSERT(index != 0);

				/* Determine write enable. */
				enable = 0;
				do { if (((((gctUINT32) (data)) >> (0 ? 16:16) & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 15:8) - (0 ? 15:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 7:7) & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 7:7) - (0 ? 7:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:7) - (0 ? 7:7) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 6:4) - (0 ? 6:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 1:1) & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)); } } while(gcvFALSE);

				/* Copy in enabled bits. */
				Context->buffer[index] = (Context->buffer[index] & ~enable)
									   | (data & enable);
				break;

			case 0x050A:
				gcmASSERT(index != 0);

				/* Determine write enable. */
				enable = 0;
				do { if (((((gctUINT32) (data)) >> (0 ? 31:31) & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 19:19) & ((gctUINT32) ((((1 ? 19:19) - (0 ? 19:19) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:19) - (0 ? 19:19) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 19:19) - (0 ? 19:19) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:19) - (0 ? 19:19) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 27:24) - (0 ? 27:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ? 27:24)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 18:18) & ((gctUINT32) ((((1 ? 18:18) - (0 ? 18:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 18:18) - (0 ? 18:18) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 18:18) - (0 ? 18:18) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 18:18) - (0 ? 18:18) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 23:20) - (0 ? 23:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ? 23:20)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 17:17) & ((gctUINT32) ((((1 ? 17:17) - (0 ? 17:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:17) - (0 ? 17:17) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 17:17) - (0 ? 17:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:17) - (0 ? 17:17) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 15:15) & ((gctUINT32) ((((1 ? 15:15) - (0 ? 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 15:15) - (0 ? 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 3:3) & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 2:2) & ((gctUINT32) ((((1 ? 2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 2:2) - (0 ? 2:2) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:2) - (0 ? 2:2) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 1:1) & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:1) - (0 ? 1:1) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)); } } while(gcvFALSE);

				/* Copy in enabled bits. */
				Context->buffer[index] = (Context->buffer[index] & ~enable)
									   | (data & enable);
				break;

			case 0x050B:
				gcmASSERT(index != 0);

				/* Determine write enable. */
				enable = 0;
				do { if (((((gctUINT32) (data)) >> (0 ? 21:21) & ((gctUINT32) ((((1 ? 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 20:20) - (0 ? 20:20) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ? 20:20)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 17:17) & ((gctUINT32) ((((1 ? 17:17) - (0 ? 17:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:17) - (0 ? 17:17) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 17:17) - (0 ? 17:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:17) - (0 ? 17:17) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 12:12) & ((gctUINT32) ((((1 ? 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 11:8) - (0 ? 11:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 4:4) & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)); } } while(gcvFALSE);

				/* Copy in enabled bits. */
				Context->buffer[index] = (Context->buffer[index] & ~enable)
									   | (data & enable);
				break;

			case 0x0E06:
				gcmASSERT(index != 0);

				/* Determine write enable. */
				enable = 0;
				do { if (((((gctUINT32) (data)) >> (0 ? 3:3) & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 8:8) & ((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 8:8) - (0 ? 8:8) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 7:4) - (0 ? 7:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 15:15) & ((gctUINT32) ((((1 ? 15:15) - (0 ? 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 15:15) - (0 ? 15:15) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:15) - (0 ? 15:15) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 14:12) - (0 ? 14:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12)); } } while(gcvFALSE);
				do { if (((((gctUINT32) (data)) >> (0 ? 19:19) & ((gctUINT32) ((((1 ? 19:19) - (0 ? 19:19) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:19) - (0 ? 19:19) + 1)))))) == (0x0 & ((gctUINT32) ((((1 ? 19:19) - (0 ? 19:19) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 19:19) - (0 ? 19:19) + 1)))))))) { enable |=  (((gctUINT32) (((gctUINT32) ((((1 ? 17:16) - (0 ? 17:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16)); } } while(gcvFALSE);

				/* Copy in enabled bits. */
				Context->buffer[index] = (Context->buffer[index] & ~enable)
									   | (data & enable);
				break;

			case 0x0595:
				gcmASSERT(index != 0);

				/* Buffer state but force auto-disable to be disabled. */
				Context->buffer[index] = ((((gctUINT32) (data)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)));
				Context->buffer[index] = ((((gctUINT32) (Context->buffer[index])) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));
				Context->buffer[index] = ((((gctUINT32) (Context->buffer[index])) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 13:13) - (0 ? 13:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ? 13:13))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 13:13) - (0 ? 13:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ? 13:13)));
				break;

			default:
				/* Buffer state value. */
				if (index != 0)
				{
					if ((address & (~0U << 4)) ==
						0x0180)
					{
						gctUINT32 offset = address
										 & ~(~0 << 4);
						gctUINT32 base = index - offset;

						Context->buffer[base - 1]
							= ((((gctUINT32) (Context->buffer[index-offset-1])) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (offset+1) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

						for (++offset;
							 offset < 16;
							 ++offset
						)
						{
							Context->buffer[base + offset]
								= (offset & 1)
								? ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x03 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
								: 0xDEADDEAD;
						}
					}

					Context->buffer[index] = data;
				}
				break;
			}
		}

		/* Success. */
		gcmFOOTER_NO();
		return gcvSTATUS_OK;
    }
	while (gcvFALSE);

    /* Return the status. */
	gcmFOOTER();
    return status;
}

/******************************************************************************\
**
**  AQCONTEXT_BufferX
**
**  Buffer a number of states given in fixed-point.
**
**  INPUT:
**
**      gcoCONTEXT Context
**          Pointer to an gcoCONTEXT object.
**
**      gctUINT32 Address
**          Address of first state to buffer.
**
**		gctSIZE_T Count
**			Number of states to buffer.
**
**      gctUINT32_PTR Data
**          Pointer to state data to buffer.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoCONTEXT_BufferX(
    IN gcoCONTEXT Context,
    IN gctUINT32 Address,
	IN gctSIZE_T Count,
    IN gctFIXED_POINT * Data
    )
{
    gceSTATUS status;
	gctUINT32 address = Address >> 2;
	gctSIZE_T i;

	gcmHEADER_ARG("Context=0x%x Address=%x Count=%d Data=0x%x",
					Context, Address, Count, Data);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Context, gcvOBJ_CONTEXT);

	do
	{
	    /* See if PostCommit needs to be called. */
	    if (Context->postCommit)
		{
	        /* Call PostCommit. */
		    gcmERR_BREAK(gcoCONTEXT_PostCommit(Context));
		}

		/* Process all state addresses. */
		for (i = 0; i < Count; ++i, ++address, ++Data)
		{
			/* Lookup index into context buffer. */
			gctUINT32 index = Context->map[address];

			gctFIXED_POINT data = *Data;

			/* Dump the state. */
			gcmDUMP(Context->os, "@[state.x 0x%04X 0x%08X]", address, data);

		    /* Test for special states. */
			switch (address)
			{
			case 0x0280:
			case 0x0281:
			case 0x0283:
			case 0x0284:
			case 0x0300:
			case 0x0301:
			case 0x0302:
			case 0x0303:
				/* Buffer state value. */
				gcmASSERT(index != 0);
				Context->buffer[index] = data;
				break;

			default:
				if (index != 0)
				{
					/* Convert state to floating point. */
					gctFLOAT flt = data / 65536.0f;
					Context->buffer[index] = *(gctUINT32_PTR) &flt;
				}
			}
		}

		/* Success. */
		gcmFOOTER_NO();
		return gcvSTATUS_OK;
    }
	while (gcvFALSE);

    /* Return the status. */
	gcmFOOTER();
    return status;
}

gceSTATUS
gcoCONTEXT_PreCommit(
    IN OUT gcoCONTEXT Context
    )
{
	gceSTATUS status;

	gcmHEADER_ARG("Context=0x%x", Context);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Context, gcvOBJ_CONTEXT);

	do
	{
		/* See if PostCommit needs to be called. */
		if (Context->postCommit)
		{
			/* Call PostCommit. */
			gcmERR_BREAK(gcoCONTEXT_PostCommit(Context));
        }

		/* Check for hinting. */
		if (Context->hintArray != gcvNULL)
		{
			/* Mark end of hint table. */
			*Context->hintIndex = 0;

			/* Reset hint table. */
			Context->hintIndex = Context->hintArray;
			if (Context->hintValue++ == 255)
			{
				gctSIZE_T i;

				/* Hint value has wrapped around, reset all values. */
				Context->hintValue = 2;
				for (i = 0; i < Context->stateCount; ++i)
				{
					if (Context->hint[i] != 0)
					{
						Context->hint[i] = 1;
					}
				}
			}
		}

		/* Check if there is a physical context buffer. */
		if (Context->logical != gcvNULL)
		{
			/* Extract the pointer to the physical context buffer. */
			gctUINT32_PTR buffer = Context->logical;

			if (gcoHARDWARE_IsFeatureAvailable(Context->hardware,
			                                   gcvFEATURE_PIPE_2D)
			    == gcvSTATUS_TRUE)
			{
				/* Switch to the pipe at the entry of the command buffer. */
				if (((((gctUINT32) (Context->entryPipe)) >> (0 ? 0:0) & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1)))))) == (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 0:0) - (0 ? 0:0) + 1))))))))
				{
					/* 2D pipe. */
					_SwitchPipe(buffer, Context->pipe2DIndex, gcvTRUE);
				}
				else
				{
					gctINT i;

					for (i = 0; i < 8; i += 2)
					{
						buffer[Context->pipe2DIndex + i + 0] =
							((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x03 & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

						buffer[Context->pipe2DIndex + i + 1] = 0xDEADDEAD;
					}
				}
			}

	        /* Setup pointer to LINK. */
			Context->link = (gctPOINTER) &buffer[Context->linkIndex];
		}

		/* Need PostCommit. */
		Context->postCommit = gcvTRUE;

	    /* Success. */
		gcmFOOTER_NO();
		return gcvSTATUS_OK;
	}
	while (gcvFALSE);

    /* Return the status. */
	gcmFOOTER();
    return status;
}

gceSTATUS
gcoCONTEXT_PostCommit(
    IN OUT gcoCONTEXT Context
    )
{
    gceSTATUS status;
    gcsHAL_INTERFACE iface;

	gcmHEADER_ARG("Context=0x%x", Context);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Context, gcvOBJ_CONTEXT);

	do
	{
	    if ((Context->inUse != gcvNULL) && *Context->inUse)
		{
			/* Mark the current physical context buffer for deletion. */
			iface.command = gcvHAL_FREE_CONTIGUOUS_MEMORY;
			iface.u.FreeContiguousMemory.bytes    = Context->bytes;
			iface.u.FreeContiguousMemory.physical = Context->physical;
			iface.u.FreeContiguousMemory.logical  = Context->logical;

	        /* Call the kernel service. */
			gcmERR_BREAK(gcoHARDWARE_CallEvent(Context->hardware, &iface));

			/* Physical context buffer has been freed. */
			Context->logical = gcvNULL;

	        /* Clear the usage flag. */
		    Context->inUse = gcvNULL;
		}

	    if (Context->logical == gcvNULL)
		{
			/* Create a new physical context buffer. */
			Context->bytes = Context->bufferSize;
		    gcmERR_BREAK(gcoOS_AllocateContiguous(Context->os,
												  gcvTRUE,
												  &Context->bytes,
												  &Context->physical,
												  &Context->logical));

			/* Set pointer to inUse flag. */
			Context->inUse = (gctBOOL *) ((gctUINT32_PTR) Context->logical)
						   + Context->inUseIndex;
		}

	    /* Copy the current state into the physical context buffer. */
		gcmVERIFY_OK(gcoOS_MemCopy(Context->logical,
								   Context->buffer,
								   Context->bufferSize));

	    /* Copy current pipe select. */
		Context->entryPipe = Context->currentPipe;

	    /* Context is not yet used. */
		*Context->inUse = gcvFALSE;

	    /* PostCommit finished. */
		Context->postCommit = gcvFALSE;

	    /* Success. */
		gcmFOOTER_NO();
		return gcvSTATUS_OK;
	}
	while (gcvFALSE);

	/* Return the status. */
	gcmFOOTER();
	return status;
}

