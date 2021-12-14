#  Copyright (C) 2001-2016 Alan W. Irwin

#  Simple line plot and multiple windows demo.
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
# Generates several simple line plots.  Demonstrates:
#   - subwindow capability
#   - setting up the window, drawing plot, and labelling
#   - changing the color
#   - automatic axis rescaling to exponential notation
#   - placing the axes in the middle of the box
#   - gridded coordinate axes

def main(w):

    # For starting from scratch this call to pladv increments cursub, but
    # then the following plssub sets it to zero so the whole thing is
    # essentially a nop.  However, for the case when other examples are run
    # first, this call to pladv is absolutely essential to finish the
    # preceding page.
    w.pladv(0)
    # Do plots on 4 subwindows of the first page
    w.plssub(2, 2)

    # Do a plot with one range of data

    plot1(w, 6., 1., 0., 0.)

    # Do a plot with another range of data

    digmax = 5
    w.plsyax(digmax, 0)
    plot1(w, 1., 0.0014, 0., 0.0185)

    plot2(w)

    plot3(w)

    # Restore defaults
    w.plssub(1, 1)
    w.pleop()
    # Must be done independently because otherwise this changes output files
    # and destroys agreement with C examples.
    #w.plcol0(1)

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

    # Set up the viewport and window using pl.env. The range in X
    # is 0.0 to 6.0, and the range in Y is 0.0 to 30.0. The axes
    # are scaled separately (just = 0), and we just draw a
    # labelled box (axis = 0).

    w.plcol0(1)
    w.plenv(xmin, xmax, ymin, ymax, 0, 0)
    w.plcol0(2)
    w.pllab("(x)", "(y)", "#frPLplot Example 1 - y=x#u2")

    # Plot the data points

    w.plcol0(4)
    w.plpoin(xs, ys, 9)

    # Draw the line through the data

    w.plcol0(3)
    w.plline(x, y)

# ===============================================================

def plot2(w):

    # Set up the viewport and window using pl.env. The range in X
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
    w.plwidth(2)
    w.plline(x, y)
    w.plwidth(1)

# ===============================================================

def plot3(w):

    # For the final graph we wish to override the default tick
    # intervals, so do not use pl.env

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
