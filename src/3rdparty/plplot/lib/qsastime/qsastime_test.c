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
#include "qsastime_extra.h"
#include "qsastimeP.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
// Test program to run MJDtime

int main()
{
    char       buf[360];
    char       copy[360];
    int        y    = 2004;
    int        m    = 0;
    int        d    = 23;
    int        hour = 13;
    int        min  = 39;
    double     sec  = 2.345678901;
    const char *ISOstring;
    double     epoch;
    int        doy, ifleapyear, y1;
    double     zeroTest;

    size_t     used;
    time_t     localt;
    struct tm  *ptm;

    MJDtime    MJD1;
    MJDtime    MJD2;


    printf( "Start date/time components: %d-%d-%d %d:%d:%13.11g\n", y, m + 1, d, hour, min, sec );

    setFromUT( y, m, d, hour, min, sec, &MJD2, 0 );

    breakDownMJD( &y, &m, &d, &hour, &min, &sec, &MJD2, 0 );
    printf( "date/time components: %d-%d-%d %d:%d:%13.11g\n\n", y, m + 1, d, hour, min, sec );

    printf( "MJD = %d, seconds = %17.15g\n", MJD2.base_day, MJD2.time_sec );
    printf( " MJD = %18.10f \n", getMJD( &MJD2 ) );
    printf( " JD = %18.10f \n\n", getJD( &MJD2 ) );

    ISOstring = getISOString( &MJD2, 0, 0 );
    printf( "ISO string = '%s'\n\n", ISOstring );

    strfMJD( &( buf[0] ), 360, "%Y-%m-%d %H:%M:%S%.", &MJD2, 0, 0 );
    printf( "strfMJD:ISO equiv:   '%s'\n", buf );
    strfMJD( &( buf[0] ), 360, "%Y-%m-%dT%H:%M:%S%4Z", &MJD2, 0, 0 );
    printf( "strfMJD:ISO equiv:  '%s'\n", buf );

    getYAD( &y1, &ifleapyear, &doy, &MJD2, 0 );
    setFromDOY( y, doy, hour, min, sec, &MJD1, 0 );

    zeroTest = getDiffSecs( &MJD1, &MJD2 );
    printf( "difference MJD (month/day) - MJD(doy) '%g'\n\n", zeroTest );

    hour += 24;
    d    -= 1;
    setFromUT( y, m, d, hour, min, sec, &MJD2, 0 );
    zeroTest = getDiffDays( &MJD1, &MJD2 );
    printf( "difference MJD (d-1, h+24) - MJD(d, h) '%g'\n\n", zeroTest );

    epoch = getCDFepoch( &MJD2 );
    printf( "CDF epoch sec %18.3f\n", epoch );
    setFromCDFepoch( epoch, &MJD2 );
    printf( "from CDF ISO string (CDF epoch is accurate to msec only) = '%s'\n", getISOString( &MJD2, 1, 0 ) );

    printf( "Day of week is/was %s\n\n", getDayOfWeek( &MJD2 ) );

    ISOstring = getISOString( &MJD1, 0, 0 );
    printf( "ISO string = '%s'\n\n", ISOstring );
    setFromISOstring( ISOstring, &MJD1, 0 );
    printf( "for %s, MJD = %d, seconds = %17.11g\n", ISOstring, MJD1.base_day, MJD1.time_sec );
    ISOstring = getISOString( &MJD1, 1, 0 );
    printf( "ISO string = '%s'\n\n", ISOstring );
    setFromISOstring( ISOstring, &MJD1, 0 );
    printf( "for %s, MJD = %d, seconds = %17.11g\n\n", ISOstring, MJD1.base_day, MJD1.time_sec );

    // try julian/gregorian changeover
    y    = 1752;
    m    = 8;
    d    = 14;
    hour = 0;

    setFromUT( y, m, d, hour, min, sec, &MJD1, 0 );
    strcpy( &( copy[0] ), getISOString( &MJD1, 0, 0 ) ); // copy because getISOString() returns a pointer to a static string
    printf( "Gregorian = '%s'\n", &( copy[0] ) );
    //setFromUT(y, m, d, hour, min, sec, &MJD1, 1); // set from Julian date
    printf( "%s Julian = '%s' Gregorian, (give us back our 11 days)\n", getISOString( &MJD1, 1, 1 ), &( copy[0] ) );


    // Compare formatting from strftime()


    used = strfMJD( &( buf[0] ), 360, "   strfMJD():\n   --------\n '%+' \n %c\n %D %F \n %j \n %r \n %s \n %v\n\n", &MJD2, 0, 0 );
    printf( "chars %d for \n%s\n", (int) used, buf );

    // seconds since 01 Jan 1970 Gregorian for strftime use
    localt = (int) MJD2.time_sec + ( MJD2.base_day - 40587 ) * 86400;
    ptm    = gmtime( &localt );
#ifndef _MSC_VER
    // note %s not implemented in cygwin 1.5 gcc 3.x nothing printed
    strftime( &( buf[0] ), 360,
        "  strftime(): (invalid before 1970 on some systems and subject to time zones on all systems)\n   ------\n '%a %b %e %H:%M:%S UTC %Y' \n %c\n %D %F \n %j \n %r \n %s \n %e-%b-%Y", ptm );
#else
    // the following format options are not defined in MSVC (2008)
    // and are replaced as follows
    // %e -> %d     will print as 01 etc
    // %D -> %m/%d/%y
    // %F -> %Y-%m-%d
    // %r -> %I:%M:%S %p
    // %s -> %%s - turned into literal as not implemented
    //
    // also %c and %p are locale dependent
    //
    strftime( &( buf[0] ), 360,
        "  strftime(): (invalid before 1970)\n   ------\n '%a %b %d %H:%M:%S UTC %Y' \n %c\n %m/%d/%y %Y-%m-%d \n %j \n %I:%M:%S %p \n %%s not implemented \n %d-%b-%Y", ptm );
#endif
    printf( "%s\n", buf );
}
