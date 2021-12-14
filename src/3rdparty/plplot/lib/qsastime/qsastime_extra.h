#ifndef _QSASTIME_EXTRA_H_
#define _QSASTIME_EXTRA_H_
//
// This software originally contributed under the LGPL in January 2009 to
// PLplot by the
// Cluster Science Centre
// QSAS team,
// Imperial College, London
// Copyright (C) 2009 Imperial College, London
//
// This file is part of PLplot.
//
// PLplot is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as published
// by the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// PLplot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with PLplot; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
//

// MJD measures from the start of 17 Nov 1858

// These utilities use the Gregorian calendar after 4 Oct 1582 (Julian) i.e. from 15 Oct 1582 Gregorian
// Note C libraries use Gregorian only from 14 Sept 1752
// More detailed discussion can be found at http://aa.usno.navy.mil/data/docs/JulianDate.php
// These routines have been compared with the results of the US Naval Observatory online converter.
// Modified Julian Date (MJD) = Julian Date (JD) - 2400000.5
//
// In all routines, specifying a day, hour, minute or second field greater than would be valid is
// handled with modulo arithmetic and safe.
// Thus 2006-12-32 00:62:00.0 will safely, and correctly, be treated as 2007-01-01 01:02:00.0
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// include header file for dll definitions
#include "qsastimeP.h"

QSASTIMEDLLIMPEXP void setFromDOY( int year, int doy, int hour, int min, double sec, MJDtime *MJD, int forceJulian );
QSASTIMEDLLIMPEXP void setFromBCE( int yearBCE, int month, int day, int hour, int min, double sec, MJDtime *MJD, int forceJulian );
QSASTIMEDLLIMPEXP void setFromMJD( double ModifiedJulianDate, MJDtime *MJD );
QSASTIMEDLLIMPEXP void setFromCDFepoch( double cdfepoch, MJDtime *MJD );
QSASTIMEDLLIMPEXP void setFromJD( double JulianDate, MJDtime *MJD );
QSASTIMEDLLIMPEXP int setFromISOstring( const char* ISOstring, MJDtime *MJD, int forceJulian );
QSASTIMEDLLIMPEXP double getMJD( MJDtime *MJD );
QSASTIMEDLLIMPEXP double getJD( MJDtime *MJD );
QSASTIMEDLLIMPEXP double getDiffDays( MJDtime *MJD1, MJDtime *MJD2 );
QSASTIMEDLLIMPEXP double getDiffSecs( MJDtime *MJD1, MJDtime *MJD2 );
QSASTIMEDLLIMPEXP double getCDFepoch( MJDtime *MJD );
QSASTIMEDLLIMPEXP const char * getISOString( MJDtime *MJD, int delim, int forceJulian );
//** Warning getISOString is not thread safe **

#endif
