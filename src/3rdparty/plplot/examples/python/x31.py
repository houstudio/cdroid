#!/usr/bin/env python

#  Copyright (C) 2008-2016 Alan W. Irwin

#  Set/get tester.
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

# Test setting / getting familying parameters before w.plinit
# Save values set by w.plparseopts to be restored later.
(fam0, num0, bmax0) = w.plgfam()
fam1 = False
num1 = 10
bmax1 = 1000
w.plsfam(fam1, num1, bmax1)

# Retrieve the same values?
(fam2, num2, bmax2) = w.plgfam()
sys.stdout.write("family parameters: fam, num, bmax = %d %d %d\n" % (fam2, num2, bmax2))
if fam2 != fam1  or num2 != num1 or bmax2 != bmax1:
    sys.stderr.write("plgfam test failed\n")
    status = 1
# Restore values set initially by w.plparseopts.
w.plsfam(fam0, num0, bmax0)

# Test setting / getting page parameters before w.plinit.
# Save values set by w.plparseopts to be restored later.
(xp0, yp0, xleng0, yleng0, xoff0, yoff0) = w.plgpage()
xp1 = 200.
yp1 = 200.
xleng1 = 400
yleng1 = 200
xoff1 = 10
yoff1 = 20
w.plspage(xp1, yp1, xleng1, yleng1, xoff1, yoff1)
(xp2, yp2, xleng2, yleng2, xoff2, yoff2) = w.plgpage()
sys.stdout.write("page parameters: xp, yp, xleng, yleng, xoff, yoff = %f %f %d %d %d %d\n" % (xp2, yp2, xleng2, yleng2, xoff2, yoff2))
if xp2 != xp1 or yp2 != yp1 or xleng2 != xleng1 or yleng2 != yleng1 or xoff2 != xoff1 or yoff2 != yoff1:
    sys.stderr.write("plgpage test failed\n")
    status = 1
# Restore values set initially by w.plparseopts.
w.plspage(xp0, yp0, xleng0, yleng0, xoff0, yoff0)

# Test setting / getting compression parameter across w.plinit.
compression1 = 95
w.plscompression(compression1)

# Initialize plplot
w.plinit()

def main(w):

    status = 0

    # Test if device initialization screwed around with the preset
    # compression parameter.
    compression2 = w.plgcompression()
    sys.stdout.write("Output various PLplot parameters\n")
    sys.stdout.write("compression parameter = %d\n" % compression2)
    if compression2 != compression1:
        sys.stderr.write("plgcompression test failed\n")
        status = 1

    # Exercise w.plscolor, w.plscol0, w.plscmap1, and w.plscmap1a to make sure
    # they work without any obvious error messages.
    w.plscolor(1)
    w.plscol0(1, 255, 0, 0)
    r1 = [0, 255]
    g1 = [255, 0]
    b1 = [0, 0]
    a1 = [1.0, 1.0]
    w.plscmap1(r1,g1,b1)
    w.plscmap1a(r1,g1,b1,a1)

    level2 = w.plglevel()
    sys.stdout.write("level parameter = %d\n" % level2)
    if level2 != 1:
        sys.stderr.write("plglevel test failed.\n")
        status = 1

    w.pladv(0)
    xmin0, xmax0, ymin0, ymax0 = (0.01, 0.99, 0.02, 0.49)
    w.plvpor(xmin0, xmax0, ymin0, ymax0)
    (xmin, xmax, ymin, ymax) = w.plgvpd()
    sys.stdout.write("plvpor: xmin, xmax, ymin, ymax = %f %f %f %f\n" % (xmin, xmax, ymin, ymax))
    if xmin != xmin0 or xmax != xmax0 or ymin != ymin0 or ymax != ymax0:
        sys.stderr.write("plgvpd test failed\n")
        status = 1
    xmid = 0.5*(xmin+xmax)
    ymid = 0.5*(ymin+ymax)

    xmin0, xmax0, ymin0, ymax0 = (0.2, 0.3, 0.4, 0.5)
    w.plwind(xmin0, xmax0, ymin0, ymax0)
    (xmin, xmax, ymin, ymax) = w.plgvpw()
    sys.stdout.write("plwind: xmin, xmax, ymin, ymax = %f %f %f %f\n" % (xmin, xmax, ymin, ymax))
    if xmin != xmin0 or xmax != xmax0 or ymin != ymin0 or ymax != ymax0:
        sys.stderr.write("plgvpw test failed\n")
        status = 1

    # Get world coordinates for middle of viewport
    (wx, wy, win) = w.plcalc_world(xmid,ymid)
    sys.stdout.write("world parameters: wx, wy, win = %f %f %d\n" % (wx, wy, win))
    if abs(wx-0.5*(xmin+xmax)) > 1.0E-5 or abs(wy-0.5*(ymin+ymax)) > 1.0E-5:
        sys.stderr.write("plcalc_world test failed\n")
        status = 1

    # Retrieve and print the name of the output file (if any)
    fnam = w.plgfnam()
    if fnam == '' :
        sys.stdout.write("No output file name is set\n")
    else :
        sys.stdout.write("Output file name read\n")
    sys.stderr.write("Output file name is %s\n" % fnam)

    # Set and get the number of digits used to display axis labels
    # Note digits is currently ignored in w.pls[xyz]ax and
    # therefore it does not make sense to test the returned value.
    w.plsxax(3,0)
    (digmax, digits) = w.plgxax()
    sys.stdout.write("x axis parameters: digmax, digits = %d %d\n" % (digmax, digits))
    if digmax != 3:
        sys.stderr.write("plgxax test failed\n")
        status = 1

    w.plsyax(4,0)
    (digmax, digits) = w.plgyax()
    sys.stdout.write("y axis parameters: digmax, digits = %d %d\n" % (digmax, digits))
    if digmax != 4:
        sys.stderr.write("plgyax test failed\n")
        status = 1

    w.plszax(5,0)
    (digmax, digits) = w.plgzax()
    sys.stdout.write("z axis parameters: digmax, digits = %d %d\n" % (digmax, digits))
    if digmax != 5:
        sys.stderr.write("plgzax test failed\n")
        status = 1

    (mar0, aspect0, jx0, jy0) = (0.05, w.PL_NOTSET, 0.1, 0.2)
    w.plsdidev(mar0, aspect0, jx0, jy0)
    (mar, aspect, jx, jy) = w.plgdidev()
    sys.stdout.write("device-space window parameters: mar, aspect, jx, jy = %f %f %f %f\n" % (mar, aspect, jx, jy))
    if mar != mar0 or jx != jx0 or jy != jy0:
        sys.stderr.write("plgdidev test failed\n")
        status = 1

    ori0 = 1.0
    w.plsdiori(ori0)
    ori = w.plgdiori()
    sys.stdout.write("ori parameter = %f\n" % ori)
    if ori != ori0:
        sys.stderr.write("plgdiori test failed\n")
        status = 1

    (xmin0, ymin0, xmax0, ymax0) = (0.1, 0.2, 0.9, 0.8)
    w.plsdiplt(xmin0, ymin0, xmax0, ymax0)
    (xmin, ymin, xmax, ymax) = w.plgdiplt()
    sys.stdout.write("plot-space window parameters: xmin, ymin, xmax, ymax = %f %f %f %f\n" % (xmin, ymin, xmax, ymax))
    if xmin != xmin0 or ymin != ymin0 or xmax != xmax0 or ymax != ymax0:
        sys.stderr.write("plgdiplt test failed\n")
        status = 1

    (zxmin0, zymin0, zxmax0, zymax0) = (0.1, 0.1, 0.9, 0.9)
    w.plsdiplz(zxmin0, zymin0, zxmax0, zymax0)
    (zxmin, zymin, zxmax, zymax) = w.plgdiplt()
    sys.stdout.write("zoomed plot-space window parameters: xmin, ymin, xmax, ymax = %f %f %f %f\n" % (zxmin, zymin, zxmax, zymax))
    if abs(zxmin -(xmin + (xmax-xmin)*zxmin0)) > 1.0E-5 or abs(zymin -(ymin+(ymax-ymin)*zymin0)) > 1.0E-5 or abs(zxmax -(xmin+(xmax-xmin)*zxmax0)) > 1.0E-5 or abs(zymax -(ymin+(ymax-ymin)*zymax0)) > 1.0E-5 :
        sys.stderr.write("plsdiplz test failed\n")
        status = 1

    (r0, g0, b0) = (10, 20, 30)
    w.plscolbg(r0, g0, b0)
    (r, g, b) = w.plgcolbg()
    sys.stdout.write("background colour parameters: r, g, b = %d %d %d\n" % (r, g, b))
    if r != r0 or g != g0 or b != b0:
        sys.stderr.write("plgcolbg test failed\n")
        status = 1

    (r0, g0, b0, a0) = (20, 30, 40, 0.5)
    w.plscolbga(r0, g0, b0, a0)
    (r, g, b, a) = w.plgcolbga()
    sys.stdout.write("background/transparency colour parameters: r, g, b, a = %d %d %d %f\n" % (r, g, b, a))
    if r != r0 or g != g0 or b != b0 or a != a0:
        sys.stderr.write("plgcolbga test failed\n")
        status = 1


    # Restore defaults
    # cmap0 default color palette.
    w.plspal0("cmap0_default.pal")
    # cmap1 default color palette.
    w.plspal1("cmap1_default.pal", 1)

    # Must be done independently because otherwise this changes output files
    # and destroys agreement with C examples.
    #w.plcol0(1)

    # Do not exit with status code on one of these failures
    #if status != 0 :
    #    sys.exit(status)

main(w)
# Terminate plplot
w.plend()
