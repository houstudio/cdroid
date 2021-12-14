#  Copyright (C) 2001-2016 Alan W. Irwin

#  Histogram demo.
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

NPTS = 2047

# main
#
# Draws a histogram from sample data.

def main(w):

    # Fill up data points

    delta = 2.0 * pi / NPTS
    data = sin(delta*arange(NPTS))

    w.plcol0(1)
    w.plhist(data, -1.1, 1.1, 44, 0)
    w.plcol0(2)
    w.pllab("#frValue", "#frFrequency",
           "#frPLplot Example 5 - Probability function of Oscillator")

    # Restore defaults
    # Must be done independently because otherwise this changes output files
    # and destroys agreement with C examples.
    #w.plcol0(1)
