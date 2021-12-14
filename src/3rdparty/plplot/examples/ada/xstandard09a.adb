-- Contour plot demo.

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
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Standard;
use
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    System,
    PLplot_Auxiliary,
    PLplot_Standard;

    --------------------------------------------------------------------------
    -- Does several contour plots using different coordinate mappings.
    --------------------------------------------------------------------------

procedure xstandard09a is

    XPTS : Integer := 35; -- Number of data points in x
    YPTS : Integer := 46; -- Number of data points in y

    -- polar plot data
    PERIMETERPTS : Integer := 100;
    RPTS         : Integer := 40;
    THETAPTS     : Integer := 40;

    -- potential plot data
    PPERIMETERPTS : Integer := 100;
    PRPTS         : Integer := 40;
    PTHETAPTS     : Integer := 64;
    PNLEVEL       : Integer := 20;

    xx, yy, argx, argy, distort : Long_Float;
    mark  : Integer_Array_1D(1 .. 1);
    space : Integer_Array_1D(1 .. 1);

    zgdummy : Real_Vector(0 .. 0); -- not used
    z, w : Real_Matrix(0 .. XPTS - 1, 0 .. YPTS - 1);

    clevel : Real_Vector(0 .. 10) := (-1.0, -0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, 0.8, 1.0);

    cgrid1 : aliased Transformation_Data_Type
       (x_Last => XPTS - 1,
        y_Last => YPTS - 1,
        z_Last => 0);

    cgrid2 : aliased Transformation_Data_Type_2
       (x_Last => XPTS - 1,
        y_Last => YPTS - 1);


    -- Quick and dirty function for converting from C. Apparently C lacks an
    -- exponentiation operator.
    -- Don't have a cow over the abs(a) in pow because all calls to pow() in
    -- this example have an exponent of 2.0, i.e., squaring. Without the abs()
    -- Ada raises an exception. (Why doesn't C?)
    function pow(a, b : Long_Float) return Long_Float is
        aa : Long_Float;
    begin
        aa := abs(a);
        return aa ** b;
    end pow;


    procedure mypltr -- This spec is necessary to accommodate pragma Convention().
       (x, y : Long_Float;
        tx, ty : out Long_Float;
        pltr_data : PL_Pointer);
    pragma Convention(Convention => C, Entity => mypltr);

    procedure mypltr
       (x, y : Long_Float;
        tx, ty : out Long_Float;
        pltr_data : PL_Pointer)
    is
        -- Transformation function
        XSPA : Long_Float := 2.0 / Long_Float(XPTS - 1);
        YSPA : Long_Float := 2.0 / Long_Float(YPTS - 1);
        tr : Real_Vector(0 .. 5) := (XSPA, 0.0, -1.0, 0.0, YSPA, -1.0);
    begin -- mypltr
        tx := tr(0) * x + tr(1) * y + tr(2);
        ty := tr(3) * x + tr(4) * y + tr(5);
    end mypltr;


    -- Polar contour plot example
    procedure polar is
        t, r, theta : Long_Float;
        px, py : Real_Vector(0 .. PERIMETERPTS - 1);
        lev : Real_Vector(0 .. 9);
        z : Real_Matrix(0 .. RPTS - 1, 0 .. THETAPTS - 1);
        cgrid2 : aliased Transformation_Data_Type_2
           (x_Last => RPTS - 1,
            y_Last => THETAPTS - 1);

    begin -- polar
        Set_Environment(-1.0, 1.0, -1.0, 1.0, Not_Justified, No_Box);
        Set_Pen_Color(Red);

        -- Perimeter
        for i in 0 .. PERIMETERPTS - 1 loop
            t := (2.0 * pi / Long_Float(PERIMETERPTS - 1)) * Long_Float(i);
            px(i) := cos(t);
            py(i) := sin(t);
        end loop;

        Draw_Curve(px, py);

        for i in 0 .. RPTS - 1 loop
            r := Long_Float(i) / Long_Float(RPTS - 1);
            for j in 0 .. THETAPTS - 1 loop
                theta := (2.0 * pi / Long_Float(THETAPTS - 1)) * Long_Float(j);
                cgrid2.xg(i, j) := r * cos(theta);
                cgrid2.yg(i, j) := r * sin(theta);
                z(i, j) := r;
              end loop;
        end loop;

        for i in lev'range loop
            lev(i) := 0.05 + 0.10 * Long_Float(i);
        end loop;

        Set_Pen_Color(Yellow);
        Contour_Plot(z, 1, RPTS, 1, THETAPTS, lev, Plot_Transformation_2'access, cgrid2'Address);
        Set_Pen_Color(Red);
        Write_Labels("", "", "Polar Contour Plot");
    end polar;


    -- Shielded potential contour plot example
    procedure potential is
        nlevelneg, nlevelpos : Integer;
        ncollin, ncolbox, ncollab : Integer;
        rmax, xmin, xmax, x0, ymin, ymax, y0, zmin, zmax : Long_Float;
        peps, xpmin, xpmax, ypmin, ypmax : Long_Float;
        eps, q1, d1, q1i, d1i, q2, d2, q2i, d2i : Long_Float;
        div1, div1i, div2, div2i : Long_Float;
        t, r, theta : Long_Float;
        dz, clevel : Long_Float;
        clevelneg : Real_Vector(0 .. 9); -- Compare with x09c.c which
        clevelpos : Real_Vector(0 .. 9); -- over-allocates these arrays.
        px, py : Real_Vector(0 .. PPERIMETERPTS - 1);
        z : Real_Matrix(0 .. PRPTS - 1, 0 .. PTHETAPTS - 1);
        cgrid2 : aliased Transformation_Data_Type_2
           (x_Last => PRPTS - 1,
            y_Last => PTHETAPTS - 1);

    begin -- potential
        -- Create data to be contoured.
        for i in 0 .. PRPTS - 1 loop
            r := 0.5 + Long_Float(i);
                for j in 0 .. PTHETAPTS - 1 loop
                    theta := (2.0 * pi / Long_Float(PTHETAPTS - 1)) * (0.5 + Long_Float(j));
                    cgrid2.xg(i, j) := r * cos(theta);
                    cgrid2.yg(i, j) := r * sin(theta);
            end loop;
        end loop;

        rmax := r;
        xmin := Matrix_Min(cgrid2.xg);
        xmax := Matrix_Max(cgrid2.xg);
        ymin := Matrix_Min(cgrid2.yg);
        ymax := Matrix_Max(cgrid2.yg);
        x0 := (xmin + xmax) / 2.0;
        y0 := (ymin + ymax) / 2.0;

        -- Expanded limits
        peps := 0.05;
        xpmin := xmin - abs(xmin) * peps;
        xpmax := xmax + abs(xmax) * peps;
        ypmin := ymin - abs(ymin) * peps;
        ypmax := ymax + abs(ymax) * peps;

        -- Potential inside a conducting cylinder (or sphere) by method of images
        -- Charge 1 is placed at (d1, d1), with image charge at (d2, d2).
        -- Charge 2 is placed at (d1, -d1), with image charge at (d2, -d2).
        -- Also put in smoothing term at small distances.
        eps := 2.0;

        q1 := 1.0;
        d1 := rmax / 4.0;

        q1i := -q1 * rmax / d1;
        d1i := rmax * rmax / d1;

        q2 := -1.0;
        d2 := rmax / 4.0;

        q2i := -q2 * rmax / d2;
        d2i := rmax * rmax / d2;

        for i in 0 .. PRPTS - 1 loop
            for j in 0 .. PTHETAPTS - 1 loop
                div1  := sqrt(pow(cgrid2.xg(i, j)-d1,  2.0) + pow(cgrid2.yg(i, j)-d1,  2.0) + pow(eps, 2.0));
                div1i := sqrt(pow(cgrid2.xg(i, j)-d1i, 2.0) + pow(cgrid2.yg(i, j)-d1i, 2.0) + pow(eps, 2.0));
                div2  := sqrt(pow(cgrid2.xg(i, j)-d2,  2.0) + pow(cgrid2.yg(i, j)+d2,  2.0) + pow(eps, 2.0));
                div2i := sqrt(pow(cgrid2.xg(i, j)-d2i, 2.0) + pow(cgrid2.yg(i, j)+d2i, 2.0) + pow(eps, 2.0));
                z(i, j) := q1/div1 + q1i/div1i + q2/div2 + q2i/div2i;
            end loop;
        end loop;
        zmin := Matrix_Min(z);
        zmax := Matrix_Max(z);

        -- Positive and negative contour levels.
        dz := (zmax - zmin) / Long_Float(PNLEVEL);
        nlevelneg := 0;
        nlevelpos := 0;
        for i in 0 .. PNLEVEL - 1 loop
            clevel := zmin + (Long_Float(i) + 0.5) * dz;
            if clevel <= 0.0 then
                clevelneg(nlevelneg) := clevel;
                nlevelneg := nlevelneg + 1;
            else
                clevelpos(nlevelpos) := clevel;
                nlevelpos := nlevelpos + 1;
            end if;
        end loop;

        -- Colours!
        ncollin := Cyan;
        ncolbox := Red;
        ncollab := Yellow;

        -- Finally start plotting this page!
        Advance_To_Subpage(Next_Subpage);
        Set_Pen_Color(ncolbox);

        Set_Viewport_Maximized_For_Aspect_Ratio(0.1, 0.9, 0.1, 0.9, 1.0);
        Set_Viewport_World(xpmin, xpmax, ypmin, ypmax);
        Box_Around_Viewport("", 0.0, 0, "", 0.0, 0);

        Set_Pen_Color(ncollin);
        if nlevelneg >0 then
            -- Negative contours
            Select_Line_Style(2);
            Contour_Plot(z, 1, PRPTS, 1, PTHETAPTS, clevelneg, Plot_Transformation_2'access, cgrid2'Address);
        end if;

        if nlevelpos > 0 then
            -- Positive contours
            Select_Line_Style(1);
            Contour_Plot(z, 1, PRPTS, 1, PTHETAPTS, clevelpos, Plot_Transformation_2'access, cgrid2'Address);
        end if;

        -- Draw outer boundary
        for i in 0 .. PPERIMETERPTS - 1 loop
            t := (2.0 * pi / Long_Float(PPERIMETERPTS - 1)) * Long_Float(i);
            px(i) := x0 + rmax * cos(t);
            py(i) := y0 + rmax * sin(t);
        end loop;

        Set_Pen_Color(ncolbox);
        Draw_Curve(px, py);

        Set_Pen_Color(ncollab);
        Write_Labels("", "", "Shielded potential of charges in a conducting sphere");

    end potential;


begin -- main
    -- Parse and process command line arguments
    Parse_Command_Line_Arguments(Parse_Full);

    -- Initialize plplot
    Initialize_PLplot;

    -- Take care of limitation on singleton positional aggregates.
    mark(1)  := 1500;
    space(1) := 1500;
    zgdummy(0) := 0.0;

    for i in 0 .. XPTS - 1 loop
        xx := Long_Float(i - XPTS / 2) / Long_Float(XPTS / 2);
        for j in 0 .. YPTS - 1 loop
            yy := Long_Float(j - YPTS / 2) / Long_Float(YPTS / 2) - 1.0;
            z(i, j) := xx * xx - yy * yy;
            w(i, j) := 2.0 * xx * yy;
        end loop;
    end loop;

    -- Set up grids
    cgrid1.zg := zgdummy; -- dummy; not used
    for i in 0 .. XPTS - 1 loop
        for j in 0 .. YPTS - 1 loop
            mypltr(Long_Float(i), Long_Float(j), xx, yy, System.Null_Address); --here

            argx := xx * pi / 2.0;
            argy := yy * pi / 2.0;
            distort := 0.4;

            cgrid1.xg(i) := xx + distort * cos(argx);
            cgrid1.yg(j) := yy - distort * cos(argy);

            cgrid2.xg(i, j) := xx + distort * cos(argx) * cos(argy);
            cgrid2.yg(i, j) := yy - distort * cos(argx) * cos(argy);
        end loop;
    end loop;

    -- The following use of Unrestricted_Access works but is GNAT-specific.
    -- I don't know how else to solve the problem of having a user-specified
    -- function that is passed through five or so levels of C as a callback
    -- while also not requiring the user to modify the source for plplot_thin.adb
    -- which is where the type for the "pltr" subprograms is declared. Even if
    -- I move that type declaration into the thick bindings, the problem remains.

    Set_Contour_Label_Format(4,3);
    Set_Contour_Label_Parameters(0.006, 0.3, 0.1, True);
    Set_Environment(-1.0, 1.0, -1.0, 1.0, Not_Justified, Linear_Box_Plus);
    Set_Pen_Color(Yellow);
    Contour_Plot(z, 1, XPTS, 1, YPTS, clevel, mypltr'Unrestricted_Access, System.Null_Address);

    Set_Line_Style(mark, space);
    Set_Pen_Color(Green);
    Contour_Plot(w, 1, XPTS, 1, YPTS, clevel, mypltr'Unrestricted_Access, System.Null_Address);
    Set_Line_Style(0);
    Set_Pen_Color(Red);
    Write_Labels("X Coordinate", "Y Coordinate", "Streamlines of flow");

    -- Plot using 1d coordinate transform
    Set_Contour_Label_Parameters(0.006, 0.3, 0.1, False);
    Set_Environment(-1.0, 1.0, -1.0, 1.0, Not_Justified, Linear_Box_Plus);
    Set_Pen_Color(Yellow);
    Contour_Plot(z, 1, XPTS, 1, YPTS, clevel, Plot_Transformation_1'access, cgrid1'Address);

    Set_Line_Style(mark, space);
    Set_Pen_Color(Green);
    Contour_Plot(w, 1, XPTS, 1, YPTS, clevel, Plot_Transformation_1'access, cgrid1'Address);
    Set_Line_Style(0);
    Set_Pen_Color(Red);
    Write_Labels("X Coordinate", "Y Coordinate", "Streamlines of flow");

    -- Plot using 2d coordinate transform
    Set_Environment(-1.0, 1.0, -1.0, 1.0, Not_Justified, Linear_Box_Plus);
    Set_Pen_Color(Yellow);
    Contour_Plot(z, 1, XPTS, 1, YPTS, clevel, Plot_Transformation_2'access, cgrid2'Address);

    Set_Line_Style(mark, space);
    Set_Pen_Color(Green);
    Contour_Plot(w, 1, XPTS, 1, YPTS, clevel, Plot_Transformation_2'access, cgrid2'Address);
    Set_Line_Style(0);
    Set_Pen_Color(Red);
    Write_Labels("X Coordinate", "Y Coordinate", "Streamlines of flow");

    -- Make polar plot.
    Set_Contour_Label_Parameters(0.006, 0.3, 0.1, False);
    polar;

    -- Make potential plot.
    Set_Contour_Label_Parameters(0.006, 0.3, 0.1, False);
    potential;

    End_PLplot;
end xstandard09a;
