proc x04 {{w loopback}} {

    $w cmd plfont 2
    plot41 $w 0
    plot41 $w 1

    # Restore defaults
    $w cmd plfont 1
    # $w cmd plcol0 1

}

proc plot41 {w type} {


    matrix freql f 101
    matrix ampl f 101
    matrix phase f 101

    $w cmd pladv 0

    set f0 1.0
    for {set i 0} {$i <= 100} {incr i} {
	freql $i = [expr {-2.0 + $i / 20.0}]
	set freq [expr {pow(10.0, [freql $i])}]
	ampl $i = [expr {20.0 * log10(1.0 / sqrt(1.0 + pow(($freq/$f0), 2)))}]
	phase $i = [expr {-(180.0 / $::PLPLOT::PL_PI) * atan($freq/$f0)}]
    }

    $w cmd plvpor 0.15 0.85 0.1 0.9
    $w cmd plwind -2.0 3.0 -80.0 0.0

    $w cmd plcol0 1
    if {$type == 0} {
       $w cmd plbox "bclnst" 0.0 0 "bnstv" 0.0 0
    } elseif {$type ==1} {
       $w cmd plbox "bcfghlnst" 0.0 0 "bcghnstv" 0.0 0
    } else {
       puts "error: type must be either 0 or 1"
    }

    $w cmd plcol0 2
    $w cmd plline freql ampl

    $w cmd plcol0 2
    $w cmd plptex 1.6 -30.0 1.0 -20.0 0.5 "-20 dB/decade"

    # Put labels on
    $w cmd plcol0 1
    $w cmd plmtex "b" 3.2 0.5 0.5 "Frequency"
    $w cmd plmtex "t" 2.0 0.5 0.5 "Single Pole Low-Pass Filter"
    $w cmd plcol0 2
    $w cmd plmtex "l" 5.0 0.5 0.5 "Amplitude (dB)"

    set nlegend 1

    # For the gridless case, put phase vs freq on same plot
    if {$type == 0} {
       $w cmd plcol0 1
       $w cmd plwind -2.0 3.0 -100.0 0.0
       $w cmd plbox "" 0.0 0 "cmstv" 30.0 3
       $w cmd plcol0 3
       $w cmd plline freql phase
       $w cmd plstring freql phase "#(728)"
       $w cmd plcol0 3
       $w cmd plmtex "r" 5.0 0.5 0.5 "Phase shift (degrees)"

       set nlegend 2
    }

    # Draw a legend
    # First legend entry.
    set opt_array   [list $::PLPLOT::PL_LEGEND_LINE]
    set text_colors [list 2]
    set text        [list Amplitude]
    set line_colors [list 2]
    set line_styles [list 1]
    set line_widths [list 1.0]

    # note from the above opt_array the first symbol (and box) indices
    # will not be used, but they have to be specified anyway!
    # (make sure the values are reasonable)

    # Second legend entry.
    lappend opt_array      [expr {$::PLPLOT::PL_LEGEND_LINE | $::PLPLOT::PL_LEGEND_SYMBOL}]
    lappend text_colors    3
    lappend text           "Phase shift"
    lappend line_colors    3
    lappend line_styles    1
    lappend line_widths    1
    set     symbol_colors  [list 0 3]
    set     symbol_scales  [list 0.0 1.0]
    set     symbol_numbers [list 0 4]
    set     symbols        [list "" "#(728)"]

    # from the above opt_arrays we can completely ignore everything
    # to do with boxes.

    $w cmd plscol0a 15 32 32 32 0.70
    foreach { legend_width legend_height } \
        [$w cmd pllegend [expr {$::PLPLOT::PL_LEGEND_BACKGROUND | $::PLPLOT::PL_LEGEND_BOUNDING_BOX}] 0 \
            0.0 0.0 0.1 15 \
            1 1 0 0 \
            [lrange $opt_array 0 [expr {$nlegend-1}]] \
            1.0 1.0 2.0 \
            1. $text_colors $text \
            {} {} {} {} \
            $line_colors $line_styles $line_widths \
            $symbol_colors $symbol_scales $symbol_numbers $symbols] {
        break
    }
}
