--[[
	3-d plot demo.

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


----------------------------------------------------------------------------
-- cmap1_init1
--
-- Initializes color map 1 in HLS space.
-- Basic grayscale variation from half-dark (which makes more interesting
-- looking plot compared to dark) to light.
-- An interesting variation on this:
--	s[1] = 1.0
----------------------------------------------------------------------------

function cmap1_init(gray)
  i = { 0, 1 } -- left and right boundary

  if gray ~= 0 then
    h = { 0, 0 } -- hue -- low: red (arbitrary if s=0), high: red (arbitrary if s=0)
    l = { 0.5, 1 }		-- lightness -- low: half-dark, high: light
    s = { 0, 0 }		-- minimum saturation
  else
    h = { 240, 0 } -- blue -> green -> yellow -> red
    l = { 0.6, 0.6 }
    s = { 0.8, 0.8 }
  end

  pl.scmap1n(256)
  pl.scmap1l(0, i, h, l, s)
end


----------------------------------------------------------------------------
-- main
--
-- Does a series of 3-d plots for a given data set, with different
-- viewing options in each plot.
----------------------------------------------------------------------------

-- These values must be odd, for the middle
-- of the index range to be an integer, and thus
-- to correspond to the exact floating point centre
-- of the sombrero.
XPTS = 35		-- Data points in x
YPTS = 45		-- Data points in y
LEVELS = 10

alt = { 60, 40 }
az  = { 30, -30 }

title = {
    "#frPLplot Example 8 - Alt=60, Az=30",
    "#frPLplot Example 8 - Alt=40, Az=-30"
}

clevel = {}
nlevel = LEVELS
rosen = 0

-- Parse and process command line arguments
pl.parseopts(arg, pl.PL_PARSE_FULL)

-- Initialize plplot
pl.init()

-- Allocate data structures
x = {}
y = {}
z = {}

dx = 2. / ( XPTS - 1 )
dy = 2. / ( YPTS - 1 )

for i=1, XPTS do
  x[i] = -1. + (i-1)*dx
  if rosen~=0 then x[i]=x[i]*1.5 end
end

for j=1, YPTS do
  y[j] = -1. + (j-1)*dy
  if rosen~=0 then y[j]=y[j]+0.5 end
end

for i=1, XPTS do
  xx = x[i]
  z[i]= {}
  for j=1, YPTS do
    yy = y[j]
    if rosen~=0 then
      z[i][j] = (1-xx)^2 + 100*(yy-xx^2)^2
      -- The log argument might be zero for just the right grid.
      if z[i][j] > 0 then
        z[i][j] = math.log(z[i][j])
      else
        z[i][j] = -5   -- MAXFLOAT would mess-up up the scale
      end
    else
      r = math.sqrt(xx^2 + yy^2)
      z[i][j] = math.exp(-r^2) * math.cos(2*math.pi*r)
    end
  end
end

zmax, zmin = pl.MinMax2dGrid(z)
step = (zmax-zmin)/(nlevel+1)
for i=1, nlevel do
  clevel[i] = zmin + step + step*(i-1)
end

indexxmin = 0
indexxmax = XPTS
-- Temporary test values.
--indexxmin = 5
--indexxmax = XPTS-5
zlimited = {}
indexymin = {}
indexymax = {}
-- Parameters of ellipse that limits the data.
x0 = 0.5*(XPTS - 1)
a = 0.9*x0
y0 = 0.5*(YPTS - 1)
b = 0.7*y0

-- Lua calls to PLplot only work if every value
-- in a sequence is defined from index 1 to n.
-- So we do that especially for Lua below, but we
-- define zlimited to a very large value in the
-- region that should be ignored just to test that
-- it is really ignored.

-- These values should all be ignored in C because of the i index range,
-- but they have to be defined in Lua to get the correct extent of the arrays
-- passed to C.
for i = 1, indexxmin do
    zlimited[i] = {}
    indexymin[i] = 0
    indexymax[i] = YPTS
    for j = indexymin[i]+1, indexymax[i] do
        zlimited[i][j] = 1.e300
    end
end

--print("XPTS =", XPTS);
--print("x0 =", x0);
--print("a =", a);
--print("YPTS =", YPTS);
--print("y0 =", y0);
--print("b =", b);


for i = indexxmin+1, indexxmax do
    zlimited[i] = {}
    square_root = math.sqrt(1. - math.min(1., ((i - 1 - x0)/a)^2))
    -- Add 0.5 to find nearest integer and therefore preserve symmetry
    -- with regard to lower and upper bound of y range.
    indexymin[i] = math.max(0, math.floor(0.5 + y0 - b*square_root))
    -- indexymax calculated with the convention that it is 1
    -- greater than highest valid index.
    indexymax[i] = math.min(YPTS, 1 + math.floor(0.5 + y0 + b*square_root))

    --print("i, b*square_root, indexymin[i], YPTS - indexymax[i] =", i, b*square_root, indexymin[i], YPTS - indexymax[i])
    -- These values should all be ignored in C because of the j index range.
    for j = 1, indexymin[i] do
        zlimited[i][j] = 1.e300
    end
    -- Only define zlimited in a limited elliptical range.
    for j = indexymin[i]+1, indexymax[i] do
        zlimited[i][j] = z[i][j]
    end
    -- These values should all be ignored in C because of the j index range.
    for j = indexymax[i]+1, YPTS do
        zlimited[i][j] = 1.e300
    end
end

-- In C indexxmax is defined by the extent of the
-- indexxymin and indexymax arrays so comment out that part
-- of the loop to not extend those arrays.  Still extend
-- zlimited however so that it is consistent with length
-- of x and y.
for i = indexxmax+1, XPTS do
    zlimited[i] = {}
--    indexymin[i] = 0
--    indexymax[i] = YPTS
    for j = 1, YPTS do
        zlimited[i][j] = 1.e300
    end
end

pl.lightsource(1, 1, 1)

for k=1, 2 do
  for ifshade = 1, 5 do
    pl.adv(0)
    pl.vpor(0, 1, 0, 0.9)
    pl.wind(-1, 1, -0.9, 1.1)
    pl.col0(3)
    pl.mtex("t", 1, 0.5, 0.5, title[k])
    pl.col0(1)
    if rosen~=0 then
      pl.w3d(1, 1, 1, -1.5, 1.5, -0.5, 1.5, zmin, zmax, alt[k], az[k])
    else
      pl.w3d(1, 1, 1, -1, 1, -1, 1, zmin, zmax, alt[k], az[k])
    end

    pl.box3("bnstu", "x axis", 0, 0,
           "bnstu", "y axis", 0, 0,
           "bcdmnstuv", "z axis", 0, 0)
    pl.col0(2)

    if ifshade==1 then -- diffuse light surface plot
      cmap1_init(1)
      pl.surf3d(x, y, z, 0, clevel)
    end

    if ifshade==2 then -- magnitude colored plot
      cmap1_init(0)
      pl.surf3d(x, y, z, pl.MAG_COLOR, {})
    end

    if ifshade==3 then --  magnitude colored plot with faceted squares
      cmap1_init(0)
      pl.surf3d(x, y, z, lor(pl.MAG_COLOR, pl.FACETED), {})
    end

    if ifshade==4 then  -- magnitude colored plot with contours
      cmap1_init(0)
      pl.surf3d(x, y, z, lor(lor(pl.MAG_COLOR, pl.SURF_CONT), pl.BASE_CONT), clevel)
    end

    if ifshade==5 then  -- magnitude colored plot with contours and index limits
      cmap1_init(0)
      pl.surf3dl(x, y, zlimited, lor(lor(pl.MAG_COLOR, pl.SURF_CONT), pl.BASE_CONT), clevel, indexxmin, indexymin, indexymax)
    end

  end
end

-- Clean up
pl.plend()
