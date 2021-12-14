## Sample plots using date / time formatting for axes
##
## Copyright (C) 2007, 2008 Andrew Ross
##
##
## This file is part of PLplot.
##
## PLplot is free software; you can redistribute it and/or modify
## it under the terms of the GNU Library General Public License as published
## by the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## PLplot is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Library General Public License for more details.
##
## You should have received a copy of the GNU Library General Public License
## along with PLplot; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
##

1;

##

function ix29c


    ## Initialize plplot
    plinit();

    plsesc('@');

    plot1();

    plot2();

    plot3();

    plot4();

    plend1();

endfunction

## Plot a model diurnal cycle of temperature
function plot1

## Data points every 10 minutes for 1 day
    npts = 73;

    xmin = 0;
    xmax = 60.0*60.0*24.0;    ## Number of seconds in a day
    ymin = 10.0;
    ymax = 20.0;

    i = 0:npts-1;
    x = i*xmax/npts;
    y = 15.0 - 5.0*cos( 2*pi*i/npts);
    xerr1 = x-60.0*5.0;
    xerr2 = x+60.0*5.0;
    yerr1 = y-0.1;
    yerr2 = y+0.1;

    pladv(0);

    ## Rescale major ticks marks by 0.5
    plsmaj(0.0,0.5);
    ## Rescale minor ticks and error bar marks by 0.5
    plsmin(0.0,0.5);

    plvsta();
    plwind(xmin, xmax, ymin, ymax);

    ## Draw a box with ticks spaced every 3 hour in X and 1 degree C in Y.
    plcol0(1);
    ## Set time format to be hours:minutes
    pltimefmt("%H:%M");
    plbox("bcnstd", 3.0*60*60, 3, "bcnstv", 1, 5);

    plcol0(3);
    pllab("Time (hours:mins)", "Temperature (degC)", "@frPLplot Example 29 - Daily temperature");

    plcol0(4);

    plline(x', y');
    plcol0(2);
    plerrx(xerr1', xerr2', y');
    plcol0(3);
    plerry(x', yerr1', yerr2');

    ## Rescale major / minor tick marks back to default
    plsmin(0.0,1.0);
    plsmaj(0.0,1.0);

endfunction

## Plot the number of hours of daylight as a function of day for a year
function plot2

## Latitude for London
    lat = 51.5;

    npts = 365;

    xmin = 0;
    xmax = npts*60.0*60.0*24.0;
    ymin = 0;
    ymax = 24;

    ## Formula for hours of daylight from
    ## "A Model Comparison for Daylength as a Function of Latitude and
    ## Day of the Year", 1995, Ecological Modelling, 80, pp 87-95.
    i = 0:npts-1;
    x = i*60.0*60.0*24.0;
    p = asin(0.39795*cos(0.2163108 + 2*atan(0.9671396*tan(0.00860*(i-186)))));
    d = 24.0 - (24.0/pi)*acos( (sin(0.8333*pi/180.0) + ...
                                sin(lat*pi/180.0)*sin(p)) ...
                               ./(cos(lat*pi/180.0)*cos(p)) );
    y = d;

    plcol0(1);
    ## Set time format to be abbreviated month name followed by day of month
    pltimefmt("%b %d");
    plprec(1,1);
    plenv(xmin, xmax, ymin, ymax, 0, 40);


    plcol0(3);
    pllab("Date", "Hours of daylight", "@frPLplot Example 29 - Hours of daylight at 51.5N");

    plcol0(4);

    plline(x', y');

    plprec(0,0);

endfunction

function plot3

    ## Calculate seconds since the Unix epoch for 2005-12-01 UTC.
    tm.year = 105; ## Years since 1900
    tm.mon = 11;   ## 0 == January, 11 = December
    tm.mday = 1;   ## 1 = 1st of month
    tm.hour = 0;
    tm.min = 0;
    tm.sec = 0;

    ## NB - no need to call tzset in octave - it doesn't exist.
    tz = getenv("TZ");
    putenv("TZ","");

    ## tstart is a time_t value (cast to PLFLT) which represents the number
    ## of seconds elapsed since 00:00:00 on January 1, 1970, Coordinated
    ## Universal Time (UTC).
    tstart = mktime(tm);

    ## Note currently octave appears to have no way to unset a env
    ## variable.
    putenv("TZ",tz);

    npts = 62;

    xmin = tstart;
    xmax = xmin + npts*60.0*60.0*24.0;
    ymin = 0.0;
    ymax = 5.0;

    i = 0:npts-1;
    x = xmin + i*60.0*60.0*24.0;
    y = 1.0 + sin( 2*pi*i / 7.0 ) + exp( min(i,npts-i) / 31.0);

    pladv(0);

    plvsta();
    plwind(xmin, xmax, ymin, ymax);
    plcol0(1);
    ## Set time format to be ISO 8601 standard YYYY-MM-DD. Note that this is
    ## equivalent to %f for C99 compliant implementations of strftime.
    pltimefmt("%Y-%m-%d");
    ## Draw a box with ticks spaced every 14 days in X and 1 hour in Y.
    plbox("bcnstd", 14*24.0*60.0*60.0,14, "bcnstv", 1, 4);

    plcol0(3);
    pllab("Date", "Hours of television watched", "@frPLplot Example 29 - Hours of television watched in Dec 2005 / Jan 2006");

    plcol0(4);

    ## Rescale symbol size (used by plpoin) by 0.5
    plssym(0.0,0.5);
    plpoin(x', y', 2);
    plline(x', y');

endfunction

function plot4

    ## TAI-UTC (seconds) as a function of time.

    ## Continuous time unit is Besselian years from whatever epoch is
    ## chosen below.  Could change to seconds (or days) from the
    ## epoch, but then would have to adjust xlabel_step below.
    scale = 365.242198781;
    ## MJD epoch (see <https://en.wikipedia.org/wiki/Julian_day>).
    ## This is only set for illustrative purposes, and is overwritten
    ## below for the time-representation reasons given in the
    ## discussion below.
    epoch_year  = 1858;
    epoch_month = 11;
    epoch_day   = 17;
    epoch_hour  = 0;
    epoch_min   = 0;
    epoch_sec   = 0.;
    ## To illustrate the time-representation issues of using the MJD
    ## epoch, in 1985, MJD was roughly 46000 days which corresponds to
    ## 4e9 seconds.  Thus, for the -DPL_DOUBLE=ON case where PLFLT is
    ## a double which can represent continuous time to roughly 16
    ## decimal digits of precision the time-representation error is
    ## roughly ~400 nanoseconds.  Therefore the MJD epoch would be
    ## acceptable for the plots below in the -DPL_DOUBLE=ON case.
    ## However, that epoch is obviously not acceptable for the
    ## -DPL_DOUBLE=OFF case where PLFLT is a float which can represent
    ## continuous time to only ~7 decimal digits of precision
    ## corresponding to a time representation error of 400 seconds (!)
    ## in 1985.  For this reason, we do not use the MJD epoch below
    ## and instead choose the best epoch for each case to minimize
    ## time-representation issues.

    for kind=0:6
        if (kind == 0)
            ## Choose midpoint to maximize time-representation precision.
            epoch_year  = 1985;
            epoch_month = 0;
            epoch_day   = 2;
            epoch_hour  = 0;
            epoch_min   = 0;
            epoch_sec   = 0.;
            plconfigtime( scale, 0., 0., 0x0, 1, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
            xmin = plctime(1950,0,2,0,0,0.);
            xmax = plctime(2020,0,2,0,0,0.);
            npts = 70*12 + 1;
            ymin = 0.0;
            ymax = 36.0;
            time_format = "%Y%";
            if_TAI_time_format = 1;
            title_suffix = "from 1950 to 2020";
            xtitle =  "Year";
            xlabel_step = 10.;
        elseif (kind == 1 || kind ==2)
            ## Choose midpoint to maximize time-representation precision.
            epoch_year  = 1961;
            epoch_month = 7;
            epoch_day   = 1;
            epoch_hour  = 0;
            epoch_min   = 0;
            epoch_sec   = 1.64757;
            plconfigtime( scale, 0., 0., 0x0, 1, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
            xmin = plctime(1961,7,1,0,0,1.64757-.20);
            xmax = plctime(1961,7,1,0,0,1.64757+.20);
            npts = 1001;
            ymin = 1.625;
            ymax = 1.725;
            time_format = "%S%2%";
            title_suffix = "near 1961-08-01 (TAI)";
            xlabel_step = 0.05/(scale*86400.);
            if (kind == 1)
                if_TAI_time_format = 1;
                xtitle = "Seconds (TAI)";
            else
                if_TAI_time_format = 0;
                xtitle = "Seconds (TAI) labelled with corresponding UTC";
            endif
        elseif (kind == 3 || kind ==4)
            ## Choose midpoint to maximize time-representation precision.
            epoch_year  = 1963;
            epoch_month = 10;
            epoch_day   = 1;
            epoch_hour  = 0;
            epoch_min   = 0;
            epoch_sec   = 2.6972788;
            plconfigtime( scale, 0., 0., 0x0, 1, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
            xmin = plctime(1963,10,1,0,0,2.6972788-.20);
            xmax = plctime(1963,10,1,0,0,2.6972788+.20);
            npts = 1001;
            ymin = 2.55;
            ymax = 2.75;
            time_format = "%S%2%";
            title_suffix = "near 1963-11-01 (TAI)";
            xlabel_step = 0.05/(scale*86400.);
            if (kind == 3)
                if_TAI_time_format = 1;
                xtitle = "Seconds (TAI)";
            else
                if_TAI_time_format = 0;
                xtitle = "Seconds (TAI) labelled with corresponding UTC";
            endif
        elseif (kind == 5 || kind == 6)
            ## Choose midpoint to maximize time-representation precision.
            epoch_year  = 2009;
            epoch_month = 0;
            epoch_day   = 1;
            epoch_hour  = 0;
            epoch_min   = 0;
            epoch_sec   = 34.;
            plconfigtime( scale, 0., 0., 0x0, 1, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
            xmin = plctime(2009,0,1,0,0,34.-5.);
            xmax = plctime(2009,0,1,0,0,34.+5.);
            npts = 1001;
            ymin = 32.5;
            ymax = 34.5;
            time_format = "%S%2%";
            title_suffix = "near 2009-01-01 (TAI)";
            xlabel_step = 1./(scale*86400.);
            if (kind == 5)
                if_TAI_time_format = 1;
                xtitle = "Seconds (TAI)";
            else
                if_TAI_time_format = 0;
                xtitle = "Seconds (TAI) labelled with corresponding UTC";
            endif
        endif

        x = xmin + (0:npts-1)*(xmax-xmin)/(npts-1);
        tai = x;
        for i=1:npts
            plconfigtime( scale, 0., 0., 0x0, 1, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
            [tai_year(i) , tai_month(i), tai_day(i), tai_hour(i), tai_min(i), tai_sec(i)] = plbtime(tai(i));
            ## Calculate residual using tai as the epoch to nearly maximize time-representation precision.
            plconfigtime( scale, 0., 0., 0x0, 1, tai_year(i) , tai_month(i), tai_day(i), tai_hour(i), tai_min(i), tai_sec(i));
            ## Calculate continuous tai with new epoch.
            tai(i) = plctime(tai_year(i), tai_month(i), tai_day(i), tai_hour(i), tai_min(i), tai_sec(i));
            ## Calculate broken-down utc (with leap seconds inserted) from continuous tai with new epoch.
            plconfigtime( scale, 0., 0., 0x2, 1, tai_year(i) , tai_month(i), tai_day(i), tai_hour(i), tai_min(i), tai_sec(i));
            [utc_year(i), utc_month(i), utc_day(i), utc_hour(i), utc_min(i), utc_sec(i)] = plbtime(tai(i));
            ## Calculate continuous utc from broken-down utc using same epoch as for the continuous tai.
            plconfigtime( scale, 0., 0., 0x0, 1, tai_year(i) , tai_month(i), tai_day(i), tai_hour(i), tai_min(i), tai_sec(i));
            utc(i) = plctime(utc_year(i), utc_month(i), utc_day(i), utc_hour(i), utc_min(i), utc_sec(i));
        endfor
        ## Convert residuals to seconds.
        y = (tai-utc)*scale*86400.0;

        pladv(0);
        plvsta();
        plwind(xmin, xmax, ymin, ymax);
        plcol0(1);
        if (if_TAI_time_format)
            plconfigtime( scale, 0., 0., 0x0, 1, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
        else
            plconfigtime( scale, 0., 0., 0x2, 1, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec );
        endif
        pltimefmt(time_format);
        plbox("bcnstd", xlabel_step, 0, "bcnstv", 0., 0);
        plcol0(3);
        title = ["@frPLplot Example 29 - TAI-UTC ", title_suffix];
        pllab(xtitle, "TAI-UTC (sec)", title);

        plcol0(4);

        plline(x', y');
    endfor
endfunction

ix29c
