//     Sample plots using date / time formatting for axes
//
// Copyright (C) 2007 Andrew Ross
//
// This file is part of PLplot.
//
//  PLplot is free software; you can redistribute it and/or modify
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

#include "plcdemos.h"

static PLFLT x[365], y[365];
static PLFLT xerr1[365], xerr2[365], yerr1[365], yerr2[365];

// Function prototypes

void plot1( void );
void plot2( void );
void plot3( void );
void plot4( void );

//--------------------------------------------------------------------------
// main
//
// Draws several plots which demonstrate the use of date / time formats for
// the axis labels.
// Time formatting is done using the strfqsas routine from the qsastime
// library.  This is similar to strftime, but works for a broad
// date range even on 32-bit systems.  See the
// documentation of strfqsas for full details of the available formats.
//
// 1) Plotting temperature over a day (using hours / minutes)
// 2) Plotting
//
// Note: We currently use the default call for plconfigtime (done in
// plinit) which means continuous times are interpreted as seconds since
// 1970-01-01, but that may change in future, more extended versions of
// this example.
//
//--------------------------------------------------------------------------

int
main( int argc, char *argv[] )
{
    // Parse command line arguments
    plparseopts( &argc, argv, PL_PARSE_FULL );

    // Initialize plplot
    plinit();

    // Change the escape character to a '@' instead of the default '#'
    plsesc( '@' );

    plot1();

    plot2();

    plot3();

    plot4();

    // Don't forget to call plend() to finish off!
    plend();
    exit( 0 );
}

// Plot a model diurnal cycle of temperature
void
plot1( void )
{
    int   i, npts;
    PLFLT xmin, xmax, ymin, ymax;

    // Data points every 10 minutes for 1 day
    npts = 73;

    xmin = 0;
    xmax = 60.0 * 60.0 * 24.0; // Number of seconds in a day
    ymin = 10.0;
    ymax = 20.0;

    for ( i = 0; i < npts; i++ )
    {
        x[i] = xmax * ( (PLFLT) i / (PLFLT) npts );
        y[i] = 15.0 - 5.0 * cos( 2 * M_PI * ( (PLFLT) i / (PLFLT) npts ) );
        // Set x error bars to +/- 5 minute
        xerr1[i] = x[i] - 60 * 5;
        xerr2[i] = x[i] + 60 * 5;
        // Set y error bars to +/- 0.1 deg C
        yerr1[i] = y[i] - 0.1;
        yerr2[i] = y[i] + 0.1;
    }

    pladv( 0 );

    // Rescale major ticks marks by 0.5
    plsmaj( 0.0, 0.5 );
    // Rescale minor ticks and error bar marks by 0.5
    plsmin( 0.0, 0.5 );

    plvsta();
    plwind( xmin, xmax, ymin, ymax );

    // Draw a box with ticks spaced every 3 hour in X and 1 degree C in Y.
    plcol0( 1 );
    // Set time format to be hours:minutes
    pltimefmt( "%H:%M" );
    plbox( "bcnstd", 3.0 * 60 * 60, 3, "bcnstv", 1, 5 );

    plcol0( 3 );
    pllab( "Time (hours:mins)", "Temperature (degC)", "@frPLplot Example 29 - Daily temperature" );

    plcol0( 4 );

    plline( npts, x, y );
    plcol0( 2 );
    plerrx( npts, xerr1, xerr2, y );
    plcol0( 3 );
    plerry( npts, x, yerr1, yerr2 );

    // Rescale major / minor tick marks back to default
    plsmin( 0.0, 1.0 );
    plsmaj( 0.0, 1.0 );
}

// Plot the number of hours of daylight as a function of day for a year
void
plot2( void )
{
    int   j, npts;
    PLFLT xmin, xmax, ymin, ymax;
    PLFLT lat, p, d;

    // Latitude for London
    lat = 51.5;

    npts = 365;

    xmin = 0;
    xmax = npts * 60.0 * 60.0 * 24.0;
    ymin = 0;
    ymax = 24;

    // Formula for hours of daylight from
    // "A Model Comparison for Daylength as a Function of Latitude and
    // Day of the Year", 1995, Ecological Modelling, 80, pp 87-95.
    for ( j = 0; j < npts; j++ )
    {
        x[j] = j * 60.0 * 60.0 * 24.0;
        p    = asin( 0.39795 * cos( 0.2163108 + 2 * atan( 0.9671396 * tan( 0.00860 * ( j - 186 ) ) ) ) );
        d    = 24.0 - ( 24.0 / M_PI ) *
               acos( ( sin( 0.8333 * M_PI / 180.0 ) + sin( lat * M_PI / 180.0 ) * sin( p ) ) /
            ( cos( lat * M_PI / 180.0 ) * cos( p ) ) );
        y[j] = d;
    }

    plcol0( 1 );
    // Set time format to be abbreviated month name followed by day of month
    pltimefmt( "%b %d" );
    plprec( 1, 1 );
    plenv( xmin, xmax, ymin, ymax, 0, 40 );


    plcol0( 3 );
    pllab( "Date", "Hours of daylight", "@frPLplot Example 29 - Hours of daylight at 51.5N" );

    plcol0( 4 );

    plline( npts, x, y );

    plprec( 0, 0 );
}

void
plot3( void )
{
    int   i, npts;
    PLFLT xmin, xmax, ymin, ymax;
    PLFLT tstart;

    // Calculate continuous time corresponding to 2005-12-01 UTC.
    plctime( 2005, 11, 01, 0, 0, 0., &tstart );
    npts = 62;

    xmin = tstart;
    xmax = xmin + npts * 60.0 * 60.0 * 24.0;
    ymin = 0.0;
    ymax = 5.0;

    for ( i = 0; i < npts; i++ )
    {
        x[i] = xmin + i * 60.0 * 60.0 * 24.0;
        y[i] = 1.0 + sin( 2 * M_PI * ( (PLFLT) i ) / 7.0 ) +
               exp( ( (PLFLT) MIN( i, npts - i ) ) / 31.0 );
    }
    pladv( 0 );

    plvsta();
    plwind( xmin, xmax, ymin, ymax );

    plcol0( 1 );
    // Set time format to be ISO 8601 standard YYYY-MM-DD.
    pltimefmt( "%F" );
    // Draw a box with ticks spaced every 14 days in X and 1 hour in Y.
    plbox( "bcnstd", 14 * 24.0 * 60.0 * 60.0, 14, "bcnstv", 1, 4 );

    plcol0( 3 );
    pllab( "Date", "Hours of television watched", "@frPLplot Example 29 - Hours of television watched in Dec 2005 / Jan 2006" );

    plcol0( 4 );

    // Rescale symbol size (used by plpoin) by 0.5
    plssym( 0.0, 0.5 );
    plpoin( npts, x, y, 2 );
    plline( npts, x, y );
}

void
plot4( void )
{
    // TAI-UTC (seconds) as a function of time.

    PLFLT scale;
    PLFLT xmin, xmax, ymin = 0.0, ymax = 0.0, xlabel_step = 0.0;
    int   kind, npts = 0, if_TAI_time_format = 0, i;
    char  time_format[10];
    char  title_suffix[100];
    char  xtitle[100];
    char  title[100];
    PLFLT xx[1001], yy[1001];
    PLINT epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min;
    PLFLT epoch_sec;
    PLINT tai_year, tai_month, tai_day, tai_hour, tai_min;
    PLFLT tai_sec, tai;
    PLINT utc_year, utc_month, utc_day, utc_hour, utc_min;
    PLFLT utc_sec, utc;

    // Continuous time unit is Besselian years from whatever epoch is
    // chosen below.  Could change to seconds (or days) from the
    // epoch, but then would have to adjust xlabel_step below.
    scale = 365.242198781;
    // MJD epoch (see <https://en.wikipedia.org/wiki/Julian_day>).
    // This is only set for illustrative purposes, and is overwritten
    // below for the time-representation reasons given in the
    // discussion below.
    epoch_year  = 1858;
    epoch_month = 11;
    epoch_day   = 17;
    epoch_hour  = 0;
    epoch_min   = 0;
    epoch_sec   = 0.;
    // To illustrate the time-representation issues of using the MJD
    // epoch, in 1985, MJD was roughly 46000 days which corresponds to
    // 4e9 seconds.  Thus, for the -DPL_DOUBLE=ON case where PLFLT is
    // a double which can represent continuous time to roughly 16
    // decimal digits of precision the time-representation error is
    // roughly ~400 nanoseconds.  Therefore the MJD epoch would be
    // acceptable for the plots below in the -DPL_DOUBLE=ON case.
    // However, that epoch is obviously not acceptable for the
    // -DPL_DOUBLE=OFF case where PLFLT is a float which can represent
    // continuous time to only ~7 decimal digits of precision
    // corresponding to a time representation error of 400 seconds (!)
    // in 1985.  For this reason, we do not use the MJD epoch below
    // and instead choose the best epoch for each case to minimize
    // time-representation issues.

    for ( kind = 0; kind < 7; kind++ )
    {
        if ( kind == 0 )
        {
            // Choose midpoint to maximize time-representation precision.
            epoch_year  = 1985;
            epoch_month = 0;
            epoch_day   = 2;
            epoch_hour  = 0;
            epoch_min   = 0;
            epoch_sec   = 0.;
            plconfigtime( scale, 0., 0., 0x0, 1, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
            plctime( 1950, 0, 2, 0, 0, 0., &xmin );
            plctime( 2020, 0, 2, 0, 0, 0., &xmax );
            npts = 70 * 12 + 1;
            ymin = 0.0;
            ymax = 36.0;
            strncpy( time_format, "%Y%", 10 );
            if_TAI_time_format = 1;
            strncpy( title_suffix, "from 1950 to 2020", 100 );
            strncpy( xtitle, "Year", 100 );
            xlabel_step = 10.;
        }
        else if ( kind == 1 || kind == 2 )
        {
            // Choose midpoint to maximize time-representation precision.
            epoch_year  = 1961;
            epoch_month = 7;
            epoch_day   = 1;
            epoch_hour  = 0;
            epoch_min   = 0;
            epoch_sec   = 1.64757;
            plconfigtime( scale, 0., 0., 0x0, 1, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
            plctime( 1961, 7, 1, 0, 0, 1.64757 - .20, &xmin );
            plctime( 1961, 7, 1, 0, 0, 1.64757 + .20, &xmax );
            npts = 1001;
            ymin = 1.625;
            ymax = 1.725;
            strncpy( time_format, "%S%2%", 10 );
            strncpy( title_suffix, "near 1961-08-01 (TAI)", 100 );
            xlabel_step = 0.05 / ( scale * 86400. );
            if ( kind == 1 )
            {
                if_TAI_time_format = 1;
                strncpy( xtitle, "Seconds (TAI)", 100 );
            }
            else
            {
                if_TAI_time_format = 0;
                strncpy( xtitle, "Seconds (TAI) labelled with corresponding UTC", 100 );
            }
        }
        else if ( kind == 3 || kind == 4 )
        {
            // Choose midpoint to maximize time-representation precision.
            epoch_year  = 1963;
            epoch_month = 10;
            epoch_day   = 1;
            epoch_hour  = 0;
            epoch_min   = 0;
            epoch_sec   = 2.6972788;
            plconfigtime( scale, 0., 0., 0x0, 1, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
            plctime( 1963, 10, 1, 0, 0, 2.6972788 - .20, &xmin );
            plctime( 1963, 10, 1, 0, 0, 2.6972788 + .20, &xmax );
            npts = 1001;
            ymin = 2.55;
            ymax = 2.75;
            strncpy( time_format, "%S%2%", 10 );
            strncpy( title_suffix, "near 1963-11-01 (TAI)", 100 );
            xlabel_step = 0.05 / ( scale * 86400. );
            if ( kind == 3 )
            {
                if_TAI_time_format = 1;
                strncpy( xtitle, "Seconds (TAI)", 100 );
            }
            else
            {
                if_TAI_time_format = 0;
                strncpy( xtitle, "Seconds (TAI) labelled with corresponding UTC", 100 );
            }
        }
        else if ( kind == 5 || kind == 6 )
        {
            // Choose midpoint to maximize time-representation precision.
            epoch_year  = 2009;
            epoch_month = 0;
            epoch_day   = 1;
            epoch_hour  = 0;
            epoch_min   = 0;
            epoch_sec   = 34.;
            plconfigtime( scale, 0., 0., 0x0, 1, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
            plctime( 2009, 0, 1, 0, 0, 34. - 5., &xmin );
            plctime( 2009, 0, 1, 0, 0, 34. + 5., &xmax );
            npts = 1001;
            ymin = 32.5;
            ymax = 34.5;
            strncpy( time_format, "%S%2%", 10 );
            strncpy( title_suffix, "near 2009-01-01 (TAI)", 100 );
            xlabel_step = 1. / ( scale * 86400. );
            if ( kind == 5 )
            {
                if_TAI_time_format = 1;
                strncpy( xtitle, "Seconds (TAI)", 100 );
            }
            else
            {
                if_TAI_time_format = 0;
                strncpy( xtitle, "Seconds (TAI) labelled with corresponding UTC", 100 );
            }
        }

        for ( i = 0; i < npts; i++ )
        {
            xx[i] = xmin + i * ( xmax - xmin ) / ( (PLFLT) ( npts - 1 ) );
            tai   = xx[i];
            plconfigtime( scale, 0., 0., 0x0, 1, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
            plbtime( &tai_year, &tai_month, &tai_day, &tai_hour, &tai_min, &tai_sec, tai );
            // Calculate residual using tai as the epoch to nearly maximize time-representation precision.
            plconfigtime( scale, 0., 0., 0x0, 1, tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec );
            // Calculate continuous tai with new epoch.
            plctime( tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec, &tai );
            // Calculate broken-down utc (with leap seconds inserted) from continuous tai with new epoch.
            plconfigtime( scale, 0., 0., 0x2, 1, tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec );
            plbtime( &utc_year, &utc_month, &utc_day, &utc_hour, &utc_min, &utc_sec, tai );
            // Calculate continuous utc from broken-down utc using same epoch as for the continuous tai.
            plconfigtime( scale, 0., 0., 0x0, 1, tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec );
            plctime( utc_year, utc_month, utc_day, utc_hour, utc_min, utc_sec, &utc );
            // Convert residuals to seconds.
            yy[i] = ( tai - utc ) * scale * 86400.;
        }

        pladv( 0 );
        plvsta();
        plwind( xmin, xmax, ymin, ymax );
        plcol0( 1 );
        if ( if_TAI_time_format )
            plconfigtime( scale, 0., 0., 0x0, 1, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
        else
            plconfigtime( scale, 0., 0., 0x2, 1, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
        pltimefmt( time_format );
        plbox( "bcnstd", xlabel_step, 0, "bcnstv", 0., 0 );
        plcol0( 3 );
        strncpy( title, "@frPLplot Example 29 - TAI-UTC ", 100 );
        strncat( title, title_suffix, 100 - strlen( title ) - 1 );
        pllab( xtitle, "TAI-UTC (sec)", title );

        plcol0( 4 );

        plline( npts, xx, yy );
    }
}
