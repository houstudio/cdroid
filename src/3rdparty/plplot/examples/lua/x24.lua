--[[
	Unicode Pace Flag

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

   In Debian, run like this:

   ( TTFDIR=/usr/share/fonts/truetype  \
     PLPLOT_FREETYPE_SANS_FONT=$TTFDIR/arphic/bkai00mp.ttf \
     PLPLOT_FREETYPE_SERIF_FONT=$TTFDIR/freefont/FreeSerif.ttf \
     PLPLOT_FREETYPE_MONO_FONT=$TTFDIR/ttf-devanagari-fonts/lohit_hi.ttf \
     PLPLOT_FREETYPE_SCRIPT_FONT=$TTFDIR/unfonts/UnBatang.ttf \
     PLPLOT_FREETYPE_SYMBOL_FONT=$TTFDIR/ttf-bengali-fonts/JamrulNormal.ttf \
     ./x24c -dev png -drvopt smooth=0 -o x24c.png )

   Packages needed:

   ttf-arphic-bkai00mp
   ttf-freefont
   ttf-devanagari-fonts
   ttf-unfonts
   ttf-bengali-fonts
--]]

-- initialise Lua bindings for PLplot examples.
dofile("plplot_examples.lua")

red   = { 240, 204, 204, 204,   0,  39, 125 }
green = { 240,   0, 125, 204, 204,  80,   0 }
blue  = { 240,   0,   0,   0,   0, 204, 125 }

px = { 0, 0, 1, 1 }
py = { 0, 0.25, 0.25, 0 }

sx = {
  0.16374,
  0.15844,
  0.15255,
  0.17332,
  0.50436,
  0.51721,
  0.49520,
  0.48713,
  0.83976,
  0.81688,
  0.82231,
  0.82647
}

sy = {
  0.125,
  0.375,
  0.625,
  0.875,
  0.125,
  0.375,
  0.625,
  0.875,
  0.125,
  0.375,
  0.625,
  0.875
}


-- Taken from http://www.columbia.edu/~fdc/pace/

peace = {
  -- Mandarin
  "#<0x00>和平",
  -- Hindi
  "#<0x20>शांति",
  -- English
  "#<0x10>Peace",
  -- Hebrew
  "#<0x10>שלום",
  -- Russian
  "#<0x10>Мир",
  -- German
  "#<0x10>Friede",
  -- Korean
  "#<0x30>평화",
  -- French
  "#<0x10>Paix",
  -- Spanish
  "#<0x10>Paz",
  -- Arabic
  "#<0x10>ﺳﻼم",
  -- Turkish
  "#<0x10>Barış",
  -- Kurdish
  "#<0x10>Hasîtî",
}

pl.parseopts(arg, pl.PL_PARSE_FULL)

pl.init()

pl.adv(0)
pl.vpor(0, 1, 0, 1)
pl.wind(0, 1, 0, 1)
pl.col0(0)
pl.box("", 1, 0, "", 1, 0)

pl.scmap0n(7)
pl.scmap0(red, green, blue)

pl.schr(0, 4)
pl.font(1)

for i = 1, 4 do
  pl.col0(i)
  pl.fill(px, py)

  for j = 1, 4 do
    py[j] = py[j] + 1/4
  end
end

pl.col0(0)
for i = 1, 12 do
  pl.ptex(sx[i], sy[i], 1, 0, 0.5, peace[i])
end

pl.plend()
