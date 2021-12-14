--[[
	Font demo.

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
-- main
--
-- Displays the entire "plpoin" symbol (font) set.
----------------------------------------------------------------------------

-- Parse and process command line arguments
pl.parseopts(arg, pl.PL_PARSE_FULL)

-- Initialize plplot
pl.init()

for kind_font = 0,1 do
  pl.fontld( kind_font )
  if kind_font == 0 then
    maxfont = 1
  else
    maxfont = 4
  end

  for font = 1,maxfont do
    pl.font( font )
    pl.adv(0)

    -- Set up viewport and window
    pl.col0(2)
    pl.vpor(0.1, 1, 0.1, 0.9)
    pl.wind(0, 1, 0, 1.3)

    -- Draw the grid using plbox
    pl.box("bcg", 0.1, 0, "bcg", 0.1, 0)

    -- Write the digits below the frame
    pl.col0(15)
    for i=0, 9 do
      pl.mtex("b", 1.5, (0.1 * i + 0.05), 0.5, tostring(i))
    end

    k = 0
    x = {}
    y ={}
    for i = 0, 12 do
      -- Write the digits to the left of the frame
      pl.mtex("lv", 1, (1 - (2 * i + 1) / 26), 1, tostring(10*i))
      for j = 0, 9 do
        x[1] = 0.1 * j + 0.05
        y[1] = 1.25 - 0.1 * i

        -- Display the symbols
        if k < 128 then
          pl.poin(x, y, k)
        end
        k = k + 1
      end
    end

    if kind_font == 0 then
      pl.mtex("t", 1.5, 0.5, 0.5, "PLplot Example 6 - plpoin symbols (compact)")
    else
      pl.mtex("t", 1.5, 0.5, 0.5, "PLplot Example 6 - plpoin symbols (extended)")
    end
  end
end
pl.plend()
