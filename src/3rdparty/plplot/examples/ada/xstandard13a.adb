-- Pie chart demo.

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
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Standard;
use
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Standard;

------------------------------------------------------------------------------
-- Does a simple pie chart.
------------------------------------------------------------------------------

procedure xstandard13a is
    j, dthet, theta0, theta1, theta : Integer;
    just, dx, dy : Long_Float;
    x, y : Real_Vector(0 .. 499);
    per  : Real_Vector(0 .. 4) := (10.0, 32.0, 12.0, 30.0, 16.0);

begin
    -- Parse and process command line arguments
    Parse_Command_Line_Arguments(Parse_Full);

    -- Initialize plplot
    Initialize_PLplot;

    Advance_To_Subpage(Next_Subpage);

    -- Ensure window has aspect ratio of one so circle is plotted as a circle.
    Set_Viewport_Aspect_Ratio(1.0);
    Set_Viewport_World(0.0, 10.0, 0.0, 10.0);
    -- Set_Environment(0.0, 10.0, 0.0, 10.0, Justified, No_Box);
    Set_Pen_Color(Yellow);

    -- n.b. all theta quantities scaled by 2 * pi / 500 to be integers to avoid
    -- floating point logic problems.
    theta0 := 0;
    dthet := 1;
    for i in per'range loop
        j := 0;
        x(j) := 5.0;
        y(j) := 5.0;
        j := j + 1;

        -- n.b. the theta quantities multiplied by 2 * pi / 500 afterward so
        -- in fact per is interpreted as a percentage.
        theta1 := Integer(Long_Float(theta0) + 5.0 * per(i));
        if i = 4 then
            theta1 := 500;
        end if;

        theta := theta0;
        while theta <= theta1 loop
            x(j) := 5.0 + 3.0 * cos((2.0 * pi / 500.0) * Long_Float(theta));
            y(j) := 5.0 + 3.0 * sin((2.0 * pi / 500.0) * Long_Float(theta));
            j := j + 1;
            theta := theta + dthet;
        end loop;

        Set_Pen_Color(i + 1);
        Select_Fill_Pattern((i + 3) mod 8 + 1);

        declare
            x_Temp, y_Temp : Real_Vector(0 .. j - 1);
        begin
            x_Temp := x(0 .. j - 1);
            y_Temp := y(0 .. j - 1);
            Fill_Polygon(x_Temp, y_Temp);
            Set_Pen_Color(Red);
            Draw_Curve(x_Temp, y_Temp);
        end;

        just := (2.0 * pi / 500.0) * Long_Float((theta0 + theta1))/2.0;
        dx := 0.25 * cos(just);
        dy := 0.25 * sin(just);
        if (theta0 + theta1) < 250 or (theta0 + theta1) > 750 then
            just := 0.0;
        else
            just := 1.0;
        end if;

        if i = 0 then
            Write_Text_World(x(j / 2) + dx, y(j / 2) + dy, 1.0, 0.0, just, "Maurice");
        elsif i = 1 then
            Write_Text_World(x(j / 2) + dx, y(j / 2) + dy, 1.0, 0.0, just, "Geoffrey");
        elsif i = 2 then
            Write_Text_World(x(j / 2) + dx, y(j / 2) + dy, 1.0, 0.0, just, "Alan");
        elsif i = 3 then
            Write_Text_World(x(j / 2) + dx, y(j / 2) + dy, 1.0, 0.0, just, "Rafael");
        elsif i = 4 then
            Write_Text_World(x(j / 2) + dx, y(j / 2) + dy, 1.0, 0.0, just, "Vince");
        end if;

        theta0 := theta - dthet;
    end loop;
    Set_Font_Style(Roman_Font);
    Set_Character_Height(0.0, 1.3);
    Write_Text_World(5.0, 9.0, 1.0, 0.0, 0.5, "Percentage of Sales");

    -- Don't forget to call PLEND to finish off!
    End_PLplot;
end xstandard13a;
