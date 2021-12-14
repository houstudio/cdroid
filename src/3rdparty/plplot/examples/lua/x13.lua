--[[
	Pie chart demo.

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

text = { "Maurice", "Geoffrey", "Alan",
         "Rafael", "Vince" }

--------------------------------------------------------------------------
-- main
--
-- Does a simple pie chart.
--------------------------------------------------------------------------

per = { 10, 32, 12, 30, 16 }

-- Parse and process command line arguments

pl.parseopts(arg, pl.PL_PARSE_FULL);

-- Initialize plplot

pl.init()

pl.adv(0)

-- Ensure window has aspect ratio of one so circle is
-- plotted as a circle.
pl.vasp(1)
pl.wind(0, 10, 0, 10)
pl.col0(2)

-- n.b. all theta quantities scaled by 2*M_PI/500 to be integers to avoid
--floating point logic problems.
theta0 = 0
dthet = 1
for i = 1, 5 do
  x = { 5 }
  y = { 5 }
  j = 2
  -- n.b. the theta quantities multiplied by 2*math.pi/500 afterward so
  -- in fact per is interpreted as a percentage.
	theta1 = theta0 + 5 * per[i]
	if i == 5 then theta1 = 500 end

	for theta = theta0, theta1, dthet do
	    x[j] = 5 + 3 * math.cos(2*math.pi/500*theta)
	    y[j] = 5 + 3 * math.sin(2*math.pi/500*theta)
      j = j + 1
      thetasave=theta
	end
	pl.col0(i)
	pl.psty(((i + 2) % 8) + 1)
	pl.fill(x, y)
	pl.col0(1)
	pl.line(x, y)
	just = 2*math.pi/500*(theta0 + theta1)/2
	dx = 0.25 * math.cos(just)
	dy = 0.25 * math.sin(just)
	if (theta0 + theta1)<250 or (theta0 + theta1)>750 then
	  just = 0
	else
    just = 1
  end

	pl.ptex((x[(j-1)/2+1] + dx), (y[(j-1)/2+1] + dy), 1, 0, just, text[i]);
	theta0 = thetasave
end

pl.font(2)
pl.schr(0, 1.3)
pl.ptex(5, 9, 1, 0, 0.5, "Percentage of Sales")

pl.plend()

