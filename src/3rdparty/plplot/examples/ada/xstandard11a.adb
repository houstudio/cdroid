-- Mesh plot demo.

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
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Standard;
use
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Standard;

procedure xstandard11a is
    XPTS   : constant Integer := 35;   -- Data points in x
    YPTS   : constant Integer := 46;   -- Data points in y
    LEVELS : constant Integer := 10;

    opt : Integer_Array_1D(0 ..1) := (DRAW_LINEXY, DRAW_LINEXY);
    alt : Real_Vector(0 ..1) := (33.0, 17.0);
    az  : Real_Vector(0 ..1) := (24.0, 115.0);

    title0 : String(1 .. 43) := ("#frPLplot Example 11 - Alt=33, Az=24, Opt=3");
    title1 : String(1 .. 44) := ("#frPLplot Example 11 - Alt=17, Az=115, Opt=3");

    x : Real_Vector(0 .. XPTS - 1);
    y : Real_Vector(0 .. YPTS - 1);
    z : Real_Matrix(0 .. XPTS - 1, 0 .. YPTS - 1);
    xx, yy : Long_Float;
    nlevel : Integer := LEVELS;
    clevel : Real_Vector(0 .. LEVELS - 1);
    zmin, zmax, step : Long_Float;

    procedure cmap1_init is
        i, h, l, s : Real_Vector(0 .. 1);
    begin
        i(0) := 0.0;		-- left boundary
        i(1) := 1.0;		-- right boundary

        h(0) := 240.0;     -- blue -> green -> yellow ->
        h(1) := 0.0;       -- -> red

        l(0) := 0.6;
        l(1) := 0.6;

        s(0) := 0.8;
        s(1) := 0.8;

        Set_Number_Of_Colors_In_Color_Map_1(256);
        Set_Color_Map_1_Piecewise(HLS, i, h, l, s, Alt_Hue_Path_None);
    end cmap1_init;


    -- Does a series of mesh plots for a given data set, with different
    -- viewing options in each plot.
begin
    -- Parse and process command line arguments

    Parse_Command_Line_Arguments(Parse_Full);

    -- Initialize plplot
    Initialize_PLplot;

    for i in x'range loop
        x(i) := 3.0 * Long_Float(i - XPTS / 2) / Long_Float(XPTS / 2);
    end loop;

    for i in y'range loop
        y(i) := 3.0 * Long_Float(i - YPTS / 2) / Long_Float(YPTS / 2);
    end loop;

    for i in x'range loop
        xx := x(i);
        for j in y'range loop
            yy := y(j);
            z(i, j) := 3.0 * (1.0-xx)*(1.0-xx) * exp(-(xx*xx) - (yy+1.0)*(yy+1.0)) -
            10.0 * (xx/5.0 - xx**3 - yy**5) * exp(-xx*xx-yy*yy) -
            1.0/3.0 * exp(-(xx+1.0)*(xx+1.0) - (yy*yy));

--            if(0) {-- Jungfraujoch/Interlaken
--            if (z(i)(j) < -1.)
--            z(i)(j) := -1.;
--          }
        end loop;
    end loop;

    zmin := Matrix_Min(z);
    zmax := Matrix_max(z);
    step := (zmax - zmin) / Long_Float(nlevel + 1);
    for i in clevel'range loop
        clevel(i) := zmin + step + step * Long_Float(i);
    end loop;

    cmap1_init;

    for k in 0 .. 1 loop
        for i in 0 .. 3 loop
            Advance_To_Subpage(Next_Subpage);
            Set_Pen_Color(Red);
            Set_Viewport_Normalized(0.0, 1.0, 0.0, 0.9);
            Set_Viewport_World(-1.0, 1.0, -1.0, 1.5);
            Set_Up_3D(1.0, 1.0, 1.2, -3.0, 3.0, -3.0, 3.0, zmin, zmax, alt(k), az(k));
            Box_Around_Viewport_3D("bnstu", "x axis", 0.0, 0, "bnstu", "y axis", 0.0, 0, "bcdmnstuv", "z axis", 0.0, 4);
            Set_Pen_Color(Yellow);

            -- wireframe plot
            if i = 0 then
                Mesh_3D(x, y, z, opt(k));

            -- magnitude colored wireframe plot
            elsif i = 1 then
                Mesh_3D(x, y, z, opt(k) + MAG_COLOR);

            -- magnitude colored wireframe plot with sides
            elsif i = 2 then
                Plot_3D(x, y, z, opt(k) + MAG_COLOR, True);

            -- magnitude colored wireframe plot with base contour
            elsif i = 3 then
                Mesh_3D_Base_Contour(x, y, z, opt(k) + MAG_COLOR + BASE_CONT, clevel);
            end if;

            Set_Pen_Color(Green);
            if k = 0 then
                Write_Text_Viewport("t", 1.0, 0.5, 0.5, title0);
            end if;
            if k = 1 then
                Write_Text_Viewport("t", 1.0, 0.5, 0.5, title1);
            end if;
        end loop;
    end loop;

    -- Clean up
    End_PLplot;
end xstandard11a;
