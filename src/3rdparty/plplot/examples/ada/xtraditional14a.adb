-- Demo of multiple stream/window capability (requires Tk or Tcl-DP).

--      Maurice LeBrun
--      IFS, University of Texas at Austin

-- Copyright (C) 2008 Jerry Bauck

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
    System,
    Ada.Text_IO,
    Ada.Strings,
    Ada.Strings.Fixed,
    Ada.Strings.Unbounded,
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Traditional;
use
    Ada.Text_IO,
    Ada.Numerics,
    Ada.Strings,
    Ada.Strings.Fixed,
    Ada.Strings.Unbounded,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Traditional;

------------------------------------------------------------------------------
-- Plots several simple functions from other example programs.
--
-- This version sends the output of the first 4 plots (one page) to two
-- independent streams.
------------------------------------------------------------------------------

procedure xtraditional14a is
    -- Select either TK or DP driver and use a small window
    -- Using DP results in a crash at the end due to some odd cleanup problems
    -- The geometry strings MUST be in writable memory
    geometry_master : String := "500x410+100+200";
    geometry_slave  : String := "500x410+650+200";
    driver : Unbounded_String;
    digmax : Integer;
    xscale, yscale, xoff, yoff : Long_Float;
    xs, ys : Real_Vector(0 .. 5);
    space0 : Integer_Array_1D(1 .. 1) := (Others => 0);
    mark0  : Integer_Array_1D(1 .. 1) := (Others => 0);
    space1 : Integer_Array_1D(1 .. 1) := (Others => 1500);
    mark1  : Integer_Array_1D(1 .. 1) := (Others => 1500);
    fam : Boolean;
    num, bmax : Integer;
    xp0, yp0 : Long_Float;
    xleng0, yleng0, xoff0, yoff0 : Integer;
    valid_geometry : Boolean;

    procedure plot1 is
        xmin, xmax, ymin, ymax : Long_Float;
        x, y : Real_Vector(0 .. 59);
    begin
        for i in x'range loop
            x(i) := xoff + xscale * Long_Float(i + 1) / Long_Float(x'Length);
            y(i) := yoff + yscale * x(i) * x(i);
        end loop;

        xmin := x(x'First);
        xmax := x(x'Last);
        ymin := y(y'First);
        ymax := y(y'Last);

        for i in xs'range loop
            xs(i) := x(i * 10 + 3);
            ys(i) := y(i * 10 + 3);
        end loop;

        -- Set up the viewport and window using PLENV. The range in X is
        -- 0.0 to 6.0, and the range in Y is 0.0 to 30.0. The axes are
        -- scaled separately (just := 0), and we just draw a labelled
        -- box (axis := 0).
        plcol0(1);
        plenv(xmin, xmax, ymin, ymax, 0, 0);
        plcol0(6);
        pllab("(x)", "(y)", "#frPLplot Example 1 - y=x#u2");

        -- Plot the data points
        plcol0(9);
        plpoin(xs, ys, 9);

        -- Draw the line through the data
        plcol0(4);
        plline(x, y);
        plflush;
    end plot1;

    -- ================================================================


    procedure plot2 is
        x, y : Real_Vector(0 .. 99);
    begin
        -- Set up the viewport and window using PLENV. The range in X is -2.0 to
        -- 10.0, and the range in Y is -0.4 to 2.0. The axes are scaled separately
        -- (just = 0), and we draw a box with axes (axis = 1).
        plcol0(1);
        plenv(-2.0, 10.0, -0.4, 1.2, 0, 1);
        plcol0(2);
        pllab("(x)", "sin(x)/x", "#frPLplot Example 1 - Sinc Function");

        -- Fill up the arrays
        for i in x'range loop
            x(i) := (Long_Float(i) - 19.0) / 6.0;
            y(i) := 1.0;
            if x(i) /= 0.0 then
                y(i) := sin(x(i)) / x(i);
            end if;
        end loop;

        -- Draw the line
        plcol0(3);
        plline(x, y);
        plflush;
    end plot2;

    -- ================================================================


    procedure plot3 is
        x, y : Real_Vector(0 .. 100);
    begin
        -- For the final graph we wish to override the default tick intervals, and
        -- so do not use PLENV

        pladv(0);

        -- Use standard viewport, and define X range from 0 to 360 degrees, Y range
        -- from -1.2 to 1.2.
        plvsta;
        plwind(0.0, 360.0, -1.2, 1.2);

        -- Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y.
        plcol0(1);
        plbox("bcnst", 60.0, 2, "bcnstv", 0.2, 2);

        -- Superimpose a dashed line grid, with 1.5 mm marks and spaces.
        plstyl(mark1, space1);
        plcol0(2);
        plbox("g", 30.0, 0, "g", 0.2, 0);
        plstyl(Default_Continuous_Line);

        plcol0(3);
        pllab("Angle (degrees)", "sine", "#frPLplot Example 1 - Sine function");

        for i in x'range loop
            x(i) := 3.6 * Long_Float(i);
            y(i) := sin(x(i) * pi / 180.0);
        end loop;

        plcol0(4);
        plline(x, y);
        plflush;
        end plot3;

    -- ================================================================


    procedure plot4 is
        dtr, theta, dx, dy, r : Long_Float;
        x, y, x0, y0 : Real_Vector(0 .. 360);
    begin
        dtr := pi / 180.0;
        for i in x0'range loop
            x0(i) := cos(dtr * Long_Float(i));
            y0(i) := sin(dtr * Long_Float(i));
        end loop;

        -- Set up viewport and window, but do not draw box
        plenv(-1.3, 1.3, -1.3, 1.3, 1, -2);
        for i in 1 .. 10 loop
            for j in x'range loop
                x(j) := 0.1 * Long_Float(i) * x0(j);
                y(j) := 0.1 * Long_Float(i) * y0(j);
            end loop;

            -- Draw circles for polar grid
            plline(x, y);
        end loop;

        plcol0(2);
        for i in 0 .. 11 loop
            theta := 30.0 * Long_Float(i);
            dx := cos(dtr * theta);
            dy := sin(dtr * theta);

            -- Draw radial spokes for polar grid
            pljoin(0.0, 0.0, dx, dy);

            -- Write labels for angle
            -- Slightly off zero to avoid floating point logic flips at 90 and 270 deg.
            if dx >= -0.00001 then
                plptex(dx, dy, dx, dy, -0.15, Trim(Integer'image(Integer(theta)), Left));
            else
                plptex(dx, dy, -dx, -dy, 1.15, Trim(Integer'image(Integer(theta)), Left));
            end if;
        end loop;

        -- Draw the graph
        for i in x'range loop
            r := sin(dtr * Long_Float(5 * i));
            x(i) := x0(i) * r;
            y(i) := y0(i) * r;
        end loop;
        plcol0(3);
        plline(x, y);

        plcol0(4);
        plmtex("t", 2.0, 0.5, 0.5, "#frPLplot Example 3 - r(#gh)=sin 5#gh");
        plflush;
    end plot4;

    -- ================================================================


    -- Demonstration of contour plotting
    procedure plot5 is
        XPTS : constant Integer := 35;
        YPTS : constant Integer := 46;
        -- Transformation function
        XSPA : Long_Float := 2.0 / Long_Float(XPTS - 1);
        YSPA : Long_Float := 2.0 / Long_Float(YPTS - 1);
        tr : Real_Vector(0 .. 5) := (XSPA, 0.0, -1.0, 0.0, YSPA, -1.0);
        xx, yy : Long_Float;
        mark  : Integer_Array_1D(1 .. 1) := (Others => 1500);
        space : Integer_Array_1D(1 .. 1) := (Others => 1500);
        z, w : Real_Matrix(0 .. XPTS -1, 0 .. YPTS - 1);
        clevel : Real_Vector(0 .. 10) :=
            (-1.0, -0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, 0.8, 1.0);

        procedure mypltr -- This spec is necessary to accommodate pragma Convention(C...).
           (x, y : Long_Float;
            tx, ty : out Long_Float;
            pltr_data : PL_Pointer);
        pragma Convention(Convention => C, Entity => mypltr);

        procedure mypltr
           (x, y : Long_Float;
            tx, ty : out Long_Float;
            pltr_data : PL_Pointer)
        is
        begin
            tx := tr(0) * x + tr(1) * y + tr(2);
            ty := tr(3) * x + tr(4) * y + tr(5);
        end mypltr;

    begin
        for i in z'range(1) loop
            xx := Long_Float(i - (XPTS / 2)) / Long_Float(XPTS / 2);
            for j in z'range(2) loop
                yy := Long_Float(j - (YPTS / 2)) / Long_Float(YPTS / 2) - 1.0;
                z(i, j) := xx * xx - yy * yy;
                w(i, j) := 2.0 * xx * yy;
            end loop;
        end loop;

        plenv(-1.0, 1.0, -1.0, 1.0, 0, 0);
        plcol0(2);
        plcont(z, 1, XPTS, 1, YPTS, clevel, mypltr'Unrestricted_Access, System.Null_Address);
        plstyl(mark, space);
        plcol0(3);
        plcont(w, 1, XPTS, 1, YPTS, clevel, mypltr'Unrestricted_Access, System.Null_Address);
        plcol0(1);
        pllab("X Coordinate", "Y Coordinate", "Streamlines of flow");
        plflush;
    end plot5;


begin
    -- plplot initialization
    -- Parse and process command line arguments
    plparseopts(PL_PARSE_FULL);

    driver := To_Unbounded_String(plgdev);
    plgfam(fam, num, bmax);

    Put_Line("Demo of multiple output streams via the " & plgdev & " driver.");
    Put_Line("Running with the second stream as slave to the first.");
    New_Line;

    -- If valid geometry specified on command line, use it for both streams.
    plgpage(xp0, yp0, xleng0, yleng0, xoff0, yoff0);
    valid_geometry := (xleng0 > 0 and yleng0 > 0);

    -- Set up first stream
    if valid_geometry then
       plspage(xp0, yp0, xleng0, yleng0, xoff0, yoff0);
    else
       plsetopt("geometry", geometry_master);
    end if;

    plsdev(To_String(driver));
    plssub(2, 2);
    plinit;

    -- Start next stream
    plsstrm(1);

    if valid_geometry then
       plspage(xp0, yp0, xleng0, yleng0, xoff0, yoff0);
    else
       plsetopt("geometry", geometry_slave);
    end if;

    -- Turn off pause to make this a slave (must follow master)
    plspause(False);
    plsfam(fam, num, bmax);
    plsetopt("fflen","2");
    plsdev(To_String(driver));
    plinit;

    -- Set up the data & plot
    -- Original case
    plsstrm(0);

    xscale := 6.0;
    yscale := 1.0;
    xoff := 0.0;
    yoff := 0.0;
    plot1;

    -- Set up the data & plot
    xscale := 1.0;
    yscale := 1.0e+6;
    plot1;

    -- Set up the data & plot
    xscale := 1.0;
    yscale := 1.0e-6;
    digmax := 2;
    plsyax(digmax, 0);
    plot1;

    -- Set up the data & plot
    xscale := 1.0;
    yscale := 0.0014;
    yoff := 0.0185;
    digmax := 5;
    plsyax(digmax, 0);
    plot1;

    -- To slave
    -- The pleop ensures the eop indicator gets lit.
    plsstrm(1);
    plot4;
    pleop;

    -- Back to master
    plsstrm(0);
    plot2;
    plot3;

    -- To slave
    plsstrm(1);
    plot5;
    pleop;

    -- Back to master to wait for user to advance
    plsstrm(0);
    pleop;

    -- Call plend to finish off.
    plend;
end xtraditional14a;
