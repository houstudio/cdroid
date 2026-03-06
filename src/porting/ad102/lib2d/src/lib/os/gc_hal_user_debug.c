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
**	Debug code for hal user layers.
**
*/

#include "gc_hal_user_linux.h"
#include <stdlib.h>
/* Include this if you want to print thread IDs as well. */
/*#include <pthread.h>*/

/******************************************************************************\
******************************** Debug Variables *******************************
\******************************************************************************/

static gceSTATUS _lastError  = gcvSTATUS_OK;
static gctUINT32 _debugLevel = gcvLEVEL_ERROR;
static gctUINT32 _debugZones[16];
static FILE *    _debugFile;
static FILE *    _debugFileVS;
static FILE *    _debugFileFS;
static gctUINT32 _shaderFileType;
static int       _indent;

#define gcdBUFFER_SIZE 0
#if gcdBUFFER_SIZE
static unsigned char _buffer[gcdBUFFER_SIZE];
static unsigned long _bytes;
#endif

static void _Flush(
	IN gctFILE File
	)
{
#if gcdBUFFER_SIZE
	if (_bytes > 0)
	{
		fwrite(_buffer, _bytes, 1, (File == NULL) ? stderr : File);
		_bytes = 0;
	}
#endif
}

static void
_Print(
	IN gctFILE File,
	IN gctCONST_STRING Message,
	IN va_list Arguments
	)
{
	static char buffer[256];
	int n, i;

	/* Test for level decrease. */
	if (strncmp(Message, "--", 2) == 0)
	{
		_indent -= 2;
	}

#ifdef PTHREAD_ONCE_INIT
	i = snprintf(buffer, sizeof(buffer), "%08lx: ", pthread_self());
#else
	i = 0;
#endif

	/* Indent message. */
	for (n = i + _indent; i < n; ++i)
	{
		buffer[i] = ' ';
	}

	/* Print message to buffer. */
	n = vsnprintf(buffer + i, sizeof(buffer) - i, Message, Arguments);
	if ((n <= 0) || (buffer[i + n - 1] != '\n'))
	{
		/* Append new-line. */
		strncat(buffer, "\n", sizeof(buffer));
	}

	/* Output to file or debugger. */
#if gcdBUFFER_SIZE
	n = strlen(buffer);
	if (_bytes + n > gcdBUFFER_SIZE)
	{
		_Flush(File);
	}
	memcpy(_buffer + _bytes, buffer, n);
	_bytes += n;
#else
	fprintf((File == gcvNULL) ? stderr : File, buffer);
#endif

	/* Test for level increase. */
	if (strncmp(Message, "++", 2) == 0)
	{
		_indent += 2;
	}
}

/******************************************************************************\
********************************* Debug Macros *********************************
\******************************************************************************/

#define gcmDEBUGPRINT(FileHandle, Message) \
{ \
	va_list arguments; \
	\
	va_start(arguments, Message); \
	_Print(FileHandle, Message, arguments); \
	va_end(arguments); \
}

/******************************************************************************\
********************************** Debug Code **********************************
\******************************************************************************/

/*******************************************************************************
**
**	gcoOS_Print
**
**	Print a message.
**
**	INPUT:
**
**		gctSTRING Message
**			Pointer to message.
**
**		...
**			Optional arguments.
*/

void
gcoOS_Print(
	IN gctCONST_STRING Message,
	...
	)
{
	gcmDEBUGPRINT(_debugFile, Message);
}

/*******************************************************************************
**
**	gcoOS_DebugTrace
**
**	Send a leveled message to the debugger.
**
**	INPUT:
**
**		gctUINT32 Level
**			Debug level of message.
**
**		gctCONST_STRING Message
**			Pointer to message.
**
**		...
**			Optional arguments.
**
**	OUTPUT:
**
**		Nothing.
*/

void
gcoOS_DebugTrace(
	IN gctUINT32 Level,
	IN gctCONST_STRING Message,
	...
	)
{
	if (Level > _debugLevel)
	{
		return;
	}

	gcmDEBUGPRINT(_debugFile, Message);
}

/*******************************************************************************
**
**	gcoOS_DebugTraceZone
**
**	Send a leveled and zoned message to the debugger.
**
**	INPUT:
**
**		gctUINT32 Level
**			Debug level for message.
**
**		gctUINT32 Zone
**			Debug zone for message.
**
**		gctCONST_STRING Message
**			Pointer to message.
**
**		...
**			Optional arguments.
**
**	OUTPUT:
**
**		Nothing.
*/

void
gcoOS_DebugTraceZone(
	IN gctUINT32 Level,
	IN gctUINT32 Zone,
	IN gctCONST_STRING Message,
	...
	)
{
	/* Verify that the debug level and zone are valid. */
	if ((Level > _debugLevel)
	||  !(_debugZones[gcmZONE_GET_API(Zone)] & Zone & gcdZONE_MASK)
	)
	{
		return;
	}

	gcmDEBUGPRINT(_debugFile, Message);
}

/*******************************************************************************
**
**	gcoOS_DebugBreak
**
**	Break into the debugger.
**
**	INPUT:
**
**		Nothing.
**
**	OUTPUT:
**
**		Nothing.
*/

void
gcoOS_DebugBreak(
	void
	)
{
	gcmTRACE(gcvLEVEL_ERROR, "%s(%d)", __FUNCTION__, __LINE__);
}

/*******************************************************************************
**
**	gcoOS_DebugFatal
**
**	Send a message to the debugger and break into the debugger.
**
**	INPUT:
**
**		gctCONST_STRING Message
**			Pointer to message.
**
**		...
**			Optional arguments.
**
**	OUTPUT:
**
**		Nothing.
*/

void
gcoOS_DebugFatal(
	IN gctCONST_STRING Message,
	...
	)
{
	gcmDEBUGPRINT(_debugFile, Message);

	/* Break into the debugger. */
	gcoOS_DebugBreak();
}

/*******************************************************************************
**
**	gcoOS_SetDebugLevel
**
**	Set the debug level.
**
**	INPUT:
**
**		gctUINT32 Level
**			New debug level.
**
**	OUTPUT:
**
**		Nothing.
*/

void
gcoOS_SetDebugLevel(
	IN gctUINT32 Level
	)
{
	_debugLevel = Level;
}

/*******************************************************************************
**
**	gcoOS_SetDebugZone
**
**	Set the debug zone.
**
**	INPUT:
**
**		gctUINT32 Zone
**			New debug zone.
**
**	OUTPUT:
**
**		Nothing.
*/

void
gcoOS_SetDebugZone(
	IN gctUINT32 Zone
	)
{
	_debugZones[gcmZONE_GET_API(Zone)] = Zone;
}

/*******************************************************************************
**
**	gcoOS_SetDebugLevelZone
**
**	Set the debug level and zone.
**
**	INPUT:
**
**		gctUINT32 Level
**			New debug level.
**
**		gctUINT32 Zone
**			New debug zone.
**
**	OUTPUT:
**
**		Nothing.
*/

void
gcoOS_SetDebugLevelZone(
	IN gctUINT32 Level,
	IN gctUINT32 Zone
	)
{
	_debugLevel                        = Level;
	_debugZones[gcmZONE_GET_API(Zone)] = Zone;
}

/*******************************************************************************
**
**	gcoOS_SetDebugZones
**
**	Enable or disable debug zones.
**
**	INPUT:
**
**		gctBOOL Enable
**			Set to gcvTRUE to enable the zones (or the Zones with the current
**			zones) or gcvFALSE to disable the specified Zones.
**
**		gctUINT32 Zones
**			Debug zones to enable or disable.
**
**	OUTPUT:
**
**		Nothing.
*/

void
gcoOS_SetDebugZones(
	IN gctUINT32 Zones,
	IN gctBOOL Enable
	)
{
	if (Enable)
	{
		/* Enable the zones. */
		_debugZones[gcmZONE_GET_API(Zones)] |= Zones;
	}
	else
	{
		/* Disable the zones. */
		_debugZones[gcmZONE_GET_API(Zones)] &= ~Zones;
	}
}

/*******************************************************************************
**
**	gcoOS_Verify
**
**	Called to verify the result of a function call.
**
**	INPUT:
**
**		gceSTATUS Status
**			Function call result.
**
**	OUTPUT:
**
**		Nothing.
*/

void
gcoOS_Verify(
	IN gceSTATUS Status
	)
{
	_lastError = Status;
}

/*******************************************************************************
**
**	gcoOS_SetDebugFile
**
**	Open or close the debug file.
**
**	INPUT:
**
**		gcoOS Os
**			Pointer to gcoOS object.
**
**		gctCONST_STRING FileName
**			Name of debug file to open or gcvNULL to close the current debug
**			file.
**
**	OUTPUT:
**
**		Nothing.
*/

void
gcoOS_SetDebugFile(
	IN gctCONST_STRING FileName
	)
{
	_Flush(_debugFile);

	/* Close any existing file handle. */
	if (_debugFile != gcvNULL)
	{
		fclose(_debugFile);
		_debugFile = gcvNULL;
	}

	if (FileName != gcvNULL)
	{
		_debugFile = fopen(FileName, "w");
	}
}

/*******************************************************************************
**
**	gcoOS_SetDebugShaderFiles
**
**	Called to redirect shader debug output to file(s).
**  Passing gcvNULL argument closes previously open file handles.
**
**	INPUT:
**
**		gctCONST_STRING VSFileName
**			Vertex Shader Filename.
**
**		gctCONST_STRING FSFileName
**			Fragment Shader Filename.
**
**	OUTPUT:
**
**		Nothing.
*/

void
gcoOS_SetDebugShaderFiles(
	IN gctCONST_STRING VSFileName,
	IN gctCONST_STRING FSFileName
	)
{
	if (_debugFileVS != gcvNULL)
	{
		fclose(_debugFileVS);
		_debugFileVS = gcvNULL;
	}

	if (_debugFileFS != gcvNULL)
	{
		fclose(_debugFileFS);
		_debugFileFS = gcvNULL;
	}

	if (VSFileName != gcvNULL)
	{
		_debugFileVS = fopen(VSFileName, "w");
	}

	if (FSFileName != gcvNULL)
	{
		_debugFileFS = fopen(FSFileName, "w");
	}
}

/*******************************************************************************
**
**	gcoOS_SetDebugShaderFileType
**
**	Called to set debugging output to vertex/fragment shader file.
**
**	INPUT:
**
**		gctUINT32 ShaderType
**			0 for Vertex Shader, 1 for Fragment Shader.
**
**	OUTPUT:
**
**		Nothing.
*/

void
gcoOS_SetDebugShaderFileType(
	IN gctUINT32 ShaderType
	)
{
	if (ShaderType <= 1)
	{
		_shaderFileType = ShaderType;
	}
}

/*******************************************************************************
**
**	gcoOS_DebugShaderTrace
**
**	Dump a message to a shader file.
**
**	INPUT:
**
**		gctCONST_STRING Message
**			Pointer to message.
**
**		...
**			Optional arguments.
**
**	OUTPUT:
**
**		Nothing.
*/

void
gcoOS_DebugShaderTrace(
	IN gctCONST_STRING Message,
	...
	)
{
	FILE * file;

	/* Verify that the shader file handle is valid. */
	if (_shaderFileType && (_debugFileFS != gcvNULL))
	{
		file = _debugFileFS;
	}
	else if (!_shaderFileType && (_debugFileVS != gcvNULL))
	{
		file = _debugFileVS;
	}
	else
	{
		return;
	}

	gcmDEBUGPRINT(file, Message);
}

