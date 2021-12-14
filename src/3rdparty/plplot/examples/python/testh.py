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
# Like yellow lines better.
w.plcol0(2)
w.pladv(0)
w.plvpor(0.1, 0.9, 0.1, 0.9)
w.plwind(0., 1., 0., 1.)
x=0.*arange(2)
y=1.*arange(2)
w.plline(x,y)
x=0.1*arange(2)
y=0.5+0.*arange(2)
w.plline(x,y)
w.plschr(0., 4.)
#w.plmtex("l", 0., 0.5, 0.5, "#[0x00d7]#[0x00d7]#[0x00d7]#[0x00d7]#[0x00d7]#[0x00d7]#[0x00d7]")
w.plmtex("l", 0., 0.5, 0.5, "HHHHHHH")

# Terminate plplot
w.plend()

