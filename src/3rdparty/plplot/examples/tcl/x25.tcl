#	Filling and clipping polygons.
#
#   Copyright (C) 2005 Arjen Markus
#   Copyright (C) 2008 Andrew Ross
#
#
#   This file is part of PLplot.
#
#   PLplot is free software; you can redistribute it and/or modify
#   it under the terms of the GNU Library General Public License as published
#   by the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   PLplot is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Library General Public License for more details.
#
#   You should have received a copy of the GNU Library General Public License
#   along with PLplot; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


#--------------------------------------------------------------------------
# main
#
# Test program for filling polygons and proper clipping
#--------------------------------------------------------------------------

proc x25 {{w loopback}} {
    set npts 0

    $w cmd pladv 0
    $w cmd plssub 3 3

    matrix xextreme f  10 2
    matrix yextreme f  10 2
    matrix x0 f 10
    matrix y0 f 10

    xextreme 0 0 = -120.0
    xextreme 0 1  = 120.0
    yextreme 0 0  = -120.0
    yextreme 0 1  =  120.0
    xextreme 1 0  = -120.0
    xextreme 1 1  =  120.0
    yextreme 1 0  =   20.0
    yextreme 1 1  =  120.0
    xextreme 2 0  = -120.0
    xextreme 2 1  =  120.0
    yextreme 2 0  =  -20.0
    yextreme 2 1  =  120.0
    xextreme 3 0  =  -80.0
    xextreme 3 1  =   80.0
    yextreme 3 0  =  -20.0
    yextreme 3 1  =  120.0
    xextreme 4 0  = -220.0
    xextreme 4 1  = -120.0
    yextreme 4 0  = -120.0
    yextreme 4 1  =  120.0
    xextreme 5 0  =  -20.0
    xextreme 5 1  =   20.0
    yextreme 5 0  = -120.0
    yextreme 5 1  =  120.0
    xextreme 6 0  =  -20.0
    xextreme 6 1  =   20.0
    yextreme 6 0  =  -20.0
    yextreme 6 1  =   20.0
    xextreme 7 0  =  -80.0
    xextreme 7 1  =   80.0
    yextreme 7 0  =  -80.0
    yextreme 7 1  =   80.0
    xextreme 8 0  =   20.0
    xextreme 8 1  =  120.0
    yextreme 8 0  = -120.0
    yextreme 8 1  =  120.0

    for {set k 0} {$k < 2} {incr k} {
	for {set j 0} {$j < 4} {incr j} {
	    if { $j == 0 } {
		# Polygon 1: a diamond
		x0 0 =    0
		y0 0 = -100
		x0 1 = -100
		y0 1 =    0
		x0 2 =    0
		y0 2 =  100
		x0 3 =  100
		y0 3 =    0
		set npts 4
	    }
	    if { $j == 1 } {
		# Polygon 1: a diamond - reverse direction
		x0 3 =    0
		y0 3 = -100
		x0 2 = -100
		y0 2 =    0
		x0 1 =    0
		y0 1 =  100
		x0 0 =  100
		y0 0 =    0
		set npts 4
	    }
	    if { $j == 2 } {
		# Polygon 2: a square with punctures
		x0 0 = -100
		y0 0 = -100
		x0 1 = -100
		y0 1 =  -80
		x0 2 =   80
		y0 2 =    0
		x0 3 = -100
		y0 3 =   80
		x0 4 = -100
		y0 4 =  100
		x0 5 =  -80
		y0 5 =  100
		x0 6 =    0
		y0 6 =   80
		x0 7 =   80
		y0 7 =  100
		x0 8 =  100
		y0 8 =  100
		x0 9 =  100
		y0 9 = -100
		set npts 10
	    }
	    if { $j == 3 } {
		# Polygon 2: a square with punctures - reversed direction
		x0 9 = -100
		y0 9 = -100
		x0 8 = -100
		y0 8 =  -80
		x0 7 =   80
		y0 7 =    0
		x0 6 = -100
		y0 6 =   80
		x0 5 = -100
		y0 5 =  100
		x0 4 =  -80
		y0 4 =  100
		x0 3 =    0
		y0 3 =   80
		x0 2 =   80
		y0 2 =  100
		x0 1 =  100
		y0 1 =  100
		x0 0 =  100
		y0 0 = -100
		set npts 10
	    }

	    for {set i 0} {$i < 9} {incr i} {
		$w cmd pladv 0
		$w cmd plvsta
		$w cmd plwind [xextreme $i 0] [xextreme $i 1] [yextreme $i 0] [yextreme $i 1]

		$w cmd plcol0 2
		$w cmd plbox "bc" 1.0 0 "bcnv" 10.0 0
		$w cmd plcol0 1
		$w cmd plpsty 0
		matrix xplot f $npts = [x0 :]
		matrix yplot f $npts = [y0 :]
		if { $k == 0 } {
		    $w cmd plfill xplot yplot
		} else {
		    $w cmd plgradient xplot yplot 45.0
		}
		$w cmd plcol0 2
		$w cmd pllsty 1
		$w cmd plline xplot yplot
		xplot delete
		yplot delete
	    }
	}
    }

    # Restore defaults
    $w cmd plssub 1 1
    $w cmd pleop

}
