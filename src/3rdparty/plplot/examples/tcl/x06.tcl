proc x06 {{w loopback}} {

    matrix x f 1
    matrix y f 1

    for {set kind_font 0} {$kind_font < 2} {incr kind_font} {

       $w cmd plfontld $kind_font
       if {$kind_font == 0} {
           set maxfont 1
       } else {
           set maxfont 4
       }

       for {set font 0} {$font < $maxfont} {incr font} {

           $w cmd plfont [expr {$font+1}]

           $w cmd pladv 0

# Set up viewport and window

           $w cmd plcol0 2
           $w cmd plvpor 0.1 1.0 0.1 0.9
           $w cmd plwind 0.0 1.0 0.0 1.3

# Draw the grid using plbox

           $w cmd plbox "bcg" 0.1 0 "bcg" 0.1 0

# Write the digits below the frame

           $w cmd plcol0 15
           for {set i 0} {$i <= 9} {incr i} {
	       $w cmd plmtex "b" 1.5 [expr {0.1 * $i + 0.05}] 0.5 $i
           }

           set k 0
           for {set i 0} {$i <= 12} {incr i} {

    # Write the digits to the left of the frame

	       $w cmd plmtex "lv" 1.0 [expr {1.0 - (2 * $i + 1)/26.0}] 1.0 [expr {10*$i}]
	       for {set j 0} {$j <= 9} {incr j} {
	           x 0 = [expr {0.1 * $j + 0.05}]
	           y 0 = [expr {1.25 - 0.1 * $i}]

	# Display the symbols

	           if {$k < 128} {
		       $w cmd plpoin x y $k
	           }
	           incr k
	       }
            }

            if {$kind_font==0} {
                $w cmd plmtex "t" 1.5 0.5 0.5 "PLplot Example 6 - plpoin symbols (compact)"
            } else {
                $w cmd plmtex "t" 1.5 0.5 0.5 "PLplot Example 6 - plpoin symbols (extended)"
            }
        }
    }
    # Restore defaults
    # $w cmd plcol0 1
}
