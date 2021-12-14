-- Filling and clipping polygons.

-- Copyright (C) 2008, 2010 Jerry Bauck

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
    PLplot_Auxiliary,
    PLplot_Traditional;
use
    PLplot_Auxiliary,
    PLplot_Traditional;

-- Test program for filling polygons and proper clipping
procedure xtraditional25a is
    npts : Integer := 0;
    xextreme, yextreme : Real_Matrix(0 .. 8, 0 .. 1);
    x0, y0 : Real_Vector(0 .. 9);
begin
    -- Parse and process command line arguments
    plparseopts(PL_PARSE_FULL);

    -- Initialize plplot
    plssub(3,3);
    plinit;

    xextreme(0, 0) := -120.0; xextreme(0, 1) :=  120.0; yextreme(0, 0) := -120.0; yextreme(0, 1) :=  120.0;
    xextreme(1, 0) := -120.0; xextreme(1, 1) :=  120.0; yextreme(1, 0) :=   20.0; yextreme(1, 1) :=  120.0;
    xextreme(2, 0) := -120.0; xextreme(2, 1) :=  120.0; yextreme(2, 0) :=  -20.0; yextreme(2, 1) :=  120.0;
    xextreme(3, 0) :=  -80.0; xextreme(3, 1) :=   80.0; yextreme(3, 0) :=  -20.0; yextreme(3, 1) :=  120.0;
    xextreme(4, 0) := -220.0; xextreme(4, 1) := -120.0; yextreme(4, 0) := -120.0; yextreme(4, 1) :=  120.0;
    xextreme(5, 0) :=  -20.0; xextreme(5, 1) :=   20.0; yextreme(5, 0) := -120.0; yextreme(5, 1) :=  120.0;
    xextreme(6, 0) :=  -20.0; xextreme(6, 1) :=   20.0; yextreme(6, 0) :=  -20.0; yextreme(6, 1) :=   20.0;
    xextreme(7, 0) :=  -80.0; xextreme(7, 1) :=   80.0; yextreme(7, 0) :=  -80.0; yextreme(7, 1) :=   80.0;
    xextreme(8, 0) :=   20.0; xextreme(8, 1) :=  120.0; yextreme(8, 0) := -120.0; yextreme(8, 1) :=  120.0;

    for k in 0 .. 1 loop
        for j in 0 .. 3 loop
            if j = 0 then
                -- Polygon 1: a diamond
                x0(0) :=    0.0; y0(0) := -100.0;
                x0(1) := -100.0; y0(1) :=    0.0;
                x0(2) :=    0.0; y0(2) :=  100.0;
                x0(3) :=  100.0; y0(3) :=    0.0;
                npts := 4;
            end if;
            if j = 1 then
                -- Polygon 1: a diamond - reverse direction
                x0(3) :=    0.0; y0(3) := -100.0;
                x0(2) := -100.0; y0(2) :=    0.0;
                x0(1) :=    0.0; y0(1) :=  100.0;
                x0(0) :=  100.0; y0(0) :=    0.0;
                npts := 4;
            end if;
            if j = 2 then
                -- Polygon 2: a square with punctures
                x0(0) := -100.0; y0(0) := -100.0;
                x0(1) := -100.0; y0(1) :=  -80.0;
                x0(2) :=   80.0; y0(2) :=    0.0;
                x0(3) := -100.0; y0(3) :=   80.0;
                x0(4) := -100.0; y0(4) :=  100.0;
                x0(5) :=  -80.0; y0(5) :=  100.0;
                x0(6) :=    0.0; y0(6) :=   80.0;
                x0(7) :=   80.0; y0(7) :=  100.0;
                x0(8) :=  100.0; y0(8) :=  100.0;
                x0(9) :=  100.0; y0(9) := -100.0;
                npts := 10;
            end if;
            if j = 3 then
                -- Polygon 2: a square with punctures - reversed direction
                x0(9) := -100.0; y0(9) := -100.0;
                x0(8) := -100.0; y0(8) :=  -80.0;
                x0(7) :=   80.0; y0(7) :=    0.0;
                x0(6) := -100.0; y0(6) :=   80.0;
                x0(5) := -100.0; y0(5) :=  100.0;
                x0(4) :=  -80.0; y0(4) :=  100.0;
                x0(3) :=    0.0; y0(3) :=   80.0;
                x0(2) :=   80.0; y0(2) :=  100.0;
                x0(1) :=  100.0; y0(1) :=  100.0;
                x0(0) :=  100.0; y0(0) := -100.0;
                npts := 10;
            end if;

            for i in xextreme'range(1) loop
                pladv(0);
                plvsta;
                plwind(xextreme(i, 0), xextreme(i, 1), yextreme(i, 0), yextreme(i, 1));

                declare
                    x0_Stop_Sucking, y0_Stop_Sucking : Real_Vector(0 .. npts - 1);
                begin
                    x0_Stop_Sucking := x0(0 .. npts - 1);
                    y0_Stop_Sucking := y0(0 .. npts - 1);

                    plcol0(2);
                    plbox("bc", 1.0, 0, "bcnv", 10.0, 0);
                    plcol0(1);
                    plpsty(0);
                     -- Solid fill or gradient fill
                   if k = 0 then
                        plfill(x0_Stop_Sucking, y0_Stop_Sucking);
                    else
                        plgradient(x0_Stop_Sucking, y0_Stop_Sucking, 45.0);
                    end if;
                    plcol0(2);
                    pllsty(1);
                    plline(x0_Stop_Sucking, y0_Stop_Sucking);
                end;
            end loop; -- i
        end loop; -- j
    end loop; --k

    -- Don't forget to call plend to finish off!
    plend;
end xtraditional25a;
