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
# Displays the entire "plpoin" symbol (font) set.

def main(w):

    for kind_font in range(2):

        w.plfontld( kind_font )

        if kind_font == 0 :
            maxfont = 1
        else :
            maxfont = 4

        for font in range(maxfont):
            w.plfont( font + 1 )

            w.pladv(0)

            # Set up viewport and window

            w.plcol0(2)
            w.plvpor(0.1, 1.0, 0.1, 0.9)
            w.plwind(0.0, 1.0, 0.0, 1.3)

            # Draw the grid using plbox

            w.plbox("bcg", 0.1, 0, "bcg", 0.1, 0)

            # Write the digits below the frame

            w.plcol0(15)
            for i in range(10):
                w.plmtex("b", 1.5, (0.1 * i + 0.05), 0.5, str(i))

            k = 0
            for i in range(13):

                # Write the digits to the left of the frame

                w.plmtex("lv", 1.0, (1.0 - (2 * i + 1) / 26.0), 1.0, str(10 * i))

                for j in range(10):
                    x = 0.1 * j + 0.05
                    y = 1.25 - 0.1 * i

                    # Display the symbol (plpoin expects that x
                    # and y are arrays so pass lists)

                    if k < 128:
                        w.plpoin([x], [y], k)
                    k = k + 1

            if kind_font == 0 :
                w.plmtex("t", 1.5, 0.5, 0.5, "PLplot Example 6 - plpoin symbols (compact)")
            else :
                w.plmtex("t", 1.5, 0.5, 0.5, "PLplot Example 6 - plpoin symbols (extended)")

    # Restore defaults
    w.plfontld(1)
    w.plfont(1)
    # Must be done independently because otherwise this changes output files
    # and destroys agreement with C examples.
    #w.plcol0(1)
