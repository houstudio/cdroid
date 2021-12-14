--[[
	plshade demo, using color fill.

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

-- Fundamental settings.  See notes[] for more info.
ns = 20		-- Default number of shade levels
nx = 35		-- Default number of data points in x
ny = 46		-- Default number of data points in y
exclude = 0   -- By default do not plot a page illustrating
              -- exclusion.  API is probably going to change
              -- anyway, and cannot be reproduced by any
				      -- front end other than the C one.

-- polar plot data
PERIMETERPTS = 100

-- Transformation function
tr = {}

function mypltr(x, y)
	tx = tr[1] * x + tr[2] * y + tr[3]
	ty = tr[4] * x + tr[5] * y + tr[6]

	return tx, ty
end

----------------------------------------------------------------------------
-- f2mnmx
--
-- Returns min & max of input 2d array.
----------------------------------------------------------------------------
function f2mnmx(f, nx, ny)
  fmax = f[1][1]
  fmin = fmax

  for i = 1, nx do
    for j = 1, ny do
      fmax = math.max(fmax, f[i][j])
      fmin = math.min(fmin, f[i][j])
    end
  end

  return fmin, fmax
end


function zdefined(x, y)
  z = math.sqrt(x^2 + y^2)

  return z<0.4 or z>0.6
end

-- return single bit (for OR)
function bit(x,b)
  return ((x % 2^b) - (x % 2^(b-1)) > 0)
end

-- logic OR for number values
function lor(x,y)
  result = 0
  for p=1,16 do result = result + (((bit(x,p) or bit(y,p)) == true) and 2^(p-1) or 0) end
  return result
end


----------------------------------------------------------------------------
-- main
--
-- Does several shade plots using different coordinate mappings.
----------------------------------------------------------------------------

px = {}
py = {}

fill_width = 2.
cont_color = 0
cont_width = 0.

axis_opts = { "bcvtm" }
num_values = {}
values = {}
axis_ticks = { 0.0 }
axis_subticks = { 0 }
label_opts = { pl.PL_COLORBAR_LABEL_BOTTOM }
labels = { "Magnitude" }

-- Parse and process command line arguments
pl.parseopts(arg, pl.PL_PARSE_FULL)

-- Load colour palettes
pl.spal0("cmap0_black_on_white.pal");
pl.spal1("cmap1_gray.pal",1);

-- Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
pl.scmap0n(3)

-- Initialize plplot
pl.init()

-- Set up transformation function
tr = { 2/(nx-1), 0, -1, 0, 2/(ny-1), -1 }

-- Allocate data structures
clevel = {}
shedge = {}
z = {}
w = {}

-- Set up data array
for i = 1, nx do
	x = (i-1 - math.floor(nx/2))/math.floor(nx/2)
  z[i] = {}
  w[i] = {}
	for j = 1, ny do
    y = (j-1 - math.floor(ny/2))/math.floor(ny/2)-1
    z[i][j] = -math.sin(7*x) * math.cos(7*y) + x^2 - y^2
    w[i][j] = -math.cos(7*x) * math.sin(7*y) + 2*x*y
	end
end

zmin, zmax = f2mnmx(z, nx, ny)
for i = 1, ns do
	clevel[i] = zmin + (zmax-zmin)*(i-0.5)/ns
end

for i = 1, ns+1 do
	shedge[i] = zmin + (zmax-zmin)*(i-1)/ns
end

-- Set up coordinate grids
cgrid1 = {}
cgrid1["xg"] = {}
cgrid1["yg"] = {}
cgrid1["nx"] = nx
cgrid1["ny"] = ny

cgrid2 = {}
cgrid2["xg"] = {}
cgrid2["yg"] = {}
cgrid2["nx"] = nx
cgrid2["ny"] = ny

for i = 1, nx do
  cgrid2["xg"][i] = {}
  cgrid2["yg"][i] = {}
	for j = 1, ny do
    x, y = mypltr(i-1, j-1)

    argx = x*math.pi/2
    argy = y*math.pi/2
    distort = 0.4

    cgrid1["xg"][i] = x + distort * math.cos(argx)
    cgrid1["yg"][j] = y - distort * math.cos(argy)

    cgrid2["xg"][i][j] = x + distort * math.cos(argx) * math.cos(argy)
    cgrid2["yg"][i][j] = y - distort * math.cos(argx) * math.cos(argy)
  end
end

-- Plot using identity transform
pl.adv(0)
pl.vpor(0.1, 0.9, 0.1, 0.9)
pl.wind(-1, 1, -1, 1)

pl.psty(0)

pl.shades(z, -1, 1, -1, 1, shedge, fill_width, cont_color, cont_width, 1)

-- Smaller text
pl.schr( 0.0, 0.75 )
-- Small ticks on the vertical axis
pl.smaj( 0.0, 0.5 )
pl.smin( 0.0, 0.5 )

num_values[1] = ns + 1
values[1] = shedge
colorbar_width, colorbar_height = pl.colorbar( lor(pl.PL_COLORBAR_SHADE, pl.PL_COLORBAR_SHADE_LABEL), 0, 0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0, cont_color, cont_width, label_opts, labels, axis_opts, axis_ticks, axis_subticks, num_values, values )

-- Reset text and tick sizes
pl.schr( 0.0, 1.0 )
pl.smaj( 0.0, 1.0 )
pl.smin( 0.0, 1.0 )

pl.col0(1)
pl.box("bcnst", 0, 0, "bcnstv", 0, 0)
pl.col0(2)

--pl.cont(w, 1, nx, 1, ny, clevel, mypltr, {})
pl.lab("distance", "altitude", "Bogon density")

-- Plot using 1d coordinate transform

-- Load colour palettes
pl.spal0("cmap0_black_on_white.pal");
pl.spal1("cmap1_blue_yellow.pal",1);

-- Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
pl.scmap0n(3);

pl.adv(0)
pl.vpor(0.1, 0.9, 0.1, 0.9)
pl.wind(-1, 1, -1, 1)

pl.psty(0)

pl.shades(z, -1, 1, -1, 1, shedge, fill_width, cont_color, cont_width, 1, "pltr1", cgrid1)

-- Smaller text
pl.schr( 0.0, 0.75 )
-- Small ticks on the vertical axis
pl.smaj( 0.0, 0.5 )
pl.smin( 0.0, 0.5 )

num_values[1] = ns + 1
values[1] = shedge

colorbar_width, colorbar_height = pl.colorbar( lor(pl.PL_COLORBAR_SHADE, pl.PL_COLORBAR_SHADE_LABEL), 0, 0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0, cont_color, cont_width, label_opts, labels, axis_opts, axis_ticks, axis_subticks, num_values, values )

-- Reset text and tick sizes
pl.schr( 0.0, 1.0 )
pl.smaj( 0.0, 1.0 )
pl.smin( 0.0, 1.0 )

pl.col0(1)
pl.box("bcnst", 0, 0, "bcnstv", 0, 0)
pl.col0(2)
pl.lab("distance", "altitude", "Bogon density")

-- Plot using 2d coordinate transform

-- Load colour palettes
pl.spal0("cmap0_black_on_white.pal");
pl.spal1("cmap1_blue_red.pal",1);

-- Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
pl.scmap0n(3);

pl.adv(0)
pl.vpor(0.1, 0.9, 0.1, 0.9)
pl.wind(-1, 1, -1, 1)

pl.psty(0)

pl.shades(z, -1, 1, -1, 1, shedge, fill_width, cont_color, cont_width, 0, "pltr2", cgrid2)

-- Smaller text
pl.schr( 0.0, 0.75 )
-- Small ticks on the vertical axis
pl.smaj( 0.0, 0.5 )
pl.smin( 0.0, 0.5 )

num_values[1] = ns + 1
values[1]     = shedge
colorbar_width, colorbar_height = pl.colorbar( lor(pl.PL_COLORBAR_SHADE, pl.PL_COLORBAR_SHADE_LABEL), 0, 0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0, cont_color, cont_width, label_opts, labels, axis_opts, axis_ticks, axis_subticks, num_values, values )

-- Reset text and tick sizes
pl.schr( 0.0, 1.0 )
pl.smaj( 0.0, 1.0 )
pl.smin( 0.0, 1.0 )

pl.col0(1)
pl.box("bcnst", 0, 0, "bcnstv", 0, 0)
pl.col0(2)
pl.cont(w, 1, nx, 1, ny, clevel, "pltr2", cgrid2)

pl.lab("distance", "altitude", "Bogon density, with streamlines")

-- Plot using 2d coordinate transform

-- Load colour palettes
pl.spal0("");
pl.spal1("",1);

-- Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
pl.scmap0n(3);

pl.adv(0)
pl.vpor(0.1, 0.9, 0.1, 0.9)
pl.wind(-1, 1, -1, 1)

pl.psty(0)

pl.shades(z, -1, 1, -1, 1, shedge, fill_width, 2, 3., 0, "pltr2", cgrid2)

-- Smaller text
pl.schr( 0.0, 0.75 )
-- Small ticks on the vertical axis
pl.smaj( 0.0, 0.5 )
pl.smin( 0.0, 0.5 )

num_values[1] = ns + 1
values[1]     = shedge
colorbar_width, colorbar_height = pl.colorbar( lor(pl.PL_COLORBAR_SHADE, pl.PL_COLORBAR_SHADE_LABEL), 0, 0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0, 2, 3., label_opts, labels, axis_opts, axis_ticks, axis_subticks, num_values, values )

-- Reset text and tick sizes
pl.schr( 0.0, 1.0 )
pl.smaj( 0.0, 1.0 )
pl.smin( 0.0, 1.0 )

pl.col0(1)
pl.box("bcnst", 0, 0, "bcnstv", 0, 0)
pl.col0(2)

pl.lab("distance", "altitude", "Bogon density")

-- Note this exclusion API will probably change.

-- Plot using 2d coordinate transform and exclusion
if exclude~=0 then

	-- Load colour palettes
  pl.spal0("cmap0_black_on_white.pal");
  pl.spal1("cmap1_gray.pal",1);

	-- Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
  pl.scmap0n(3);

  pl.adv(0)
  pl.vpor(0.1, 0.9, 0.1, 0.9)
  pl.wind(-1, 1, -1, 1)

  plpsty(0)

  pl.shades(z, zdefined, -1, 1, -1, 1, shedge, fill_width, cont_color, cont_width,
            0, "pltr2", cgrid2)

  pl.col0(1)
  pl.box("bcnst", 0, 0, "bcnstv", 0, 0)

  pl.lab("distance", "altitude", "Bogon density with exclusion")
end

-- Example with polar coordinates.

-- Load colour palettes
pl.spal0("cmap0_black_on_white.pal");
pl.spal1("cmap1_gray.pal",1);

-- Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
pl.scmap0n(3);

pl.adv(0)
pl.vpor(.1, .9, .1, .9)
pl.wind(-1, 1, -1, 1)

pl.psty(0)

-- Build new coordinate matrices.
for i = 1, nx do
  r = (i-1)/(nx-1)
	for j = 1, ny do
	   t = 2*math.pi/(ny-1)*(j-1)
	   cgrid2["xg"][i][j] = r*math.cos(t)
	   cgrid2["yg"][i][j] = r*math.sin(t)
	   z[i][j] = math.exp(-r^2)*math.cos(5*math.pi*r)*math.cos(5*t)
	end
end

-- Need a new shedge to go along with the new data set.
zmin, zmax = f2mnmx(z, nx, ny)

for i = 1, ns+1 do
	shedge[i] = zmin + (zmax-zmin)*(i-1)/ns
end

--  Now we can shade the interior region.
pl.shades(z, -1, 1, -1, 1, shedge, fill_width, cont_color, cont_width, 0, "pltr2", cgrid2)

-- Smaller text
pl.schr( 0.0, 0.75 )
-- Small ticks on the vertical axis
pl.smaj( 0.0, 0.5 )
pl.smin( 0.0, 0.5 )

num_values[1] = ns + 1
values[1]     = shedge
colorbar_width, colorbar_height = pl.colorbar( lor(pl.PL_COLORBAR_SHADE, pl.PL_COLORBAR_SHADE_LABEL), 0, 0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0, cont_color, cont_width, label_opts, labels, axis_opts, axis_ticks, axis_subticks, num_values, values )

-- Reset text and tick sizes
pl.schr( 0.0, 1.0 )
pl.smaj( 0.0, 1.0 )
pl.smin( 0.0, 1.0 )

-- Now we can draw the perimeter.  (If do before, shade stuff may overlap.)
for i = 1, PERIMETERPTS do
   t = 2*math.pi/(PERIMETERPTS-1)*(i-1)
   px[i] = math.cos(t)
   py[i] = math.sin(t)
end
pl.col0(1)
pl.line(px, py)

-- And label the plot.
pl.col0(2)
pl.lab( "", "",  "Tokamak Bogon Instability" )

pl.plend()
