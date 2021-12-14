#!/usr/bin/env python
# Simple test of plgradient for a complicated polygon.

# Append to effective python path so that can find plplot modules.
from plplot_python_start import *

import sys
import plplot as w
from numpy import *
from math import *

# Parse and process command line arguments
w.plparseopts(sys.argv, w.PL_PARSE_FULL)

xmin,xmax,ymin,ymax = (0., 1., 0., 1.)
x = (0., 0.2, 1., 0.3)
y = (0., 1., 0.2, 0.6)
#x = (0., 0., 1., 1.)
#y = (0., 1., 1., 0.)
# Initialize plplot
w.plinit()
for i in range(1):
    w.plcol0(2)
    w.pladv(0)
    w.plvpor(0.1, 0.9, 0.1, 0.9)
    w.plwind(xmin, xmax, ymin, ymax)
# Just to show edges of viewport
    w.plbox("bc", 0., 0, "bc", 0., 0)
    w.plcol0(1)
    w.plgradient(x,y, 45. + i)
w.plend()
