-- Auxiliary types and subprograms to be with-ed by all the Ada
-- bindings to PLplot

-- Copyright (C) 2008-2010 Jerry Bauck

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

package body PLplot_Auxiliary is

--------------------------------------------------------------------------------
--            Utility procedures useful in compiling the examples             --
--------------------------------------------------------------------------------

    -- Mimic C conversion of float to integer; something similar works in e.g.
    -- plplot_thin.adb.
    -- C truncates towards 0. Ada rounds to nearest integer; midway rounded
    -- away from zero, e.g. Inteter(±3.5) is ±4. But any completely reliable
    -- conversion is probalby not possible; indeed, this one exactly emulates C
    -- when tested for values around ±2 to ±3. Both convert ±2.9999999999999997
    -- to ±2 and ±2.9999999999999998 to ±3 which is of course wrong. But that's C.
    function Trunc(a : Long_Float) return Integer is
    begin
        if a >= 0.0 then
            return Integer(a - 0.4999999999999999);
        else
            return Integer(a + 0.4999999999999999);
        end if;
    end Trunc;


    -- Find minimum in a 1D array.
    function Vector_Min(x : Real_Vector) return Long_Float is
        Result : Long_Float;
    begin
        Result := x(x'First);
        for i in x'First+1..x'Last loop
            Result := Long_Float'min(Result, x(i));
        end loop;
        return Result;
    end Vector_Min;


    -- Find minimum and its location in a 1D array.
    procedure Vector_Min(x               : Real_Vector;
                         The_Minimum     : out Long_Float;
                         Location_Of_Min : out Integer) is
    begin
        The_Minimum := x(x'First);
        Location_Of_Min := x'First;
        for i in x'First+1..x'Last loop
            if x(i) < x(i - 1) then -- Finds the first of repeated mins.
                The_Minimum := x(i);
                Location_Of_Min := i;
            end if;
        end loop;
    end Vector_Min;


    -- Find maximum in a 1D array.
    function Vector_Max(x : Real_Vector) return Long_Float  is
        Result : Long_Float;
    begin
        Result := x(x'First);
        for i in x'First+1..x'Last loop
            Result := Long_Float'max(Result, x(i));
        end loop;
        return Result;
    end Vector_Max;


    -- Find maximum and its location in a 1D array.
    procedure Vector_Max(x               : Real_Vector;
                         The_Maximum     : out Long_Float;
                         Location_Of_Max : out Integer) is
    begin
        The_Maximum := x(x'First);
        Location_Of_Max := x'First;
        for i in x'First+1..x'Last loop
            if x(i) > x(i - 1) then -- Finds the first of repeated maxes.
                The_Maximum := x(i);
                Location_Of_Max := i;
            end if;
        end loop;
    end Vector_Max;


    -- Find minimum in a 2D array.
    function Matrix_Min(x : Real_Matrix) return Long_Float is
        Result : Long_Float := Long_Float'large;
    begin
        for j in x'First(2)..x'Last(2) loop
            for i in x'First(1)..x'Last(1) loop
                Result := Long_Float'Min(Result, x(i, j));
            end loop;
        end loop;
        return Result;
    end Matrix_Min;


    -- Find maximum in a 2D array.
    function Matrix_Max(x : Real_Matrix) return Long_Float is
        Result : Long_Float := Long_Float'small;
    begin
        for j in x'First(2)..x'Last(2) loop
            for i in x'First(1)..x'Last(1) loop
                Result := Long_Float'Max(Result, x(i, j));
            end loop;
        end loop;
        return Result;
    end Matrix_Max;

end PLplot_Auxiliary;
