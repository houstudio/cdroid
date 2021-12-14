#  Copyright (C) 2001-2017 Alan W. Irwin

#  Contour plot demo.
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

XPTS = 35
YPTS = 46
XSPA = 2./(XPTS-1)
YSPA = 2./(YPTS-1)
#polar plot data
PERIMETERPTS = 100
RPTS = 40
THETAPTS = 40

#potential plot data
PPERIMETERPTS = 100
PRPTS = 40
PTHETAPTS = 64

tr = array((XSPA, 0.0, -1.0, 0.0, YSPA, -1.0))

def mypltr(x, y, data):
    result0 = data[0] * x + data[1] * y + data[2]
    result1 = data[3] * x + data[4] * y + data[5]
    return array((result0, result1))

def polar(w):
    #polar contour plot example.
    w.plenv(-1., 1., -1., 1., 0, -2,)
    w.plcol0(1)

    # Perimeter
    t = (2.*pi/(PERIMETERPTS-1))*arange(PERIMETERPTS)
    px = cos(t)
    py = sin(t)
    w.plline(px, py)

    # create data to be contoured.
    r = arange(RPTS)/float(RPTS-1)
    r.shape = (-1,1)
    theta = (2.*pi/float(THETAPTS-1))*arange(THETAPTS-1)
    xg = r*cos(theta)
    yg = r*sin(theta)
    zg = r*ones(THETAPTS-1)

    lev = 0.05 + 0.10*arange(10)

    w.plcol0(2)
    w.plcont(zg, lev, w.pltr2, xg, yg, 2)
    #                              ^-- :-).  Means: "2nd coord is wrapped."
    w.plcol0(1)
    w.pllab("", "", "Polar Contour Plot")

def potential(w):
    #shielded potential contour plot example.

    # create data to be contoured.
    r = 0.5 + arange(PRPTS)
    r.shape = (-1,1)
    theta = (2.*pi/float(PTHETAPTS-1))*(0.5+arange(PTHETAPTS-1))
    xg = r*cos(theta)
    yg = r*sin(theta)

    rmax = max(r.flat)
    xmin = min(xg.flat)
    xmax = max(xg.flat)
    ymin = min(yg.flat)
    ymax = max(yg.flat)
    x0 = (xmin + xmax)/2.
    y0 = (ymin + ymax)/2.
    #perturbed (expanded) limits

    peps = 0.05
    xpmin = xmin - abs(xmin)*peps
    xpmax = xmax + abs(xmax)*peps
    ypmin = ymin - abs(ymin)*peps
    ypmax = ymax + abs(ymax)*peps


    # Potential inside a conducting cylinder (or sphere) by method of images.
    # Charge 1 is placed at (d1, d1), with image charge at (d2, d2).
    # Charge 2 is placed at (d1, -d1), with image charge at (d2, -d2).
    # Also put in smoothing term at small distances.

    eps = 2.

    q1 = 1.
    d1 = rmax/4.

    q1i = - q1*rmax/d1
    d1i = rmax**2/d1

    q2 = -1.
    d2 = rmax/4.

    q2i = - q2*rmax/d2
    d2i = rmax**2/d2

    div1 = sqrt((xg-d1)**2 + (yg-d1)**2 + eps**2)
    div1i = sqrt((xg-d1i)**2 + (yg-d1i)**2 + eps**2)

    div2 = sqrt((xg-d2)**2 + (yg+d2)**2 + eps**2)
    div2i = sqrt((xg-d2i)**2 + (yg+d2i)**2 + eps**2)

    zg = q1/div1 + q1i/div1i + q2/div2 + q2i/div2i

    zmin = min(zg.flat)
    zmax = max(zg.flat)
#    print "%.15g %.15g %.15g %.15g %.15g %.15g %.15g %.15g \n" % \
#    (q1, d1, q1i, d1i, q2, d2, q2i, d2i)
#    print "%.15g %.15g %.15g %.15g %.15g %.15g \n" % \
#    (xmin, xmax, ymin, ymax, zmin, zmax)

    # Positive and negative contour levels.
    nlevel = 20
    dz = (zmax-zmin)/float(nlevel)
    clevel = zmin + (arange(20)+0.5)*dz
    clevelpos = compress(clevel > 0., clevel)
    clevelneg = compress(clevel <= 0., clevel)

    #Colours!
    ncollin = 11
    ncolbox = 1
    ncollab = 2

    #Finally start plotting this page!
    w.pladv(0)
    w.plcol0(ncolbox)

    w.plvpas(0.1, 0.9, 0.1, 0.9, 1.0)
    w.plwind(xpmin, xpmax, ypmin, ypmax)
    w.plbox("", 0., 0, "", 0., 0)

    w.plcol0(ncollin)
    # Negative contours
    w.pllsty(2)
    w.plcont(zg, clevelneg, w.pltr2, xg, yg, 2)

    # Positive contours
    w.pllsty(1)
    w.plcont(zg, clevelpos, w.pltr2, xg, yg, 2)


    # Draw outer boundary
    t = (2.*pi/(PPERIMETERPTS-1))*arange(PPERIMETERPTS)
    px = x0 + rmax*cos(t)
    py = y0 + rmax*sin(t)

    w.plcol0(ncolbox)
    w.plline(px, py)

    w.plcol0(ncollab)
    w.pllab("", "", "Shielded potential of charges in a conducting sphere")

def main(w):

    mark = 1500
    space = 1500

    clevel = -1. + 0.2*arange(11)

    xx = (arange(XPTS) - XPTS//2) / float((XPTS//2))
    yy = (arange(YPTS) - YPTS//2) / float((YPTS//2)) - 1.
    xx.shape = (-1,1)
    z = (xx*xx)-(yy*yy)
    # 2.*outerproduct(xx,yy) for new versions of Numeric which have outerproduct.
    w_array = 2.*xx*yy

    # Set up grids.

    # Note *for the given* tr, mypltr(i,j,tr)[0] is only a function of i
    # and mypltr(i,j,tr)[1] is only function of j.
    xg0 = mypltr(arange(XPTS),0,tr)[0]
    yg0 = mypltr(0,arange(YPTS),tr)[1]

    distort = 0.4
    cos_x = cos((pi/2.)*xg0)
    cos_y = cos((pi/2.)*yg0)
    xg1 = xg0 + distort*cos_x
    yg1 = yg0 - distort*cos_y
    # Need independent copy here so the shape changes for xg0t do not affect
    # xg0.
    xg0t = xg0.copy()
    cos_x.shape = (-1,1)
    xg0t.shape = (-1,1)
    xg2 = xg0t + distort*cos_x*cos_y
    yg2 = yg0 - distort*cos_x*cos_y

    # Plot using mypltr (scaled identity) transformation used to create
    # xg0 and yg0
#    w.pl_setcontlabelparam(0.006, 0.3, 0.1, 0)
#    w.plenv(-1.0, 1.0, -1.0, 1.0, 0, 0)
#    w.plcol0(2)
#    w.plcont(z, clevel, mypltr, tr)
#    w.plstyl([mark], [space])
#    w.plcol0(3)
#    w.plcont(w, clevel, mypltr, tr)
#    w.plstyl([], [])
#    w.plcol0(1)
#    w.pllab("X Coordinate", "Y Coordinate", "Streamlines of flow")

    w.pl_setcontlabelformat(4,3)
    w.pl_setcontlabelparam(0.006, 0.3, 0.1, 1)
    w.plenv(-1.0, 1.0, -1.0, 1.0, 0, 0)
    w.plcol0(2)
    w.plcont(z, clevel, mypltr, tr)
    w.plstyl([mark], [space])
    w.plcol0(3)
    w.plcont(w_array, clevel, mypltr, tr)
    w.plstyl([], [])
    w.plcol0(1)
    w.pllab("X Coordinate", "Y Coordinate", "Streamlines of flow")

    # Plot using 1D coordinate transformation.
    w.pl_setcontlabelparam(0.006, 0.3, 0.1, 0)
    w.plenv(-1.0, 1.0, -1.0, 1.0, 0, 0)
    w.plcol0(2)
    w.plcont(z, clevel, w.pltr1, xg1, yg1)
    w.plstyl([mark], [space])
    w.plcol0(3)
    w.plcont(w_array, clevel, w.pltr1, xg1, yg1)
    w.plstyl([], [])
    w.plcol0(1)
    w.pllab("X Coordinate", "Y Coordinate", "Streamlines of flow")

#    w.pl_setcontlabelparam(0.006, 0.3, 0.1, 1)
#    w.plenv(-1.0, 1.0, -1.0, 1.0, 0, 0)
#    w.plcol0(2)
#    w.plcont(z, clevel, w.pltr1, xg1, yg1)
#    w.plstyl([mark], [space])
#    w.plcol0(3)
#    w.plcont(w, clevel, w.pltr1, xg1, yg1)
#    w.plstyl([], [])
#    w.plcol0(1)
#    w.pllab("X Coordinate", "Y Coordinate", "Streamlines of flow")
#    w.pl_setcontlabelparam(0.006, 0.3, 0.1, 0)
#
    # Plot using 2D coordinate transformation.
    w.plenv(-1.0, 1.0, -1.0, 1.0, 0, 0)
    w.plcol0(2)
    w.plcont(z, clevel, w.pltr2, xg2, yg2)
    w.plstyl([mark], [space])
    w.plcol0(3)
    w.plcont(w_array, clevel, w.pltr2, xg2, yg2)
    w.plstyl([], [])
    w.plcol0(1)
    w.pllab("X Coordinate", "Y Coordinate", "Streamlines of flow")

#    w.pl_setcontlabelparam(0.006, 0.3, 0.1, 1)
#    w.plenv(-1.0, 1.0, -1.0, 1.0, 0, 0)
#    w.plcol0(2)
#    w.plcont(z, clevel, w.pltr2, xg2, yg2)
#    w.plstyl([mark], [space])
#    w.plcol0(3)
#    w.plcont(w, clevel, w.pltr2, xg2, yg2)
#    w.plstyl([], [])
#    w.plcol0(1)
#    w.pllab("X Coordinate", "Y Coordinate", "Streamlines of flow")
#
#   polar contour examples.
    w.pl_setcontlabelparam(0.006, 0.3, 0.1, 0)
    polar(w)
#    w.pl_setcontlabelparam(0.006, 0.3, 0.1, 1)
#    polar(w)

#   potential contour examples.
    w.pl_setcontlabelparam(0.006, 0.3, 0.1, 0)
    potential(w)
#    w.pl_setcontlabelparam(0.006, 0.3, 0.1, 1)
#    potential(w)

    # Restore defaults
    w.pl_setcontlabelparam(0.006, 0.3, 0.1, 0)
    # Must be done independently because otherwise this changes output files
    # and destroys agreement with C examples.
    #w.plcol0(1)
