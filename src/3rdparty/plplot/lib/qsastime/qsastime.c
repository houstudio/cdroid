//
// This software originally contributed under the LGPL in January 2009 to
// PLplot by the
// Cluster Science Centre
// QSAS team,
// Imperial College, London
// Copyright (C) 2009 Imperial College, London
// Copyright (C) 2009 Alan W. Irwin
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
#include <ctype.h>
#include <math.h>
#include "qsastimeP.h"
#include "tai-utc.h"

// MJD for 0000-01-01 (correctly Jan 01, BCE 1)
// Julian proleptic calendar value.
#define MJD_0000J    -678943
// Gregorian proleptic calendar value.  (At MJD_0000J the Gregorian proleptic
// calendar reads two days behind the Julian proleptic calendar, i.e. - 2 days,
// see http://en.wikipedia.org/wiki/Proleptic_Gregorian_calendar,
// so MJD_0000G = MJD_0000J+2)
#define MJD_0000G    -678941
// MJD for 0001-01-01 which is 366 days later than previous definitions because
// the year 0 is a leap year in both calendars.
#define MJD_0001J    -678577
#define MJD_0001G    -678575
// MJD for Jan 01, 1970 00:00:00 Gregorian, the Unix epoch.
#define MJD_1970     40587

static const double SecInDay        = 86400; // we ignore leap seconds
static const int    MonthStartDOY[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
static const int    MonthStartDOY_L[] = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 };

// Static function declarations.
static int geMJDtime_TAI( const MJDtime *number1, const TAI_UTC *number2 );
static int geMJDtime_UTC( const MJDtime *number1, const TAI_UTC *number2 );
static double leap_second_TAI( const MJDtime *MJD_TAI, int *inleap, int *index );
// End of static function declarations.

int setFromUT( int year, int month, int day, int hour, int min, double sec, MJDtime *MJD, int forceJulian )
{
    // convert broken-down time to MJD
    // MJD measures from the start of 17 Nov 1858

    // If forceJulian is true (non-zero), the Julian proleptic calendar is
    // used whatever the year.  Otherwise, the Gregorian proleptic calendar
    // is used whatever the year.
    // Note C libraries use Gregorian only (at least on Linux).  In contrast,
    // the Linux (and Unix?) cal application uses Julian for earlier dates
    // and Gregorian from 14 Sept 1752 onwards.

    int    leaps, year4, year100, year400;
    double dbase_day, non_leaps = 365.;
    //int dbase_day, non_leaps = 365;
    double time_sec, dextraDays;
    int    extraDays;

    if ( month < 0 || month > 11 )
    {
        fprintf( stderr, "setfromUT: invalid month value\n" );
        exit( EXIT_FAILURE );
    }
    // As year increases, year4/4 increments by 1 at
    // year = -7, -3, 1, 5, 9, etc.
    // As year increases, year100/100 increments by 1 at
    // year = -299, -199, -99, 1, 101, 201, 301, etc.
    // As year increases, year400/400 increments by 1 at
    // year = -1199, -799, -399, 1, 401, 801, 1201, etc.
    if ( year <= 0 )
    {
        year4   = year - 4;
        year100 = year - 100;
        year400 = year - 400;
    }
    else
    {
        year4   = year - 1;
        year100 = year - 1;
        year400 = year - 1;
    }

    if ( forceJulian )
    {
        // count leap years on proleptic Julian Calendar starting from MJD_0000J
        leaps = year4 / 4;
        if ( year % 4 == 0 )
            dbase_day = year * non_leaps + leaps + MonthStartDOY_L[month] + day + MJD_0000J;
        else
            dbase_day = year * non_leaps + leaps + MonthStartDOY[month] + day + MJD_0000J;
    }
    else
    {
        // count leap years for proleptic Gregorian Calendar.
        // Algorithm below for 1858-11-17 (0 MJD) gives
        // leaps = 450 and hence dbase_day of 678941, so subtract that value
        // or add MJD_0000G (which is two days different from MJD_0000J, see
        // above).
        leaps = year4 / 4 - year100 / 100 + year400 / 400;

        // left to right associativity means the double value of
        // non_leaps propagate to make all calculations be
        // done in double precision without the potential of
        // integer overflow.  The result should be a double which
        // stores the expected exact integer results of the
        // calculation with exact representation unless the
        // result is much larger than the integer overflow limit.
        if ( ( year % 4 == 0 && year % 100 != 0 ) || ( year % 4 == 0 && year % 400 == 0 ) )
            dbase_day = year * non_leaps + leaps + MonthStartDOY_L[month] + day + MJD_0000G;
        else
            dbase_day = year * non_leaps + leaps + MonthStartDOY[month] + day + MJD_0000G;
    }

    time_sec = sec + ( (double) min + (double) hour * 60. ) * 60.;

    if ( time_sec >= SecInDay )
    {
        dextraDays = ( time_sec / SecInDay );
        // precaution against overflowing extraDays.
        if ( fabs( dextraDays ) > 2.e9 )
        {
            return 3;
        }
        extraDays  = (int) ( dextraDays );
        dbase_day += extraDays;
        time_sec  -= extraDays * SecInDay;
    }
    // precaution against overflowing MJD->base_day.
    if ( fabs( dbase_day ) > 2.e9 )
    {
        return 4;
    }
    else
    {
        // The exact integer result should be represented exactly in the
        // double, dbase_day, and its absolute value should be less than
        // the integer overflow limit.  So the cast to int should be
        // exact.
        MJD->base_day = (int) dbase_day;
        MJD->time_sec = time_sec;
        return 0;
    }
}

void getYAD( int *year, int *ifleapyear, int *doy, const MJDtime *MJD, int forceJulian )
{
    // Get year and day of year from normalized MJD

    int j, ifcorrect, year4, year100, year400;

    j = MJD->base_day;

    if ( forceJulian )
    {
        // Shift j epoch to 0000-01-01 for the Julian proleptic calendar.
        j -= MJD_0000J;

        // 365.25 is the exact period of the Julian year so year will be correct
        // if the day offset is set exactly right so that years -4, 0, 4 are
        // leap years, i.e. years -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5 start with
        // j =  -1826 -1461, -1095, -730, -365, 0, 366, 731, 1096, 1461, 1827
        if ( j >= 366 )
        {
            *year = (int) ( (double) ( j ) / 365.25 );
            year4 = *year - 1;
        }
        else
        {
            *year = (int) ( (double) ( j - 365 ) / 365.25 );
            year4 = *year - 4;
        }

        *doy = j - *year * 365 - year4 / 4;

        *ifleapyear = *year % 4 == 0;
    }
    else
    {
        // Shift j epoch to 0000-01-01 for the Gregorian proleptic calendar.
        j -= MJD_0000G;
        // 365.245 is the exact period of the Gregorian year so year will be correct
        // on average, but because the leap year rule is irregular within
        // the 400-year Gregorian cycle, the first and last days of the
        // year may need further adjustment.

        if ( j >= 366 )
        {
            *year   = (int) ( (double) ( j ) / 365.2425 );
            year4   = *year - 1;
            year100 = *year - 1;
            year400 = *year - 1;
        }
        else
        {
            *year   = (int) ( (double) ( j - 365 ) / 365.2425 );
            year4   = *year - 4;
            year100 = *year - 100;
            year400 = *year - 400;
        }

        *doy        = j - *year * 365 - year4 / 4 + year100 / 100 - year400 / 400;
        *ifleapyear = ( *year % 4 == 0 && *year % 100 != 0 ) || ( *year % 4 == 0 && *year % 400 == 0 );

        // Rare corrections to above average Gregorian relations.
        if ( *doy < 1 )
        {
            ( *year )--;
            ifcorrect = 1;
        }
        else if ( *doy > 365 && ( !*ifleapyear || *doy > 366 ) )
        {
            ( *year )++;
            ifcorrect = 1;
        }
        else
        {
            ifcorrect = 0;
        }
        if ( ifcorrect )
        {
            if ( j >= 366 )
            {
                year4   = *year - 1;
                year100 = *year - 1;
                year400 = *year - 1;
            }
            else
            {
                year4   = *year - 4;
                year100 = *year - 100;
                year400 = *year - 400;
            }

            *doy        = j - *year * 365 - year4 / 4 + year100 / 100 - year400 / 400;
            *ifleapyear = ( *year % 4 == 0 && *year % 100 != 0 ) || ( *year % 4 == 0 && *year % 400 == 0 );
        }
    }
}

void normalize_MJD( MJDtime *MJD )
{
    int extra_days;
    // Calculate MJDout as normalized version
    // (i.e., 0. <= MJDout->time_sec < 86400.) of MJDin.
    if ( MJD->time_sec >= 0 )
    {
        extra_days = (int) ( MJD->time_sec / SecInDay );
    }
    else
    {
        // allow for negative seconds push into previous day even if less than 1 day
        extra_days = (int) ( MJD->time_sec / SecInDay ) - 1;
    }

    MJD->base_day += extra_days;
    MJD->time_sec -= extra_days * SecInDay;
}

void breakDownMJD( int *year, int *month, int *day, int *hour, int *min, double *sec, const MJDtime *MJD, int forceJulian )
{
    // Convert MJD struct into date/time elements
    // Note year 0 CE (AD) [1 BCE (BC)] is a leap year

    int     doy, ifleapyear;
    MJDtime nMJD_value, *nMJD = &nMJD_value;

    *nMJD = *MJD;
    normalize_MJD( nMJD );
    // Time part

    *sec  = nMJD->time_sec;
    *hour = (int) ( *sec / 3600. );
    *sec -= (double) *hour * 3600.;
    *min  = (int) ( *sec / 60. );
    *sec -= (double) *min * 60.;

    getYAD( year, &ifleapyear, &doy, nMJD, forceJulian );

    // calculate month part with doy set to be the day within
    // the year in the range from 1 to 366
    *month = -1;
    if ( ifleapyear )
    {
        while ( doy > MonthStartDOY_L[*month + 1] )
        {
            ( *month )++;
            if ( *month == 11 )
                break;
        }
        *day = doy - MonthStartDOY_L[*month];
    }
    else
    {
        while ( doy > MonthStartDOY[*month + 1] )
        {
            ( *month )++;
            if ( *month == 11 )
                break;
        }
        *day = doy - MonthStartDOY[*month];
    }
}

const char * getDayOfWeek( const MJDtime *MJD )
{
    static const char *dow = { "Wed\0Thu\0Fri\0Sat\0Sun\0Mon\0Tue" };
    int d = MJD->base_day % 7;
    if ( d < 0 )
        d += 7;
    return &( dow[d * 4] );
}

const char * getLongDayOfWeek( const MJDtime *MJD )
{
    static const char *dow = { "Wednesday\0Thursday\0\0Friday\0\0\0\0Saturday\0\0Sunday\0\0\0\0Monday\0\0\0\0Tuesday" };
    int d = MJD->base_day % 7;
    if ( d < 0 )
        d += 7;
    return &( dow[d * 10] );
}

const char * getMonth( int m )
{
    static const char *months = { "Jan\0Feb\0Mar\0Apr\0May\0Jun\0Jul\0Aug\0Sep\0Oct\0Nov\0Dec" };
    return &( months[( m ) * 4] );
}

const char * getLongMonth( int m )
{
    static const char *months = { "January\0\0\0February\0\0March\0\0\0\0\0April\0\0\0\0\0May\0\0\0\0\0\0\0June\0\0\0\0\0\0July\0\0\0\0\0\0August\0\0\0\0September\0October\0\0\0November\0\0December" };
    return &( months[( m ) * 10] );
}


size_t strfMJD( char * buf, size_t len, const char *format, const MJDtime *MJD, int forceJulian, int inleap )
{
    // Format a text string according to the format string.
    // Uses the same syntax as strftime() but does not use current locale.
    // The null terminator is included in len for safety.

    // if inleap is true (non-zero) then renormalize so that (int) sec
    // is 60 to mark results as a flag that a leap increment (recently
    // always a second, but historically it was sometimes smaller than
    // that) was in the process of being inserted for this particular
    // epoch just prior to a positive discontinuity in TAI-UTC.

    int        year, month, day, hour, min, ysign, second, d, y;
    int        y1, ifleapyear;
    int        i, secsSince1970;
    int        nplaces, fmtlen, slen;
    int        resolution;
    double     shiftPlaces;
    char       * ptr;
    double     sec, sec_fraction;
    int        w, doy, days_in_wk1;
    const char *dayText;
    const char *monthText;
    char       DateTime[80];
    size_t     posn = 0;
    size_t     last = len - 1;
    MJDtime    nMJD_value, *nMJD = &nMJD_value;
    char       dynamic_format[10];

    // Find required resolution
    resolution = 0;
    fmtlen     = (int) strlen( format );
    i          = 0;
    while ( i < fmtlen )
    {
        char next = format[i];
        if ( next == '%' )
        {
            // find seconds format if used
            i++;
            next = format[i];
            if ( isdigit( next ) != 0 )
            {
                nplaces = (int) strtol( &( format[i] ), NULL, 10 );
                if ( nplaces > resolution )
                    resolution = nplaces;
            }
            else if ( next == '.' )
            {
                resolution = 9; // maximum resolution allowed
            }
        }
        i++;
    }

    // ensure rounding is done before breakdown
    shiftPlaces     = pow( 10, (double) resolution );
    *nMJD           = *MJD;
    nMJD->time_sec += 0.5 / shiftPlaces;

    buf[last] = '\0';
    buf[0]    = '\0'; // force overwrite of old buffer since strnctat() used hereafter

    if ( inleap )
        nMJD->time_sec -= 1.;

    breakDownMJD( &year, &month, &day, &hour, &min, &sec, nMJD, forceJulian );
    if ( inleap )
        sec += 1.;

    if ( year < 0 )
    {
        ysign = 1;
        year  = -year;
    }
    else
        ysign = 0;

    //truncate seconds to resolution to stop formatting rounding up
    sec    = floor( sec * shiftPlaces ) / shiftPlaces;
    second = (int) sec;

    // Read format string, character at a time
    i = 0;
    while ( i < fmtlen )
    {
        char next = format[i];
        if ( next == '%' )
        {
            // format character or escape
            i++;
            next = format[i];
            if ( next == '%' )
            {
                // escaped %, pass it on
                buf[posn] = next;
                posn++;
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'a' )
            {
                // short day name
                dayText = getDayOfWeek( nMJD );
                strncat( &( buf[posn] ), dayText, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'A' )
            {
                // long day name
                dayText = getLongDayOfWeek( nMJD );
                strncat( &( buf[posn] ), dayText, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'b' || next == 'h' )
            {
                // short month name
                monthText = getMonth( month );
                strncat( &( buf[posn] ), monthText, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'B' )
            {
                // long month name
                monthText = getLongMonth( month );
                strncat( &( buf[posn] ), monthText, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'c' )
            {
                // Date and Time with day of week
                dayText   = getDayOfWeek( nMJD );
                monthText = getMonth( month );
                if ( ysign == 0 )
                    sprintf( DateTime, "%s %s %02d %02d:%02d:%02d %04d", dayText, monthText, day, hour, min, second, year );
                else
                    sprintf( DateTime, "%s %s %02d %02d:%02d:%02d -%04d", dayText, monthText, day, hour, min, second, year );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'C' )
            {
                //  year / 100 so, e.g. 1989 is 20th century but comes out as 19
                int century = year / 100;
                if ( ysign == 0 )
                    sprintf( DateTime, "%02d", century );
                else
                    sprintf( DateTime, "-%02d", century + 1 );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'd' )
            {
                // day of month (01 - 31)
                sprintf( DateTime, "%02d", day );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'D' )
            {
                // month/day/year
                y = year % 100;
                if ( ysign == 0 )
                    sprintf( DateTime, "%02d/%02d/%02d", month + 1, day, y );
                else
                    sprintf( DateTime, "%02d/%02d/-%02d", month + 1, day, y );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'e' )
            {
                // day of month ( 1 - 31)
                if ( day < 10 )
                    sprintf( DateTime, " %01d", day );
                else
                    sprintf( DateTime, "%02d", day );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'F' )
            {
                // year-month-day
                if ( ysign == 0 )
                    sprintf( DateTime, "%04d-%02d-%02d", year, month + 1, day );
                else
                    sprintf( DateTime, "-%04d-%02d-%02d", year, month + 1, day );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'H' )
            {
                // hour, 24 hour clock (00 - 23)
                sprintf( DateTime, "%02d", hour );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'I' )
            {
                // hour, 12 hour clock (01 - 12)
                if ( hour == 0 )
                    sprintf( DateTime, "%02d", hour + 12 );
                else if ( hour > 12 )
                    sprintf( DateTime, "%02d", hour - 12 );
                else
                    sprintf( DateTime, "%02d", hour );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'j' )
            {
                // day of year
                getYAD( &y1, &ifleapyear, &doy, nMJD, forceJulian );
                sprintf( DateTime, "%03d", doy );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'k' )
            {
                // hour, 24 hour clock ( 0 - 23)
                if ( hour < 10 )
                    sprintf( DateTime, " %01d", hour );
                else
                    sprintf( DateTime, "%02d", hour );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'l' )
            {
                // hour, 12 hour clock ( 1 - 12)
                if ( hour == 0 )
                    sprintf( DateTime, "%02d", hour + 12 );
                else if ( hour < 10 )
                    sprintf( DateTime, " %01d", hour );
                else if ( hour <= 12 )
                    sprintf( DateTime, "%02d", hour );
                else if ( hour < 22 )
                    sprintf( DateTime, " %01d", hour - 12 );
                else
                    sprintf( DateTime, "%02d", hour - 12 );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'm' )
            {
                // month (01 - 12)
                sprintf( DateTime, "%02d", month + 1 );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'M' )
            {
                // minute (00 - 59)
                sprintf( DateTime, "%02d", min );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'n' )
            {
                //  newline
                buf[posn] = '\n';
                posn++;
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'p' )
            {
                // am/pm on12 hour clock
                if ( hour < 0 )
                    sprintf( DateTime, "AM" );
                else
                    sprintf( DateTime, "PM" );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'r' )
            {
                // hour:min:sec AM, 12 hour clock (01 - 12):(00 - 59):(00 - 59) (AM - PM)
                if ( hour == 0 )
                    sprintf( DateTime, "%02d:%02d:%02d AM", hour + 12, min, second );
                else if ( hour > 12 )
                    sprintf( DateTime, "%02d:%02d:%02d PM", hour - 12, min, second );
                else if ( hour == 12 )
                    sprintf( DateTime, "%02d:%02d:%02d PM", hour, min, second );
                else
                    sprintf( DateTime, "%02d:%02d:%02d AM", hour, min, second );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'R' )
            {
                // hour:min, 24 hour clock (00 - 23):(00 - 59)
                sprintf( DateTime, "%02d:%02d", hour, min );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'S' )
            {
                // second (00 - 59 with optional decimal point and numbers after the decimal point.)
                if ( i + 2 < fmtlen && format[i + 1] == '%' && ( format[i + 2] == '.' || isdigit( format[i + 2] ) != 0 ) )
                {
                    // nplaces is number of decimal places ( 0 < nplaces <= 9 )
                    if ( format[i + 2] == '.' )
                        // maximum number of places
                        nplaces = 9;
                    else
                        nplaces = (int) strtol( &( format[i + 2] ), NULL, 10 );
                    i += 2;
                }
                else
                {
                    nplaces = 0;
                }

                if ( nplaces == 0 )
                {
                    sprintf( DateTime, "%02d", (int) ( sec + 0.5 ) );
                }
                else
                {
                    sprintf( dynamic_format, "%%0%d.%df", nplaces + 3, nplaces );
                    sprintf( DateTime, dynamic_format, sec );
                    if ( format[i] == '.' )
                    {
                        slen = (int) strlen( DateTime ) - 1;
                        while ( DateTime[slen] == '0' && DateTime[slen - 1] != '.' )
                        {
                            DateTime[slen] = '\0'; // remove trailing zeros
                            slen--;
                        }
                    }
                }

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 's' )
            {
                // seconds since 01 Jan 1970 Gregorian
                secsSince1970 = (int) ( nMJD->time_sec + ( nMJD->base_day - MJD_1970 ) * SecInDay );
                sprintf( DateTime, "%d", secsSince1970 );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 't' )
            {
                //  tab
                buf[posn] = '\t';
                posn++;
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'T' )
            {
                // hour:min:sec, 24 hour clock (00 - 23):(00 - 59):(00 - 59)
                sprintf( DateTime, "%02d:%02d:%02d", hour, min, second );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'U' )
            {
                // week of year as a number,  (00 - 53) start of week is Sunday
                getYAD( &y1, &ifleapyear, &doy, nMJD, forceJulian );
                days_in_wk1 = ( nMJD->base_day - doy - 4 ) % 7;

                w = ( doy + 6 - days_in_wk1 ) / 7;

                sprintf( DateTime, "%02d", w );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'u' )
            {
                // weekday as a number,  0 = Monday
                d = 1 + ( nMJD->base_day - 5 ) % 7;

                sprintf( DateTime, "%01d", d );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'v' )
            {
                // day-MonthName-year day of month ( 1 - 31) - month (Jan ... Dec) - year (yyyy)

                monthText = getMonth( month );

                if ( ysign == 0 )
                {
                    if ( day < 10 )
                        sprintf( DateTime, " %01d-%s-%04d", day, monthText, year );
                    else
                        sprintf( DateTime, "%02d-%s-%04d", day, monthText, year );
                }
                else
                {
                    if ( day < 10 )
                        sprintf( DateTime, " %01d-%s-(-)%04d", day, monthText, year );
                    else
                        sprintf( DateTime, "%02d-%s-(-)%04d", day, monthText, year );
                }

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'V' )
            {
                // week of year as a number,  (01 - 53) start of week is Monday and first week has at least 3 days in year
                getYAD( &y1, &ifleapyear, &doy, nMJD, forceJulian );
                days_in_wk1 = ( nMJD->base_day - doy - 3 ) % 7;

                if ( days_in_wk1 <= 3 )
                    w = ( doy + 6 - days_in_wk1 ) / 7;                     // ensure first week has at least 3 days in this year
                else
                    w = 1 + ( doy + 6 - days_in_wk1 ) / 7;

                if ( w == 0 )
                    w = 53;
                sprintf( DateTime, "%02d", w );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'w' )
            {
                // weekday as a number,  0 = Sunday
                d = ( nMJD->base_day - 4 ) % 7;

                sprintf( DateTime, "%01d", d );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'W' )
            {
                // week of year as a number,  (00 - 53) start of week is Monday
                getYAD( &y1, &ifleapyear, &doy, nMJD, forceJulian );
                days_in_wk1 = ( nMJD->base_day - doy - 3 ) % 7;

                w = ( doy + 6 - days_in_wk1 ) / 7;

                sprintf( DateTime, "%02d", w );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'x' )
            {
                // date string
                dayText   = getDayOfWeek( nMJD );
                monthText = getMonth( month );
                if ( ysign == 0 )
                    sprintf( DateTime, "%s %s %02d, %04d", dayText, monthText, day, year );
                else
                    sprintf( DateTime, "%s %s %02d, -%04d", dayText, monthText, day, year );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'X' )
            {
                // time string
                sprintf( DateTime, "%02d:%02d:%02d", hour, min, second );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'y' )
            {
                // 2 digit year
                y = year % 100;

                if ( ysign == 0 )
                    sprintf( DateTime, "%02d", y );
                else
                    sprintf( DateTime, "-%02d", y );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'Y' )
            {
                // 4 digit year
                if ( ysign == 0 )
                    sprintf( DateTime, "%04d", year );
                else
                    sprintf( DateTime, "-%04d", year );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'Z' )
            {
                // time zone and calendar, always UTC
                if ( forceJulian )
                    strncat( &( buf[posn] ), "UTC Julian", last - posn );
                else
                    strncat( &( buf[posn] ), "UTC Gregorian", last - posn );

                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == 'z' )
            {
                // time zone, always UTC
                strncat( &( buf[posn] ), "+0000", last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == '+' )
            {
                // date and time
                dayText   = getDayOfWeek( nMJD );
                monthText = getMonth( month );
                if ( ysign == 0 )
                    sprintf( DateTime, "%s %s %02d %02d:%02d:%02d UTC %04d", dayText, monthText, day, hour, min, second, year );
                else
                    sprintf( DateTime, "%s %s %02d %02d:%02d:%02d UTC -%04d", dayText, monthText, day, hour, min, second, year );

                strncat( &( buf[posn] ), DateTime, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
            else if ( next == '.' || isdigit( next ) != 0 )
            {
                // nplaces is number of decimal places ( 0 < nplaces <= 9 )
                if ( next == '.' )
                    // maximum number of places
                    nplaces = 9;
                else
                    nplaces = (int) strtol( &( format[i] ), NULL, 10 );

                // fractional part of seconds to maximum available accuracy
                sec_fraction = sec - (int) sec;
                sprintf( dynamic_format, "%%-%d.%df", nplaces + 2, nplaces );
                //sprintf(DateTime, "%-11.9f",  sec_fraction);
                sprintf( DateTime, dynamic_format, sec_fraction );
                while ( ( ptr = strrchr( &( DateTime[0] ), ' ' ) ) != NULL )
                    ptr[0] = '\0';                                                          // remove trailing white space

                if ( next == '.' )
                {
                    slen = (int) strlen( DateTime ) - 1;
                    while ( DateTime[slen] == '0' && DateTime[slen - 1] != '.' )
                    {
                        DateTime[slen] = '\0'; // remove trailing zeros
                        slen--;
                    }
                }

                ptr = strchr( DateTime, '.' );
                // remove everything in front of the decimal point, and
                // ignore case (%0) where no decimal point exists at all.
                if ( ptr != NULL )
                    strncat( &( buf[posn] ), ptr, last - posn );
                posn = strlen( buf );
                if ( posn >= last )
                    return posn;
            }
        }
        else
        {
            // regular multi-byte character, pass it on
            buf[posn] = next;
            posn++;
            if ( posn >= last )
                return posn;
        }
        buf[posn] = '\0';
        i++;
    }
    return posn;
}

int geMJDtime_TAI( const MJDtime *number1, const TAI_UTC *number2 )
{
    // Returns true if number1 >= number2.
    // N.B. both number1 and number2  must be normalized.
    if ( number1->base_day > number2->base_day )
    {
        return 1;
    }
    else if ( number1->base_day < number2->base_day )
    {
        return 0;
    }
    else
    {
        return ( number1->time_sec >= number2->time_sec_tai );
    }
}

int geMJDtime_UTC( const MJDtime *number1, const TAI_UTC *number2 )
{
    // Returns true if number1 >= number2.
    // N.B. both number1 and number2  must be normalized.
    if ( number1->base_day > number2->base_day )
    {
        return 1;
    }
    else if ( number1->base_day < number2->base_day )
    {
        return 0;
    }
    else
    {
        return ( number1->time_sec >= number2->time_sec_utc );
    }
}

double leap_second_TAI( const MJDtime *MJD_TAI, int *inleap, int *index )
{
    // Logic assumes input MJD_TAI is in TAI
    // *inleap lets the calling routine know whether MJD_TAI corresponds
    // to an epoch when a positive leap increment is being inserted.

    MJDtime MJD_value, *MJD = &MJD_value;
    double  leap;
    int     debug = 0;
    // N.B. geMJDtime_TAI only works for normalized values.
    *MJD = *MJD_TAI;
    normalize_MJD( MJD );
    // Search for index such that TAI_UTC_lookup_table[*index] <= MJD(TAI) < TAI_UTC_lookup_table[*index+1]
    bhunt_search( MJD, TAI_UTC_lookup_table, number_of_entries_in_tai_utc_table, sizeof ( TAI_UTC ), index, ( int ( * )( const void *, const void * ) )geMJDtime_TAI );
    if ( debug == 2 )
        fprintf( stderr, "*index = %d\n", *index );
    if ( *index == -1 )
    {
        // MJD is less than first table entry.
        // Debug: check that condition is met
        if ( debug && geMJDtime_TAI( MJD, &TAI_UTC_lookup_table[*index + 1] ) )
        {
            fprintf( stderr, "libqsastime (leap_second_TAI) logic ERROR: bad condition for *index = %d\n", *index );
            exit( EXIT_FAILURE );
        }
        // There is (by assertion) no discontinuity at the start of the table.
        // Therefore, *inleap cannot be true.
        *inleap = 0;
        // Use initial offset for MJD values before first table entry.
        // Calculate this offset strictly from offset1.  The slope term
        // doesn't enter because offset2 is the same as the UTC of the
        // first epoch of the table.
        return -TAI_UTC_lookup_table[*index + 1].offset1;
    }
    else if ( *index == number_of_entries_in_tai_utc_table - 1 )
    {
        // MJD is greater than or equal to last table entry.
        // Debug: check that condition is met
        if ( debug && !geMJDtime_TAI( MJD, &TAI_UTC_lookup_table[*index] ) )
        {
            fprintf( stderr, "libqsastime (leap_second_TAI) logic ERROR: bad condition for *index = %d\n", *index );
            exit( EXIT_FAILURE );
        }
        // If beyond end of table, cannot be in middle of leap second insertion.
        *inleap = 0;
        // Use final offset for MJD values after last table entry.
        // The slope term doesn't enter because modern values of the slope
        // are zero.
        return -TAI_UTC_lookup_table[*index].offset1;
    }
    else if ( *index >= 0 && *index < number_of_entries_in_tai_utc_table )
    {
        // table[*index] <= MJD < table[*index+1].
        // Debug: check that condition is met
        if ( debug && !( geMJDtime_TAI( MJD, &TAI_UTC_lookup_table[*index] ) && !geMJDtime_TAI( MJD, &TAI_UTC_lookup_table[*index + 1] ) ) )
        {
            fprintf( stderr, "MJD = {%d, %f}\n", MJD->base_day, MJD->time_sec );
            fprintf( stderr, "libqsastime (leap_second_TAI) logic ERROR: bad condition for *index = %d\n", *index );
            exit( EXIT_FAILURE );
        }
        leap = -( TAI_UTC_lookup_table[*index].offset1 + ( ( MJD->base_day - TAI_UTC_lookup_table[*index].offset2 ) + MJD->time_sec / SecInDay ) * TAI_UTC_lookup_table[*index].slope ) / ( 1. + TAI_UTC_lookup_table[*index].slope / SecInDay );
        // Convert MJD(TAI) to normalized MJD(UTC).
        MJD->time_sec += leap;
        normalize_MJD( MJD );
        // If MJD(UT) is in the next interval of the corresponding
        // TAI_UTC_lookup_table, then we are right in the middle of a
        // leap interval (recently a second but for earlier epochs it could be
        // less) insertion.  Note this logic even works when leap intervals
        // are taken away from UTC (i.e., leap is positive) since in that
        // case the UTC *index always corresponds to the TAI *index.
        *inleap = geMJDtime_UTC( MJD, &TAI_UTC_lookup_table[*index + 1] );
        return leap;
    }
    else
    {
        fprintf( stderr, "libqsastime (leap_second_TAI) logic ERROR: bad *index = %d\n", *index );
        exit( EXIT_FAILURE );
    }
}

void configqsas( double scale, double offset1, double offset2, int ccontrol, int ifbtime_offset, int year, int month, int day, int hour, int min, double sec, QSASConfig **qsasconfig )
{
    // Configure the transformation between continuous time and broken-down time
    // that is used for ctimeqsas, btimeqsas, and strfqsas.
    int     forceJulian, ret;
    MJDtime MJD_value, *MJD = &MJD_value;

    // Allocate memory for *qsasconfig if that hasn't been done by a
    // previous call.
    if ( *qsasconfig == NULL )
    {
        *qsasconfig = (QSASConfig *) malloc( (size_t) sizeof ( QSASConfig ) );
        if ( *qsasconfig == NULL )
        {
            fprintf( stderr, "configqsas: out of memory\n" );
            exit( EXIT_FAILURE );
        }
    }

    // Set bhunt_search index to a definite value less than -1 to insure no
    // initial hunt phase from some random index value is done.
    ( *qsasconfig )->index = -40;

    if ( scale != 0. )
    {
        if ( ifbtime_offset )
        {
            if ( ccontrol & 0x1 )
                forceJulian = 1;
            else
                forceJulian = 0;
            ret = setFromUT( year, month, day, hour, min, sec, MJD, forceJulian );
            if ( ret )
            {
                fprintf( stderr, "configqsas: some problem with broken-down arguments\n" );
                exit( EXIT_FAILURE );
            }
            offset1 = (double) MJD->base_day;
            offset2 = MJD->time_sec / (double) SecInDay;
        }
        ( *qsasconfig )->scale    = scale;
        ( *qsasconfig )->offset1  = offset1;
        ( *qsasconfig )->offset2  = offset2;
        ( *qsasconfig )->ccontrol = ccontrol;
    }
    else
    {
        // if scale is 0., then use default values.  Currently, that
        // default is continuous time (stored as a double) is seconds since
        // 1970-01-01 while broken-down time is Gregorian with no other
        // additional corrections.
        ( *qsasconfig )->scale    = 1. / (double) SecInDay;
        ( *qsasconfig )->offset1  = (double) MJD_1970;
        ( *qsasconfig )->offset2  = 0.;
        ( *qsasconfig )->ccontrol = 0x0;
    }
}

void closeqsas( QSASConfig **qsasconfig )
{
    // Close library if it has been opened.
    if ( *qsasconfig != NULL )
    {
        free( (void *) *qsasconfig );
        *qsasconfig = NULL;
    }
}

int ctimeqsas( int year, int month, int day, int hour, int min, double sec, double * ctime, QSASConfig *qsasconfig )
{
    MJDtime MJD_value, *MJD = &MJD_value;
    int     forceJulian, ret;

    if ( qsasconfig == NULL )
    {
        fprintf( stderr, "libqsastime (ctimeqsas) ERROR: configqsas must be called first.\n" );
        exit( EXIT_FAILURE );
    }

    if ( qsasconfig->ccontrol & 0x1 )
        forceJulian = 1;
    else
        forceJulian = 0;

    ret = setFromUT( year, month, day, hour, min, sec, MJD, forceJulian );
    if ( ret )
        return ret;
    *ctime = ( ( (double) ( MJD->base_day ) - qsasconfig->offset1 ) - qsasconfig->offset2 + MJD->time_sec / (double) SecInDay ) / qsasconfig->scale;
    return 0;
}

void btimeqsas( int *year, int *month, int *day, int *hour, int *min, double *sec, double ctime, QSASConfig *qsasconfig )
{
    MJDtime MJD_value, *MJD = &MJD_value;
    int     forceJulian;
    double  integral_offset1, integral_offset2, integral_scaled_ctime;
    int     inleap;

    if ( qsasconfig == NULL )
    {
        fprintf( stderr, "libqsastime (btimeqsas) ERROR: configqsas must be called first.\n" );
        exit( EXIT_FAILURE );
    }

    MJD->time_sec = SecInDay * ( modf( qsasconfig->offset1, &integral_offset1 ) + modf( qsasconfig->offset2, &integral_offset2 ) + modf( ctime * qsasconfig->scale, &integral_scaled_ctime ) );
    MJD->base_day = (int) ( integral_offset1 + integral_offset2 + integral_scaled_ctime );

    if ( qsasconfig->ccontrol & 0x1 )
        forceJulian = 1;
    else
        forceJulian = 0;

    if ( qsasconfig->ccontrol & 0x2 )
        MJD->time_sec += leap_second_TAI( MJD, &inleap, &( qsasconfig->index ) );
    else
        inleap = 0;

    // If in the middle of a positive leap increment insertion, normalize the
    // broken-down result so that *sec exceeds 60 to mark the insertion
    // (similar to the way February 29 marks a leap day).

    if ( inleap )
        MJD->time_sec -= 1.;

    breakDownMJD( year, month, day, hour, min, sec, MJD, forceJulian );
    if ( inleap )
        *sec += 1.;
}

size_t strfqsas( char * buf, size_t len, const char *format, double ctime, QSASConfig *qsasconfig )
{
    MJDtime MJD_value, *MJD = &MJD_value;
    int     forceJulian;
    double  integral_offset1, integral_offset2, integral_scaled_ctime;
    int     inleap;

    if ( qsasconfig == NULL )
    {
        fprintf( stderr, "libqsastime (strfqsas) ERROR: configqsas must be called first.\n" );
        exit( EXIT_FAILURE );
    }
    MJD->time_sec = SecInDay * ( modf( qsasconfig->offset1, &integral_offset1 ) + modf( qsasconfig->offset2, &integral_offset2 ) + modf( ctime * qsasconfig->scale, &integral_scaled_ctime ) );
    MJD->base_day = (int) ( integral_offset1 + integral_offset2 + integral_scaled_ctime );

    if ( qsasconfig->ccontrol & 0x1 )
        forceJulian = 1;
    else
        forceJulian = 0;

    if ( qsasconfig->ccontrol & 0x2 )
        MJD->time_sec += leap_second_TAI( MJD, &inleap, &( qsasconfig->index ) );
    else
        inleap = 0;

    return strfMJD( buf, len, format, MJD, forceJulian, inleap );
}

// bhunt_search.  Search an ordered table with a binary hunt phase and a
// binary search phase.
//
// On entry *low is used to help the hunt phase speed up the binary
// search when consecutive calls to bhunt_search are made with
// similar key values.  On exit, *low is adjusted such that
// base[*low] <= key < base[(*low+1)] with the special cases of
//low set to -1 to indicate the key < base[0] and *low set to n-1
// to indicate base[n-1] <= key.  The function *ge must return true (1)
// if its first argument (the search key) is greater than or equal to
// its second argument (a table entry).  Otherwise it returns false
// (0).  Items in the array base must be in ascending order.

void bhunt_search( const void *key, const void *base, int n, size_t size, int *low, int ( *ge )( const void *keyval, const void *datum ) )
{
    const void *indexbase;
    int        mid, high, hunt_inc = 1;
    // If previous search found below range, then assure one hunt cycle
    // just in case new key is also below range.
    if ( *low == -1 )
        *low = 0;
    // Protect against invalid or undefined *low values where hunt
    // is waste of time.
    if ( *low < 0 || *low >= n )
    {
        *low = -1;
        high = n;
    }
    else
    {
        // binary hunt phase where we are assured 0 <= *low < n
        indexbase = (const void *) ( ( (const char *) base ) + ( size * (size_t) ( *low ) ) );
        if ( ( *ge )( key, indexbase ) )
        {
            high      = ( *low ) + hunt_inc;
            indexbase = (const void *) ( ( (const char *) base ) + ( size * (size_t) high ) );
            // indexbase is valid if high < n.
            while ( ( high < n ) && ( ( *ge )( key, indexbase ) ) )
            {
                *low      = high;
                hunt_inc += hunt_inc;
                high      = high + hunt_inc;
                indexbase = (const void *) ( ( (const char *) base ) + ( size * (size_t) high ) );
            }
            if ( high >= n )
                high = n;
            // At this point, low is valid and base[low] <= key
            // and either key < base[high] for valid high or high = n.
        }
        else
        {
            high      = *low;
            *low      = high - hunt_inc;
            indexbase = (const void *) ( ( (const char *) base ) + ( size * (size_t) ( *low ) ) );
            // indexbase is valid if(*low) >= 0
            while ( ( ( *low ) >= 0 ) && !( ( *ge )( key, indexbase ) ) )
            {
                high      = *low;
                hunt_inc += hunt_inc;
                *low      = ( *low ) - hunt_inc;
                indexbase = (const void *) ( ( (const char *) base ) + ( size * (size_t) ( *low ) ) );
            }
            if ( ( *low ) < 0 )
                *low = -1;
            // At this point high is valid and key < base[high]
            // and either base[low] <= key for valid low or low = -1.
        }
    }
    // binary search phase where we are assured base[low] <= key < base[high]
    // when both low and high are valid with obvious special cases signalled
    // by low = -1 or high = n.
    while ( high - *low > 1 )
    {
        mid       = *low + ( high - *low ) / 2;
        indexbase = (const void *) ( ( (const char *) base ) + ( size * (size_t) mid ) );
        if ( ( *ge )( key, indexbase ) )
            *low = mid;
        else
            high = mid;
    }
}
