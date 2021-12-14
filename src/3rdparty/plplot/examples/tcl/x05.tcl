proc x05 {{w loopback}} {
    set npts 2047
    matrix data f $npts

# Fill up data points

    set delta [expr 2.0 * $::PLPLOT::PL_PI / $npts]
    for {set i 0} {$i < $npts} {incr i} {
	data $i = [expr sin($i * $delta)]
    }

    $w cmd plcol0 1
    $w cmd plhist data -1.1 1.1 44 0

    $w cmd plcol0 2
    $w cmd pllab "#frValue" "#frFrequency" \
	"#frPLplot Example 5 - Probability function of Oscillator"
    # Restore defaults
    # $w cmd plcol0 1
}
