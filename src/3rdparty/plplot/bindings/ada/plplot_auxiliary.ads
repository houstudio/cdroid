-- Auxiliary types and subprograms to be with-ed and by all the Ada
-- bindings to PLplot

-- Copyright (C) 2006-2010 Jerry Bauck

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
    Ada.Strings.Bounded, -- fixme Probable cruft.
    Ada.Strings.Unbounded;

use
    Ada.Strings.Bounded,
    Ada.Strings.Unbounded;

   -- with Ada.Numerics.Long_Real_Arrays;

package PLplot_Auxiliary is

--------------------------------------------------------------------------------
--           Utility type declarations used by the bindings                   --
--------------------------------------------------------------------------------

    -- Declarations for Ada 95 and Ada 2005 when it is desired to _not_ invoke
    -- the numerical capability of Annex G.3.
type Real_Vector is array (Integer range <>) of Long_Float;
type Real_Matrix is array (Integer range <>, Integer range <>) of Long_Float;


    -- Declarations when using Ada 2005 and it is desired to invoke the numerics
    -- Annex G.3 or the user simply prefers to declare real vectors and matrices
    -- in a manner that is type-compatible with that annex. ALSO IN THIS CASE
    -- uncomment the line above: with Ada.Numerics.Long_Real_Arrays;.
    -- Using Annex G.3 requires linking to BLAS and LAPACK libraries or the
    -- PLplot build process will fail when attempting to link the Ada examples
    -- e.g. x01a.adb.
-- subtype Real_Vector is Ada.Numerics.Long_Real_Arrays.Real_Vector;
-- subtype Real_Matrix is Ada.Numerics.Long_Real_Arrays.Real_Matrix;

    ----------------------------------------------------------------------------
    -- Implementation note: The easy ability to switch to Ada 2005 Annex G.3
    -- capability (with only these simple edits to this file) is the only reason
    -- for requiring that this package be with-ed in the bindings. Only the
    -- examples use the utility procedures below--not the bindings themselves.
    -- If it were ever to be decided to abandon Ada 95 compatibility and to
    -- require all Ada-capable PLplot builds to link to BLAS and LAPACK, then
    -- the with-s to this package in the bindings could be removed.
    ----------------------------------------------------------------------------


--------------------------------------------------------------------------------
--            Utility procedures useful in compiling the examples             --
--------------------------------------------------------------------------------

    -- Mimic C conversion of float to integer; something similar works in e.g.
    -- plplot_thin.adb.
    -- C truncates towards 0. Ada rounds to nearest integer; midway rounded
    -- away from zero, e.g. Inteter(±3.5) is ±4. But any completely reliable
    -- conversion is probalby not possible; indeed, this one exactly emulates C
    -- when tested for values around ±2 to ±3. Both convert ±2.9999999999999997
    -- to ±2 and ±2.9999999999999998 to ±3.
    function Trunc(a : Long_Float) return Integer;

    -- Find minimum in a 1D array.
    function Vector_Min(x : Real_Vector) return Long_Float;

    -- Find minimum and its location in a 1D array.
    procedure Vector_Min(x               : Real_Vector;
                         The_Minimum     : out Long_Float;
                         Location_Of_Min : out Integer);

    -- Find maximum in a 1D array.
    function Vector_Max(x : Real_Vector) return Long_Float;

    -- Find maximum and its location in a 1D array.
    procedure Vector_Max(x               : Real_Vector;
                         The_Maximum     : out Long_Float;
                         Location_Of_Max : out Integer);

    -- Find minimum in a 2D array.
    function Matrix_Min(x : Real_Matrix) return Long_Float;

    -- Find maximum in a 2D array.
    function Matrix_Max(x : Real_Matrix) return Long_Float;

end PLplot_Auxiliary;
