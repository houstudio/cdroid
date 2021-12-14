-- Simple line plot and multiple windows demo.

-- Copyright (C) 2007 - 2011 Jerry Bauck

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
    Ada.Text_IO,
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Standard;
use
    Ada.Text_IO,
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Standard;

procedure xstandard01a is
    xs, ys : Real_Vector (0 .. 5);
    xscale, yscale, xoff, yoff : Long_Float;
    fontset : Integer := 1;
    notes : String := "Make sure you get it right!";

    procedure plot1 is
        xmin, xmax, ymin, ymax : Long_Float;
        x, y : Real_Vector (0 .. 59);
    begin
        for i in x'Range loop
            x(i) := xoff + xscale * Long_Float(i + 1) / Long_Float(x'Length);
            y(i) := yoff + yscale * x(i)**2.0;
        end loop;

        xmin := x(x'First);
        xmax := x(x'Last);
        ymin := y(y'First);
        ymax := y(y'Last);
        for i in xs'Range loop
            xs(i) := x(i * 10 + 3);
            ys(i) := y(i * 10 + 3);
        end loop;

        -- Set up the viewport and window using Set_Environment. The range in X is
        -- 0.0 to 6.0, and the range in Y is 0.0 to 30.0. The axes are
        -- scaled separately (just = 0), and we just draw a labelled
        -- box (axis = 0).
        Set_Pen_Color(Red);
        Set_Environment(xmin, xmax, ymin, ymax, Not_Justified, Linear_Box_Plus);
        Set_Pen_Color(Yellow);
        Write_Labels("(x)", "(y)", "#frPLplot Example 1 - y=x#u2");

        -- Plot the data points
        Set_Pen_Color(Aquamarine);
        Draw_Points(xs, ys, 9);

        -- Draw the line through the data
        Set_Pen_Color(Green);
        Draw_Curve(x, y);
    end plot1;


    procedure plot2 is
        x, y : Real_Vector (0 .. 99);
    begin
        -- Set up the viewport and window using PLENV. The range in X is -2.0 to
        -- 10.0, and the range in Y is -0.4 to 2.0. The axes are scaled separately
        -- (just = 0), and we draw a box with axes (axis = 1).
        Set_Pen_Color(Red);
        Set_Environment(-2.0, 10.0, -0.4, 1.2, Not_Justified, Linear_Zero_Axes);
        Set_Pen_Color(Yellow);
        Write_Labels("(x)", "sin(x)/x", "#frPLplot Example 1 - Sinc Function");

        -- Fill up the arrays
        for i in x'Range loop
            x(i) := (Long_Float(i) - 19.0) / 6.0;
            y(i) := 1.0;
            if x(i) /= 0.0 then
                y(i) := sin(x(i)) / x(i);
            end if;
        end loop;

        -- Draw the line
        Set_Pen_Color(Green);
        Set_Pen_Width(2.0);
        Draw_Curve(x, y);
        Set_Pen_Width(1.0);
    end plot2;


    procedure plot3 is
        x, y : Real_Vector (0 .. 100);
        space1, mark1 : Integer_Array_1D(1 .. 1) := (others => 1500);
    begin
        Advance_To_Subpage(Next_SubPage);

        -- Use standard viewport, and define X range from 0 to 360 degrees,
        -- Y range from -1.2 to 1.2.
        Set_Viewport_Standard;
        Set_Viewport_World(0.0, 360.0, -1.2, 1.2);

        -- Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y.
        Set_Pen_Color(Red);
        Box_Around_Viewport("bcnst", 60.0, 2, "bcnstv", 0.2, 2);

        -- Superimpose a dashed line grid, with 1.5 mm marks and spaces.
        -- Set_Line_Style expects a pointer! (-- Not Ada.)
        Set_Line_Style(mark1, space1);
        Set_Pen_Color(Yellow);
        Box_Around_Viewport("g", 30.0, 0, "g", 0.2, 0);
        Set_Line_Style(Default_Continuous_Line);

        Set_Pen_Color(Green);
        Write_Labels("Angle (degrees)", "sine", "#frPLplot Example 1 - Sine function");

        for i in x'Range loop
            x(i) := 3.6 * Long_Float(i);
            y(i) := sin(x(i) * pi / 180.0);
        end loop;

        Set_Pen_Color(Aquamarine);
        Draw_Curve(x, y);
    end plot3;

begin
    -- Parse and process command line arguments
    Parse_Command_Line_Arguments(Parse_Full);

    -- Get version number, just for kicks
    Put_Line("PLplot library version: " & Get_Version_Number);

    -- Initialize plplot
    -- Divide page into 2x2 plots unless user overrides
    Initialize_Query_Device(2, 2);

    -- Set up the data
    -- Original case
    xscale := 6.0;
    yscale := 1.0;
    xoff   := 0.0;
    yoff   := 0.0;

    -- Do a plot
    plot1;

    -- Set up the data
    xscale := 1.0;
    yscale := 0.0014;
    yoff   := 0.0185;

    -- Do a plot
    Set_Floating_Point_Display_Y(Max_Digits => 5, Field_Digits => 0);
    plot1;
    plot2;
    plot3;

    -- Don't forget to call End_PLplot to finish off!
    End_PLplot;
end xstandard01a;
