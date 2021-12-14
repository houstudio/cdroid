# -*- coding: utf-8; -*-

#  Copyright (C) 2001-2016 Alan W. Irwin

#  3-d line and point plot demo.
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

opt = [1, 0, 1, 0]

alt = [20.0, 35.0, 50.0, 65.0]

az = [30.0, 40.0, 50.0, 60.0]

# main
#
# Does a series of 3-d plots for a given data set, with different
# viewing options in each plot.

NPTS = 1000

def main(w):

        for k in range(4):
                test_poly(w, k)

        # From the mind of a sick and twisted physicist...

        z = -1. + (2./NPTS) * arange(NPTS)
        x = z*cos((2.*pi*6./NPTS)*arange(NPTS))
        y = z*sin((2.*pi*6./NPTS)*arange(NPTS))

        for k in range(4):
                w.pladv(0)
                w.plvpor(0.0, 1.0, 0.0, 0.9)
                w.plwind(-1.0, 1.0, -0.9, 1.1)
                w.plcol0(1)
                w.plw3d(1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0,
                        alt[k], az[k])
                w.plbox3("bnstu", "x axis", 0.0, 0,
                         "bnstu", "y axis", 0.0, 0,
                         "bcdmnstuv", "z axis", 0.0, 0)

                w.plcol0(2)

                if opt[k]:
                        w.plline3(x, y, z)
                else:
                        # U+22C5 DOT OPERATOR.
                        w.plstring3(x, y, z, "⋅")

                w.plcol0(3)
                title = "#frPLplot Example 18 - Alt=%.0f, Az=%.0f" % (alt[k], az[k])
                w.plmtex("t", 1.0, 0.5, 0.5, title)

        # Restore defaults
        # Must be done independently because otherwise this changes output files
        # and destroys agreement with C examples.
        #w.plcol0(1)

def THETA(a):
    return 2. * pi * (a) / 20.

def PHI(a):
    return pi * (a) / 20.1

def test_poly(w, k):

        draw = [ [ 1, 1, 1, 1 ],
                 [ 1, 0, 1, 0 ],
                 [ 0, 1, 0, 1 ],
                 [ 1, 1, 0, 0 ] ]

        w.pladv(0)
        w.plvpor(0.0, 1.0, 0.0, 0.9)
        w.plwind(-1.0, 1.0, -0.9, 1.1)
        w.plcol0(1)
        w.plw3d(1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, alt[k], az[k])
        w.plbox3("bnstu", "x axis", 0.0, 0,
                 "bnstu", "y axis", 0.0, 0,
                 "bcdmnstuv", "z axis", 0.0, 0)

        w.plcol0(2)

#      x = r sin(phi) cos(theta)
#      y = r sin(phi) sin(theta)
#      z = r cos(phi)
#      r = 1 :=)

        cosi0 = cos(THETA(arange(20)))
        cosi1 = cos(THETA(arange(1,21)))
        sini0 = sin(THETA(arange(20)))
        sini1 = sin(THETA(arange(1,21)))
        cosi0.shape = (-1,1)
        cosi1.shape = (-1,1)
        sini0.shape = (-1,1)
        sini1.shape = (-1,1)
        cosj0 = cos(PHI(arange(20)))
        cosj1 = cos(PHI(arange(1,21)))
        sinj0 = sin(PHI(arange(20)))
        sinj1 = sin(PHI(arange(1,21)))

        x0 = cosi0*sinj0
        y0 = sini0*sinj0
        z0 = cosj0

        x1 = cosi0*sinj1
        y1 = sini0*sinj1
        z1 = cosj1

        x2 = cosi1*sinj1
        y2 = sini1*sinj1
        z2 = cosj1

        x3 = cosi1*sinj0
        y3 = sini1*sinj0
        z3 = cosj0

        x4 = x0
        y4 = y0
        z4 = z0

        for i in range(20):
                for j in range(20):

                        x = [x0[i,j],x1[i,j],x2[i,j],x3[i,j],x4[i,j]]
                        y = [y0[i,j],y1[i,j],y2[i,j],y3[i,j],y4[i,j]]
                        z = [z0[j],z1[j],z2[j],z3[j],z4[j]]

                        # Since negative dimensions don't make sense here
                        # to specify that points are to be drawn in
                        # counter-clockwise direction (as in x18c.c and
                        # x18.tcl) this must be specified with an optional
                        # extra argument in python API.
                        w.plpoly3(x, y, z, draw[k], 1)

        w.plcol0(3)
        w.plmtex("t", 1.0, 0.5, 0.5, "unit radius sphere" )
