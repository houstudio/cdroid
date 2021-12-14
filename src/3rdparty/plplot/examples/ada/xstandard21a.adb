-- Grid data demo

-- Copyright (C) 2007 Jerry Bauck

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

-- Ada note: This example originally used Ada's random number generator, but in
-- order to make Postscript results that are identical to the C version, a
-- PLplot-specific random number generator was substituted. The original Ada
-- generator lines are left in as comments for reference.

with
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    -- Ada.Numerics.Float_Random,
    Ada.Strings,
    Ada.Strings.Fixed,
    Ada.Calendar,
    System,
    PLplot_Auxiliary,
    PLplot_Standard;
use
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    -- Ada.Numerics.Float_Random,
    Ada.Strings,
    Ada.Strings.Fixed,
    Ada.Calendar,
    PLplot_Auxiliary,
    PLplot_Standard;

procedure xstandard21a is
    pts : Integer := 500;
    xp  : Integer := 25;
    yp  : Integer := 20;
    nl  : Integer := 16;
    knn_order : Integer := 20;
    threshold : Long_Float := 1.001;
    wmin : Long_Float := -1.0e3;
    randn, rosen : Integer := 0;
    xm, xMM, ym, yMM : Long_Float;
    zmin, zmax, lzm, lzMM : Long_Float;
    dist, d : Long_Float;
    x, y, z : Real_Vector(0 .. pts - 1);
    clev : Real_Vector(0 .. nl - 1);
    xg : Real_Vector(0 .. xp - 1);
    yg : Real_Vector(0 .. yp - 1);
    zg : Real_Matrix(0 .. xp - 1, 0 .. yp - 1);
    opt : Real_Vector(0 .. 5) := (0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    -- Random_Generator : Generator; -- From Ada.Numerics.Float_Random


    function title(which : Integer) return String is
    begin
        if which = 0 then return "Cubic Spline Approximation"; end if;
        if which = 1 then return "Delaunay Linear Interpolation"; end if;
        if which = 2 then return "Natural Neighbors Interpolation"; end if;
        if which = 3 then return "KNN Inv. Distance Weighted"; end if;
        if which = 4 then return "3NN Linear Interpolation"; end if;
        if which = 5 then return "4NN Around Inv. Dist. Weighted"; end if;
        return "oops";
    end title;


    procedure cmap1_init is
        i, h, l, s : Real_Vector(0 .. 1);
    begin
        i(0) := 0.0;   -- left boundary
        i(1) := 1.0;   -- right boundary

        h(0) := 240.0; -- blue -> green -> yellow -> red
        h(1) := 0.0;

        l(0) := 0.6;
        l(1) := 0.6;

        s(0) := 0.8;
        s(1) := 0.8;

        Set_Number_Of_Colors_In_Color_Map_1(256);
        Set_Color_Map_1_Piecewise(HLS, i, h, l, s, Alt_Hue_Path_None);
    end cmap1_init;


    procedure create_grid(x, y : out Real_Vector) is
    begin
        for i in x'range loop
            x(i) := xm + (xMM - xm) * Long_Float(i) / (Long_Float(x'length) - 1.0);
        end loop;

        for i in y'range loop
            y(i) := ym + (yMM - ym) * Long_Float(i) / (Long_Float(y'length) - 1.0);
        end loop;
    end create_grid;


    procedure create_data(x, y, z : out Real_Vector) is
      r, xt, yt : Long_Float;
    begin
        for i in x'range loop
            -- xt := Long_Float(Random(Random_Generator));
            -- yt := Long_Float(Random(Random_Generator));
            xt := (xMM - xm) * Random_Number; -- Use the PLplot random number generator
            yt := (yMM - ym) * Random_Number; -- to make the same plot as C example 21.
            if randn = 0 then
                x(i) := xt + xm;
                y(i) := yt + ym;
            else  -- std=1, meaning that many points are outside the plot range
                x(i) := sqrt(-2.0 *log(xt)) * cos(2.0 * pi * yt) + xm;
                x(i) := sqrt(-2.0 *log(xt)) * sin(2.0 * pi * yt) + ym;
            end if;
            if rosen = 0 then
              r := sqrt((x(i)) * (x(i)) + (y(i)) * (y(i)));
              z(i) := exp(-r * r) * cos(2.0 * pi * r);
            else
                z(i) := log((1.0 - x(i))*(1.0 - x(i)) + 100.0 * (y(i) - x(i)*x(i))*(y(i) - x(i)*x(i)));
            end if;
          end loop;
    end create_data;


    -- Ada lacks full access to IEEE 754 aka IEC 559. The following works
    -- because a NaN is not equal to any other float, including itself.
    -- Use of the 'valid attribute might also work, as might casting to a 64-bit
    -- Integer and comparing to the known bit pattern for NaN; but beware of
    -- quiet NaNs and signalling NaNs. See the discussion at
    -- http://groups.google.com/group/comp.lang.ada/browse_thread/thread/772ddcb41cd06d5b?hl=en
    function Is_NaN(x : Long_Float) return Boolean is
    begin
        return x /= x;
    end Is_NaN;

begin
    xm  := -0.2;
    ym  := -0.2;
    xMM :=  0.6;
    yMM :=  0.6;

    opt(2) := wmin;
    opt(3) := Long_Float(knn_order);
    opt(4) := threshold;

    -- Parse and process command line arguments
    Parse_Command_Line_Arguments(Parse_Full);

    -- Initialize plplot
    Initialize_PLplot;

    cmap1_init;

    Random_Number_Seed(5489);

    create_data(x, y, z); -- the sampled data
    zmin := Vector_Min(z);
    zmax := Vector_Max(z);

    create_grid(xg, yg); -- Grid the data at the output grided data.

    Set_Pen_Color(Red);
    Set_Environment(xm, xMM, ym, yMM, Justified_Square_Box, Linear_Box_Plus);
    Set_Pen_Color(White);
    Write_Labels("X", "Y", "The original data sampling");
    for i in 0 .. (pts-1) loop
        Set_Color_Map_1( (z(i)-zmin)/(zmax-zmin) );
        Draw_String( x(i .. i), y(i .. i), "#(727)" );
    end loop;
    Advance_To_Subpage(Next_Subpage);

    Set_Number_Of_Subpages(3, 2);

    for k in 0 .. 1 loop
        Advance_To_Subpage(Next_Subpage);
        for alg in 1 .. 6 loop

            Grid_Data(x, y, z, xg, yg, zg, alg, opt(alg - 1));

            -- CSA can generate NaNs (only interpolates?!).
            -- DTLI and NNI can generate NaNs for points outside the convex hull
            -- of the data points.
            -- NNLI can generate NaNs if a sufficiently thick triangle is not found
            -- PLplot should be NaN/Inf aware, but changing it now is quite a job...
            -- so, instead of not plotting the NaN regions, a weighted average over
            -- the neighbors is done.
            if alg = GRID_CSA or alg = GRID_DTLI or alg = GRID_NNLI or alg = GRID_NNI then
                for i in xg'range loop
                    for j in yg'range loop
                        if Is_NaN(zg(i, j)) then -- average (IDW) over the 8 neighbors
                            zg(i, j) := 0.0;
                            dist := 0.0;

                            for ii in i - 1 .. i + 1 loop
                                exit when ii >= xp;
                                for jj in j - 1 .. j + 1 loop
                                    exit when jj >= yp;
                                    if ii >= 0 and jj >= 0 then
                                        if not Is_NaN(zg(ii, jj)) then
                                            if abs(ii - i) + abs(jj - j) = 1 then
                                                d := 1.0;
                                            else
                                                d := 1.4142;
                                            end if;
                                            zg(i, j) := zg(i, j) + zg(ii, jj) / (d * d);
                                            dist := dist + d;
                                        end if;
                                    end if;
                                end loop; -- jj
                            end loop; -- ii

                            if dist /= 0.0 then
                                zg(i, j) := zg(i,j) / dist;
                            else
                                zg(i, j) := zmin;
                            end if;

                        end if;
                    end loop; -- j
                end loop; -- i
            end if;

            lzm  := Matrix_Min(zg);
            lzMM := Matrix_Max(zg);

            lzm :=  Vector_Min((lzm,  zmin));
            lzMM := Vector_Max((lzMM, zmax));

            -- Increase limits slightly to prevent spurious contours
            -- due to rounding errors.
            lzm := lzm - 0.01;
            lzMM := lzMM + 0.01;

            Set_Pen_Color(Red);
            Advance_To_Subpage(alg);

            if k = 0 then
                for i in clev'range loop
                    clev(i) := lzm + (lzMM - lzm) / Long_Float(nl-1) * Long_Float(i);
                end loop;

                Set_Environment_Clear_Subpage(xm, xMM, ym, yMM, Justified_Square_Box, Linear_Box_Plus);
                Set_Pen_Color(White);
                Write_Labels("X", "Y", title(alg - 1));
                Shade_Regions(zg, null, xm, xMM, ym, yMM,
                 clev, 1.0, 0, 1.0, Fill_Polygon'access, True, null, System.Null_Address);
                Set_Pen_Color(Yellow);
            else
                for i in clev'range loop
                    clev(i) := lzm + (lzMM - lzm) / Long_Float(nl - 1) * Long_Float(i);
                end loop;

                Set_Viewport_Normalized(0.0, 1.0, 0.0, 0.9);
                Set_Viewport_World(-1.1, 0.75, -0.65, 1.20);

                -- For the comparition to be fair, all plots should have the
                -- same z values, but to get the max/min of the data generated
                -- by all algorithms would imply two passes. Keep it simple.
                Set_Up_3D(1.0, 1.0, 1.0, xm, xMM, ym, yMM, lzm, lzMM, 30.0, -40.0);
                Box_Around_Viewport_3D("bntu", "X", 0.0, 0,
                   "bntu", "Y", 0.0, 0,
                   "bcdfntu", "Z", 0.5, 0);
                Set_Pen_Color(White);
                Write_Labels("", "", title(alg - 1));
                Plot_3D_Base_Contour(xg, yg, zg, DRAW_LINEXY + MAG_COLOR + BASE_CONT, clev);
            end if;
        end loop; -- alg
    end loop; -- k

    End_PLplot;
end xstandard21a;
