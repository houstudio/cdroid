-- Histogram demo.
-- Draws a histogram from sample data.

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

with
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Traditional;
use
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Traditional;

procedure xtraditional05a is
    NPTS : constant Integer := 2047;
    data : Real_Vector(0 .. NPTS - 1);
    delt : Long_Float; -- delta is a reserved word in Ada.
begin
    -- Parse and process command line arguments
    plparseopts(PL_PARSE_FULL);

    -- Initialize plplot
    plinit;

    -- Fill up data points
    delt := 2.0 * pi / Long_Float(NPTS);
    for i in data'range loop
        data(i) := sin(Long_Float(i) * delt);
    end loop;

    plcol0(1);
    plhist(data, -1.1, 1.1, 44, 0);
    plcol0(2);
    pllab("#frValue", "#frFrequency",
        "#frPLplot Example 5 - Probability function of Oscillator");
    plend;
end xtraditional05a;
