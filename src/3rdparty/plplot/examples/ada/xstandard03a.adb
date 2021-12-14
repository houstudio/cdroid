-- Polar plot demo.
-- Generates polar plot, with 1-1 scaling.

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
    Ada.Strings,
    Ada.Strings.Fixed,
    PLplot_Auxiliary,
    PLplot_Standard;
use
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    Ada.Strings,
    Ada.Strings.Fixed,
    PLplot_Auxiliary,
    PLplot_Standard;

procedure xstandard03a is
    dtr, theta, dx, dy, r, offset : Long_Float;
    x0, y0 : Real_Vector(0..360);
    x,  y  : Real_Vector(0..360);

begin
    dtr := pi / 180.0;
    for i in x0'range loop
        x0(i) := cos(dtr * Long_Float(i));
        y0(i) := sin(dtr * Long_Float(i));
    end loop;

    -- Parse and process command line arguments.
    Parse_Command_Line_Arguments(Parse_Full);

    -- Set orientation to landscape. Note not all device drivers
    -- support this, in particular most interactive drivers do not.
    Set_Orientation(Portrait);

    -- Initialize plplot
    Initialize_PLplot;

    -- Set up viewport and window, but do not draw box.
    Set_Environment(-1.3, 1.3, -1.3, 1.3, Justified, No_Box);

    -- Draw circles for polar grid.
    for i in 1 .. 10 loop
        Draw_Arc(0.0, 0.0, 0.1 * Long_Float(i), 0.1 * Long_Float(i), 0.0, 360.0, 0.0, False);
    end loop;

    Set_Pen_Color(2);
    for i in 0..11 loop
        theta := 30.0 * Long_Float(i);
        dx := cos(dtr * theta);
        dy := sin(dtr * theta);

        -- Draw radial spokes for polar grid.
        Draw_Line(0.0, 0.0, dx, dy);

        -- Write labels for angle.

        if theta < 9.99 then
           offset := 0.45;
        elsif theta < 99.9 then
	   offset := 0.30;
        else
           offset := 0.15;
        end if;

        -- Slightly off zero to avoid floating point logic flips at 90 and 270 deg
        if dx >= -0.00001 then
            Write_Text_World(dx, dy, dx, dy, -offset, Trim(Integer'image(Integer(theta)), Left));
        else
            Write_Text_World(dx, dy, -dx, -dy, 1.0 + offset, Trim(Integer'image(Integer(theta)), Left));
        end if;
    end loop;

    -- Draw the graph.
    for i in x'range loop
        r := sin(dtr * Long_Float(5 * i));
        x(i) := x0(i) * r;
        y(i) := y0(i) * r;
    end loop;
    Set_Pen_Color(Green);
    Draw_Curve(x, y);

    Set_Pen_Color(Aquamarine);
    Write_Text_Viewport("t", 2.0, 0.5, 0.5, "#frPLplot Example 3 - r(#gh)=sin 5#gh");

    -- Close the plot at end.
    End_PLplot;
end xstandard03a;
