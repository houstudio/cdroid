#  Copyright (C) 2007, 2008 Andrew Ross
#  Copyright (C) 2007-2016 Alan W. Irwin


#  Filling and clipping polygons.
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
# Test program for filling polygons and proper clipping

def main(w):

    w.pladv(0)
    w.plssub(3,3)

    xextreme = [ [ -120.0,  120.0 ],
                 [ -120.0,  120.0 ],
                 [ -120.0,  120.0 ],
                 [  -80.0,   80.0 ],
                 [ -220.0, -120.0 ],
                 [  -20.0,   20.0 ],
                 [  -20.0,   20.0 ],
                 [  -80.0,   80.0 ],
                 [   20.0,  120.0 ] ]

    yextreme = [ [ -120.0,  120.0 ],
                 [   20.0,  120.0 ],
                 [  -20.0,  120.0 ],
                 [  -20.0,  120.0 ],
                 [ -120.0,  120.0 ],
                 [ -120.0,  120.0 ],
                 [  -20.0,   20.0 ],
                 [  -80.0,   80.0 ],
                 [ -120.0,  120.0 ] ]

    for k in range(2):
        for j in range(4):

            # Polygon 1: a diamond
            if (j == 0):
                x0 = [0, -100, 0, 100]
                y0 = [-100, 0, 100, 0]
            # Polygon 2: a diamond - reverse direction
            elif (j == 1):
                x0 = [100, 0, -100, 0]
                y0 = [0, 100, 0, -100]
            # Polygon 3: a square with punctures
            elif (j == 2):
                x0 = [-100, -100, 80, -100, -100, -80, 0, 80, 100, 100]
                y0 = [-100, -80, 0, 80, 100, 100, 80, 100, 100, -100]
            # Polygon 4: a square with punctures - reverse direction
            elif (j == 3):
                x0 = [100, 100, 80, 0, -80, -100, -100, 80, -100, -100]
                y0 = [-100, 100, 100, 80, 100, 100, 80, 0, -80, -100]

            for i in range(9):
                w.pladv(0)
                w.plvsta()
                w.plwind(xextreme[i][0], xextreme[i][1], yextreme[i][0], yextreme[i][1])

                w.plcol0(2)
                w.plbox("bc", 1.0, 0, "bcnv", 10.0, 0)
                w.plcol0(1)
                w.plpsty(0)
                if (k == 0):
                    w.plfill(x0,y0)
                else:
                    w.plgradient(x0, y0, 45.)
                w.plcol0(2)
                w.pllsty(1)
                w.plline(x0,y0)

    # Restore defaults
    w.plssub(1, 1)
    w.pleop()

    # Must be done independently because otherwise this changes output files
    # and destroys agreement with C examples.
    #w.plcol0(1)
