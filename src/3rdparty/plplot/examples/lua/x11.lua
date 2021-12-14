--[[
	Mesh plot demo.

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

XPTS = 35		-- Data points in x
YPTS = 46		-- Data points in y
LEVELS = 10

opt = { pl.DRAW_LINEXY, pl.DRAW_LINEXY }

alt = { 33, 17 }
az  = { 24, 115 }

title = {
    "#frPLplot Example 11 - Alt=33, Az=24, Opt=3",
    "#frPLplot Example 11 - Alt=17, Az=115, Opt=3" }


-- bitwise or operator from http://lua-users.org/wiki/BaseSixtyFour
-- (c) 2006-2008 by Alex Kloss
-- licensed under the terms of the LGPL2

-- return single bit (for OR)
function bit(x,b)
  return ((x % 2^b) - (x % 2^(b-1)) > 0)
end

-- logic OR for number values
function lor(x,y)
	result = 0
	for p=1,8 do result = result + (((bit(x,p) or bit(y,p)) == true) and 2^(p-1) or 0) end
	return result
end


function cmap1_init()
  i = { 0, 1 }      -- left boundary , right boundary
  h = { 240, 0 }    -- blue -> green -> yellow -> red
  l = { 0.6, 0.6 }
  s = { 0.8, 0.8 }

  pl.scmap1n(256)
  pl.scmap1l(0, i, h, l, s)
end


----------------------------------------------------------------------------
-- main
--
-- Does a series of mesh plots for a given data set, with different
-- viewing options in each plot.
----------------------------------------------------------------------------

nlevel = LEVELS
clevel = {}

-- Parse and process command line arguments
pl.parseopts(arg, pl.PL_PARSE_FULL)

-- Initialize plplot
pl.init()

x = {}
y = {}
z = {}

for i=1, XPTS do
  x[i] = 3 * (i-1-math.floor(XPTS/2)) / math.floor(XPTS/2)
end

for i=1, YPTS do
  y[i] = 3 * (i-1-math.floor(YPTS/2)) / math.floor(YPTS/2)
end

for i=1, XPTS do
  xx = x[i]
  z[i] = {}
  for j=1, YPTS do
    yy = y[j]
    z[i][j] = 3 * (1-xx)^2 * math.exp(-xx^2 - (yy+1.)^2) -
              10 * (xx/5 - xx^3 - yy^5) * math.exp(-xx^2-yy^2) -
              1/3 * math.exp(-(xx+1)^2 - yy^2)

    -- Jungfraujoch/Interlaken
    if false then
      if z[i][j] < -1 then z[i][j] = -1 end
    end
  end
end

zmax, zmin = pl.MinMax2dGrid(z)
step = (zmax - zmin)/(nlevel+1)
for i=1, nlevel do
  clevel[i] = zmin + step + step*(i-1)
end

cmap1_init()
for k=1, 2 do
  for i=1, 4 do
    pl.adv(0)
    pl.col0(1)
    pl.vpor(0, 1, 0, 0.9)
    pl.wind(-1, 1, -1, 1.5)
    pl.w3d(1, 1, 1.2, -3, 3, -3, 3, zmin, zmax, alt[k], az[k])
    pl.box3("bnstu", "x axis", 0, 0,
            "bnstu", "y axis", 0, 0,
            "bcdmnstuv", "z axis", 0, 4)

    pl.col0(2)

    -- wireframe plot
    if i==1 then
      pl.mesh(x, y, z, opt[k])
    end

    -- magnitude colored wireframe plot
    if i==2 then
      pl.mesh(x, y, z, lor(opt[k], pl.MAG_COLOR))
    end

    -- magnitude colored wireframe plot with sides
    if i==3 then
      pl.plot3d(x, y, z, lor(opt[k], pl.MAG_COLOR), 1)
    end

    -- magnitude colored wireframe plot with base contour
    if i==4 then
      pl.meshc(x, y, z, lor(lor(opt[k], pl.MAG_COLOR), pl.BASE_CONT), clevel)
    end

    pl.col0(3)
    pl.mtex("t", 1, 0.5, 0.5, title[k])
  end
end

-- Clean up

pl.plend()
