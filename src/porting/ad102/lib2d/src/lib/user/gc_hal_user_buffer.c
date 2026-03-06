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
**	gcoBUFFER object for user HAL layers.
**
*/

#include "gc_hal_user_precomp.h"
#include "gc_hal_user_context.h"

#define _GC_OBJ_ZONE			gcvZONE_BUFFER

/******************************************************************************\
********************************** Structures **********************************
\******************************************************************************/

struct _gcoBUFFER
{
	/* Object. */
	gcsOBJECT					object;

	/* Pointer to the required objects. */
	gcoOS						os;
	gcoHAL						hal;
    gcoHARDWARE					hardware;

	/* Size of buffer. */
	gctSIZE_T					size;
	gctSIZE_T					maxSize;

	/* Array of command buffers and their signals. */
    gcoCMDBUF				    commandBuffers[gcdCMD_BUFFERS];
    gctSIGNAL                   signal[gcdCMD_BUFFERS];

	/* Current command buffer. */
	gctUINT                     currentCommandBufferIndex;
    gcoCMDBUF				    currentCommandBuffer;

    /* Reserved bytes. */
	struct _gcsCOMMAND_INFO
	{
		gctSIZE_T	alignment;
		gctSIZE_T	reservedHead;
		gctSIZE_T	reservedTail;
	}							info;
	gctSIZE_T					totalReserved;
};

/******************************************************************************\
***************************** gcoCMDBUF Object Code *****************************
\******************************************************************************/

/*******************************************************************************
**
**	gcoCMDBUF_Construct
**
**	Construct a new gcoCMDBUF object.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to a gcoOS object.
**
**		gcoHARDWARE Hardware
**			Pointer to a gcoHARDWARE object.
**
**		gctSIZE_T Bytes
**			Number of bytes for the buffer.
**
**      gcsCOMMAND_BUFFER_PTR Info
**          Alignment and head/tail information.
**
**	OUTPUT:
**
**		gcoCMDBUF * CommandBuffer
**			Pointer to a variable that will hold the the gcoCMDBUF object
**          pointer.
*/
gceSTATUS
gcoCMDBUF_Construct(
	IN gcoOS Os,
	IN gcoHARDWARE Hardware,
    IN gctSIZE_T Bytes,
	IN gcsCOMMAND_INFO_PTR Info,
	OUT gcoCMDBUF * CommandBuffer
	)
{
    gceSTATUS status;
    gcoCMDBUF commandBuffer = gcvNULL;

    gcmHEADER_ARG("Os=0x%x Hardware=0x%x Bytes=%lu Info=0x%x",
    			  Os, Hardware, Bytes, Info);

#ifdef __QNXNTO__
	gctPHYS_ADDR physical;
	gctSIZE_T bytes;
#endif

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmVERIFY_ARGUMENT(Bytes > 0);
    gcmVERIFY_ARGUMENT(CommandBuffer != gcvNULL);

    /* Allocate the gcoCMDBUF object. */
#ifdef __QNXNTO__
    bytes = gcmSIZEOF(struct _gcoCMDBUF);
	gcmONERROR(
    	gcoOS_AllocateContiguous(Os,
								 gcvTRUE,
								 &bytes,
								 &physical,
								 (gctPOINTER *) &commandBuffer));
#else
   	gcmONERROR(
		gcoOS_Allocate(Os,
    				   gcmSIZEOF(struct _gcoCMDBUF),
					   (gctPOINTER *) &commandBuffer));
#endif

    /* Initialize the gcoCMDBUF object. */
    commandBuffer->object.type = gcvOBJ_COMMANDBUFFER;

    commandBuffer->os       = Os;
	commandBuffer->hardware = Hardware;
    commandBuffer->bytes    = Bytes;
	commandBuffer->logical  = gcvNULL;

    /* Allocate the physical buffer for the command. */
	gcmONERROR(
		gcoOS_AllocateContiguous(Os,
								 gcvTRUE,
								 &commandBuffer->bytes,
								 &commandBuffer->physical,
								 &commandBuffer->logical));

    /* Initialize command buffer. */
    commandBuffer->offset  = 0;
    commandBuffer->free    = commandBuffer->bytes;

#if gcdSECURE_USER
	/* Allocate memory for the map table. */
	gcmONERROR(
		gcoOS_Allocate(Os, Bytes, (gctPOINTER *) &commandBuffer->hintTable));
#endif

    /* Return pointer to the gcoCMDBUF object. */
    *CommandBuffer = commandBuffer;

    /* Success. */
    gcmFOOTER_ARG("*CommandBuffer=0x%x", *CommandBuffer);
    return gcvSTATUS_OK;

OnError:
	if (commandBuffer != gcvNULL)
	{
		if (commandBuffer->logical != gcvNULL)
		{
			/* Roll back the contiguous memory allocation. */
			gcmVERIFY_OK(
				gcoOS_FreeContiguous(Os,
									 commandBuffer->physical,
									 commandBuffer->logical,
									 commandBuffer->bytes));

#ifdef __QNXNTO__
			/* Roll back the command buffer allocation. */
	        gcmVERIFY_OK(
				gcoOS_FreeContiguous(Os,
									 gcvNULL,
									 commandBuffer,
									 gcmSIZEOF(struct _gcoCMDBUF)));
#endif
		}

		/* Roll back. */
        gcmVERIFY_OK(gcoOS_Free(Os, commandBuffer));
	}

	/* Return the status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoCMDBUF_Destroy
**
**	Destroy a gcoCMDBUF object.
**
**	INPUT:
**
**		gcoCMDBUF CommandBuffer
**			Pointer to an gcoCMDBUF object.
**
**	OUTPUT:
**
**		None.
*/
gceSTATUS
gcoCMDBUF_Destroy(
	IN gcoCMDBUF CommandBuffer
	)
{
    gceSTATUS status;
    gcsHAL_INTERFACE iface;

    gcmHEADER_ARG("CommandBuffer=0x%x", CommandBuffer);

	/* Verify the object. */
	gcmVERIFY_OBJECT(CommandBuffer, gcvOBJ_COMMANDBUFFER);

	/* Use events to free the buffer. */
	iface.command = gcvHAL_FREE_CONTIGUOUS_MEMORY;
	iface.u.FreeContiguousMemory.bytes    = CommandBuffer->bytes;
	iface.u.FreeContiguousMemory.physical = CommandBuffer->physical;
	iface.u.FreeContiguousMemory.logical  = CommandBuffer->logical;

	/* Send event. */
	gcmONERROR(
		gcoHARDWARE_CallEvent(CommandBuffer->hardware, &iface));

#if gcdSECURE_USER
	/* Free the map table. */
	gcmVERIFY_OK(gcoOS_Free(CommandBuffer->os, CommandBuffer->hintTable));
#endif

	/* Free the gcoCMDBUF object. */
#ifdef __QNXNTO__
    gcmVERIFY_OK(gcoOS_FreeContiguous(CommandBuffer->os,
									  gcvNULL,
									  CommandBuffer,
									  gcmSIZEOF(struct _gcoCMDBUF)));
#else
	gcmVERIFY_OK(gcoOS_Free(CommandBuffer->os, CommandBuffer));
#endif

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
	/* Return the status. */
	gcmFOOTER();
	return status;
}

/******************************************************************************\
******************************* gcoBUFFER API Code ******************************
\******************************************************************************/

gceSTATUS
_GetCMDBUF(
	IN gcoBUFFER Buffer
	)
{
	gcoCMDBUF command;
	gceSTATUS status;

	gcmHEADER_ARG("Buffer=0x%x", Buffer);

	/* Increment current command buffer index. */
	Buffer->currentCommandBufferIndex =
		(Buffer->currentCommandBuffer == gcvNULL)
			? 0
			: ((Buffer->currentCommandBufferIndex + 1) % gcdCMD_BUFFERS);
	Buffer->currentCommandBuffer =
		Buffer->commandBuffers[Buffer->currentCommandBufferIndex];

	/* Wait for buffer to become available. */
	gcmONERROR(
		gcoOS_WaitSignal(Buffer->os,
						 Buffer->signal[Buffer->currentCommandBufferIndex],
						 gcvINFINITE));

	/* Grab pointer to current command buffer. */
	command = Buffer->currentCommandBuffer;

	/* Reset command buffer. */
	command->startOffset = 0;
	command->offset      = Buffer->info.reservedHead;
	command->free        = command->bytes - Buffer->totalReserved;

#if gcdSECURE_USER
	/* Reset mapping table. */
	command->hintIndex  = command->hintTable;
	command->hintCommit = command->hintTable;
#endif

	/* Succees. */
	gcmFOOTER_ARG("currentCommandBufferIndex=%d",
				  Buffer->currentCommandBufferIndex);
	return gcvSTATUS_OK;

OnError:
	/* Return the status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoBUFFER_Construct
**
**	Construct a new gcoBUFFER object.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
**
**		gcoHARDWARE Hardware
**			Pointer to an gcoHARDWARE object.
**
**		gctSIZE_T MaxSize
**			Maximum size of buffer.
**
**	OUTPUT:
**
**		gcoBUFFER * Buffer
**			Pointer to a variable that will hold the the gcoBUFFER object
**			pointer.
*/
gceSTATUS
gcoBUFFER_Construct(
	IN gcoHAL Hal,
	IN gcoHARDWARE Hardware,
	IN gctSIZE_T MaxSize,
	OUT gcoBUFFER * Buffer
	)
{
	gcoBUFFER buffer = gcvNULL;
	gceSTATUS status;
	gctUINT i = 0;

	gcmHEADER_ARG("Hal=0x%x Hardware=0x%x MaxSize=%lu", Hal, Hardware, MaxSize);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Hal, gcvOBJ_HAL);
	gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
	gcmVERIFY_ARGUMENT(Buffer != gcvNULL);

	/* Allocate the gcoBUFFER object. */
	gcmONERROR(
		gcoOS_Allocate(Hal->os,
					   gcmSIZEOF(struct _gcoBUFFER),
					   (gctPOINTER *) &buffer));

	/* Initialize the gcoBUFFER object. */
	buffer->object.type = gcvOBJ_BUFFER;
	buffer->hal         = Hal;
	buffer->os          = Hal->os;
	buffer->hardware    = Hardware;

	/* Maximum size of buffer. */
	buffer->size    = 0;
	buffer->maxSize = MaxSize;

	/* Query alignment. */
	gcmONERROR(
		gcoHARDWARE_QueryCommandBuffer(Hardware,
    								   &buffer->info.alignment,
	                                   &buffer->info.reservedHead,
    	                               &buffer->info.reservedTail));

	buffer->totalReserved = buffer->info.reservedHead
						  + buffer->info.reservedTail
						  + buffer->info.alignment;

    /* Zero the command buffers. */
    for (i = 0; i < gcmCOUNTOF(buffer->commandBuffers); ++i)
    {
		buffer->commandBuffers[i]  = gcvNULL;
		buffer->signal[i]          = gcvNULL;
	}

	/* Initialize the command buffers. */
	for (i = 0; i < gcmCOUNTOF(buffer->commandBuffers); ++i)
	{
		/* Construct a command buffer. */
		gcmONERROR(
			gcoCMDBUF_Construct(buffer->os,
			   					buffer->hardware,
            					buffer->maxSize,
			            		&buffer->info,
								&buffer->commandBuffers[i]));

		/* Create the signal. */
		gcmONERROR(
			gcoOS_CreateSignal(buffer->os,
							   gcvFALSE,
							   &buffer->signal[i]));

		/* Mark the buffer as available. */
		gcmONERROR(
			gcoOS_Signal(buffer->os,
						 buffer->signal[i],
						 gcvTRUE));
	}

	/* Grab the first command buffer. */
	buffer->currentCommandBuffer = gcvNULL;
	gcmONERROR(_GetCMDBUF(buffer));

	/* Return pointer to the gcoBUFFER object. */
	*Buffer = buffer;

	/* Success. */
	gcmFOOTER_ARG("*Buffer=0x%x", *Buffer);
	return gcvSTATUS_OK;

OnError:
	if (buffer != gcvNULL)
	{
		/* Roll back all command buffers. */
		for (i = 0; i < gcmCOUNTOF(buffer->commandBuffers); ++i)
		{
			if (buffer->commandBuffers[i] != gcvNULL)
			{
				/* Destroy command buffer. */
				gcmVERIFY_OK(
					gcoCMDBUF_Destroy(buffer->commandBuffers[i]));
			}

			if (buffer->signal[i] != gcvNULL)
			{
				/* Destroy signal. */
				gcmVERIFY_OK(
					gcoOS_DestroySignal(Hal->os,
										buffer->signal[i]));
			}
		}

		/* Free the object memory. */
		gcmVERIFY_OK(gcoOS_Free(Hal->os, buffer));
	}

	/* Return the status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoBUFFER_Destroy
**
**	Destroy an gcoBUFFER object.
**
**	INPUT:
**
**		gcoBUFFER Buffer
**			Pointer to an gcoBUFFER object to delete.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoBUFFER_Destroy(
	IN gcoBUFFER Buffer
	)
{
	gctUINT i = 0;

	gcmHEADER_ARG("Buffer=0x%x", Buffer);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Buffer, gcvOBJ_BUFFER);

	/* Commit buffer before destroying it. */
	if (Buffer->size != 0)
	{
		/* Commit the command buffers. */
		gcmVERIFY_OK(gcoHARDWARE_Commit(Buffer->hardware));

		/* Stall the hardware. */
		gcmVERIFY_OK(gcoHARDWARE_Stall(Buffer->hardware));
	}

	/* Delete all allocated buffers. */
    for (i = 0; i < gcmCOUNTOF(Buffer->commandBuffers); ++i)
    {
		/* Destroy the command buffer. */
        gcmVERIFY_OK(
			gcoCMDBUF_Destroy(Buffer->commandBuffers[i]));

		/* Destroy the signal. */
        gcmVERIFY_OK(
			gcoOS_DestroySignal(Buffer->os, Buffer->signal[i]));
    }

	/* Free the gcoBUFFER object. */
	gcmVERIFY_OK(gcoOS_Free(Buffer->os, Buffer));

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoBUFFER_Write
**
**	Copy a number of bytes into the buffer.
**
**	INPUT:
**
**		gcoBUFFER Buffer
**			Pointer to an gcoBUFFER object.
**
**		gctCONST_POINTER Data
**			Pointer to a buffer that contains the data to be copied.
**
**		IN gctSIZE_T Bytes
**			Number of bytes to copy.
**
**		IN gctBOOL Aligned
**			gcvTRUE if the data needs to be aligned to 64-bit.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoBUFFER_Write(
	IN gcoBUFFER Buffer,
	IN gctCONST_POINTER Data,
	IN gctSIZE_T Bytes,
	IN gctBOOL Aligned
	)
{
	gceSTATUS status;
	gctPOINTER memory;

	gcmHEADER_ARG("Buffer=0x%x Data=0x%x Bytes=%lu Aligned=%d",
				  Buffer, Data, Bytes, Aligned);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Buffer, gcvOBJ_BUFFER);
	gcmVERIFY_ARGUMENT(Data != gcvNULL);
	gcmVERIFY_ARGUMENT(Bytes > 0);

	/* Reserve data in the buffer. */
	gcmONERROR(
		gcoBUFFER_Reserve(Buffer, Bytes, Aligned, gcvNULL, &memory));

	/* Write data into the buffer. */
	gcmONERROR(
		gcoOS_MemCopy(memory, Data, Bytes));

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
**	gcoBUFFER_Reserve
**
**	Reserve a number of bytes in the buffer.
**
**	INPUT:
**
**		gcoBUFFER Buffer
**			Pointer to an gcoBUFFER object.
**
**		gctSIZE_T Bytes
**			Number of bytes to reserve.
**
**		gctBOOL Aligned
**			gcvTRUE if the data needs to be aligned to 64-bit.
**
**		gctBOOL_PTR AddressHints
**			Optional pointer that points to an array of boolean values that
**			define whether the state of the specified 32-bit valueis a physical
**			address or not.
**
**	OUTPUT:
**
**		gctPOINTER * Memory
**			Pointer to a variable that will hold the address of location in the
**			buffer that has been reserved.
*/
gceSTATUS
gcoBUFFER_Reserve(
	IN gcoBUFFER Buffer,
	IN gctSIZE_T Bytes,
	IN gctBOOL Aligned,
	IN gctBOOL_PTR AddressHints,
	OUT gctPOINTER * Memory
	)
{
	gceSTATUS status;
	gcoCMDBUF current;
	gctSIZE_T alignBytes, bytes;

	gcmHEADER_ARG("Buffer=0x%x Bytes=%lu Aligned=%d AddressHints=0x%x",
				  Buffer, Bytes, Aligned, AddressHints);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Buffer, gcvOBJ_BUFFER);
	gcmVERIFY_ARGUMENT(Memory != gcvNULL);

	/* Get the current command buffer. */
	current = Buffer->currentCommandBuffer;

	/* Compute the number of aligned bytes. */
	alignBytes = Aligned
			   ? ( gcmALIGN(current->offset, Buffer->info.alignment)
			     - current->offset
				 )
			   : 0;

	/* Compute the number of required bytes. */
    bytes = Bytes + alignBytes;

	if (bytes > Buffer->maxSize - Buffer->totalReserved)
    {
		/* This just won't fit! */
		gcmFATAL("FATAL: Command of %lu bytes is too big!", Bytes);
		gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
	}

    if (bytes > current->free)
    {
		gcsHAL_INTERFACE iface;

        /* Sent event to signal when command buffer completes. */
        iface.command            = gcvHAL_SIGNAL;
		iface.u.Signal.signal	 = Buffer->signal
								   [Buffer->currentCommandBufferIndex];
		iface.u.Signal.auxSignal = gcvNULL;
		iface.u.Signal.process	 = Buffer->hal->process;
        iface.u.Signal.fromWhere = gcvKERNEL_COMMAND;

		/* Send event. */
		gcmONERROR(
			gcoHARDWARE_CallEvent(Buffer->hardware, &iface));

        /* Commit current command buffer. */
        gcmONERROR(
			gcoHARDWARE_Commit(Buffer->hardware));

		/* Grab a new command buffer. */
		gcmONERROR(
			_GetCMDBUF(Buffer));

		/* Get the pointer. */
        current = Buffer->currentCommandBuffer;

        /* Calculate total bytes again. */
        alignBytes = 0;
        bytes      = Bytes;
    }

	gcmASSERT(current != gcvNULL);
	gcmASSERT(bytes   <= current->free);

    /* Allocate space in current command buffer. */
	*Memory = (gctUINT8_PTR) current->logical + current->offset + alignBytes;

#if gcdSECURE_USER
	/* See if there is an address hint. */
	if (AddressHints != gcvNULL)
	{
		gctSIZE_T i;
		gctUINT32 offset = current->offset + alignBytes;

		/* Walk the address hint array. */
		for (i = 0;
			 i < Bytes / gcmSIZEOF(gctBOOL);
			 ++i, offset += gcmSIZEOF(gctUINT32)
		)
		{
			/* Check for a hinted address at the specified location. */
			if (AddressHints[i])
			{
				*current->hintIndex++ = offset;
			}
		}
	}
#endif

	/* Adjust command buffer size. */
	current->offset += bytes;
	current->free   -= bytes;

	/* Success. */
	gcmFOOTER_ARG("*Memory=0x%x", *Memory);
    return gcvSTATUS_OK;

OnError:
	/* Return the status. */
	gcmFOOTER();
	return status;
}

/*******************************************************************************
**
**	gcoBUFFER_Commit
**
**	Commit the command buffer to the hardware.
**
**	INPUT:
**
**		gcoBUFFER Buffer
**			Pointer to a gcoBUFFER object.
**
**		gcoCONTEXT Context
**			Pointer to a gcoCONTEXT object.
**
**		gcoQUEUE Queue
**			Pointer to a gcoQUEUE object.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoBUFFER_Commit(
    IN gcoBUFFER Buffer,
    IN gcoCONTEXT Context,
	IN gcoQUEUE Queue
    )
{
	gcsHAL_INTERFACE iface;
	gceSTATUS status;
	gcoCMDBUF current;

	gcmHEADER_ARG("Buffer=0x%x Context=0x%x Queue=0x%x", Buffer, Context, Queue);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Buffer, gcvOBJ_BUFFER);
    gcmVERIFY_OBJECT(Context, gcvOBJ_CONTEXT);
    gcmVERIFY_OBJECT(Queue, gcvOBJ_QUEUE);

	/* Grab current command buffer. */
	current = Buffer->currentCommandBuffer;

    if (current == gcvNULL)
    {
        /* No command buffer, done. */
        gcmFOOTER_NO();
		return gcvSTATUS_OK;
	}

	if (current->offset - current->startOffset <= Buffer->info.reservedHead)
    {
		/* Commit the event queue. */
		status = gcoQUEUE_Commit(Queue);
		gcmFOOTER();
		return status;
    }

	/* Make sure the tail got aligned properly. */
	current->offset = gcmALIGN(current->offset, Buffer->info.alignment);

	if (Buffer->hal->dump != gcvNULL)
	{
		/* Dump the command buffer. */
		gcmVERIFY_OK(
			gcoDUMP_DumpData(Buffer->hal->dump,
							 gcvTAG_COMMAND,
							 0,
							 current->offset
							 - current->startOffset
							 - Buffer->info.reservedHead,
							 (gctUINT8_PTR) current->logical
							 + current->startOffset
							 + Buffer->info.reservedHead));
	}

#if gcdSECURE_USER
	/* Mark the end of the map table. */
	*current->hintIndex++ = 0;
#endif

	/* Send command and context buffer to hardware. */
	iface.command = gcvHAL_COMMIT;
	iface.u.Commit.commandBuffer = current;
	iface.u.Commit.contextBuffer = Context;
    iface.u.Commit.process       = gcoOS_GetCurrentProcessID();

	/* Call kernel service. */
	gcmONERROR(
		gcoOS_DeviceControl(Buffer->os,
    						IOCTL_GCHAL_INTERFACE,
							&iface, gcmSIZEOF(iface),
							&iface, gcmSIZEOF(iface)));

	gcmONERROR(iface.status);

	/* Advance the offset for next commit. */
	current->startOffset = current->offset + Buffer->info.reservedTail;

	if (current->bytes - current->startOffset > Buffer->totalReserved)
	{
		/* Adjust buffer offset and size. */
		current->offset	= current->startOffset + Buffer->info.reservedHead;
		current->free	= current->bytes - current->offset
						- Buffer->info.alignment
						- Buffer->info.reservedTail;
	}
	else
	{
		/* Buffer is full. */
		current->startOffset = current->bytes;
		current->offset      = current->bytes;
		current->free        = 0;
	}

#if gcdSECURE_USER
	current->hintCommit = current->hintIndex;
#endif

	/* Commit the event queue. */
	gcmONERROR(gcoQUEUE_Commit(Queue));

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

