#  Copyright (C) 2008 Andrew Ross
#  Copyright (C) 2008-2016 Alan W. Irwin

#  Sample plots using date / time formatting for axes
#
#  This file is part of PLplot.
#
#  PLplot is free software; you can redistribute it and/or modify
#  it under the terms of the GNU Library General Public License as published
#  by the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  PLplot is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Library General Public License for more details.
#
#  You should have received a copy of the GNU Library General Public License
#  along with PLplot; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
from numpy import *

import calendar

#--------------------------------------------------------------------------
# main
#
#  Draws several plots which demonstrate the use of date / time formats for
#  the axis labels.
#  Time formatting is done using the system strftime routine. See the
#  documentation of this for full details of the available formats.
#
#  1) Plotting temperature over a day (using hours / minutes)
#  2) Plotting
#
#  Note: Times are stored as seconds since the epoch (usually 1st Jan 1970).
#
#--------------------------------------------------------------------------

def main(w):

    w.plsesc('@')

    plot1(w)

    plot2(w)

    plot3(w)

    plot4(w)

    # Restore defaults
    w.plsesc('#')

    # Must be done independently because otherwise this changes output files
    # and destroys agreement with C examples.
    #w.plcol0(1)

# Plot a model diurnal cycle of temperature
def plot1(w):

    # Data points every 10 minutes for 1 day
    npts = 73;

    xmin = 0.0;
    xmax = 60.0*60.0*24.0;    # Number of seconds in a day
    ymin = 10.0;
    ymax = 20.0;

    x = xmax*arange(npts)/float(npts)
    y = 15.0 - 5.0*cos(2*pi*x/xmax)
    xerr1 = x-60.0*5.0
    xerr2 = x+60.0*5.0
    yerr1 = y-0.1
    yerr2 = y+0.1

    w.pladv(0)

    w.plsmaj(0.0,0.5)
    w.plsmin(0.0,0.5)

    w.plvsta()
    w.plwind(xmin, xmax, ymin, ymax)

    # Draw a box with ticks spaced every 3 hour in X and 1 degree C in Y.
    w.plcol0(1)
    # Set time format to be hours:minutes
    w.pltimefmt("%H:%M")
    w.plbox("bcnstd", 3.0*60*60, 3, "bcnstv", 1, 5)

    w.plcol0(3)
    w.pllab("Time (hours:mins)", "Temperature (degC)", "@frPLplot Example 29 - Daily temperature")

    w.plcol0(4)

    w.plline(x, y)
    w.plcol0(2)
    w.plerrx(xerr1,xerr2,y)
    w.plcol0(3)
    w.plerry(x,yerr1,yerr2)

    w.plsmin(0.0,1.0)
    w.plsmaj(0.0,1.0)

# Plot the number of hours of daylight as a function of day for a year
def plot2(w):

    # Latitude for London
    lat = 51.5

    npts = 365

    xmin = 0.0
    xmax = npts*60.0*60.0*24.0
    ymin = 0.0
    ymax = 24.0

    # Formula for hours of daylight from
    # "A Model Comparison for Daylength as a Function of Latitude and
    # Day of the Year", 1995, Ecological Modelling, 80, pp 87-95.
    x = arange(npts)*60.0*60.0*24.0
    p = arcsin(0.39795*cos(0.2163108 + 2*arctan(0.9671396*tan(0.00860*(arange(npts)-186)))))
    d = 24.0 - (24.0/pi)*arccos( (sin(0.8333*pi/180.0) + sin(lat*pi/180.0)*sin(p)) / (cos(lat*pi/180.0)*cos(p)) )
    y = d

    w.plcol0(1)
    # Set time format to be abbreviated month name followed by day of month
    w.pltimefmt("%b %d")
    w.plprec(1,1)
    w.plenv(xmin, xmax, ymin, ymax, 0, 40)


    w.plcol0(3)
    w.pllab("Date", "Hours of daylight", "@frPLplot Example 29 - Hours of daylight at 51.5N")

    w.plcol0(4)

    w.plline(x, y)

    w.plprec(0,0)

def plot3(w):

# number of seconds elapsed since the Unix epoch (1970-01-01, UTC) for
# 2005-12-01, UTC.
    xmin = w.plctime(2005,11,1,0,0,0.)

    npts = 62

    xmax = xmin + npts*60.0*60.0*24.0
    ymin = 0.0
    ymax = 5.0

    i = arange(npts)
    imin = float(npts)/2.0-abs(i - float(npts)/2.0)
    x = xmin + i*60.0*60.0*24.0
    y = 1.0 + sin(2*pi*i/7.0) + exp( imin / 31.0)
    w.pladv(0)

    w.plvsta()
    w.plwind(xmin, xmax, ymin, ymax)

    w.plcol0(1)
    # Set time format to be ISO 8601 standard YYYY-MM-DD. Note that this is
    # equivalent to %f for C99 compliant implementations of strftime.
    w.pltimefmt("%Y-%m-%d")
    # Draw a box with ticks spaced every 14 days in X and 1 hour in Y.
    w.plbox("bcnstd", 14*24.0*60.0*60.0,14, "bcnstv", 1, 4)

    w.plcol0(3)
    w.pllab("Date", "Hours of television watched", "@frPLplot Example 29 - Hours of television watched in Dec 2005 / Jan 2006")

    w.plcol0(4)

    w.plssym(0.0,0.5)
    w.plpoin(x, y, 2)
    w.plline(x, y)

def plot4(w):

    # TAI-UTC (seconds) as a function of time.

    # Continuous time unit is Besselian years from whatever epoch is
    # chosen below.  Could change to seconds (or days) from the
    # epoch, but then would have to adjust xlabel_step below.
    scale = 365.242198781
    # MJD epoch (see <https://en.wikipedia.org/wiki/Julian_day>).
    # This is only set for illustrative purposes, and is overwritten
    # below for the time-representation reasons given in the
    # discussion below.
    epoch_year  = 1858
    epoch_month = 11
    epoch_day   = 17
    epoch_hour  = 0
    epoch_min   = 0
    epoch_sec   = 0.
    # To illustrate the time-representation issues of using the MJD
    # epoch, in 1985, MJD was roughly 46000 days which corresponds to
    # 4e9 seconds.  Thus, for the -DPL_DOUBLE=ON case where PLFLT is
    # a double which can represent continuous time to roughly 16
    # decimal digits of precision the time-representation error is
    # roughly ~400 nanoseconds.  Therefore the MJD epoch would be
    # acceptable for the plots below in the -DPL_DOUBLE=ON case.
    # However, that epoch is obviously not acceptable for the
    # -DPL_DOUBLE=OFF case where PLFLT is a float which can represent
    # continuous time to only ~7 decimal digits of precision
    # corresponding to a time representation error of 400 seconds (!)
    # in 1985.  For this reason, we do not use the MJD epoch below
    # and instead choose the best epoch for each case to minimize
    # time-representation issues.
    for kind in range(7):
        if kind == 0:
            # Choose midpoint to maximize time-representation precision.
            epoch_year  = 1985
            epoch_month = 0
            epoch_day   = 2
            epoch_hour  = 0
            epoch_min   = 0
            epoch_sec   = 0.
            w.plconfigtime(scale, 0., 0., 0x0, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec)
            xmin = w.plctime(1950,0,2,0,0,0.)
            xmax = w.plctime(2020,0,2,0,0,0.)
            npts = 70*12 + 1
            ymin = 0.0
            ymax = 36.0
            time_format = "%Y%"
            if_TAI_time_format = True
            title_suffix = "from 1950 to 2020"
            xtitle = "Year"
            xlabel_step = 10.
        elif kind == 1 or kind ==2:
            # Choose midpoint to maximize time-representation precision.
            epoch_year  = 1961
            epoch_month = 7
            epoch_day   = 1
            epoch_hour  = 0
            epoch_min   = 0
            epoch_sec   = 1.64757
            w.plconfigtime(scale, 0., 0., 0x0, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec)
            xmin = w.plctime(1961,7,1,0,0,1.64757-.20)
            xmax = w.plctime(1961,7,1,0,0,1.64757+.20)
            npts = 1001
            ymin = 1.625
            ymax = 1.725
            time_format = "%S%2%"
            title_suffix = "near 1961-08-01 (TAI)"
            xlabel_step = 0.05/(scale*86400.)
            if kind == 1:
                if_TAI_time_format = True
                xtitle = "Seconds (TAI)"
            else:
                if_TAI_time_format = False
                xtitle = "Seconds (TAI) labelled with corresponding UTC"
        elif kind == 3 or kind ==4:
            # Choose midpoint to maximize time-representation precision.
            epoch_year  = 1963
            epoch_month = 10
            epoch_day   = 1
            epoch_hour  = 0
            epoch_min   = 0
            epoch_sec   = 2.6972788
            w.plconfigtime(scale, 0., 0., 0x0, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec)
            xmin = w.plctime(1963,10,1,0,0,2.6972788-.20)
            xmax = w.plctime(1963,10,1,0,0,2.6972788+.20)
            npts = 1001
            ymin = 2.55
            ymax = 2.75
            time_format = "%S%2%"
            title_suffix = "near 1963-11-01 (TAI)"
            xlabel_step = 0.05/(scale*86400.)
            if kind == 3:
                if_TAI_time_format = True
                xtitle = "Seconds (TAI)"
            else:
                if_TAI_time_format = False
                xtitle = "Seconds (TAI) labelled with corresponding UTC"
        elif kind == 5 or kind == 6:
            # Choose midpoint to maximize time-representation precision.
            epoch_year  = 2009
            epoch_month = 0
            epoch_day   = 1
            epoch_hour  = 0
            epoch_min   = 0
            epoch_sec   = 34.
            w.plconfigtime(scale, 0., 0., 0x0, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec)
            xmin = w.plctime(2009,0,1,0,0,34.-5.)
            xmax = w.plctime(2009,0,1,0,0,34.+5.)
            npts = 1001
            ymin = 32.5
            ymax = 34.5
            time_format = "%S%2%"
            title_suffix = "near 2009-01-01 (TAI)"
            xlabel_step = 1./(scale*86400.)
            if kind == 5:
                if_TAI_time_format = True
                xtitle = "Seconds (TAI)"
            else:
                if_TAI_time_format = False
                xtitle = "Seconds (TAI) labelled with corresponding UTC"

        i = arange(npts)
        x = xmin + i*(xmax-xmin)/float(npts-1)
        y = zeros(npts)
        for j in range(npts):
            tai = x[j]
            w.plconfigtime(scale, 0., 0., 0x0, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec)
            (tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec) = w.plbtime(tai)
            # Calculate residual using tai as the epoch to nearly maximize time-representation precision.
            w.plconfigtime(scale, 0., 0., 0x0, True, tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec)
            # Calculate continuous tai with new epoch.
            tai = w.plctime(tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec)
            # Calculate broken-down utc (with leap seconds inserted) from continuous tai with new epoch.
            w.plconfigtime(scale, 0., 0., 0x2, True, tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec)
            (utc_year, utc_month, utc_day, utc_hour, utc_min, utc_sec) = w.plbtime(tai)
            # Calculate continuous utc from broken-down utc using same epoch as for the continuous tai.
            w.plconfigtime(scale, 0., 0., 0x0, True, tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec)
            utc = w.plctime(utc_year, utc_month, utc_day, utc_hour, utc_min, utc_sec)
            # Convert residuals to seconds.
            y[j]=(tai-utc)*scale*86400.

        w.pladv(0)
        w.plvsta()
        w.plwind(xmin, xmax, ymin, ymax)
        w.plcol0(1)
        if if_TAI_time_format:
            w.plconfigtime(scale, 0., 0., 0x0, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec)
        else:
          w.plconfigtime(scale, 0., 0., 0x2, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec)
        w.pltimefmt(time_format)
        w.plbox("bcnstd", xlabel_step, 0, "bcnstv", 0., 0)
        w.plcol0(3)
        w.pllab(xtitle, "TAI-UTC (sec)", "@frPLplot Example 29 - TAI-UTC " + title_suffix)

        w.plcol0(4)

        w.plline(x, y)
