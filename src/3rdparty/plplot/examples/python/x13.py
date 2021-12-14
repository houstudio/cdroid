#  Copyright (C) 2001-2016 Alan W. Irwin

#  Pie chart demo.
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

text = ["Maurice",
        "Geoffrey",
        "Alan",
        "Rafael",
        "Vince"]

# main
#
# Does a simple pie chart.

def main(w):

    w.pladv(0)
    # Ensure window has aspect ratio of one so circle is
    # plotted as a circle.
    w.plvasp(1.0)
    w.plwind(0., 10., 0., 10.)
    #w.plenv(0., 10., 0., 10., 1, -2)
    w.plcol0(2)
    #if these don't add up to 100, the logic adds to the last to make
    #up the balance.
    per = [10., 32., 12., 30., 16.]
    theta0 = 0

    for i in range(5):
        #theta quantities scaled as integers to make logic independent of
        #floating point considerations.
        #n.b. theta multiplied by 2 pi/500 afterward so per is in fact
        #interpreted as a percentage.
        theta1 = theta0 + int(5.*per[i])
        if i == 4:
            theta1 = 500

        theta = arange(theta0,theta1+1)
        x = concatenate((array([5.]), (5. + 3.*cos((2.*pi/500.)*theta))))
        y = concatenate((array([5.]), (5. + 3.*sin((2.*pi/500.)*theta))))

        w.plcol0(i + 1)
        w.plpsty((i + 3) % 8 + 1)
        w.plfill(x, y)
        w.plcol0(1)
        w.plline(x, y)
        just = (2.*pi/500.)*(theta0 + theta1) / 2.
        dx = .25 * cos(just)
        dy = .25 * sin(just)
        if (theta0 + theta1) < 250 or (theta0 + theta1) > 750:
            just = 0.
        else:
            just = 1.

        mid = int(len(x) / 2)
        w.plptex((x[mid] + dx), (y[mid] + dy), 1.0, 0.0, just, text[i])
        theta0 = theta[-1]

    w.plfont(2)
    w.plschr(0., 1.3)
    w.plptex(5.0, 9.0, 1.0, 0.0, 0.5, "Percentage of Sales")

    # Restore defaults
    w.plschr( 0.0, 1.0 )
    w.plfont(1)

    # Must be done independently because otherwise this changes output files
    # and destroys agreement with C examples.
    #w.plcol0(1)
