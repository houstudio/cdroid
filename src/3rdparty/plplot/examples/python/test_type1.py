#!/usr/bin/env python

# Copyright 2010-2016 Alan W. Irwin
# This file is part of PLplot.

# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; version 2 of the License.

# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.

# You should have received a copy of the GNU Library General Public License
# along with the file PLplot; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

# Plots the first 256 unicode glyphs.
# N.B. If ps.c is edited to #define PL_TEST_TYPE1 (which interprets
# unicode in the range from 0 to 255 directly as Type1 font indices) and
# if the -dev ps -drvopt hrshsym=0 command-line options are used, then
# the results are similar to the results obtained from the historical
# gfontview application that is no longer available under Linux.

# Append to effective python path so that can find plplot modules.
from plplot_python_start import *

import sys
import plplot as w

# Parse and process command line arguments
w.plparseopts(sys.argv, w.PL_PARSE_FULL)

# Initialize plplot
w.plinit()

# main
#
# Displays all available Type1 glyphs.

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


    FCI_COMBINATIONS = 30
    w.plsfont(0, 0, 0)
    for index_fci in range(0,FCI_COMBINATIONS):
        family_index = index_fci % 5
        style_index = (index_fci/5) % 3
        weight_index = ((index_fci/5)/3) % 2
        title = "Type 1 glyphs for family = " + family[family_index] + ", style = " + style[style_index] + ", weight = " + weight[weight_index]
        w.pladv(0)

# Set up viewport and window

        w.plcol0(2)
        w.plvpor(0.1, 1.0, 0.1, 0.9)
        w.plwind(0.0, 1.0, 0.0, 1.0)

# Draw the grid using w.plbox

        w.plbox("bcg", 1./16., 0, "bcg", 1./16., 0)

# Write the digits below the frame

        w.plcol0(15)
        for i in range(16):
            w.plmtex("b", 1.5, ((i+0.5)/16.), 0.5, str(i))

        k = 0
        w.plmtex("t", 1.5, 0.5, 0.5, title)
        for i in range(16):

            # Write the digits to the left of the frame

            w.plmtex("lv", 1.0, (1.0 - (i+0.5)/16.), 1.0, str(16 * i))

            w.plsfont(family_index, style_index, weight_index)
            for j in range(16):
                x = (j+0.5)/16.
                y = 1. - (i+0.5)/16

                # Display the Type 1 glyph corresponding to k
                glyph_string = unichr(k).encode('utf-8')
                # Escape the escape.
                if glyph_string == "#":
                    glyph_string = "##"
                w.plptex(x, y, 1., 0., 0.5, glyph_string)
                k = k + 1
            w.plsfont(0, 0, 0)

main(w)
w.plend()
