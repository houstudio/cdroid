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




#include "gc_hal_user_precomp.h"
#include "gc_hal_user_brush.h"

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE	gcvZONE_2D

/******************************************************************************\
********************************** Structures **********************************
\******************************************************************************/

typedef struct _gcsBRUSH_LIST *		gcsBRUSH_LIST_PTR;
typedef struct _gcsBRUSH_NODE *		gcsBRUSH_NODE_PTR;
typedef struct _gcsCACHE_NODE *		gcsCACHE_NODE_PTR;

struct _gcsBRUSH_LIST
{
	/* Link to the previous node. */
	gcsBRUSH_LIST_PTR		prev;

	/* Link to the next node. */
	gcsBRUSH_LIST_PTR		next;
};

struct _gcsBRUSH_NODE
{
	/* Link node. */
	struct _gcsBRUSH_LIST	node;

	/* Brush instance. */
	gcoBRUSH				brush;

	/* Brush cache parameters. */
	gctUINT					id;
	gctINT					usageCount;
	gcsCACHE_NODE_PTR		cacheNode;
};

struct _gcsCACHE_NODE
{
	/* Link node. */
	struct _gcsBRUSH_LIST	node;

	/* Pointer to video memory block. */
	gcsSURF_NODE			patternNode;

	/* Pointer to the brush node that uses this block. */
	gcsBRUSH_NODE_PTR		brushNode;
};

struct _gcoBRUSH_CACHE
{
	/* Object. */
	gcsOBJECT				object;

	/* Pointer to an gcoHAL object. */
	gcoHAL					hal;

	/* Maximum allowed and current number of cached brushes. */
	gctUINT					maxCached;
	gctUINT					curAllocated;
	gctUINT					curFree;

	/* Last flushed node. */
	gcsBRUSH_NODE_PTR		lastFlushed;

	/* Cache nodes. */
	gcsCACHE_NODE_PTR		cacheHead;
	gcsCACHE_NODE_PTR		cacheTail;

	/* Brush nodes. */
	gcsBRUSH_NODE_PTR		brushHead;
	gcsBRUSH_NODE_PTR		brushTail;
};

const gcsCACHE_NODE_PTR CACHELESS = (gcsCACHE_NODE_PTR) ~0;


/******************************************************************************\
********************************* Support Code *********************************
\******************************************************************************/

/*******************************************************************************
**
**	_ComputeId
**
**	Computes an ID that identifies specified data set.
**	This ID is *not* guaranteed to be unique.
**
**	INPUT:
**
**		gctPOINTER Data
**			Points to a data set.
**
**		gctUINT32 Count
**			The size of the data set in bytes.
**
**	OUTPUT:
**
**		gctUINT32 *
**			ID of the data set.
*/
static void _ComputeId(
	IN gctPOINTER Data,
	IN gctUINT32 Count,
	IN OUT gctUINT32 * Id
	)
{
	gctUINT8 * data;
	gctUINT8 brushIDBytes[4] = {0, 0, 0, 0};
	gctUINT32 i;

	/* Cast the data pointer. */
	data = (gctUINT8*)Data;

	/* Determine the ID of the data buffer. */
	for (i = 0; i < Count; i++)
	{
		brushIDBytes[i & 3] ^= data[i];
	}

	/* Construct the final ID. */
	*Id = ((gctUINT32)(brushIDBytes[0]))
		| ((gctUINT32)(brushIDBytes[1]) << 8)
		| ((gctUINT32)(brushIDBytes[2]) << 16)
		| ((gctUINT32)(brushIDBytes[3]) << 24);
}

/*******************************************************************************
**
**	_FindByOb
**
**	Attempts to find a brush by its object pointer.
**
**	INPUT:
**
**		gcsBRUSH_NODE_PTR List
**			Pointer to a list of gcoBRUSH objects.
**
**		gcoBRUSH Brush
**			Pointer to a valid brush.
**
**	OUTPUT:
**
**		Nothing.
**
**	RETURN:
**
**		gcsBRUSH_NODE_PTR
**			Pointer to the brush node.
*/
static gcsBRUSH_NODE_PTR _FindByOb(
	IN gcsBRUSH_NODE_PTR List,
	IN gcoBRUSH Brush
	)
{
	/* Scan through all nodes. */
	while (List)
	{
		if (List->brush == Brush)
		{
			break;
		}

		List = (gcsBRUSH_NODE_PTR)(((gcsBRUSH_LIST_PTR)List)->next);
	}

	return List;
}

/*******************************************************************************
**
**	_FindById
**
**	Find a brush specified by its ID.
**
**	INPUT:
**
**		gctUINT Id
**			Search ID.
**
**		gctPOINTER BrushData
**			A pointer to the brush data set.
**
**		gctUINT DataCount
**			The size of brush data set in bytes.
**
**		gcsBRUSH_NODE_PTR List
**			List of brushes to search in.
**
**	OUTPUT:
**
**		gcsBRUSH_NODE_PTR * Node
**			A pointer to the node of the matching brush.
*/
static gceSTATUS _FindById(
	IN gctUINT Id,
	IN gctPOINTER BrushData,
	IN gctUINT32 DataCount,
	IN gcsBRUSH_NODE_PTR List,
	IN gcsBRUSH_NODE_PTR * Node
	)
{
	gceSTATUS status;
	gctUINT32 * newBrushData;
	gctUINT32 curBrushData[10 + 64];
	gctUINT32 curDataCount;
	gctUINT32 matchFound;
	gctUINT32 i;

	/* Reset the brush pointer. */
	*Node = gcvNULL;

	/* Cast the new data set. */
	newBrushData = (gctUINT32*)BrushData;

	/* Scan through nodes. */
	while (List)
	{
		/* Check IDs. */
		if (List->id == Id)
		{
			/* Set the size of the buffer. */
			curDataCount = sizeof(curBrushData);

			/* Get the data set of the current bitmap. */
			status = gcoBRUSH_GetBrushData(List->brush,
										  curBrushData,
										  &curDataCount);

			if (status != gcvSTATUS_OK)
			{
				/* Error. */
				return status;
			}

			/* Same data count? */
			if (curDataCount == DataCount)
			{
				/* Assume success. */
				matchFound = 1;

				/* Compare buffers. */
				for (i = 0; i < DataCount; i += 4)
				{
					if (curBrushData[i] != newBrushData[i])
					{
						matchFound = 0;
						break;
					}
				}

				/* Match found? */
				if (matchFound)
				{
					*Node = List;
					break;
				}
			}

		}

		List = (gcsBRUSH_NODE_PTR)(((gcsBRUSH_LIST_PTR)List)->next);
	}

	/* Success. */
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	_AddHeadNode
**
**	Connects a node at the beginning of the list.
**
**	INPUT:
**
**		gcsBRUSH_LIST_PTR * Head
**			Pointer to the head of the node list.
**
**		gcsBRUSH_LIST_PTR * Tail
**			Pointer to the tail of the node list.
**
**		gcsBRUSH_LIST_PTR Node
**			Pointer to a valid node.
**
**	OUTPUT:
**
**		Nothing.
*/
static void _AddHeadNode(
	IN gcsBRUSH_LIST_PTR * Head,
	IN gcsBRUSH_LIST_PTR * Tail,
	IN gcsBRUSH_LIST_PTR Node
	)
{
	if (*Head == gcvNULL)
	{
		/*
		   No items in the list.
		*/

		Node->prev = gcvNULL;
		Node->next = gcvNULL;

		*Head = Node;
		*Tail = Node;

	}
	else
	{
		/*
		   List is not empty.
		*/

		Node->prev = gcvNULL;
		Node->next = (*Head);

		(*Head)->prev = Node;
		(*Head) = Node;
	}
}

/*******************************************************************************
**
**	_AddTailNode
**
**	Connects a node at the end of the list.
**
**	INPUT:
**
**		gcsBRUSH_LIST_PTR * Head
**			Pointer to the head of the node list.
**
**		gcsBRUSH_LIST_PTR * Tail
**			Pointer to the tail of the node list.
**
**		gcsBRUSH_LIST_PTR Node
**			Pointer to a valid node.
**
**	OUTPUT:
**
**		Nothing.
*/
static void _AddTailNode(
	IN gcsBRUSH_LIST_PTR * Head,
	IN gcsBRUSH_LIST_PTR * Tail,
	IN gcsBRUSH_LIST_PTR Node
	)
{
	if (*Tail == gcvNULL)
	{
		/*
		   No items in the list.
		*/

		Node->prev = gcvNULL;
		Node->next = gcvNULL;

		*Head = Node;
		*Tail = Node;

	}
	else
	{
		/*
		   List is not empty.
		*/

		Node->prev = (*Tail);
		Node->next = gcvNULL;

		(*Tail)->next = Node;
		(*Tail) = Node;
	}
}

/*******************************************************************************
**
**	_DisconnectNode
**
**	Disconects a node from its list.
**
**	INPUT:
**
**		gcsBRUSH_LIST_PTR * Head
**			Pointer to the head of the node list.
**
**		gcsBRUSH_LIST_PTR * Tail
**			Pointer to the tail of the node list.
**
**		gcsBRUSH_LIST_PTR Node
**			Pointer to a valid node.
**
**	OUTPUT:
**
**		Nothing.
*/
static void _DisconnectNode(
	IN gcsBRUSH_LIST_PTR * Head,
	IN gcsBRUSH_LIST_PTR * Tail,
	IN gcsBRUSH_LIST_PTR Node
	)
{
	/* Disconnect previous link. */
	if (Node->prev)
	{
		Node->prev->next = Node->next;
	}
	else
	{
		*Head = Node->next;
	}

	/* Disconnect next link. */
	if (Node->next)
	{
		Node->next->prev = Node->prev;
	}
	else
	{
		*Tail = Node->prev;
	}
}

/*******************************************************************************
**
**	_MoveToHead
**
**	Moves the specified node to the head.
**
**	INPUT:
**
**		gcsBRUSH_LIST_PTR * Head
**			Pointer to the head of the node list.
**
**		gcsBRUSH_LIST_PTR * Tail
**			Pointer to the tail of the node list.
**
**		gcsBRUSH_LIST_PTR Node
**			Pointer to a valid node.
**
**	OUTPUT:
**
**		Nothing.
**
**	RETURN:
**
**		gctBOOL
**			Not zero if the function actually moved the item.
*/
static gctBOOL _MoveToHead(
	IN gcsBRUSH_LIST_PTR * Head,
	IN gcsBRUSH_LIST_PTR * Tail,
	IN gcsBRUSH_LIST_PTR Node
	)
{
	if (Head != gcvNULL)
	{
		/* Already the head? */
		if (*Head == Node)
		{
			return gcvFALSE;
		}

		/* Remove from the list. */
		_DisconnectNode(Head, Tail, Node);
	}

	/* Add to the top. */
	_AddHeadNode(Head, Tail, Node);
	return gcvTRUE;
}

/*******************************************************************************
**
**	_MoveToTail
**
**	Moves the specified node to the tail.
**
**	INPUT:
**
**		gcsBRUSH_LIST_PTR * Head
**			Pointer to the head of the node list.
**
**		gcsBRUSH_LIST_PTR * Tail
**			Pointer to the tail of the node list.
**
**		gcsBRUSH_LIST_PTR Node
**			Pointer to a valid node.
**
**	OUTPUT:
**
**		Nothing.
**
**	RETURN:
**
**		gctBOOL
**			Not zero if the function actually moved the item.
*/
static gctBOOL _MoveToTail(
	IN gcsBRUSH_LIST_PTR * Head,
	IN gcsBRUSH_LIST_PTR * Tail,
	IN gcsBRUSH_LIST_PTR Node
	)
{
	if (Tail != gcvNULL)
	{
		/* Already the head? */
		if (*Tail == Node)
		{
			return gcvFALSE;
		}

		/* Remove from the list. */
		_DisconnectNode(Head, Tail, Node);
	}

	/* Add to the top. */
	_AddTailNode(Head, Tail, Node);
	return gcvTRUE;
}

/*******************************************************************************
**
**	_GetCacheNode
**
**	Returns a pointer to an available cache node.
**
**	INPUT:
**
**		gcoBRUSH_CACHE BrushCache
**			Pointer to a valid gcoBRUSH_CACHE object.
**
**	OUTPUT:
**
**		Nothing.
*/
static gceSTATUS _GetCacheNode(
	IN gcoBRUSH_CACHE BrushCache,
	IN OUT gcsCACHE_NODE_PTR * Node
	)
{
	gceSTATUS status;
	gcsHAL_INTERFACE iface;
	gcsCACHE_NODE_PTR node;
	gcoOS os;

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(BrushCache, gcvOBJ_BRUSHCACHE);
	gcmVERIFY_ARGUMENT(Node != gcvNULL);

	/* Extract the gcoOS object pointer. */
	os = BrushCache->hal->os;
	gcmVERIFY_OBJECT(os, gcvOBJ_OS);

	/* Assume success. */
	status = gcvSTATUS_OK;

	do
	{
		/* See if there are free nodes available. */
		if (BrushCache->curFree)
		{
			/* Free nodes must be at the end of the list. */
			*Node = BrushCache->cacheTail;
			break;
		}

		/* See if we can still allocate new nodes. */
		if (BrushCache->curAllocated < BrushCache->maxCached)
		{
			/* Allocate new cache node. */
			status = gcoOS_Allocate(os,
								   sizeof(struct _gcsCACHE_NODE),
								   (gctPOINTER *) &node);

			if (status != gcvSTATUS_OK)
			{
				/* Error. */
				break;
			}

			/* Allocate video memory. */
			iface.command = gcvHAL_ALLOCATE_LINEAR_VIDEO_MEMORY;
			iface.u.AllocateLinearVideoMemory.bytes     = 8 * 8 * 4;
            iface.u.AllocateLinearVideoMemory.alignment = 64;
            iface.u.AllocateLinearVideoMemory.pool      = gcvPOOL_DEFAULT;
			iface.u.AllocateLinearVideoMemory.type      = gcvSURF_BITMAP;

			/* Call kernel API. */
			status = gcoHAL_Call(BrushCache->hal, &iface);

			if (status != gcvSTATUS_OK)
			{
				/* Roll back. */
				gcmVERIFY_OK(gcoOS_Free(os, node));

				/* Error. */
				break;
			}

			/* Init node. */
			node->patternNode.pool
				= iface.u.AllocateLinearVideoMemory.pool;
			node->patternNode.u.normal.node
				= iface.u.AllocateLinearVideoMemory.node;

			node->patternNode.valid          = gcvFALSE;
			node->patternNode.lockCount      = 0;
			node->patternNode.lockedInKernel = gcvFALSE;
			node->brushNode                  = gcvNULL;

			/* Lock the node. */
			gcmVERIFY_OK(gcoHARDWARE_Lock(
				BrushCache->hal->hardware,
				&node->patternNode,
				gcvNULL,
				gcvNULL
				));

			/* Add to the end of the list. */
			_AddTailNode((gcsBRUSH_LIST_PTR*)&BrushCache->cacheHead,
						 (gcsBRUSH_LIST_PTR*)&BrushCache->cacheTail,
						 (gcsBRUSH_LIST_PTR)node);

			/* Update the counts. */
			BrushCache->curAllocated++;
			BrushCache->curFree++;

			/* Set the result. */
			*Node = node;
			break;
		}

		/* Take the last node in the list. */
		if (BrushCache->curAllocated > 0)
		{
			/* Get a pointer to the last node. */
			node = BrushCache->cacheTail;

			/* Free up the node first. */
			node->brushNode->cacheNode = gcvNULL;
			node->brushNode = gcvNULL;
			BrushCache->curFree++;

			/* Set the result. */
			*Node = node;
			break;
		}

		/* Error. */
		status = gcvSTATUS_OUT_OF_MEMORY;
	}
	while (gcvFALSE);

	/* Return status. */
	return status;
}

/******************************************************************************\
**************************** gcoBRUSH_CACHE API Code ****************************
\******************************************************************************/

/*******************************************************************************
**
**	gcoBRUSH_CACHE_Construct
**
**	Create an gcoBRUSH_CACHE object.
**
**	INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
**
**	OUTPUT:
**
**		gcoBRUSH_CACHE * BrushCache
**			Pointer to the variable that will hold the gcoBRUSH_CACHE object.
*/
gceSTATUS gcoBRUSH_CACHE_Construct(
	IN gcoHAL Hal,
	gcoBRUSH_CACHE * BrushCache
	)
{
	gceSTATUS status;
	gcoOS os;
	gcoBRUSH_CACHE brushcache;

	gcmHEADER_ARG("Hal=0x%x", Hal);

	/* Verify the arguments. */
	gcmVERIFY_ARGUMENT(Hal != gcvNULL);
	gcmVERIFY_ARGUMENT(BrushCache != gcvNULL);

	/* Extract the gcoOS object pointer. */
	os = Hal->os;
	gcmVERIFY_OBJECT(os, gcvOBJ_OS);

	/* Allocate the gcoBRUSH_CACHE object. */
	status = gcoOS_Allocate(os,
						   sizeof(struct _gcoBRUSH_CACHE),
						   (gctPOINTER *) &brushcache);

	if (status != gcvSTATUS_OK)
	{
		/* Error. */
		gcmFOOTER();
		return status;
	}

	/* Initialize the object. */
	brushcache->object.type = gcvOBJ_BRUSHCACHE;
	brushcache->hal = Hal;

	/* Set members. */
	brushcache->maxCached = 8;
	brushcache->curAllocated = 0;
	brushcache->curFree = 0;

	brushcache->lastFlushed = gcvNULL;

	brushcache->cacheHead = gcvNULL;
	brushcache->cacheTail = gcvNULL;

	brushcache->brushHead = gcvNULL;
	brushcache->brushTail = gcvNULL;

	/* Return pointer to the gcoBRUSH_CACHE object. */
	*BrushCache = brushcache;

	/* Success. */
	gcmFOOTER_ARG("*BrushCache=0x%x", *BrushCache);
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoBRUSH_CACHE_Destroy
**
**	Destroy an gcoBRUSH_CACHE object.
**
**	INPUT:
**
**		gcoBRUSH_CACHE BrushCache
**			Pointer to an gcoBRUSH_CACHE object to be destroyed.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoBRUSH_CACHE_Destroy(
	IN gcoBRUSH_CACHE BrushCache
	)
{
	gcsBRUSH_NODE_PTR node;

	gcmHEADER_ARG("BrushCache=0x%x", BrushCache);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(BrushCache, gcvOBJ_BRUSHCACHE);

	/* Free cache nodes. */
	gcmVERIFY_OK(gcoBRUSH_CACHE_SetBrushLimit(BrushCache, 0));

	/* Free all brushHead brushes. */
	while (BrushCache->brushHead)
	{
		/* Get the current brush. */
		node = BrushCache->brushHead;
		_DisconnectNode((gcsBRUSH_LIST_PTR*)&BrushCache->brushHead,
						(gcsBRUSH_LIST_PTR*)&BrushCache->brushTail,
						(gcsBRUSH_LIST_PTR)node);

		/* Destroy the brush. */
		gcmVERIFY_OK(gcoBRUSH_Delete(node->brush));

		/* Free the gcsBRUSH_NODE_PTR structure. */
		gcmVERIFY_OK(gcoOS_Free(BrushCache->hal->os, node));
	}

	/* Mark gcoBRUSH_CACHE object as unknown. */
	BrushCache->object.type = gcvOBJ_UNKNOWN;

	/* Free the gcoBRUSH_CACHE object. */
	gcmVERIFY_OK(gcoOS_Free(BrushCache->hal->os, BrushCache));

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoBRUSH_CACHE_SetBrushLimit
**
**	Sets the maximum number of brushes in the cache.
**
**	INPUT:
**
**		gcoBRUSH_CACHE BrushCache
**			Pointer to a valid gcoBRUSH_CACHE object.
**
**		gctUINT MaxCount
**			Maximum number of brushes allowed in the cache at the same time.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoBRUSH_CACHE_SetBrushLimit(
	IN gcoBRUSH_CACHE BrushCache,
	IN gctUINT MaxCount
	)
{
	gcsCACHE_NODE_PTR node;

	gcmHEADER_ARG("BrushCache=0x%x MaxCount=%u", BrushCache, MaxCount);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(BrushCache, gcvOBJ_BRUSHCACHE);

	/* Update the limit. */
	BrushCache->maxCached = MaxCount;

	/* Trim cache if necessary. */
	while (BrushCache->curAllocated > BrushCache->maxCached)
	{
		/* Remove the last brush from the cache. */
		node = BrushCache->cacheHead;
		_DisconnectNode((gcsBRUSH_LIST_PTR*)&BrushCache->cacheHead,
						(gcsBRUSH_LIST_PTR*)&BrushCache->cacheTail,
						(gcsBRUSH_LIST_PTR)node);

		/* Update the number of allocated nodes. */
		BrushCache->curAllocated--;

		/* Is this node free? */
		if (node->brushNode == gcvNULL)
		{
			/* Yes, update free node count. */
			BrushCache->curFree--;
		}
		else
		{
			/* Reset the brush node. */
			node->brushNode->cacheNode = gcvNULL;
		}

		/* Unlock the memory. */
		gcmVERIFY_OK(gcoHARDWARE_Unlock(
			BrushCache->hal->hardware,
			&node->patternNode,
			gcvSURF_TYPE_UNKNOWN
			));

		/* Free the video memory. */
		gcmVERIFY_OK(gcoHARDWARE_ScheduleVideoMemory(
			BrushCache->hal->hardware,
			&node->patternNode
			));

		/* Delete the node. */
		gcmVERIFY_OK(gcoOS_Free(BrushCache->hal->os, node));
	}

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoBRUSH_CACHE_GetBrushID
**
**	Compute the brush ID based on the brush data.
**
**	INPUT:
**
**		gcoBRUSH_CACHE BrushCache
**			Pointer to a valid gcoBRUSH_CACHE object.
**
**		gctPOINTER BrushData
**			A pointer to the brush data set.
**
**		gctUINT DataCount
**			The size of brush data set in bytes.
**
**	OUTPUT:
**
**		gctUINT32 * BrushID
**			Brush ID.
*/
gceSTATUS gcoBRUSH_CACHE_GetBrushID(
	IN gcoBRUSH_CACHE BrushCache,
	IN gctPOINTER BrushData,
	IN gctUINT32 DataCount,
	IN OUT gctUINT32 * BrushID
	)
{
	gcmHEADER_ARG("BrushCache=0x%x BrushData=0x%x DataCount=%d BrushID=0x%x",
					BrushCache, BrushData, DataCount, BrushID);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(BrushCache, gcvOBJ_BRUSHCACHE);
	gcmVERIFY_ARGUMENT(BrushData != gcvNULL);
	gcmVERIFY_ARGUMENT(BrushID != gcvNULL);

	/* Determine the ID of the brush. */
	_ComputeId(BrushData, DataCount, BrushID);

	/* Success. */
	gcmFOOTER_ARG("*BrushID=%d", *BrushID);
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoBRUSH_CACHE_GetBrush
**
**	Find a matching brush by the passed brush ID and brush data set and return
**	a pointer to the brush. If a matching brush was found, its usage counter is
**	automatically incremented. Call gcoBRUSH_CACHE_DeleteBrush to release
**	the brush.
**
**	INPUT:
**
**		gcoBRUSH_CACHE BrushCache
**			Pointer to a valid gcoBRUSH_CACHE object.
**
**		gctUINT32 BrushID
**			ID of the brush.
**
**		gctPOINTER BrushData
**			A pointer to the brush data set.
**
**		gctUINT DataCount
**			The size of brush data set in bytes.
**
**	OUTPUT:
**
**		gcoBRUSH * Brush
**			A pointer to matching brush.
*/
gceSTATUS gcoBRUSH_CACHE_GetBrush(
	IN gcoBRUSH_CACHE BrushCache,
	IN gctUINT32 BrushID,
	IN gctPOINTER BrushData,
	IN gctUINT32 DataCount,
	IN OUT gcoBRUSH * Brush
	)
{
	gceSTATUS status;
	gcsBRUSH_NODE_PTR node;

	gcmHEADER_ARG("BrushCache=0x%x BrushID=%d BrushData=0x%x DataCount=%d Brush=0x%x",
					BrushCache, BrushID, BrushData, DataCount, Brush);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(BrushCache, gcvOBJ_BRUSHCACHE);
	gcmVERIFY_ARGUMENT(BrushData != gcvNULL);
	gcmVERIFY_ARGUMENT(Brush != gcvNULL);

	/* Reset the brush pointer. */
	*Brush = gcvNULL;

	do
	{
		/* Search for the node in brushHead brush list first. */
		status = _FindById(BrushID,
						   BrushData,
						   DataCount,
						   BrushCache->brushHead,
						   &node);

		if (status != gcvSTATUS_OK)
		{
			/* Error. */
			break;
		}

		/* Found it? */
		if (node != gcvNULL)
		{
			/* Increment the usage counter. */
			node->usageCount++;

			/* Set the result. */
			*Brush = node->brush;
			break;
		}
	}
	while (gcvFALSE);

	gcmFOOTER_ARG("*Brush=0x%x status=%d", *Brush, status);
	return status;
}

/*******************************************************************************
**
**	gcoBRUSH_CACHE_AddBrush
**
**	Add a brush to the brush cache.
**
**	INPUT:
**
**		gcoBRUSH_CACHE BrushCache
**			Pointer to a valid gcoBRUSH_CACHE object.
**
**		gcoBRUSH Brush
**			Pointer to a valid brush to be added to the cache.
**
**		gctUINT32 BrushID
**			Brush ID.
**
**		gctBOOL NeedMemory
**			If not zero, the brush requires video memory for its initialization.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoBRUSH_CACHE_AddBrush(
	IN gcoBRUSH_CACHE BrushCache,
	IN gcoBRUSH Brush,
	IN gctUINT32 BrushID,
	IN gctBOOL NeedMemory
	)
{
	gceSTATUS status;
	gcsBRUSH_NODE_PTR node;
	gcoOS os;

	gcmHEADER_ARG("BrushCache=0x%x Brush=0x%x BrushID=%d NeedMemory=%d",
					BrushCache, Brush, BrushID, NeedMemory);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(BrushCache, gcvOBJ_BRUSHCACHE);
	gcmVERIFY_OBJECT(Brush, gcvOBJ_BRUSH);

	/* Extract the gcoOS object pointer. */
	os = BrushCache->hal->os;
	gcmVERIFY_OBJECT(os, gcvOBJ_OS);

	/* Create a new brush. */
	status = gcoOS_Allocate(os,
						   sizeof(struct _gcsBRUSH_NODE),
						   (gctPOINTER *) &node);

	if (status != gcvSTATUS_OK)
	{
		/* Error. */
		gcmFOOTER();
		return status;
	}

	/* Init members. */
	node->brush = Brush;
	node->id = BrushID;
	node->usageCount = 1;
	node->cacheNode = NeedMemory ? gcvNULL : CACHELESS;

	/* Add the brush node to the end of the brush list. */
	_AddTailNode((gcsBRUSH_LIST_PTR*)&BrushCache->brushHead,
				 (gcsBRUSH_LIST_PTR*)&BrushCache->brushTail,
				 (gcsBRUSH_LIST_PTR)node);

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoBRUSH_CACHE_DeleteBrush
**
**	Remove a brush from the brush cache.
**
**	INPUT:
**
**		gcoBRUSH_CACHE BrushCache
**			Pointer to a valid gcoBRUSH_CACHE object.
**
**		gcoBRUSH Brush
**			Pointer to a valid brush to be deleted from the cache.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoBRUSH_CACHE_DeleteBrush(
	IN gcoBRUSH_CACHE BrushCache,
	IN gcoBRUSH Brush
	)
{
	gcsBRUSH_NODE_PTR node;

	gcmHEADER_ARG("BrushCache=0x%x Brush=0x%x", BrushCache, Brush);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(BrushCache, gcvOBJ_BRUSHCACHE);
	gcmVERIFY_OBJECT(Brush, gcvOBJ_BRUSH);

	/* Try to find the node of the brush. */
	node = _FindByOb(BrushCache->brushHead, Brush);

	if (node == gcvNULL)
	{
		/* The brush does not seem to belong to this cache. */
		gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_ARGUMENT);
		return gcvSTATUS_INVALID_ARGUMENT;
	}

	/* Decrement the usage count. */
	node->usageCount--;

	/* Not used anymore? */
	if (node->usageCount <= 0)
	{
		/* Remove the node from the list. */
		_DisconnectNode((gcsBRUSH_LIST_PTR*)&BrushCache->brushHead,
						(gcsBRUSH_LIST_PTR*)&BrushCache->brushTail,
						(gcsBRUSH_LIST_PTR)node);

		/* Free up the cache node. */
		if ((node->cacheNode != gcvNULL) &&
			(node->cacheNode != CACHELESS))
		{
			/* Increment the free node count. */
			BrushCache->curFree++;

			/* Mark the cache as free. */
			node->cacheNode->brushNode = gcvNULL;

			/* Move the cache node to the end of the list. */
			_MoveToTail((gcsBRUSH_LIST_PTR*)&BrushCache->cacheHead,
						(gcsBRUSH_LIST_PTR*)&BrushCache->cacheTail,
						(gcsBRUSH_LIST_PTR)node->cacheNode);
		}

		/* Reset last flushed. */
		if (BrushCache->lastFlushed == node)
		{
			BrushCache->lastFlushed = gcvNULL;
		}

		/* Delete the brush. */
		gcmVERIFY_OK(gcoBRUSH_Delete(node->brush));

		/* Delete the node. */
		gcmVERIFY_OK(gcoOS_Free(BrushCache->hal->os, node));
	}

	/* Success. */
	gcmFOOTER_NO();
	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcoBRUSH_CACHE_FlushBrush
**
**	Flush the brush.
**
**	INPUT:
**
**		gcoBRUSH_CACHE BrushCache
**			Pointer to a valid gcoBRUSH_CACHE object.
**
**		gcoBRUSH Brush
**			Pointer to a valid brush to be deleted from the cache.
**
**	OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoBRUSH_CACHE_FlushBrush(
	IN gcoBRUSH_CACHE BrushCache,
	IN gcoBRUSH Brush
	)
{
	gceSTATUS status;
	gcsBRUSH_NODE_PTR brushNode;
	gcsCACHE_NODE_PTR cacheNode;
	gcsSURF_NODE_PTR memoryNode;
	gctBOOL upload;

	gcmHEADER_ARG("BrushCache=0x%x Brush=0x%x", BrushCache, Brush);

	/* Verify the arguments. */
	gcmVERIFY_OBJECT(BrushCache, gcvOBJ_BRUSHCACHE);
	gcmVERIFY_OBJECT(Brush, gcvOBJ_BRUSH);

	do
	{
		/* Try to find the node of the brush. */
		brushNode = _FindByOb(BrushCache->brushHead, Brush);

		if (brushNode == gcvNULL)
		{
			/* The brush does not seem to belong to this cache. */
			status = gcvSTATUS_INVALID_ARGUMENT;
			break;
		}

		/* Get a pointer to the cache node. */
		cacheNode = brushNode->cacheNode;
		upload = gcvFALSE;

		/* Does the brush node have a cache node assigned? */
		if (cacheNode == gcvNULL)
		{
			/* Acquire a node. */
			status = _GetCacheNode(BrushCache, &cacheNode);

			if (status != gcvSTATUS_OK)
			{
				/* Error. */
				break;
			}

			/* Link cache and brush nodes. */
			cacheNode->brushNode = brushNode;
			brushNode->cacheNode = cacheNode;

			/* Update free cache node count. */
			BrushCache->curFree--;

			/* Set dirty cache flag. */
			upload = gcvTRUE;
		}

		/* Move the cache node to the beginning of the list. */
		if (cacheNode == CACHELESS)
		{
			/* Nothing to move for cacheless, just reset. */
			memoryNode = gcvNULL;
		}
		else
		{
			/* Move the node. */
			_MoveToHead((gcsBRUSH_LIST_PTR*)&BrushCache->cacheHead,
						(gcsBRUSH_LIST_PTR*)&BrushCache->cacheTail,
						(gcsBRUSH_LIST_PTR)cacheNode);

			/* Set the memory node. */
			memoryNode = &cacheNode->patternNode;
		}

		/* Flush the brush. */
		if (BrushCache->lastFlushed != brushNode)
		{
			/* Flush the brush. */
			status = gcoBRUSH_FlushBrush(Brush, upload, memoryNode);

			/* Update last flushed. */
			if (status == gcvSTATUS_OK)
			{
				BrushCache->lastFlushed = brushNode;
			}
		}
		else
		{
			status = gcvSTATUS_OK;
		}
	}
	while (gcvFALSE);

	/* Return status. */
	gcmFOOTER();
	return status;
}

