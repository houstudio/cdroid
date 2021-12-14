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
    PLplot_Standard;
use
    Ada.Strings.Unbounded,
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Standard;

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

procedure xstandard29a is
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

        Advance_To_Subpage(Next_Subpage);

        -- Rescale major ticks marks by 0.5
        Set_Major_Tick_Length(0.0, 0.5);

        -- Rescale minor ticks and error bar marks by 0.5
        Set_Minor_Tick_Length(0.0, 0.5);

        Set_Viewport_Standard;
        Set_Viewport_World(xmin, xmax, ymin, ymax);

        -- Draw a box with ticks spaced every 3 hours in X and 1 degree C in Y.
        Set_Pen_Color(Red);

        -- Set time format to be hours:minutes
        Set_Date_Time_Label_Format("%H:%M");
        Box_Around_Viewport("bcnstd", 3.0 * 60.0 * 60.0, 3, "bcnstv", 1.0, 5);

        Set_Pen_Color(Green);
        Write_Labels("Time (hours:mins)", "Temperature (degC)", "@frPLplot Example 29 - Daily temperature");
        Set_Pen_Color(Aquamarine);
        Draw_Curve(x, y);

        Set_Pen_Color(Yellow);
        Draw_Error_Bars_X(xerr1, xerr2, y);
        Set_Pen_Color(Green);
        Draw_Error_Bars_Y(x, yerr1, yerr2);

        -- Rescale major / minor tick marks back to default
        Set_Minor_Tick_Length(0.0, 1.0);
        Set_Major_Tick_Length(0.0, 1.0);
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

        Set_Pen_Color(Red);

        -- Set time format to be abbreviated month name followed by day of month
        Set_Date_Time_Label_Format("%b %d");
        Set_Numeric_Label_Precision(True, 1);
        Set_Environment(xmin, xmax, ymin, ymax, 0, 40);

        Set_Pen_Color(Green);
        Write_Labels("Date", "Hours of daylight", "@frPLplot Example 29 - Hours of daylight at 51.5N");
        Set_Pen_Color(Aquamarine);
        Draw_Curve(x, y);

        Set_Numeric_Label_Precision(False, 0);
    end plot2;


    procedure plot3 is
        xmin, xmax, ymin, ymax, tstart : Long_Float;
        x, y : Real_Vector(0 .. 61);
    begin
        -- Calculate continuous time corresponding to 2005-12-01 UTC.
        Continuous_From_Broken_Down_Time(2005, 11, 01, 0, 0, 0.0, tstart);

        xmin := tstart;
        xmax := xmin + Long_Float(x'length) * 60.0 * 60.0 * 24.0;
        ymin := 0.0;
        ymax := 5.0;

        for i in x'range loop
            x(i) := xmin + Long_Float(i) * 60.0 * 60.0 * 24.0;
            y(i) := 1.0 + sin(2.0 * pi * Long_Float(i) / 7.0) +
                exp((Long_Float(Integer'min(i, x'length - i))) / 31.0);
        end loop;

        Advance_To_Subpage(Next_Subpage);

        Set_Viewport_Standard;
        Set_Viewport_World(xmin, xmax, ymin, ymax);

        Set_Pen_Color(Red);

        -- Set time format to be ISO 8601 standard YYYY-MM-DD.
        Set_Date_Time_Label_Format("%F");

        -- Draw a box with ticks spaced every 14 days in X and 1 hour in Y.
        Box_Around_Viewport("bcnstd", 14.0 * 24.0 * 60.0 * 60.0, 14, "bcnstv", 1.0, 4);

        Set_Pen_Color(Green);
        Write_Labels("Date", "Hours of television watched", "@frPLplot Example 29 - Hours of television watched in Dec 2005 / Jan 2006");
        Set_Pen_Color(Aquamarine);

        -- Rescale symbol size (used by Draw_Points) by 0.5
        Set_Symbol_Size(0.0,0.5);

        Draw_Points(x, y, 2);
        Draw_Curve(x, y);

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
	        Configure_Time_Transformation(scale, 0.0, 0.0, 0, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec);
                Continuous_From_Broken_Down_Time(1950, 0, 2, 0, 0, 0.0, xmin);
                Continuous_From_Broken_Down_Time(2020, 0, 2, 0, 0, 0.0, xmax);
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
	        Configure_Time_Transformation(scale, 0.0, 0.0, 0, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec);
                Continuous_From_Broken_Down_Time(1961, 7, 1, 0, 0, 1.64757 - 0.20, xmin);
                Continuous_From_Broken_Down_Time(1961, 7, 1, 0, 0, 1.64757 + 0.20, xmax);
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
	        Configure_Time_Transformation(scale, 0.0, 0.0, 0, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec);
                Continuous_From_Broken_Down_Time(1963, 10, 1, 0, 0, 2.6972788 - 0.20, xmin);
                Continuous_From_Broken_Down_Time(1963, 10, 1, 0, 0, 2.6972788 + 0.20, xmax);
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
	        Configure_Time_Transformation(scale, 0.0, 0.0, 0, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec);
                Continuous_From_Broken_Down_Time(2009, 0, 1, 0, 0, 34.0 - 5.0, xmin);
                Continuous_From_Broken_Down_Time(2009, 0, 1, 0, 0, 34.0 + 5.0, xmax);
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
	        Configure_Time_Transformation(scale, 0.0, 0.0, 0, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec);
                Broken_Down_From_Continuous_Time(tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec, tai);
		-- Calculate residual using tai as the epoch to nearly maximize time-representation precision.
	        Configure_Time_Transformation(scale, 0.0, 0.0, 0, True, tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec) ;
		-- Calculate continuous tai with new epoch.
                Continuous_From_Broken_Down_Time(tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec, tai);
		-- Calculate broken-down utc (with leap seconds inserted) from continuous tai with new epoch.
	        Configure_Time_Transformation(scale, 0.0, 0.0, 2, True, tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec) ;
                Broken_Down_From_Continuous_Time(utc_year, utc_month, utc_day, utc_hour, utc_min, utc_sec, tai);
		-- Calculate continuous utc from broken-down utc using same epoch as for the continuous tai.
	        Configure_Time_Transformation(scale, 0.0, 0.0, 2, True, tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec) ;
                Continuous_From_Broken_Down_Time(utc_year, utc_month, utc_day, utc_hour, utc_min, utc_sec, utc);
		-- Convert residuals to seconds.
                y(i) := (tai - utc) * scale * 86400.0;
            end loop;

            Advance_To_Subpage(Next_Subpage);
            Set_Viewport_Standard;
            Set_Viewport_World(xmin, xmax, ymin, ymax);
            Set_Pen_Color(Red);
            if if_TAI_time_format then
	        Configure_Time_Transformation(scale, 0.0, 0.0, 0, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec);
            else
	        Configure_Time_Transformation(scale, 0.0, 0.0, 2, True, epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec);
            end if;
            Set_Date_Time_Label_Format(To_String(time_format));
            Box_Around_Viewport("bcnstd", xlabel_step, 0, "bcnstv", 0.0, 0);
            Set_Pen_Color(Green);
            title := To_Unbounded_String("@frPLplot Example 29 - TAI-UTC ");
            title := title & title_suffix;
            Write_Labels(To_String(xtitle), "TAI-UTC (sec)", To_String(title));

            Set_Pen_Color(Aquamarine);

            if kind = 0 then -- Shorter x and y
                Draw_Curve(x(0 .. 70 * 12), y(0 .. 70 * 12));
            else -- Longer x and y
                Draw_Curve(x, y);
            end if;

        end loop; -- kind
    end plot4;

begin -- main
    -- Parse command line arguments
    Parse_Command_Line_Arguments(Parse_Full);

    -- Change the escape character to a '@' instead of the default '#'
    Set_Escape_Character('@');

    -- Initialize plplot
    Initialize_PLplot;

    plot1;
    plot2;
    plot3;
    plot4;

    -- Don't forget to call End_PLplot to finish off!
    End_PLplot;
end xstandard29a;
