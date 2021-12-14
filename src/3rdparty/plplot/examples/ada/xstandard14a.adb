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
    PLplot_Standard;
use
    Ada.Text_IO,
    Ada.Numerics,
    Ada.Strings,
    Ada.Strings.Fixed,
    Ada.Strings.Unbounded,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Standard;

------------------------------------------------------------------------------
-- Plots several simple functions from other example programs.
--
-- This version sends the output of the first 4 plots (one page) to two
-- independent streams.
------------------------------------------------------------------------------

procedure xstandard14a is
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
        Set_Pen_Color(Red);
        Set_Environment(xmin, xmax, ymin, ymax, Not_Justified, Linear_Box_Plus);
        Set_Pen_Color(Wheat);
        Write_Labels("(x)", "(y)", "#frPLplot Example 1 - y=x#u2");

        -- Plot the data points
        Set_Pen_Color(Blue);
        Draw_Points(xs, ys, 9);

        -- Draw the line through the data
        Set_Pen_Color(Aquamarine);
        Draw_Curve(x, y);
        Flush_Output_Stream;
    end plot1;

    -- ================================================================


    procedure plot2 is
        x, y : Real_Vector(0 .. 99);
    begin
        -- Set up the viewport and window using PLENV. The range in X is -2.0 to
        -- 10.0, and the range in Y is -0.4 to 2.0. The axes are scaled separately
        -- (just = 0), and we draw a box with axes (axis = 1).
        Set_Pen_Color(Red);
        Set_Environment(-2.0, 10.0, -0.4, 1.2, Not_Justified, Linear_Zero_Axes);
        Set_Pen_Color(Yellow);
        Write_Labels("(x)", "sin(x)/x", "#frPLplot Example 1 - Sinc Function");

        -- Fill up the arrays
        for i in x'range loop
            x(i) := (Long_Float(i) - 19.0) / 6.0;
            y(i) := 1.0;
            if x(i) /= 0.0 then
                y(i) := sin(x(i)) / x(i);
            end if;
        end loop;

        -- Draw the line
        Set_Pen_Color(Green);
        Draw_Curve(x, y);
        Flush_Output_Stream;
    end plot2;

    -- ================================================================


    procedure plot3 is
        x, y : Real_Vector(0 .. 100);
    begin
        -- For the final graph we wish to override the default tick intervals, and
        -- so do not use PLENV

        Advance_To_Subpage(Next_Subpage);

        -- Use standard viewport, and define X range from 0 to 360 degrees, Y range
        -- from -1.2 to 1.2.
        Set_Viewport_Standard;
        Set_Viewport_World(0.0, 360.0, -1.2, 1.2);

        -- Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y.
        Set_Pen_Color(Red);
        Box_Around_Viewport("bcnst", 60.0, 2, "bcnstv", 0.2, 2);

        -- Superimpose a dashed line grid, with 1.5 mm marks and spaces.
        Set_Line_Style(mark1, space1);
        Set_Pen_Color(Yellow);
        Box_Around_Viewport("g", 30.0, 0, "g", 0.2, 0);
        Set_Line_Style(Default_Continuous_Line);

        Set_Pen_Color(Green);
        Write_Labels("Angle (degrees)", "sine", "#frPLplot Example 1 - Sine function");

        for i in x'range loop
            x(i) := 3.6 * Long_Float(i);
            y(i) := sin(x(i) * pi / 180.0);
        end loop;

        Set_Pen_Color(Aquamarine);
        Draw_Curve(x, y);
        Flush_Output_Stream;
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
        Set_Environment(-1.3, 1.3, -1.3, 1.3, Justified, No_Box);
        for i in 1 .. 10 loop
            for j in x'range loop
                x(j) := 0.1 * Long_Float(i) * x0(j);
                y(j) := 0.1 * Long_Float(i) * y0(j);
            end loop;

            -- Draw circles for polar grid
            Draw_Curve(x, y);
        end loop;

        Set_Pen_Color(Yellow);
        for i in 0 .. 11 loop
            theta := 30.0 * Long_Float(i);
            dx := cos(dtr * theta);
            dy := sin(dtr * theta);

            -- Draw radial spokes for polar grid
            Draw_Line(0.0, 0.0, dx, dy);

            -- Write labels for angle
            -- Slightly off zero to avoid floating point logic flips at 90 and 270 deg.
            if dx >= -0.00001 then
                Write_Text_World(dx, dy, dx, dy, -0.15, Trim(Integer'image(Integer(theta)), Left));
            else
                Write_Text_World(dx, dy, -dx, -dy, 1.15, Trim(Integer'image(Integer(theta)), Left));
            end if;
        end loop;

        -- Draw the graph
        for i in x'range loop
            r := sin(dtr * Long_Float(5 * i));
            x(i) := x0(i) * r;
            y(i) := y0(i) * r;
        end loop;
        Set_Pen_Color(Green);
        Draw_Curve(x, y);

        Set_Pen_Color(Aquamarine);
        Write_Text_Viewport("t", 2.0, 0.5, 0.5, "#frPLplot Example 3 - r(#gh)=sin 5#gh");
        Flush_Output_Stream;
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

        Set_Environment(-1.0, 1.0, -1.0, 1.0, Not_Justified, Linear_Box_Plus);
        Set_Pen_Color(Yellow);
        Contour_Plot(z, 1, XPTS, 1, YPTS, clevel, mypltr'Unrestricted_Access, System.Null_Address);
        Set_Line_Style(mark, space);
        Set_Pen_Color(Green);
        Contour_Plot(w, 1, XPTS, 1, YPTS, clevel, mypltr'Unrestricted_Access, System.Null_Address);
        Set_Pen_Color(Red);
        Write_Labels("X Coordinate", "Y Coordinate", "Streamlines of flow");
        Flush_Output_Stream;
    end plot5;


begin
    -- plplot initialization
    -- Parse and process command line arguments
    Parse_Command_Line_Arguments(Parse_Full);

    driver := To_Unbounded_String(Get_Device_Name);
    Get_File_Family_Parameters(fam, num, bmax);

    Put_Line("Demo of multiple output streams via the " & Get_Device_Name & " driver.");
    Put_Line("Running with the second stream as slave to the first.");
    New_Line;

    -- If valid geometry specified on command line, use it for both streams.
    Get_Page_Parameters(xp0, yp0, xleng0, yleng0, xoff0, yoff0);
    valid_geometry := (xleng0 > 0 and yleng0 > 0);

    -- Set up first stream
    if valid_geometry then
      Set_Page_Parameters(xp0, yp0, xleng0, yleng0, xoff0, yoff0);
    else
      Set_Command_Line_Option("geometry", geometry_master);
    end if;

    Set_Device_Name(To_String(driver));
    Set_Number_Of_Subpages(2, 2);
    Initialize_PLplot;

    -- Start next stream
    Set_Stream_Number(1);

    if valid_geometry then
      Set_Page_Parameters(xp0, yp0, xleng0, yleng0, xoff0, yoff0);
    else
      Set_Command_Line_Option("geometry", geometry_slave);
    end if;

    -- Turn off pause to make this a slave (must follow master)
    Set_Pause(False);
    Set_Device_Name(To_String(driver));
    Set_File_Family_Parameters(fam, num, bmax);
    Set_Command_Line_Option("fflen","2");
    Initialize_PLplot;

    -- Set up the data & plot
    -- Original case
    Set_Stream_Number(0);

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
    Set_Floating_Point_Display_Y(digmax, 0);
    plot1;

    -- Set up the data & plot
    xscale := 1.0;
    yscale := 0.0014;
    yoff := 0.0185;
    digmax := 5;
    Set_Floating_Point_Display_Y(digmax, 0);
    plot1;

    -- To slave
    -- The Eject_Current_Page ensures the eop indicator gets lit.
    Set_Stream_Number(1);
    plot4;
    Eject_Current_Page;

    -- Back to master
    Set_Stream_Number(0);
    plot2;
    plot3;

    -- To slave
    Set_Stream_Number(1);
    plot5;
    Eject_Current_Page;

    -- Back to master to wait for user to advance
    Set_Stream_Number(0);
    Eject_Current_Page;

    -- Call End_PLplot to finish off.
    End_PLplot;
end xstandard14a;
