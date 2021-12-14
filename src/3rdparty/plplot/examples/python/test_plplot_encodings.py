#!/usr/bin/env python
# -*- coding: utf-8; -*-
# Test whether utf8 and PLplot #[0x...] unicode encoding give same result.
# Also test whether #<FCI-COMMAND-STRING/> form works to change font
# characteristics on the fly.
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
w.plwind(0., 1., 0., 1.)
# Just to show edges of viewport
w.plbox("bc", 0., 0, "bc", 0., 0)
w.plcol0(1)
w.plschr(0., 2.)
w.plptex(0.5, 0.5, 1., 0., 0.5, "#<sans-serif/>#[0x222e] is a path integral sign.")
# semitransparent green.
w.plscol0a(2, 0, 255, 0, 0.5)
w.plcol0(2)
w.plptex(0.5, 0.5, 1., 0., 0.5, "#<sans-serif/>∮ is a path integral sign.")
w.plend()
