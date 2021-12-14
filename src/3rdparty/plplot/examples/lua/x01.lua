--[[
	Simple line plot and multiple windows demo.

  Copyright (C) 2008  Werner Smekal

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

-- Variables and data arrays used by plot generators
x = {}
y = {}
xs = {}
ys = {}

fontset = 1
f_name = ""


function plot1(do_test)
  for i = 1, 60 do
    x[i] = xoff + xscale * (i) / 60
    y[i] = yoff + yscale * x[i]^2
  end

  xmin = x[1]
  xmax = x[60]
  ymin = y[1]
  ymax = y[60]

  for i = 1, 6 do
    xs[i] = x[(i-1) * 10 + 4]
    ys[i] = y[(i-1) * 10 + 4]
  end

  -- Set up the viewport and window using PLENV. The range in X is
  -- 0.0 to 6.0, and the range in Y is 0.0 to 30.0. The axes are
  -- scaled separately (just = 0), and we just draw a labelled
  -- box (axis = 0).
  pl.col0(1)
  pl.env(xmin, xmax, ymin, ymax, 0, 0)
  pl.col0(2)
  pl.lab("(x)", "(y)", "#frPLplot Example 1 - y=x#u2")

  -- Plot the data points
  pl.col0(4)
  pl.poin(xs, ys, 9)

  -- Draw the line through the data
  pl.col0(3)
  pl.line(x, y)
end


function plot2()
  -- Set up the viewport and window using PLENV. The range in X is -2.0 to
  -- 10.0, and the range in Y is -0.4 to 2.0. The axes are scaled separately
  --(just = 0), and we draw a box with axes (axis = 1).
  pl.col0(1)
  pl.env(-2, 10, -0.4, 1.2, 0, 1)
  pl.col0(2)
  pl.lab("(x)", "sin(x)/x", "#frPLplot Example 1 - Sinc Function")

  -- Fill up the arrays
  for i=1, 100 do
    x[i] = (i - 20) / 6
    y[i] = 1
    if x[i] ~= 0 then y[i] = math.sin(x[i])/x[i] end
  end

  -- Draw the line
  pl.col0(3)
  pl.width(2)
  pl.line(x, y)
  pl.width(1)
end


function plot3()
  space0 = { }
  mark0 = { }
  space1 = { 1500 }
  mark1 = { 1500 }

  -- For the final graph we wish to override the default tick intervals, and
  --so do not use plenv().
  pl.adv(0)

  -- Use standard viewport, and define X range from 0 to 360 degrees, Y range
  --from -1.2 to 1.2.
  pl.vsta()
  pl.wind(0, 360, -1.2, 1.2)

  -- Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y.
  pl.col0(1)
  pl.box("bcnst", 60, 2, "bcnstv", 0.2, 2)

  -- Superimpose a dashed line grid, with 1.5 mm marks and spaces.
  -- plstyl expects a pointer!
  pl.styl(mark1, space1)
  pl.col0(2)
  pl.box("g", 30, 0, "g", 0.2, 0)
  pl.styl(mark0, space0)

  pl.col0(3)
  pl.lab("Angle (degrees)", "sine", "#frPLplot Example 1 - Sine function")

  for i=1, 101 do
    x[i] = 3.6*(i-1)
    y[i] = math.sin(x[i]*math.pi/180)
  end

  pl.col0(4)
  pl.line(x, y)
end


----------------------------------------------------------------------------
-- main
--
-- Generates several simple line plots.  Demonstrates:
--   - subwindow capability
--   - setting up the window, drawing plot, and labelling
--   - changing the color
--   - automatic axis rescaling to exponential notation
--   - placing the axes in the middle of the box
--   - gridded coordinate axes
----------------------------------------------------------------------------

-- plplot initialization

-- Parse and process command line arguments
pl.parseopts(arg, pl.PL_PARSE_FULL)

-- Get version number, just for kicks
ver=pl.gver()
print("PLplot library version: " .. ver)

-- Initialize plplot
-- Divide page into 2x2 plots
-- Note: calling plstar replaces separate calls to plssub and plinit
pl.star(2,2)

-- Select font set as per input flag
if fontset ~= 0 then
	pl.fontld(1)
else
	pl.fontld(0)
end

-- Set up the data
-- Original case
xscale = 6
yscale = 1
xoff = 0
yoff = 0

-- Do a plot
plot1(0)

-- Set up the data
xscale = 1
yscale = 0.0014
yoff = 0.0185

-- Do a plot
digmax = 5
pl.syax(digmax, 0)

plot1(1)
plot2()
plot3()

-- Show how to save a plot:
-- Open a new device, make it current, copy parameters,
-- and replay the plot buffer
if f_name~="" then -- command line option '-save filename'
  print("The current plot was saved in color Postscript under the name " .. f_name .. ".\n")
  cur_strm = pl.gstrm()    -- get current stream
  new_strm = pl.mkstrm()   -- create a new one

  pl.sfnam(f_name)       -- file name
  pl.sdev("psc")         -- device type

  pl.cpstrm(cur_strm, 0) -- copy old stream parameters to new stream
  pl.replot()	     -- do the save by replaying the plot buffer
  pl.plend1()              -- finish the device

  pl.sstrm(cur_strm)     -- return to previous stream
end

-- Don't forget to call plend() to finish off!
pl.plend()
