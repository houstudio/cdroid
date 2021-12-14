#
#  Alpha color values demonstration.
#
#  Copyright (C) 2008 Hazen Babcock
#  Copyright (C) 2008 Andrew Ross
#
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
#  This example will only really be interesting when used with devices that
#  support or alpha (or transparency) values, such as the cairo device family.
#

proc x30 {{w loopback}} {

    matrix red i 4   = {  0, 255,   0,   0}
    matrix green i 4 = {  0,   0, 255,   0}
    matrix blue i 4  = {  0,   0,   0, 255}
    matrix alpha f 4 = {1.0, 1.0, 1.0, 1.0}

    matrix px f 4 = {0.1, 0.5, 0.5, 0.1}
    matrix py f 4 = {0.1, 0.1, 0.5, 0.5}

    matrix pos f 2 = {0.0, 1.0}
    matrix rcoord f 2 = {1.0, 1.0}
    matrix gcoord f 2 = {0.0, 0.0}
    matrix bcoord f 2 = {0.0, 0.0}
    matrix acoord f 2 = {0.0, 1.0}
    matrix rev i 2 = {0, 0}

    $w cmd plscmap0n 4
    $w cmd plscmap0a red green blue alpha

    #
    # Page 1:
    #
    # This is a series of red, green and blue rectangles overlaid
    # on each other with gradually increasing transparency.
    #

    # Set up the window
    $w cmd pladv 0
    $w cmd plvpor 0.0 1.0 0.0 1.0
    $w cmd plwind 0.0 1.0 0.0 1.0
    $w cmd plcol0 0
    $w cmd plbox "" 1.0 0 "" 1.0 0

    #   Draw the boxes
    for {set i 0} {$i < 9} {incr i} {
	set icol [expr {$i%3 + 1}]

	# Get a color, change its transparency and
	# set it as the current color.
	$w cmd plgcol0a $icol r g b a
	$w cmd plscol0a $icol $r $g $b [expr {1.0 - double($i)/9.0}]
	$w cmd plcol0 $icol

	# Draw the rectangle
	$w cmd plfill px py

	# Shift the rectangles coordinates
	for {set j 0} {$j < 4} {incr j} {
	    px $j = [expr {[px $j] + 0.5/9.0}]
	    py $j = [expr {[py $j] + 0.5/9.0}]
	}
    }

    #
    # Page 2:
    #
    # This is a bunch of boxes colored red, green or blue with a single
    # large (red) box of linearly varying transparency overlaid. The
    # overlaid box is completely transparent at the bottom and completely
    # opaque at the top.
    #

    # Set up the window
    $w cmd pladv 0
    $w cmd plvpor 0.1 0.9 0.1 0.9
    $w cmd plwind 0.0 1.0 0.0 1.0

    # Draw the boxes. There are 25 of them drawn on a 5 x 5 grid.
    for {set i 0} {$i < 5} {incr i} {
	# Set box X position
	px 0 = [expr {0.05 + 0.2 * $i}]
	px 1 = [expr {[px 0] + 0.1}]
	px 2 = [px 1]
	px 3 = [px 0]

	# We don't want the boxes to be transparent, so since we changed
	# the colors transparencies in the first example we have to change
	# the transparencies back to completely opaque.
	set icol [expr {$i%3 + 1}]
	$w cmd plgcol0a $icol r g b a
	$w cmd plscol0a $icol $r $g $b 1.0
	$w cmd plcol0 $icol
	for {set j 0} {$j < 5} {incr j} {
	    # Set box y position and draw the box.
	    py 0 = [expr {0.05 + 0.2 * $j}]
	    py 1 = [py 0]
	    py 2 = [expr {[py 0] + 0.1}]
	    py 3 = [py 2]
	    $w cmd plfill px py
	}
    }

    # The overlaid box is drawn using plshades with a color map that is
    # the same color but has a linearly varying transparency.

    # Create the color map with 128 colors and use plscmap1la to initialize
    # the color values with a linear varying transparency (or alpha)
    $w cmd plscmap1n 128
    $w cmd plscmap1la 1 pos rcoord gcoord bcoord acoord rev

    # Use that cmap1 to create a transparent red gradient for the whole
    # window.
    px 0 = 0.
    px 1 = 1.
    px 2 = 1.
    px 3 = 0.

    py 0 = 0.
    py 1 = 0.
    py 2 = 1.
    py 3 = 1.

    $w cmd plgradient px py 90.

}

