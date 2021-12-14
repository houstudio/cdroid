--  Shade plot demo.

--  Maurice LeBrun
--  IFS, University of Texas at Austin
--  31 Aug 1993

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
    Ada.Numerics,
    System,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Standard;
use
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Standard;

procedure xstandard15a is
    XPTS : constant Integer := 35; -- Data points in x
    YPTS : constant Integer := 46; -- Data points in y
    zmin, zmax, xx, yy : Long_Float;
    z : Real_Matrix(0 .. XPTS - 1, 0 .. YPTS - 1);

    ----------------------------------------------------------------------------
    -- cmap1_init2
    -- Initializes color map 1 in HLS space.
    ----------------------------------------------------------------------------
    procedure cmap1_init2 is
        i, h, l, s : Real_Vector(0 .. 3);
    begin
        -- Set control points.
        i(0) := 0.0;		-- left boundary
        i(1) := 0.45;       -- just before center
        i(2) := 0.55;       -- just after center
        i(3) := 1.0;		-- right boundary

        -- Set hue levels.
        h(0) := 260.0;		-- low: blue-violet
        h(1) := 260.0;		-- only change as we go over vertex
        h(2) := 20.0;		-- hue -- high: red
        h(3) := 20.0;		-- keep fixed

        -- Set lightness levels.
        l(0) := 0.6;		-- low
        l(1) := 0.0;		-- center
        l(2) := 0.0;		-- center
        l(3) := 0.6;		-- high

        -- Set saturation levels.
        s(0) := 1.0;		-- low
        s(1) := 0.5;		-- center
        s(2) := 0.5;		-- center
        s(3) := 1.0;		-- high

        Set_Color_Map_1_Piecewise(HLS, i, h, l, s, Alt_Hue_Path_None);
    end cmap1_init2;

    ----------------------------------------------------------------------------
    -- plot1
    -- Illustrates a single shaded region.
    ----------------------------------------------------------------------------
    procedure plot1 is
        shade_min, shade_max, sh_color : Long_Float;
        sh_cmap  : Integer := 0;
        sh_width : Long_Float;
        min_color, max_color : Integer;
        min_width, max_width : Long_Float;
    begin
        Advance_To_Subpage(Next_Subpage);
        Set_Viewport_Normalized(0.1, 0.9, 0.1, 0.9);
        Set_Viewport_World(-1.0, 1.0, -1.0, 1.0);

        -- Plot using identity transform --
        shade_min := zmin + (zmax-zmin) * 0.4;
        shade_max := zmin + (zmax-zmin) * 0.6;
        sh_color := 7.0;
        sh_width := 2.0;
        min_color := 9;
        max_color := 2;
        min_width := 2.0;
        max_width := 2.0;

        Select_Fill_Pattern(8);

        Shade_Region(z, Null, -1.0, 1.0, -1.0, 1.0,
             shade_min, shade_max,
             sh_cmap, sh_color, sh_width,
             min_color, min_width, max_color, max_width,
             Fill_Polygon'access, True, Null, System.Null_Address);

        Set_Pen_Color(Red);
        Box_Around_Viewport("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
        Set_Pen_Color(Yellow);
        Write_Labels("distance", "altitude", "Bogon flux");
    end plot1;

    ----------------------------------------------------------------------------
    -- plot2
    -- Illustrates multiple adjacent shaded regions, using different fill
    -- patterns for each region.
    ----------------------------------------------------------------------------
    procedure plot2 is
        shade_min, shade_max, sh_color : Long_Float;
        sh_cmap  : Integer := 0;
        sh_width : Long_Float;
        min_color, max_color : Integer := 0;
        min_width, max_width : Long_Float := 0.0;

        -- C run amok. The following arrays are made to accommodate the
        -- original example which is written in perverse C.
        inc_0_To_4 : array (0 .. 4) of Integer_Array_1D(0 .. 0);
        inc_5_To_9 : array (5 .. 9) of Integer_Array_1D(0 .. 1) :=
           ((450, -450), (0, 900), (0, 450), (450, -450), (0, 900));

        del_0_To_4 : array (0 .. 4) of Integer_Array_1D(0 .. 0);
        del_5_To_9 : array (5 .. 9) of Integer_Array_1D(0 .. 1) :=
           ((2000, 2000), (2000, 2000), (2000, 2000), (4000, 4000), (4000, 2000));
    begin
        -- Initialize the rest of the amokified arrays.
        inc_0_To_4(0)(0) :=  450;
        inc_0_To_4(1)(0) := -450;
        inc_0_To_4(2)(0) :=    0;
        inc_0_To_4(3)(0) :=  900;
        inc_0_To_4(4)(0) :=  300;

        del_0_To_4(0)(0) := 2000;
        del_0_To_4(1)(0) := 2000;
        del_0_To_4(2)(0) := 2000;
        del_0_To_4(3)(0) := 2000;
        del_0_To_4(4)(0) := 2000;

        sh_width := 2.0;

        Advance_To_Subpage(Next_Subpage);
        Set_Viewport_Normalized(0.1, 0.9, 0.1, 0.9);
        Set_Viewport_World(-1.0, 1.0, -1.0, 1.0);

        -- Plot using identity transform
        for i in 0 .. 9 loop
            shade_min := zmin + (zmax - zmin) * Long_Float(i) / 10.0;
            shade_max := zmin + (zmax - zmin) * Long_Float(i +1) / 10.0;
            sh_color := Long_Float(i + 6);
            if i in 0 .. 4 then
                Set_Fill_Pattern(inc_0_To_4(i), del_0_To_4(i));
            else
                Set_Fill_Pattern(inc_5_To_9(i), del_5_To_9(i));
            end if;
            Shade_Region(z, Null, -1.0, 1.0, -1.0, 1.0,
                 shade_min, shade_max,
                 sh_cmap, sh_color, sh_width,
                 min_color, min_width, max_color, max_width,
                 Fill_Polygon'access, True, Null, System.Null_Address);
        end loop;

        Set_Pen_Color(Red);
        Box_Around_Viewport("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
        Set_Pen_Color(Yellow);
        Write_Labels("distance", "altitude", "Bogon flux");
    end plot2;

    ----------------------------------------------------------------------------
    -- plot3
    -- Illustrates shaded regions in 3d, using a different fill pattern for
    -- each region.
    ----------------------------------------------------------------------------
    procedure plot3 is
    xx : array (0 .. 1) of Real_Vector(0 ..4) :=
       ((-1.0, 1.0, 1.0, -1.0, -1.0),
        (-1.0, 1.0, 1.0, -1.0, -1.0));
    yy : array (0 .. 1) of Real_Vector(0 ..4) :=
       ((1.0,  1.0,  0.0, 0.0,  1.0),
        (-1.0, -1.0, 0.0, 0.0, -1.0));
    zz : array (0 .. 1) of Real_Vector(0 ..4) :=
       ((0.0, 0.0, 1.0, 1.0, 0.0),
        (0.0, 0.0, 1.0, 1.0, 0.0));
    begin
        Advance_To_Subpage(Next_Subpage);
        Set_Viewport_Normalized(0.1, 0.9, 0.1, 0.9);
        Set_Viewport_World(-1.0, 1.0, -1.0, 1.0);
        Set_Up_3D(1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 0.0, 1.5, 30.0, -40.0);

        -- Plot using identity transform
        Set_Pen_Color(Red);
        Box_Around_Viewport_3D("bntu", "X", 0.0, 0, "bntu", "Y", 0.0, 0, "bcdfntu", "Z", 0.5, 0);
        Set_Pen_Color(Yellow);
        Write_Labels("","","3-d polygon filling");
        Set_Pen_Color(Green);
        Select_Fill_Pattern(1);
        Draw_Curve_3D(xx(0), yy(0), zz(0));
        Fill_Polygon_3D(xx(0)(0 .. 3), yy(0)(0 .. 3), zz(0)(0 .. 3));
        Select_Fill_Pattern(2);
        Draw_Curve_3D(xx(1), yy(1), zz(1));
        Fill_Polygon_3D(xx(1)(0 .. 3), yy(1)(0 .. 3), zz(1)(0 .. 3));
    end;

begin
    -- Parse and process command line arguments
    Parse_Command_Line_Arguments(Parse_Full);

    -- Set up color map 1
    cmap1_init2;

    -- Initialize plplot
    Initialize_PLplot;

    -- Set up data array
    for i in z'range(1) loop
        xx := Long_Float(i - XPTS / 2) / Long_Float(XPTS / 2);
        for j in z'range(2) loop
            yy := Long_Float(j - YPTS / 2) / Long_Float(YPTS / 2) - 1.0;
            z(i, j) := xx*xx - yy*yy + (xx - yy)/(xx*xx+yy*yy + 0.1);
        end loop;
    end loop;

    zmin := Matrix_Min(z);
    zmax := Matrix_Max(z);

    plot1;
    plot2;
    plot3;

    End_PLplot;
end xstandard15a;
