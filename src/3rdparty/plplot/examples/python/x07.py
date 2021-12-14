#  Copyright (C) 2001-2016 Alan W. Irwin

#  Font demo.
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

# main
#
# Displays the entire "plsym" symbol (font) set.

def main(w):

    base = [0, 100, 0, 100, 200, 500, 600, 700, 800, 900, 2000,
            2100, 2200, 2300, 2400, 2500, 2600, 2700, 2800, 2900]

    w.plfontld( 0 )

    for l in range(20):
        if l == 2:
            w.plfontld( 1 )

        w.pladv(0)

        # Set up viewport and window

        w.plcol0(2)
        w.plvpor(0.15, 0.95, 0.1, 0.9)
        w.plwind(0.0, 1.0, 0.0, 1.0)

        # Draw the grid using plbox

        w.plbox("bcg", 0.1, 0, "bcg", 0.1, 0)

        # Write the digits below the frame

        w.plcol0(15)
        for i in range(10):
            w.plmtex("b", 1.5, (0.1 * i + 0.05), 0.5, str(i))

        k = 0
        for i in range(10):

            # Write the digits to the left of the frame

            text = str(base[l] + 10 * i)
            w.plmtex("lv", 1.0, (0.95 - 0.1 * i), 1.0, text)

            for j in range(10):
                x = [ 0.1 * j + 0.05 ]
                y = [ 0.95 - 0.1 * i ]

                # Display the symbols

                w.plsym(x, y, base[l] + k)
                k = k + 1
        if l < 2 :
            w.plmtex("t", 1.5, 0.5, 0.5, "PLplot Example 7 - PLSYM symbols (compact)")
        else :
            w.plmtex("t", 1.5, 0.5, 0.5, "PLplot Example 7 - PLSYM symbols (extended)")

    # Restore defaults
    w.plfontld(1)
    w.plfont(1)
    # Must be done independently because otherwise this changes output files
    # and destroys agreement with C examples.
    #w.plcol0(1)
