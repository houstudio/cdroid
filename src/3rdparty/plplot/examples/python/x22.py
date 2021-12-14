#  Copyright (C) 2004, 2005, 2006, 2007, 2008 Andrew Ross
#  Copyright (C) 2004-2016 Alan W. Irwin

#  Simple vector plot example.
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

# Pairs of points making the line segments used to plot the user defined arrow
arrow_x = [-0.5, 0.5, 0.3, 0.5, 0.3, 0.5]
arrow_y = [0.0, 0.0, 0.2, 0.0, -0.2, 0.0]
arrow2_x = [-0.5, 0.3, 0.3, 0.5, 0.3, 0.3]
arrow2_y = [0.0, 0.0,   0.2, 0.0, -0.2, 0.0]
xmax = 0.0

def circulation(w):

    nx = 20
    ny = 20

    dx = 1.0
    dy = 1.0

    xmin = -nx/2*dx
    xmax = nx/2*dx
    ymin = -ny/2*dy
    ymax = ny/2*dy

    # Create data - circulation around the origin.
    ix = ones(nx)
    iy = ones(ny)
    x = arange(nx)+0.5-nx/2.0
    y = arange(ny)+0.5-ny/2.0
    xg = multiply.outer(x,iy)
    yg = multiply.outer(ix,y)
    u = yg
    v = -xg

    # Plot vectors with default arrows
    w.plenv(xmin, xmax, ymin, ymax, 0, 0)
    w.pllab("(x)", "(y)", "#frPLplot Example 22 - circulation")
    w.plcol0(2)
    scaling = 0.0
    w.plvect(u,v,scaling,w.pltr2,xg,yg)
    w.plcol0(1)


# Vector plot of flow through a constricted pipe
def constriction( w, astyle ):

    nx = 20
    ny = 20

    dx = 1.0
    dy = 1.0

    xmin = -nx/2*dx
    xmax = nx/2*dx
    ymin = -ny/2*dy
    ymax = ny/2*dy

    Q = 2.0
    ix = ones(nx)
    iy = ones(ny)
    x = (arange(nx)-nx/2+0.5)*dx
    y = (arange(ny)-ny/2+0.5)*dy
    xg = multiply.outer(x,iy)
    yg = multiply.outer(ix,y)
    b = ymax/4.0*(3-cos(pi*x/xmax))
    b2 = multiply.outer(b,iy)
    mask = greater.outer(b,abs(y))
    dbdx = ymax/4.0*(sin(pi*xg/xmax)*pi/xmax*yg/b2)
    u = Q*ymax/b2*mask
    v = dbdx*u

    w.plenv(xmin, xmax, ymin, ymax, 0, 0)
    w.pllab("(x)", "(y)", "#frPLplot Example 22 - constriction (arrow style "+str(astyle)+")")
    w.plcol0(2)
    scaling=-1.0
    w.plvect(u,v,scaling,w.pltr2,xg,yg)
    w.plcol0(1)

def transform( x, y, xt, yt, data ):

    xt[0] = x
    yt[0] = y / 4.0 * ( 3 - cos( pi * x / xmax ) )


# Vector plot of flow through a constricted pipe
def constriction2(w):

    global xmax

    nx = 20
    ny = 20
    nc = 11
    nseg = 20

    dx = 1.0
    dy = 1.0

    xmin = -nx/2*dx
    xmax = nx/2*dx
    ymin = -ny/2*dy
    ymax = ny/2*dy

    w.plstransform( transform, None )

    Q = 2.0
    ix = ones(nx)
    iy = ones(ny)
    x = (arange(nx)-nx/2+0.5)*dx
    y = (arange(ny)-ny/2+0.5)*dy
    xg = multiply.outer(x,iy)
    yg = multiply.outer(ix,y)
    b = ymax/4.0*(3-cos(pi*x/xmax))
    b2 = multiply.outer(b,iy)
    u = Q*ymax/b2
    v = multiply.outer(zeros(nx),iy)

    clev = Q + arange(nc)*Q/(nc-1)

    w.plenv(xmin, xmax, ymin, ymax, 0, 0)
    w.pllab("(x)", "(y)", "#frPLplot Example 22 - constriction with plstransform")

    w.plcol0(2)
    w.plshades(u,xmin+dx/2,xmax-dx/2,ymin+dy/2,ymax-dy/2,clev,0.0,1,1.0,0,None,None)
    scaling=-1.0
    w.plvect(u,v,scaling,w.pltr2,xg,yg)
    w.plpath(nseg,xmin,ymax,xmax,ymax)
    w.plpath(nseg,xmin,ymin,xmax,ymin)
    w.plcol0(1)

    w.plstransform(None,None)

# Vector plot of the gradient of a shielded potential (see example 9)
def potential(w):
    nper = 100
    nlevel = 10
    nr = 20
    ntheta = 20

    # Create data to be contoured
    r = 0.5+arange(nr)
    r.shape = (-1,1)
    theta = (2.*pi/float(ntheta-1))*(0.5+arange(ntheta))
    xg = r*cos(theta)
    yg = r*sin(theta)

    rmax = nr
    xmin = min(xg.flat)
    xmax = max(xg.flat)
    ymin = min(yg.flat)
    ymax = max(yg.flat)

    x = xg
    y = yg

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

    div1 = sqrt((x-d1)**2 + (y-d1)**2 + eps**2)
    div1i = sqrt((x-d1i)**2 + (y-d1i)**2 + eps**2)
    div2 = sqrt((x-d2)**2 + (y+d2)**2 + eps**2)
    div2i = sqrt((x-d2i)**2 + (y+d2i)**2 + eps**2)
    zg = q1/div1 + q1i/div1i + q2/div2 + q2i/div2i
    zmin = min(zg.flat)
    zmax = max(zg.flat)

    ug = (-q1*(x-d1)/div1**3 - q1i*(x-d1i)/div1i**3
          - q2*(x-d2)/div2**3 - q2i*(x-d2i)/div2i**3 )
    vg = (-q1*(y-d1)/div1**3 - q1i*(y-d1i)/div1i**3
          - q2*(y+d2)/div2**3 - q2i*(y+d2i)/div2i**3 )

    umin = min(ug.flat)
    umax = max(ug.flat)
    vmin = min(vg.flat)
    vmax = max(vg.flat)

    w.plenv(xmin, xmax, ymin, ymax, 0, 0)
    w.pllab("(x)", "(y)", "#frPLplot Example 22 - potential gradient vector plot")
    # Plot contours of the potential
    dz = (zmax-zmin)/float(nlevel)
    clevel = zmin + (arange(nlevel)+0.5)*dz
    du = (umax-umin)/float(nlevel)
    clevelu = umin + (arange(nlevel)+0.5)*du
    dv = (vmax-vmin)/float(nlevel)
    clevelv = vmin + (arange(nlevel)+0.5)*dv

    w.plcol0(3)
    w.pllsty(2)
    w.plcont(zg,clevel,w.pltr2,xg,yg)
    w.pllsty(1)
    w.plcol0(1)

    # Plot the vectors of the gradient of the potential
    w.plcol0(2)
    scaling = 25.0
    w.plvect(ug,vg,scaling,w.pltr2,xg,yg)
    w.plcol0(1)

    # Perimeter
    t = (2.*pi/(nper-1))*arange(nper)
    px = rmax*cos(t)
    py = rmax*sin(t)
    w.plline(px,py)

# main
#
# Does a series of vector plots
#
def main(w):

    circulation(w)

    narr = 6
    fill = 0

# Set arrow style using arrow_x and arrow_y then
# plot using these arrows.
    w.plsvect(arrow_x, arrow_y, fill)
    constriction(w, 1)

# Set arrow style using arrow2_x and arrow2_y then
# plot using these filled arrows.
    fill = 1
    w.plsvect(arrow2_x, arrow2_y, fill)
    constriction(w, 2)

    constriction2(w)

    w.plsvect( None, None, 0)

    potential(w)

    # Restore defaults
    # Must be done independently because otherwise this changes output files
    # and destroys agreement with C examples.
    #w.plcol0(1)

