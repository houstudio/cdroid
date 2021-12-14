--[[
	Histogram demo.

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

--------------------------------------------------------------------------
-- main
--
-- Draws a histogram from sample data.
--------------------------------------------------------------------------

NPTS=2047
data = {}

-- Parse and process command line arguments
pl.parseopts(arg, pl.PL_PARSE_FULL);

-- Initialize plplot
pl.init()

-- Fill up data points
delta = 2.0*math.pi/NPTS
for i=1, NPTS do
  data[i] = math.sin((i-1)*delta)
end

pl.col0(1)
pl.hist(data, -1.1, 1.1, 44, 0)
pl.col0(2)
pl.lab("#frValue", "#frFrequency",
         "#frPLplot Example 5 - Probability function of Oscillator")

pl.plend()
