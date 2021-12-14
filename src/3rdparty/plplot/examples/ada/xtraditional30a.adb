-- Alpha color values demonstration

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
    System,
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Traditional;
use
    System,
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Traditional;

procedure xtraditional30a is
    red   : Integer_Array_1D(0 .. 3) := (  0, 255,   0,   0);
    green : Integer_Array_1D(0 .. 3) := (  0,   0, 255,   0);
    blue  : Integer_Array_1D(0 .. 3) := (  0,   0,   0, 255);
    alpha : Real_Vector(0 .. 3) := (1.0, 1.0, 1.0, 1.0);

    px : Real_Vector(0 .. 3) := (0.1, 0.5, 0.5, 0.1);
    py : Real_Vector(0 .. 3) := (0.1, 0.1, 0.5, 0.5);

    pos    : Real_Vector(0 .. 1) := (0.0, 1.0);
    rcoord : Real_Vector(0 .. 1) := (1.0, 1.0);
    gcoord : Real_Vector(0 .. 1) := (0.0, 0.0);
    bcoord : Real_Vector(0 .. 1) := (0.0, 0.0);
    acoord : Real_Vector(0 .. 1) := (0.0, 1.0);

    icol, r, g, b : Integer;
    a : Long_Float;

begin
    plparseopts(PL_PARSE_FULL);
    plinit;

    plscmap0n(4);
    plscmap0a(red, green, blue, alpha);

    -- Page 1:
    -- This is a series of red, green and blue rectangles overlaid
    -- on each other with gradually increasing transparency.

    -- Set up the window
    pladv(0);
    plvpor(0.0, 1.0, 0.0, 1.0);
    plwind(0.0, 1.0, 0.0, 1.0);
    plcol0(0);
    plbox("", 1.0, 0, "", 1.0, 0);

    -- Draw the boxes
    for i in 0 .. 8 loop
        icol := i mod 3 + 1;

        -- Get a color, change its transparency and set it as the current color.
        plgcol0a(icol, r, g, b, a);
        plscol0a(icol, r, g, b, 1.0 - Long_Float(i) / 9.0);
        plcol0(icol);

        -- Draw the rectangle
        plfill(px, py);

        -- Shift the rectangles coordinates
        for j in px'range loop
            px(j) := px(j) + 0.5 / 9.0;
            py(j) := py(j) + 0.5 / 9.0;
        end loop;
    end loop;

    -- Page 2:
    -- This is a bunch of boxes colored red, green or blue with a single
    -- large (red) box of linearly varying transparency overlaid. The
    -- overlaid box is completely transparent at the bottom and completely
    -- opaque at the top.

    -- Set up the window
    pladv(0);
    plvpor(0.1, 0.9, 0.1, 0.9);
    plwind(0.0, 1.0, 0.0, 1.0);

    -- Draw the boxes. There are 25 of them drawn on a 5 x 5 grid.
    for i in 0 .. 4 loop
        -- Set box X position.
        px(0) := 0.05 + 0.2 * Long_Float(i);
        px(1) := px(0) + 0.1;
        px(2) := px(1);
        px(3) := px(0);

        -- We don't want the boxes to be transparent, so since we changed
        -- the colors transparencies in the first example we have to change
        -- the transparencies back to completely opaque.
        icol := i mod 3 + 1;
        plgcol0a(icol, r, g, b, a);
        plscol0a(icol, r, g, b, 1.0);
        plcol0(icol);
        for j in 0 ..4 loop
            -- Set box y position and draw the box.
            py(0) := 0.05 + 0.2 * Long_Float(j);
            py(1) := py(0);
            py(2) := py(0) + 0.1;
            py(3) := py(2);
            plfill(px, py);
        end loop;
    end loop;

    -- Create the color map with 128 colors and use plscmap1la to initialize
    -- the color values with a linearly varying transparency (or alpha).
    plscmap1n(128);
    plscmap1la(RGB, pos, rcoord, gcoord, bcoord, acoord, Alt_Hue_Path_None);

    -- Use that cmap1 to create a transparent red gradient for the whole window.
    px(0) := 0.0;
    px(1) := 1.0;
    px(2) := 1.0;
    px(3) := 0.0;

    py(0) := 0.0;
    py(1) := 0.0;
    py(2) := 1.0;
    py(3) := 1.0;

    plgradient(px, py, 90.0);

    plend;
end xtraditional30a;
