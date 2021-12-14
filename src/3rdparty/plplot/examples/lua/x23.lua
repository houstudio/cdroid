--[[
    Displays Greek letters and mathematically interesting Unicode ranges

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

----------------------------------------------------------------------------
-- main
--
-- Displays Greek letters and mathematically interesting Unicode ranges
----------------------------------------------------------------------------

Greek = {
  "#gA","#gB","#gG","#gD","#gE","#gZ","#gY","#gH","#gI","#gK","#gL","#gM",
  "#gN","#gC","#gO","#gP","#gR","#gS","#gT","#gU","#gF","#gX","#gQ","#gW",
  "#ga","#gb","#gg","#gd","#ge","#gz","#gy","#gh","#gi","#gk","#gl","#gm",
  "#gn","#gc","#go","#gp","#gr","#gs","#gt","#gu","#gf","#gx","#gq","#gw"
}

Type1 = {
  32, 33, 35, 37, 38,
  40, 41, 43, 44, 46,
  47, 48, 49, 50, 51,
  52, 53, 54, 55, 56,
  57, 58, 59, 60, 61,
  62, 63, 91, 93, 95,
  123, 124, 125, 169, 172,
  174, 176, 177, 215, 247,
  402, 913, 914, 915, 916,
  917, 918, 919, 920, 921,
  922, 923, 924, 925, 926,
  927, 928, 929, 931, 932,
  933, 934, 935, 936, 937,
  945, 946, 947, 948, 949,
  950, 951, 952, 953, 954,
  955, 956, 957, 958, 959,
  960, 961, 962, 963, 964,
  965, 966, 967, 968, 969,
  977, 978, 981, 982, 8226,
  8230, 8242, 8243, 8254, 8260,
  8465, 8472, 8476, 8482, 8486,
  8501, 8592, 8593, 8594, 8595,
  8596, 8629, 8656, 8657, 8658,
  8659, 8660, 8704, 8706, 8707,
  8709, 8710, 8711, 8712, 8713,
  8715, 8719, 8721, 8722, 8725,
  8727, 8730, 8733, 8734, 8736,
  8743, 8744, 8745, 8746, 8747,
  8756, 8764, 8773, 8776, 8800,
  8801, 8804, 8805, 8834, 8835,
  8836, 8838, 8839, 8853, 8855,
  8869, 8901, 8992, 8993, 9001,
  9002, 9674, 9824, 9827, 9829,
  9830
}

title = {
  "#<0x10>PLplot Example 23 - Greek Letters",
  "#<0x10>PLplot Example 23 - Type 1 Symbol Font Glyphs by Unicode (a)",
  "#<0x10>PLplot Example 23 - Type 1 Symbol Font Glyphs by Unicode (b)",
  "#<0x10>PLplot Example 23 - Type 1 Symbol Font Glyphs by Unicode (c)",
  "#<0x10>PLplot Example 23 - Number Forms Unicode Block",
  "#<0x10>PLplot Example 23 - Arrows Unicode Block (a)",
  "#<0x10>PLplot Example 23 - Arrows Unicode Block (b)",
  "#<0x10>PLplot Example 23 - Mathematical Operators Unicode Block (a)",
  "#<0x10>PLplot Example 23 - Mathematical Operators Unicode Block (b)",
  "#<0x10>PLplot Example 23 - Mathematical Operators Unicode Block (c)",
  "#<0x10>PLplot Example 23 - Mathematical Operators Unicode Block (d)"
}

lo = {
  0,
  0,
  64,
  128,
  8531,
  8592,
  8656,
  8704,
  8768,
  8832,
  8896
}

hi = {
  48,
  64,
  128,
  166,
  8580,
  8656,
  8704,
  8768,
  8832,
  8896,
  8960
}

nxcells = {
  12,
  8,
  8,
  8,
  8,
  8,
  8,
  8,
  8,
  8,
  8
}

nycells = {
  8,
  8,
  8,
  8,
  8,
  8,
  8,
  8,
  8,
  8,
  8
}

-- non-zero values Must be consistent with nxcells and nycells.
offset = {
  0,
  0,
  64,
  128,
  0,
  0,
  0,
  0,
  0,
  0,
  0
}

-- 30 possible FCI values.
FCI_COMBINATIONS = 30
fci = {
  2147483648,
  2147483649,
  2147483650,
  2147483651,
  2147483652,
  2147483664,
  2147483665,
  2147483666,
  2147483667,
  2147483668,
  2147483680,
  2147483681,
  2147483682,
  2147483683,
  2147483684,
  2147483904,
  2147483905,
  2147483906,
  2147483907,
  2147483908,
  2147483920,
  2147483921,
  2147483922,
  2147483923,
  2147483924,
  2147483936,
  2147483937,
  2147483938,
  2147483939,
  2147483940
}

family = {
  "sans-serif",
  "serif",
  "monospace",
  "script",
  "symbol"
}

style = {
  "upright",
  "italic",
  "oblique"
}

weight = {
  "medium",
  "bold"
}

-- Parse and process command line arguments
pl.parseopts(arg, pl.PL_PARSE_FULL)

-- Initialize plplot
pl.init()

for page=1, 11 do
  pl.adv(0)

  -- Set up viewport and window
  pl.vpor(0.02, 0.98, 0.02, 0.90)
  pl.wind(0, 1, 0, 1)
  xmin, xmax, ymin, ymax = pl.gspa()
  pl.schr(0, 0.8)
  ycharacter_scale = (1-0)/(ymax-ymin)

  -- Factor should be 0.5, but heuristically it turns out to be larger.
  chardef, charht = pl.gchr()
  yoffset = charht*ycharacter_scale

  -- Draw the grid using plbox
  pl.col0(2)
  deltax = 1.0/nxcells[page]
  deltay = 1.0/nycells[page]
  pl.box("bcg", deltax, 0, "bcg", deltay, 0)
  pl.col0(15)
  length=hi[page]-lo[page]
  slice = 1

  for j=nycells[page]-1, -1, -1 do
    y = (0.5+j)*deltay
    for i=1, nxcells[page] do
      x  = (i-0.5)*deltax
      if slice<=length then
        if page==1 then
          cmdString = "#" .. Greek[slice]
        elseif (page>=2) and (page<=4) then
          cmdString = string.format("##[0x%.4x]", Type1[offset[page]+slice])
        elseif page>4 then
          cmdString = string.format("##[0x%.4x]", lo[page]+slice-1)
        end
        pl.ptex(x, y+yoffset, 1, 0, 0.5, string.sub(cmdString,2))
        pl.ptex(x, y-yoffset, 1, 0, 0.5, cmdString)
      end
      slice = slice + 1
    end
  end

  pl.schr(0, 1)

  -- Page title
  pl.mtex("t", 1.5, 0.5, 0.5, title[page])
end

-- Demonstrate methods of getting the current fonts
fci_old = pl.gfci()
ifamily, istyle, iweight = pl.gfont()
print(string.format("For example 23 prior to page 12 the FCI is 0x%x", fci_old))
print(string.format("For example 23 prior to page 12 the font family, style and weight are %s %s %s",
                    family[ifamily+1], style[istyle+1], weight[iweight+1]))

for page=12, 16 do
  dy = 0.030

  pl.adv(0)
  pl.vpor(0.02, 0.98, 0.02, 0.90)
  pl.wind(0, 1, 0, 1)
  pl.sfci(0)

  if page==12 then
    pl.mtex("t", 1.5, 0.5, 0.5, "#<0x10>PLplot Example 23 - Set Font with plsfci")
  elseif page==13 then
    pl.mtex("t", 1.5, 0.5, 0.5, "#<0x10>PLplot Example 23 - Set Font with plsfont")
  elseif page==14 then
    pl.mtex("t", 1.5, 0.5, 0.5, "#<0x10>PLplot Example 23 - Set Font with ##<0x8nnnnnnn> construct")
  elseif page==15 then
    pl.mtex("t", 1.5, 0.5, 0.5, "#<0x10>PLplot Example 23 - Set Font with ##<0xmn> constructs")
  elseif page==16 then
    pl.mtex("t", 1.5, 0.5, 0.5, "#<0x10>PLplot Example 23 - Set Font with ##<FCI COMMAND STRING/> constructs")
  end

  pl.schr(0, 0.75)
  for i=1, FCI_COMBINATIONS do
    family_index = ((i-1) %  5) +1
    style_index = (math.floor((i-1)/5) % 3)+1
    weight_index = (math.floor((i-1)/5/3) % 2)+1
    if page==12 then
      pl.sfci(fci[i])
      str = string.format("Page 12, %s, %s, %s:  The quick brown fox jumps over the lazy dog",
                          family[family_index], style[style_index], weight[weight_index])
    elseif page==13 then
      pl.sfont(family_index-1, style_index-1, weight_index-1)
      str = string.format("Page 13, %s, %s, %s:  The quick brown fox jumps over the lazy dog",
                          family[family_index], style[style_index], weight[weight_index])
    elseif page==14 then
      str = string.format("Page 14, %s, %s, %s:  #<0x%x>The quick brown fox jumps over the lazy dog",
                          family[family_index], style[style_index], weight[weight_index], fci[i])
    elseif page==15 then
      str = string.format("Page 15, %s, %s, %s:  #<0x%1x0>#<0x%1x1>#<0x%1x2>The quick brown fox jumps over the lazy dog",
                          family[family_index], style[style_index], weight[weight_index],
                          family_index-1, style_index-1, weight_index-1)
    elseif page==16 then
      str = string.format("Page 16, %s, %s, %s:  #<%s/>#<%s/>#<%s/>The quick brown fox jumps over the lazy dog",
                          family[family_index], style[style_index], weight[weight_index],
                          family[family_index], style[style_index], weight[weight_index])
    end
    pl.ptex(0, 1-(i-0.5)*dy, 1, 0, 0, str)
  end

  pl.schr(0, 1)
end

-- Restore defaults
pl.col0(1)

pl.plend()
