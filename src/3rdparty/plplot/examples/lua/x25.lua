--[[
	Filling and clipping polygons.

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
-- Test program for filling polygons and proper clipping
--------------------------------------------------------------------------

xextreme = {}
yextreme ={}
x0 = {}
y0 = {}

-- Parse and process command line arguments
pl.parseopts(arg, pl.PL_PARSE_FULL)

-- Initialize plplot
pl.ssub(3, 3)
pl.init()

xextreme = { { -120, 120 }, { -120, 120 }, { -120, 120 }, { -80,  80 }, { -220, -120 },
             {  -20,  20 }, {  -20,  20 }, {  -80,  80 }, {  20, 120 } }

yextreme = { { -120, 120 }, {   20, 120 }, {  -20, 120 }, {  -20, 120 }, { -120, 120 },
             { -120, 120 }, {  -20,  20 }, {  -80,  80 }, { -120, 120 } }

for k = 1, 2 do
for j = 1, 4 do
  if j==1 then
    -- Polygon 1: a diamond
    x0 = { 0, -100, 0, 100 }
    y0 = { -100, 0, 100, 0}
  end
  if j==2 then
    -- Polygon 1: a diamond - reverse direction
    x0 = { 100, 0, -100, 0 }
    y0 = { 0, 100, 0, -100}
  end
  if j==3 then
    -- Polygon 2: a square with punctures
    x0 = { -100, -100, 80, -100, -100, -80, 0, 80, 100, 100 }
    y0 = { -100, -80, 0, 80, 100, 100, 80, 100, 100, -100}
  end
  if j==4 then
    -- Polygon 2: a square with punctures - reversed direction
    x0 = { 100, 100, 80, 0, -80, -100, -100, 80, -100, -100 }
    y0 = { -100, 100, 100, 80, 100, 100, 80, 0, -80, -100}
  end

  for i = 1, 9 do
    pl.adv(0)
    pl.vsta()
    pl.wind(xextreme[i][1], xextreme[i][2], yextreme[i][1], yextreme[i][2])

    pl.col0(2)
    pl.box("bc", 1, 0, "bcnv", 10, 0)
    pl.col0(1)
    pl.psty(0)
    if k==1 then
      pl.fill(x0, y0)
    else
      pl.gradient(x0, y0, 45.)
    end
    pl.col0(2)
    pl.lsty(1)
    pl.line(x0, y0)
  end
end
end

-- Don't forget to call plend() to finish off!
pl.plend()
