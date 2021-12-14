--[[
	Grid data demo

   Copyright (C) 200  Werner Smekal

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

-- Options data structure definition.
pts = 500
xp = 25
yp = 20
nl = 16
knn_order = 20
threshold = 1.001
wmin = -1e3
randn = 0
rosen = 0


function cmap1_init()
  i = { 0, 1 } -- left and right boundary

  h = { 240, 0 } -- blue -> green -> yellow -> red
  l = { 0.6, 0.6 }
  s = { 0.8, 0.8 }

  pl.scmap1n(256)
  pl.scmap1l(0, i, h, l, s)
end


function create_grid(px,  py)
  local x = {}
  local y = {}

  for i = 1, px do
    x[i] = xm + (xM-xm)*(i-1)/(px-1)
  end

  for i = 1, py do
    y[i] = ym + (yM-ym)*(i-1)/(py-1)
  end

  return x, y
end


function create_data(pts)
  local x = {}
  local y = {}
  local z = {}

  for i = 1, pts do
    xt = (xM-xm)*pl.randd()
    yt = (yM-ym)*pl.randd()
    if randn==0 then
      x[i] = xt + xm
      y[i] = yt + ym
    else  -- std=1, meaning that many points are outside the plot range
      x[i] = math.sqrt(-2*math.log(xt)) * math.cos(2*math.pi*yt) + xm
      y[i] = math.sqrt(-2*math.log(xt)) * math.sin(2*math.pi*yt) + ym
    end
    if rosen==0 then
      r = math.sqrt(x[i]^2 + y[i]^2)
      z[i] = math.exp(-r^2) * math.cos(2*math.pi*r)
    else
      z[i] = math.log((1-x[i])^2 + 100*(y[i] - x[i]^2)^2)
    end
  end

  return x, y, z
end


title = { "Cubic Spline Approximation",
          "Delaunay Linear Interpolation",
          "Natural Neighbors Interpolation",
          "KNN Inv. Distance Weighted",
          "3NN Linear Interpolation",
          "4NN Around Inv. Dist. Weighted" }



xm = -0.2
ym = -0.2
xM = 0.6
yM = 0.6

pl.parseopts(arg, pl.PL_PARSE_FULL)

opt = { 0, 0, wmin, knn_order, threshold, 0 }

-- Initialize plplot
pl.init()

cmap1_init()

-- Initialise random number generator
pl.seed(5489)

x, y, z =  create_data(pts) -- the sampled data
zmin = z[1]
zmax = z[1]
for i=2, pts do
  if z[i]>zmax then zmax = z[i] end
  if z[i]<zmin then zmin = z[i] end
end

xg, yg = create_grid(xp, yp) -- grid the data at
clev = {}
xx = {}
yy = {}

pl.col0(1)
pl.env(xm, xM, ym, yM, 2, 0)
pl.col0(15)
pl.lab("X", "Y", "The original data sampling")
for i=1, pts do
  pl.col1( (z[i]-zmin)/(zmax-zmin) )
  xx[1] = x[i]
  yy[1] = y[i]
  pl.string( xx, yy, "#(727)" )
end
pl.adv(0)

pl.ssub(3, 2)

for k = 1, 2 do
  pl.adv(0)
  for alg=1, 6 do
    zg = pl.griddata(x, y, z, xg, yg, alg, opt[alg])

    --[[
       - CSA can generate NaNs (only interpolates?!).
       - DTLI and NNI can generate NaNs for points outside the convex hull
         of the data points.
       - NNLI can generate NaNs if a sufficiently thick triangle is not found

       PLplot should be NaN/Inf aware, but changing it now is quite a job...
       so, instead of not plotting the NaN regions, a weighted average over
       the neighbors is done. --]]


    if alg==pl.GRID_CSA or alg==pl.GRID_DTLI or alg==pl.GRID_NNLI or alg==pl.GRID_NNI then
      for i = 1, xp do
        for j = 1, yp do
          if zg[i][j]~=zg[i][j] then -- average (IDW) over the 8 neighbors
            zg[i][j] = 0
            dist = 0

            for ii=i-1, i+1 do
              if ii<=xp then
                for jj=j-1, j+1 do
                  if jj<=yp then
                    if ii>=1 and jj>=1 and zg[ii][jj]==zg[ii][jj] then
                      if (math.abs(ii-i) + math.abs(jj-j)) == 1 then
                        d = 1
                      else
                        d = 1.4142
                      end
                      zg[i][j] = zg[i][j] + zg[ii][jj]/(d^2)
                      dist = dist + d
                    end
                  end
                end
              end
            end
            if dist~=0 then
              zg[i][j] = zg[i][j]/dist
            else
              zg[i][j] = zmin
            end
          end
        end
      end
    end

    lzM, lzm = pl.MinMax2dGrid(zg)

    if lzm~=lzm then lzm=zmin else lzm = math.min(lzm, zmin) end
    if lzM~=lzM then lzM=zmax else lzM = math.max(lzM, zmax) end

    -- Increase limits slightly to prevent spurious contours
    -- due to rounding errors
    lzm = lzm-0.01
    lzM = lzM+0.01

    pl.col0(1)

    pl.adv(alg)

    if k==1 then
      for i = 1, nl do
        clev[i] = lzm + (lzM-lzm)/(nl-1)*(i-1)
      end

      pl.env0(xm, xM, ym, yM, 2, 0)
      pl.col0(15)
      pl.lab("X", "Y", title[alg])
      pl.shades(zg, xm, xM, ym, yM, clev, 1., 0, 1., 1)
      pl.col0(2)
    else
      for i = 1, nl do
        clev[i] = lzm + (lzM-lzm)/(nl-1)*(i-1)
      end

      pl.vpor(0, 1, 0, 0.9)
      pl.wind(-1.1, 0.75, -0.65, 1.20)

      -- For the comparison to be fair, all plots should have the
      -- same z values, but to get the max/min of the data generated
      -- by all algorithms would imply two passes. Keep it simple.
      --
      -- pl.w3d(1, 1, 1, xm, xM, ym, yM, zmin, zmax, 30, -60)


      pl.w3d(1, 1, 1, xm, xM, ym, yM, lzm, lzM, 30, -40)
      pl.box3("bntu", "X", 0, 0,
              "bntu", "Y", 0, 0,
              "bcdfntu", "Z", 0.5, 0)
      pl.col0(15)
      pl.lab("", "", title[alg])
      pl.plot3dc(xg, yg, zg, lor(lor(pl.DRAW_LINEXY, pl.MAG_COLOR), pl.BASE_CONT), clev)
    end
  end
end

pl.plend()
