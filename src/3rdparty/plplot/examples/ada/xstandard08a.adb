-- 3-d plot demo.

-- Copyright (C) 2008-2016 Jerry Bauck

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

procedure xstandard08a is
    -- These values must be odd, for the middle
    -- of the index range to be an integer, and thus
    -- to correspond to the exact floating point centre
    -- of the sombrero.
    XPTS : Integer := 35;
    YPTS : Integer := 45;
    x : Real_Vector(0 .. XPTS - 1);
    y : Real_Vector(0 .. YPTS - 1);
    z : Real_Matrix(0 .. XPTS - 1, 0 .. YPTS - 1);
    z_row_major : Real_Vector(0 .. XPTS * YPTS - 1); -- Guess at the actual dimensions since
    z_col_major : Real_Vector(0 .. XPTS * YPTS - 1); -- C doesn't give a clue.
    dx : Long_Float := 2.0 / Long_Float(XPTS - 1);
    dy : Long_Float := 2.0 / Long_Float(YPTS - 1);
    xx, yy, r : Long_Float;
    zmin, zmax, step : Long_Float;
    LEVELS : Integer := 10;
    clevel: Real_Vector(0 .. LEVELS - 1);
    nlevel : Integer := LEVELS;

    indexxmin : Integer := 0;
    indexxmax : Integer := XPTS;
    indexymin : Integer_Array_1D(0 .. XPTS - 1);
    indexymax : Integer_Array_1D(0 .. XPTS - 1);
    zlimited : Real_Matrix(0 .. XPTS - 1, 0 .. YPTS - 1);

    -- Parameters of ellipse (in x, y index coordinates) that limits the data.
    -- x0, y0 correspond to the exact floating point centre of the index range.
    x0 : Long_Float := 0.5 * Long_Float(XPTS - 1);
    a  : Long_Float := 0.9 * x0;
    y0 : Long_Float := 0.5 * Long_Float(YPTS - 1);
    b  : Long_Float := 0.7 * y0;
    square_root : Long_Float;

    sombrero : Boolean := True; -- Edit this to choose sombrero or Rosenbrock function.
    rosen : Boolean := not sombrero; -- Toggle Rosenbrock according to sombrero.
    alt : Real_Vector(0 .. 1) := (60.0, 40.0);
    az  : Real_Vector(0 .. 1) := (30.0, -30.0);
    title : array(0 .. 1) of Unbounded_String :=
        (TUB("#frPLplot Example 8 - Alt=60, Az=30"),
         TUB("#frPLplot Example 8 - Alt=40, Az=-30"));

    ------------------------------------------------------------------------------
    -- cmap1_init1

    -- Initializes color map 1 in HLS space.
    -- Basic grayscale variation from half-dark (which makes more interesting
    -- looking plot compared to dark) to light.
    -- An interesting variation on this:
    -- s[1] = 1.0
    ----------------------------------------------------------------------------

    procedure cmap1_init(gray : Boolean) is
        i, h, l, s : Real_Vector(0 .. 1);
    begin
        i(0) := 0.0;        -- left boundary
        i(1) := 1.0;        -- right boundary

        if gray then
            h(0) := 0.0;	-- hue -- low: red (arbitrary if s=0)
            h(1) := 0.0;	-- hue -- high: red (arbitrary if s=0)

            l(0) := 0.5;	-- lightness -- low: half-dark
            l(1) := 1.0;	-- lightness -- high: light

            s(0) := 0.0;	-- minimum saturation
            s(1) := 0.0;	-- minimum saturation
         else
            h(0) := 240.0;  -- blue -> green -> yellow ->
            h(1) := 0.0;    -- -> red

            l(0) := 0.6;
            l(1) := 0.6;

            s(0) := 0.8;
            s(1) := 0.8;
        end if;

        Set_Number_Of_Colors_In_Color_Map_1(256);
        Set_Color_Map_1_Piecewise(HLS, i, h, l, s, Alt_Hue_Path_None);
    end cmap1_init;

begin
    -- Parse and process command line arguments
    Parse_Command_Line_Arguments(Parse_Full);
    -- Chose sombrero or rosen in declarations, above.

    -- Initialize plplot
    Initialize_PLplot;

    for i in x'range loop
        -- x(i) := Long_Float(i - XPTS / 2) / Long_Float(XPTS / 2);
        x(i) := -1.0 + Long_Float(i) * dx;
        if rosen then
            x(i) :=  x(i) * 1.5;
        end if;
    end loop;

    for j in y'range loop
        -- y(i) := Long_Float(i - YPTS / 2) / Long_Float(YPTS / 2);
        y(j) := -1.0 + Long_Float(j) * dy;
        if rosen then
            y(j) := y(j) + 0.5;
        end if;
    end loop;

    for i in x'range loop
        xx := x(i);
        for j in y'range loop
            yy := y(j);
            if rosen then
                z(i, j) := (1.0 - xx) * (1.0 - xx) + 100.0 * (yy - (xx * xx)) * (yy - (xx * xx));
                -- The log argument might be zero for just the right grid.
                if z(i, j) > 0.0 then
                    z(i, j) := log(z(i, j));
                else
                    z(i, j) := -5.0; -- -MAXFLOAT would mess-up up the scale
                end if;
            else -- Sombrero
                r := sqrt(xx * xx + yy * yy);
                z(i, j) := exp(-r * r) * cos(2.0 * pi * r);
            end if;
            z_row_major(i * YPTS + j) := z(i, j);
            z_col_major(i + XPTS * j) := z(i, j);
        end loop; -- j
    end loop; -- i

    for i in indexxmin .. indexxmax - 1 loop
        square_root := sqrt( 1.0 - Long_Float'Min(1.0, ((Long_Float(i) - x0) / a)**2));
        -- Add 0.5 to find nearest integer and therefore preserve symmetry
        -- with regard to lower and upper bound of y range.
        -- Ada note: Trunc() is in plplot_auxiliary.adb.
        indexymin(i) := Integer'Max(0, Trunc(0.5 + y0 - b * square_root ));
        -- indexymax calculated with the convention that it is 1
        -- greater than highest valid index.
        indexymax(i) := Integer'Min(YPTS, 1 + Trunc(0.5 + y0 + b * square_root));
        for j in indexymin(i) .. indexymax(i) - 1 loop
            zlimited(i, j) := z(i, j);
        end loop;
    end loop;

    zmin := Matrix_Min(z);
    zmax := Matrix_Max(z);
    step := (zmax - zmin) / Long_Float(nlevel + 1);
    for i in clevel'range loop
        clevel(i) := zmin + step + step * Long_Float(i);
    end loop;

    Set_Light_Source(1.0, 1.0, 1.0);

    for k in alt'range loop
        for ifshade in 0 .. 4 loop
            Advance_To_Subpage(Next_Subpage);
            Set_Viewport_Normalized(0.0, 1.0, 0.0, 0.9);
            Set_Viewport_World(-1.0, 1.0, -0.9, 1.1);
            Set_Pen_Color(Green);
            Write_Text_Viewport("t", 1.0, 0.5, 0.5, To_String(title(k)));
            Set_Pen_Color(Red);
            if rosen then
                Set_Up_3D(1.0, 1.0, 1.0, -1.5, 1.5, -0.5, 1.5, zmin, zmax, alt(k), az(k));
            else
                Set_Up_3D(1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, zmin, zmax, alt(k), az(k));
            end if;

            Box_Around_Viewport_3D("bnstu", "x axis", 0.0, 0,
                "bnstu", "y axis", 0.0, 0,
                "bcdmnstuv", "z axis", 0.0, 0);
            Set_Pen_Color(Yellow);

            if ifshade = 0 then -- diffuse light surface plot
                cmap1_init(True);
                Shaded_Surface_3D(x, y, z, 0, clevel); -- clevel is not used here
            elsif ifshade = 1 then -- magnitude colored plot
                cmap1_init(False);
                Shaded_Surface_3D(x, y, z, MAG_COLOR, clevel);
            elsif ifshade = 2 then --  magnitude colored plot with faceted squares
                cmap1_init(False);
                Shaded_Surface_3D(x, y, z, MAG_COLOR + FACETED, clevel);
            elsif ifshade = 3 then                    -- magnitude colored plot with contours
                cmap1_init(False);
                Shaded_Surface_3D(x, y, z, MAG_COLOR + SURF_CONT + BASE_CONT, clevel);
            else -- magnitude colored plot with contours and index limits
                cmap1_init(False);
                Shaded_Surface_3D_Non_Rectangular(x, y, zlimited, MAG_COLOR + SURF_CONT + BASE_CONT,
                    clevel, indexxmin, indexxmax, indexymin, indexymax );
            end if;
        end loop; -- ifshade
    end loop; -- k
    End_PLplot;
end xstandard08a;
