//
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
#include "qsastime.h"
#include "qsastimeP.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

#define TEST01      0x1
#define TEST02      0x2
#define TEST03      0x4
#define TEST04      0x8
#define TEST05      0x10
#define TEST06      0x20
#define TEST07      0x40
#define TEST08      0x80
#define TEST09      0x100
#define TEST10      0x200
#define TEST11      0x400
#define TEST12      0x800
#define TEST13      0x1000
#define TEST14      0x2000
#define TEST15      0x4000
#define TEST16      0x8000
// MJD for Jan 01, 1970 00:00:00 Gregorian, the Unix epoch.
#define MJD_1970    40587

// Recommended (by Linux timegm man page) POSIX equivalent of Linux timegm C library function
time_t my_timegm( struct tm *tm )
{
    time_t ret;
    char   *tz;

    tz = getenv( "TZ" );
    setenv( "TZ", "", 1 );
    tzset();
    ret = mktime( tm );
    if ( tz )
        setenv( "TZ", tz, 1 );
    else
        unsetenv( "TZ" );
    tzset();
    return ret;
}

int testlib_broken_down_time( int year, int month, int day, int hour, int min, double sec, int forceJulian, int inner_test_choice, int verbose )
{
    char      buf[360];
    int       year1, month1, day1, hour1, min1;
    double    sec1;
    struct tm tm;
    struct tm *ptm = &tm;
    struct tm tm1;
    struct tm *ptm1 = &tm1;
    time_t    secs_past_epoch, secs_past_epoch1, delta_secs;

    MJDtime   MJD1, *pMJD1 = &MJD1;
    double    jd;
    int       ifleapyear, ifleapday, iffeb29, ifsamedate, ifsametime;
    ptm->tm_year = year - 1900;
    ptm->tm_mon  = month;
    ptm->tm_mday = day;
    ptm->tm_hour = hour;
    ptm->tm_min  = min;
    ptm->tm_sec  = (int) sec;
    if ( verbose )
    {
        if ( forceJulian )
        {
            printf( "Start of Julian proleptic inner test\n" );
            printf( "input and output (strfMJD) date/time\n" );
        }
        else
        {
            printf( "Start of Gregorian proleptic inner test\n" );
            printf( "input and output (strftime), and output (strfMJD) date/time\n" );
        }
        printf( "%.4d-%02d-%02dT%02d:%02d:%018.15fZ\n", year, month + 1, day, hour, min, sec );
    }

    setFromUT( year, month, day, hour, min, sec, pMJD1, forceJulian );

    // Inner TEST01: compare setFromUT with my_timegm.
    if ( !forceJulian && ( inner_test_choice & TEST01 ) )
    {
        secs_past_epoch1 = (time_t) ( 86400. * ( (double) pMJD1->base_day - (double) MJD_1970 ) + (int) pMJD1->time_sec );
        secs_past_epoch  = my_timegm( ptm );
        delta_secs       = abs( secs_past_epoch1 - secs_past_epoch );
        if ( delta_secs != 0 )
        {
            printf( "setFromUT secs_past_epoch = %lld seconds\n", (long long) secs_past_epoch1 );
            printf( "my_timegm secs_past_epoch = %lld seconds\n", (long long) secs_past_epoch );
            printf( "delta secs_past_epoch = %lld seconds\n", (long long) ( secs_past_epoch1 - secs_past_epoch ) );
            printf( "test failed with inconsistency between setFromUT and my_timegm\n" );
            return 1;
        }
    }

    // Inner TEST02: check minimal fields of strfMJD (Julian) or
    // strftime and strfMJD (Gregorian)
    if ( inner_test_choice & TEST02 )
    {
        if ( !forceJulian )
        {
            strftime( &( buf[0] ), 360, "%Y-%m-%dT%H:%M:%SZ\n", ptm );
            if ( verbose )
                printf( "%s", buf );
        }
        strfMJD( &( buf[0] ), 360, "%Y-%m-%dT%H:%M:%S%.Z\n", pMJD1, forceJulian, 0 );
        if ( verbose )
            printf( "%s", buf );
    }

    if ( verbose )
    {
        jd = 2400000.5 + pMJD1->base_day + pMJD1->time_sec / 86400.;
        printf( "setFromUT JD = %25.16f days\n", jd );
    }

    if ( forceJulian )
        ifleapyear = ( year % 4 == 0 );
    else
        ifleapyear = ( ( year % 4 == 0 && year % 100 != 0 ) || year % 400 == 0 );
    iffeb29   = month == 1 && day == 29;
    ifleapday = ( ifleapyear && iffeb29 );

    // Inner TEST03: compare setFromUT with its inverse, breakDownMJD
    if ( inner_test_choice & TEST03 )
    {
        breakDownMJD( &year1, &month1, &day1, &hour1, &min1, &sec1, pMJD1, forceJulian );
        ifsamedate = ( year1 - year == 0 && ( ( ( !iffeb29 || ifleapday ) && ( month1 - month == 0 && day1 - day == 0 ) ) || ( ( iffeb29 && !ifleapday ) && ( month1 == 2 && day1 == 1 ) ) ) );
        ifsametime = ( hour1 - hour == 0 && min1 - min == 0 && fabs( sec1 - sec ) < 1.e-10 );

        if ( !( ifsamedate && ifsametime ) )
        {
            printf( "output date calculated with breakDownMJD = %d-%02d-%02dT%02d:%02d:%018.15fZ\n", year1, month1 + 1, day1, hour1, min1, sec1 );
            printf( "test failed with inconsistency between setFromUT and breakDownMJD\n" );
            return 1;
        }
    }

    // Inner TEST04: compare setFromUT with its inverse, the C library gmtime.
    if ( !forceJulian && ( inner_test_choice & TEST04 ) )
    {
        ptm1       = gmtime( &secs_past_epoch );
        ifsamedate = ( ptm1->tm_year == ptm->tm_year && ( ( ( !iffeb29 || ifleapday ) && ( ptm1->tm_mon == ptm->tm_mon && ptm1->tm_mday == ptm->tm_mday ) ) || ( ( iffeb29 && !ifleapday ) && ( ptm1->tm_mon == 2 && ptm1->tm_mday == 1 ) ) ) );
        ifsametime = ( ptm1->tm_hour == ptm->tm_hour && ptm1->tm_min == ptm->tm_min && ptm1->tm_sec == ptm->tm_sec );

        if ( !( ifsamedate && ifsametime ) )
        {
            printf( "test failed with inconsistency between my_timegm and its C library inverse gmtime" );
            return 1;
        }
    }
    return 0;
}

int testlib_MJD( const MJDtime *MJD, int forceJulian, int inner_test_choice, int verbose )
{
    int       year, month, day, hour, min;
    double    sec;
    char      buf[360];
    int       year1, month1, day1, hour1, min1;
    double    sec1;
    struct tm tm;
    struct tm *ptm = &tm;
    struct tm tm1;
    struct tm *ptm1 = &tm1;
    time_t    secs_past_epoch, secs_past_epoch1;

    MJDtime   MJD1_value, *MJD1 = &MJD1_value;
    MJDtime   MJD2_value, *MJD2 = &MJD2_value;
    double    jd;
    int       ifleapyear, ifleapday, iffeb29, ifsamedate, ifsametime;

    *MJD1 = *MJD;
    normalize_MJD( MJD1 );
    secs_past_epoch = (time_t) ( 86400. * ( (double) MJD1->base_day - (double) MJD_1970 ) + MJD1->time_sec );
    breakDownMJD( &year, &month, &day, &hour, &min, &sec, MJD1, forceJulian );

    ptm->tm_year = year - 1900;
    ptm->tm_mon  = month;
    ptm->tm_mday = day;
    ptm->tm_hour = hour;
    ptm->tm_min  = min;
    ptm->tm_sec  = (int) sec;
    if ( verbose )
    {
        if ( forceJulian )
        {
            printf( "Start of Julian proleptic inner test\n" );
            printf( "input and output (strfMJD) date/time\n" );
        }
        else
        {
            printf( "Start of Gregorian proleptic inner test\n" );
            printf( "input and output (strftime), and output (strfMJD) date/time\n" );
        }
        printf( "%.4d-%02d-%02dT%02d:%02d:%018.15fZ\n", year, month + 1, day, hour, min, sec );
    }

    // Inner TEST01: compare breakDownMJD with gmtime.
    if ( !forceJulian && ( inner_test_choice & TEST01 ) )
    {
        ptm1 = gmtime( &secs_past_epoch );
        if ( !( ( ptm1->tm_year + 1900 ) == year && ptm1->tm_mon == month && ptm1->tm_mday == day && ptm1->tm_hour == hour && ptm1->tm_min == min && ptm1->tm_sec == (int) sec ) )
        {
            printf( "date calculated with breakDownMJD = %d-%02d-%02dT%02d:%02d:%018.15fZ\n", year, month + 1, day, hour, min, sec );
            printf( "date calculated with gmtime = %d-%02d-%02dT%02d:%02d:%02dZ\n", ptm1->tm_year + 1900, ptm1->tm_mon + 1, ptm1->tm_mday, ptm1->tm_hour, ptm1->tm_min, ptm1->tm_sec );
            printf( "test failed with inconsistency between breakDownMJD and gmtime\n" );
            return 1;
        }
    }

    // Inner TEST02: check minimal fields of strfMJD (Julian) or
    // strftime and strfMJD (Gregorian)
    if ( inner_test_choice & TEST02 )
    {
        if ( !forceJulian )
        {
            strftime( &( buf[0] ), 360, "%Y-%m-%dT%H:%M:%SZ\n", ptm );
            if ( verbose )
                printf( "%s", buf );
        }
        strfMJD( &( buf[0] ), 360, "%Y-%m-%dT%H:%M:%S%.Z\n", MJD1, forceJulian, 0 );
        if ( verbose )
            printf( "%s", buf );
    }

    if ( verbose )
    {
        jd = 2400000.5 + MJD1->base_day + MJD1->time_sec / 86400.;
        printf( "JD = %25.16f days\n", jd );
    }

    if ( forceJulian )
        ifleapyear = ( year % 4 == 0 );
    else
        ifleapyear = ( ( year % 4 == 0 && year % 100 != 0 ) || year % 400 == 0 );
    iffeb29   = month == 1 && day == 29;
    ifleapday = ( ifleapyear && iffeb29 );

    // Inner TEST03: compare breakDownMJD with its inverse, setFromUT
    if ( inner_test_choice & TEST03 )
    {
        setFromUT( year, month, day, hour, min, sec, MJD2, forceJulian );
        if ( !( MJD2->time_sec == MJD1->time_sec && MJD2->base_day == MJD1->base_day ) )
        {
            printf( "(normalized) input MJD components are = %d, %f\n", MJD1->base_day, MJD1->time_sec );
            printf( "(output MJD2 components generated by setFromUT are = %d, %f\n", MJD2->base_day, MJD2->time_sec );
            printf( "test failed with inconsistency between breakDownMJD and setFromUT\n" );
            return 1;
        }
    }

    // Inner TEST04: compare breakDownMJD with its inverse, my_timegm
    if ( !forceJulian && ( inner_test_choice & TEST04 ) )
    {
        secs_past_epoch1 = my_timegm( ptm );
        if ( !( secs_past_epoch == secs_past_epoch1 ) )
        {
            printf( "secs_past_epoch calculated from input = %lld\n", (long long) secs_past_epoch );
            printf( "secs_past_epoch calculated from my_timegm = %lld\n", (long long) secs_past_epoch1 );
            printf( "delta secs_past_epoch = %lld seconds\n", (long long) ( secs_past_epoch1 - secs_past_epoch ) );
            printf( "test failed with inconsistency between breakDownMJD and its C library based inverse, my_timegm\n" );
            return 1;
        }
    }
    return 0;
}

// Test program to do extensive comparisons between setFromUT, breakDownMJD,
// and strfMJD and the closest corresponding _Linux_ C library routines,
// timegm, gmtime, and strftime.

int main()
{
    char      buf[360];
    char      buf1[360];
    int       year, month, day, hour, min;
    double    sec;
    int       year1, month1, day1, hour1, min1;
    double    sec1;
    struct tm tm;
    struct tm *ptm = &tm;
    struct tm tm1;
    struct tm *ptm1 = &tm1;
    int       seconds;

    MJDtime   MJD1_value, *MJD1 = &MJD1_value;
    double    jd;
    int       test_choice, date_choice, ret;

    // choose test(s) to be run using bit-pattern in test_choice that is
    // input from stdin.
    scanf( "%i", &test_choice );

    printf( "sizeof(time_t) = %d\n", (int) sizeof ( time_t ) );
    if ( sizeof ( time_t ) < 8 )
    {
        printf( "tests abandoned because time_t is too small on this platform to represent the extremely large date range used for many of these tests.  Note, the limitation is in the C library routines (gmtime and mktime) used for these test comparisons and not libqsastime itself.\n" );
        return 1;
    }

    printf( "sizeof(int) = %d\n", (int) sizeof ( int ) );
    if ( sizeof ( int ) != 4 )
    {
        printf( "tests abandoned because int must be 32-bits to test this library properly for how well it will potentially perform on 32-bit platforms\n" );
        return 2;
    }
    // strftime affected by locale so force 0 timezone for this complete test.
    setenv( "TZ", "", 1 );
    tzset();

    if ( test_choice & TEST01 )
    {
        printf( "Test 01 of calendar dates in the vicinity of the JD epoch \n" );

        for ( date_choice = 0; date_choice < 5; date_choice++ )
        {
            if ( date_choice == 0 )
            {
                month = 0;
                day   = 1;
            }
            else if ( date_choice == 1 )
            {
                month = 1;
                day   = 28;
            }
            else if ( date_choice == 2 )
            {
                month = 1;
                day   = 29;
            }
            else if ( date_choice == 3 )
            {
                month = 2;
                day   = 1;
            }
            else if ( date_choice == 4 )
            {
                month = 11;
                day   = 31;
            }
            hour = 12;
            min  = 0;
            sec  = 0.;

            for ( year = -4717; year <= -4707; year++ )
            {
                printf( "\n" );
                ret = testlib_broken_down_time( year, month, day, hour, min, sec, 1, 0xffff, 1 );
                if ( ret )
                    return ret;
                ret = testlib_broken_down_time( year, month, day, hour, min, sec, 0, 0xffff, 1 );
                if ( ret )
                    return ret;
            }
        }
    }

    if ( test_choice & TEST02 )
    {
        printf( "Test 02 of calendar dates in the vicinity of the year epoch. \n" );

        for ( date_choice = 0; date_choice < 5; date_choice++ )
        {
            if ( date_choice == 0 )
            {
                month = 0;
                day   = 1;
            }
            else if ( date_choice == 1 )
            {
                month = 1;
                day   = 28;
            }
            else if ( date_choice == 2 )
            {
                month = 1;
                day   = 29;
            }
            else if ( date_choice == 3 )
            {
                month = 2;
                day   = 1;
            }
            else if ( date_choice == 4 )
            {
                month = 11;
                day   = 31;
            }
            hour = 0;
            min  = 0;
            sec  = 0.;

            for ( year = -5; year <= 5; year++ )
            {
                printf( "\n" );
                ret = testlib_broken_down_time( year, month, day, hour, min, sec, 1, 0xffff, 1 );
                if ( ret )
                    return ret;
                ret = testlib_broken_down_time( year, month, day, hour, min, sec, 0, 0xffff, 1 );
                if ( ret )
                    return ret;
            }
        }
    }

    if ( test_choice & TEST03 )
    {
        printf( "Test 03 of calendar dates in the vicinity of the MJD epoch. \n" );

        for ( date_choice = 0; date_choice < 6; date_choice++ )
        {
            if ( date_choice == 0 )
            {
                month = 0;
                day   = 1;
            }
            else if ( date_choice == 1 )
            {
                month = 1;
                day   = 28;
            }
            else if ( date_choice == 2 )
            {
                month = 1;
                day   = 29;
            }
            else if ( date_choice == 3 )
            {
                month = 2;
                day   = 1;
            }
            else if ( date_choice == 4 )
            {
                month = 10;
                day   = 17;
            }
            else if ( date_choice == 5 )
            {
                month = 11;
                day   = 31;
            }
            hour = 0;
            min  = 0;
            sec  = 0.;

            for ( year = 1853; year <= 1863; year++ )
            {
                printf( "\n" );
                ret = testlib_broken_down_time( year, month, day, hour, min, sec, 1, 0xffff, 1 );
                if ( ret )
                    return ret;
                ret = testlib_broken_down_time( year, month, day, hour, min, sec, 0, 0xffff, 1 );
                if ( ret )
                    return ret;
            }
        }
    }

    if ( test_choice & TEST04 )
    {
        printf( "Test 04 of small second range near Year 0 (Julian)\n" );

        ret = setFromUT( 0, 0, 1, 0, 0, 0., MJD1, 1 );
        if ( ret )
        {
            printf( "Test 04 cannot even start for Year 0 (Julian)" );
            return ret;
        }

        for ( seconds = -5; seconds < 5; seconds++ )
        {
            printf( "\n" );
            ret = testlib_MJD( MJD1, 1, 0xffff, 1 );
            if ( ret )
                return ret;
            MJD1->time_sec++;
        }

        printf( "Test 04 of small second range near Year 0 (Gregorian)\n" );


        ret = setFromUT( 0, 0, 1, 0, 0, 0., MJD1, 0 );
        if ( ret )
        {
            printf( "Test 04 cannot even start for Year 0 (Gregorian)" );
            return ret;
        }

        for ( seconds = -5; seconds < 5; seconds++ )
        {
            printf( "\n" );
            ret = testlib_MJD( MJD1, 0, 0xffff, 1 );
            if ( ret )
                return ret;
            MJD1->time_sec++;
        }

        printf( "Test 04 of small second range near 2009-01-01 (Gregorian) when a leap second was inserted\n" );


        ret = setFromUT( 2009, 0, 1, 0, 0, 0.1234567890123456 - 5., MJD1, 0 );
        if ( ret )
        {
            printf( "Test 04 cannot even start for Year 0 (Gregorian)" );
            return ret;
        }

        for ( seconds = -5; seconds < 5; seconds++ )
        {
            printf( "\n" );
            ret = testlib_MJD( MJD1, 0, 0xffff, 1 );
            if ( ret )
                return ret;
            MJD1->time_sec++;
        }
    }

    if ( test_choice & TEST05 )
    {
        printf( "Test 05 of normalization of breakDownMJD result and strfMJD results near the hour.\n" );
        MJD1->base_day = 51910;
        MJD1->time_sec = 3600.;
        int iepsilon;
        for ( iepsilon = -1; iepsilon < 2; iepsilon++ )
        {
            MJD1->time_sec = 3600. + 1.e-8 * (double) iepsilon;
            breakDownMJD( &year, &month, &day, &hour, &min, &sec, MJD1, 0 );
            printf( "MJD = {%d,%20.15f}\n", MJD1->base_day, MJD1->time_sec );
            printf( "breakDownMJD result is year, month, day, hour, min, sec = %d, %d, %d, %d, %d, %20.15f\n", year, month, day, hour, min, sec );
            strfMJD( &( buf[0] ), 360, "%Y-%m-%dT%H:%M:%S%9Z\n", MJD1, 0, 0 );
            printf( "strfMJD %%S%%9 result is %s", buf );
            strfMJD( &( buf[0] ), 360, "%Y-%m-%dT%H:%M:%S%.Z\n", MJD1, 0, 0 );
            printf( "strfMJD %%S%%. result is %s", buf );
            strfMJD( &( buf[0] ), 360, "%H:%M:%S, %H:%M:%S%0, %H:%M:%S%1, %H:%M:%S%2, %H:%M:%S%3, %H:%M:%S%4\n          %H:%M:%S %0,%H:%M:%S %1,%H:%M:%S %2,%H:%M:%S %3,%H:%M:%S %4\n", MJD1, 0, 0 );
            printf( "strfMJD more heavily rounded results (the latter ones with a blank before the\ndecimal point to prove separated formatting works) for H:M:S are the following:\n%s", buf );
        }
    }

    if ( test_choice & TEST06 )
    {
        printf( "Test 06 (non-verbose) of calendar dates for every year from -5000000 to 5000000\n" );

        for ( date_choice = 0; date_choice < 5; date_choice++ )
        {
            if ( date_choice == 0 )
            {
                month = 0;
                day   = 1;
            }
            else if ( date_choice == 1 )
            {
                month = 1;
                day   = 28;
            }
            else if ( date_choice == 2 )
            {
                month = 1;
                day   = 29;
            }
            else if ( date_choice == 3 )
            {
                month = 2;
                day   = 1;
            }
            else if ( date_choice == 4 )
            {
                month = 11;
                day   = 31;
            }
            hour = 0;
            min  = 0;
            sec  = 0.123456;

            // test reduced range of years that just barely misses overflowing
            // the MJD integer.  e.g., 6000000 overflows it.
            for ( year = -5000000; year <= 5000000; year += 1 )
            {
                ret = testlib_broken_down_time( year, month, day, hour, min, sec, 1, 0xffff, 0 );
                if ( ret )
                    return ret;
                ret = testlib_broken_down_time( year, month, day, hour, min, sec, 0, 0xffff, 0 );
                if ( ret )
                    return ret;
            }
        }
    }

    if ( test_choice & TEST07 )
    {
        printf( "Test 07 (non-verbose) of all seconds from late 2007 to early 2009\n" );
        ret = setFromUT( 2007, 11, 30, 0, 0, 0., MJD1, 0 );
        if ( ret )
        {
            printf( "Test 06 cannot even start" );
            return ret;
        }

        // 430 days or ~ 37 million seconds should cover the complete next year for both Julian and Gregorian .
        for ( seconds = 0; seconds < 430 * 86400; seconds++ )
        {
            MJD1->time_sec = (double) seconds;
            ret            = testlib_MJD( MJD1, 1, 0xffff, 0 );
            if ( ret )
                return ret;

            ret = testlib_MJD( MJD1, 0, 0xffff, 0 );
            if ( ret )
                return ret;
        }
    }


    return 0;
}
