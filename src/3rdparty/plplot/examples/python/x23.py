#  Copyright (C) 2005-2017 Alan W. Irwin

#  Displays Greek letters and mathematically interesting Unicode ranges.
#
#  This file is part of PLplot.
#
#  PLplot is free software; you can redistribute it and/or modify
#  it under the terms of the GNU Library General Public License as published
#  by the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  PLplot is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Library General Public License for more details.
#
#  You should have received a copy of the GNU Library General Public License
#  along with PLplot; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
from numpy import *

# main
#
# Displays Greek letters and mathematically interesting Unicode ranges

Greek = (
"#gA","#gB","#gG","#gD","#gE","#gZ","#gY","#gH","#gI","#gK","#gL","#gM",
"#gN","#gC","#gO","#gP","#gR","#gS","#gT","#gU","#gF","#gX","#gQ","#gW",
"#ga","#gb","#gg","#gd","#ge","#gz","#gy","#gh","#gi","#gk","#gl","#gm",
"#gn","#gc","#go","#gp","#gr","#gs","#gt","#gu","#gf","#gx","#gq","#gw",
)

Type1 = (
"0x0020","0x0021","0x0023","0x0025","0x0026",
"0x0028","0x0029","0x002b","0x002c","0x002e",
"0x002f","0x0030","0x0031","0x0032","0x0033",
"0x0034","0x0035","0x0036","0x0037","0x0038",
"0x0039","0x003a","0x003b","0x003c","0x003d",
"0x003e","0x003f","0x005b","0x005d","0x005f",
"0x007b","0x007c","0x007d","0x00a9","0x00ac",
"0x00ae","0x00b0","0x00b1","0x00d7","0x00f7",
"0x0192","0x0391","0x0392","0x0393","0x0394",
"0x0395","0x0396","0x0397","0x0398","0x0399",
"0x039a","0x039b","0x039c","0x039d","0x039e",
"0x039f","0x03a0","0x03a1","0x03a3","0x03a4",
"0x03a5","0x03a6","0x03a7","0x03a8","0x03a9",
"0x03b1","0x03b2","0x03b3","0x03b4","0x03b5",
"0x03b6","0x03b7","0x03b8","0x03b9","0x03ba",
"0x03bb","0x03bc","0x03bd","0x03be","0x03bf",
"0x03c0","0x03c1","0x03c2","0x03c3","0x03c4",
"0x03c5","0x03c6","0x03c7","0x03c8","0x03c9",
"0x03d1","0x03d2","0x03d5","0x03d6","0x2022",
"0x2026","0x2032","0x2033","0x203e","0x2044",
"0x2111","0x2118","0x211c","0x2122","0x2126",
"0x2135","0x2190","0x2191","0x2192","0x2193",
"0x2194","0x21b5","0x21d0","0x21d1","0x21d2",
"0x21d3","0x21d4","0x2200","0x2202","0x2203",
"0x2205","0x2206","0x2207","0x2208","0x2209",
"0x220b","0x220f","0x2211","0x2212","0x2215",
"0x2217","0x221a","0x221d","0x221e","0x2220",
"0x2227","0x2228","0x2229","0x222a","0x222b",
"0x2234","0x223c","0x2245","0x2248","0x2260",
"0x2261","0x2264","0x2265","0x2282","0x2283",
"0x2284","0x2286","0x2287","0x2295","0x2297",
"0x22a5","0x22c5","0x2320","0x2321","0x2329",
"0x232a","0x25ca","0x2660","0x2663","0x2665",
"0x2666",
)

title = (
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
"#<0x10>PLplot Example 23 - Mathematical Operators Unicode Block (d)",
)

lo = (
"0x0",
"0x0",
"0x40",
"0x80",
"0x2153",
"0x2190",
"0x21d0",
"0x2200",
"0x2240",
"0x2280",
"0x22c0",
)

hi = (
"0x30",
"0x40",
"0x80",
"0xA6",
"0x2184",
"0x21d0",
"0x2200",
"0x2240",
"0x2280",
"0x22c0",
"0x2300",
)
nxcells = (
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
8,
)

nycells = (
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
8,
)

# non-zero values Must be consistent with nxcells and nycells.
offset = (
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
0,
)

# 30 possible FCI values.

fci = (
0x80000000,
0x80000001,
0x80000002,
0x80000003,
0x80000004,
0x80000010,
0x80000011,
0x80000012,
0x80000013,
0x80000014,
0x80000020,
0x80000021,
0x80000022,
0x80000023,
0x80000024,
0x80000100,
0x80000101,
0x80000102,
0x80000103,
0x80000104,
0x80000110,
0x80000111,
0x80000112,
0x80000113,
0x80000114,
0x80000120,
0x80000121,
0x80000122,
0x80000123,
0x80000124,
)

family = (
"sans-serif",
"serif",
"monospace",
"script",
"symbol",
)
style = (
"upright",
"italic",
  "oblique",
)
weight = (
"medium",
"bold",
)

def main(w):

    for page in range(11):
        w.pladv(0)

        # Set up viewport and window

        w.plvpor(0.02, 0.98, 0.02, 0.90)
        w.plwind(0.0, 1.0, 0.0, 1.0)
        mm = w.plgspa()
        w.plschr(0., 0.8)
        ycharacter_scale = (1.0 - 0.0)/(mm[3]-mm[2])
        # Factor should be 0.5, but heuristically it turns out to be larger.
        yoffset = 1.0*w.plgchr()[1]*ycharacter_scale

        # Draw the grid using w.plbox

        w.plcol0(2)
        deltax = 1./float(nxcells[page])
        deltay = 1./float(nycells[page])
        w.plbox("bcg", deltax, 0, "bcg", deltay, 0)
        w.plcol0(15)
        length = int(hi[page],16) - int(lo[page],16)
        slice = 0
        for y in (0.5+arange(nycells[page]-1,-1,-1))*deltay:
            for x in (0.5+arange(nxcells[page]))*deltax:
                if slice < length:
                    if page == 0:
                        # Greek letters.
                        CommandString = str(Greek[slice]);
                    elif 1 <= page and page <= 3:
                        # Unicode for Type 1 Symbol Glyphs.
                        CommandString = "#[" + str(Type1\
                        [offset[page]+slice]) + "]"
                    elif page >= 4:
                        CommandString = "#[" + hex(int(lo[page],16)+slice) + "]"
                    w.plptex(x,y+yoffset,1.,0.,0.5, CommandString)
                    w.plptex(x,y-yoffset,1.,0.,0.5, "#" +CommandString)
                slice += 1

        w.plschr(0., 1.0)
        # Page title
        w.plmtex("t", 1.5, 0.5, 0.5, title[page])

    # Demonstrate methods of getting the current fonts

    fci_old = w.plgfci()
    font = w.plgfont()
    ifamily = font[0]
    istyle = font[1]
    iweight = font[2]
    print("For example 23 prior to page 12 the FCI is 0x%x" % (fci_old))
    print("For example 23 prior to page 12 the font family, style and weight are %s %s %s" % (family[ifamily],style[istyle],weight[iweight]))

    FCI_COMBINATIONS = 30
    dy = 0.030
    for page in range(11,16):
        w.pladv(0)
        w.plvpor(0.02, 0.98, 0.02, 0.90)
        w.plwind(0.0, 1.0, 0.0, 1.0)
        w.plsfci(0)
        if(page == 11):
            w.plmtex("t", 1.5, 0.5, 0.5,"#<0x10>PLplot Example 23 - Set Font with plsfci")
        elif(page == 12):
            w.plmtex("t", 1.5, 0.5, 0.5,"#<0x10>PLplot Example 23 - Set Font with plsfont")
        elif(page == 13):
            w.plmtex("t", 1.5, 0.5, 0.5,"#<0x10>PLplot Example 23 - Set Font with ##<0x8nnnnnnn> construct")
        elif(page == 14):
            w.plmtex("t", 1.5, 0.5, 0.5,"#<0x10>PLplot Example 23 - Set Font with ##<0xmn> constructs")
        elif(page == 15):
          w.plmtex("t", 1.5, 0.5, 0.5,"#<0x10>PLplot Example 23 - Set Font with ##<FCI COMMAND STRING/> constructs")
        w.plschr(0., 0.75)
        for i in range(0,FCI_COMBINATIONS):
            family_index = i % 5
            style_index = int((i/5) % 3)
            weight_index = int(((i/5)/3) % 2)
            if(page == 11):
                w.plsfci(fci[i])
                string = "Page 12, "+family[family_index]+", "+style[style_index]+", "+weight[weight_index]+":  "+"The quick brown fox jumps over the lazy dog"
            elif(page == 12):
                w.plsfont(family_index, style_index, weight_index)
                string = "Page 13, "+family[family_index]+", "+style[style_index]+", "+weight[weight_index]+":  "+"The quick brown fox jumps over the lazy dog"
            elif(page == 13):
                string = "Page 14, "+family[family_index]+", "+style[style_index]+", "+weight[weight_index]+":  #<"+hex(fci[i]).rstrip('L')+">"+"The quick brown fox jumps over the lazy dog"
            elif(page == 14):
                string = "Page 15, "+family[family_index]+", "+style[style_index]+", "+weight[weight_index]+":  #<"+hex(family_index)+"0>"+"#<"+hex(style_index)+"1>"+"#<"+hex(weight_index)+"2>"+"The quick brown fox jumps over the lazy dog"
            elif(page == 15):
                string = "Page 16, "+family[family_index]+", "+style[style_index]+", "+weight[weight_index]+":  #<"+family[family_index]+"/>#<"+style[style_index]+"/>#<"+weight[weight_index]+"/>"+"The quick brown fox jumps over the lazy dog"
            w.plptex (0., 1. - (i+0.5)*dy, 1., 0., 0., string)

        w.plschr(0., 1.0)

    # Restore defaults
    w.plschr( 0.0, 1.0 )

    # Would not do this ordinarily, but this is required to follow C example.
    w.plcol0(1)

    # Must be done independently because otherwise this changes output files
    # and destroys agreement with C examples.
    #w.plcol0(1)

