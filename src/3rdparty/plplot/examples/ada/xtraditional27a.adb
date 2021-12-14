-- Drawing "spirograph" curves - epitrochoids, cycolids, roulettes

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
    PLplot_Traditional;
use
    Ada.Numerics,
    Ada.Numerics.Long_Elementary_Functions,
    PLplot_Auxiliary,
    PLplot_Traditional;

------------------------------------------------------------------------------
-- Generates two kinds of plots:
--   - construction of a cycloid (animated)
--   - series of epitrochoids and hypotrochoids
------------------------------------------------------------------------------

procedure xtraditional27a is
    -- R, r, p, N
    -- R and r should be integers to give correct termination of the
    -- angle loop using gcd.
    -- N.B. N is just a place holder since it is no longer used
    -- (because we now have proper termination of the angle loop).
    params : Real_Matrix(0 .. 8, 0 .. 3) :=
      ((21.0,  7.0,  7.0,  3.0),
       (21.0,  7.0, 10.0,  3.0),
       (21.0, -7.0, 10.0,  3.0),
       (20.0,  3.0,  7.0, 20.0),
       (20.0,  3.0, 10.0, 20.0),
       (20.0, -3.0, 10.0, 20.0),
       (20.0, 13.0,  7.0, 20.0),
       (20.0, 13.0, 20.0, 20.0),
       (20.0,-13.0, 20.0, 20.0));

    fill : Boolean;

    -- To understand why spiro is written this way you need to understand the
    -- C code from which this was derived. In the main C program, params
    -- is a two-dimensional array with 9 rows numbered 0 .. 8 and 4 columns
    -- numbered 0 .. 3. When spiro is called, it is passed the _address_ of the
    -- element of params's ith row, 0th column--nothing else. Then, inside spiro,
    -- the corresponding entity (also called params!) appears as a
    -- _one-dimensional_ array whose 0th element shares the same address as what
    -- was passed from the main program. So memory locations starting there,
    -- and numbered from 0, represent the 1D array equivalent to the ith row of
    -- params in the main program. Wilma, call Barney--we're programming a
    -- micaprocessor here.
    procedure spiro(params : Real_Matrix; row : Integer; fill : Boolean) is
        NPNT : constant Integer := 2000;
        xcoord, ycoord : Real_Vector(0 .. NPNT);
        windings : Integer;
        steps    : Integer;
        phi      : Long_Float;
        phiw     : Long_Float;
        dphi     : Long_Float;
        xmin     : Long_Float;
        xmax     : Long_Float;
        xrange_adjust : Long_Float;
        ymin     : Long_Float;
        ymax     : Long_Float;
        yrange_adjust : Long_Float;

        function Trunc(a : Long_Float) return Integer renames PLplot_Auxiliary.Trunc;

        -- Calculate greatest common divisor following pseudo-code for the
        -- Euclidian algorithm at http://en.wikipedia.org/wiki/Euclidean_algorithm
        function gcd(a, b : Integer) return Integer is
            t : Integer;
            aa : Integer := a;
            bb : Integer := b;
        begin
            aa := abs(aa);
            bb := abs(bb);
            while bb /= 0 loop
                t := bb;
                bb := aa mod bb;
                aa := t;
            end loop;
            return aa;
        end gcd;

    begin -- spiro
        -- Fill the coordinates.
        -- Proper termination of the angle loop very near the beginning
        -- point, see http://mathforum.org/mathimages/index.php/Hypotrochoid
        windings := Trunc(abs(params(row, 1)) /
            Long_Float(gcd(Trunc(params(row, 0)), Trunc(params(row, 1)))));
        steps    := NPNT / windings;
        dphi     := 2.0 * pi / Long_Float(steps);
        for i in 0 .. windings * steps loop
            phi       := Long_Float(i) * dphi;
            phiw      := (params(row, 0) - params(row, 1)) / params(row, 1) * phi;
            xcoord(i) := (params(row, 0)-params(row, 1))*cos(phi)+params(row, 2)*cos(phiw);
            ycoord(i) := (params(row, 0)-params(row, 1))*sin(phi)-params(row, 2)*sin(phiw);
            if i = 0 then
                xmin := xcoord(i);
                xmax := xcoord(i);
                ymin := ycoord(i);
                ymax := ycoord(i);
            end if;
            if xmin > xcoord(i) then xmin := xcoord(i); end if;
            if xmax < xcoord(i) then xmax := xcoord(i); end if;
            if ymin > ycoord(i) then ymin := ycoord(i); end if;
            if ymax < ycoord(i) then ymax := ycoord(i); end if;
        end loop;

        xrange_adjust := 0.15 * (xmax - xmin);
        xmin := xmin - xrange_adjust;
        xmax := xmax + xrange_adjust;
        yrange_adjust := 0.15 * (ymax - ymin);
        ymin := ymin - yrange_adjust;
        ymax := ymax + yrange_adjust;

        plwind(xmin, xmax, ymin, ymax);
        plcol0(1);

        declare
            xcoord_local, ycoord_local : Real_Vector(0 .. steps * windings);
        begin
            xcoord_local := xcoord(0 .. steps * windings);
            ycoord_local := ycoord(0 .. steps * windings);
            if fill then
                plfill(xcoord_local, ycoord_local);
            else
                plline(xcoord_local, ycoord_local);
            end if;
        end;
    end spiro;

    procedure cycloid is
    begin
        null; -- TODO
    end cycloid;

    procedure arcs is
       NSEG : constant Integer := 8;
       theta : Long_Float;
       dtheta : Long_Float;
       a : Long_Float;
       b : Long_Float;
    begin

       theta := 0.0;
       dtheta := 360.0/Long_Float(NSEG);
       plenv( -10.0, 10.0, -10.0, 10.0, 1, 0 );

       -- Plot segments of circle in different colors
       for i in 0 .. NSEG-1 loop
          plcol0( (i mod 2) + 1 );
          plarc(0.0, 0.0, 8.0, 8.0, theta, theta + dtheta, 0.0, False);
          theta := theta + dtheta;
       end loop;

       -- Draw several filled ellipses inside the circle at different
       -- angles.
       a := 3.0;
       b := a * tan( (dtheta/180.0*pi)/2.0 );
       theta := dtheta/2.0;
       for i in 0 .. NSEG-1 loop
          plcol0( 2 - (i mod 2 ) );
          plarc( a*cos(theta/180.0*pi), a*sin(theta/180.0*pi), a, b, 0.0, 360.0, theta, True);
          theta := theta + dtheta;
       end loop;

    end arcs;

begin
    -- Parse and process command line arguments
    plparseopts(PL_PARSE_FULL);

    -- Initialize plplot
    plinit;

    -- Illustrate the construction of a cycloid
    cycloid;

    -- Loop over the various curves.
    -- First an overview, then all curves one by one
    plssub(3, 3) ; -- Three by three window

    -- Overview
    fill := False;
    for i in params'range(1) loop
        pladv(0);
        plvpor(0.0, 1.0, 0.0, 1.0);
        spiro(params, i, fill);
    end loop;

    -- Don't fill the curves.
    pladv(0) ;
    plssub(1, 1) ; -- One window per curve
    for i in params'range(1) loop
        pladv(0) ;
        plvpor(0.0, 1.0, 0.0, 1.0);
        spiro(params, i, fill);
    end loop;

    -- Fill the curves
    fill := True;
    pladv(0);
    plssub(1, 1); -- One window per curve
    for i in params'range(1) loop
        pladv(0) ;
        plvpor(0.0, 1.0, 0.0, 1.0);
        spiro(params, i, fill);
    end loop;

    arcs;

    -- Don't forget to call plend to finish off!
    plend;
end xtraditional27a;
