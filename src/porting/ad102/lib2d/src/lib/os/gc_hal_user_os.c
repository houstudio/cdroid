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
**	OS object for hal user layers.
**
*/

#include "gc_hal_user_linux.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <string.h>

#if USE_NEW_LINUX_SIGNAL
#include <signal.h>
#include <errno.h>
#include "gc_hal_user_context.h"
#endif

#define GAL_DEV	"/dev/galcore"
#define _GC_OBJ_ZONE	gcvZONE_OS

#ifdef LINUX_OABI
gctINT32 __sync_fetch_and_add_4(gcsATOM_PTR Atom, gctINT32 Value);
gctINT32 __sync_fetch_and_sub_4(gcsATOM_PTR Atom, gctINT32 Value);

#define __sync_fetch_and_add 	__sync_fetch_and_add_4
#define __sync_fetch_and_sub 	__sync_fetch_and_sub_4
#endif

/******************************************************************************\
***************************** gcoOS Object Structure ***************************
\******************************************************************************/

#if USE_NEW_LINUX_SIGNAL
typedef struct _gcsSIGNAL *			gcsSIGNAL_PTR;
typedef struct _gcsSIGNAL_RECORD *	gcsSIGNAL_RECORD_PTR;

typedef struct _gcsSIGNAL_RECORD
{
	/* Pointer to next signal. */
	gcsSIGNAL_RECORD_PTR next;

	/* Signal. */
	gcsSIGNAL_PTR signal;
}
gcsSIGNAL_RECORD;
#endif

typedef struct _gcsDRIVER_ARGS
{
	gctPOINTER InputBuffer;
	gctUINT32  InputBufferSize;
	gctPOINTER OutputBuffer;
	gctUINT32  OutputBufferSize;
}
gcsDRIVER_ARGS;

struct _gcoOS
{
	/* Object. */
	gcsOBJECT object;

	/* Context. */
	gctPOINTER context;

	/* Handle to the device. */
	int device;

	/* Heap. */
	gcoHEAP heap;

	/* Base address. */
	gctUINT32 baseAddress;

#if USE_NEW_LINUX_SIGNAL
	/* Signal. */
	gctINT signal;
	pthread_t signalThread;
	struct _gcsOS_QUEUE
	{
		/* Pointer to head of queue. */
		gcsSIGNAL_RECORD_PTR head;

		/* Pointer to tail of queue. */
		gcsSIGNAL_RECORD_PTR tail;

		/* List of free gcsSIGNAL_RECORD structures. */
		volatile gcsSIGNAL_RECORD_PTR free;

		/* Mutex and condition. */
		pthread_mutex_t mutex;
		pthread_cond_t condition;

		/* Mutex and condition for free memory. */
		pthread_mutex_t mutexFree;
		pthread_cond_t conditionFree;

		/* Memory allocation. */
		gcsSIGNAL_RECORD memory[8];
	}
	queue;
#endif
};

#if USE_NEW_LINUX_SIGNAL
/*******************************************************************************
**	Signal management.
**
**	Linux signals are a pain in the you-know-where.  This is how it works:
**
**	The kernel sends a real-time signal (so it is queued and won't be lost) to
**	the thread who originated the request (the main thread).  The main thread
**	get interrupted and the signal handler is called.  The signal handler will
**	append this signal to the signal queue and signals the signal thread of an
**	addition to the signal queue.
**
**	The signal thread will walk the signal queue and process all signals
**	asynchroneously.  If the signal queue is empty, it will wait for the signal
**	handler to inform it of an addition.
**
**	A seperate thread is required because we can have a deadlock in the
**	pthread_mutex required for conditions.  In order to broadcast the signal, we
**	need to lock the mutex - but what is is locked by the main thread already?
**	Since the main thread get sinterrupted - and won't resume until the signal
**	handler is finished - we have a nice deadlock in our hands.  Hence the extra
**	thread.
**
**	One caveat is the memory management.  The signal handler needs memory for the
**	signal queue and calling gcsOS_AllocateMemory is not a good idea (the main
**	thread might be doing the same thing while interrupted), so we pre-allocate a
**	number of records.  If those records become full, we have to wait for the
**	signal thread to at least handle one of those signals.  As long as the signal
**	thread keeps looping while there are valid entries in the signal queue - we
**	don't have a problem.
*/

typedef struct _gcsSIGNAL
{
	/* Pointer to gcoOS object. */
	gcoOS			os;

	/* Signaled state. */
	gctBOOL			state;

	/* Manual reset flag. */
	gctBOOL			manual;

	/* Mutex. */
	pthread_mutex_t	mutex;

	/* Condition. */
	pthread_cond_t	condition;

	/* Number of signals pending in the command queue. */
	gctINT			pending;

	/* Number of signals received. */
	gctINT			received;
}
gcsSIGNAL;

/* Signal handler. */
static void
_SignalHandler(
	int Signal,
	siginfo_t * Info,
	void * Argument
	)
{
	gcsSIGNAL * signal = (gcsSIGNAL *) Info->si_ptr;
	gcoOS os = signal->os;
	gcsSIGNAL_RECORD_PTR queue;

	/* Lock the mutex. */
	if (pthread_mutex_lock(&os->queue.mutex))
	{
		gcmTRACE(gcvLEVEL_ERROR,
				 "%s(%d): pthread_mutex_lock failed.",
				 __FUNCTION__, __LINE__);
	}

	/* Wait until there are free queue records available. */
	while (os->queue.free == gcvNULL)
	{
		/* Release the queue mutex. */
		if (pthread_mutex_unlock(&os->queue.mutex))
		{
			gcmTRACE(gcvLEVEL_ERROR,
					 "%s(%d): pthread_mutex_unlock failed.",
					 __FUNCTION__, __LINE__);
		}

		/* Wait for the free condition variable. */
		if (pthread_mutex_lock(&os->queue.mutexFree))
		{
			gcmTRACE(gcvLEVEL_ERROR,
					 "%s(%d): pthread_mutex_lock failed.",
					 __FUNCTION__, __LINE__);
		}

		if (pthread_cond_wait(&os->queue.conditionFree, &os->queue.mutexFree))
		{
			gcmTRACE(gcvLEVEL_ERROR,
					 "%s(%d): pthread_cond_wait failed.",
					 __FUNCTION__, __LINE__);
		}

		if (pthread_mutex_unlock(&os->queue.mutexFree))
		{
			gcmTRACE(gcvLEVEL_ERROR,
					 "%s(%d): pthread_mutex_unlock failed.",
					 __FUNCTION__, __LINE__);
		}

		/* Lock the queue mutex. */
		if (pthread_mutex_lock(&os->queue.mutex))
		{
			gcmTRACE(gcvLEVEL_ERROR,
					 "%s(%d): pthread_mutex_lock failed.",
					 __FUNCTION__, __LINE__);
		}
	}

	/* Grab a queue record from the free list. */
	queue          = os->queue.free;
	os->queue.free = queue->next;

	/* Initialize the queue record. */
	queue->next   = gcvNULL;
	queue->signal = signal;

	/* Push it to the tail of the queue. */
	if (os->queue.head == gcvNULL)
	{
		os->queue.head = queue;
		os->queue.tail = queue;
	}
	else
	{
		os->queue.tail->next = queue;
		os->queue.tail       = queue;
	}

	/* Signal the thread. */
	if (pthread_cond_signal(&os->queue.condition))
	{
		gcmTRACE(gcvLEVEL_ERROR,
				 "%s(%d): pthread_cond_signal failed.",
				 __FUNCTION__, __LINE__);
	}

	/* Release the mutex. */
	if (pthread_mutex_unlock(&os->queue.mutex))
	{
		gcmTRACE(gcvLEVEL_ERROR,
				 "%s(%d): pthread_mutex_unlock failed.",
				 __FUNCTION__, __LINE__);
	}
}

/* Signal thread. */
static void *
_SignalThread(
	void * Argument
	)
{
	gcoOS os = (gcoOS) Argument;
	gcsSIGNAL_RECORD_PTR queue;

	/* Loop forever. */
	while (os->signal >= 0)
	{
		/* Lock the mutex. */
		if (pthread_mutex_lock(&os->queue.mutex))
		{
			gcmTRACE(gcvLEVEL_ERROR,
					 "%s(%d): pthread_mutex_lock failed.",
					 __FUNCTION__, __LINE__);
		}

		if (os->queue.head == gcvNULL)
		{
			/* Wait for the condition. */
			if (pthread_cond_wait(&os->queue.condition, &os->queue.mutex))
			{
				gcmTRACE(gcvLEVEL_ERROR,
						 "%s(%d): pthread_cond_wait failed.",
						 __FUNCTION__, __LINE__);
			}
		}

		/* See if we are asked to close the thread. */
		if (os->signal < 0)
		{
			/* Release the mutex. */
			if (pthread_mutex_unlock(&os->queue.mutex))
			{
				gcmTRACE(gcvLEVEL_ERROR,
						 "%s(%d): pthread_mutex_unlock failed.",
						 __FUNCTION__, __LINE__);
			}

			break;
		}

		if (os->queue.head != gcvNULL)
		{
			/* Pop the record from the head of the queue. */
			queue          = os->queue.head;
			os->queue.head = queue->next;

			/* Mark signal as received. */
			++ queue->signal->received;

			/* Set the signal. */
			gcmVERIFY_OK(
				gcoOS_Signal(os, queue->signal, gcvTRUE));

			/* Insert this queue record to the free list. */
			queue->next    = os->queue.free;
			os->queue.free = queue;
		}

		/* Release the mutex. */
		if (pthread_mutex_unlock(&os->queue.mutex))
		{
			gcmTRACE(gcvLEVEL_ERROR,
					 "%s(%d): pthread_mutex_unlock failed.",
					 __FUNCTION__, __LINE__);
		}

		/* Signal the handler there is a free record. */
		if (pthread_mutex_lock(&os->queue.mutexFree))
		{
			gcmTRACE(gcvLEVEL_ERROR,
					 "%s(%d): pthread_mutex_lock failed.",
					 __FUNCTION__, __LINE__);
		}

		if (pthread_cond_signal(&os->queue.conditionFree))
		{
			gcmTRACE(gcvLEVEL_ERROR,
					 "%s(%d): pthread_cond_signal failed.",
					 __FUNCTION__, __LINE__);
		}

		if (pthread_mutex_unlock(&os->queue.mutexFree))
		{
			gcmTRACE(gcvLEVEL_ERROR,
					 "%s(%d): pthread_mutex_unlock failed.",
					 __FUNCTION__, __LINE__);
		}
	}

	/* Return. */
	return gcvNULL;
}
#endif

/******************************************************************************\
 ********************************* gcoOS API Code ********************************
 \******************************************************************************/

static int g_Device    = -1;
static int g_DeviceRef =  0;

/*******************************************************************************
 **
 **	gcoOS_Construct
 **
 **	Construct a new gcoOS object.
 **
 **	INPUT:
 **
 **		gctPOINTER Context
 **			Pointer to an OS specific context.
 **
 **	OUTPUT:
 **
 **		gcoOS * Os
 **			Pointer to a variable that will hold the pointer to the gcoOS object.
 */
gceSTATUS
gcoOS_Construct(
	IN gctPOINTER Context,
	OUT gcoOS *Os
	)
{
    gcoOS os;
	gcsHAL_INTERFACE iface;
	gceSTATUS status;
#if USE_NEW_LINUX_SIGNAL
	struct sigaction action;
	gctSIZE_T i;
#endif

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(Os != NULL);

    /* Allocate the gcoOS structure. */
    os = malloc(sizeof(struct _gcoOS));

    if (os == NULL)
    {
        /* Out of memory. */
        return gcvSTATUS_OUT_OF_MEMORY;
    }

    /* Initialize teh gcoOS object. */
    os->object.type = gcvOBJ_OS;

    /* Save pointer to context. */
    os->context = Context;
    os->heap    = gcvNULL;

	if (g_Device < 0)
	{
		/* Create a handle to the device. */
		g_Device = open(GAL_DEV, O_RDWR);

		if (g_Device < 0)
		{
			gcmTRACE(gcvLEVEL_ERROR,
					 "%s(%d): open failed.",
					 __FUNCTION__, __LINE__);

			os->object.type = gcvOBJ_UNKNOWN;
			free(os);

			return gcvSTATUS_OUT_OF_RESOURCES;
		}
	}

	os->device = g_Device;
	++g_DeviceRef;

    /* Construct heap. */
    if (gcmIS_ERROR(gcoHEAP_Construct(os, gcdHEAP_SIZE, &os->heap)))
    {
    	gcmTRACE_ZONE(gcvZONE_OS, gcvLEVEL_WARNING,
    	    	      "gcoOS_Construct: Could not construct gcoHEAP.");
    	os->heap = gcvNULL;
    }
#if VIVANTE_PROFILER
    else
    {
    	/* Start profiler. */
    	gcoHEAP_ProfileStart(os->heap);
    }
#endif

#if USE_NEW_LINUX_SIGNAL
	/* Query the signal. */
	iface.command = gcvHAL_QUERY_KERNEL_SETTINGS;
	status = gcoOS_DeviceControl(os,
								 IOCTL_GCHAL_INTERFACE,
								 &iface, gcmSIZEOF(iface),
								 &iface, gcmSIZEOF(iface));

	if (gcmIS_ERROR(status))
	{
		gcmTRACE(gcvLEVEL_ERROR, "%s(%d): QueryKernelSettings failed.",
				 __FUNCTION__, __LINE__);

		goto OnError;
	}

	/* Save the signal. */
	os->signal = iface.u.QueryKernelSettings.settings.signal;

	/* Initialize the free queue. */
	os->queue.head = gcvNULL;
	os->queue.free = os->queue.memory;
	for (i = 1; i < gcmCOUNTOF(os->queue.memory); ++i)
	{
		os->queue.memory[i - 1].next = &os->queue.memory[i];
	}

	/* Create the queue. */
	if (pthread_mutex_init(&os->queue.mutex, gcvNULL))
	{
		gcmTRACE(gcvLEVEL_ERROR,
				 "%s(%d): pthread_mutex_init failed.",
				 __FUNCTION__, __LINE__);

		goto OnError;
	}

	if (pthread_cond_init(&os->queue.condition, gcvNULL))
	{
		gcmTRACE(gcvLEVEL_ERROR,
				 "%s(%d): pthread_cond_init failed.",
				 __FUNCTION__, __LINE__);

		goto OnError;
	}

	/* Create the free mutex and condition variable. */
	if (pthread_mutex_init(&os->queue.mutexFree, gcvNULL))
	{
		gcmTRACE(gcvLEVEL_ERROR,
				 "%s(%d): pthread_mutex_init failed.",
				 __FUNCTION__, __LINE__);

		goto OnError;
	}

	if (pthread_cond_init(&os->queue.conditionFree, gcvNULL))
	{
		gcmTRACE(gcvLEVEL_ERROR,
				 "%s(%d): pthread_cond_init failed.",
				 __FUNCTION__, __LINE__);

		goto OnError;
	}

	/* Set action. */
	sigemptyset(&action.sa_mask);
	sigaddset(&action.sa_mask, os->signal);
	action.sa_flags     = SA_SIGINFO | SA_RESTART;
	action.sa_sigaction = _SignalHandler;
	sigaction(os->signal, &action, gcvNULL);

	/* Create signal thread. */
	if (pthread_create(&os->signalThread, gcvNULL, _SignalThread, os))
	{
		gcmTRACE(gcvLEVEL_ERROR,
				 "%s(%d): pthread_create failed.",
				 __FUNCTION__, __LINE__);

		goto OnError;
	}
#endif

	/* Query base address. */
	iface.command = gcvHAL_GET_BASE_ADDRESS;

    /* Call kernel driver. */
    status = gcoOS_DeviceControl(os,
								 IOCTL_GCHAL_INTERFACE,
								 &iface, gcmSIZEOF(iface),
								 &iface, gcmSIZEOF(iface));

	if (gcmIS_SUCCESS(status))
	{
		os->baseAddress = iface.u.GetBaseAddress.baseAddress;

		gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
					  "gcoOS_Construct: baseAddress is 0x%08X.",
					  os->baseAddress);
	}
	else
	{
		gcmTRACE_ZONE(gcvLEVEL_WARNING, gcvZONE_OS,
					  "gcoOS_Construct: Setting default baseAddress of 0.");

		os->baseAddress = 0;
	}

	/* Return pointer to the gcoOS object. */
    *Os = os;

	gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
				  "Successfully opened %s devId->%d",
				  gcvHAL_CLASS,
				  os->device);

    /* Success. */
    return gcvSTATUS_OK;

#if USE_NEW_LINUX_SIGNAL
OnError:
	/* Destroy the gcoOS structure. */
	gcmVERIFY_OK(gcoOS_Destroy(os));

	/* Return the status. */
	return status;
#endif
}

/*******************************************************************************
 **
 **	gcoOS_Destroy
 **
 **	Destroys an gcoOS object.
 **
 **	ARGUMENTS:
 **
 **		gcoOS Os
 **			Pointer to the gcoOS object that needs to be destroyed.
 **
 **	OUTPUT:
 **
 **		Nothing.
 **
 */
gceSTATUS
gcoOS_Destroy(
	IN gcoOS Os
	)
{
    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Os, gcvOBJ_OS);

#if USE_NEW_LINUX_SIGNAL
	/* Kill the signal thread. */
	if (Os->signalThread != 0)
	{
		/* Mark the signal thread to terminate. */
		Os->signal = -1;

		/* Send condition to signal thread. */
		pthread_mutex_lock(&Os->queue.mutex);
		pthread_cond_signal(&Os->queue.condition);
		pthread_mutex_unlock(&Os->queue.mutex);

		/* Wait for the signal thread to terminate. */
		pthread_join(Os->signalThread, gcvNULL);

		/* Destroy the queue mutex and condition. */
		pthread_mutex_destroy(&Os->queue.mutex);
		pthread_cond_destroy(&Os->queue.condition);

		/* Destroy the free mutex and condition. */
		pthread_mutex_destroy(&Os->queue.mutexFree);
		pthread_cond_destroy(&Os->queue.conditionFree);
	}
#endif

    if (Os->heap != gcvNULL)
    {
    	gcoHEAP heap = Os->heap;

#if VIVANTE_PROFILER
    	/* End profiler. */
    	gcoHEAP_ProfileEnd(heap, "gcoOS_HEAP");
#endif

    	/* Mark the heap as gone. */
    	Os->heap = gcvNULL;

    	/* Destroy the heap. */
    	gcmVERIFY_OK(gcoHEAP_Destroy(heap));
    }

    /* Close the handle to the kernel service. */
	if (--g_DeviceRef == 0)
	{
		close(g_Device);

		g_Device = -1;
	}

    /* Mark the gcoOS object as unknown. */
    Os->object.type = gcvOBJ_UNKNOWN;

    /* Free the gcoOS structure. */
    free(Os);

    /* Success. */
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoOS_QueryVideoMemory
**
**	Query the amount of video memory.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to a gcoOS object.
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
gcoOS_QueryVideoMemory(
	IN gcoOS Os,
	OUT gctPHYS_ADDR * InternalAddress,
	OUT gctSIZE_T * InternalSize,
	OUT gctPHYS_ADDR * ExternalAddress,
	OUT gctSIZE_T * ExternalSize,
	OUT gctPHYS_ADDR * ContiguousAddress,
	OUT gctSIZE_T * ContiguousSize
	)
{
	gceSTATUS status;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);

	do
	{
		gcsHAL_INTERFACE iface;

		/* Call kernel HAL to query video memory. */
		iface.command = gcvHAL_QUERY_VIDEO_MEMORY;

		/* Call kernel service. */
		gcmERR_BREAK(
			gcoOS_DeviceControl(Os,
								IOCTL_GCHAL_INTERFACE,
								&iface, sizeof(iface),
								&iface, sizeof(iface)));

		/* Verify the status. */
		gcmERR_BREAK(iface.status);

		if (InternalAddress != gcvNULL)
		{
			/* Verify arguments. */
			gcmVERIFY_ARGUMENT(InternalSize != gcvNULL);

			/* Save internal memory size. */
			*InternalAddress = iface.u.QueryVideoMemory.internalPhysical;
			*InternalSize    = iface.u.QueryVideoMemory.internalSize;
		}

		if (ExternalAddress != gcvNULL)
		{
			/* Verify arguments. */
			gcmVERIFY_ARGUMENT(ExternalSize != gcvNULL);

			/* Save external memory size. */
			*ExternalAddress = iface.u.QueryVideoMemory.externalPhysical;
			*ExternalSize    = iface.u.QueryVideoMemory.externalSize;
		}

		if (ContiguousAddress != gcvNULL)
		{
			/* Verify arguments. */
			gcmVERIFY_ARGUMENT(ContiguousSize != gcvNULL);

			/* Save contiguous memory size. */
			*ContiguousAddress = iface.u.QueryVideoMemory.contiguousPhysical;
			*ContiguousSize    = iface.u.QueryVideoMemory.contiguousSize;
		}
	}
	while (gcvFALSE);

	/* Return status. */
	return status;
}

/*******************************************************************************
**
**	gcoOS_GetBaseAddress
**
**	Get the base address for the physical memory.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to the gcoOS object.
**
**	OUTPUT:
**
**		gctUINT32_PTR BaseAddress
**			Pointer to a variable that will receive the base address.
*/
gceSTATUS
gcoOS_GetBaseAddress(
	IN gcoOS Os,
	OUT gctUINT32_PTR BaseAddress
	)
{
	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
	gcmVERIFY_ARGUMENT(BaseAddress != gcvNULL);

	/* Return base address. */
	*BaseAddress = Os->baseAddress;

	/* Success. */
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoOS_Allocate
**
**	Allocate memory.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**		gctSIZE_T Bytes
**			Number of bytes to allocate.
**
**	OUTPUT:
**
**		gctPOINTER * Memory
**			Pointer to a variable that will hold the pointer to the memory
**			allocation.
*/
gceSTATUS
gcoOS_Allocate(
    IN gcoOS Os,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Memory
    )
{
	/* Special wrapper when Os is gcvNULL. */
	if (Os == gcvNULL)
	{
		if (Bytes == 0)
		{
			return gcvSTATUS_INVALID_ARGUMENT;
		}

		*Memory = malloc(Bytes);
		if (*Memory == gcvNULL)
		{
			return gcvSTATUS_OUT_OF_MEMORY;
		}

		return gcvSTATUS_OK;
	}

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmVERIFY_ARGUMENT(Bytes > 0);
    gcmVERIFY_ARGUMENT(Memory != gcvNULL);

    return (Os->heap != gcvNULL)
	   ? gcoHEAP_Allocate(Os->heap, Bytes, Memory)
	   : gcoOS_AllocateMemory(Os, Bytes, Memory);
}

/*******************************************************************************
 **
 **	gcoOS_AllocateMemory
 **
 **	Allocate memory from the user heap.
 **
 **	INPUT:
 **
 **		gcoOS Os
 **			Pointer to an gcoOS object.
 **
 **		gctSIZE_T Bytes
 **			Number of bytes to allocate.
 **
 **	OUTPUT:
 **
 **		gctPOINTER * Memory
 **			Pointer to a variable that will hold the pointer to the memory
 **			allocation.
 */
gceSTATUS
gcoOS_AllocateMemory(
    IN gcoOS Os,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Memory
    )
{
    gctPOINTER memory;

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmVERIFY_ARGUMENT(Bytes > 0);
    gcmVERIFY_ARGUMENT(Memory != NULL);

    /* Allocate the memory. */
    memory = malloc(Bytes);

    if (memory == NULL)
    {
        /* Out of memory. */
        return gcvSTATUS_OUT_OF_MEMORY;
    }

    /* Return pointer to the memory allocation. */
    *Memory = memory;

    gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
    	  "gcoOS_AllocateMemory: Memory allocated->%p:0x%X bytes",
		  memory,
		  (gctUINT32) Bytes);

    /* Success. */
    return gcvSTATUS_OK;
}

/*******************************************************************************
 **
 **	gcoOS_Free
 **
 **	Free allocated memory from the user heap.
 **
 **	INPUT:
 **
 **		gcoOS Os
 **			Pointer to an gcoOS object.
 **
 **		gctPOINTER Memory
 **			Pointer to the memory allocation that needs to be freed.
 **
 **	OUTPUT:
 **
 **		Nothing.
 */
gceSTATUS
gcoOS_Free(
    IN gcoOS Os,
    IN gctPOINTER Memory
    )
{
	/* Special wrapper when Os is gcvNULL. */
	if (Os == gcvNULL)
	{
		if (Memory == gcvNULL)
		{
			return gcvSTATUS_INVALID_ARGUMENT;
		}

		free(Memory);

		return gcvSTATUS_OK;
	}

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmVERIFY_ARGUMENT(Memory != gcvNULL);

    return (Os->heap != gcvNULL)
    	? gcoHEAP_Free(Os->heap, Memory)
    	: gcoOS_FreeMemory(Os, Memory);
}

/*******************************************************************************
 **
 **	gcoOS_FreeMemory
 **
 **	Free allocated memory from the user heap.
 **
 **	INPUT:
 **
 **		gcoOS Os
 **			Pointer to an gcoOS object.
 **
 **		gctPOINTER Memory
 **			Pointer to the memory allocation that needs to be freed.
 **
 **	OUTPUT:
 **
 **		Nothing.
 */
gceSTATUS
gcoOS_FreeMemory(
    IN gcoOS Os,
    IN gctPOINTER Memory
    )
{
    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmVERIFY_ARGUMENT(Memory != NULL);

    /* Free the memory allocation. */
    free(Memory);

    /* Success. */
    return gcvSTATUS_OK;
}

/*******************************************************************************
 **
 **	gcoOS_DeviceControl
 **
 **	Perform a device I/O control call to the kernel API.
 **
 **	INPUT:
 **
 **		gcoOS Os
 **			Pointer to an gcoOS object.
 **
 **		gctUINT32 IoControlCode
 **			I/O control code to execute.
 **
 **		gctPOINTER InputBuffer
 **			Pointer to the input buffer.
 **
 **		gctSIZE_T InputBufferSize
 **			Size of the input buffer in bytes.
 **
 **		gctSIZE_T outputBufferSize
 **			Size of the output buffer in bytes.
 **
 **	OUTPUT:
 **
 **		gctPOINTER OutputBuffer
 **			Output buffer is filled with the data returned from the kernel HAL
 **			layer.
 */
gceSTATUS gcoOS_DeviceControl(
        IN gcoOS Os,
        IN gctUINT32 IoControlCode,
        IN gctPOINTER InputBuffer,
        IN gctSIZE_T InputBufferSize,
        OUT gctPOINTER OutputBuffer,
        IN gctSIZE_T OutputBufferSize
        )
{
	gcsDRIVER_ARGS args =
	{
		InputBuffer, InputBufferSize,
		OutputBuffer, OutputBufferSize
	};

	gcsHAL_INTERFACE_PTR iface = (gcsHAL_INTERFACE_PTR) InputBuffer;

#if USE_NEW_LINUX_SIGNAL
	gcsQUEUE_PTR queue;
#endif

	switch (iface->command)
	{
    case gcvHAL_MAP_MEMORY:
    	iface->u.MapMemory.logical = mmap(NULL,
        		    	    	    	  iface->u.MapMemory.bytes,
					      				  PROT_READ | PROT_WRITE,
					      				  MAP_SHARED,
					      				  Os->device,
					      				  (off_t) iface->u.MapMemory.physical);

    	if (iface->u.MapMemory.logical != MAP_FAILED)
		{
    	    return (iface->status = gcvSTATUS_OK);
    	}
		break;

	case gcvHAL_UNMAP_MEMORY:
    	munmap(iface->u.UnmapMemory.logical,
	       	   iface->u.UnmapMemory.bytes);

		return (iface->status = gcvSTATUS_OK);

#if USE_NEW_LINUX_SIGNAL
	case gcvHAL_SIGNAL:
		++ ((gcsSIGNAL_PTR) iface->u.Signal.signal)->pending;
		break;

	case gcvHAL_EVENT_COMMIT:
	/* Walk the event queue. */
		for (queue = iface->u.Event.queue; queue != gcvNULL; queue = queue->next)
		{
			/* Test for signal event. */
			if (queue->iface.command == gcvHAL_SIGNAL)
			{
				++ ((gcsSIGNAL_PTR) queue->iface.u.Signal.signal)->pending;
			}
		}
		break;
#endif /* USE_NEW_LINUX_SIGNAL */

	default:
		break;
	}

    if (ioctl(Os->device, IoControlCode, &args) < 0)
    {
		gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
        			  "%s(%d): ioctl failed.",
					  __FUNCTION__, __LINE__);

        /* Generic I/O error. */
        return gcvSTATUS_GENERIC_IO;
    }

    /* Success. */
    return ((gcsHAL_INTERFACE_PTR) OutputBuffer)->status;
}

/*******************************************************************************
 **
 **	gcoOS_AllocateNonPagedMemory
 **
 **	Allocate non-paged memory from the kernel.
 **
 **	INPUT:
 **
 **		gcoOS Os
 **			Pointer to an gcoOS object.
 **
 **      gctBOOL InUserSpace
 **          TRUE to mape the memory into the user space.
 **
 **      gctSIZE_T * Bytes
 **          Pointer to the number of bytes to allocate.
 **
 **	OUTPUT:
 **
 **		gctSIZE_T * Bytes
 **			Pointer to a variable that will receive the aligned number of bytes
 **          allocated.
 **
 **		gctPHYS_ADDR * Physical
 **			Pointer to a variable that will receive the physical addresses of
 **          the allocated pages.
 **
 **		gctPOINTER * Logical
 **			Pointer to a variable that will receive the logical address of the
 **			allocation.
 */
gceSTATUS gcoOS_AllocateNonPagedMemory(
        IN gcoOS Os,
        IN gctBOOL InUserSpace,
        IN OUT gctSIZE_T * Bytes,
        OUT gctPHYS_ADDR * Physical,
        OUT gctPOINTER * Logical
        )
{
    gcsHAL_INTERFACE iface;
    gceSTATUS status;

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmVERIFY_ARGUMENT(Bytes != NULL);
    gcmVERIFY_ARGUMENT(Physical != NULL);
    gcmVERIFY_ARGUMENT(Logical != NULL);

    /* Initialize the gcsHAL_INTERFACE structure. */
    iface.command = gcvHAL_ALLOCATE_NON_PAGED_MEMORY;
    iface.u.AllocateNonPagedMemory.bytes = *Bytes;

    /* Call kernel driver. */
    status = gcoOS_DeviceControl(Os,
				IOCTL_GCHAL_INTERFACE,
				&iface,
            	sizeof(iface),
				&iface,
            	sizeof(iface));

    if (status == gcvSTATUS_OK)
    {
        /* Get status from kernel. */
        status = iface.status;

        if (status == gcvSTATUS_OK)
        {
            /* Return allocated number of bytes. */
            *Bytes = iface.u.AllocateNonPagedMemory.bytes;

            /* Return physical address. */
            *Physical = iface.u.AllocateNonPagedMemory.physical;

            *Logical = iface.u.AllocateNonPagedMemory.logical;
        }
        else
        {
			gcmTRACE_ZONE(gcvLEVEL_INFO,
						gcvZONE_DRIVER,
            			"gcoOS_AllocateNonPagedMemory: failed to allocate memory status->%d",
						status);
        }
    }

    /* Return status. */
    return status;
}

/*******************************************************************************
 **
 **	gcoOS_FreeNonPagedMemory
 **
 **	Free non-paged memory from the kernel.
 **
 **	INPUT:
 **
 **		gcoOS Os
 **			Pointer to an gcoOS object.
 **
 **      gctBOOL InUserSpace
 **          TRUE to mape the memory into the user space.
 **
 **      gctSIZE_T * Bytes
 **          Pointer to the number of bytes to allocate.
 **
 **	OUTPUT:
 **
 **		gctSIZE_T * Bytes
 **			Pointer to a variable that will receive the aligned number of bytes
 **          allocated.
 **
 **		gctPHYS_ADDR * Physical
 **			Pointer to a variable that will receive the physical addresses of
 **          the allocated pages.
 **
 **		gctPOINTER * Logical
 **			Pointer to a variable that will receive the logical address of the
 **			allocation.
 */
gceSTATUS gcoOS_FreeNonPagedMemory(
        IN gcoOS Os,
        IN gctSIZE_T Bytes,
        IN gctPHYS_ADDR Physical,
        IN gctPOINTER Logical
        )
{
    gcsHAL_INTERFACE iface;
    gceSTATUS status;

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Os, gcvOBJ_OS);

    /* Initialize the gcsHAL_INTERFACE structure. */
    iface.command = gcvHAL_FREE_NON_PAGED_MEMORY;
    iface.u.FreeNonPagedMemory.bytes = Bytes;
    iface.u.FreeNonPagedMemory.physical = Physical;
    iface.u.FreeNonPagedMemory.logical = Logical;

    /* Call kernel driver. */
    status = gcoOS_DeviceControl(Os,
				IOCTL_GCHAL_INTERFACE,
				&iface,
            	sizeof(iface),
				&iface,
            	sizeof(iface));

    if (status == gcvSTATUS_OK)
    {
        /* Get status from kernel. */
        status = iface.status;
    }

    /* Return status. */
    return status;
}

/*******************************************************************************
**
**	gcoOS_Open
**
**	Open or create a file.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**      gctCONST_STRING FileName
**          File name of file to open or create.
**
**      gceFILE_MODE Mode
**          Mode to open file with:
**
**				gcvFILE_CREATE		- Overwite any existing file.
**				gcvFILE_APPEND		- Append to an exisiting file or create a new file
**										if there is no exisiting file.
**				gcvFILE_READ		- Open an existing file for read only.
**				gcvFILE_CREATETEXT	- Overwite any existing text file.
**				gcvFILE_APPENDTEXT	- Append to an exisiting text file or create a new text file
**										if there is no exisiting file.
**				gcvFILE_READTEXT	- Open an existing text file fir read only.
**
**	OUTPUT:
**
**      gctFILE * File
**			Pointer to a variable receivig the handle to the opened file.
*/
gceSTATUS
gcoOS_Open(
	IN gcoOS Os,
	IN gctCONST_STRING FileName,
	IN gceFILE_MODE Mode,
	OUT gctFILE * File
	)
{
	static gctCONST_STRING modes[] =
	{
		"wb",
		"ab",
		"rb",
		"w",
		"a",
		"r",
	};
	FILE * file;

	/* Verify the arguments. */
	gcmVERIFY_ARGUMENT(File != gcvNULL);

	/* Open the file. */
	file = fopen(FileName, modes[Mode]);

	if (file == gcvNULL)
	{
		/* Error. */
		return gcvSTATUS_GENERIC_IO;
	}

	/* Return handle to file. */
	*File = (gctFILE) file;

	/* Success. */
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoOS_Close
**
**	Close a file.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**      gctFILE File
**          Pointer to an open file object.
**
**	OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_Close(
	IN gcoOS OS,
	IN gctFILE File
	)
{
	/* Verify the arguments. */
	gcmVERIFY_ARGUMENT(File != NULL);

	/* Close the file. */
	fclose((FILE *) File);

	/* Success. */
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoOS_Read
**
**	Read data from an open file.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**      gctFILE File
**          Pointer to an open file object.
**
**		gctSIZE_T ByteCount
**			Number of bytes to read from the file.
**
**		gctCONST_POINTER Data
**			Pointer to the data to read from the file.
**
**	OUTPUT:
**
**      gctSIZE_T * ByteRead
**			Point to a variable receiving the number of bytes read from the file.
*/
gceSTATUS
gcoOS_Read(
	IN gcoOS Os,
	IN gctFILE File,
	IN gctSIZE_T ByteCount,
	IN gctPOINTER Data,
	OUT gctSIZE_T * ByteRead
	)
{
	size_t byteRead;

	/* Verify the arguments. */
	gcmVERIFY_ARGUMENT(File != gcvNULL);
	gcmVERIFY_ARGUMENT(ByteCount > 0);
	gcmVERIFY_ARGUMENT(Data != gcvNULL);

	/* Read the data from the file. */
	byteRead = fread(Data, 1, ByteCount, (FILE *) File);

	if (ByteRead != gcvNULL) *ByteRead = (gctSIZE_T) byteRead;

	/* Success. */
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoOS_Write
**
**	Write data to an open file.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**      gctFILE File
**          Pointer to an open file object.
**
**		gctSIZE_T ByteCount
**			Number of bytes to write to the file.
**
**		gctCONST_POINTER Data
**			Pointer to the data to write to the file.
**
**	OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_Write(
	IN gcoOS Os,
	IN gctFILE File,
	IN gctSIZE_T ByteCount,
	IN gctCONST_POINTER Data
	)
{
	size_t byteWritten;

	/* Verify the arguments. */
	gcmVERIFY_ARGUMENT(File != gcvNULL);
	gcmVERIFY_ARGUMENT(ByteCount > 0);
	gcmVERIFY_ARGUMENT(Data != gcvNULL);

	/* Write the data to the file. */
	byteWritten = fwrite(Data, 1, ByteCount, (FILE *) File);

	if (byteWritten == ByteCount)
	{
		/* Success. */
		return gcvSTATUS_OK;
	}
	else
	{
		/* Error */
		return gcvSTATUS_GENERIC_IO;
	}
}

/*******************************************************************************
**
**	gcoOS_GetPos
**
**	Get the current position of a file.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**      gctFILE File
**          Pointer to an open file object.
**
**	OUTPUT:
**
**      gctUINT32 * Position
**          Pointer to a variable receiving the current position of the file.
*/
gceSTATUS
gcoOS_GetPos(
	IN gcoOS Os,
	IN gctFILE File,
	OUT gctUINT32 * Position
	)
{
	/* Verify the arguments. */
	gcmVERIFY_ARGUMENT(File != NULL);
	gcmVERIFY_ARGUMENT(Position != NULL);

	/* Get the current file position. */
	*Position = ftell((FILE *) File);

	/* Success. */
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoOS_SetPos
**
**	Set position for a file.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**      gctFILE File
**          Pointer to an open file object.
**
**      gctUINT32 Position
**          Absolute position of the file to set.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoOS_SetPos(
	IN gcoOS Os,
	IN gctFILE File,
	IN gctUINT32 Position
	)
{
	/* Verify the arguments. */
	gcmVERIFY_ARGUMENT(File != NULL);

	/* Set file position. */
	fseek((FILE *) File, Position, SEEK_SET);

	/* Success. */
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoOS_Seek
**
**	Set position for a file.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**      gctFILE File
**          Pointer to an open file object.
**
**      gctUINT32 Offset
**          Offset added to the position specified by Whence.
**
**      gceFILE_WHENCE Whence
**          Mode that specify how to add the offset to the position:
**
**				gcvFILE_SEEK_SET	- Relative to the start of the file.
**				gcvFILE_SEEK_CUR	- Relative to the current position.
**				gcvFILE_SEEK_END	- Relative to the end of the file.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoOS_Seek(
	IN gcoOS Os,
	IN gctFILE File,
	IN gctUINT32 Offset,
	IN gceFILE_WHENCE Whence
	)
{
	gctINT result = 0;

	/* Verify the arguments. */
	gcmVERIFY_ARGUMENT(File != gcvNULL);

	/* Set file position. */
	switch (Whence)
	{
	case gcvFILE_SEEK_SET:
		result = fseek((FILE *) File, Offset, SEEK_SET);
		break;
	case gcvFILE_SEEK_CUR:
		result = fseek((FILE *) File, Offset, SEEK_CUR);
		break;
	case gcvFILE_SEEK_END:
		result = fseek((FILE *) File, Offset, SEEK_END);
		break;
	}

	if (result == 0)
	{
		/* Success. */
		return gcvSTATUS_OK;
	}
	else
	{
		/* Error */
		return gcvSTATUS_GENERIC_IO;
	}
}

/*******************************************************************************
**
**	gcoOS_CreateMutex
**
**	Create a new mutex.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**	OUTPUT:
**
**		gctPOINTER * Mutex
**			Pointer to a variable that will hold a pointer to the mutex.
*/
gceSTATUS
gcoOS_CreateMutex(
	IN gcoOS Os,
	OUT gctPOINTER * Mutex
	)
{
	gceSTATUS status;
	pthread_mutex_t* mutex;

	/* Validate the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
	gcmVERIFY_ARGUMENT(Mutex != gcvNULL);

	/* Allocate memory for the mutex. */
	status = gcoOS_Allocate(Os,
							gcmSIZEOF(pthread_mutex_t),
							(gctPOINTER *) &mutex);

	if (gcmIS_SUCCESS(status))
	{
		/* Initialize the mutex. */
		pthread_mutex_init(mutex, NULL);

		/* Return mutex to caller. */
		*Mutex = (gctPOINTER) mutex;
	}

	/* Success. */
	return status;
}

/*******************************************************************************
**
**	gcoOS_DeleteMutex
**
**	Delete a mutex.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**		gctPOINTER Mutex
**			Pointer to the mute to be deleted.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoOS_DeleteMutex(
	IN gcoOS Os,
	IN gctPOINTER Mutex
	)
{
	pthread_mutex_t *mutex;

	/* Validate the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
	gcmVERIFY_ARGUMENT(Mutex != gcvNULL);

	/* Cast the pointer. */
	mutex = (pthread_mutex_t *) Mutex;

	/* Destroy the mutex. */
	pthread_mutex_destroy(mutex);

	/* Free the memory. */
	gcmVERIFY_OK(gcoOS_Free(Os, mutex));

	/* Success. */
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoOS_AcquireMutex
**
**	Acquire a mutex.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**		gctPOINTER Mutex
**			Pointer to the mutex to be acquired.
**
**		gctUINT32 Timeout
**			Timeout value specified in milliseconds.  If 'Timeout' is
**			gcvINFINITE, the thread will be suspended forever until the mutex has
**			been acquired.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoOS_AcquireMutex(
	IN gcoOS Os,
	IN gctPOINTER Mutex,
	IN gctUINT32 Timeout
	)
{
	gceSTATUS status;
	pthread_mutex_t *mutex;

	/* Validate the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
	gcmVERIFY_ARGUMENT(Mutex != gcvNULL);

	/* Cast the pointer. */
	mutex = (pthread_mutex_t *) Mutex;

	/* Test for infinite. */
	if (Timeout == gcvINFINITE)
	{
		/* Lock the mutex. */
		if (pthread_mutex_lock(mutex))
		{
			/* Some error. */
			status = gcvSTATUS_GENERIC_IO;
		}
		else
		{
			/* Success. */
			status = gcvSTATUS_OK;
		}
	}
	else
	{
		/* Try locking the mutex. */
		if (pthread_mutex_trylock(mutex))
		{
			/* Assume timeout. */
			status = gcvSTATUS_TIMEOUT;

			/* Loop while not timeout. */
			while (Timeout-- > 0)
			{
				/* Try locking the mutex. */
				if (pthread_mutex_trylock(mutex) == 0)
				{
					/* Success. */
					status = gcvSTATUS_OK;
					break;
				}

				/* Sleep 1 millisecond. */
				usleep(1000);
			}
		}
		else
		{
			/* Success. */
			status = gcvSTATUS_OK;
		}
	}

	/* Return the status. */
	return status;
}

/*******************************************************************************
**
**	gcoOS_ReleaseMutex
**
**	Release an acquired mutex.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**		gctPOINTER Mutex
**			Pointer to the mutex to be released.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoOS_ReleaseMutex(
	IN gcoOS Os,
	IN gctPOINTER Mutex
	)
{
	pthread_mutex_t *mutex;

	/* Validate the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
	gcmVERIFY_ARGUMENT(Mutex != gcvNULL);

	/* Cast the pointer. */
	mutex = (pthread_mutex_t *) Mutex;

	/* Release the mutex. */
	pthread_mutex_unlock(mutex);

	/* Success. */
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoOS_CreateSignal
**
**	Create a new signal.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**		gctBOOL ManualReset
**			If set to gcvTRUE, gcoOS_Signal with gcvFALSE must be called in
**			order to set the signal to nonsignaled state.
**			If set to gcvFALSE, the signal will automatically be set to
**			nonsignaled state by gcoOS_WaitSignal function.
**
**	OUTPUT:
**
**		gctSIGNAL * Signal
**			Pointer to a variable receiving the created gctSIGNAL.
*/
gceSTATUS
gcoOS_CreateSignal(
	IN gcoOS Os,
	IN gctBOOL ManualReset,
	OUT gctSIGNAL * Signal
	)
{
#if USE_NEW_LINUX_SIGNAL
	gceSTATUS status;
	gcsSIGNAL * signal = gcvNULL;
	gctINT mutexResult = -1;
	gctINT condResult = -1;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
	gcmVERIFY_ARGUMENT(Signal != NULL);

	/* Allocate a signal structure. */
	gcmONERROR(
		gcoOS_Allocate(Os, gcmSIZEOF(gcsSIGNAL), (gctPOINTER *) &signal));

	/* Initialize mutex. */
	mutexResult = pthread_mutex_init(&signal->mutex, gcvNULL);
	if (mutexResult != 0)
	{
		status = gcvSTATUS_GENERIC_IO;
		goto OnError;
	}

	/* Initialize the condition. */
	condResult = pthread_cond_init(&signal->condition, gcvNULL);
	if (condResult != 0)
	{
		status = gcvSTATUS_GENERIC_IO;
		goto OnError;
	}

	/* Initialize signal states. */
	signal->os       = Os;
	signal->state    = gcvFALSE;
	signal->manual   = ManualReset;
	signal->pending  = 0;
	signal->received = 0;

	/* Set the signal handle. */
	*Signal = (gctSIGNAL) signal;

	/* Success. */
	return gcvSTATUS_OK;

OnError:
	/* Roll back. */
	if (signal != gcvNULL)
	{
		if (condResult == 0)
		{
			/* Destroy the condition variable. */
			pthread_cond_destroy(&signal->condition);
		}

		if (mutexResult == 0)
		{
			/* Destroy the mutex. */
			pthread_mutex_destroy(&signal->mutex);
		}

		/* Free the signal structure */
		gcmVERIFY_OK(gcoOS_Free(Os, signal));
	}

	/* Return the status. */
	return status;
#else
	gceSTATUS status;
    gcsHAL_INTERFACE iface;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
	gcmVERIFY_ARGUMENT(Signal != NULL);

    /* Initialize the gcsHAL_INTERFACE structure. */
    iface.command = gcvHAL_USER_SIGNAL;
    iface.u.UserSignal.command = gcvUSER_SIGNAL_CREATE;
    iface.u.UserSignal.manualReset = ManualReset;

    /* Call kernel driver. */
	status = gcoOS_DeviceControl(Os,
				IOCTL_GCHAL_INTERFACE,
				&iface,
				sizeof(iface),
				&iface,
				sizeof(iface)
				);

    if (status == gcvSTATUS_OK)
    {
        /* Get status from kernel. */
        status = iface.status;

		if (status == gcvSTATUS_OK)
		{
			*Signal = (gctSIGNAL) iface.u.UserSignal.id;
		}
    }

	return status;
#endif /* USE_NEW_LINUX_SIGNAL */
}

/*******************************************************************************
**
**	gcoOS_DestroySignal
**
**	Destroy a signal.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**		gctSIGNAL Signal
**			The ID of the signal.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoOS_DestroySignal(
	IN gcoOS Os,
	IN gctSIGNAL Signal
	)
{
#if USE_NEW_LINUX_SIGNAL
	gcsSIGNAL * signal;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
	gcmVERIFY_ARGUMENT(Signal != gcvNULL);

	/* Cast the handle to the signal structure. */
	signal = (gcsSIGNAL *) Signal;

	/* Wait until all pending signals have been received - we don't want a
	** segfault reading structures in the signal handler that have been freed. */
	if (signal->pending != signal->received)
	{
		gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
					  "%s(%d): Signal=%p Pending=%d Received=%d.",
					  __FUNCTION__, __LINE__,
					  signal,
					  signal->pending,
					  signal->received);

		gcmVERIFY_OK(
			gcoOS_Signal(Os, Signal, gcvFALSE));

		gcoOS_WaitSignal(Os, Signal, 10);
	}

	/* Destroy the condition variable. */
	pthread_cond_destroy(&signal->condition);

	/* Destroy the mutex. */
	pthread_mutex_destroy(&signal->mutex);

	/* Free the signal structure. */
	gcmVERIFY_OK(gcoOS_Free(Os, signal));

	/* Success. */
	return gcvSTATUS_OK;
#else
	gceSTATUS status;
	gcsHAL_INTERFACE iface;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);

	gcmTRACE_ZONE(gcvLEVEL_VERBOSE,
				gcvZONE_OS,
				"gcoOS_DestroySignal: signal->%d.\n",
				(gctINT) Signal
				);

    /* Initialize the gcsHAL_INTERFACE structure. */
    iface.command = gcvHAL_USER_SIGNAL;
    iface.u.UserSignal.command = gcvUSER_SIGNAL_DESTROY;
    iface.u.UserSignal.id = (gctINT) Signal;

    /* Call kernel driver. */
	status = gcoOS_DeviceControl(Os,
				IOCTL_GCHAL_INTERFACE,
				&iface,
				sizeof(iface),
				&iface,
				sizeof(iface)
				);

    if (status == gcvSTATUS_OK)
    {
        /* Get status from kernel. */
        status = iface.status;
    }

	/* Success. */
	return status;
#endif /* USE_NEW_LINUX_SIGNAL */
}

/*******************************************************************************
**
**	gcoOS_Signal
**
**	Set a state of the specified signal.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**		gctSIGNAL Signal
**			The ID of the signal.
**
**		gctBOOL State
**			If gcvTRUE, the signal will be set to signaled state.
**			If gcvFALSE, the signal will be set to nonsignaled state.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoOS_Signal(
	IN gcoOS Os,
	IN gctSIGNAL Signal,
	IN gctBOOL State
	)
{
#if USE_NEW_LINUX_SIGNAL
	gcsSIGNAL * signal;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
	gcmVERIFY_ARGUMENT(Signal != gcvNULL);

	/* Cast the handle to the signal structure. */
	signal = (gcsSIGNAL *) Signal;

	/* Acquire the mutex. */
	if (pthread_mutex_lock(&signal->mutex))
	{
		return gcvSTATUS_GENERIC_IO;
	}

	/* Set the state. */
	signal->state = State;

	/* If the state is signaled, notify all waiting threads. */
	if (State && pthread_cond_broadcast(&signal->condition))
	{
		gcmTRACE(gcvLEVEL_WARNING,
				 "%s(%d): pthread_cond_broadcast failed (%d).",
				 __FUNCTION__, __LINE__, errno);
	}

	/* Release the mutex. */
	if (pthread_mutex_unlock(&signal->mutex))
	{
		gcmTRACE(gcvLEVEL_ERROR,
				 "%s(%d): pthread_mutex_unlock failed (%d).",
				 __FUNCTION__, __LINE__, errno);

		return gcvSTATUS_GENERIC_IO;
	}

	/* Success. */
	return gcvSTATUS_OK;
#else
	gceSTATUS status;
	gcsHAL_INTERFACE iface;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);

	gcmTRACE_ZONE(gcvLEVEL_VERBOSE,
		gcvZONE_OS,
		"gcoOS_Signal: signal->%d, state->%d.\n",
		(gctINT) Signal,
		State
		);

    /* Initialize the gcsHAL_INTERFACE structure. */
    iface.command = gcvHAL_USER_SIGNAL;
    iface.u.UserSignal.command = gcvUSER_SIGNAL_SIGNAL;
    iface.u.UserSignal.id = (gctINT) Signal;
    iface.u.UserSignal.state = State;

    /* Call kernel driver. */
	status = gcoOS_DeviceControl(Os,
				IOCTL_GCHAL_INTERFACE,
				&iface,
				sizeof(iface),
				&iface,
				sizeof(iface)
				);

    if (status == gcvSTATUS_OK)
    {
        /* Get status from kernel. */
        status = iface.status;
    }

	return status;
#endif /* USE_NEW_LINUX_SIGNAL */
}

/*******************************************************************************
**
**	gcoOS_WaitSignal
**
**	Wait for a signal to become signaled.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**		gctSIGNAL Signal
**			The ID of the signal.
**
**		gctUINT32 Wait
**			Number of milliseconds to wait.
**			Pass the value of gcvINFINITE for an infinite wait.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoOS_WaitSignal(
	IN gcoOS Os,
	IN gctSIGNAL Signal,
	IN gctUINT32 Wait
	)
{
#if USE_NEW_LINUX_SIGNAL
	gcsSIGNAL * signal;
	gctINT result;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
	gcmVERIFY_ARGUMENT(Signal != NULL);

	/* Cast the handle to the signal structure. */
	signal = (gcsSIGNAL *) Signal;

	/* Acquire the mutex. */
	if ((result = pthread_mutex_lock(&signal->mutex)) == 0)
	{
		/* See if we have to wait for the signal. */
		if (!signal->state)
		{
			if (Wait == 0)
			{
				/* User just wants to check the signal state. */
				result = ETIMEDOUT;
			}

			else if (Wait == gcvINFINITE)
			{
				/* Wait forever. */
				result = pthread_cond_wait(&signal->condition, &signal->mutex);
			}

			else
			{
				struct timeval current;
				struct timespec timeout;
				gctUINT64 nanos;

				/* Get current time. */
				if (gettimeofday(&current, gcvNULL))
				{
					result = 1;
				}
				else
				{
					/* Compute absolute time. */
					nanos = current.tv_usec * 1000ULL
						  + Wait * 1000000ULL;

					/* Test for overflow. */
					if (nanos > 1000000000ULL)
					{
						timeout.tv_sec  = (long) (nanos / 1000000000ULL)
										+ current.tv_sec;
						timeout.tv_nsec = (long) (nanos % 1000000000ULL);
					}
					else
					{
						timeout.tv_sec  = current.tv_sec;
						timeout.tv_nsec = (long) nanos;
					}

					/* Wait until either the condition is set or time out. */
					result = pthread_cond_timedwait(&signal->condition,
									 				&signal->mutex,
													&timeout);
				}
			}
		}

		/* Release the mutex. */
		if (pthread_mutex_unlock(&signal->mutex))
		{
			return gcvSTATUS_GENERIC_IO;
		}
	}

	if (result == ETIMEDOUT)
	{
		/* Time out. */
		return gcvSTATUS_TIMEOUT;
	}

	else if (result != 0)
	{
		/* Some other error. */
		return gcvSTATUS_GENERIC_IO;
	}

	/* See if we need to reset the signal. */
	if (!signal->manual)
	{
		/* Acquire the mutex. */
		if (pthread_mutex_lock(&signal->mutex))
		{
			gcmTRACE(gcvLEVEL_ERROR,
					 "%s(%d): pthread_mutex_lock failed (%d).",
					 __FUNCTION__, __LINE__, errno);

			/* Error. */
			return gcvSTATUS_GENERIC_IO;
		}
		else
		{
			/* Clear the state. */
			signal->state = gcvFALSE;

			/* Release the mutex. */
			if (pthread_mutex_unlock(&signal->mutex))
			{
				gcmTRACE(gcvLEVEL_ERROR,
						 "%s(%d): pthread_mutex_unlock failed (%d).",
						 __FUNCTION__, __LINE__, errno);

				/* Error. */
				return gcvSTATUS_GENERIC_IO;
			}
		}
	}

	/* Success. */
	return gcvSTATUS_OK;
#else
	gceSTATUS status;
	gcsHAL_INTERFACE iface;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);

	gcmTRACE_ZONE(gcvLEVEL_VERBOSE,
		gcvZONE_OS,
		"gcoOS_WaitSignal: signal->%d, wait->%d.\n",
		(gctINT) Signal,
		Wait
		);

    /* Initialize the gcsHAL_INTERFACE structure. */
    iface.command = gcvHAL_USER_SIGNAL;
    iface.u.UserSignal.command = gcvUSER_SIGNAL_WAIT;
    iface.u.UserSignal.id = (gctINT) Signal;
    iface.u.UserSignal.wait = Wait;

    /* Call kernel driver. */
	status = gcoOS_DeviceControl(Os,
		IOCTL_GCHAL_INTERFACE,
		&iface,
		sizeof(iface),
		&iface,
		sizeof(iface)
		);

    if (status == gcvSTATUS_OK)
    {
        /* Get status from kernel. */
        status = iface.status;
    }

	return status;
#endif /* USE_NEW_LINUX_SIGNAL */
}

gceSTATUS
gcoOS_Delay(
	IN gcoOS Os,
	IN gctUINT32 Delay
	)
{
	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);

	usleep((Delay == 0) ? 1 : (1000 * Delay));

	/* Success. */
	return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_MemCopy(
	IN gctPOINTER Destination,
	IN gctCONST_POINTER Source,
	IN gctSIZE_T Bytes
	)
{
	gcmVERIFY_ARGUMENT(Destination != NULL);
	gcmVERIFY_ARGUMENT(Source != NULL);
	gcmVERIFY_ARGUMENT(Bytes > 0);

	memcpy(Destination, Source, Bytes);

	return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_ZeroMemory(
	IN gctPOINTER Memory,
	IN gctSIZE_T Bytes
	)
{
	gcmVERIFY_ARGUMENT(Memory != NULL);
	gcmVERIFY_ARGUMENT(Bytes > 0);

	/* Zero the memory. */
	memset(Memory, 0, Bytes);

	/* Success. */
	return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_MemFill(
	IN gctPOINTER Memory,
	IN gctUINT8 Filler,
	IN gctSIZE_T Bytes
	)
{
	gcmVERIFY_ARGUMENT(Memory != NULL);

	if (Bytes > 0)
	{
		memset(Memory, Filler, Bytes);
	}

	return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_StrLen(
	IN gctCONST_STRING String,
	OUT gctSIZE_T * Length
	)
{
	gcmVERIFY_ARGUMENT(String != NULL);
	gcmVERIFY_ARGUMENT(Length != NULL);

	*Length = (gctSIZE_T) strlen(String);

	return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_StrFindReverse(
	IN gctCONST_STRING String,
	IN gctINT8 Character,
    OUT gctSTRING * Output
	)
{
	/* Verify the arguments. */
	gcmVERIFY_ARGUMENT(String != gcvNULL);
	gcmVERIFY_ARGUMENT(Output != gcvNULL);

    /* Call C. */
	*Output = strrchr(String, Character);

	/* Success. */
    return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_StrCopySafe(
	IN gctSTRING Destination,
	IN gctSIZE_T DestinationSize,
	IN gctCONST_STRING Source
	)
{
	/* Verify the arguments. */
	gcmVERIFY_ARGUMENT(Destination != gcvNULL);
	gcmVERIFY_ARGUMENT(Source != gcvNULL);

	/* Don't overflow the destination buffer. */
	strncpy(Destination, Source, DestinationSize - 1);

	/* Put this there in case the strncpy overflows. */
	Destination[DestinationSize - 1] = '\0';

	/* Success. */
	return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_StrCatSafe(
	IN gctSTRING Destination,
	IN gctSIZE_T DestinationSize,
	IN gctCONST_STRING Source
	)
{
	gctINT n;

	/* Verify the arguments. */
	gcmVERIFY_ARGUMENT(Destination != gcvNULL);
	gcmVERIFY_ARGUMENT(Source != gcvNULL);

	/* Find the end of the destination. */
	n = strlen(Destination);
	if (n < DestinationSize)
	{
		/* Append the string but don't overflow the destination buffer. */
		strncpy(Destination + n, Source, DestinationSize - n - 1);

		/* Put this there in case the strncpy overflows. */
		Destination[DestinationSize - 1] = '\0';
	}

	/* Success. */
	return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_StrCmp(
	IN gctCONST_STRING String1,
	IN gctCONST_STRING String2
	)
{
	int result;

	/* Verify the arguments. */
	gcmVERIFY_ARGUMENT(String1 != gcvNULL);
	gcmVERIFY_ARGUMENT(String2 != gcvNULL);

	/* Compare the strings and return proper status. */
	result = strcmp(String1, String2);

	return (result == 0)
			? gcvSTATUS_OK
			: ((result > 0) ? gcvSTATUS_LARGER : gcvSTATUS_SMALLER);
}

gceSTATUS
gcoOS_StrNCmp(
	IN gctCONST_STRING String1,
	IN gctCONST_STRING String2,
	IN gctSIZE_T Count
	)
{
	int result;

	/* Verify the arguments. */
	gcmVERIFY_ARGUMENT(String1 != gcvNULL);
	gcmVERIFY_ARGUMENT(String2 != gcvNULL);

	/* Compare the strings and return proper status. */
	result = strncmp(String1, String2, Count);

	return (result == 0)
			? gcvSTATUS_OK
			: ((result > 0) ? gcvSTATUS_LARGER : gcvSTATUS_SMALLER);
}

gceSTATUS
gcoOS_StrToFloat(
	IN gctCONST_STRING String,
	OUT gctFLOAT * Float
	)
{
	/* Verify the arguments. */
	gcmVERIFY_ARGUMENT(String != gcvNULL);

	*Float = (gctFLOAT)atof(String);

	return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_StrToInt(
	IN gctCONST_STRING String,
	OUT gctINT * Int
	)
{
	/* Verify the arguments. */
	gcmVERIFY_ARGUMENT(String != gcvNULL);

	*Int = (gctINT)atoi(String);

	return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_MemCmp(
	IN gctCONST_POINTER Memory1,
	IN gctCONST_POINTER Memory2,
	IN gctSIZE_T Bytes
	)
{
	gcmVERIFY_ARGUMENT(Memory1 != NULL);
	gcmVERIFY_ARGUMENT(Memory2 != NULL);
	gcmVERIFY_ARGUMENT(Bytes > 0);

	return (memcmp(Memory1, Memory2, Bytes) == 0)
			   ? gcvSTATUS_OK
			   : gcvSTATUS_MISMATCH;
}

gceSTATUS
gcoOS_PrintStrSafe(
	IN gctSTRING String,
	IN gctSIZE_T StringSize,
	IN OUT gctUINT_PTR Offset,
	IN gctCONST_STRING Format,
	...
	)
{
	va_list arguments;
	gceSTATUS status;

	va_start(arguments, Format);

	/* Pass through to the V case. */
	status = gcoOS_PrintStrVSafe(String, StringSize, Offset, Format, * ((gctPOINTER *) &arguments));

	va_end(arguments);

	return status;
}

gceSTATUS
gcoOS_PrintStrVSafe(
    OUT gctSTRING String,
    IN gctSIZE_T StringSize,
    IN OUT gctUINT_PTR Offset,
    IN gctCONST_STRING Format,
    IN gctPOINTER Arguments
    )
{
    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(String != gcvNULL);
    gcmVERIFY_ARGUMENT(Offset != gcvNULL);
    gcmVERIFY_ARGUMENT(Format != gcvNULL);

    if (*Offset < StringSize)
    {
		/* Format the string. */
	    gctINT n = vsnprintf(String + *Offset,
	    					 StringSize - *Offset - 1,
	    					 Format,
	    					 *(va_list *) &Arguments);

		if (n > 0)
		{
			*Offset += n;
		}
	}

    /* Success. */
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoOS_MapUserMemory
**
**	Lock down a user buffer and return an DMA'able address to be used by the
**	hardware to access it.
**
**	INPUT:
**
**		gctPOINTER Memory
**			Pointer to memory to lock down.
**
**		gctSIZE_T Size
**			Size in bytes of the memory to lock down.
**
**	OUTPUT:
**
**		gctPOINTER * Info
**			Pointer to variable receiving the information record required by
**			gcoOS_UnmapUserMemory.
**
**		gctUINT32_PTR Address
**			Pointer to a variable that will receive the address DMA'able by the
**			hardware.
*/
gceSTATUS
gcoOS_MapUserMemory(
	IN gcoOS Os,
	IN gctPOINTER Memory,
	IN gctSIZE_T Size,
	OUT gctPOINTER * Info,
	OUT gctUINT32_PTR Address
	)
{
	gceSTATUS status;
	gcsHAL_INTERFACE ioctl;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
	gcmVERIFY_ARGUMENT(Memory != gcvNULL);
	gcmVERIFY_ARGUMENT(Size > 0);
	gcmVERIFY_ARGUMENT(Address != gcvNULL);

    /* Initialize the gcsHAL_INTERFACE structure. */
    ioctl.command = gcvHAL_MAP_USER_MEMORY;
	ioctl.u.MapUserMemory.memory = Memory;
	ioctl.u.MapUserMemory.size   = Size;

    /* Call kernel driver. */
    status = gcoOS_DeviceControl(Os,
								 IOCTL_GCHAL_INTERFACE,
								 &ioctl, sizeof(ioctl),
								 &ioctl, sizeof(ioctl));

	if (gcmNO_ERROR(status))
	{
		/* Return the info on success. */
		*Info    = ioctl.u.MapUserMemory.info;

		/* Return the address on success. */
		*Address = ioctl.u.MapUserMemory.address;
	}

	/* Return the status. */
	return status;
}

/*******************************************************************************
**
**	gcoOS_UnmapUserMemory
**
**	Unlock a user buffer and that was previously locked down by
**	gcoOS_MapUserMemory.
**
**	INPUT:
**
**		gctPOINTER Memory
**			Pointer to memory to unlock.
**
**		gctSIZE_T Size
**			Size in bytes of the memory to unlock.
**
**		gctPOINTER Info
**			Information record returned by gcoOS_MapUserMemory.
**
**		gctUINT32_PTR Address
**			The address returned by gcoOS_MapUserMemory.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoOS_UnmapUserMemory(
	IN gcoOS Os,
	IN gctPOINTER Memory,
	IN gctSIZE_T Size,
	IN gctPOINTER Info,
	IN gctUINT32 Address
	)
{
	gcsHAL_INTERFACE ioctl;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
	gcmVERIFY_ARGUMENT(Memory != gcvNULL);
	gcmVERIFY_ARGUMENT(Size > 0);

    /* Initialize the gcsHAL_INTERFACE structure. */
    ioctl.command = gcvHAL_UNMAP_USER_MEMORY;
	ioctl.u.UnmapUserMemory.memory  = Memory;
	ioctl.u.UnmapUserMemory.size    = Size;
	ioctl.u.UnmapUserMemory.info    = Info;
	ioctl.u.UnmapUserMemory.address = Address;

    /* Call kernel driver. */
    return gcoOS_DeviceControl(Os,
							   IOCTL_GCHAL_INTERFACE,
							   &ioctl, sizeof(ioctl),
							   &ioctl, sizeof(ioctl));
}

/*******************************************************************************
**
**	gcoOS_StrDup
**
**	Duplicate the given string by copying it into newly allocated memory.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to gcoOS object.
**
**		gctCONST_STRING String
**			Pointer to string to duplicate.
**
**	OUTPUT:
**
**		gctSTRING * Target
**			Pointer to variable holding the duplicated string address.
*/
gceSTATUS
gcoOS_StrDup(
	IN gcoOS Os,
	IN gctCONST_STRING String,
	OUT gctSTRING * Target
	)
{
    gctSIZE_T bytes;
    gctSTRING string;
    gceSTATUS status;

    gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmVERIFY_ARGUMENT(String != gcvNULL);
    gcmVERIFY_ARGUMENT(Target != gcvNULL);

    do
    {
	    gcmERR_BREAK(gcoOS_StrLen(String, &bytes));

	    gcmERR_BREAK(gcoOS_Allocate(Os, bytes + 1, (gctPOINTER *) &string));

	    memcpy(string, String, bytes + 1);

	    *Target = string;

	    status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    return status;
}

/*******************************************************************************
**
**  gcoOS_LoadLibrary
**
**  Load a library.
**
**  INPUT:
**
**  	gcoOS Os
**  	    Pointer to gcoOS object.
**
**  	gctCONST_STRING Library
**	    Name of library to load.
**
**  OUTPUT:
**
**  	gctHANDLE * Handle
**  	    Pointer to variable receiving the library handle.
*/
gceSTATUS
gcoOS_LoadLibrary(
    IN gcoOS Os,
    IN gctCONST_STRING Library,
    OUT gctHANDLE * Handle
    )
{
#if defined STATIC_LINK
	return gcvSTATUS_NOT_SUPPORTED;
#else
    gctSIZE_T length;
    gctSTRING library = gcvNULL;

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmVERIFY_ARGUMENT(Handle != gcvNULL);

    if (Library != gcvNULL)
    {
    	/* Get the length of the library name. */
    	length = strlen(Library);

    	/* Test if the libray has ".so" at the end. */
    	if (strcmp(Library + length - 3, ".so") != 0)
    	{
    	    /* Allocate temporay string buffer. */
    	    gceSTATUS status = gcoOS_Allocate(Os,
				    	      length + 3 + 1,
					      (gctPOINTER *) &library);
    	    if (gcmIS_ERROR(status))
    	    {
    	    	/* Out of memory. */
    	    	return status;
    	    }

    	    /* Copy the library name to the temporary string buffer. */
    	    strcpy(library, Library);

    	    /* Append the ".so" to the temporary string buffer. */
    	    strcat(library, ".so");
    	}
    }

    /* Load the library. */
    *Handle = dlopen((library != gcvNULL) ? library : Library, RTLD_NOW);

    /* Free the temporary string buffer. */
    if (library != gcvNULL)
    {
    	gcmVERIFY_OK(gcoOS_Free(Os, library));
    }

    /* Return error if library could not be loaded. */
    return (*Handle == gcvNULL)
    	   ? gcvSTATUS_NOT_FOUND
	   : gcvSTATUS_OK;
#endif
}

/*******************************************************************************
**
**	gcoOS_FreeLibrary
**
**	Unload a loaded library.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to gcoOS object.
**
**		gctHANDLE Handle
**			Handle of a loaded libarry.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoOS_FreeLibrary(
	IN gcoOS Os,
	IN gctHANDLE Handle
	)
{
#if defined STATIC_LINK
	return gcvSTATUS_NOT_SUPPORTED;
#else
	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);

	/* Free the library. */
	dlclose(Handle);

	/* Success. */
	return gcvSTATUS_OK;
#endif
}

/*******************************************************************************
**
**	gcoOS_GetProcAddress
**
**	Get the address of a function inside a loaded library.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to gcoOS object.
**
**		gctHANDLE Handle
**			Handle of a loaded libarry.
**
**		gctCONST_STRING Name
**			Name of function to get the address of.
**
**	OUTPUT:
**
**		gctPOINTER * Function
**			Pointer to variable receiving the function pointer.
*/
gceSTATUS
gcoOS_GetProcAddress(
	IN gcoOS Os,
	IN gctHANDLE Handle,
	IN gctCONST_STRING Name,
	OUT gctPOINTER * Function
	)
{
#if defined STATIC_LINK
	return gcvSTATUS_NOT_SUPPORTED;
#else
	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
	gcmVERIFY_ARGUMENT(Name != gcvNULL);
	gcmVERIFY_ARGUMENT(Function != gcvNULL);

	/* Get the address of the function. */
	*Function = dlsym(Handle, Name);

	/* Return error if function could not be found. */
	return (*Function == gcvNULL)
		? gcvSTATUS_NOT_FOUND
		: gcvSTATUS_OK;
#endif
}

gceSTATUS
gcoOS_ProfileStart(
	IN gcoOS Os
	)
{
	return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_ProfileEnd(
	IN gcoOS Os,
	IN gctCONST_STRING Title
	)
{
	return gcvSTATUS_OK;
}

gctHANDLE
gcoOS_GetCurrentProcessID(
	void
	)
{
	return (gctHANDLE)getpid();
}


gceSTATUS
gcoOS_Compact(
	IN gcoOS Os
	)
{
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);

	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoOS_AllocateContiguous
**
**	Allocate contiguous memory from the kernel.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**      gctBOOL InUserSpace
**          gcvTRUE to map the memory into the user space.
**
**      gctSIZE_T * Bytes
**          Pointer to the number of bytes to allocate.
**
**	OUTPUT:
**
**		gctSIZE_T * Bytes
**			Pointer to a variable that will receive the aligned number of bytes
**          allocated.
**
**		gctPHYS_ADDR * Physical
**			Pointer to a variable that will receive the physical addresses of
**          the allocated memory.
**
**		gctPOINTER * Logical
**			Pointer to a variable that will receive the logical address of the
**			allocation.
*/
gceSTATUS
gcoOS_AllocateContiguous(
    IN gcoOS Os,
    IN gctBOOL InUserSpace,
    IN OUT gctSIZE_T * Bytes,
    OUT gctPHYS_ADDR * Physical,
    OUT gctPOINTER * Logical
    )
{
    gcsHAL_INTERFACE iface;
    gceSTATUS status;

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmVERIFY_ARGUMENT(Bytes != gcvNULL);
    gcmVERIFY_ARGUMENT(Physical != gcvNULL);
    gcmVERIFY_ARGUMENT(Logical != gcvNULL);

	do
	{
		/* Initialize the gcsHAL_INTERFACE structure. */
		iface.command = gcvHAL_ALLOCATE_CONTIGUOUS_MEMORY;
		iface.u.AllocateContiguousMemory.bytes = *Bytes;

		/* Call kernel driver. */
		gcmERR_BREAK(gcoOS_DeviceControl(Os,
										 IOCTL_GCHAL_INTERFACE,
										 &iface,
										 gcmSIZEOF(iface),
										 &iface,
										 gcmSIZEOF(iface)));

        /* Get status from kernel. */
        gcmERR_BREAK(iface.status);

		gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_MEMORY,
					  "gcoOS_AllocateContiguousMemory: %u bytes @ %p",
					  iface.u.AllocateContiguousMemory.bytes,
					  iface.u.AllocateContiguousMemory.logical);

		/* Return allocated number of bytes. */
        *Bytes = iface.u.AllocateContiguousMemory.bytes;

		/* Return physical address. */
        *Physical = iface.u.AllocateContiguousMemory.physical;

        /* Return logical address. */
        *Logical = iface.u.AllocateContiguousMemory.logical;

		/* Success. */
		return gcvSTATUS_OK;
    }
	while (gcvFALSE);

    /* Return the status. */
    return status;
}

/*******************************************************************************
**
**	gcoOS_FreeContiguous
**
**	Free contiguous memory from the kernel.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**		gctPHYS_ADDR Physical
**			The physical addresses of the allocated pages.
**
**		gctPOINTER Logical
**			The logical address of the allocation.
**
**      gctSIZE_T Bytes
**          Number of bytes allocated.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoOS_FreeContiguous(
    IN gcoOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes
    )
{
    gcsHAL_INTERFACE iface;
    gceSTATUS status;

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Os, gcvOBJ_OS);

	gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_MEMORY,
				  "gcoOS_FreeContiguousMemory: %u bytes @ %p",
				  Bytes, Logical);

	do
	{
	    /* Initialize the gcsHAL_INTERFACE structure. */
	    iface.command = gcvHAL_FREE_CONTIGUOUS_MEMORY;
		iface.u.FreeContiguousMemory.bytes    = Bytes;
		iface.u.FreeContiguousMemory.physical = Physical;
		iface.u.FreeContiguousMemory.logical  = Logical;

	    /* Call kernel driver. */
		gcmERR_BREAK(gcoOS_DeviceControl(Os,
										 IOCTL_GCHAL_INTERFACE,
										 &iface,
										 gcmSIZEOF(iface),
										 &iface,
										 gcmSIZEOF(iface)));

        /* Get status from kernel. */
		gcmERR_BREAK(iface.status);

		/* Success. */
		return gcvSTATUS_OK;
	}
	while (gcvFALSE);

    /* Return the status. */
    return status;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------- Atoms ----------------------------------*/

struct gcsATOM
{
	/* Counter. */
	gctINT32 counter;

	/* Pointer to gcoOS object. */
	gcoOS os;

	/* Mutex. */
	pthread_mutex_t mutex;
};

/* Create an atom. */
gceSTATUS
gcoOS_AtomConstruct(
	IN gcoOS Os,
	OUT gcsATOM_PTR * Atom
	)
{
	gceSTATUS status;
	gcsATOM_PTR atom = gcvNULL;

	gcmHEADER_ARG("Os=%p", Os);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
	gcmVERIFY_ARGUMENT(Atom != gcvNULL);

	do
	{
		/* Allocate memory for the atom. */
		gcmERR_BREAK(gcoOS_Allocate(Os,
        							gcmSIZEOF(struct gcsATOM),
                                    (gctPOINTER *) &atom));

		/* Initialize the atom to 0. */
		atom->counter = 0;
		atom->os      = Os;
		if (pthread_mutex_init(&atom->mutex, gcvNULL) != 0)
		{
			status = gcvSTATUS_OUT_OF_RESOURCES;
			break;
		}

		/* Return pointer to atom. */
		*Atom = atom;

		/* Success. */
		gcmFOOTER_ARG("*Atom=%p", *Atom);
		return gcvSTATUS_OK;
	}
	while (gcvFALSE);

	/* Free the atom. */
	if (atom != gcvNULL)
	{
		gcoOS_Free(Os, atom);
	}

	/* Return error status. */
	gcmFOOTER();
	return status;
}

/* Destroy an atom. */
gceSTATUS
gcoOS_AtomDestroy(
	IN gcoOS Os,
	IN gcsATOM_PTR Atom
	)
{
	gceSTATUS status;

	gcmHEADER_ARG("Os=%p Atom=%p", Os, Atom);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
	gcmVERIFY_ARGUMENT(Atom != gcvNULL);

	/* Free the atom. */
	status = gcoOS_Free(Os, Atom);

	/* Return the status. */
	gcmFOOTER();
	return status;
}

/* Increment an atom. */
gceSTATUS
gcoOS_AtomIncrement(
	IN gcoOS Os,
    IN gcsATOM_PTR Atom,
    OUT gctINT32_PTR OldValue
	)
{
	gctINT32 value;

	gcmHEADER_ARG("Os=%p Atom=%p", Os, Atom);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
	gcmVERIFY_ARGUMENT(Atom != gcvNULL);

	/* Increment the atom's counter. */
#ifdef LINUX_OABI
	value = __sync_fetch_and_add(Atom, 1);
#else
	value = __sync_fetch_and_add(&Atom->counter, 1);
#endif

	if (OldValue != gcvNULL)
	{
		/* Return the original value to the caller. */
		*OldValue = value;
	}

	/* Success. */
	gcmFOOTER_ARG("*OldValue=%d", gcmOPT_VALUE(OldValue));
	return gcvSTATUS_OK;
}

/* Decrement an atom. */
gceSTATUS
gcoOS_AtomDecrement(
	gcoOS Os,
	gcsATOM_PTR Atom,
	gctINT32_PTR OldValue
	)
{
	gctINT32 value;

	gcmHEADER_ARG("Os=%p Atom=%p", Os, Atom);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
	gcmVERIFY_ARGUMENT(Atom != gcvNULL);

	/* Decrement the atom's counter. */
#ifdef LINUX_OABI
	value = __sync_fetch_and_sub(Atom, 1);
#else
	value = __sync_fetch_and_sub(&Atom->counter, 1);
#endif

	if (OldValue != gcvNULL)
	{
		/* Return the original value to the caller. */
		*OldValue = value;
	}

	/* Success. */
	gcmFOOTER_ARG("*OldValue=%d", gcmOPT_VALUE(OldValue));
	return gcvSTATUS_OK;
}

/* Supply fallback functions in case the CPU architecture doesn't support
  atomic operations. */

gctINT32
__sync_fetch_and_add_4(
	gcsATOM_PTR Atom,
    gctINT32 Value
)
{
	gctUINT32 value;

 	/* Lock the mutex. */
    pthread_mutex_lock(&Atom->mutex);

    /* Get original value. */
	value = Atom->counter;

    /* Add given value. */
    Atom->counter += Value;

 	/* Unlock the mutex. */
    pthread_mutex_unlock(&Atom->mutex);

	/* Return the original value. */
    return value;
}

gctINT32
__sync_fetch_and_sub_4(
	gcsATOM_PTR Atom,
    gctINT32 Value
)
{
	gctUINT32 value;

 	/* Lock the mutex. */
    pthread_mutex_lock(&Atom->mutex);

    /* Get original value. */
    value = Atom->counter;

    /* Subtract given value. */
    Atom->counter -= Value;

 	/* Unlock the mutex. */
    pthread_mutex_unlock(&Atom->mutex);

	/* Return the original value. */
	return value;
}

/*******************************************************************************
**
**	gcoOS_GetTicks
**
**	Get the number of milliseconds since the system started.
**
**	INPUT:
**
**	OUTPUT:
**
*/
gctUINT32
gcoOS_GetTicks(
    void
	)
{
	struct timeval tv;

	/* Return the time of day in milliseconds. */
    gettimeofday(&tv, 0);
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

/*******************************************************************************
**
**	gcoOS_GetTime
**
**	Get the number of microseconds since 1970/1/1.
**
**	INPUT:
**
**	OUTPUT:
**
**		gctUINT64_PTR Time
**			Pointer to a variable to get time.
**
*/
gceSTATUS
gcoOS_GetTime(
	OUT gctUINT64_PTR Time
	)
{
	struct timeval tv;

	/* Return the time of day in microseconds. */
	gettimeofday(&tv, 0);
	*Time = (tv.tv_sec * 100000) + tv.tv_usec;
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoOS_GetCPUTime
**
**	Get CPU time usage in microseconds.
**
**	INPUT:
**
**	OUTPUT:
**
**		gctUINT64_PTR CPUTime
**			Pointer to a variable to get CPU time usage.
**
*/
gceSTATUS
gcoOS_GetCPUTime(
    OUT gctUINT64_PTR CPUTime
	)
{
	struct rusage usage;

	/* Return CPU time in microseconds. */
	if (getrusage(RUSAGE_SELF, &usage) == 0)
	{
		*CPUTime  = usage.ru_utime.tv_sec * 1000000 + usage.ru_utime.tv_usec;
		*CPUTime += usage.ru_stime.tv_sec * 1000000 + usage.ru_stime.tv_usec;
		return gcvSTATUS_OK;
	}
	else
	{
		*CPUTime = 0;
		return gcvSTATUS_INVALID_ARGUMENT;
	}
}

/*******************************************************************************
**
**	gcoOS_GetMemoryUsage
**
**	Get current processes resource usage.
**
**	INPUT:
**
**	OUTPUT:
**
**		gctUINT32_PTR MaxRSS
**			Total amount of resident set memory used.
**			The value will be in terms of memory pages used.
**
**		gctUINT32_PTR IxRSS
**			Total amount of memory used by the text segment
**			in kilobytes multiplied by the execution-ticks.
**
**		gctUINT32_PTR IdRSS
**			Total amount of private memory used by a process
**			in kilobytes multiplied by execution-ticks.
**
**		gctUINT32_PTR IsRSS
**			Total amount of memory used by the stack in
**			kilobytes multiplied by execution-ticks.
**
*/
gceSTATUS
gcoOS_GetMemoryUsage(
    OUT gctUINT32_PTR MaxRSS,
    OUT gctUINT32_PTR IxRSS,
    OUT gctUINT32_PTR IdRSS,
    OUT gctUINT32_PTR IsRSS
	)
{
	struct rusage usage;

	/* Return memory usage. */
	if (getrusage(RUSAGE_SELF, &usage) == 0)
	{
		*MaxRSS = usage.ru_maxrss;
		*IxRSS  = usage.ru_ixrss;
		*IdRSS  = usage.ru_idrss;
		*IsRSS  = usage.ru_isrss;
		return gcvSTATUS_OK;
	}
	else
	{
		*MaxRSS = 0;
		*IxRSS  = 0;
		*IdRSS  = 0;
		*IsRSS  = 0;
		return gcvSTATUS_INVALID_ARGUMENT;
	}
}

/*******************************************************************************
**
**	gcoOS_ReadRegister
**
**	Read data from a register.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**		gctUINT32 Address
**			Address of register.
**
**	OUTPUT:
**
**		gctUINT32 * Data
**			Pointer to a variable that receives the data read from the register.
*/
gceSTATUS
gcoOS_ReadRegister(
	IN gcoOS Os,
	IN gctUINT32 Address,
	OUT gctUINT32 * Data
	)
{
	gcsHAL_INTERFACE ioctl;
    gceSTATUS status;

	if ( Data == gcvNULL )
	{
		return gcvSTATUS_INVALID_ARGUMENT;
	}

    /* Initialize the gcsHAL_INTERFACE structure. */
    ioctl.command = gcvHAL_READ_REGISTER;
	ioctl.u.ReadRegisterData.address = Address;
	ioctl.u.ReadRegisterData.data   = 0;

    /* Call kernel driver. */
    status = gcoOS_DeviceControl(Os,
								 IOCTL_GCHAL_INTERFACE,
								 &ioctl, sizeof(ioctl),
								 &ioctl, sizeof(ioctl));

    if (gcmIS_ERROR(status))
    {
        *Data = 0;
        return status;
    }

	/* Return the Data on success. */
	*Data = ioctl.u.ReadRegisterData.data;

	return ioctl.status;
}

/*******************************************************************************
**
**	gcoOS_WriteRegister
**
**	Write data to a register.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**		gctUINT32 Address
**			Address of register.
**
**		gctUINT32 Data
**			Data for register.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoOS_WriteRegister(
	IN gcoOS Os,
	IN gctUINT32 Address,
	IN gctUINT32 Data
	)
{
	gcsHAL_INTERFACE ioctl;
    gceSTATUS status;

    /* Initialize the gcsHAL_INTERFACE structure. */
    ioctl.command = gcvHAL_WRITE_REGISTER;
	ioctl.u.WriteRegisterData.address = Address;
	ioctl.u.WriteRegisterData.data   = Data;

    /* Call kernel driver. */
    status = gcoOS_DeviceControl(Os,
								 IOCTL_GCHAL_INTERFACE,
								 &ioctl, sizeof(ioctl),
								 &ioctl, sizeof(ioctl));

    if (gcmIS_ERROR(status))
    {
        return status;
    }

	return ioctl.status;
}

/*******************************************************************************
**
**	gcoOS_SetProfileSetting
**
**	Set Vivante profiler settings.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**		gctBOOL Enable
**			Enable or Disable Vivante profiler.
**
**		gctCONST_STRING FileName
**			Specify FileName for storing profile data into.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS
gcoOS_SetProfileSetting(
	IN gcoOS Os,
	IN gctBOOL Enable,
	IN gctCONST_STRING FileName
	)
{
	gcsHAL_INTERFACE halInterface;
	gceSTATUS status;

	if (strlen(FileName) >= gcdMAX_PROFILE_FILE_NAME)
	{
		return gcvSTATUS_INVALID_ARGUMENT;
	}

	/* Initialize the gcsHAL_INTERFACE structure. */
	halInterface.command = gcvHAL_SET_PROFILE_SETTING;
	halInterface.u.SetProfileSetting.enable = Enable;
	strcpy(halInterface.u.SetProfileSetting.fileName, FileName);

        /* Call the kernel. */
	status = gcoOS_DeviceControl(
		Os,
		IOCTL_GCHAL_INTERFACE,
		&halInterface, gcmSIZEOF(halInterface),
		&halInterface, gcmSIZEOF(halInterface)
		);

	if (gcmIS_ERROR(status))
        {
        	return status;
        }

	return halInterface.status;
}
static gceSTATUS
gcoOS_Cache(
    IN gcoOS Os,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes,
    IN gctBOOL Operation
    )
{
    gcsHAL_INTERFACE ioctl;
    gceSTATUS status;

    gcmHEADER_ARG("Logical=0x%x Bytes=%u Operation=%d",
                  Logical, Bytes, Operation);

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(Logical != gcvNULL);
    gcmVERIFY_ARGUMENT(Bytes > 0);

    /* Set up the cache. */
    ioctl.command            = gcvHAL_CACHE;
    ioctl.u.Cache.process = Logical;
    ioctl.u.Cache.invalidate = Operation;
    ioctl.u.Cache.logical    = Logical;
    ioctl.u.Cache.bytes      = Bytes;

    /* Call the kernel. */
    gcmONERROR(gcoOS_DeviceControl(Os,
                                   IOCTL_GCHAL_INTERFACE,
                                   &ioctl, gcmSIZEOF(ioctl),
                                   &ioctl, gcmSIZEOF(ioctl)));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**  gcoOS_CacheClean
**
**  Clean the cache for the specified addresses.  The GPU is going to need the
**  data.  If the system is allocating memory as non-cachable, this function can
**  be ignored.
**
**  ARGUMENTS:
**
**      gcoOS Os
**          Pointer to gcoOS object.
**
**      gctPOINTER Logical
**          Logical address to flush.
**
**      gctSIZE_T Bytes
**          Size of the address range in bytes to flush.
*/
gceSTATUS
gcoOS_CacheClean(
    IN gcoOS Os,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Logical=0x%x Bytes=%u",
                   Logical, Bytes);

    /* Call common code. */
    gcmONERROR(gcoOS_Cache(Os,Logical, Bytes, 0));/*gcvCACHE_CLEAN));*/

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**  gcoOS_CacheFlush
**
**  Flush the cache for the specified addresses and invalidate the lines as
**  well.  The GPU is going to need and modify the data.  If the system is
**  allocating memory as non-cachable, this function can be ignored.
**
**  ARGUMENTS:
**
**      gcoOS Os
**          Pointer to gcoOS object.
**
**      gctPOINTER Logical
**          Logical address to flush.
**
**      gctSIZE_T Bytes
**          Size of the address range in bytes to flush.
*/
gceSTATUS
gcoOS_CacheFlush(
    IN gcoOS Os,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Logical=0x%x Bytes=%u",
                   Logical, Bytes);

    /* Call common code. */
    gcmONERROR(gcoOS_Cache(Os,Logical, Bytes, 0));/*gcvCACHE_FLUSH));*/

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**  gcoOS_CacheInvalidate
**
**  Invalidate the lines. The GPU is going modify the data.  If the system is
**  allocating memory as non-cachable, this function can be ignored.
**
**  ARGUMENTS:
**
**      gcoOS Os
**          Pointer to gcoOS object.
**
**      gctPOINTER Logical
**          Logical address to flush.
**
**      gctSIZE_T Bytes
**          Size of the address range in bytes to invalidated.
*/
gceSTATUS
gcoOS_CacheInvalidate(
    IN gcoOS Os,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Logical=0x%x Bytes=%u",
                   Logical, Bytes);

    /* Call common code. */
    gcmONERROR(gcoOS_Cache(Os,Logical, Bytes, 1));/*gcvCACHE_INVALIDATE));*/

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}
