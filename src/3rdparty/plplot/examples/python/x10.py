#  Copyright (C) 2001-2016 Alan W. Irwin

#  Window positioning demo.
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

# main
#
# Demonstrates absolute positioning of graphs on a page.

def main(w):

    w.pladv(0)
    w.plvpor(0.0, 1.0, 0.0, 1.0)
    w.plwind(0.0, 1.0, 0.0, 1.0)
    w.plbox("bc", 0.0, 0, "bc", 0.0, 0)

    w.plsvpa(50.0, 150.0, 50.0, 100.0)
    w.plwind(0.0, 1.0, 0.0, 1.0)
    w.plbox("bc", 0.0, 0, "bc", 0.0, 0)
    w.plptex(0.5, 0.5, 1.0, 0.0, 0.5, "BOX at (50,150,50,100)")

    # Restore defaults
    # Everything was default so nothing to restore.
