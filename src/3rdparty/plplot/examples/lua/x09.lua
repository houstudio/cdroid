--[[
	Contour plot demo.

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

XSPA = 2/(XPTS-1)
YSPA = 2/(YPTS-1)

-- polar plot data
PERIMETERPTS = 100
RPTS = 40
THETAPTS = 40

-- potential plot data
PPERIMETERPTS = 100
PRPTS = 40
PTHETAPTS = 64
PNLEVEL = 20

clevel = { -1, -0.8, -0.6, -0.4, -0.2, 0, 0.2, 0.4, 0.6, 0.8, 1}

-- Transformation function
tr = { XSPA, 0, -1, 0, YSPA, -1 }

function mypltr(x, y)
	tx = tr[1] * x + tr[2] * y + tr[3]
	ty = tr[4] * x + tr[5] * y + tr[6]

	return tx, ty
end

--polar contour plot example.
function polar()
	px = {}
	py = {}
	lev = {}

	pl.env(-1, 1, -1, 1, 0, -2)
	pl.col0(1)

	--Perimeter
	for i=1, PERIMETERPTS do
		t = (2*math.pi/(PERIMETERPTS-1))*(i-1)
		px[i] = math.cos(t)
		py[i] = math.sin(t)
  end
  pl.line(px, py)

	--create data to be contoured.
  cgrid2["xg"] = {}
  cgrid2["yg"] = {}
  cgrid2["nx"] = RPTS
  cgrid2["ny"] = THETAPTS
  z = {}

  for i = 1, RPTS do
    r = (i-1)/(RPTS-1)
    cgrid2["xg"][i] = {}
    cgrid2["yg"][i] = {}
    z[i] = {}
    for j = 1, THETAPTS do
      theta = (2*math.pi/(THETAPTS-1))*(j-1)
      cgrid2["xg"][i][j] = r*math.cos(theta)
      cgrid2["yg"][i][j] = r*math.sin(theta)
      z[i][j] = r
    end
  end

  for i = 1, 10 do
    lev[i] = 0.05 + 0.10*(i-1)
  end

  pl.col0(2)
  pl.cont(z, 1, RPTS, 1, THETAPTS, lev, "pltr2", cgrid2)
  pl.col0(1)
  pl.lab("", "", "Polar Contour Plot")
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


--shielded potential contour plot example.
function potential()
  clevelneg = {}
  clevelpos = {}
  px = {}
  py = {}

  --create data to be contoured.
  cgrid2["xg"] = {}
  cgrid2["yg"] = {}
  cgrid2["nx"] = PRPTS
  cgrid2["ny"] = PTHETAPTS
  z = {}

  for i = 1, PRPTS do
    r = 0.5 + (i-1)
    cgrid2["xg"][i] = {}
    cgrid2["yg"][i] = {}
    for j = 1, PTHETAPTS do
      theta = 2*math.pi/(PTHETAPTS-1)*(j-0.5)
      cgrid2["xg"][i][j] = r*math.cos(theta)
      cgrid2["yg"][i][j] = r*math.sin(theta)
    end
  end

  rmax = PRPTS-0.5
  xmin, xmax = f2mnmx(cgrid2["xg"], PRPTS, PTHETAPTS)
  ymin, ymax = f2mnmx(cgrid2["yg"], PRPTS, PTHETAPTS)
  x0 = (xmin + xmax)/2
  y0 = (ymin + ymax)/2

  -- Expanded limits
  peps = 0.05
  xpmin = xmin - math.abs(xmin)*peps
  xpmax = xmax + math.abs(xmax)*peps
  ypmin = ymin - math.abs(ymin)*peps
  ypmax = ymax + math.abs(ymax)*peps

  -- Potential inside a conducting cylinder (or sphere) by method of images.
  --  Charge 1 is placed at (d1, d1), with image charge at (d2, d2).
  --  Charge 2 is placed at (d1, -d1), with image charge at (d2, -d2).
  --  Also put in smoothing term at small distances.
  eps = 2
  q1 = 1
  d1 = rmax/4

  q1i = - q1*rmax/d1
  d1i = rmax^2/d1

  q2 = -1
  d2 = rmax/4

  q2i = - q2*rmax/d2
  d2i = rmax^2/d2

  for i = 1, PRPTS do
    z[i] = {}
    for j = 1, PTHETAPTS do
      div1 = math.sqrt((cgrid2.xg[i][j]-d1)^2 + (cgrid2.yg[i][j]-d1)^2 + eps^2)
      div1i = math.sqrt((cgrid2.xg[i][j]-d1i)^2 + (cgrid2.yg[i][j]-d1i)^2 + eps^2)
      div2 = math.sqrt((cgrid2.xg[i][j]-d2)^2 + (cgrid2.yg[i][j]+d2)^2 + eps^2)
      div2i = math.sqrt((cgrid2.xg[i][j]-d2i)^2 + (cgrid2.yg[i][j]+d2i)^2 + eps^2)
      z[i][j] = q1/div1 + q1i/div1i + q2/div2 + q2i/div2i
    end
  end
  zmin, zmax = f2mnmx(z, PRPTS, PTHETAPTS)

  -- Positive and negative contour levels.
  dz = (zmax-zmin)/PNLEVEL
  nlevelneg = 1
  nlevelpos = 1
  for i = 1, PNLEVEL do
    clevel = zmin + (i-0.5)*dz
    if clevel <= 0 then
      clevelneg[nlevelneg] = clevel
      nlevelneg = nlevelneg + 1
    else
      clevelpos[nlevelpos] = clevel
      nlevelpos = nlevelpos + 1
    end
  end

  -- Colours!
  ncollin = 11
  ncolbox = 1
  ncollab = 2

  -- Finally start plotting this page!
  pl.adv(0)
  pl.col0(ncolbox)

  pl.vpas(0.1, 0.9, 0.1, 0.9, 1)
  pl.wind(xpmin, xpmax, ypmin, ypmax)
  pl.box("", 0, 0, "", 0, 0)

  pl.col0(ncollin)
  if nlevelneg>1 then
    -- Negative contours
    pl.lsty(2)
    pl.cont(z, 1, PRPTS, 1, PTHETAPTS, clevelneg, "pltr2", cgrid2)
  end

  if nlevelpos>1 then
    -- Positive contours
    pl.lsty(1)
    pl.cont(z, 1, PRPTS, 1, PTHETAPTS, clevelpos, "pltr2", cgrid2)
  end

  -- Draw outer boundary
  for i = 1, PPERIMETERPTS do
    t = (2*math.pi/(PPERIMETERPTS-1))*(i-1)
    px[i] = x0 + rmax*math.cos(t)
    py[i] = y0 + rmax*math.sin(t)
  end

  pl.col0(ncolbox)
  pl.line(px, py)

  pl.col0(ncollab)
  pl.lab("", "", "Shielded potential of charges in a conducting sphere")
end


----------------------------------------------------------------------------
-- main
--
-- Does several contour plots using different coordinate mappings.
----------------------------------------------------------------------------
mark = { 1500 }
space = { 1500 }

-- Parse and process command line arguments
pl.parseopts(arg, pl.PL_PARSE_FULL)

-- Initialize plplot
pl.init()

-- Set up function arrays
z = {}
w = {}

for i = 1, XPTS do
	xx = (i-1 - math.floor(XPTS/2)) / math.floor(XPTS/2)
  z[i] = {}
  w[i] = {}
	for j = 1, YPTS do
	    yy = (j-1 - math.floor(YPTS/2)) / math.floor(YPTS/2) - 1
	    z[i][j] = xx^2 - yy^2
	    w[i][j] = 2 * xx * yy
	end
end

-- Set up grids
cgrid1 = {}
cgrid1["xg"] = {}
cgrid1["yg"] = {}
cgrid1["nx"] = XPTS
cgrid1["ny"] = YPTS
cgrid2 = {}
cgrid2["xg"] = {}
cgrid2["yg"] = {}
cgrid2["nx"] = XPTS
cgrid2["ny"] = YPTS

for i = 1, XPTS do
  cgrid2["xg"][i] = {}
  cgrid2["yg"][i] = {}
	for j = 1, YPTS do
    xx, yy = mypltr(i-1, j-1)

    argx = xx * math.pi/2
    argy = yy * math.pi/2
    distort = 0.4

    cgrid1["xg"][i] = xx + distort * math.cos(argx)
    cgrid1["yg"][j] = yy - distort * math.cos(argy)

    cgrid2["xg"][i][j] = xx + distort * math.cos(argx) * math.cos(argy)
    cgrid2["yg"][i][j] = yy - distort * math.cos(argx) * math.cos(argy)
  end
end

-- Plot using identity transform
pl.setcontlabelformat(4, 3)
pl.setcontlabelparam(0.006, 0.3, 0.1, 1)
pl.env(-1, 1, -1, 1, 0, 0)
pl.col0(2)
pl.cont(z, 1, XPTS, 1, YPTS, clevel, "mypltr")
pl.styl(mark, space)
pl.col0(3)
pl.cont(w, 1, XPTS, 1, YPTS, clevel, "mypltr")
pl.styl({}, {})
pl.col0(1)
pl.lab("X Coordinate", "Y Coordinate", "Streamlines of flow")
pl.setcontlabelparam(0.006, 0.3, 0.1, 0)

-- Plot using 1d coordinate transform
pl.env(-1, 1, -1, 1, 0, 0)
pl.col0(2)
pl.cont(z, 1, XPTS, 1, YPTS, clevel, "pltr1", cgrid1)
pl.styl(mark, space)
pl.col0(3)
pl.cont(w, 1, XPTS, 1, YPTS, clevel, "pltr1", cgrid1)
pl.styl({}, {})
pl.col0(1)
pl.lab("X Coordinate", "Y Coordinate", "Streamlines of flow")

-- Plot using 2d coordinate transform
pl.env(-1, 1, -1, 1, 0, 0)
pl.col0(2)
pl.cont(z, 1, XPTS, 1, YPTS, clevel, "pltr2", cgrid2)

pl.styl(mark, space)
pl.col0(3)
pl.cont(w, 1, XPTS, 1, YPTS, clevel, "pltr2", cgrid2)
pl.styl({}, {})
pl.col0(1)
pl.lab("X Coordinate", "Y Coordinate", "Streamlines of flow")

pl.setcontlabelparam(0.006, 0.3, 0.1, 0)
polar()

pl.setcontlabelparam(0.006, 0.3, 0.1, 0)
potential()

-- Clean up
pl.plend()
