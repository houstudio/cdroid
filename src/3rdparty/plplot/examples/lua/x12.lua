--[[
	Bar chart demo.

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

function pl.fbox(x0, y0)
  x = { x0, x0, x0+1,x0+1 }
  y = {  0, y0,   y0,   0 }

  pl.fill(x, y);
  pl.col0(1);
  pl.lsty(1);
  pl.line(x, y);
end

--------------------------------------------------------------------------
-- main
--
-- Does a simple bar chart, using color fill.  If color fill is
-- unavailable, pattern fill is used instead (automatic).
--------------------------------------------------------------------------

y0 = {}

pos   = { 0, 0.25, 0.5, 0.75, 1 }
red   = { 0, 0.25, 0.5,    1, 1 }
green = { 1,  0.5, 0.5,  0.5, 1 }
blue  = { 1,    1, 0.5, 0.25, 0 }


-- Parse and process command line arguments

pl.parseopts(arg, pl.PL_PARSE_FULL);

-- Initialize plplot
pl.init()

pl.adv(0)
pl.vsta()
pl.wind(1980, 1990, 0, 35)
pl.box("bc", 1, 0, "bcnv", 10, 0)
pl.col0(2)
pl.lab("Year", "Widget Sales (millions)", "#frPLplot Example 12")

y0 = { 5, 15, 12, 24, 28, 30, 20, 8, 12, 3}

pl.scmap1l(1, pos, red, green, blue);

for i=1, 10 do
	pl.col1((i-1)/9.0);
	pl.psty(0);
	pl.fbox((1980+i-1), y0[i]);
	pl.ptex((1980+i-0.5), (y0[i]+1), 1, 0, 0.5, tostring(y0[i]));
	pl.mtex("b", 1, (i*0.1-0.05), 0.5, tostring(1980+i-1));
end

pl.plend();
