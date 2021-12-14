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

-- See x00a.adb for a version of this program that uses traditional plplot names.

with
    PLplot_Auxiliary,
    PLplot_Standard;
use
    PLplot_Auxiliary,
    PLplot_Standard;

procedure xstandard00a is
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
    Parse_Command_Line_Arguments(Parse_Full);

    -- Initialize plplot.
    Initialize_PLplot;

    -- Create a labelled box to hold the plot.
    Set_Environment(x_Min, x_Max, y_Min, y_Max,
        Justification => Not_Justified,
        Axis_Style    => Linear_Box_Plus);
    Write_Labels
       (X_Label     => "x",
        Y_Label     => "y=100 x#u2#d",
        Title_Label => "Simple PLplot demo of a 2D line plot");

    -- Plot the data that was prepared above.
    Draw_Curve(x, y);

    -- Close PLplot library.
    End_PLplot;
end xstandard00a;
