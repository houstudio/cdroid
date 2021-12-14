#!/usr/bin/env python
# Simple test of plfill for the case where the polygon is just outside the
# window.

# Append to effective python path so that can find plplot modules.
from plplot_python_start import *

import sys
import plplot as w
from numpy import *

# Parse and process command line arguments
w.plparseopts(sys.argv, w.PL_PARSE_FULL)

# Initialize plplot
w.plinit()
w.plcol0(2)
w.pladv(0)
w.plvpor(0.1, 0.9, 0.1, 0.9)
xmin,xmax,ymin,ymax = (0., 1., 0., 1.)
w.plwind(xmin, xmax, ymin, ymax)
# Just to show edges of viewport
w.plbox("bc", 0., 0, "bc", 0., 0)
w.plcol0(1)
eps = 0.001
xmin,xmax,ymin,ymax = (xmin - eps, xmax + eps, ymin - eps, ymax + eps)
x = (xmin, xmax, xmax, xmin)
y = (ymin, ymin, ymax, ymax)
w.plfill(x,y)
w.plend()
