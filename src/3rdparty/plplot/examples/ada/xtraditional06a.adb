-- Displays the entire "plpoin" symbol (font) set.

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

procedure xtraditional06a is

    k, maxfont : Integer;
    x, y : Real_Vector(1 .. 1);

begin
    -- Parse and process command line arguments
    plparseopts(PL_PARSE_FULL);

    -- Initialize plplot
    plinit;

    for Kind_Font in 0 .. 1 loop
        plfontld(Kind_Font);
        if Kind_Font = 0 then
            maxfont := 1;
        else
            maxfont := 4;
        end if;

        for font in 0 .. maxfont - 1 loop
            plfont(font + 1);
            pladv(0);

            -- Set up viewport and window
            plcol0(2);
            plvpor(0.1, 1.0, 0.1, 0.9);
            plwind(0.0, 1.0, 0.0, 1.3);

            -- Draw the grid using plbox
            plbox("bcg", 0.1, 0, "bcg", 0.1, 0);

            -- Write the digits below the frame
            plcol0(15);
            for i in 0 .. 9 loop
                plmtex("b", 1.5, (0.1 * Long_Float(i) + 0.05), 0.5, Trim(Integer'image(i), Left));
            end loop;

            k := 0;
            for i in 0 .. 12 loop

                -- Write the digits to the left of the frame
                plmtex("lv", 1.0, (1.0 - (2.0 * Long_Float(i) + 1.0) / 26.0), 1.0, Trim(Integer'image(10 * i), Left));
                for j in 0 .. 9 loop
                    x(1) := 0.1 * Long_Float(j) + 0.05;
                    y(1) := 1.25 - 0.1 * Long_Float(i);

                    -- Display the symbols (plpoin expects that x and y are arrays so
                    -- pass pointers)
                    if k < 128 then
                        plpoin(x, y, k);
                    end if;
                    k := k + 1;
                end loop;
            end loop;

            if Kind_Font = 0 then
                plmtex("t", 1.5, 0.5, 0.5, "PLplot Example 6 - plpoin symbols (compact)");
            else
                plmtex("t", 1.5, 0.5, 0.5, "PLplot Example 6 - plpoin symbols (extended)");
            end if;
        end loop; -- font
    end loop; -- Kind_Font
    plend;

end xtraditional06a;
