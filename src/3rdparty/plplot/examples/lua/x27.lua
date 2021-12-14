--[[
	Drawing "spirograph" curves - epitrochoids, cycolids, roulettes

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


--------------------------------------------------------------------------
-- Calculate greatest common divisor following pseudo-code for the
-- Euclidian algorithm at http://en.wikipedia.org/wiki/Euclidean_algorithm

function gcd (a,  b)
    a = math.floor(math.abs(a))
    b = math.floor(math.abs(b))
    while b~=0 do
        t = b
        b = a % b
        a = t
    end
    return a
end

function cycloid()
  -- TODO
end


function spiro( params, fill )
  NPNT = 2000
  xcoord = {}
  ycoord = {}

  -- Fill the coordinates
  -- Proper termination of the angle loop very near the beginning
  -- point, see
  -- http://mathforum.org/mathimages/index.php/Hypotrochoid.
  windings = math.floor(math.abs(params[2])/gcd(params[1], params[2]))
  steps    = math.floor(NPNT/windings)
  dphi     = 2*math.pi/steps

  for i = 1, windings*steps+1 do
    phi       = (i-1) * dphi
    phiw      = (params[1]-params[2])/params[2]*phi
    xcoord[i] = (params[1]-params[2])*math.cos(phi) + params[3]*math.cos(phiw)
    ycoord[i] = (params[1]-params[2])*math.sin(phi) - params[3]*math.sin(phiw)

    if i == 1 then
        xmin = xcoord[i]
        xmax = xcoord[i]
        ymin = ycoord[i]
        ymax = ycoord[i]
    end
    if xmin>xcoord[i] then xmin = xcoord[i] end
    if xmax<xcoord[i] then xmax = xcoord[i] end
    if ymin>ycoord[i] then ymin = ycoord[i] end
    if ymax<ycoord[i] then ymax = ycoord[i] end
  end

  xrange_adjust = 0.15*(xmax-xmin)
  xmin = xmin - xrange_adjust
  xmax = xmax + xrange_adjust
  yrange_adjust = 0.15*(ymax-ymin)
  ymin = ymin - yrange_adjust
  ymax = ymax + yrange_adjust

  pl.wind(xmin, xmax, ymin, ymax)

  pl.col0(1)
  if fill == 1 then
      pl.fill(xcoord, ycoord)
  else
      pl.line(xcoord, ycoord)
  end
end


function arcs()
    NSEG = 8

    theta = 0.0
    dtheta = 360.0 / NSEG
    pl.env( -10.0, 10.0, -10.0, 10.0, 1, 0 )

    -- Plot segments of circle in different colors
    for i = 0, NSEG-1 do
        pl.col0( i%2 + 1 )
        pl.arc(0.0, 0.0, 8.0, 8.0, theta, theta + dtheta, 0.0, 0)
        theta = theta + dtheta
    end

    -- Draw several filled ellipses inside the circle at different
    -- angles.
    a = 3.0
    b = a * math.tan( (dtheta/180.0*math.pi)/2.0 )
    theta = dtheta/2.0
    for i = 0, NSEG-1 do
        pl.col0( 2 - i%2 )
        pl.arc( a*math.cos(theta/180.0*math.pi), a*math.sin(theta/180.0*math.pi), a, b, 0.0, 360.0, theta, 1)
        theta = theta + dtheta
    end

end


----------------------------------------------------------------------------
-- main
--
-- Generates two kinds of plots:
--   - construction of a cycloid (animated)
--   - series of epitrochoids and hypotrochoids
----------------------------------------------------------------------------

-- R, r, p, N
-- R and r should be integers to give correct termination of the
-- angle loop using gcd.
-- N.B. N is just a place holder since it is no longer used
-- (because we now have proper termination of the angle loop).

params = {
  { 21,  7,  7,  3 },  -- Deltoid
  { 21,  7, 10,  3 },
  { 21, -7, 10,  3 },
  { 20,  3,  7, 20 },
  { 20,  3, 10, 20 },
  { 20, -3, 10, 20 },
  { 20, 13,  7, 20 },
  { 20, 13, 20, 20 },
  { 20,-13, 20, 20 } }

-- plplot initialization

-- Parse and process command line arguments
pl.parseopts(arg, pl.PL_PARSE_FULL)

-- Initialize plplot
pl.init()

-- Illustrate the construction of a cycloid
cycloid()

-- Loop over the various curves
-- First an overview, then all curves one by one

pl.ssub(3, 3)  -- Three by three window

fill = 0
for i = 1, 9 do
  pl.adv(0)
  pl.vpor(0, 1, 0, 1)
  spiro(params[i], fill)
end

pl.adv(0)
pl.ssub(1, 1)  -- One window per curve

for i = 1, 9 do
  pl.adv(0)
  pl.vpor(0, 1, 0, 1)
  spiro(params[i], fill)
end

-- fill the curves.
fill = 1
pl.adv(0)
pl.ssub(1, 1)  -- One window per curve

for i = 1, 9 do
  pl.adv(0)
  pl.vpor(0, 1, 0, 1)
  spiro(params[i], fill)
end

arcs()

pl.plend()
