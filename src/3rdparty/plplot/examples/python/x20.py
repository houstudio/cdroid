#  Copyright (C) 2007, 2008 Andrew Ross
#  Copyright (C) 2007-2016 Alan W. Irwin

#  plimage demo
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

import os.path
import sys

XDIM = 260
YDIM = 220

dbg = 0
nosombrero = 0
nointeractive = 0
f_name = ""

def gray_cmap(w, num_col):
    r = [0.0, 1.0]
    g = [0.0, 1.0]
    b = [0.0, 1.0]
    pos = [0.0, 1.0]

    w.plscmap1n(num_col)
    w.plscmap1l(1, pos, r, g, b)

def save_plot(w, fname):
    # Get current stream and create a new one
    cur_strm = w.plgstrm()
    new_strm = w.plmkstrm()

    # Set new device type and file name - use a known existing driver
    w.plsdev("psc")
    w.plsfnam(fname)

    # Copy old stream parameters to new stream, do the save,
    # then close new device.
    w.plcpstrm(cur_strm,0)
    w.plreplot()
    w.plend1()

    # Return to previous stream
    w.plsstrm(cur_strm)

def read_img(fname):

    if (not os.path.exists(fname)):
        return [1,[],0,0,0]

    fp = open(fname, mode='rb')

    # Check correct version
    ver = fp.readline().decode()

    if (ver != "P5\n"):
        fp.close()
        return [1,[],0,0,0]

    # Skip over comments
    ptr = fp.tell()
    while ( fp.read(1) == '#' ):
        fp.readline()
        ptr = fp.tell()
    fp.seek(ptr)

    # Get width, height, num colors
    [w, h] = fp.readline().split()
    w = int(w)
    h = int(h)
    nc = int(fp.readline())

    tmp = bytearray(fp.read(w*h))

    img = zeros(w*h)
    for i in range(w):
        for j in range(h):
            img[i*h+j] = tmp[(h-1-j)*w+i]

    img = reshape(img,[w,h])


    fp.close()
    return [0,img,w,h,nc]

def get_clip(w, xi, xe, yi, ye):

    sx = zeros(5)
    sy = zeros(5)

    start = 0

    st = w.plxormod(1)

    if (st):
        gin = w.PLGraphicsIn()
        gin.button = 0
        while (1):
            st = w.plxormod(0)
            w.plGetCursor(gin)
            st = w.plxormod(1)

            if (gin.button == 1):
                xi = gin.wX
                yi = gin.wY
                if (start):
                    w.plline(sx,sy)  # Clear previous rectangle

                start = 0
                sx[0] = xi
                sy[0] = yi
                sx[4] = xi
                sy[4] = yi

            if (gin.state & 0x100):
                xe = gin.wX
                ye = gin.wY
                if (start):
                    w.plline(sx,sy)

                start = 1

                sx[2] = xe
                sy[2] = ye
                sx[1] = xe
                sy[1] = yi
                sx[3] = xi
                sy[3] = ye

                w.plline(sx,sy)  # Draw new rectangle

            if (gin.button == 3 or gin.keysym == 0x0D or gin.keysym == 'Q'):
                if (start):
                    w.plline(sx,sy) # Clear previous rectangle
                break

        st = w.plxormod(0)  # Leave xor mode

        if (xe < xi):
            t = xi
            xi = xe
            xe = t

        if (yi < ye):
            t = yi
            yi = ye
            ye = t

        return [gin.keysym == 'Q', xi, xe, yi, ye]

    else:  # Driver has no xormod capability, just do nothing
        return [0, xi, xe, yi, ye]

def mypltr(x, y, stretch):
    x0 = (stretch[0] + stretch[1])*0.5
    y0 = (stretch[2] + stretch[3])*0.5
    dy = (stretch[3] - stretch[2])*0.5
    result0 = x0 + multiply.outer((x0-x),(1.0 - stretch[4]*cos((y-y0)/dy*pi*0.5)))
    result1 = multiply.outer(ones(len(x)),y)
    return array((result0, result1))

# main
#
#
def main(w):

    z = reshape(zeros(XDIM*YDIM),[XDIM,YDIM])

    # View image border pixels
    if (dbg):
        w.plenv(1.0, XDIM, 1.0, YDIM, 1, 1)

        for i in range(XDIM):
            z[i][YDIM-1] = 1.0
            z[i][0] = 1.0

        for j in range(YDIM):
            z[0][j] = 1.0
            z[XDIM-1][j] = 1.0


        w.pllab("...around a blue square."," ","A ref border should appear...")

        w.plimage(z, 1.0, XDIM, 1.0, YDIM, 0.0, 0.0, 1.0, XDIM, 1.0, YDIM)

    if (not nosombrero):
        w.plcol0(2)  # Draw a yellow box, useful for diagnostics!
        w.plenv(0.0, 2.0*pi, 0, 3.0*pi, 1, -1)


        x = arange(XDIM)*2.0*pi/(XDIM-1)
        y = arange(YDIM)*3.0*pi/(YDIM-1)

        r = sqrt( multiply.outer(x*x,ones(YDIM)) + multiply.outer(ones(XDIM),y*y)) + 1e-3
        z = sin(r) / r

        w.pllab("No, an amplitude clipped \"sombrero\"", "", "Saturn?")
        w.plptex(2., 2., 3., 4., 0., "Transparent image")
        w.plimage(z, 0., 2.*pi, 0, 3.*pi, 0.05, 1.,0., 2.*pi, 0, 3.*pi)

        # Save the plot
        if (f_name != ""):
            save_plot(w, f_name)

    # Read Chloe image
    # Note: we try two different locations to cover the case where
    # this example is being run from the test_c.sh script
    [err, img, width, height, num_col] = read_img("Chloe.pgm")
    if (err):
        [err, img, width, height, num_col] = read_img("../Chloe.pgm")
        if (err):
            sys.stderr.write("Failed to find Chloe.pgm\n")
            w.plend()
            sys.exit(1)

    # Set gray colormap
    gray_cmap(w, num_col)

    # Display Chloe
    w.plenv(1., width, 1., height, 1, -1)

    if (not nointeractive):
        w.pllab("Set and drag Button 1 to (re)set selection, Button 2 to finish."," ","Chloe...")
    else:
        w.pllab(""," ","Chloe...")

    w.plimage(img, 1., width, 1., height, 0., 0., 1., width, 1., height)

    # selection/expansion demo
    if (not nointeractive):
        xi = 25.
        xe = 130.
        yi = 235.
        ye = 125.

        [err, xi, xe, yi, ye] = get_clip(w, xi, xe, yi, ye)
        if (err):
            w.plend()
            sys.exit(0)

        w.plspause(0)
        w.pladv(0)

        # display selection only
        w.plimage(img, 1., width, 1., height, 0., 0., xi, xe, ye, yi)

        w.plspause(1)

        # zoom in selection
        w.plenv(xi, xe, ye, yi, 1, -1)
        w.plimage(img, 1., width, 1., height, 0., 0., xi, xe, ye, yi)

    # Base the dynamic range on the image contents.
    img_min = min(img.flat)
    img_max = max(img.flat)

    # Draw a saturated version of the original image.  Only use the middle 50%
    # of the image's full dynamic range.
    w.plcol0(2)
    w.plenv(0, width, 0, height, 1, -1)
    w.pllab("", "", "Reduced dynamic range image example")
    w.plimagefr(img, 0., width, 0., height, 0., 0., img_min + img_max * 0.25, \
              img_max - img_max * 0.25)

    # Draw a distorted version of the original image, showing its full dynamic range.
    w.plenv(0, width, 0, height, 1, -1)
    w.pllab("", "", "Distorted image example")

    stretch = zeros(5)
    stretch[1] = width
    stretch[3] = height
    stretch[4] = 0.5

    xg = mypltr(arange(width+1),arange(height+1),stretch)[0]
    yg = mypltr(arange(width+1),arange(height+1),stretch)[1]

    w.plimagefr(img, 0., width, 0., height, 0., 0., img_min, img_max, \
              w.pltr2, xg, yg)

    # Restore defaults
    # cmap1 default color palette.
    w.plspal1("cmap1_default.pal",1)

    # Must be done independently because otherwise this changes output files
    # and destroys agreement with C examples.
    #w.plcol0(1)
