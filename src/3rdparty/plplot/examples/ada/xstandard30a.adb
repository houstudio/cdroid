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
    PLplot_Standard;
use
    System,
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Standard;

procedure xstandard30a is
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
    Parse_Command_Line_Arguments(Parse_Full);
    Initialize_PLplot;

    Set_Number_Of_Colors_Map_0(4);
    Set_Color_Map_0_And_Alpha(red, green, blue, alpha);

    -- Page 1:
    -- This is a series of red, green and blue rectangles overlaid
    -- on each other with gradually increasing transparency.

    -- Set up the window
    Advance_To_Subpage(Next_Subpage);
    Set_Viewport_Normalized(0.0, 1.0, 0.0, 1.0);
    Set_Viewport_World(0.0, 1.0, 0.0, 1.0);
    Set_Pen_Color(Black);
    Box_Around_Viewport("", 1.0, 0, "", 1.0, 0);

    -- Draw the boxes
    for i in 0 .. 8 loop
        icol := i mod 3 + 1;

        -- Get a color, change its transparency and set it as the current color.
        Get_Color_RGB_And_Alpha(icol, r, g, b, a);
        Set_One_Color_Map_0_And_Alpha(icol, r, g, b, 1.0 - Long_Float(i) / 9.0);
        Set_Pen_Color(icol);

        -- Draw the rectangle
        Fill_Polygon(px, py);

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
    Advance_To_Subpage(Next_Subpage);
    Set_Viewport_Normalized(0.1, 0.9, 0.1, 0.9);
    Set_Viewport_World(0.0, 1.0, 0.0, 1.0);

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
        Get_Color_RGB_And_Alpha(icol, r, g, b, a);
        Set_One_Color_Map_0_And_Alpha(icol, r, g, b, 1.0);
        Set_Pen_Color(icol);
        for j in 0 ..4 loop
            -- Set box y position and draw the box.
            py(0) := 0.05 + 0.2 * Long_Float(j);
            py(1) := py(0);
            py(2) := py(0) + 0.1;
            py(3) := py(2);
            Fill_Polygon(px, py);
        end loop;
    end loop;

    -- Create the color map with 128 colors and use Set_Color_Map_1_RGBla to initialize
    -- the color values with a linearly varying transparency (or alpha).
    Set_Number_Of_Colors_In_Color_Map_1(128);
    Set_Color_Map_1_Piecewise_And_Alpha(RGB, pos, rcoord, gcoord, bcoord, acoord, Alt_Hue_Path_None);

    -- Use that cmap1 to create a transparent red gradient for the whole window.
    px(0) := 0.0;
    px(1) := 1.0;
    px(2) := 1.0;
    px(3) := 0.0;

    py(0) := 0.0;
    py(1) := 0.0;
    py(2) := 1.0;
    py(3) := 1.0;

    Fill_Polygon_Gradient(px, py, 90.0);

    End_PLplot;
end xstandard30a;
