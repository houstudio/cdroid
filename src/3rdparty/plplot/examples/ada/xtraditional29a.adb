-- Sample plots using date / time formatting for axes

-- Copyright (C) 2008-2010 Jerry Bauck

-- This file is part of PLplot.

-- PLplot is free software; you can redistribute it and/or modify
-- it under the terms of the GNU Library General Public License as published
-- by the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.

-- PLplot is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Library General Public License for more details.

-- You should have received a copy of the GNU Library General Public License
-- along with PLplot; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

with
    Ada.Strings.Unbounded,
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Traditional;
use
    Ada.Strings.Unbounded,
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Traditional;

--------------------------------------------------------------------------------
-- Draws several plots which demonstrate the use of date / time formats for
-- the axis labels.
-- Time formatting is done using the strfqsas routine from the qsastime
-- library.  This is similar to strftime, but works for a broad
-- date range even on 32-bit systems.  See the
-- documentation of strfqsas for full details of the available formats.
--
-- 1) Plotting temperature over a day (using hours / minutes)
-- 2) Plotting
--
--------------------------------------------------------------------------------

-- NOTE: The Ada user is reminded that Ada.Calendar is very capable and complete.
-- See especially Time_Of.

procedure xtraditional29a is
    -- Plot a model diurnal cycle of temperature
    procedure plot1 is
        x, y : Real_Vector(0 .. 72);
        xerr1, xerr2, yerr1, yerr2 : Real_Vector(0 .. 72);

        -- Data points every 10 minutes for 1 day
        xmin, xmax, ymin, ymax : Long_Float;
    begin
        xmin := 0.0;
        xmax := 60.0 * 60.0 * 24.0; -- Number of seconds in a day
        ymin := 10.0;
        ymax := 20.0;

        for i in x'range loop
            x(i) := xmax * Long_Float(i) / Long_Float(x'length);
            y(i) := 15.0 - 5.0 * cos( 2.0 * pi * Long_Float(i) / Long_Float(x'length));

            -- Set x error bars to +/- 5 minute
            xerr1(i) := x(i) - Long_Float(60 * 5);
            xerr2(i) := x(i) + Long_Float(60 * 5);

            -- Set y error bars to +/- 0.1 deg C */
            yerr1(i) := y(i) - 0.1;
            yerr2(i) := y(i) + 0.1;
        end loop;

        pladv(0);

        -- Rescale major ticks marks by 0.5
        plsmaj(0.0, 0.5);

        -- Rescale minor ticks and error bar marks by 0.5
        plsmin(0.0, 0.5);

        plvsta;
        plwind(xmin, xmax, ymin, ymax);

        -- Draw a box with ticks spaced every 3 hours in X and 1 degree C in Y.
        plcol0(1);

        -- Set time format to be hours:minutes
        pltimefmt("%H:%M");
        plbox("bcnstd", 3.0 * 60.0 * 60.0, 3, "bcnstv", 1.0, 5);

        plcol0(3);
        pllab("Time (hours:mins)", "Temperature (degC)", "@frPLplot Example 29 - Daily temperature");
        plcol0(4);
        plline(x, y);

        plcol0(2);
        plerrx(xerr1, xerr2, y);
        plcol0(3);
        plerry(x, yerr1, yerr2);

        -- Rescale major / minor tick marks back to default
        plsmin(0.0, 1.0);
        plsmaj(0.0, 1.0);
    end plot1;


    -- Plot the number of hours of daylight as a function of day for a year
    procedure plot2 is
        xmin, xmax, ymin, ymax : Long_Float;
        lat, p, d : Long_Float;
        x, y : Real_Vector(0 .. 364);
    begin
        -- Latitude for London
        lat := 51.5;

        xmin := 0.0;
        xmax := Long_Float(x'length) * 60.0 * 60.0 * 24.0;
        ymin := 0.0;
        ymax := 24.0;

        -- Formula for hours of daylight from
        -- "A Model Comparison for Daylength as a Function of Latitude and
        -- Day of the Year", 1995, Ecological Modelling, 80, pp 87-95.
        for j in x'range loop
            x(j):= Long_Float(j) * 60.0 * 60.0 * 24.0;
            p := arcsin(0.39795 * cos(0.2163108 + 2.0 * arctan(0.9671396 * tan(0.00860 * Long_Float(j-186)))));
            d := 24.0 - (24.0 / pi) *
                arccos((sin(0.8333 * pi / 180.0) + sin(lat * pi / 180.0) * sin(p)) /
                (cos(lat * pi / 180.0) * cos(p)));
            y(j):= d;
        end loop;

        plcol0(1);

        -- Set time format to be abbreviated month name followed by day of month
        pltimefmt("%b %d");
        plprec(True, 1);
        plenv(xmin, xmax, ymin, ymax, 0, 40);

        plcol0(3);
        pllab("Date", "Hours of daylight", "@frPLplot Example 29 - Hours of daylight at 51.5N");
        plcol0(4);
        plline(x, y);

        plprec(False, 0);
    end plot2;


    procedure plot3 is
        xmin, xmax, ymin, ymax, tstart : Long_Float;
        x, y : Real_Vector(0 .. 61);
    begin
        -- Calculate continuous time corresponding to 2005-12-01 UTC.
        plctime(2005, 11, 01, 0, 0, 0.0, tstart);

        xmin := tstart;
        xmax := xmin + Long_Float(x'length) * 60.0 * 60.0 * 24.0;
        ymin := 0.0;
        ymax := 5.0;

        for i in x'range loop
            x(i) := xmin + Long_Float(i) * 60.0 * 60.0 * 24.0;
            y(i) := 1.0 + sin(2.0 * pi * Long_Float(i) / 7.0) +
                exp((Long_Float(Integer'min(i, x'length - i))) / 31.0);
        end loop;

        pladv(0);

        plvsta;
        plwind(xmin, xmax, ymin, ymax);

        plcol0(1);

        -- Set time format to be ISO 8601 standard YYYY-MM-DD.
        pltimefmt("%F");

        -- Draw a box with ticks spaced every 14 days in X and 1 hour in Y.
        plbox("bcnstd", 14.0 * 24.0 * 60.0 * 60.0, 14, "bcnstv", 1.0, 4);

        plcol0(3);
        pllab("Date", "Hours of television watched", "@frPLplot Example 29 - Hours of television watched in Dec 2005 / Jan 2006");
        plcol0(4);

        -- Rescale symbol size (used by plpoin) by 0.5
        plssym(0.0,0.5);

        plpoin(x, y, 2);
        plline(x, y);

    end plot3;


    procedure plot4 is
        -- TAI-UTC (seconds) as a function of time.

        Scale : Long_Float;
        xmin, xmax, ymin, ymax, xlabel_step : Long_Float;
        npts : Integer;
        if_TAI_time_format : Boolean;
        time_format : Unbounded_String := To_Unbounded_String("");
        title_suffix : Unbounded_String := To_Unbounded_String("");
        xtitle : Unbounded_String := To_Unbounded_String("");
        title : Unbounded_String := To_Unbounded_String("");
        x, y : Real_Vector(0 .. 1000);
        epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min : Integer;
        epoch_sec : Long_Float;
        tai_year, tai_month, tai_day, tai_hour, tai_min : Integer;
        tai_sec, tai : Long_Float;
        utc_year, utc_month, utc_day, utc_hour, utc_min : Integer;
        utc_sec, utc : Long_Float;
    begin
        -- Continuous time unit is Besselian years from whatever epoch is
        -- chosen below.  Could change to seconds (or days) from the
        -- epoch, but then would have to adjust xlabel_step below.
        scale := 365.242198781;
	-- MJD epoch (see <https://en.wikipedia.org/wiki/Julian_day>).
	-- This is only set for illustrative purposes, and is overwritten
	-- below for the time-representation reasons given in the
	-- discussion below.
	epoch_year  := 1858;
	epoch_month := 11;
	epoch_day   := 17;
	epoch_hour  := 0;
	epoch_min   := 0;
	epoch_sec   := 0.0;
	-- To illustrate the time-representation issues of using the MJD
	-- epoch, in 1985, MJD was roughly 46000 days which corresponds to
	-- 4e9 seconds.  Thus, for the -DPL_DOUBLE=ON case where PLFLT is
	-- a double which can represent continuous time to roughly 16
	-- decimal digits of precision the time-representation error is
	-- roughly ~400 nanoseconds.  Therefore the MJD epoch would be
	-- acceptable for the plots below in the -DPL_DOUBLE=ON case.
	-- However, that epoch is obviously not acceptable for the
	-- -DPL_DOUBLE=OFF case where PLFLT is a float which can represent
	-- continuous time to only ~7 decimal digits of precision
	-- corresponding to a time representation error of 400 seconds (!)
	-- in 1985.  For this reason, we do not use the MJD epoch below
	-- and instead choose the best epoch for each case to minimize
	-- time-representation issues.

        for kind in 0 .. 6 loop
	    if kind = 0 then
	        -- Choose midpoint to maximize time-representation precision.
	        epoch_year  := 1985;
		epoch_month := 0;
		epoch_day   := 2;
		epoch_hour  := 0;
		epoch_min   := 0;
		epoch_sec   := 0.0;
	        plconfigtime(scale, 0.0, 0.0, 0, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec);
                plctime(1950, 0, 2, 0, 0, 0.0, xmin);
                plctime(2020, 0, 2, 0, 0, 0.0, xmax);
                npts := 70 * 12 + 1;
                ymin := 0.0;
                ymax := 36.0;
                time_format := To_Unbounded_String("%Y%");
                if_TAI_time_format := True;
                title_suffix := To_Unbounded_String("from 1950 to 2020");
                xtitle := To_Unbounded_String("Year");
                xlabel_step := 10.0;
            elsif kind = 1 or kind = 2 then
	        -- Choose midpoint to maximize time-representation precision.
	        epoch_year  := 1961;
		epoch_month := 7;
		epoch_day   := 1;
		epoch_hour  := 0;
		epoch_min   := 0;
		epoch_sec   := 1.64757;
	        plconfigtime(scale, 0.0, 0.0, 0, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec);
                plctime(1961, 7, 1, 0, 0, 1.64757 - 0.20, xmin);
                plctime(1961, 7, 1, 0, 0, 1.64757 + 0.20, xmax);
                npts := 1001;
                ymin := 1.625;
                ymax := 1.725;
                time_format := To_Unbounded_String("%S%2%");
                title_suffix := To_Unbounded_String("near 1961-08-01 (TAI)");
                xlabel_step := 0.05 / (scale * 86400.0);
                if kind = 1 then
                    if_TAI_time_format := True;
                    xtitle := To_Unbounded_String("Seconds (TAI)");
                else
                    if_TAI_time_format := False;
                    xtitle := To_Unbounded_String("Seconds (TAI) labelled with corresponding UTC");
                end if;
            elsif kind = 3 or kind = 4 then
	        -- Choose midpoint to maximize time-representation precision.
	        epoch_year  := 1963;
		epoch_month := 10;
		epoch_day   := 1;
		epoch_hour  := 0;
		epoch_min   := 0;
		epoch_sec   := 2.6972788;
	        plconfigtime(scale, 0.0, 0.0, 0, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec);
                plctime(1963, 10, 1, 0, 0, 2.6972788 - 0.20, xmin);
                plctime(1963, 10, 1, 0, 0, 2.6972788 + 0.20, xmax);
                npts := 1001;
                ymin := 2.55;
                ymax := 2.75;
                time_format := To_Unbounded_String("%S%2%");
                title_suffix := To_Unbounded_String("near 1963-11-01 (TAI)");
                xlabel_step := 0.05 / (scale * 86400.0);
                if kind = 3 then
                    if_TAI_time_format := True;
                    xtitle := To_Unbounded_String("Seconds (TAI)");
                else
                    if_TAI_time_format := False;
                    xtitle := To_Unbounded_String("Seconds (TAI) labelled with corresponding UTC");
                end if;
            elsif kind = 5 or kind = 6 then
	        -- Choose midpoint to maximize time-representation precision.
	        epoch_year  := 2009;
		epoch_month := 0;
		epoch_day   := 1;
		epoch_hour  := 0;
		epoch_min   := 0;
		epoch_sec   := 34.0;
	        plconfigtime(scale, 0.0, 0.0, 0, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec);
                plctime(2009, 0, 1, 0, 0, 34.0 - 5.0, xmin);
                plctime(2009, 0, 1, 0, 0, 34.0 + 5.0, xmax);
                npts := 1001;
                ymin := 32.5;
                ymax := 34.5;
                time_format := To_Unbounded_String("%S%2%");
                title_suffix := To_Unbounded_String("near 2009-01-01 (TAI)");
                xlabel_step := 1.0 / (scale * 86400.0);
                if kind = 5 then
                    if_TAI_time_format := True;
                    xtitle := To_Unbounded_String("Seconds (TAI)");
                else
                    if_TAI_time_format := False;
                    xtitle := To_Unbounded_String("Seconds (TAI) labelled with corresponding UTC");
                end if;
            end if;

            for i in 0 .. npts - 1 loop
                x(i) := xmin + Long_Float(i) * (xmax - xmin) / (Long_Float(npts - 1));
                tai := x(i);
	        plconfigtime(scale, 0.0, 0.0, 0, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec);
                plbtime(tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec, tai);
		-- Calculate residual using tai as the epoch to nearly maximize time-representation precision.
	        plconfigtime(scale, 0.0, 0.0, 0, True, tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec);
		-- Calculate continuous tai with new epoch.
                plctime(tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec, tai);
		-- Calculate broken-down utc (with leap seconds inserted) from continuous tai with new epoch.
	        plconfigtime(scale, 0.0, 0.0, 2, True, tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec);
                plbtime(utc_year, utc_month, utc_day, utc_hour, utc_min, utc_sec, tai);
		-- Calculate continuous utc from broken-down utc using same epoch as for the continuous tai.
	        plconfigtime(scale, 0.0, 0.0, 0, True, tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec);
                plctime(utc_year, utc_month, utc_day, utc_hour, utc_min, utc_sec, utc);
		-- Convert residuals to seconds.
                y(i) := (tai - utc) * scale * 86400.0;
            end loop;

            pladv(0);
            plvsta;
            plwind(xmin, xmax, ymin, ymax);
            plcol0(1);
            if if_TAI_time_format then
	        plconfigtime(scale, 0.0, 0.0, 0, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec);
            else
	        plconfigtime(scale, 0.0, 0.0, 2, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec);
            end if;
            pltimefmt(To_String(time_format));
            plbox("bcnstd", xlabel_step, 0, "bcnstv", 0.0, 0);
            plcol0(3);
            title := To_Unbounded_String("@frPLplot Example 29 - TAI-UTC ");
            title := title & title_suffix;
            pllab(To_String(xtitle), "TAI-UTC (sec)", To_String(title));

            plcol0(4);

            if kind = 0 then -- Shorter x and y
                plline(x(0 .. 70 * 12), y(0 .. 70 * 12));
            else -- Longer x and y
                plline(x, y);
            end if;

        end loop; -- kind
    end plot4;

begin -- main
    -- Parse command line arguments
    plparseopts(PL_PARSE_FULL);

    -- Change the escape character to a '@' instead of the default '#'
    plsesc('@');

    -- Initialize plplot
    plinit;

    -- Change the escape character to a '@' instead of the default '#'
    plsesc('@');

    plot1;
    plot2;
    plot3;
    plot4;

    -- Don't forget to call plend to finish off!
    plend;
end xtraditional29a;
