# Simple demo of a 2D line plot.
#
# Copyright (C) 2011-2016 Alan W. Irwin
# Copyright (C) 2012 Andrew Ross
#
# This file is part of PLplot.
#
# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published
# by the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with PLplot; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
#

from numpy import *

NSIZE = 101

def main(w):
    xmin = 0.
    xmax = 1.
    ymin = 0.
    ymax = 100.

    # Prepare data to be plotted.
    x = arange(NSIZE) / float( NSIZE - 1 )
    y = ymax*x**2

    # Create a labelled box to hold the plot.
    w.plenv( xmin, xmax, ymin, ymax, 0, 0 )
    w.pllab( "x", "y=100 x#u2#d", "Simple PLplot demo of a 2D line plot" )

    # Plot the data that was prepared above.
    w.plline( x, y )

    # Restore defaults
    # Must be done independently because otherwise this changes output files
    # and destroys agreement with C examples.
    #w.plcol0(1)

