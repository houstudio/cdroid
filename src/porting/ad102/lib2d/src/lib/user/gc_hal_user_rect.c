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
**	gcsRECT_PTR structre for user HAL layers.
**
*/

#include "gc_hal_user_precomp.h"

#define _GC_OBJ_ZONE		gcvZONE_HAL

/******************************************************************************\
******************************** gcsRECT_PTR API Code ******************************
\******************************************************************************/

/*******************************************************************************
**
**	gcsRECT_Set
**
**	Initialize rectangle structure.
**
**	INPUT:
**
**		gctINT32 Left
**		gctINT32 Top
**		gctINT32 Right
**		gctINT32 Bottom
**			Coordinates of the rectangle to set.
**
**	OUTPUT:
**
**		gcsRECT_PTR Rect
**			Initialized rectangle structure.
*/
gceSTATUS
gcsRECT_Set(
	OUT gcsRECT_PTR Rect,
	IN gctINT32 Left,
	IN gctINT32 Top,
	IN gctINT32 Right,
	IN gctINT32 Bottom
	)
{
	gcmHEADER_ARG("Left=%d Top=%d Right=%d Bottom=%d",
				  Left, Top, Right, Bottom);

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(Rect != gcvNULL);

    /* Set coordinates. */
	Rect->left = Left;
	Rect->top = Top;
	Rect->right = Right;
	Rect->bottom = Bottom;

    /* Success. */
    gcmFOOTER_ARG("Rect=0x%x", Rect);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcsRECT_Width
**
**	Return the width of the rectangle.
**
**	INPUT:
**
**		gcsRECT_PTR Rect
**			Pointer to a valid rectangle structure.
**
**	OUTPUT:
**
**		gctINT32 * Width
**			Pointer to a variable that will receive the width of the rectangle.
*/
gceSTATUS
gcsRECT_Width(
	IN gcsRECT_PTR Rect,
    OUT gctINT32 * Width
	)
{
	gcmHEADER_ARG("Rect=0x%x", Rect);

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(Rect != gcvNULL);
    gcmVERIFY_ARGUMENT(Width != gcvNULL);

    /* Compute and return width. */
	*Width = Rect->right - Rect->left;

    /* Success. */
    gcmFOOTER_ARG("*Width=%d", *Width);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcsRECT_Height
**
**	Return the height of the rectangle.
**
**	INPUT:
**
**		gcsRECT_PTR Rect
**			Pointer to a valid rectangle structure.
**
**	OUTPUT:
**
**		gctINT32 * Height
**			Pointer to a variable that will receive the height of the rectangle.
*/
gceSTATUS
gcsRECT_Height(
	IN gcsRECT_PTR Rect,
    OUT gctINT32 * Height
	)
{
	gcmHEADER_ARG("Rect=0x%x", Rect);

	/* Verify the arguments. */
	gcmVERIFY_ARGUMENT(Rect != gcvNULL);
	gcmVERIFY_ARGUMENT(Height != gcvNULL);

	/* Compute and return height. */
	*Height = Rect->bottom - Rect->top;

    /* Success. */
    gcmFOOTER_ARG("*Width=%d", *Height);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcsRECT_Normalize
**
**	Ensures that top left corner is to the left and above the right bottom.
**
**	INPUT:
**
**		gcsRECT_PTR Rect
**			Pointer to a valid rectangle structure.
**
**	OUTPUT:
**
**		gcsRECT_PTR Rect
**			Normalized rectangle.
*/
gceSTATUS
gcsRECT_Normalize(
	IN OUT gcsRECT_PTR Rect
	)
{
	gctINT32 temp;

	gcmHEADER_ARG("Rect=0x%x", Rect);

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(Rect != gcvNULL);

	if (Rect->left > Rect->right)
	{
        /* Swap left and right coordinates. */
		temp = Rect->left;
		Rect->left = Rect->right;
		Rect->right = temp;
	}

	if (Rect->top > Rect->bottom)
	{
        /* Swap top and bottom coordinates. */
		temp = Rect->top;
		Rect->top = Rect->bottom;
		Rect->bottom = temp;
	}

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcsRECT_IsEqual
**
**	Compare two rectangles.
**
**	INPUT:
**
**		gcsRECT_PTR Rect1
**			Pointer to a valid rectangle structure.
**
**		gcsRECT_PTR Rect2
**			Pointer to a valid rectangle structure.
**
**	OUTPUT:
**
**		gctBOOL * Equal
**          Poniter to a variable that will receive a gcvTRUE when the rectangles
**          are equal or gcvFALSE when they are not.
*/
gceSTATUS
gcsRECT_IsEqual(
	IN gcsRECT_PTR Rect1,
	IN gcsRECT_PTR Rect2,
    OUT gctBOOL * Equal
	)
{
	gcmHEADER_ARG("Rect1=0x%x Rect2=0x%x", Rect1, Rect2);

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(Rect1 != gcvNULL);
    gcmVERIFY_ARGUMENT(Rect2 != gcvNULL);
    gcmVERIFY_ARGUMENT(Equal != gcvNULL);

    /* Compute and return equality. */
	*Equal = (Rect1->left == Rect2->left) &&
             (Rect1->top == Rect2->top) &&
             (Rect1->right == Rect2->right) &&
		     (Rect1->bottom == Rect2->bottom);

    /* Success. */
    gcmFOOTER_ARG("*Equal=%d", *Equal);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**	gcsRECT_IsOfEqualSize
**
**	Compare the sizes of two rectangles.
**
**	INPUT:
**
**		gcsRECT_PTR Rect1
**			Pointer to a valid rectangle structure.
**
**		gcsRECT_PTR Rect2
**			Pointer to a valid rectangle structure.
**
**	OUTPUT:
**
**		gctBOOL * EqualSize
**           Pointer to a variable that will receive gcvTRUE when the rectangles
**          are of equal size or gcvFALSE if tey are not.
*/
gceSTATUS
gcsRECT_IsOfEqualSize(
	IN gcsRECT_PTR Rect1,
	IN gcsRECT_PTR Rect2,
    OUT gctBOOL * EqualSize
	)
{
	gcmHEADER_ARG("Rect1=0x%x Rect2=0x%x", Rect1, Rect2);

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(Rect1 != gcvNULL);
    gcmVERIFY_ARGUMENT(Rect2 != gcvNULL);
    gcmVERIFY_ARGUMENT(EqualSize != gcvNULL);

    /* Commpute and return equality. */
	*EqualSize =
	    ((Rect1->right - Rect1->left) == (Rect2->right - Rect2->left)) &&
	    ((Rect1->bottom - Rect1->top) == (Rect2->bottom - Rect2->top));

    /* Success. */
    gcmFOOTER_ARG("*EqualSize=%d", *EqualSize);
    return gcvSTATUS_OK;
}

