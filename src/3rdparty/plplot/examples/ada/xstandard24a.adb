-- Unicode Pace Flag

-- Copyright (C) 2008 Jerry Bauck

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
    PLplot_Standard;
use
    PLplot_Auxiliary,
    PLplot_Standard;

procedure xstandard24a is
    red   : Integer_Array_1D(0 .. 6) := (240, 204, 204, 204,   0,  39, 125);
    green : Integer_Array_1D(0 .. 6) := (240,   0, 125, 204, 204,  80,   0);
    blue  : Integer_Array_1D(0 .. 6) := (240,   0,   0,   0,   0, 204, 125);

    px : Real_Vector(0 .. 3) := (0.0, 0.0,  1.0,  1.0);
    py : Real_Vector(0 .. 3) := (0.0, 0.25, 0.25, 0.0);

    sx : Real_Vector(0 .. 11) := (
        0.16374,
        0.15844,
        0.15255,
        0.17332,
        0.50436,
        0.51721,
        0.49520,
        0.48713,
        0.83976,
        0.81688,
        0.82231,
        0.82647);

    sy : Real_Vector(0 .. 11) := (
        0.125,
        0.375,
        0.625,
        0.875,
        0.125,
        0.375,
        0.625,
        0.875,
        0.125,
        0.375,
        0.625,
        0.875);

    -- Taken from http://www.columbia.edu/~fdc/pace/
    function peace(which : Integer) return String is
    begin
        -- Mandarin
        if which = 0  then return "#<0x00>和平"; end if;
        -- Hindi
        if which = 1  then return "#<0x20>शांति"; end if;
        -- English
        if which = 2  then return "#<0x10>Peace"; end if;
        -- Hebrew
        if which = 3  then return "#<0x10>שלום"; end if;
        -- Russian
        if which = 4  then return "#<0x10>Мир"; end if;
        -- German
        if which = 5  then return "#<0x10>Friede"; end if;
        -- Korean
        if which = 6  then return "#<0x30>평화"; end if;
        -- French
        if which = 7  then return "#<0x10>Paix"; end if;
        -- Spanish
        if which = 8  then return "#<0x10>Paz"; end if;
        -- Arabic
        if which = 9  then return "#<0x10>ﺳﻼم"; end if;
        -- Turkish
        if which = 10 then return "#<0x10>Barış"; end if;
        -- Kurdish
        if which = 11 then return "#<0x10>Hasîtî"; end if;
        return "oops";
    end peace;

begin
    Parse_Command_Line_Arguments(Parse_Full);
    Initialize_PLplot;

    Advance_To_Subpage(Next_Subpage);
    Set_Viewport_Normalized(0.0, 1.0, 0.0, 1.0);
    Set_Viewport_World(0.0, 1.0, 0.0, 1.0);
    Set_Pen_Color(Black);
    Box_Around_Viewport("", 1.0, 0, "", 1.0, 0);
    Set_Number_Of_Colors_Map_0(7);
    Set_Color_Map_0(red, green, blue);
    Set_Character_Height(0.0, 4.0);
    Set_Font_Style(Normal_Font);
    for i in 0 .. 3 loop
        Set_Pen_Color(i + 1);
        Fill_Polygon(4, px, py);
            for j in 0 .. 3 loop
                py(j) := py(j) + 1.0 / 4.0;
            end loop;
    end loop;
    Set_Pen_Color(Black);
    for i in 0 .. 11 loop
        Write_Text_World(sx(i), sy(i), 1.0, 0.0, 0.5, peace(i));
    end loop;
    End_PLplot;
end xstandard24a;
