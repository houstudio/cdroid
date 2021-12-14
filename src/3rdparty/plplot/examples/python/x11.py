#  Copyright (C) 2001-2017 Alan W. Irwin

#  Mesh plot demo.
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
opt = [3, 3]

alt = [33.0, 17.0]

az = [24.0, 115.0]

title = ["#frPLplot Example 11 - Alt=33, Az=24, Opt=3",
         "#frPLplot Example 11 - Alt=17, Az=115, Opt=3"]

# Routine for defining a specific color map 1 in HLS space.
# if gray is true, use basic grayscale variation from half-dark to light.
# otherwise use false color variation from blue (240 deg) to red (360 deg).
def cmap1_init(w, gray):
    # Independent variable of control points.
    i = array((0., 1.))
    if gray:
        # Hue for control points.  Doesn't matter since saturation is zero.
        h = array((0., 0.))
        # Lightness ranging from half-dark (for interest) to light.
        l = array((0.5, 1.))
        # Gray scale has zero saturation
        s = array((0., 0.))
    else:
        # Hue ranges from blue (240 deg) to red (0 or 360 deg)
        h = array((240., 0.))
        # Lightness and saturation are constant (values taken from C example).
        l = array((0.6, 0.6))
        s = array((0.8, 0.8))

    # number of cmap1 colours is 256 in this case.
    w.plscmap1n(256)
    # Interpolate between control points to set up cmap1.
    w.plscmap1l(0, i, h, l, s)
# main
#
# Does a series of mesh plots for a given data set, with different
# viewing options in each plot.

def main(w):

    x = 3.*(arange(XPTS) - (XPTS // 2)) / float(XPTS // 2)
    y = 3.*(arange(YPTS) - (YPTS // 2)) / float(YPTS // 2)
    x.shape = (-1,1)
    z = 3. * (1.-x)*(1.-x) * exp(-(x*x) - (y+1.)*(y+1.)) - \
        10. * (x/5. - pow(x,3.) - pow(y,5.)) * exp(-x*x-y*y) - \
        1./3. * exp(-(x+1)*(x+1) - (y*y))
#    if 0: #Jungfraujoch/Interlaken
    # Not sure this is correct coding for truncating at -1, but
    # not activated anyway so ignore this question for now.
#       z = max(z,-1)
    x.shape = (-1,)

    zmin = min(z.flat)
    zmax = max(z.flat)
    nlevel = 10
    step = (zmax-zmin)/(nlevel+1)
    clevel = zmin + step + arange(nlevel)*step
    cmap1_init(w, 0)
    for k in range(2):
        for i in range(4):
            w.pladv(0)
            w.plcol0(1)
            w.plvpor(0.0, 1.0, 0.0, 0.9)
            w.plwind(-1.0, 1.0, -1.0, 1.5)
            w.plw3d(1.0, 1.0, 1.2, -3.0, 3.0, -3.0, 3.0, zmin, zmax, alt[k], az[k])
            w.plbox3("bnstu", "x axis", 0.0, 0,
                "bnstu", "y axis", 0.0, 0,
                "bcdmnstuv", "z axis", 0.0, 4)
            w.plcol0(2)

            #wireframe plot
            if i==0:
                w.plmesh(x, y, z, opt[k])

            # magnitude colored wireframe plot
            elif i==1:
                w.plmesh(x, y, z, opt[k] | w.MAG_COLOR)

            # magnitude colored wireframe plot with sides
            elif i==2:
                w.plot3d(x, y, z, opt[k] | w.MAG_COLOR, 1)

            # magnitude colored wireframe plot with base contour
            elif i==3:
                w.plmeshc(x, y, z, opt[k] | w.MAG_COLOR | w.BASE_CONT, clevel)

            w.plcol0(3)
            w.plmtex("t", 1.0, 0.5, 0.5, title[k])

    # Restore defaults
    # cmap1 default color palette.
    w.plspal1("cmap1_default.pal",1)

    # Must be done independently because otherwise this changes output files
    # and destroys agreement with C examples.
    #w.plcol0(1)
