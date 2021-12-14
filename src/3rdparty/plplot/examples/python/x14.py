#!/usr/bin/env python

#  Copyright (C) 2001-2017 Alan W. Irwin

#  Demo of multiple stream/window capability.
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


# Append to effective python path so that can find plplot modules.
from plplot_python_start import *

import sys
import plplot as w

# Parse and process command line arguments
w.plparseopts(sys.argv, w.PL_PARSE_FULL)

# Simple line plot and multiple windows demo.

from numpy import *

def main(w):

    geometry_master = "500x410+100+200"
    geometry_slave  = "500x410+650+200"
    driver = w.plgdev()
    (fam, num, bmax) = w.plgfam()

    print("Demo of multiple output streams via the %s driver." % driver)
    print("Running with the second stream as slave to the first.")
    print("")

    # Set up the first stream.
    w.plsetopt("geometry", geometry_master)
    w.plsdev(driver)
    w.plssub(2, 2)
    w.plinit()

    # Start next stream.
    w.plsstrm(1)

    # Turn off pause to make this a slave (must follow master)

    w.plsetopt("geometry", geometry_slave)
    w.plspause(0)
    w.plsdev(driver)
    w.plsfam(fam, num, bmax)
    w.plsetopt("fflen","2")
    w.plinit()

    # Set up the data & plot
    # Original case

    w.plsstrm(0)

    xscale = 6.
    yscale = 1.
    xoff = 0.
    yoff = 0.
    plot1(w, xscale, yscale, xoff, yoff)

    # Set up the data & plot

    xscale = 1.
    yscale = 1.e+6
    plot1(w, xscale, yscale, xoff, yoff)

    # Set up the data & plot

    xscale = 1.
    yscale = 1.e-6
    digmax = 2
    w.plsyax(digmax, 0)
    plot1(w, xscale, yscale, xoff, yoff)

    # Set up the data & plot

    xscale = 1.
    yscale = 0.0014
    yoff = 0.0185
    digmax = 5
    w.plsyax(digmax, 0)
    plot1(w, xscale, yscale, xoff, yoff)

    # To slave
    # The w.pleop() ensures the eop indicator gets lit.

    w.plsstrm(1)
    plot4(w)
    w.pleop()

    # Back to master

    w.plsstrm(0)
    plot2(w)
    plot3(w)

    # To slave

    w.plsstrm(1)
    plot5(w)
    w.pleop()

    # Back to master to wait for user to advance

    w.plsstrm(0)
    w.pleop()
    # Restore defaults
    # Probably only need to do that for stream 0, but not quite sure about
    # that and no need to do this currently in any case (since example 14 not used from
    # pyqtdemo or python_demos.py) so don't restore defaults for now.
# ===============================================================

def plot1(w, xscale, yscale, xoff, yoff):

    x = xoff + (xscale/60.)*(1+arange(60))
    y = yoff + yscale*pow(x,2.)

    xmin = x[0]
    xmax = x[59]
    ymin = y[0]
    ymax = y[59]

    xs = x[3::10]
    ys = y[3::10]

    # Set up the viewport and window using w.plenv. The range in X
    # is 0.0 to 6.0, and the range in Y is 0.0 to 30.0. The axes
    # are scaled separately (just = 0), and we just draw a
    # labelled box (axis = 0).

    w.plcol0(1)
    w.plenv(xmin, xmax, ymin, ymax, 0, 0)
    w.plcol0(6)
    w.pllab("(x)", "(y)", "#frPLplot Example 1 - y=x#u2")

    # Plot the data points

    w.plcol0(9)
    w.plpoin(xs, ys, 9)

    # Draw the line through the data

    w.plcol0(4)
    w.plline(x, y)
    w.plflush()

# ===============================================================

def plot2(w):

    # Set up the viewport and window using w.plenv. The range in X
    # is -2.0 to 10.0, and the range in Y is -0.4 to 2.0. The axes
    # are scaled separately (just = 0), and we draw a box with
    # axes (axis = 1).

    w.plcol0(1)
    w.plenv(-2.0, 10.0, -0.4, 1.2, 0, 1)
    w.plcol0(2)
    w.pllab("(x)", "sin(x)/x", "#frPLplot Example 1 - Sinc Function")

    # Fill up the arrays

    x = (arange(100)-19)/6.0
    if 0.0 in x:
        #replace 0.0 by small value that gives the same sinc(x) result.
        x[list(x).index(0.0)] = 1.e-30
    y = sin(x)/x

    # Draw the line

    w.plcol0(3)
    w.plline(x, y)
    w.plflush()

# ===============================================================

def plot3(w):

    # For the final graph we wish to override the default tick
    # intervals, so do not use w.plenv

    w.pladv(0)

    # Use standard viewport, and define X range from 0 to 360
    # degrees, Y range from -1.2 to 1.2.

    w.plvsta()
    w.plwind(0.0, 360.0, -1.2, 1.2)

    # Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y.

    w.plcol0(1)
    w.plbox("bcnst", 60.0, 2, "bcnstv", 0.2, 2)

    # Superimpose a dashed line grid, with 1.5 mm marks and spaces.

    w.plstyl([1500], [1500])
    w.plcol0(2)
    w.plbox("g", 30.0, 0, "g", 0.2, 0)
    w.plstyl([], [])

    w.plcol0(3)
    w.pllab("Angle (degrees)", "sine", "#frPLplot Example 1 - Sine function")

    x = 3.6*arange(101)
    y = sin((pi/180.)*x)

    w.plcol0(4)
    w.plline(x, y)
    w.plflush()

# ===============================================================

def plot4(w):

    dtr = pi / 180.0
    x0 = cos(dtr*arange(361))
    y0 = sin(dtr*arange(361))

    # Set up viewport and window, but do not draw box

    w.plenv(-1.3, 1.3, -1.3, 1.3, 1, -2)

    i = 0.1*arange(1,11)
    #outerproduct(i,x0) and outerproduct(i,y0) is what we are
    #mocking up here since old Numeric version does not have outerproduct.
    i.shape = (-1,1)
    x=i*x0
    y=i*y0

    # Draw circles for polar grid
    for i in range(10):
        w.plline(x[i], y[i])

    w.plcol0(2)
    for i in range(12):
        theta = 30.0 * i
        dx = cos(dtr * theta)
        dy = sin(dtr * theta)

        # Draw radial spokes for polar grid

        w.pljoin(0.0, 0.0, dx, dy)

        # Write labels for angle

        text = str(int(theta))
#Slightly off zero to avoid floating point logic flips at 90 and 270 deg.
        if dx >= -0.00001:
            w.plptex(dx, dy, dx, dy, -0.15, text)
        else:
            w.plptex(dx, dy, -dx, -dy, 1.15, text)

    # Draw the graph

    r = sin((dtr*5.)*arange(361))
    x = x0*r
    y = y0*r

    w.plcol0(3)
    w.plline(x, y)

    w.plcol0(4)
    w.plmtex("t", 2.0, 0.5, 0.5, "#frPLplot Example 3 - r(#gh)=sin 5#gh")
    w.plflush()

# ===============================================================

XPTS = 35
YPTS = 46
XSPA = 2./(XPTS-1)
YSPA = 2./(YPTS-1)

tr = array((XSPA, 0.0, -1.0, 0.0, YSPA, -1.0))

def mypltr(x, y, data):
    result0 = data[0] * x + data[1] * y + data[2]
    result1 = data[3] * x + data[4] * y + data[5]
    return array((result0, result1))

def plot5(w):

    mark = 1500
    space = 1500

    clevel = -1. + 0.2*arange(11)

    xx = (arange(XPTS) - XPTS//2) / float((XPTS//2))
    yy = (arange(YPTS) - YPTS//2) / float((YPTS//2)) - 1.
    xx.shape = (-1,1)
    z = (xx*xx)-(yy*yy)
    # 2.*outerproduct(xx,yy) for new versions of Numeric which have outerproduct.
    w_array = 2.*xx*yy

    w.plenv(-1.0, 1.0, -1.0, 1.0, 0, 0)
    w.plcol0(2)
    w.plcont(z, clevel, mypltr, tr)
    w.plstyl([mark], [space])
    w.plcol0(3)
    w.plcont(w_array, clevel, mypltr, tr)
    w.plstyl([], [])
    w.plcol0(1)
    w.pllab("X Coordinate", "Y Coordinate", "Streamlines of flow")
    w.plflush()

# ===============================================================

main(w)
w.plend()
