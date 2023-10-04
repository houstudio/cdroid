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
**	Architecture independent event queue management.
**
*/

#include "gc_hal_user_precomp.h"
#include "gc_hal_user_context.h"

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE	gcvZONE_BUFFER

gceSTATUS
gcoQUEUE_Construct(
	IN gcoOS Os,
	OUT gcoQUEUE * Queue
	)
{
	gcoQUEUE queue = gcvNULL;
	gceSTATUS status;

	gcmHEADER_ARG("Os=0x%x", Os);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
	gcmVERIFY_ARGUMENT(Queue != gcvNULL);

	/* Create the queue. */
	gcmONERROR(
		gcoOS_Allocate(Os,
					   gcmSIZEOF(struct _gcoQUEUE),
					   (gctPOINTER *) &queue));

	/* Initialize the object. */
	queue->object.type = gcvOBJ_QUEUE;
	queue->os          = Os;

	/* Nothing in the queue yet. */
	queue->head = queue->tail = gcvNULL;

	/* Return gcoQUEUE pointer. */
	*Queue = queue;

	/* Success. */
	gcmFOOTER_ARG("*Queue=0x%x", *Queue);
	return gcvSTATUS_OK;

OnError:
	if (queue != gcvNULL)
	{
		/* Roll back. */
		gcmVERIFY_OK(gcoOS_Free(Os, queue));
	}

	/* Return the status. */
	gcmFOOTER();
	return status;
}

gceSTATUS
gcoQUEUE_Destroy(
	IN gcoQUEUE Queue
	)
{
	gceSTATUS status;
	gcsQUEUE_PTR record;

	gcmHEADER_ARG("Queue=0x%x", Queue);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Queue, gcvOBJ_QUEUE);

	/* Free any records in the queue. */
	while (Queue->head != gcvNULL)
	{
		/* Unlink the first record from the queue. */
		record      = Queue->head;
		Queue->head = record->next;

#ifdef __QNXNTO__
		gcmONERROR(gcoOS_FreeNonPagedMemory(Queue->os,
											record->bytes,
											gcvNULL,
											record));
#else
		gcmONERROR(gcoOS_Free(Queue->os, record));
#endif
	}

	/* Free the queue. */
	gcmONERROR(gcoOS_Free(Queue->os, Queue));

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;

OnError:
	/* Return the status. */
	gcmFOOTER();
	return status;
}

gceSTATUS
gcoQUEUE_AppendEvent(
	IN gcoQUEUE Queue,
	IN gcsHAL_INTERFACE * Interface
	)
{
	gceSTATUS status;
	gcsQUEUE_PTR record = gcvNULL;
#ifdef __QNXNTO__
	gctSIZE_T allocationSize;
	gctPHYS_ADDR physAddr;
#endif

	gcmHEADER_ARG("Queue=0x%x Interface=0x%x", Queue, Interface);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Queue, gcvOBJ_QUEUE);
	gcmVERIFY_ARGUMENT(Interface != gcvNULL);

	/* Allocate record. */
#ifdef __QNXNTO__
	allocationSize = gcmSIZEOF(gcsQUEUE);
	gcmONERROR(
		gcoOS_AllocateNonPagedMemory(Queue->os,
									 gcvTRUE,
									 &allocationSize,
									 &physAddr,
									 (gctPOINTER *) &record));
#else
	gcmONERROR(
		gcoOS_Allocate(Queue->os,
					   gcmSIZEOF(gcsQUEUE),
					   (gctPOINTER *) &record));
#endif

	/* Initialize record. */
	record->next  = gcvNULL;
	gcoOS_MemCopy(&record->iface, Interface, gcmSIZEOF(record->iface));

#ifdef __QNXNTO__
		record->bytes = allocationSize;
#endif

	if (Queue->head == gcvNULL)
	{
		/* Initialize queue. */
		Queue->head = record;
	}
	else
	{
		/* Append record to end of queue. */
		Queue->tail->next = record;
	}

	/* Mark end of queue. */
	Queue->tail = record;

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;

OnError:
	if (record != gcvNULL)
	{
		/* Roll back. */
#ifdef __QNXNTO__
		gcmVERIFY_OK(gcoOS_FreeNonPagedMemory(Queue->os,
											  record->bytes,
											  gcvNULL,
											  record));
#else
		gcmVERIFY_OK(gcoOS_Free(Queue->os, record));
#endif
	}

	/* Return the status. */
	gcmFOOTER();
	return status;
}

gceSTATUS
gcoQUEUE_Commit(
	IN gcoQUEUE Queue
	)
{
	gceSTATUS status = gcvSTATUS_OK;
	gcsQUEUE_PTR record;
	gcsHAL_INTERFACE iface;

	gcmHEADER_ARG("Queue=0x%x", Queue);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Queue, gcvOBJ_QUEUE);

	/* Initialize event commit command. */
	iface.command       = gcvHAL_EVENT_COMMIT;
	iface.u.Event.queue = Queue->head;

	/* Send command to kernel. */
	gcmONERROR(
		gcoOS_DeviceControl(Queue->os,
							IOCTL_GCHAL_INTERFACE,
							&iface, gcmSIZEOF(iface),
							&iface, gcmSIZEOF(iface)));

	/* Test for error. */
	gcmONERROR(iface.status);

	/* Free any records in the queue. */
	while (Queue->head != gcvNULL)
	{
		/* Unlink the first record from the queue. */
		record      = Queue->head;
		Queue->head = record->next;

#ifdef __QNXNTO__
		gcmONERROR(gcoOS_FreeNonPagedMemory(Queue->os,
											  record->bytes,
											  gcvNULL,
											  record));
#else
		gcmONERROR(gcoOS_Free(Queue->os, record));
#endif
	}

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;

OnError:
	/* Return the status. */
	gcmFOOTER();
	return status;
}

