#!/usr/bin/env python
# -*- coding: utf-8; -*-
# Test whether line breaks work for utf8 "Peace" strings.
# Append to effective python path so that can find plplot modules.
from plplot_python_start import *

import sys
import plplot as w
from numpy import *

# Taken from http://www.columbia.edu/~fdc/pace/

peace = [
    "和平 (Mandarin)",
    "शांति (Hindi)",
    "Peace (English)",
    "שלום (Hebrew)",
    "Мир (Russian)",
    "Friede (German)",
    "평화 (Korean)",
    "Paix (French)",
    "Paz (Spanish)",
    "ﺳﻼم (Arabic)",
    "Barış (Turkish)",
    "Hasîtî (Kurdish)",
    ]

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
w.plschr(0., 1.4)
for i in arange(6):
    w.plptex(0.01, 0.90 - 0.16*i, 1., 0., 0., peace[2*i] + "\n" + peace[2*i+1])
w.plend()
