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

// These utilities use the Gregorian calendar after 4 Oct 1582 (Julian) i.e. from 15 Oct 1582 Gregoriam
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

#include "qsastime_extra.h"

static double MJDtoJD  = 2400000.5;
static double SecInDay = 86400; // we ignore leap seconds

int setFromISOstring( const char* ISOstring, MJDtime *MJD, int forceJulian )
{
    double seconds;
    int    y, m, d, h, min;
    int    startAt = 0;
    int    len     = strlen( ISOstring );

    // ISO is "1995-01-23 02:33:17.235" or "1995-01-23T02:33:17.235Z"

    // parse off year

    y = strtol( &( ISOstring[startAt] ), NULL, 10 );
    if ( ISOstring[startAt] == '-' || ISOstring[startAt] == '+' )
        startAt++;
    startAt += 5;
    if ( startAt > len )
        return 1;

    m        = strtol( &( ISOstring[startAt] ), NULL, 10 );
    startAt += 3;
    if ( startAt > len )
        return 1;

    d        = strtol( &( ISOstring[startAt] ), NULL, 10 );
    startAt += 3;
    if ( startAt > len )
        return 1;

    h        = strtol( &( ISOstring[startAt] ), NULL, 10 );
    startAt += 3;
    if ( startAt > len )
        return 1;

    min      = strtol( &( ISOstring[startAt] ), NULL, 10 );
    startAt += 3;
    if ( startAt > len )
        return 1;

    seconds = strtod( &( ISOstring[startAt] ), NULL );
    setFromUT( y, m - 1, d, h, min, seconds, MJD, forceJulian );

    return 0;
}


void setFromDOY( int year, int doy, int hour, int min, double sec, MJDtime *MJD, int forceJulian )
{
    // Set from Day Of Year format

    // convert Gregorian date plus time to MJD
    // MJD measures from the start of 17 Nov 1858

    // the int flag forceJulian forces use of Julian  calendar whatever the year
    // default is to use Gregorian after 4 Oct 1582 (Julian) i.e. from 15 Oct 1582 Gregorian
    // Note C libraries use Gregorian only from 14 Sept 1752 onwards

    int leaps, lastyear, extraDays;

    // N.B. There were known problems (both for the Julian and Gregorian
    // cases) with the following leap year logic that were completely fixed
    // in qsastime.c, but I (AWI) am not going to bother with these fixups
    // here since this code only used for a specific test routine for limited
    // date range and not for anything general.
    if ( forceJulian && year <= 0 )
    {
        // count leap years on Julian Calendar
        // MJD for Jan 1 0000 (correctly Jan 01, BCE 1) is  - 678943, count from there
        // negative CE (AD) years convert to BCE (BC) as  BCE = 1 - CE, e.g. 2 BCE = -1 CE

        leaps         = ( year - 4 ) / 4; // (note leaps is negative here and year 0 (1 BCE) was a leap year
        MJD->base_day = year * 365 + leaps + doy - 678943;
    }
    else if ( forceJulian )
    {
        // count leap years on Julian Calendar
        // MJD for Jan 1 0000 (correctly Jan 01, BCE 1) is  - 678943, count from there

        leaps         = ( year - 1 ) / 4;
        MJD->base_day = year * 365 + leaps + doy - 678943;
    }
    else
    {
        // count leap years Gregorian Calendar - modern dates
        // Algorithm below for  17 Nov 1858 (0 MJD) gives
        // leaps = 450 and hence base_day of 678941, so subtract it to give MJD day

        lastyear      = year - 1;
        leaps         = lastyear / 4 - lastyear / 100 + lastyear / 400;
        MJD->base_day = year * 365 + leaps + doy - 678941;
    }

    MJD->time_sec = sec + ( (double) min + (double) hour * 60. ) * 60.;

    if ( MJD->time_sec >= SecInDay )
    {
        extraDays      = (int) ( MJD->time_sec / SecInDay );
        MJD->base_day += extraDays;
        MJD->time_sec -= extraDays * SecInDay;
    }

    return;
}


void setFromBCE( int yearBCE, int month, int day, int hour, int min, double sec, MJDtime *MJD, int forceJulian )
{
    // utility to allow user to input dates BCE (BC)

    int year = 1 - yearBCE;
    setFromUT( year, month, day, hour, min, sec, MJD, forceJulian );
}

void setFromMJD( double ModifiedJulianDate, MJDtime *MJD )
{
    // convert MJD double into MJD structure
    MJD->base_day = (int) ModifiedJulianDate;
    MJD->time_sec = ( ModifiedJulianDate - MJD->base_day ) * SecInDay;
}

void setFromJD( double JulianDate, MJDtime *MJD )
{
    // break JD double into MJD based structure
    // Note Julian Day starts Noon, so convert to MJD first

    MJD->base_day = (int) ( JulianDate - MJDtoJD );
    MJD->time_sec = ( JulianDate - MJDtoJD - (double) MJD->base_day ) * SecInDay;
}

void setFromCDFepoch( double cdfepoch, MJDtime *MJD )
{
    // convert cdf epoch double into MJD structure
    // Note that cdfepoch is msec from 0 AD on the Gregorian calendar

    double seconds = cdfepoch * 0.001;

    MJD->base_day  = (int) ( seconds / 86400.0 );
    MJD->time_sec  = seconds - MJD->base_day * SecInDay;
    MJD->base_day -= 678941;
}

double getCDFepoch( MJDtime *MJD )
{
    // convert MJD structure into cdf epoch double
    // Note that cdfepoch is msec from 0 AD on the Gregorian Calendar

    int    days    = MJD->base_day + 678941;
    double seconds = days * SecInDay + MJD->time_sec;
    return seconds * 1000.;
}

double getMJD( MJDtime *MJD )
{
    // Return MJD as a double
    return (double) MJD->base_day + MJD->time_sec / SecInDay;
}

double getJD( MJDtime *MJD )
{
    // Return JD as a double
    double JD = getMJD( MJD ) + MJDtoJD;
    return JD;
}

double getDiffDays( MJDtime *MJD1, MJDtime *MJD2 )
{
    // Return difference MJD1 - MJD2 in days as a double
    double diff = (double) ( MJD1->base_day - MJD2->base_day ) + ( MJD1->time_sec - MJD2->time_sec ) / SecInDay;
    return diff;
}

double getDiffSecs( MJDtime *MJD1, MJDtime *MJD2 )
{
    // Return difference MJD1 - MJD2 in seconds as a double
    double diff = (double) ( MJD1->base_day - MJD2->base_day ) * SecInDay + ( MJD1->time_sec - MJD2->time_sec );
    return diff;
}

const char * getISOString( MJDtime* MJD, int delim, int forceJulian )
{
    // ISO time string for UTC
    // uses default behaviour for Julian/Gregorian switch over
    //**
    //  Warning getISOString is not thread safe
    //  as it writes to a static variable DateTime
    //*

    static char DateTime[50];
    int         y, m, d, hour, min;
    int         sec1, ysign;
    double      sec;
    int         slen;
    char        * ptr;

    breakDownMJD( &y, &m, &d, &hour, &min, &sec, MJD, forceJulian );

    if ( y < 0 )
    {
        ysign = 1;
        y     = -y;
    }
    else
        ysign = 0;

    sec1 = (int) sec / 10;
    sec -= (double) sec1 * 10;

    if ( delim == 1 )
    {
        if ( ysign == 0 )
            sprintf( DateTime, "%04d-%02d-%02dT%02d:%02d:%01d%-11.10f", y, m + 1, d, hour, min, sec1, sec );
        else
            sprintf( DateTime, "-%04d-%02d-%02dT%02d:%02d:%01d%-11.10f", y, m + 1, d, hour, min, sec1, sec );

        // remove trailing white space
        while ( ( ptr = strrchr( &( DateTime[0] ), ' ' ) ) != NULL )
            ptr[0] = '\0';
        strcat( &( DateTime[0] ), "Z" );
    }
    else
    {
        if ( ysign == 0 )
            sprintf( DateTime, "%04d-%02d-%02d %02d:%02d:%01d%-11.10f", y, m + 1, d, hour, min, sec1, sec );
        else
            sprintf( DateTime, "-%04d-%02d-%02d %02d:%02d:%01d%-11.10f", y, m + 1, d, hour, min, sec1, sec );

        // remove trailing white space
        slen = strlen( DateTime ) - 1;
        while ( DateTime[slen] == ' ' )
        {
            DateTime[slen] = '\0';
            slen--;
        }
    }
    return &( DateTime[0] );
}
