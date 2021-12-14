-- plmtex3, plptex3 demo

-- Copyright (C) 2008-2010 Jerry Bauck

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

-- Demonstrates plotting text in 3D.
procedure xtraditional28a is
    -- Choose these values to correspond to tick marks.
    xpts        : constant Integer := 2;
    ypts        : constant Integer := 2;
    nrevolution : constant Integer := 16;
    nrotation   : constant Integer := 8;
    nshear      : constant Integer := 8;

    xmin, ymin, zmin : Long_Float := 0.0;
    xmax, ymax, zmax : Long_Float := 1.0;

    xmid   : Long_Float := 0.5*(xmax + xmin);
    xrange : Long_Float := xmax - xmin;

    ymid   : Long_Float := 0.5*(ymax + ymin);
    yrange : Long_Float := ymax - ymin;

    zmid   : Long_Float := 0.5*(zmax + zmin);
    zrange : Long_Float := zmax - zmin;

    ysmin    : Long_Float := ymin + 0.1 * yrange;
    ysmax    : Long_Float := ymax - 0.1 * yrange;
    ysrange  : Long_Float := ysmax - ysmin;
    dysrot   : Long_Float := ysrange / Long_Float(nrotation - 1);
    dysshear : Long_Float := ysrange / Long_Float(nshear - 1);
    zsmin    : Long_Float := zmin + 0.1 * zrange;
    zsmax    : Long_Float := zmax - 0.1 * zrange;
    zsrange  : Long_Float := zsmax - zsmin;
    dzsrot   : Long_Float := zsrange / Long_Float(nrotation - 1);
    dzsshear : Long_Float := zsrange / Long_Float(nshear - 1);
    ys, zs   : Long_Float;

    x_inclination, y_inclination, z_inclination : Long_Float;
    x_shear, y_shear, z_shear                   : Long_Float;
    omega, sin_omega, cos_omega, domega         : Long_Float;
    radius, pitch, xpos, ypos, zpos             : Long_Float;

    -- p1string must be exactly one character + the null termination character.
    plstring : String(1 .. 1);
    pstring  : String := "The future of our civilization depends on software freedom.";

    -- Allocate and define the minimal x, y, and z to insure 3D box.
    x, y : Real_Vector(0 .. xpts - 1);
    z : Real_Matrix(0 .. xpts - 1, 0 .. ypts - 1) := ((others => 0.0), (others => 0.0));
begin
    for i in x'range loop
        x(i) := xmin + Long_Float(i) * (xmax - xmin) / Long_Float(xpts - 1);
    end loop;

    for i in y'range loop
        y(i) := ymin + Long_Float(i) * (ymax - ymin) / Long_Float(ypts - 1);
    end loop;

    -- Parse and process command line arguments.
    plparseopts(PL_PARSE_FULL);

    plinit;

    -- Page 1: Demonstrate inclination and shear capability pattern.
    pladv(0);
    plvpor(-0.15, 1.15, -0.05, 1.05);
    plwind(-1.2, 1.2, -0.8, 1.5);
    plw3d(1.0, 1.0, 1.0, xmin, xmax, ymin, ymax, zmin, zmax, 20.0, 45.0);

    plcol0(2);
    plbox3("b", "", xmax-xmin, 0, "b", "", ymax-ymin, 0, "bcd", "", zmax-zmin, 0);

    -- z = zmin
    plschr(0.0, 1.0);
    for i in 0 .. nrevolution - 1 loop
        omega := 2.0 * pi * Long_Float(i) / Long_Float(nrevolution);
        sin_omega := sin(omega);
        cos_omega := cos(omega);
        x_inclination := 0.5 * xrange * cos_omega;
        y_inclination := 0.5 * yrange * sin_omega;
        z_inclination := 0.0;
        x_shear := -0.5 * xrange * sin_omega;
        y_shear :=  0.5 * yrange * cos_omega;
        z_shear := 0.0;
        plptex3(
            xmid, ymid, zmin,
            x_inclination, y_inclination, z_inclination,
            x_shear, y_shear, z_shear,
            0.0, "  revolution");
    end loop;

    -- x = xmax
    plschr(0.0, 1.0);
    for i in 0 .. nrevolution - 1 loop
        omega := 2.0 * pi * Long_Float(i) / Long_Float(nrevolution);
        sin_omega := sin(omega);
        cos_omega := cos(omega);
        x_inclination := 0.0;
        y_inclination := -0.5*yrange*cos_omega;
        z_inclination := 0.5*zrange*sin_omega;
        x_shear := 0.0;
        y_shear := 0.5*yrange*sin_omega;
        z_shear := 0.5*zrange*cos_omega;
        plptex3(
            xmax, ymid, zmid,
            x_inclination, y_inclination, z_inclination,
            x_shear, y_shear, z_shear,
            0.0, "  revolution");
    end loop;

    -- y = ymax
    plschr(0.0, 1.0);
    for i in 0 .. nrevolution - 1 loop
        omega := 2.0 * pi * Long_Float(i) / Long_Float(nrevolution);
        sin_omega := sin(omega);
        cos_omega := cos(omega);
        x_inclination := 0.5 * xrange * cos_omega;
        y_inclination := 0.0;
        z_inclination := 0.5 * zrange * sin_omega;
        x_shear := -0.5 * xrange * sin_omega;
        y_shear :=  0.0;
        z_shear :=  0.5 * zrange * cos_omega;
        zs := zsmax - dzsrot * Long_Float(i);
        plptex3(
            xmid, ymax, zmid,
            x_inclination, y_inclination, z_inclination,
            x_shear, y_shear, z_shear,
            0.0, "  revolution");
    end loop;

    -- Draw minimal 3D grid to finish defining the 3D box.
    plmesh(x, y, z, DRAW_LINEXY);

    -- Page 2: Demonstrate rotation of string around its axis.
    pladv(0);
    plvpor(-0.15, 1.15, -0.05, 1.05);
    plwind(-1.2, 1.2, -0.8, 1.5);
    plw3d(1.0, 1.0, 1.0, xmin, xmax, ymin, ymax, zmin, zmax, 20.0, 45.0);

    plcol0(2);
    plbox3("b", "", xmax-xmin, 0, "b", "", ymax-ymin, 0, "bcd", "", zmax-zmin, 0);

    -- y = ymax
    plschr(0.0, 1.0);
    x_inclination := 1.0;
    y_inclination := 0.0;
    z_inclination := 0.0;
    x_shear := 0.0;
    for i in 0 .. nrotation - 1 loop
        omega := 2.0 * pi * Long_Float(i) / Long_Float(nrotation);
        sin_omega := sin(omega);
        cos_omega := cos(omega);
        y_shear := 0.5 * yrange * sin_omega;
        z_shear := 0.5 * zrange * cos_omega;
        zs := zsmax - dzsrot * Long_Float(i);
        plptex3(
            xmid, ymax, zs,
            x_inclination, y_inclination, z_inclination,
            x_shear, y_shear, z_shear,
            0.5, "rotation for y = y#dmax#u");
    end loop;

    -- x = xmax
    plschr(0.0, 1.0);
    x_inclination := 0.0;
    y_inclination := -1.0;
    z_inclination := 0.0;
    y_shear := 0.0;
    for i in 0 .. nrotation - 1 loop
        omega := 2.0 * pi * Long_Float(i) / Long_Float(nrotation);
        sin_omega := sin(omega);
        cos_omega := cos(omega);
        x_shear := 0.5 * xrange * sin_omega;
        z_shear := 0.5 * zrange * cos_omega;
        zs := zsmax - dzsrot * Long_Float(i);
        plptex3(
            xmax, ymid, zs,
            x_inclination, y_inclination, z_inclination,
            x_shear, y_shear, z_shear,
            0.5, "rotation for x = x#dmax#u");
    end loop;

    -- z = zmin
    plschr(0.0, 1.0);
    x_inclination := 1.0;
    y_inclination := 0.0;
    z_inclination := 0.0;
    x_shear := 0.0;
    for i in 0 .. nrotation - 1 loop
        omega := 2.0 * pi * Long_Float(i) / Long_Float(nrotation);
        sin_omega := sin(omega);
        cos_omega := cos(omega);
        y_shear := 0.5 * yrange * cos_omega;
        z_shear := 0.5 * zrange * sin_omega;
        ys := ysmax - dysrot * Long_Float(i);
        plptex3(
              xmid, ys, zmin,
              x_inclination, y_inclination, z_inclination,
              x_shear, y_shear, z_shear,
              0.5, "rotation for z = z#dmin#u");
    end loop;

    -- Draw minimal 3D grid to finish defining the 3D box.
    plmesh(x, y, z, DRAW_LINEXY);

    -- Page 3: Demonstrate shear of string along its axis.
    -- Work around xcairo and pngcairo (but not pscairo) problems for
    -- shear vector too close to axis of string. (N.B. no workaround
    -- would be domega := 0.)
    domega := 0.05;
    pladv(0);
    plvpor(-0.15, 1.15, -0.05, 1.05);
    plwind(-1.2, 1.2, -0.8, 1.5);
    plw3d(1.0, 1.0, 1.0, xmin, xmax, ymin, ymax, zmin, zmax, 20.0, 45.0);

    plcol0(2);
    plbox3("b", "", xmax-xmin, 0,
      "b", "", ymax-ymin, 0,
      "bcd", "", zmax-zmin, 0);

    -- y = ymax
    plschr(0.0, 1.0);
    x_inclination := 1.0;
    y_inclination := 0.0;
    z_inclination := 0.0;
    y_shear := 0.0;
    for i in 0 .. nshear - 1 loop
        omega := domega + 2.0 * pi * Long_Float(i) / Long_Float(nshear);
        sin_omega := sin(omega);
        cos_omega := cos(omega);
        x_shear := 0.5 * xrange * sin_omega;
        z_shear := 0.5 * zrange * cos_omega;
        zs := zsmax - dzsshear * Long_Float(i);
        plptex3(
            xmid, ymax, zs,
            x_inclination, y_inclination, z_inclination,
            x_shear, y_shear, z_shear,
            0.5, "shear for y = y#dmax#u");
    end loop;

    -- x = xmax
    plschr(0.0, 1.0);
    x_inclination := 0.0;
    y_inclination := -1.0;
    z_inclination := 0.0;
    x_shear := 0.0;
    for i in 0 .. nshear - 1 loop
        omega := domega + 2.0 * pi * Long_Float(i) / Long_Float(nshear);
        sin_omega := sin(omega);
        cos_omega := cos(omega);
        y_shear := -0.5 * yrange * sin_omega;
        z_shear :=  0.5 * zrange * cos_omega;
        zs := zsmax - dzsshear * Long_Float(i);
        plptex3(
            xmax, ymid, zs,
            x_inclination, y_inclination, z_inclination,
            x_shear, y_shear, z_shear,
            0.5, "shear for x = x#dmax#u");
    end loop;

    -- z = zmin
    plschr(0.0, 1.0);
    x_inclination := 1.0;
    y_inclination := 0.0;
    z_inclination := 0.0;
    z_shear := 0.0;
    for i in 0 .. nshear - 1 loop
        omega := domega + 2.0 * pi * Long_Float(i) / Long_Float(nshear);
        sin_omega := sin(omega);
        cos_omega := cos(omega);
        y_shear := 0.5 * yrange * cos_omega;
        x_shear := 0.5 * xrange * sin_omega;
        ys := ysmax - dysshear * Long_Float(i);
        plptex3(
            xmid, ys, zmin,
            x_inclination, y_inclination, z_inclination,
            x_shear, y_shear, z_shear,
            0.5, "shear for z = z#dmin#u");
    end loop;

    -- Draw minimal 3D grid to finish defining the 3D box.
    plmesh(x, y, z, DRAW_LINEXY);

    -- Page 4: Demonstrate drawing a string on a 3D path.
    pladv(0);
    plvpor(-0.15, 1.15, -0.05, 1.05);
    plwind(-1.2, 1.2, -0.8, 1.5);
    plw3d(1.0, 1.0, 1.0, xmin, xmax, ymin, ymax, zmin, zmax, 40.0, -30.0);

    plcol0(2);
    plbox3("b", "", xmax-xmin, 0, "b", "", ymax-ymin, 0, "bcd", "", zmax-zmin, 0);

    plschr(0.0, 1.2);

    -- domega controls the spacing between the various characters of the
    -- string and also the maximum value of omega for the given number
    -- of characters in pstring.
    domega := 2.0 * pi / Long_Float(pstring'length);
    omega := 0.0;

    -- 3D function is a helix of the given radius and pitch
    radius := 0.5;
    pitch := 1.0 / (2.0 * pi);
    for i in pstring'range loop
        sin_omega := sin(omega);
        cos_omega := cos(omega);
        xpos := xmid + radius * sin_omega;
        ypos := ymid - radius * cos_omega;
        zpos := zmin + pitch * omega;

        -- In general, the inclination is proportional to the derivative of
        -- the position wrt theta.
        x_inclination := radius * cos_omega;
        y_inclination := radius * sin_omega;
        z_inclination := pitch;

        -- The shear vector should be perpendicular to the 3D line with Z
        -- component maximized, but for low pitch a good approximation is
        -- a constant vector that is parallel to the Z axis.
        x_shear := 0.0;
        y_shear := 0.0;
        z_shear := 1.0;
        plstring(1) := pstring(i);
        plptex3(
            xpos, ypos, zpos,
            x_inclination, y_inclination, z_inclination,
            x_shear, y_shear, z_shear,
            0.5, plstring);
        omega := omega + domega;
    end loop;

    -- Draw minimal 3D grid to finish defining the 3D box.
    plmesh(x, y, z, DRAW_LINEXY);

    -- Page 5: Demonstrate plmtex3 axis labelling capability.
    pladv(0);
    plvpor(-0.15, 1.15, -0.05, 1.05);
    plwind(-1.2, 1.2, -0.8, 1.5);
    plw3d(1.0, 1.0, 1.0, xmin, xmax, ymin, ymax, zmin, zmax, 20.0, 45.0);

    plcol0(2);
    plbox3("b", "", xmax - xmin, 0, "b", "", ymax - ymin, 0, "bcd", "", zmax - zmin, 0);

    plschr(0.0, 1.0);
    plmtex3("xp",  3.0, 0.5, 0.5, "Arbitrarily displaced");
    plmtex3("xp",  4.5, 0.5, 0.5, "primary X-axis label");
    plmtex3("xs", -2.5, 0.5, 0.5, "Arbitrarily displaced");
    plmtex3("xs", -1.0, 0.5, 0.5, "secondary X-axis label");
    plmtex3("yp",  3.0, 0.5, 0.5, "Arbitrarily displaced");
    plmtex3("yp",  4.5, 0.5, 0.5, "primary Y-axis label");
    plmtex3("ys", -2.5, 0.5, 0.5, "Arbitrarily displaced");
    plmtex3("ys", -1.0, 0.5, 0.5, "secondary Y-axis label");
    plmtex3("zp",  4.5, 0.5, 0.5, "Arbitrarily displaced");
    plmtex3("zp",  3.0, 0.5, 0.5, "primary Z-axis label");
    plmtex3("zs", -2.5, 0.5, 0.5, "Arbitrarily displaced");
    plmtex3("zs", -1.0, 0.5, 0.5, "secondary Z-axis label");

    -- Draw minimal 3D grid to finish defining the 3D box.
    plmesh(x, y, z, DRAW_LINEXY);

    -- Clean up.
    plend;
end xtraditional28a;
