--  3-d line and point plot demo.  Adapted from x08c.c.

-- Copyright (C) 2008 - 2011 Jerry Bauck

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
    System,
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

procedure xtraditional18a is
    ----------------------------------------------------------------------------
    -- Does a series of 3-d plots for a given data set, with different
    -- viewing options in each plot.
    ----------------------------------------------------------------------------

    opt : array(0 .. 3) of Boolean := (True, False, True, False);
    alt : Real_Vector(0 .. 3) := (20.0, 35.0, 50.0, 65.0);
    az  : Real_Vector(0 .. 3) := (30.0, 40.0, 50.0, 60.0);
    NPTS : constant Integer := 1000;
    x, y, z : Real_Vector(0 .. NPTS - 1);
    r : Long_Float;


    procedure test_poly(k : Integer) is
        x, y, z : Real_Vector(0 .. 4);
        Two_Pi : Long_Float := 2.0 * pi;
        draw : array(0 .. 3) of Boolean_Array_1D(0 .. 3);

        function THETA(a : Integer) return Long_Float is
        begin
            return Two_Pi * Long_Float(a) / 20.0;
        end;

        function PHI(a : Integer) return Long_Float is
        begin
            return pi * Long_Float(a) / 20.1;
        end;

    begin
        draw(0) := (True,  True,  True,  True);
        draw(1) := (True,  False, True,  False);
        draw(2) := (False, True,  False, True);
        draw(3) := (True,  True,  False, False);

        pladv(0);
        plvpor(0.0, 1.0, 0.0, 0.9);
        plwind(-1.0, 1.0, -0.9, 1.1);
        plcol0(1);
        plw3d(1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, alt(k), az(k));
        plbox3("bnstu", "x axis", 0.0, 0,
           "bnstu", "y axis", 0.0, 0,
           "bcdmnstuv", "z axis", 0.0, 0);
        plcol0(2);

        for i in 0 .. 19 loop
            for j in 0 .. 19 loop
                x(0) := sin(PHI(j)) * cos(THETA(i));
                y(0) := sin(PHI(j)) * sin(THETA(i));
                z(0) := cos(PHI(j));

                x(1) := sin(PHI(j + 1)) * cos(THETA(i));
                y(1) := sin(PHI(j + 1)) * sin(THETA(i));
                z(1) := cos(PHI(j + 1));

                x(2) := sin(PHI(j + 1)) * cos(THETA(i + 1));
                y(2) := sin(PHI(j + 1)) * sin(THETA(i + 1));
                z(2) := cos(PHI(j + 1));

                x(3) := sin(PHI(j)) * cos(THETA(i + 1));
                y(3) := sin(PHI(j)) * sin(THETA(i + 1));
                z(3) := cos(PHI(j));

                x(4) := sin(PHI(j)) * cos(THETA(i));
                y(4) := sin(PHI(j)) * sin(THETA(i));
                z(4) := cos(PHI(j));

                plpoly3(x, y, z, draw(k), True );
            end loop;
        end loop;

        plcol0(3);
        plmtex("t", 1.0, 0.5, 0.5, "unit radius sphere" );
    end test_poly;

begin
    -- Parse and process command line arguments
    plparseopts(PL_PARSE_FULL);

    -- Initialize plplot
    plinit;

    for k in 0 .. 3 loop
        test_poly(k);
    end loop;

    -- From the mind of a sick and twisted physicist...
    for i in x'range loop
        z(i) := -1.0 + 2.0 * Long_Float(i) / Long_Float(NPTS);

        -- Pick one...
        r := z(i);

        x(i) := r * cos( 2.0 * pi * 6.0 * Long_Float(i) / Long_Float(NPTS) );
        y(i) := r * sin( 2.0 * pi * 6.0 * Long_Float(i) / Long_Float(NPTS) );
    end loop;

    for k in opt'range loop
        pladv(0);
        plvpor(0.0, 1.0, 0.0, 0.9);
        plwind(-1.0, 1.0, -0.9, 1.1);
        plcol0(1);
        plw3d(1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, alt(k), az(k));
        plbox3("bnstu", "x axis", 0.0, 0,
               "bnstu", "y axis", 0.0, 0,
               "bcdmnstuv", "z axis", 0.0, 0);
        plcol0(2);

        if opt(k) then
            plline3(x, y, z);
        else
            -- U+22C5 DOT OPERATOR.
            plstring3(x, y, z, "⋅");
        end if;

        plcol0(3);
        plmtex("t", 1.0, 0.5, 0.5, "#frPLplot Example 18 - Alt=" &
            Trim(Integer'image(Integer(alt(k))), Left) & ", Az=" &
            Trim(Integer'image(Integer( az(k))), Left));
    end loop;

    plend;
end xtraditional18a;
