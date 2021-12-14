--[[
	Plots a simple stripchart with four pens.

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


--------------------------------------------------------------------------
-- main
--------------------------------------------------------------------------

nsteps = 1000
colline = {}
legline = {}

-- plplot initialization
-- Parse and process command line arguments
pl.parseopts(arg, pl.PL_PARSE_FULL)

-- User sets up plot completely except for window and data
-- Eventually settings in place when strip chart is created will be
-- remembered so that multiple strip charts can be used simultaneously.

-- Specify some reasonable defaults for ymin and ymax
-- The plot will grow automatically if needed (but not shrink)
ymin = -0.1
ymax = 0.1

-- Specify initial tmin and tmax -- this determines length of window.
-- Also specify maximum jump in t
-- This can accomodate adaptive timesteps
tmin = 0
tmax = 10
tjump = 0.3	-- percentage of plot to jump

-- Axes options same as plbox.
-- Only automatic tick generation and label placement allowed
-- Eventually I'll make this fancier
colbox = 1
collab = 3
styline = { 2, 3, 4, 5 } -- line style
colline = { 2, 3, 4, 5 } -- pens color

legline= { "sum", "sin", "sin*noi", "sin+noi" } -- pens legend

xlab = 0 -- legend position
ylab = 0.25

autoy = 1	-- autoscale y
acc = 1	-- don't scrip, accumulate

-- Initialize plplot
pl.init()

pl.adv(0)
pl.vsta()

id1 = pl.stripc("bcnst", "bcnstv",
   tmin, tmax, tjump, ymin, ymax,
   xlab, ylab,
   autoy, acc,
   colbox, collab,
   colline, styline, legline,
   "t", "", "Strip chart demo")

autoy = 0	-- autoscale y
acc = 1	-- accumulate

-- This is to represent a loop over time
-- Let's try a random walk process

y1 = 0
y2 = 0
y3 = 0
y4 = 0
dt = 0.1

for n = 0, nsteps-1 do
	for i = 0, 200000 do end
	t = n * dt
	noise = pl.randd() - 0.5
	y1 = y1 + noise
	y2 = math.sin(t*math.pi/18)
	y3 = y2 * noise
	y4 = y2 + noise/3

  -- There is no need for all pens to have the same number of
  -- points or beeing equally time spaced.
	if (n % 2)~=0 then pl.stripa(id1, 0, t, y1) end
	if (n % 3)~=0 then pl.stripa(id1, 1, t, y2) end
	if (n % 4)~=0 then pl.stripa(id1, 2, t, y3) end
	if (n % 5)~=0 then pl.stripa(id1, 3, t, y4) end
end

-- Destroy strip chart and it's memory
pl.stripd(id1)
pl.plend()
