#!/usr/bin/env python

# Append to effective python path so that can find plplot modules.
from plplot_python_start import *

import sys
import plplot as w
from numpy import *

# Parse and process command line arguments
w.plparseopts(sys.argv, w.PL_PARSE_FULL)

# Initialize plplot
w.plinit()
w.plssym(0., 10.)
w.plenv(0,1,0,1,0,0)
w.plpoin([1.0],[0.5],0)
w.plend()
