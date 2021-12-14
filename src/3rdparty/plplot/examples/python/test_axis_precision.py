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
# Choose 5 here so there is room for non-exponential notation for an axis
# label of +0.08.
w.plsyax(5, 0)
w.pladv(0)
w.plvpor(0.1, 0.9, 0.1, 0.9)
w.plwind(0.00, 1.00, 0.00, 0.08)
w.plbox("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
w.plend()
