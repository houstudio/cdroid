# Simple demo of 2D line plot.

proc x00 {{w loopback}} {

    set xmin 0.0
    set xmax 1.0
    set ymin 0.0
    set ymax 100.0

    set nsize 101
    matrix x f $nsize
    matrix y f $nsize

    for {set i 0} {$i < $nsize} {incr i} {
	set xi [expr {$i / double($nsize - 1)}]
	x $i = $xi
	y $i = [expr {$ymax * $xi * $xi}]
    }

    # Create a labelled box to hold the plot
    $w cmd plenv $xmin $xmax $ymin $ymax 0 0
    $w cmd pllab "x" "y=100 x#u2#d" "Simple PLplot demo of a 2D line plot"

    # Plot the data that was prepared above
    $w cmd plline x y
}
