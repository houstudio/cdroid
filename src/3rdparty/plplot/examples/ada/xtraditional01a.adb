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
    PLplot_Traditional;
use
    Ada.Text_IO,
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Traditional;

procedure xtraditional01a is
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

        -- Set up the viewport and window using plenv. The range in X is
        -- 0.0 to 6.0, and the range in Y is 0.0 to 30.0. The axes are
        -- scaled separately (just = 0), and we just draw a labelled
        -- box (axis = 0).
        plcol0(1);
        plenv(xmin, xmax, ymin, ymax, 0, 0);
        plcol0(2);
        pllab("(x)", "(y)", "#frPLplot Example 1 - y=x#u2");

        -- Plot the data points
        plcol0(4);
        plpoin(xs, ys, 9);

        -- Draw the line through the data
        plcol0(3);
        plline(x, y);
    end plot1;


    procedure plot2 is
        x, y : Real_Vector (0 .. 99);
    begin
        -- Set up the viewport and window using PLENV. The range in X is -2.0 to
        -- 10.0, and the range in Y is -0.4 to 2.0. The axes are scaled separately
        -- (just = 0), and we draw a box with axes (axis = 1).
        plcol0(1);
        plenv(-2.0, 10.0, -0.4, 1.2, 0, 1);
        plcol0(2);
        pllab("(x)", "sin(x)/x", "#frPLplot Example 1 - Sinc Function");

        -- Fill up the arrays
        for i in x'Range loop
            x(i) := (Long_Float(i) - 19.0) / 6.0;
            y(i) := 1.0;
            if x(i) /= 0.0 then
                y(i) := sin(x(i)) / x(i);
            end if;
        end loop;

        -- Draw the line
        plcol0(3);
        plwidth(2.0);
        plline(x, y);
        plwidth(1.0);
    end plot2;


    procedure plot3 is
        x, y : Real_Vector (0 .. 100);
        space1, mark1 : Integer_Array_1D(1 .. 1) := (others => 1500);
    begin
        pladv(0);

        -- Use standard viewport, and define X range from 0 to 360 degrees,
        -- Y range from -1.2 to 1.2.
        plvsta;
        plwind(0.0, 360.0, -1.2, 1.2);

        -- Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y.
        plcol0(1);
        plbox("bcnst", 60.0, 2, "bcnstv", 0.2, 2);

        -- Superimpose a dashed line grid, with 1.5 mm marks and spaces.
        -- plstyl expects a pointer! (-- Not Ada.)
        plstyl(mark1, space1);
        plcol0(2);
        plbox("g", 30.0, 0, "g", 0.2, 0);
        plstyl(Default_Continuous_Line);

        plcol0(3);
        pllab("Angle (degrees)", "sine", "#frPLplot Example 1 - Sine function");

        for i in x'Range loop
            x(i) := 3.6 * Long_Float(i);
            y(i) := sin(x(i) * pi / 180.0);
        end loop;

        plcol0(4);
        plline(x, y);
    end plot3;

begin
    -- Parse and process command line arguments
    plparseopts(PL_PARSE_FULL);

    -- Get version number, just for kicks
    Put_Line("PLplot library version: " & plgver);

    -- Initialize plplot
    -- Divide page into 2x2 plots unless user overrides
    plstar(2, 2);

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
    plsyax(5, 0);
    plot1;
    plot2;
    plot3;

    -- Don't forget to call PLEND to finish off!
    plend;
end xtraditional01a;
