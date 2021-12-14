//       Sample plots using date / time formatting for axes
//
// Copyright (C) 2008 Andrew Ross
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

package plplot.examples;

import plplot.core.*;
import static plplot.core.plplotjavacConstants.*;

import java.util.*;


class x29 {
    PLStream pls = new PLStream();

    //------------------------------------------------------------------------
    // main
    //
    // Draws several plots which demonstrate the use of date / time formats
    // for the axis labels.
    // Time formatting is done using the system strftime routine. See the
    // documentation of this for full details of the available formats.
    //
    // 1) Plotting temperature over a day (using hours / minutes)
    // 2) Plotting
    //
    // Note: Times are stored as seconds since the epoch (usually
    // 1st Jan 1970).
    //
    //------------------------------------------------------------------------

    x29( String[] args )
    {
        // Parse command line arguments
        pls.parseopts( args, PL_PARSE_FULL | PL_PARSE_NOPROGRAM );

        // Initialize plplot
        pls.init();

        pls.sesc( '@' );

        plot1();

        plot2();

        plot3();

        plot4();

        pls.end();
    }

    // Plot a model diurnal cycle of temperature
    void plot1()
    {
        int    i, npts;
        double xmin, xmax, ymin, ymax;
        double x[], y[], xerr1[], xerr2[], yerr1[], yerr2[];

        // Data points every 10 minutes for 1 day
        npts = 73;

        x     = new double[npts];
        y     = new double[npts];
        xerr1 = new double[npts];
        xerr2 = new double[npts];
        yerr1 = new double[npts];
        yerr2 = new double[npts];

        xmin = 0;
        xmax = 60.0 * 60.0 * 24.0;    // Number of seconds in a day
        ymin = 10.0;
        ymax = 20.0;

        for ( i = 0; i < npts; i++ )
        {
            x[i] = xmax * ( (double) i / (double) npts );
            y[i] = 15.0 - 5.0 * Math.cos( 2 * Math.PI * ( (double) i / (double) npts ) );
            // Set x error bars to +/- 5 minute
            xerr1[i] = x[i] - 60 * 5;
            xerr2[i] = x[i] + 60 * 5;
            // Set y error bars to +/- 0.1 deg C
            yerr1[i] = y[i] - 0.1;
            yerr2[i] = y[i] + 0.1;
        }

        pls.adv( 0 );

        // Rescale major ticks marks by 0.5
        pls.smaj( 0.0, 0.5 );
        // Rescale minor ticks and error bar marks by 0.5
        pls.smin( 0.0, 0.5 );

        pls.vsta();
        pls.wind( xmin, xmax, ymin, ymax );

        // Draw a box with ticks spaced every 3 hour in X and 1 degree C in Y.
        pls.col0( 1 );
        // Set time format to be hours:minutes
        pls.timefmt( "%H:%M" );
        pls.box( "bcnstd", 3.0 * 60 * 60, 3, "bcnstv", 1, 5 );

        pls.col0( 3 );
        pls.lab( "Time (hours:mins)", "Temperature (degC)", "@frPLplot Example 29 - Daily temperature" );

        pls.col0( 4 );

        pls.line( x, y );
        pls.col0( 2 );
        pls.errx( xerr1, xerr2, y );
        pls.col0( 3 );
        pls.erry( x, yerr1, yerr2 );

        // Rescale major / minor tick marks back to default
        pls.smin( 0.0, 1.0 );
        pls.smaj( 0.0, 1.0 );
    }

    // Plot the number of hours of daylight as a function of day for a year
    void plot2()
    {
        int    j, npts;
        double xmin, xmax, ymin, ymax;
        double lat, p, d;
        double x[], y[];

        // Latitude for London
        lat = 51.5;

        npts = 365;

        x = new double[npts];
        y = new double[npts];

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
            p    = Math.asin( 0.39795 * Math.cos( 0.2163108 + 2 * Math.atan( 0.9671396 * Math.tan( 0.00860 * ( j - 186 ) ) ) ) );
            d    = 24.0 - ( 24.0 / Math.PI ) *
                   Math.acos( ( Math.sin( 0.8333 * Math.PI / 180.0 ) + Math.sin( lat * Math.PI / 180.0 ) * Math.sin( p ) ) /
                ( Math.cos( lat * Math.PI / 180.0 ) * Math.cos( p ) ) );
            y[j] = d;
        }

        pls.col0( 1 );
        // Set time format to be abbreviated month name followed by day of month
        pls.timefmt( "%b %d" );
        pls.prec( 1, 1 );
        pls.env( xmin, xmax, ymin, ymax, 0, 40 );


        pls.col0( 3 );
        pls.lab( "Date", "Hours of daylight", "@frPLplot Example 29 - Hours of daylight at 51.5N" );

        pls.col0( 4 );

        pls.line( x, y );

        pls.prec( 0, 0 );
    }

    void plot3()
    {
        int      i, npts;
        double   xmin, xmax, ymin, ymax;
        long     tstart;
        double   x[], y[];

        TimeZone tz  = TimeZone.getTimeZone( "UTC" );
        Calendar cal = Calendar.getInstance( tz );

        cal.set( 2005, 11, 1, 0, 0, 0 );

        tstart = cal.getTimeInMillis() / 1000;

        npts = 62;

        x = new double[npts];
        y = new double[npts];

        xmin = tstart;
        xmax = xmin + npts * 60.0 * 60.0 * 24.0;
        ymin = 0.0;
        ymax = 5.0;

        for ( i = 0; i < npts; i++ )
        {
            x[i] = xmin + i * 60.0 * 60.0 * 24.0;
            y[i] = 1.0 + Math.sin( 2 * Math.PI * ( i ) / 7.0 ) +
                   Math.exp( ( Math.min( i, npts - i ) ) / 31.0 );
        }
        pls.adv( 0 );

        pls.vsta();
        pls.wind( xmin, xmax, ymin, ymax );

        pls.col0( 1 );
        // Set time format to be ISO 8601 standard YYYY-MM-HH. Note that this is
        // equivalent to %f for C99 compliant implementations of strftime.
        pls.timefmt( "%Y-%m-%d" );
        // Draw a box with ticks spaced every 14 days in X and 1 hour in Y.
        pls.box( "bcnstd", 14 * 24.0 * 60.0 * 60.0, 14, "bcnstv", 1, 4 );

        pls.col0( 3 );
        pls.lab( "Date", "Hours of television watched", "@frPLplot Example 29 - Hours of television watched in Dec 2005 / Jan 2006" );

        pls.col0( 4 );

        pls.ssym( 0.0, 0.5 );
        pls.poin( x, y, 2 );
        pls.line( x, y );
    }

    void plot4()
    {
        // TAI-UTC (seconds) as a function of time.

        double  scale;
        double  xmin[] = new double[1], xmax[] = new double[1];
        double  ymin   = 0.0, ymax = 0.0, xlabel_step = 0.0;
        int     kind, npts = 1001, i;
        boolean if_TAI_time_format = false;
        String  time_format        = "";
        String  title_suffix       = "";
        String  xtitle             = "";
        String  title = "";
        double  x[];
        double  y[];
        int     epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min;
        double  epoch_sec;
        int     tai_year[] = new int[1], tai_month[] = new int[1],
                tai_day[]  = new int[1], tai_hour[] = new int[1],
                tai_min[]  = new int[1];
        double tai_sec[]   = new double[1], tai[] = new double[1];
        int    utc_year[]  = new int[1], utc_month[] = new int[1],
               utc_day[]   = new int[1], utc_hour[] = new int[1],
               utc_min[]   = new int[1];
        double utc_sec[]   = new double[1], utc[] = new double[1];

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
                pls.configtime( scale, 0., 0., 0x0, true, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
                pls.ctime( 1950, 0, 2, 0, 0, 0., xmin );
                pls.ctime( 2020, 0, 2, 0, 0, 0., xmax );
                npts               = 70 * 12 + 1;
                ymin               = 0.0;
                ymax               = 36.0;
                time_format        = "%Y%";
                if_TAI_time_format = true;
                title_suffix       = "from 1950 to 2020";
                xtitle             = "Year";
                xlabel_step        = 10.;
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
                pls.configtime( scale, 0., 0., 0x0, true, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
                pls.ctime( 1961, 7, 1, 0, 0, 1.64757 - .20, xmin );
                pls.ctime( 1961, 7, 1, 0, 0, 1.64757 + .20, xmax );
                npts         = 1001;
                ymin         = 1.625;
                ymax         = 1.725;
                time_format  = "%S%2%";
                title_suffix = "near 1961-08-01 (TAI)";
                xlabel_step  = 0.05 / ( scale * 86400. );
                if ( kind == 1 )
                {
                    if_TAI_time_format = true;
                    xtitle             = "Seconds (TAI)";
                }
                else
                {
                    if_TAI_time_format = false;
                    xtitle             = "Seconds (TAI) labelled with corresponding UTC";
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
                pls.configtime( scale, 0., 0., 0x0, true, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
                pls.ctime( 1963, 10, 1, 0, 0, 2.6972788 - .20, xmin );
                pls.ctime( 1963, 10, 1, 0, 0, 2.6972788 + .20, xmax );
                npts         = 1001;
                ymin         = 2.55;
                ymax         = 2.75;
                time_format  = "%S%2%";
                title_suffix = "near 1963-11-01 (TAI)";
                xlabel_step  = 0.05 / ( scale * 86400. );
                if ( kind == 3 )
                {
                    if_TAI_time_format = true;
                    xtitle             = "Seconds (TAI)";
                }
                else
                {
                    if_TAI_time_format = false;
                    xtitle             = "Seconds (TAI) labelled with corresponding UTC";
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
                pls.configtime( scale, 0., 0., 0x0, true, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
                pls.ctime( 2009, 0, 1, 0, 0, 34. - 5., xmin );
                pls.ctime( 2009, 0, 1, 0, 0, 34. + 5., xmax );
                npts         = 1001;
                ymin         = 32.5;
                ymax         = 34.5;
                time_format  = "%S%2%";
                title_suffix = "near 2009-01-01 (TAI)";
                xlabel_step  = 1. / ( scale * 86400. );
                if ( kind == 5 )
                {
                    if_TAI_time_format = true;
                    xtitle             = "Seconds (TAI)";
                }
                else
                {
                    if_TAI_time_format = false;
                    xtitle             = "Seconds (TAI) labelled with corresponding UTC";
                }
            }

            x = new double[npts];
            y = new double[npts];
            for ( i = 0; i < npts; i++ )
            {
                x[i]   = xmin[0] + i * ( xmax[0] - xmin[0] ) / ( npts - 1 );
                tai[0] = x[i];
                pls.configtime( scale, 0., 0., 0x0, true, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
                pls.btime( tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec, tai[0] );
                // Calculate residual using tai as the epoch to nearly maximize time-representation precision.
                pls.configtime( scale, 0., 0., 0x0, true, tai_year[0], tai_month[0], tai_day[0], tai_hour[0], tai_min[0], tai_sec[0] );
                // Calculate continuous tai with new epoch.
                pls.ctime( tai_year[0], tai_month[0], tai_day[0], tai_hour[0], tai_min[0], tai_sec[0], tai );
                // Calculate broken-down utc (with leap seconds inserted) from continuous tai with new epoch.
                pls.configtime( scale, 0., 0., 0x2, true, tai_year[0], tai_month[0], tai_day[0], tai_hour[0], tai_min[0], tai_sec[0] );
                pls.btime( utc_year, utc_month, utc_day, utc_hour, utc_min, utc_sec, tai[0] );
                // Calculate continuous utc from broken-down utc using same epoch as for the continuous tai.
                pls.configtime( scale, 0., 0., 0x0, true, tai_year[0], tai_month[0], tai_day[0], tai_hour[0], tai_min[0], tai_sec[0] );
                pls.ctime( utc_year[0], utc_month[0], utc_day[0], utc_hour[0], utc_min[0], utc_sec[0], utc );
                // Convert residuals to seconds.
                y[i] = ( tai[0] - utc[0] ) * scale * 86400.;
            }

            pls.adv( 0 );
            pls.vsta();
            pls.wind( xmin[0], xmax[0], ymin, ymax );
            pls.col0( 1 );
            if ( if_TAI_time_format )
                pls.configtime( scale, 0., 0., 0x0, true, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
            else
                pls.configtime( scale, 0., 0., 0x2, true, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
            pls.timefmt( time_format );
            pls.box( "bcnstd", xlabel_step, 0, "bcnstv", 0., 0 );
            pls.col0( 3 );
            title = "@frPLplot Example 29 - TAI-UTC " + title_suffix;
            pls.lab( xtitle, "TAI-UTC (sec)", title );

            pls.col0( 4 );

            pls.line( x, y );
        }
    }


    public static void main( String[] args )
    {
        new x29( args );
    }
};

