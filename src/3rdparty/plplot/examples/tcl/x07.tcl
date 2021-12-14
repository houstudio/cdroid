proc x07 {{w loopback}} {

    matrix base i 20 = \
	{0, 100, 0, 100, 200, 500, 600, 700, 800, 900, \
	     2000, 2100, 2200, 2300, 2400, 2500, 2600, 2700, 2800, 2900}

    matrix x f 1
    matrix y f 1

    $w cmd plfontld 0
    for {set l 0} {$l < 20} {incr l} {
        if {$l == 2} {
           $w cmd plfontld 1
        }
	$w cmd pladv 0

    # Set up viewport and window

	$w cmd plcol0 2
	$w cmd plvpor 0.15 0.95 0.1 0.9
	$w cmd plwind 0.0 1.0 0.0 1.0

    # Draw the grid using plbox

	$w cmd plbox "bcg" 0.1 0 "bcg" 0.1 0

    # Write the digits below the frame

	$w cmd plcol0 15
	for {set i 0} {$i <= 9} {incr i} {
	    $w cmd plmtex "b" 1.5 [expr {0.1 * $i + 0.05}] 0.5 $i
	}

	set k 0
	for {set i 0} {$i <= 9} {incr i} {

	# Write the digits to the left of the frame

	    $w cmd plmtex "lv" 1.0 [expr {0.95 - 0.1 * $i}] 1.0 \
		[expr {[base $l] + 10*$i}]

	    for {set j 0} {$j <= 9} {incr j} {
		x 0 = [expr {0.1 * $j + 0.05}]
		y 0 = [expr {0.95 - 0.1 * $i}]

	    # Display the symbols

		$w cmd plsym x y [expr {[base $l] + $k}]
		incr k
	    }
	}
        if {$l < 2} {
	    $w cmd plmtex "t" 1.5 0.5 0.5 "PLplot Example 7 - PLSYM symbols (compact)"
        } else {
	    $w cmd plmtex "t" 1.5 0.5 0.5 "PLplot Example 7 - PLSYM symbols (extended)"
        }
    }
# Restore defaults
    # $w cmd plcol0 1
    $w cmd plfontld 0
}
