#  Copyright (C) 2001-2016 Alan W. Irwin

#  Bar chart demo.
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
# Does a simple bar chart, using color fill.  If color fill is
# unavailable, pattern fill is used instead (automatic).

def main(w):

    pos = array ([0.0, 0.25, 0.5, 0.75, 1.0])
    red = array ([0.0, 0.25, 0.5, 1.0, 1.0])
    green = array ([1.0, 0.5, 0.5, 0.5, 1.0])
    blue = array ([1.0, 1.0, 0.5, 0.25, 0.0])

    w.pladv(0)
    w.plvsta()
    w.plwind(1980.0, 1990.0, 0.0, 35.0)
    w.plbox("bc", 1.0, 0, "bcnv", 10.0, 0)
    w.plcol0(2)
    w.pllab("Year", "Widget Sales (millions)", "#frPLplot Example 12")

    y0 = [5, 15, 12, 24, 28, 30, 20, 8, 12, 3]

    w.plscmap1l(1, pos, red, green, blue)

    for i in range(10):
        # w.plcol0(i + 1)
        w.plcol1(i / 9.0)
        w.plpsty(0)
        fbox(w, (1980. + i), y0[i] )
        string = str(y0[i])
        w.plptex((1980. + i + .5), (y0[i] + 1.), 1.0, 0.0, .5, string)
        string = str(1980 + i)
        w.plmtex("b", 1.0, ((i + 1) * .1 - .05), 0.5, string)

    # Restore defaults
    # cmap1 default color palette.
    w.plspal1("cmap1_default.pal",1)

    # Must be done independently because otherwise this changes output files
    # and destroys agreement with C examples.
    #w.plcol0(1)

def fbox(w,  x0, y0 ):

    x = [x0, x0, x0 + 1., x0 + 1.]
    y = [0., y0, y0, 0.]
    w.plfill(x, y)
    w.plcol0(1)
    w.pllsty(1)
    w.plline(x, y)
