-- Font demo.

-- Copyright (C) 2010 Jerry Bauck

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
    Ada.Strings,
    Ada.Strings.Fixed,
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Traditional;
use
    Ada.Strings,
    Ada.Strings.Fixed,
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Traditional;

procedure xtraditional07a is

    k : Integer;
    x, y : Real_Vector(1 .. 1);
    base : array(0 .. 19) of Integer := (0, 100, 0, 100, 200, 500, 600, 700, 800,
        900, 2000, 2100, 2200, 2300, 2400, 2500, 2600, 2700, 2800, 2900);

begin
    -- Parse and process command line arguments
    plparseopts(PL_PARSE_FULL);

    -- Initialize plplot
    plinit;

    plfontld(0);
    for l in 0 .. 19 loop
        if l = 2 then
            plfontld(1);
        end if;
        pladv(0);

        -- Set up viewport and window
        plcol0(2);
        plvpor(0.15, 0.95, 0.1, 0.9);
        plwind(0.0, 1.0, 0.0, 1.0);

        -- Draw the grid using plbox
        plbox("bcg", 0.1, 0, "bcg", 0.1, 0);

        -- Write the digits below the frame
        plcol0(15);
        for i in 0 .. 9 loop
            plmtex("b", 1.5, (0.1 * Long_Float(i) + 0.05), 0.5, Trim(Integer'image(i), Left));
        end loop;

        k := 0;
        for i in 0 .. 9 loop

            -- Write the digits to the left of the frame
            plmtex("lv", 1.0, (0.95 - 0.1 * Long_Float(i)), 1.0, Trim(Integer'image(base(l) + 10 * i), Left));
            for j in 0 .. 9 loop
                x(1) := 0.1 * Long_Float(j) + 0.05;
                y(1) := 0.95 - 0.1 * Long_Float(i);

                -- Display the symbols
                plsym(x, y, base(l) + k);
                k := k + 1;
            end loop;
        end loop;

        if l < 2 then
            plmtex("t", 1.5, 0.5, 0.5, "PLplot Example 7 - PLSYM symbols (compact)");
        else
            plmtex("t", 1.5, 0.5, 0.5, "PLplot Example 7 - PLSYM symbols (extended)");
        end if;
    end loop;
    plend;

end xtraditional07a;
