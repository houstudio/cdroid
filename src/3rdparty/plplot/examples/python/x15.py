#  Copyright (C) 2001-2017 Alan W. Irwin

#  Shade plot demo.
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

XPTS = 35               # Data points in x
YPTS = 46               # Data points in y


def main(w):
# Does a variety of shade plots with discrete colours and patterns.

    # Do not fiddle with cmap1 since this example actually uses cmap0.
    # Furthermore, if you do fiddle with cmap1, it will screw up other
    # plots (unless you return to default cmap1 like is done with eighth
    # example.
    x = (arange(XPTS) - (XPTS // 2)) / float(XPTS // 2)
    y = ((arange(YPTS) - (YPTS // 2)) / float(YPTS // 2)) - 1.
    x.shape = (-1,1)
    z = x*x - y*y + (x - y)/(x*x+y*y + 0.1)
    x.shape = (-1,)
    zmin = min(z.flat)
    zmax = max(z.flat)

    plot1(w, z, zmin, zmax)
    plot2(w, z, zmin, zmax)
    plot3(w)

    # Restore defaults
    # Must be done independently because otherwise this changes output files
    # and destroys agreement with C examples.
    #w.plcol0(1)

def plot1(w, z, zmin, zmax):
# Illustrates a single shaded region

    w.pladv(0)
    w.plvpor(0.1, 0.9, 0.1, 0.9)
    w.plwind(-1.0, 1.0, -1.0, 1.0)

    shade_min = zmin + (zmax-zmin)*0.4
    shade_max = zmin + (zmax-zmin)*0.6
    sh_cmap = 0
    sh_color = 7
    sh_width = 2.
    min_color = 9
    min_width = 2.
    max_color = 2
    max_width = 2.

    w.plpsty(8)

    # Just use identity transform on indices of z mapped to -1, 1 range
    # in X and Y coordinates
    w.plshade( z, -1., 1., -1., 1.,
    shade_min, shade_max, sh_cmap, sh_color, sh_width,
    min_color, min_width, max_color, max_width, 1, None, None)

    w.plcol0(1)
    w.plbox("bcnst", 0.0, 0, "bcnstv", 0.0, 0)
    w.plcol0(2)
    w.pllab("distance", "altitude", "Bogon flux")

def plot2(w, z, zmin, zmax):
# Illustrates multiple adjacent shaded regions, using different fill
# patterns for each region.

    nlin = array( [1, 1, 1, 1, 1, 2, 2, 2, 2, 2] )
    inc = array( [ [450, 0], [-450, 0], [0, 0], [900, 0],
                 [300, 0], [450,-450], [0, 900], [0, 450],
                 [450, -450], [0, 900] ] )
    spa = array( [ [2000, 2000], [2000, 2000], [2000, 2000],
                 [2000, 2000], [2000, 2000], [2000, 2000],
                 [2000, 2000], [2000, 2000], [4000, 4000],
                 [4000, 2000] ] )

    w.pladv(0)
    w.plvpor(0.1, 0.9, 0.1, 0.9)
    w.plwind(-1.0, 1.0, -1.0, 1.0)

    sh_cmap = 0
    sh_width = 2.
    min_color = 0
    min_width = 0.
    max_color = 0
    max_width = 0.

    for i in range(10):
        shade_min = zmin + (zmax - zmin) * i / 10.0
        shade_max = zmin + (zmax - zmin) * (i +1) / 10.0
        sh_color = i+6
        n = nlin[i]
        w.plpat(inc[i][0:n], spa[i][0:n])

        # Just use identity transform on indices of z mapped to -1, 1 range
        # in X and Y coordinates
        w.plshade( z, -1., 1., -1., 1.,
        shade_min, shade_max, sh_cmap, sh_color, sh_width,
        min_color, min_width, max_color, max_width, 1, None, None)

    w.plcol0(1)
    w.plbox("bcnst", 0.0, 0, "bcnstv", 0.0, 0)
    w.plcol0(2)
    w.pllab("distance", "altitude", "Bogon flux")

def plot3(w):
# Illustrates shaded regions in 3d, using a different fill pattern for
# each region.
    xx = array( [ [-1.0, 1.0, 1.0, -1.0, -1.0],
                  [-1.0, 1.0, 1.0, -1.0, -1.0] ] )
    yy = array( [ [1.0, 1.0, 0.0, 0.0, 1.0],
                [-1.0, -1.0, 0.0, 0.0, -1.0] ] )
    zz = array( [ [0.0, 0.0, 1.0, 1.0, 0.0],
                  [0.0, 0.0, 1.0, 1.0, 0.0] ] )

    w.pladv(0)
    w.plvpor(0.1, 0.9, 0.1, 0.9)
    w.plwind(-1.0, 1.0, -1.0, 1.0)
    w.plw3d(1., 1., 1., -1.0, 1.0, -1.0, 1.0, 0.0, 1.5, 30, -40)

    # Plot using identity transform

    w.plcol0(1)
    w.plbox3("bntu", "X", 0.0, 0, "bntu", "Y", 0.0, 0, "bcdfntu", "Z", 0.5, 0)
    w.plcol0(2)
    w.pllab("","","3-d polygon filling")

    w.plcol0(3)
    w.plpsty(1)
    w.plline3(xx[0], yy[0], zz[0])
    w.plfill3(xx[0][0:4], yy[0][0:4], zz[0][0:4])
    w.plpsty(2)
    w.plline3(xx[1], yy[1], zz[1])
    w.plfill3(xx[1][0:4], yy[1][0:4], zz[1][0:4])
