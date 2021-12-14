--[[
	Multiple window and color map 0 demo.

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

----------------------------------------------------------------------------
-- draw_windows
--
-- Draws a set of numbered boxes with colors according to cmap0 entry.
----------------------------------------------------------------------------

function draw_windows(nw, cmap0_offset)
  pl.schr(0, 3.5)
  pl.font(4)

  for i = 0, nw-1 do
    pl.col0(i+cmap0_offset)
    pl.adv(0)
    vmin = 0.1
    vmax = 0.9
    for j = 0, 2 do
      pl.width(j + 1)
      pl.vpor(vmin, vmax, vmin, vmax)
      pl.wind(0, 1, 0, 1)
      pl.box("bc", 0, 0, "bc", 0, 0)
      vmin = vmin + 0.1
      vmax = vmax - 0.1
    end
    pl.width(1)
    pl.ptex(0.5, 0.5, 1, 0, 0.5, tostring(i))
  end
end


----------------------------------------------------------------------------
-- demo1
--
-- Demonstrates multipl.e windows and default color map 0 palette.
----------------------------------------------------------------------------

function demo1()
  pl.bop()

  -- Divide screen into 16 regions
  pl.ssub(4, 4)

  draw_windows(16, 0)

  pl.eop()
end


----------------------------------------------------------------------------
-- demo2
--
-- Demonstrates multipl.e windows, user-modified color map 0 palette, and
-- HLS -> RGB translation.
----------------------------------------------------------------------------

function demo2()
  -- Set up cmap0
  -- Use 100 custom colors in addition to base 16
  r = {}
  g = {}
  b = {}

  -- Min & max lightness values
  lmin = 0.15
  lmax = 0.85

  pl.bop()

  -- Divide screen into 100 regions

  pl.ssub(10, 10)

  for i = 0, 99 do
    -- Bounds on HLS, from pl.hlsrgb() commentary --
    --	hue		[0., 360.]	degrees
    --	lightness	[0., 1.]	magnitude
    --	saturation	[0., 1.]	magnitude

    -- Vary hue uniformly from left to right
    h = (360/10) * (i % 10)

    -- Vary lightness uniformly from top to bottom, between min & max
    l = lmin + (lmax - lmin) * math.floor(i/10)/9

    -- Use max saturation
    s = 1

    r1, g1, b1 = pl.hlsrgb(h, l, s)

    -- Use 255.001 to avoid close truncation decisions in this example.
    r[i+1+16] = r1 * 255.001
    g[i+1+16] = g1 * 255.001
    b[i+1+16] = b1 * 255.001
  end

  -- Load default cmap0 colors into our custom set
  for i = 0, 15 do
    r[i+1], g[i+1], b[i+1] = pl.gcol0(i)
  end

  -- Now set cmap0 all at once (faster, since fewer driver calls)
  pl.scmap0(r, g, b)

  draw_windows(100, 16)

  pl.eop()
end


----------------------------------------------------------------------------
-- main
--
-- Demonstrates multipl.e windows and color map 0 palette, both default and
-- user-modified.
----------------------------------------------------------------------------

-- Parse and process command line arguments
pl.parseopts(arg, pl.PL_PARSE_FULL)

-- Initialize pl.pl.ot
pl.init()

-- Run demos
demo1()
demo2()

pl.plend()
