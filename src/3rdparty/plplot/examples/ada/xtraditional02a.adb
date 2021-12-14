-- Multiple window and color map 0 demo.

-- Copyright (C) 2007 Alan W. Irwin

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
    Ada.Strings,
    Ada.Strings.Fixed,
    PLplot_Auxiliary,
    PLplot_Traditional;
use
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    Ada.Strings,
    Ada.Strings.Fixed,
    PLplot_Auxiliary,
    PLplot_Traditional;

------------------------------------------------------------------------------
-- Demonstrates multiple windows and color map 0 palette, both default and
-- user-modified.
------------------------------------------------------------------------------

procedure xtraditional02a is
    --------------------------------------------------------------------------
    -- draw_windows
    -- Draws a set of numbered boxes with colors according to cmap0 entry.
    --------------------------------------------------------------------------

    procedure draw_windows(nw, cmap0_offset : Integer) is
        vmin, vmax : Long_Float;

    begin
        plschr(0.0, 3.5);
        plfont(4);

        for i in 0..(nw-1) loop
            plcol0(i+cmap0_offset);
            pladv(0);
            vmin := 0.1;
            vmax := 0.9;
            for j in 0..2 loop
                plwidth(Long_Float(j + 1));
                plvpor(vmin, vmax, vmin, vmax);
                plwind(0.0, 1.0, 0.0, 1.0);
                plbox("bc", 0.0, 0, "bc", 0.0, 0);
                vmin := vmin + 0.1;
                vmax := vmax - 0.1;
            end loop;
            plwidth(1.0);
            plptex(0.5, 0.5, 1.0, 0.0, 0.5, Trim(Integer'image(i), Left));
        end loop;
    end draw_windows;

    --------------------------------------------------------------------------
    -- demo1
    -- Demonstrates multiple windows and default color map 0 palette.
    --------------------------------------------------------------------------

    procedure demo1 is
    begin
        plbop;

        -- Divide screen into 16 regions
        plssub(4, 4);
        draw_windows(16, 0);
        pleop;
    end demo1;

    --------------------------------------------------------------------------
    -- demo2
    -- Demonstrates multiple windows, user-modified color map 0 palette, and
    -- HLS -> RGB translation.
    --------------------------------------------------------------------------

    procedure demo2 is
        -- Set up cmap0
        -- Use 100 custom colors in addition to base 16
        r, g, b: Integer_Array_1D(0..115);

        -- Min & max lightness values
        lmin : Long_Float := 0.15;
        lmax : Long_Float := 0.85;
        h, l, s : Long_Float;
        r1, g1, b1 : Long_Float;
    begin
        plbop;

        -- Divide screen into 100 regions.
        plssub(10, 10);

        for i in 0..99 loop

            -- Bounds on HLS, from plhlsrgb() commentary
            -- hue        [0., 360.] degrees
            -- lightness  [0., 1.]   magnitude
            -- saturation [0., 1.]   magnitude

            -- Vary hue uniformly from left to right
            h := (360.0 / 10.0 ) * Long_Float( i mod 10 );

            -- Vary lightness uniformly from top to bottom, between min & max.
            l := lmin + (lmax - lmin) * Long_Float(i / 10) / 9.0;

            -- Use max saturation.
            s := 1.0;

            plhlsrgb(h, l, s, r1, g1, b1);

            -- Ada converts floats to integers by rounding while C does so by
            -- truncation. There is no fool-proof way to fix that but this is pretty
            -- close: Add a bit less than 1/2 to the float before converting. A good
            -- number to use appears to be about 0.5 - 10^-16 which _might_
            -- be an exact fix for 64-bit floats since they have about 16 digits
            -- of accuracy.
            r(i+16) := Integer((r1 * 255.001) - 0.499999999999999);
            g(i+16) := Integer((g1 * 255.001) - 0.499999999999999);
            b(i+16) := Integer((b1 * 255.001) - 0.499999999999999);
        end loop;

        -- Load default cmap0 colors into our custom set.
        for i in 0..15 loop
            plgcol0(i, r(i), g(i), b(i));
        end loop;

        -- Now set cmap0 all at once (faster, since fewer driver calls).
        plscmap0(r, g, b);

        draw_windows(100, 16);

        pleop;
    end demo2;

begin
    -- Parse and process command line arguments.
    plparseopts(PL_PARSE_FULL);

    -- Initialize plplot.
    plinit;

    -- Run demos.
    demo1;
    demo2;

    -- Don't forget to call plend to finish off!
    plend;

end xtraditional02a;
