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


-- This program uses Ada-specific extensions to PLplot to create similar
-- results as xtraditional00a.adb and xstandard00a.adb but with less effort.

with
    PLplot_Auxiliary,
    PLplot_Standard;
use
    PLplot_Auxiliary,
    PLplot_Standard;

procedure x00_easy_a is
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

    -- -- Initialize plplot.
     Initialize_PLplot;

    -- Make the plot.
    Simple_Plot(x, y,
        X_Label => "x",
        Y_Label => "y=100 x#u2#d",
        Title_Label => "Simple PLplot demo of a 2D line plot");

    -- Close PLplot library.
    End_PLplot;
end x00_easy_a;
