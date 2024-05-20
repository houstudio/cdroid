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




#include "gc_hal_user_linux.h"
#include <math.h>

gctUINT32
gcoMATH_Log2in5dot5(
	IN gctINT X
	)
{
	return (X > 0)
		? (gctUINT32) (logf(X) / logf(2.0f) * 32.0f)
		: 0;
}

gctFLOAT
gcoMATH_Sine(
	IN gctFLOAT X
	)
{
	return sinf(X);
}

gctFLOAT
gcoMATH_Cosine(
	IN gctFLOAT X
	)
{
	return cosf(X);
}

gctFLOAT
gcoMATH_Floor(
	IN gctFLOAT X
	)
{
	return floorf(X);
}

gctFLOAT
gcoMATH_Ceiling(
	IN gctFLOAT X
	)
{
	return ceilf(X);
}

gctFLOAT
gcoMATH_SquareRoot(
	IN gctFLOAT X
	)
{
	return sqrtf(X);
}

gctFLOAT
gcoMATH_Log2(
	IN gctFLOAT X
	)
{
	return logf(X) / logf(2.0f);
}

gctFLOAT
gcoMATH_Power(
	IN gctFLOAT X,
	IN gctFLOAT Y
	)
{
	return powf(X, Y);
}

gctFLOAT
gcoMATH_Modulo(
	IN gctFLOAT X,
	IN gctFLOAT Y
	)
{
	return fmodf(X, Y);
}

gctFLOAT
gcoMATH_Exp(
	IN gctFLOAT X
	)
{
	return expf(X);
}

gctFLOAT
gcoMATH_Absolute(
	IN gctFLOAT X
	)
{
	return fabsf(X);
}

gctFLOAT
gcoMATH_ArcCosine(
	IN gctFLOAT X
	)
{
	return acosf(X);
}

gctFLOAT
gcoMATH_Tangent(
	IN gctFLOAT X
	)
{
	return tanf(X);
}

gctFLOAT
gcoMATH_UInt2Float(
	IN gctUINT X
	)
{
	return (gctFLOAT) X;
}

gctUINT
gcoMATH_Float2UInt(
	IN gctFLOAT X
	)
{
	return (gctUINT) X;
}

gctFLOAT
gcoMATH_Multiply(
	IN gctFLOAT X,
	IN gctFLOAT Y
	)
{
	return X * Y;
}

