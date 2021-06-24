/* win32_compat.c: Win32 compatibility types and functions

   Copyright (C) 2009 Philip de Nier <philipn@users.sourceforge.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Philip de Nier
*/

#ifndef _LIBSVG_WIN32_COMPAT_H_
#define _LIBSVG_WIN32_COMPAT_H_


/* inttypes.h or stdint.h */

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned long int uint32_t;
typedef unsigned __int64 uint64_t;

typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed long int int32_t;
typedef __int64 int64_t;


/* sys/param.h */

#define MAXPATHLEN  _MAX_PATH


/* unistd.h */

#include <direct.h>
#include <io.h>

#define dup         _dup
#define getcwd      _getcwd
#define chdir       _chdir



/* libgen.h */

#include <stdlib.h>

static char *
dirname (char *path)
{
    char drive[_MAX_DRIVE];
    static char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];

    _splitpath (path, drive, dir, fname, ext);

    return dir;
}


#endif

