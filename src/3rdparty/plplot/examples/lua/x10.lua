--[[
	Window positioning demo.

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
-- Demonstrates absolute positioning of graphs on a page.
----------------------------------------------------------------------------

-- Parse and process command line arguments
pl.parseopts(arg, pl.PL_PARSE_FULL);

-- Initialize plplot
pl.init()

pl.adv(0)
pl.vpor(0.0, 1.0, 0.0, 1.0)
pl.wind(0.0, 1.0, 0.0, 1.0)
pl.box("bc", 0.0, 0, "bc", 0.0, 0)

pl.svpa(50.0, 150.0, 50.0, 100.0)
pl.wind(0.0, 1.0, 0.0, 1.0)
pl.box("bc", 0.0, 0, "bc", 0.0, 0)
pl.ptex(0.5, 0.5, 1.0, 0.0, 0.5, "BOX at (50,150,50,100)")
pl.plend()
