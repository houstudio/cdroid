--[[
	pl.mtex3, plptex3 demo.

   Copyright (C) 2009 Werner Smekal

  This file is part of PLplot.

  PLplot is free software you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as published
  by the Free Software Foundation either version 2 of the License, or
  (at your option) any later version.

  PLplot is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with PLplot if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
--]]


-- initialise Lua bindings for PLplot examples.
dofile("plplot_examples.lua")

-- Choose these values to correspond to tick marks.
XPTS = 2
YPTS = 2
NREVOLUTION = 16
NROTATION = 8
NSHEAR = 8

----------------------------------------------------------------------------
-- main
--
-- Demonstrates plotting text in 3D.
----------------------------------------------------------------------------

xmin=0
xmax=1
xmid = 0.5*(xmax + xmin)
xrange = xmax - xmin
ymin=0
ymax=1
ymid = 0.5*(ymax + ymin)
yrange = ymax - ymin
zmin=0
zmax=1
zmid = 0.5*(zmax + zmin)
zrange = zmax - zmin
ysmin    = ymin + 0.1 * yrange
ysmax    = ymax - 0.1 * yrange
ysrange  = ysmax - ysmin
dysrot   = ysrange / ( NROTATION - 1 )
dysshear = ysrange / ( NSHEAR - 1 )
zsmin    = zmin + 0.1 * zrange
zsmax    = zmax - 0.1 * zrange
zsrange  = zsmax - zsmin
dzsrot   = zsrange / ( NROTATION - 1 )
dzsshear = zsrange / ( NSHEAR - 1 )

pstring = "The future of our civilization depends on software freedom."

-- Allocate and define the minimal x, y, and z to insure 3D box
x = {}
y = {}
z = {}

for i = 1, XPTS do
  x[i] = xmin + (i-1) * (xmax-xmin)/(XPTS-1)
end

for j = 1, YPTS do
 y[j] = ymin + (j-1) * (ymax-ymin)/(YPTS-1)
end

for i = 1, XPTS do
  z[i] = {}
  for j = 1, YPTS do
    z[i][j] = 0
  end
end

-- Parse and process command line arguments
pl.parseopts(arg, pl.PL_PARSE_FULL)

pl.init()

-- Page 1: Demonstrate inclination and shear capability pattern.
pl.adv(0)
pl.vpor(-0.15, 1.15, -0.05, 1.05)
pl.wind(-1.2, 1.2, -0.8, 1.5)
pl.w3d(1, 1, 1, xmin, xmax, ymin, ymax, zmin, zmax, 20, 45)

pl.col0(2)
pl.box3("b", "", xmax-xmin, 0,
        "b", "", ymax-ymin, 0,
        "bcd", "", zmax-zmin, 0)

-- z = zmin.
pl.schr(0, 1)
for i = 1, NREVOLUTION do
  omega = 2*math.pi*(i-1)/NREVOLUTION
  sin_omega = math.sin(omega)
  cos_omega = math.cos(omega)
  x_inclination = 0.5*xrange*cos_omega
  y_inclination = 0.5*yrange*sin_omega
  z_inclination = 0
  x_shear = -0.5*xrange*sin_omega
  y_shear = 0.5*yrange*cos_omega
  z_shear = 0
  pl.ptex3( xmid, ymid, zmin, x_inclination, y_inclination, z_inclination,
           x_shear, y_shear, z_shear, 0, "  revolution")
end

-- x = xmax.
pl.schr(0, 1)
for i = 1, NREVOLUTION do
  omega = 2.*math.pi*(i-1)/NREVOLUTION
  sin_omega = math.sin(omega)
  cos_omega = math.cos(omega)
  x_inclination = 0.
  y_inclination = -0.5*yrange*cos_omega
  z_inclination = 0.5*zrange*sin_omega
  x_shear = 0
  y_shear = 0.5*yrange*sin_omega
  z_shear = 0.5*zrange*cos_omega
  pl.ptex3(xmax, ymid, zmid, x_inclination, y_inclination, z_inclination,
           x_shear, y_shear, z_shear, 0, "  revolution")
end

-- y = ymax.
pl.schr(0, 1)
for i = 1, NREVOLUTION do
  omega = 2.*math.pi*(i-1)/NREVOLUTION
  sin_omega = math.sin(omega)
  cos_omega = math.cos(omega)
  x_inclination = 0.5*xrange*cos_omega
  y_inclination = 0.
  z_inclination = 0.5*zrange*sin_omega
  x_shear = -0.5*xrange*sin_omega
  y_shear = 0.
  z_shear = 0.5*zrange*cos_omega
  pl.ptex3(xmid, ymax, zmid, x_inclination, y_inclination, z_inclination,
           x_shear, y_shear, z_shear, 0, "  revolution")
end

-- Draw minimal 3D grid to finish defining the 3D box.
pl.mesh(x, y, z, pl.DRAW_LINEXY)

-- Page 2: Demonstrate rotation of string around its axis.
pl.adv(0)
pl.vpor(-0.15, 1.15, -0.05, 1.05)
pl.wind(-1.2, 1.2, -0.8, 1.5)
pl.w3d(1, 1, 1, xmin, xmax, ymin, ymax, zmin, zmax, 20, 45)

pl.col0(2)
pl.box3("b", "", xmax-xmin, 0,
        "b", "", ymax-ymin, 0,
        "bcd", "", zmax-zmin, 0)

-- y = ymax.
pl.schr(0, 1)
x_inclination = 1
y_inclination = 0
z_inclination = 0
x_shear = 0
for i = 1, NROTATION do
  omega = 2.*math.pi*(i-1)/NROTATION
  sin_omega = math.sin(omega)
  cos_omega = math.cos(omega)
  y_shear = 0.5*yrange*sin_omega
  z_shear = 0.5*zrange*cos_omega
  zs = zsmax - dzsrot * (i-1)
  pl.ptex3(xmid, ymax, zs,
           x_inclination, y_inclination, z_inclination,
           x_shear, y_shear, z_shear,
           0.5, "rotation for y = y#dmax#u")
end

-- x = xmax.
pl.schr(0, 1)
x_inclination = 0
y_inclination = -1
z_inclination = 0
y_shear = 0
for i = 1, NROTATION do
  omega = 2.*math.pi*(i-1)/NROTATION
  sin_omega = math.sin(omega)
  cos_omega = math.cos(omega)
  x_shear = 0.5*xrange*sin_omega
  z_shear = 0.5*zrange*cos_omega
  zs = zsmax - dzsrot * (i-1)
  pl.ptex3(xmax, ymid, zs,
           x_inclination, y_inclination, z_inclination,
           x_shear, y_shear, z_shear,
           0.5, "rotation for x = x#dmax#u")
end

-- z = zmin.
pl.schr(0, 1)
x_inclination = 1
y_inclination = 0
z_inclination = 0
x_shear = 0
for i = 1, NROTATION do
  omega = 2.*math.pi*(i-1)/NROTATION
  sin_omega = math.sin(omega)
  cos_omega = math.cos(omega)
  y_shear = 0.5*yrange*cos_omega
  z_shear = 0.5*zrange*sin_omega
  ys = ysmax - dysrot * (i-1)
  pl.ptex3(xmid, ys, zmin,
           x_inclination, y_inclination, z_inclination,
           x_shear, y_shear, z_shear,
           0.5, "rotation for z = z#dmin#u")
end

-- Draw minimal 3D grid to finish defining the 3D box.
pl.mesh(x, y, z, pl.DRAW_LINEXY)

-- Page 3: Demonstrate shear of string along its axis.
-- Work around xcairo and pngcairo (but not pscairo) problems for
-- shear vector too close to axis of string. (N.B. no workaround
-- would be domega = 0.)
domega = 0.05
pl.adv(0)
pl.vpor(-0.15, 1.15, -0.05, 1.05)
pl.wind(-1.2, 1.2, -0.8, 1.5)
pl.w3d(1, 1, 1, xmin, xmax, ymin, ymax, zmin, zmax, 20, 45)

pl.col0(2)
pl.box3("b", "", xmax-xmin, 0,
        "b", "", ymax-ymin, 0,
        "bcd", "", zmax-zmin, 0)

-- y = ymax.
pl.schr(0, 1)
x_inclination = 1
y_inclination = 0
z_inclination = 0
y_shear = 0
for i = 1, NSHEAR do
  omega = domega + 2.*math.pi*(i-1)/NSHEAR
  sin_omega = math.sin(omega)
  cos_omega = math.cos(omega)
  x_shear = 0.5*xrange*sin_omega
  z_shear = 0.5*zrange*cos_omega
  zs = zsmax - dzsshear * (i-1)
  pl.ptex3(xmid, ymax, zs,
           x_inclination, y_inclination, z_inclination,
           x_shear, y_shear, z_shear,
           0.5, "shear for y = y#dmax#u")
end

-- x = xmax.
pl.schr(0, 1)
x_inclination = 0
y_inclination = -1
z_inclination = 0
x_shear = 0
for i = 1, NSHEAR do
  omega = domega + 2.*math.pi*(i-1)/NSHEAR
  sin_omega = math.sin(omega)
  cos_omega = math.cos(omega)
  y_shear = -0.5*yrange*sin_omega
  z_shear = 0.5*zrange*cos_omega
  zs = zsmax - dzsshear * (i-1)
  pl.ptex3(xmax, ymid, zs,
           x_inclination, y_inclination, z_inclination,
           x_shear, y_shear, z_shear,
           0.5, "shear for x = x#dmax#u")
end

-- z = zmin.
pl.schr(0, 1)
x_inclination = 1
y_inclination = 0
z_inclination = 0
z_shear = 0
for i = 1, NSHEAR do
  omega = domega + 2.*math.pi*(i-1)/NSHEAR
  sin_omega = math.sin(omega)
  cos_omega = math.cos(omega)
  y_shear = 0.5*yrange*cos_omega
  x_shear = 0.5*xrange*sin_omega
  ys = ysmax - dysshear * (i-1)
  pl.ptex3(xmid, ys, zmin,
           x_inclination, y_inclination, z_inclination,
           x_shear, y_shear, z_shear,
           0.5, "shear for z = z#dmin#u")
end

-- Draw minimal 3D grid to finish defining the 3D box.
pl.mesh(x, y, z, pl.DRAW_LINEXY)

-- Page 4: Demonstrate drawing a string on a 3D path.
pl.adv(0)
pl.vpor(-0.15, 1.15, -0.05, 1.05)
pl.wind(-1.2, 1.2, -0.8, 1.5)
pl.w3d(1, 1, 1, xmin, xmax, ymin, ymax, zmin, zmax, 40, -30)

pl.col0(2)
pl.box3("b", "", xmax-xmin, 0,
        "b", "", ymax-ymin, 0,
        "bcd", "", zmax-zmin, 0)

pl.schr(0, 1.2)
-- domega controls the spacing between the various characters of the
-- string and also the maximum value of omega for the given number
-- of characters in *pstring.
domega = 2.*math.pi/string.len(pstring)
omega = 0

-- 3D function is a helix of the given radius and pitch
radius = 0.5
pitch = 1/(2*math.pi)

for i = 1, string.len(pstring) do
  sin_omega = math.sin(omega)
  cos_omega = math.cos(omega)
  xpos = xmid + radius*sin_omega
  ypos = ymid - radius*cos_omega
  zpos = zmin + pitch*omega

  -- In general, the inclination is proportional to the derivative of
  --the position wrt theta.
  x_inclination = radius*cos_omega
  y_inclination = radius*sin_omega
  z_inclination = pitch

  -- The shear vector should be perpendicular to the 3D line with Z
  -- component maximized, but for low pitch a good approximation is
  --a constant vector that is parallel to the Z axis.
  x_shear = 0
  y_shear = 0
  z_shear = 1
  pl.ptex3(xpos, ypos, zpos, x_inclination, y_inclination, z_inclination,
           x_shear, y_shear, z_shear, 0.5, string.sub(pstring, i, i))
  omega = omega + domega
end

-- Draw minimal 3D grid to finish defining the 3D box.
pl.mesh(x, y, z, pl.DRAW_LINEXY)

-- Page 5: Demonstrate pl.mtex3 axis labelling capability
pl.adv(0)
pl.vpor(-0.15, 1.15, -0.05, 1.05)
pl.wind(-1.2, 1.2, -0.8, 1.5)
pl.w3d(1, 1, 1, xmin, xmax, ymin, ymax, zmin, zmax, 20, 45)

pl.col0(2)
pl.box3("b", "", xmax-xmin, 0,
        "b", "", ymax-ymin, 0,
        "bcd", "", zmax-zmin, 0)

pl.schr(0, 1)
pl.mtex3("xp", 3, 0.5, 0.5, "Arbitrarily displaced")
pl.mtex3("xp", 4.5, 0.5, 0.5, "primary X-axis label")
pl.mtex3("xs", -2.5, 0.5, 0.5, "Arbitrarily displaced")
pl.mtex3("xs", -1, 0.5, 0.5, "secondary X-axis label")
pl.mtex3("yp", 3, 0.5, 0.5, "Arbitrarily displaced")
pl.mtex3("yp", 4.5, 0.5, 0.5, "primary Y-axis label")
pl.mtex3("ys", -2.5, 0.5, 0.5, "Arbitrarily displaced")
pl.mtex3("ys", -1, 0.5, 0.5, "secondary Y-axis label")
pl.mtex3("zp", 4.5, 0.5, 0.5, "Arbitrarily displaced")
pl.mtex3("zp", 3, 0.5, 0.5, "primary Z-axis label")
pl.mtex3("zs", -2.5, 0.5, 0.5, "Arbitrarily displaced")
pl.mtex3("zs", -1, 0.5, 0.5, "secondary Z-axis label")

-- Draw minimal 3D grid to finish defining the 3D box.
pl.mesh(x, y, z, pl.DRAW_LINEXY)

pl.plend()
