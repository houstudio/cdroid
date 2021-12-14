proc x03 {{w loopback}} {
    set twopi  [expr {2. * $::PLPLOT::PL_PI}]
# Set up viewport and window, but do not draw box

    $w cmd plenv -1.3 1.3 -1.3 1.3 1 -2

# Draw circles for polar grid

    set ni 10
    set nj 360
    set nj1 [expr {$nj + 1}]

    set dr     [expr {1. / $ni}]
    set dtheta [expr {$twopi / $nj}]

    matrix xj f $nj1
    matrix yj f $nj1

    for {set i 1} {$i <= $ni} {incr i} {
        $w cmd plarc 0.0 0.0 [expr {0.1 * $i}]] [expr {0.1 * $i}] 0.0 360.0 0.0 0
    }

# Draw radial spokes for polar grid and write labels for angle

    $w cmd plcol0 2
    for {set j 0} {$j <= 11} {incr j} {
	set theta [expr {$j * $twopi / 12.}]
	set xg [expr {cos($theta)}]
	set yg [expr {sin($theta)}]
	$w cmd pljoin 0.0 0.0 $xg $yg

	set theta_deg [expr {$theta*360./$twopi}]
	if {$theta_deg < 9.99} {
	    set offset 0.45
	} elseif {$theta_deg < 99.9} {
	    set offset 0.30
	} else {
	    set offset 0.15
	}

# Slightly off zero to avoid floating point logic flips at 90 and 270 deg.
	if {$xg >= -0.00001} {
	    set dx $xg
	    set dy $yg
	    set just [expr {-$offset}]
	} else {
	    set dx [expr {-$xg}]
	    set dy [expr {-$yg}]
	    set just [expr {1. + $offset}]
	}
	set label [expr {round($theta*360./$twopi)}]

# N.B. cannot get this command to give same postscript output.  Also visual
# inspection shows 90 deg label jumping around slightly compared to python
# and C front ends.  No idea why (AWI comment).
	$w cmd plptex $xg $yg $dx $dy $just $label
    }

# Draw the graph

    set npts 360
    set npts1 [expr {$npts+1}]

    set dtheta [expr {$twopi / $npts}]

    matrix x f $npts1
    matrix y f $npts1

    for {set j 0} {$j <= $npts} {incr j} {
	set theta [expr {$j * $dtheta}]
	set r     [expr {sin(5 * $theta)}]
	x $j = [expr {$r * cos($theta)}]
	y $j = [expr {$r * sin($theta)}]
    }
    $w cmd plcol0 3
    $w cmd plline x y

    $w cmd plcol0 4
    $w cmd plmtex "t" 2.0 0.5 0.5 "#frPLplot Example 3 - r(#gh)=sin 5#gh"

# Restore defaults
#    $w cmd plcol0 1
}
