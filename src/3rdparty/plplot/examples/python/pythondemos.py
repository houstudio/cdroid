#!/usr/bin/env python

# Copyright (C) 2004-2016 Alan W. Irwin
# Copyright (C) 2004 Andrew Ross
#
# This file is part of PLplot.
#
# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published
# by the Free Software Foundation; version 2 of the License.
#
# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with the file PLplot; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA


# Run all python plplot examples non-interactively.

# Append to effective python path so that can find plplot modules.
from plplot_python_start import *

import sys
import plplot as w
import restore

# Parse and process command line arguments
w.plparseopts(sys.argv, w.PL_PARSE_FULL)

# Initialize plplot
w.plinit()

failed = []
for i in range(0,34):
    # Exclude interactive demos 14, 17, 31 which call plinit in the x??.py
    # script.  Also, exclude 32 which is not implemented.
    if i == 14 or i == 17 or i == 31 or i == 32:
        continue
    script = 'x' + '%02d' % i
    module = __import__(script, globals(), locals(), [])
    try:
        module.main(w)
    except:
        failed.append(script)

    restore.main(w)

# Terminate plplot
w.plend()

if len(failed) != 0:
    import string
    print "Failed script(s): " + string.join(failed,", ")
    sys.exit(1)

