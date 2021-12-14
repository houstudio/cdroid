-- Simple demo of a 2D line plot.
--
-- Copyright (C) 2011  Jerry Bauck
--
-- This file is part of PLplot.
--
-- PLplot is free software; you can redistribute it and/or modify
-- it under the terms of the GNU Library General Public License as published
-- by the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--
-- PLplot is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Library General Public License for more details.
--
-- You should have received a copy of the GNU Library General Public License
-- along with PLplot; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


-- See x00_easy_a.adb for an easier way to generate similar results using
-- Ada-specific extensions to PLplot.

-- See xstandard00a.adb for a version of this program that uses easy-to-read names.

with
    PLplot_Auxiliary,
    PLplot_Traditional;
use
    PLplot_Auxiliary,
    PLplot_Traditional;

procedure xtraditional00a is
    x, y : Real_Vector(0 .. 100);
    x_Min, y_Min : Long_Float := 0.0;
    x_Max : Long_Float := 1.0;
    y_Max : Long_Float := 100.0;
begin
    -- Prepare data to be plotted.
    for i in x'range loop
        x(i) := Long_Float(i) / Long_Float(x'length - 1);
        y(i) := y_Max * x(i)**2;
    end loop;

    -- Parse and process command line arguments.
    plparseopts(PL_PARSE_FULL);

    -- Initialize plplot.
    plinit;

    -- Create a labelled box to hold the plot.
    plenv(x_Min, x_Max, y_Min, y_Max, 0, 0);
    pllab("x", "y=100 x#u2#d", "Simple PLplot demo of a 2D line plot");

    -- Plot the data that was prepared above.
    plline(x, y);

    -- Close PLplot library.
    plend;
end xtraditional00a;
