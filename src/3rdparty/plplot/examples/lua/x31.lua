--[[
  Copyright (C) 2008 Werner Smekal
  Copyright (C) 2008-2016 Alan W. Irwin

  set/get tester

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

r1 = { 0, 255 }
g1 = { 255, 0 }
b1 = { 0, 0 }
a1 = { 1, 1 }

-- Parse and process command line arguments
status = 0
pl.parseopts(arg, pl.PL_PARSE_FULL)

-- Test setting / getting familying parameters before plinit
-- Save values set by plparseopts to be restored later.
fam0, num0, bmax0 = pl.gfam()
fam1 = 0
num1 = 10
bmax1 = 1000
pl.sfam(fam1, num1, bmax1)

-- Retrieve the same values?
fam2, num2, bmax2 = pl.gfam()
print(string.format("family parameters: fam, num, bmax = %d %d %d", fam2, num2, bmax2))
if fam2~=fam1 or num2~=num1 or bmax2~=bmax1 then
  io.stderr:write("plgfam test failed\n")
  status = 1
end
-- Restore values set initially by plparseopts.
pl.sfam(fam0, num0, bmax0)

-- Test setting / getting page parameters before plinit
-- Save values set by plparseopts to be restored later.
xp0, yp0, xleng0, yleng0, xoff0, yoff0 = pl.gpage()
xp1 = 200.
yp1 = 200.
xleng1 = 400
yleng1 = 200
xoff1 = 10
yoff1 = 20
pl.spage(xp1, yp1, xleng1, yleng1, xoff1, yoff1)

-- Retrieve the same values?
xp2, yp2, xleng2, yleng2, xoff2, yoff2 = pl.gpage()
print(string.format("page parameters: xp, yp, xleng, yleng, xoff, yoff = %f %f %d %d %d %d", xp2, yp2, xleng2, yleng2, xoff2, yoff2))
if xp2~=xp1 or yp2~=yp1 or xleng2~=xleng1 or yleng2~=yleng1 or xoff2~=xoff1 or yoff2~=yoff1 then
  io.stderr:write("plgpage test failed\n")
  status = 1
end
-- Restore values set initially by plparseopts.
pl.spage(xp0, yp0, xleng0, yleng0, xoff0, yoff0)

-- Test setting / getting compression parameter across plinit.
compression1 = 95
pl.scompression(compression1)

-- Initialize plplot
pl.init()

-- Test if device initialization screwed around with the preset
-- compression parameter.
compression2 = pl.gcompression()
print("Output various PLplot parameters")
print(string.format("compression parameter = %d", compression2))
if compression2~=compression1 then
  io.stderr:write("plgcompression test failed\n")
  status = 1
end


-- Exercise plscolor, plscol0, plscmap1, and plscmap1a to make sure
--they work without any obvious error messages.
pl.scolor(1)
pl.scol0(1, 255, 0, 0)
pl.scmap1(r1, g1, b1)
pl.scmap1a(r1, g1, b1, a1)

level2 = pl.glevel()
print(string.format("level parameter = %d", level2))
if level2~=1 then
  io.stderr:write("plglevel test failed.\n")
  status = 1
end

pl.adv(0)

xmin0 = 0.01
xmax0 = 0.99
ymin0 = 0.02
ymax0 = 0.49
pl.vpor(xmin0, xmax0, ymin0, ymax0)
xmin, xmax, ymin, ymax = pl.gvpd()
print(string.format("plvpor: xmin, xmax, ymin, ymax = %f %f %f %f", xmin, xmax, ymin, ymax))
if xmin~=xmin0 or xmax~=xmax0 or ymin~=ymin0 or ymax~=ymax0 then
  io.stderr:write("plgvpd test failed\n")
  status = 1
end
xmid = 0.5*(xmin+xmax)
ymid = 0.5*(ymin+ymax)

xmin0 = 0.2
xmax0 = 0.3
ymin0 = 0.4
ymax0 = 0.5
pl.wind(xmin0, xmax0, ymin0, ymax0)
xmin, xmax, ymin, ymax = pl.gvpw()
print(string.format("plwind: xmin, xmax, ymin, ymax = %f %f %f %f", xmin, xmax, ymin, ymax))
if xmin~=xmin0 or xmax~=xmax0 or ymin~=ymin0 or ymax~=ymax0 then
  io.stderr:write("plgvpw test failed\n")
  status = 1
end

-- Get world coordinates for middle of viewport
wx, wy, win = pl.calc_world(xmid,ymid)
print(string.format("world parameters: wx, wy, win = %f %f %d", wx, wy, win))
if math.abs(wx-0.5*(xmin+xmax))>1.0e-5 or math.abs(wy-0.5*(ymin+ymax))>1.0e-5 then
  io.stderr:write("plcalc_world test failed\n")
  status = 1
end

-- Retrieve and print the name of the output file (if any).
-- This goes to stderr not stdout since it will vary between tests and
-- we want stdout to be identical for compare test.
fnam = pl.gfnam()
if fnam=="" then
  print("No output file name is set")
else
  print("Output file name read")
end
io.stderr:write(string.format("Output file name is %s\n",fnam))

-- Set and get the number of digits used to display axis labels
-- Note digits is currently ignored in pls[xyz]ax and
-- therefore it does not make sense to test the returned
-- value
pl.sxax(3,0)
digmax, digits = pl.gxax()
print(string.format("x axis parameters: digmax, digits = %d %d", digmax, digits))
if digmax~=3 then
  io.stderr:write("plgxax test failed\n")
  status = 1
end

pl.syax(4,0)
digmax, digits = pl.gyax()
print(string.format("y axis parameters: digmax, digits = %d %d", digmax, digits))
if digmax~=4 then
  io.stderr:write("plgyax test failed\n")
  status = 1
end

pl.szax(5,0)
digmax,digits = pl.gzax()
print(string.format("z axis parameters: digmax, digits = %d %d", digmax, digits))
if digmax~=5 then
  io.stderr:write("plgzax test failed\n")
  status = 1
end

mar0 = 0.05
aspect0 = pl.PL_NOTSET
jx0 = 0.1
jy0 = 0.2
pl.sdidev(mar0, aspect0, jx0, jy0)
mar, aspect, jx, jy = pl.gdidev()
print(string.format("device-space window parameters: mar, aspect, jx, jy = %f %f %f %f" , mar, aspect, jx, jy))
if mar~=mar0 or jx~=jx0 or jy~=jy0 then
  io.stderr:write("plgdidev test failed\n")
  status = 1
end

ori0 = 1.0
pl.sdiori(ori0)
ori = pl.gdiori()
print(string.format("ori parameter = %f", ori))
if ori~=ori0 then
  io.stderr:write("plgdiori test failed\n")
  status = 1
end

xmin0 = 0.1
ymin0 = 0.2
xmax0 = 0.9
ymax0 = 0.8
pl.sdiplt(xmin0, ymin0, xmax0, ymax0)
xmin, ymin, xmax, ymax = pl.gdiplt()
print(string.format("plot-space window parameters: xmin, ymin, xmax, ymax = %f %f %f %f", xmin, ymin, xmax, ymax))
if xmin~=xmin0 or ymin~=ymin0 or xmax~=xmax0 or ymax~=ymax0 then
  io.stderr:write("plgdiplt test failed\n")
  status = 1
end

zxmin0 = 0.1
zymin0 = 0.1
zxmax0 = 0.9
zymax0 = 0.9
pl.sdiplz(zxmin0, zymin0, zxmax0, zymax0)
zxmin, zymin, zxmax, zymax = pl.gdiplt()
print(string.format("zoomed plot-space window parameters: xmin, ymin, xmax, ymax = %f %f %f %f", zxmin, zymin, zxmax, zymax))
if math.abs(zxmin -(xmin + (xmax-xmin)*zxmin0)) > 1.0e-5 or
   math.abs(zymin -(ymin + (ymax-ymin)*zymin0)) > 1.0e-5 or
   math.abs(zxmax -(xmin + (xmax-xmin)*zxmax0)) > 1.0e-5 or
   math.abs(zymax -(ymin + (ymax-ymin)*zymax0)) > 1.0e-5 then
  io.stderr:write("plsdiplz test failed\n")
  status = 1
end

r0 = 10
g0 = 20
b0 = 30
pl.scolbg(r0, g0, b0)
r, g, b = pl.gcolbg()
print(string.format("background colour parameters: r, g, b = %d %d %d", r, g, b))
if r~=r0 or g~=g0 or b~=b0 then
  io.stderr:write("plgcolbg test failed\n")
  status = 1
end

r0 = 20
g0 = 30
b0 = 40
a0 = 0.5
pl.scolbga(r0, g0, b0, a0)
r, g, b, a = pl.gcolbga()
print(string.format("background/transparency colour parameters: r, g, b, a = %d %d %d %f", r, g, b, a))
if r~=r0 or g~=g0 or b~=b0 or a~=a0 then
  io.stderr:write("plgcolbga test failed\n")
  status = 1
end

pl.plend()
