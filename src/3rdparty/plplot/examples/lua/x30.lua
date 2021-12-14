--[[
	Alpha color values demonstration.

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

red   = {  0, 255,   0,   0 }
green = {  0,   0, 255,   0 }
blue  = {  0,   0,   0, 255 }
alpha = {  1,   1,   1,   1 }

px = { 0.1, 0.5, 0.5, 0.1 }
py = { 0.1, 0.1, 0.5, 0.5 }

pos = { 0, 1 }
rcoord = { 1, 1 }
gcoord = { 0, 0 }
bcoord = { 0, 0 }
acoord = { 0, 1 }
rev = { 0, 0 }

pl.parseopts (arg, pl.PL_PARSE_FULL);

pl.init()
pl.scmap0n(4)
pl.scmap0a(red, green, blue, alpha)

--   Page 1:
--
--   This is a series of red, green and blue rectangles overlaid
--   on each other with gradually increasing transparency.

-- Set up the window
pl.adv(0)
pl.vpor(0, 1, 0, 1)
pl.wind(0, 1, 0, 1)
pl.col0(0)
pl.box("", 1, 0, "", 1, 0)

-- Draw the boxes
for i = 0, 8 do
  icol = (i % 3) + 1

  -- Get a color, change its transparency and
  -- set it as the current color.
  r, g, b, a = pl.gcol0a(icol)
  pl.scol0a(icol, r, g, b, 1-i/9)
  pl.col0(icol)

  -- Draw the rectangle
  pl.fill(px, py)

  -- Shift the rectangles coordinates
  for j = 1, 4 do
    px[j] = px[j] + 0.5/9
    py[j] = py[j] + 0.5/9
  end
end

--   Page 2:

--   This is a bunch of boxes colored red, green or blue with a single
--   large (red) box of linearly varying transparency overlaid. The
--   overlaid box is completely transparent at the bottom and completely
--   opaque at the top.

-- Set up the window
pl.adv(0)
pl.vpor(0.1, 0.9, 0.1, 0.9)
pl.wind(0.0, 1.0, 0.0, 1.0)

-- Draw the boxes. There are 25 of them drawn on a 5 x 5 grid.
for i = 0, 4 do
  -- Set box X position
  px[1] = 0.05 + 0.2 * i
  px[2] = px[1] + 0.1
  px[3] = px[2]
  px[4] = px[1]

  -- We don't want the boxes to be transparent, so since we changed
  -- the colors transparencies in the first example we have to change
  -- the transparencies back to completely opaque.
  icol = (i % 3) + 1
  r, g, b, a = pl.gcol0a(icol)
  pl.scol0a(icol, r, g, b, 1)
  pl.col0(icol)

  for j = 0, 4 do
    -- Set box y position and draw the box.
    py[1] = 0.05 + 0.2 * j
    py[2] = py[1]
    py[3] = py[1] + 0.1
    py[4] = py[3]
    pl.fill(px, py)
  end
end

-- Create the color map with 128 colors and use plscmap1la to initialize
-- the color values with a linearly varying red transparency (or alpha)
pl.scmap1n(128)
pl.scmap1la(1, pos, rcoord, gcoord, bcoord, acoord, rev)

-- Use that cmap1 to create a transparent red gradient for the whole
-- window.
px[1] = 0.
px[2] = 1.
px[3] = 1.
px[4] = 0.

py[1] = 0.
py[2] = 0.
py[3] = 1.
py[4] = 1.

pl.gradient(px, py, 90.)

pl.plend()
