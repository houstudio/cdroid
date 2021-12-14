--[[
	Demo of multiple stream/window capability (requires Tk or Tcl-DP).

	Maurice LeBrun
	IFS, University of Texas at Austin

   Copyright (C) 2009  Werner Smekal

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

xs = {}
ys = {}
space0 = {}
mark0 = {}
space1 = { 1500 }
mark1 = { 1500 }


function plot1()
  x = {}
  y = {}

  for i = 1, 60 do
    x[i] = xoff + xscale*i/60
    y[i] = yoff + yscale*x[i]^2
  end

  xmin = x[1]
  xmax = x[60]
  ymin = y[1]
  ymax = y[60]

  for i = 1, 6 do
    xs[i] = x[(i-1)*10 + 4]
    ys[i] = y[(i-1)*10 + 4]
  end

  -- Set up the viewport and window using PLENV. The range in X is
  -- 0.0 to 6.0, and the range in Y is 0.0 to 30.0. The axes are
  -- scaled separately (just = 0), and we just draw a labelled
  -- box (axis = 0).
  pl.col0(1)
  pl.env(xmin, xmax, ymin, ymax, 0, 0)
  pl.col0(6)
  pl.lab("(x)", "(y)", "#frPLplot Example 1 - y=x#u2")

  -- Plot the data points
  pl.col0(9)
  pl.poin(xs, ys, 9)

  -- Draw the line through the data
  pl.col0(4)
  pl.line(x, y)
  pl.flush()
end


function plot2()
  x = {}
  y = {}

  -- Set up the viewport and window using PLENV. The range in X is -2.0 to
  -- 10.0, and the range in Y is -0.4 to 2.0. The axes are scaled separately
  -- (just = 0), and we draw a box with axes (axis = 1).
  pl.col0(1)
  pl.env(-2, 10, -0.4, 1.2, 0, 1)
  pl.col0(2)
  pl.lab("(x)", "sin(x)/x", "#frPLplot Example 1 - Sinc Function")

  -- Fill up the arrays
  for i = 1, 100 do
    x[i] = (i-20)/6
    y[i] = 1
    if x[i]~=0 then
      y[i] = math.sin(x[i])/x[i]
    end
  end

  -- Draw the line
  pl.col0(3)
  pl.line(x, y)
  pl.flush()
end


function plot3()
  x = {}
  y = {}

  -- For the final graph we wish to override the default tick intervals, and
  -- so do not use PLENV
  pl.adv(0)

  -- Use standard viewport, and define X range from 0 to 360 degrees, Y range
  -- from -1.2 to 1.2.
  pl.vsta()
  pl.wind(0, 360, -1.2, 1.2)

  -- Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y.
  pl.col0(1)
  pl.box("bcnst", 60, 2, "bcnstv", 0.2, 2)

  -- Superimpose a dashed line grid, with 1.5 mm marks and spaces. plstyl
  -- expects a pointer!!
  pl.styl(mark1, space1)
  pl.col0(2)
  pl.box("g", 30, 0, "g", 0.2, 0)
  pl.styl(mark0, space0)

  pl.col0(3)
  pl.lab("Angle (degrees)", "sine", "#frPLplot Example 1 - Sine function")

  for i = 1, 101 do
    x[i] = 3.6 * (i-1)
    y[i] = math.sin(x[i]*math.pi/180)
  end

  pl.col0(4)
  pl.line(x, y)
  pl.flush()
end


function plot4()
  x0 = {}
  y0 = {}
  x = {}
  y = {}

  dtr = math.pi/180
  for i = 1, 361 do
    x0[i] = math.cos(dtr*(i-1))
    y0[i] = math.sin(dtr*(i-1))
  end

  -- Set up viewport and window, but do not draw box
  pl.env(-1.3, 1.3, -1.3, 1.3, 1, -2)
  for i = 1, 10 do
  	for j = 1, 361 do
	    x[j] = 0.1*i*x0[j]
	    y[j] = 0.1*i*y0[j]
    end

    -- Draw circles for polar grid
  	pl.line(x, y)
  end

  pl.col0(2)
  for i = 1, 12 do
    theta = 30.0 * (i-1)
    dx = math.cos(dtr * theta)
    dy = math.sin(dtr * theta)

    -- Draw radial spokes for polar grid
    pl.join(0, 0, dx, dy)

    -- Write labels for angle
    -- Slightly off zero to avoid floating point logic flips at 90 and 270 deg.
    if dx>=-0.00001 then
      pl.ptex(dx, dy, dx, dy, -0.15, string.format("%d", theta))
    else
      pl.ptex(dx, dy, -dx, -dy, 1.15, string.format("%d", theta))
    end
  end

  x = {}
  y = {}

  -- Draw the graph
  for i = 1, 361 do
    r = math.sin(dtr * (5*(i-1)))
    x[i] = x0[i] * r
    y[i] = y0[i] * r
  end

  pl.col0(3)
  pl.line(x, y)

  pl.col0(4)
  pl.mtex("t", 2, 0.5, 0.5, "#frPLplot Example 3 - r(#gh)=sin 5#gh")
  pl.flush()
end

-- Demonstration of contour plotting

XPTS = 35
YPTS = 46
XSPA = 2/(XPTS-1)
YSPA = 2/(YPTS-1)

tr = { XSPA, 0, -1, 0, YSPA, -1 }

function mypltr(x, y)
  tx = tr[1]*x + tr[2]*y + tr[3]
  ty = tr[4]*x + tr[5]*y + tr[6]

  return tx, ty
end

clevel = { -1, -.8, -.6, -.4, -.2, 0, .2, .4, .6, .8, 1 }

function plot5()
  mark = { 1500 }
  space = { 1500 }

  -- Set up function arrays
  z = {}
  w = {}

  for i = 1, XPTS do
  	xx = (i-1-math.floor(XPTS/2))/math.floor(XPTS/2)
    z[i] = {}
    w[i] = {}
    for j = 1, YPTS do
      yy = (j-1-math.floor(YPTS/2))/math.floor(YPTS/2)-1
      z[i][j] = xx * xx - yy * yy
      w[i][j] = 2 * xx * yy
    end
  end

  pl.env(-1, 1, -1, 1, 0, 0)
  pl.col0(2)
  pl.cont(z, 1, XPTS, 1, YPTS, clevel, "mypltr")
  pl.styl(mark, space)
  pl.col0(3)
  pl.cont(w, 1, XPTS, 1, YPTS, clevel, "mypltr")
  pl.col0(1)
  pl.lab("X Coordinate", "Y Coordinate", "Streamlines of flow")
  pl.flush()
end


----------------------------------------------------------------------------
-- main
--
-- Plots several simple functions from other example programs.
--
-- This version sends the output of the first 4 plots (one page) to two
-- independent streams.
----------------------------------------------------------------------------

-- Select either TK or DP driver and use a small window
-- Using DP results in a crash at the end due to some odd cleanup problems
-- The geometry strings MUST be in writable memory
geometry_master = "500x410+100+200"
geometry_slave  = "500x410+650+200"

-- plplot initialization
-- Parse and process command line arguments
pl.parseopts(arg, pl.PL_PARSE_FULL)

-- If valid geometry specified on command line, use it for both streams.
xp0, yp0, xleng0, yleng0, xoff0, yoff0 = pl.gpage()
valid_geometry = xleng0>0 and yleng0>0

-- Set up first stream
if valid_geometry==true then
  pl.spage(xp0, yp0, xleng0, yleng0, xoff0, yoff0)
else
  pl.setopt("geometry", geometry_master)
end

pl.ssub(2, 2)
pl.init()

driver = pl.gdev()
fam, num, bmax = pl.gfam()

print("Demo of multiple output streams via the " .. driver .." driver.")
print("Running with the second stream as slave to the first.\n")

-- Start next stream
pl.sstrm(1)

if valid_geometry==true then
  pl.spage(xp0, yp0, xleng0, yleng0, xoff0, yoff0)
else
  pl.setopt("geometry", geometry_slave)
end

-- Turn off pause to make this a slave (must follow master)
pl.spause(0)
pl.sdev(driver)
pl.sfam(fam,num,bmax)

-- Currently number of digits in format number can only be
--set via the command line option
pl.setopt("fflen", "2")
pl.init()

-- Set up the data & plot
-- Original case
pl.sstrm(0)

xscale = 6
yscale = 1
xoff = 0
yoff = 0
plot1()

-- Set up the data & plot
xscale = 1
yscale = 1e6
plot1()

-- Set up the data & plot
xscale = 1.
yscale = 1.e-6
digmax = 2
pl.syax(digmax, 0)
plot1()

-- Set up the data & plot
xscale = 1
yscale = 0.0014
yoff = 0.0185
digmax = 5
pl.syax(digmax, 0)
plot1()

-- To slave
-- The pleop() ensures the eop indicator gets lit.
pl.sstrm(1)
plot4()
pl.eop()

-- Back to master
pl.sstrm(0)
plot2()
plot3()

-- To slave
pl.sstrm(1)
plot5()
pl.eop()

-- Back to master to wait for user to advance
pl.sstrm(0)
pl.eop()

-- Call plend to finish off.
pl.plend()
