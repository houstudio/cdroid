--[[
	Shade plot demo.

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

z = {}

-- Function prototypes

----------------------------------------------------------------------------
-- cmap1_init1
--
-- Initializes color map 1 in HLS space.
----------------------------------------------------------------------------

function cmap1_init1()
  i = { 0, 0,45, 0.55, 1 }	-- left boundary, just before center,
                            -- just after center, right boundary
  h = { 260, 260, 20, 20 }  -- hue -- low: blue-violet, only change as we go over vertex
                            -- hue -- high: red, keep fixed
  l = { 0.5, 0, 0, 0.5 }  -- lightness -- low, lightness -- center
                          -- lightness -- center, lightness -- high
  s = { 1, 1, 1, 1 }  -- maximum saturation

  pl.scmap1l(0, i, h, l, s)
end


----------------------------------------------------------------------------
-- cmap1_init2
--
-- Initializes color map 1 in HLS space.
----------------------------------------------------------------------------

function cmap1_init2()
  i = { 0, 0.45, 0.55, 1 }	-- left boundary, just before center,
                            -- just after center, right boundary
  h = { 260, 260, 20, 20 }  -- hue -- low: blue-violet, only change as we go over vertex
                            -- hue -- high: red, keep fixed
  l = { 0.6, 0, 0, 0.6 }  -- lightness -- low, lightness -- center
                          -- lightness -- center, lightness -- high
  s = { 1, 0.5, 0.5, 1 }  -- saturation -- low, saturation -- center
                          -- saturation -- center, saturation -- high

  pl.scmap1l(0, i, h, l, s)
end


----------------------------------------------------------------------------
-- plot1
--
-- Illustrates a single shaded region.
----------------------------------------------------------------------------

function plot1()
  sh_cmap = 0

  pl.adv(0)
  pl.vpor(0.1, 0.9, 0.1, 0.9)
  pl.wind(-1, 1, -1, 1)

  -- Plot using identity transform
  shade_min = zmin + (zmax-zmin)*0.4
  shade_max = zmin + (zmax-zmin)*0.6
  sh_color = 7
  sh_width = 2.
  min_color = 9
  max_color = 2
  min_width = 2.
  max_width = 2.

  pl.psty(8)
  pl.shade(z, -1, 1, -1, 1, shade_min, shade_max, sh_cmap, sh_color, sh_width,
            min_color, min_width, max_color, max_width, 1)

  pl.col0(1)
  pl.box("bcnst", 0, 0, "bcnstv", 0, 0)
  pl.col0(2)
  pl.lab("distance", "altitude", "Bogon flux")
end


----------------------------------------------------------------------------
-- plot2
--
-- Illustrates multiple adjacent shaded regions, using different fill
-- patterns for each region.
----------------------------------------------------------------------------

function plot2()
  sh_cmap = 0
  min_color = 0
  min_width = 0.
  max_color = 0
  max_width = 0.

  inc = { {450}, {-450}, {0}, {900}, {300},
          {450,-450}, {0, 900}, {0, 450}, {450, -450}, {0, 900} }
  del = { {2000}, {2000}, {2000}, {2000}, {2000},
          {2000, 2000}, {2000, 2000}, {2000, 2000}, {4000, 4000}, {4000, 2000} }

  sh_width = 2.

  pl.adv(0)
  pl.vpor(0.1, 0.9, 0.1, 0.9)
  pl.wind(-1, 1, -1, 1)

  -- Plot using identity transform

  for i = 1, 10 do
    shade_min = zmin + (zmax - zmin) * (i-1)/10
    shade_max = zmin + (zmax - zmin) * i/10
    sh_color = i+5
    pl.pat(inc[i], del[i])

    pl.shade(z, -1, 1, -1, 1, shade_min, shade_max, sh_cmap, sh_color, sh_width,
             min_color, min_width, max_color, max_width, 1)
  end

  pl.col0(1)
  pl.box("bcnst", 0, 0, "bcnstv", 0, 0)
  pl.col0(2)
  pl.lab("distance", "altitude", "Bogon flux")
end


----------------------------------------------------------------------------
-- plot3
--
-- Illustrates shaded regions in 3d, using a different fill pattern for
-- each region.
----------------------------------------------------------------------------

function plot3()
  xx = { {-1, 1, 1, -1, -1}, {-1, 1, 1, -1, -1} }
  yy = { {1, 1, 0, 0, 1}, {-1, -1, 0, 0, -1} }
  zz = { {0, 0, 1, 1, 0}, {0, 0, 1, 1, 0} }

  pl.adv(0)
  pl.vpor(0.1, 0.9, 0.1, 0.9)
  pl.wind(-1, 1, -1, 1.)
  pl.w3d(1, 1, 1, -1, 1, -1, 1, 0, 1.5, 30, -40)

  -- Plot using identity transform
  pl.col0(1)
  pl.box3("bntu", "X", 0, 0, "bntu", "Y", 0, 0, "bcdfntu", "Z", 0.5, 0)
  pl.col0(2)
  pl.lab("","","3-d polygon filling")

  pl.col0(3)
  pl.psty(1)
  pl.line3(xx[1], yy[1], zz[1])
  pl.fill3(xx[1], yy[1], zz[1])
  pl.psty(2)
  pl.line3(xx[2], yy[2], zz[2])
  pl.fill3(xx[2], yy[2], zz[2])
end


----------------------------------------------------------------------------
-- f2mnmx
--
-- Returns min & max of input 2d array.
----------------------------------------------------------------------------

function f2mnmx(f, nx, ny)
	fmax = f[1][1]
	fmin = fmax

	for i=1, nx do
		for j=1, ny do
	    fmax = math.max(fmax, f[i][j])
	    fmin = math.min(fmin, f[i][j])
		end
	end

	return fmin, fmax
end

----------------------------------------------------------------------------
-- main
--
-- Does a variety of shade plots.
----------------------------------------------------------------------------

-- Parse and process command line arguments
pl.parseopts(arg, pl.PL_PARSE_FULL)

-- Set up color map 1
cmap1_init2()

-- Initialize plplot
pl.init()

-- Set up data array
for i=1, XPTS do
	xx = ((i-1) - math.floor(XPTS/2))/math.floor(XPTS/2)
  z[i] = {}
	for j = 1, YPTS do
    yy = ((j-1) - math.floor(YPTS/2))/math.floor(YPTS/2) - 1
    z[i][j] = xx^2 - yy^2 + (xx - yy)/(xx^2+yy^2 + 0.1)
	end
end
zmin, zmax = f2mnmx(z, XPTS, YPTS)

plot1()
plot2()
plot3()

pl.plend()
