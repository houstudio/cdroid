-- Thin Ada binding to PLplot

-- Copyright (C) 2006-2013 Jerry Bauck

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
    PLplot_Auxiliary,
    System.Address_To_Access_Conversions;
use
    PLplot_Auxiliary,
    Ada.Text_IO;

package body PLplot_Thin is

--------------------------------------------------------------------------------
-- Utility for passing matrices to C                                          --
--------------------------------------------------------------------------------

    -- Take a Real_Matrix as defined in Ada.Numerics.Generic_Real_Arrays
    -- and its instances and produce a 1D array of access variables to the
    -- element located at the first column of each row. This is then suitable
    -- for passing to an external C subroutine which expects a "2D array" in the
    -- form of an array of pointers to "1D arrays" which in turn are pointers to
    -- the first element of each row in C-land. This currently uses the GNAT
    -- attribute Unrestricted_Access which makes it non-portable but allows the
    -- accessing of the matrix elements without aliasing them, which is useful
    -- because the Ada 2005 vector and matrix types are non-aliased.

    -- For more about the Unrestricted_Access attribute, see Implementation
    -- Defined Attributes in the GNAT Reference Manual.

    -- TO-DO: Write a function which accepts x(Index, Index_Of_First_Column)
    -- as an argument and returns a "proper" access variable using the method
    -- discussed in "Ada as a Second Language," Second Edition, by Norman H.
    -- Cohen, Section 19.3, for portability. This should remove GNAT dependence.

    -- Question: Will Unchecked_Access, a normal Ada feature, work instead? fix this

    function Matrix_To_Pointers(x : Real_Matrix) return Long_Float_Pointer_Array is
        Index_Of_First_Column : Integer := x'First(2);
        x_As_Pointers : Long_Float_Pointer_Array (x'range(1));
    begin
        for Index in x'range(1) loop
            x_As_Pointers(Index) := x(Index, Index_Of_First_Column)'Unrestricted_Access;
        end loop;
        return x_As_Pointers;
    end Matrix_To_Pointers;


    --------------------------------------------------------------------------------
    -- Transpose a Matrix.                                                        --
    --------------------------------------------------------------------------------

    function PL_Transpose(A : Real_Matrix) return Real_Matrix is
        B : Real_Matrix(A'range(2), A'range(1));
    begin
        for i in A'range(1) loop
            for j in A'range(2) loop
                B(j, i) := A(i, j);
            end loop;
        end loop;
        return B;
    end PL_Transpose;


--------------------------------------------------------------------------------
--         Functions for use from C or C++ only                               --
--         (Not really ;).                                                    --
--------------------------------------------------------------------------------
-- THESE FUNCTIONS ^^^ ARE NOT IMPLEMENTED FOR THE ADA BINDING
-- EXCEPT FOR THE FOLLOWING.

    -- plparseopts here is an exact copy (exept for the name) of
    -- Parse_Command_Line_Arguments in the thick binding. The reason for
    -- departing from the usual method of simply pragma Import-ing as in
    -- most or all of the other interfaces to C is because of the need to
    -- figure out what the command lines arguments are by also pragma
    -- Import-ing Gnat_Argc and Gnat_Argv. A single-argument version is made
    -- at the request of the development team rather than the three-argument
    -- version of the documetation. The caller specifies only the parse mode.

    -- Process options list using current options info.
    procedure plparseopts(Mode : Parse_Mode_Type) is

        Gnat_Argc : aliased Integer;
        pragma Import (C, Gnat_Argc);

        Gnat_Argv : System.Address;
        pragma Import (C, Gnat_Argv);

        type Gnat_Argc_Access_Type is access all Integer;
        Gnat_Argc_Access : Gnat_Argc_Access_Type;

        procedure
        plparseopts_local(argc : Gnat_Argc_Access_Type;
                          argv : System.Address;
                          mode : Parse_Mode_Type);
        pragma Import(C, plparseopts_local, "c_plparseopts");

    begin
        Gnat_Argc_Access := Gnat_Argc'access;
        plparseopts_local(Gnat_Argc_Access, Gnat_Argv, Mode);
    end plparseopts;


    -- This is a three-argument version of plparseopts as indicated in the
    -- documentation.

    -- Process options list using current options info.
    procedure plparseopts
       (Gnat_Argc : Integer;
        Gnat_Argv : System.Address;
        Mode      : Parse_Mode_Type) is

        Gnat_Argc_Dummy : aliased Integer;

        type Gnat_Argc_Access_Type is access all Integer;
        Gnat_Argc_Access : Gnat_Argc_Access_Type;

        procedure
        plparseopts_local(argc : Gnat_Argc_Access_Type;
                          argv : System.Address;
                          mode : Parse_Mode_Type);
        pragma Import(C, plparseopts_local, "c_plparseopts");

    begin
        Gnat_Argc_Dummy := Gnat_Argc;
        Gnat_Argc_Access := Gnat_Argc_Dummy'access;
        plparseopts_local(Gnat_Argc_Access, Gnat_Argv, Mode);
    end plparseopts;


	-- Transformation routines

    -- pltr0, pltr1, and pltr2 had to be re-written in Ada in order to make the
    -- callback work while also passing the data structure along, e.g.
    -- pltr_data in the formal names below. The machinery surroundinging this idea
    -- also allows for user-defined plot transformation subprograms to be easily
    -- written.

    -- Identity transformation. Re-write of pltr0 in plcont.c in Ada.
    procedure pltr0
       (x, y      : PLFLT;
        tx, ty    : out PLFLT;
        pltr_data : PLplot_thin.PL_Pointer) is
    begin
        tx := x;
        ty := y;
    end pltr0;


    -- Re-write of pltr1 in Ada.
    procedure pltr1
       (x, y      : PLFLT;
        tx, ty    : out PLFLT;
        pltr_data : PLplot_thin.PL_Pointer)
    is
        ul, ur, vl, vr : PLINT;
        du, dv : PLFLT;
        xl, xr, yl, yr : PLFLT;
        nx, ny : PLINT;

        -- Tell the program what structure the data beginning at pltr_data has.
        package Transformation_Data_Type_Address_Conversions is new System.Address_To_Access_Conversions(Transformation_Data_Type);
        Transformation_Data_Pointer : Transformation_Data_Type_Address_Conversions.Object_Pointer;

    begin
        Transformation_Data_Pointer := Transformation_Data_Type_Address_Conversions.To_Pointer(pltr_data);

        nx := Transformation_Data_Pointer.nx;
        ny := Transformation_Data_Pointer.ny;

        -- Ada converts floats to integers by rounding while C does so by
        -- truncation. There is no fool-proof way to fix that. Here, we simply
        -- subtract 0.499999999999999 before doing the conversion. Subtracting
        -- 0.5 results in index constraint errors being raised.
        ul := Integer(x - 0.499999999999999);
        ur := ul + 1;
        du := x - Long_Float(ul);

        vl := Integer(y - 0.499999999999999);
        vr := vl + 1;
        dv := y - Long_Float(vl);

        if x < 0.0 or x > Long_Float(nx - 1) or y < 0.0 or y > Long_Float(ny - 1) then
            -- Mimic (badly, probably) plexit. Don't call plend and don't abort.
            -- This might be better if an exception is declared where plcont is called
            -- to prevent multiple calls to this while trying to make a sinlge plot.
            tx := 0.0; -- These shouldn't be used.
            ty := 0.0; -- Only set them to prevent compiler warning.
            Put_Line("*** PLPLOT ERROR, IMMEDIATE EXIT ***");
            Put_Line("pltr1: Invalid coordinates");
            return; -- Return to caller instead of aborting like plexit.
        end if;

        xl := Transformation_Data_Pointer.xg(ul);
        yl := Transformation_Data_Pointer.yg(vl);

        if ur = Transformation_Data_Pointer.nx then
            tx := xl;
        else
            xr := Transformation_Data_Pointer.xg(ur);
            tx := xl * (1.0 - du) + xr * du;
        end if;

        if vr = Transformation_Data_Pointer.ny then
            ty := yl;
        else
            yr := Transformation_Data_Pointer.yg(vr);
            ty := yl * (1.0 - dv) + yr * dv;
        end if;
    end pltr1;


    -- Re-write of pltr2 in Ada.
    -- Does linear interpolation from doubly dimensioned coord arrays
    -- (column dominant, as per normal C 2d arrays).
    procedure pltr2
       (x, y      : PLFLT;
        tx, ty    : out PLFLT;
        pltr_data : PLplot_thin.PL_Pointer)
    is
        ul, ur, vl, vr : Integer;
        du, dv                 : PLFLT;
        xll, xlr, xrl, xrr     : PLFLT;
        yll, ylr, yrl, yrr     : PLFLT;
        xmin, xmax, ymin, ymax : PLFLT;

        package Transformation_Data_Type_2_Address_Conversions is new System.Address_To_Access_Conversions(Transformation_Data_Type_2);
        TD : Transformation_Data_Type_2_Address_Conversions.Object_Pointer;

    begin
        TD := Transformation_Data_Type_2_Address_Conversions.To_Pointer(pltr_data);

        -- Ada converts floats to integers by rounding while C does so, dangerously, by
        -- truncation. There is no fool-proof way to fix that. Here, we simply
        -- subtract 0.499999999999999 before doing the conversion. Subtracting
        -- 0.5 results in index constraint errors being raised.
        ul := Integer(x - 0.499999999999999);
        ur := ul + 1;
        du := x - Long_Float(ul);

        vl := Integer(y - 0.499999999999999);
        vr := vl + 1;
        dv := y - Long_Float(vl);

        xmin := 0.0;
        xmax := Long_Float(TD.nx - 1);
        ymin := 0.0;
        ymax := Long_Float(TD.ny - 1);

        if x < xmin or x > xmax or y < ymin or y > ymax then
            Put_Line("*** PLPLOT WARNING ***");
            Put_Line("pltr2: Invalid coordinates");

            if x < xmin then

                if y < ymin then
                    tx := TD.xg(0, 0);
                    ty := TD.yg(0, 0);
                elsif y > ymax then
                    tx := TD.xg(0, TD.ny - 1);
                    ty := TD.yg(0, TD.ny - 1);
                else
                    xll := TD.xg(0, vl);
                    yll := TD.yg(0, vl);
                    xlr := TD.xg(0, vr);
                    ylr := TD.yg(0, vr);

                    tx := xll * (1.0 - dv) + xlr * dv;
                    ty := yll * (1.0 - dv) + ylr * dv;
                end if;

            elsif x > xmax then

                if y < ymin then
                    tx := TD.xg(TD.nx - 1, 0);
                    ty := TD.yg(TD.nx - 1, 0);
                elsif y > ymax then
                    tx := TD.xg(TD.nx - 1, TD.ny - 1);
                    ty := TD.yg(TD.nx - 1, TD.ny - 1);
                else
                    xll := TD.xg(TD.nx - 1, vl);
                    yll := TD.yg(TD.nx - 1, vl);
                    xlr := TD.xg(TD.nx - 1, vr);
                    ylr := TD.yg(TD.nx - 1, vr);

                    tx := xll * (1.0 - dv) + xlr * dv;
                    ty := yll * (1.0 - dv) + ylr * dv;
                end if;

            else

                if y < ymin then
                    xll := TD.xg(ul, 0);
                    xrl := TD.xg(ur, 0);
                    yll := TD.yg(ul, 0);
                    yrl := TD.yg(ur, 0);

                    tx := xll * (1.0 - du) + xrl * du;
                    ty := yll * (1.0 - du) + yrl * du;
                elsif y > ymax then
                    xlr := TD.xg(ul, TD.ny - 1);
                    xrr := TD.xg(ur, TD.ny - 1);
                    ylr := TD.yg(ul, TD.ny - 1);
                    yrr := TD.yg(ur, TD.ny - 1);
                    tx := xlr * (1.0 - du) + xrr * du;
                    ty := ylr * (1.0 - du) + yrr * du;
                end if;

            end if;

        -- Normal case.
        -- Look up coordinates in row-dominant array.
        -- Have to handle right boundary specially -- if at the edge, we'd
        -- better not reference the out of bounds point.
        else

            xll := TD.xg(ul, vl);
            yll := TD.yg(ul, vl);

            -- ur is out of bounds
            if ur = TD.nx and vr < TD.ny then
                xlr := TD.xg(ul, vr);
                ylr := TD.yg(ul, vr);

                tx := xll * (1.0 - dv) + xlr * dv;
                ty := yll * (1.0 - dv) + ylr * dv;

            -- vr is out of bounds
            elsif ur < TD.nx and vr = TD.ny then
                xrl := TD.xg(ur, vl);
                yrl := TD.yg(ur, vl);

                tx := xll * (1.0 - du) + xrl * du;
                ty := yll * (1.0 - du) + yrl * du;

            -- both ur and vr are out of bounds
            elsif ur = TD.nx and vr = TD.ny then
                tx := xll;
                ty := yll;

            -- everything in bounds
            else
                xrl := TD.xg(ur, vl);
                xlr := TD.xg(ul, vr);
                xrr := TD.xg(ur, vr);

                yrl := TD.yg(ur, vl);
                ylr := TD.yg(ul, vr);
                yrr := TD.yg(ur, vr);

                tx := xll * (1.0 - du) * (1.0 - dv) + xlr * (1.0 - du) * dv +
                    xrl * du * (1.0 - dv) + xrr * du * dv;

                ty := yll * (1.0 - du) * (1.0 - dv) + ylr * (1.0 - du) * dv +
                    yrl * du * (1.0 - dv) + yrr * du * dv;
            end if;
        end if;
    end pltr2;

end PLplot_Thin;
