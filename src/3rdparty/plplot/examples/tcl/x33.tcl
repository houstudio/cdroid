# Demonstrate most pllegend capability including unicode symbols.
#
# Copyright (C) 2010-2014 Alan Irwin
#
# This file is part of PLplot.
#
# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published
# by the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with PLplot; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
#
# This example designed just for devices (e.g., the cairo-related and
# qt-related devices) where the best choice of glyph is automatically
# selected by the related libraries (pango/cairo or Qt4) for each
# unicode character depending on what system fonts are installed.  Of
# course, you must have the appropriate TrueType fonts installed to
# have access to all the required glyphs.

# TODO
#set options [list {
#    {
#        "colorbar",              // Turns on pages showing colorbars
#        NULL,
#        NULL,
#        &colorbar,
#        PL_OPT_BOOL,
#        "-colorbar",
#        "Plot the \"color bar\" pages."
#    },
#    {
#        NULL,                   // option
#        NULL,                   // handler
#        NULL,                   // client data
#        NULL,                   // address of variable to set
#        0,                      // mode flag
#        NULL,                   // short syntax
#        NULL
#    }                           // long syntax
#};

# --------------------------------------------------------------------------
# main
#
# Demonstrate most pllegend capability including unicode symbols.
# --------------------------------------------------------------------------

set MAX_NLEGEND    7

proc x33 {{w loopback}} {

    set position_options [list \
        [expr {$::PLPLOT::PL_POSITION_LEFT | $::PLPLOT::PL_POSITION_TOP | $::PLPLOT::PL_POSITION_OUTSIDE}]     \
        [expr {$::PLPLOT::PL_POSITION_TOP | $::PLPLOT::PL_POSITION_OUTSIDE}]                                   \
        [expr {$::PLPLOT::PL_POSITION_RIGHT | $::PLPLOT::PL_POSITION_TOP | $::PLPLOT::PL_POSITION_OUTSIDE}]    \
        [expr {$::PLPLOT::PL_POSITION_RIGHT | $::PLPLOT::PL_POSITION_OUTSIDE}]                                 \
        [expr {$::PLPLOT::PL_POSITION_RIGHT | $::PLPLOT::PL_POSITION_BOTTOM | $::PLPLOT::PL_POSITION_OUTSIDE}] \
        [expr {$::PLPLOT::PL_POSITION_BOTTOM | $::PLPLOT::PL_POSITION_OUTSIDE}]                                \
        [expr {$::PLPLOT::PL_POSITION_LEFT | $::PLPLOT::PL_POSITION_BOTTOM | $::PLPLOT::PL_POSITION_OUTSIDE}]  \
        [expr {$::PLPLOT::PL_POSITION_LEFT | $::PLPLOT::PL_POSITION_OUTSIDE}]                                  \
        [expr {$::PLPLOT::PL_POSITION_LEFT | $::PLPLOT::PL_POSITION_TOP | $::PLPLOT::PL_POSITION_INSIDE}]      \
        [expr {$::PLPLOT::PL_POSITION_TOP | $::PLPLOT::PL_POSITION_INSIDE}]                                    \
        [expr {$::PLPLOT::PL_POSITION_RIGHT | $::PLPLOT::PL_POSITION_TOP | $::PLPLOT::PL_POSITION_INSIDE}]     \
        [expr {$::PLPLOT::PL_POSITION_RIGHT | $::PLPLOT::PL_POSITION_INSIDE}]                                  \
        [expr {$::PLPLOT::PL_POSITION_RIGHT | $::PLPLOT::PL_POSITION_BOTTOM | $::PLPLOT::PL_POSITION_INSIDE}]  \
        [expr {$::PLPLOT::PL_POSITION_BOTTOM | $::PLPLOT::PL_POSITION_INSIDE}]                                 \
        [expr {$::PLPLOT::PL_POSITION_LEFT | $::PLPLOT::PL_POSITION_BOTTOM | $::PLPLOT::PL_POSITION_INSIDE}]   \
        [expr {$::PLPLOT::PL_POSITION_LEFT | $::PLPLOT::PL_POSITION_INSIDE}]                                   ]

    # Pick 5 arbitrary UTF-8 symbols useful for plotting points (✠✚✱✪✽✺✰✴✦).
    set special_symbols [list \
        "✰" \
        "✴" \
        "✱" \
        "✽" \
        "✦" ]

    # plcolorbar options

    # Colorbar type options

    set ::colorbar_option_kinds [list \
        $::PLPLOT::PL_COLORBAR_SHADE \
        [expr {$::PLPLOT::PL_COLORBAR_SHADE | $::PLPLOT::PL_COLORBAR_SHADE_LABEL}] \
        $::PLPLOT::PL_COLORBAR_IMAGE \
        $::PLPLOT::PL_COLORBAR_GRADIENT]

    set ::colorbar_option_kind_labels [list \
    "Shade colorbars" \
    "Shade colorbars with custom labels" \
    "Image colorbars" \
    "Gradient colorbars"]

    # Which side of the page are we positioned relative to?
    set ::colorbar_position_options [list \
    $::PLPLOT::PL_POSITION_LEFT \
    $::PLPLOT::PL_POSITION_RIGHT \
    $::PLPLOT::PL_POSITION_TOP \
    $::PLPLOT::PL_POSITION_BOTTOM]
    set ::colorbar_position_option_labels [list \
    "Left" \
    "Right" \
    "Top" \
    "Bottom"]

    # Colorbar label positioning options
    set ::colorbar_label_options [list \
    $::PLPLOT::PL_COLORBAR_LABEL_LEFT \
    $::PLPLOT::PL_COLORBAR_LABEL_RIGHT \
    $::PLPLOT::PL_COLORBAR_LABEL_TOP \
    $::PLPLOT::PL_COLORBAR_LABEL_BOTTOM]
    set ::colorbar_label_option_labels [list \
    "Label left" \
    "Label right" \
    "Label top" \
    "Label bottom"]

    # Colorbar cap options
    set ::colorbar_cap_options [list \
    $::PLPLOT::PL_COLORBAR_CAP_NONE \
    $::PLPLOT::PL_COLORBAR_CAP_LOW \
    $::PLPLOT::PL_COLORBAR_CAP_HIGH \
    [expr {$::PLPLOT::PL_COLORBAR_CAP_LOW | $::PLPLOT::PL_COLORBAR_CAP_HIGH}]]
    set ::colorbar_cap_option_labels [list \
    "No caps" \
    "Low cap" \
    "High cap" \
    "Low and high caps"]

    set colorbar 1 ;# By default do not plot plcolorbar pages
                    # for now while we are working out the API.

    set notes { "Make sure you get it right!" NULL }

    # Parse and process command line arguments
    # TODO
    # plMergeOpts options "x33c options" notes
    # void plparseopts &argc argv PL_PARSE_FULL

    # First page illustrating the 16 standard positions.
    $w cmd pladv 0
    $w cmd plvpor 0.25 0.75 0.25 0.75
    $w cmd plwind 0.0 1.0 0.0 1.0
    $w cmd plbox "bc" 0.0 0 "bc" 0.0 0
    $w cmd plsfont [expr {$::PLPLOT::PL_FCI_SANS}] -1 -1
    $w cmd plmtex "t" 8.0 0.5 0.5 "The 16 standard legend positions with"
    $w cmd plmtex "t" 6.0 0.5 0.5 "the same (0.05) offset in x and y"

    set nlegend 1
    # Only specify legend data that are required according to the
    # value of opt_array for that entry.
    set opt_base       [expr {$::PLPLOT::PL_LEGEND_BACKGROUND | $::PLPLOT::PL_LEGEND_BOUNDING_BOX}]
    set opt_array      [list [expr {$::PLPLOT::PL_LEGEND_LINE | $::PLPLOT::PL_LEGEND_SYMBOL}]]
    set line_styles    [list 1]
    set line_widths    [list 1.]
    set symbol_scales  [list 1.]
    set symbol_numbers [list 4]
    set symbols        [list "#(728)"]

    # Use monotype fonts so that all legends are the same size.
    $w cmd plsfont [expr {$::PLPLOT::PL_FCI_MONO}] -1 -1
    $w cmd plscol0a 15 32 32 32 0.70

    for {set k 0} {$k < 16} {incr k} {
        set position      [lindex $position_options $k]
        set opt           $opt_base
        set text          [list [format "%2.2d" $k]]
        set text_colors   [list [expr {1 +  $k % 8}]]
        set line_colors   [list [expr {1 +  $k % 8}]]
        set symbol_colors [list [expr {1 +  $k % 8}]]

        $w cmd pllegend $opt $position 0.05 0.05 \
            0.1 15 1 1 0 0 \
            $opt_array 1.0 1.0 2.0 \
            1. $text_colors $text \
            {} {} {} {} \
            $line_colors $line_styles $line_widths \
            $symbol_colors $symbol_scales $symbol_numbers $symbols
    }

    # Second page illustrating effect of nrow, ncolumn for the same legend
    # data.
    $w cmd pladv 0
    $w cmd plvpor 0.25 0.75 0.25 0.75
    $w cmd plwind 0.0 1.0 0.0 1.0
    $w cmd plbox "bc" 0.0 0 "bc" 0.0 0
    $w cmd plsfont [expr {$::PLPLOT::PL_FCI_SANS}] -1 -1
    $w cmd plmtex "t" 8.0 0.5 0.5 "The effect of nrow, ncolumn, PL_LEGEND_ROW_MAJOR,"
    $w cmd plmtex "t" 6.0 0.5 0.5 "and position for the same legend data"

    set nlegend 7

    # Only specify legend data that are required according to the
    # value of opt_array for that entry.
    set opt_base       [expr {$::PLPLOT::PL_LEGEND_BACKGROUND | $::PLPLOT::PL_LEGEND_BOUNDING_BOX}]
    set opt_array      [list]
    set line_styles    [list]
    set line_widths    [list]
    set symbol_scales  [list]
    set symbol_numbers [list]
    set symbols        [list]
    set text           [list]
    set text_colors    [list]
    set line_colors    [list]
    set symbol_colors  [list]
    for {set k 0} {$k < $nlegend} {incr k} {
        lappend opt_array      [expr {$::PLPLOT::PL_LEGEND_LINE | $::PLPLOT::PL_LEGEND_SYMBOL}]
        lappend line_styles    1
        lappend line_widths    1.
        lappend symbol_scales  1.
        lappend symbol_numbers 2
        lappend symbols        "#(728)"
        lappend text           [format "%2.2d" $k]
        lappend text_colors    [expr {1 +  $k % 8}]
        lappend line_colors    [expr {1 +  $k % 8}]
        lappend symbol_colors  [expr {1 +  $k % 8}]
    }

    # Use monotype fonts so that all legends are the same size.
    $w cmd plsfont [expr {$::PLPLOT::PL_FCI_MONO}] -1 -1
    $w cmd plscol0a 15 32 32 32 0.70

    set position [expr {$::PLPLOT::PL_POSITION_TOP | $::PLPLOT::PL_POSITION_OUTSIDE}]
    set opt      $opt_base
    set x        0.
    set y        0.1
    set nrow     1
    set ncolumn  $nlegend
    $w cmd pllegend $opt $position $x $y \
        0.05 15 1 1 $nrow $ncolumn \
        $opt_array 1.0 1.0 2.0 \
        1. $text_colors $text \
        {} {} {} {} \
        $line_colors $line_styles $line_widths \
        $symbol_colors $symbol_scales $symbol_numbers $symbols

    set position [expr {$::PLPLOT::PL_POSITION_BOTTOM | $::PLPLOT::PL_POSITION_OUTSIDE}]
    set opt      $opt_base
    set x        0.
    set y        0.1
    set nrow     1
    set ncolumn  $nlegend
    $w cmd pllegend $opt $position $x $y \
        0.05 15 1 1 $nrow $ncolumn \
        $opt_array 1.0 1.0 2.0 \
        1. $text_colors $text \
        {} {} {} {} \
        $line_colors $line_styles $line_widths \
        $symbol_colors $symbol_scales $symbol_numbers $symbols

    set position [expr {$::PLPLOT::PL_POSITION_LEFT | $::PLPLOT::PL_POSITION_OUTSIDE}]
    set opt      $opt_base
    set x        0.1
    set y        0.
    set nrow     $nlegend
    set ncolumn  1
    $w cmd pllegend $opt $position $x $y \
        0.05 15 1 1 $nrow $ncolumn \
        $opt_array 1.0 1.0 2.0 \
        1. $text_colors $text \
        {} {} {} {} \
        $line_colors $line_styles $line_widths \
        $symbol_colors $symbol_scales $symbol_numbers $symbols

    set position [expr {$::PLPLOT::PL_POSITION_RIGHT | $::PLPLOT::PL_POSITION_OUTSIDE}]
    set opt      $opt_base
    set x        0.1
    set y        0.
    set nrow     $nlegend
    set ncolumn  1
    $w cmd pllegend $opt $position $x $y \
        0.05 15 1 1 $nrow $ncolumn \
        $opt_array 1.0 1.0 2.0 \
        1. $text_colors $text \
        {} {} {} {} \
        $line_colors $line_styles $line_widths \
        $symbol_colors $symbol_scales $symbol_numbers $symbols

    set position [expr {$::PLPLOT::PL_POSITION_LEFT | $::PLPLOT::PL_POSITION_TOP | $::PLPLOT::PL_POSITION_INSIDE}]
    set opt      $opt_base
    set x        0.
    set y        0.
    set nrow     6
    set ncolumn  2
    $w cmd pllegend $opt $position $x $y \
        0.05 15 1 1 $nrow $ncolumn \
        $opt_array 1.0 1.0 2.0 \
        1. $text_colors $text \
        {} {} {} {} \
        $line_colors $line_styles $line_widths \
        $symbol_colors $symbol_scales $symbol_numbers $symbols

    set position [expr {$::PLPLOT::PL_POSITION_RIGHT | $::PLPLOT::PL_POSITION_TOP | $::PLPLOT::PL_POSITION_INSIDE}]
    set opt      [expr {$opt_base | $::PLPLOT::PL_LEGEND_ROW_MAJOR}]
    set x        0.
    set y        0.
    set nrow     6
    set ncolumn  2
    $w cmd pllegend $opt $position $x $y \
        0.05 15 1 1 $nrow $ncolumn \
        $opt_array 1.0 1.0 2.0 \
        1. $text_colors $text \
        {} {} {} {} \
        $line_colors $line_styles $line_widths \
        $symbol_colors $symbol_scales $symbol_numbers $symbols

    set position [expr {$::PLPLOT::PL_POSITION_BOTTOM | $::PLPLOT::PL_POSITION_INSIDE}]
    set opt      [expr {$opt_base | $::PLPLOT::PL_LEGEND_ROW_MAJOR}]
    set x        0.
    set y        0.
    set nrow     3
    set ncolumn  3
    $w cmd pllegend $opt $position $x $y \
        0.05 15 1 1 $nrow $ncolumn \
        $opt_array 1.0 1.0 2.0 \
        1. $text_colors $text \
        {} {} {} {} \
        $line_colors $line_styles $line_widths \
        $symbol_colors $symbol_scales $symbol_numbers $symbols

    # Third page demonstrating legend alignment
    $w cmd pladv 0
    $w cmd plvpor 0.0 1.0 0.0 0.9
    $w cmd plwind 0.0 1.0 0.0 1.0
    $w cmd plsfont [expr {$::PLPLOT::PL_FCI_SANS}] -1 -1
    $w cmd plmtex "t" 2.0 0.5 0.5 "Demonstrate legend alignment"

    set x        0.1
    set y        0.1
    set nturn    4
    set nlegend  0
    set position [expr {$::PLPLOT::PL_POSITION_TOP | $::PLPLOT::PL_POSITION_LEFT | $::PLPLOT::PL_POSITION_SUBPAGE}]
    set opt_base [expr {$::PLPLOT::PL_LEGEND_BACKGROUND | $::PLPLOT::PL_LEGEND_BOUNDING_BOX}]
    set opt      $opt_base
    for {set i 0} {$i < 9} {incr i} {
        # Set up legend arrays with the correct size, type.
        if { $i <= $nturn } {
            incr nlegend 1
        } else {
            incr nlegend -1
        }
        set nlegend [max 1 $nlegend]
        # nly specify legend data that are required according to the
        # value of opt_array for that entry.
        set opt_array      [list]
        set line_styles    [list]
        set line_widths    [list]
        set symbol_scales  [list]
        set symbol_numbers [list]
        set symbols        [list]
        set text           [list]
        set text_colors    [list]
        set line_colors    [list]
        set symbol_colors  [list]
        for {set k 0} {$k < $nlegend} {incr k} {
            lappend opt_array      [expr {$::PLPLOT::PL_LEGEND_LINE | $::PLPLOT::PL_LEGEND_SYMBOL}]
            lappend line_styles    1
            lappend line_widths    1.
            lappend symbol_scales  1.
            lappend symbol_numbers 2
            lappend symbols        "#(728)"
            lappend text           [format "%2.2d" $k]
            lappend text_colors    [expr {1 + $k % 8}]
            lappend line_colors    [expr {1 + $k % 8}]
            lappend symbol_colors  [expr {1 + $k % 8}]
        }
        # Use monotype fonts so that all legends are the same size.
        $w cmd plsfont [expr {$::PLPLOT::PL_FCI_MONO}] -1 -1
        $w cmd plscol0a 15 32 32 32 0.70

        set nrow    [min 3 $nlegend]
        set ncolumn 0

        set legend_data [$w cmd pllegend $opt $position $x $y \
            0.025 15 1 1 $nrow $ncolumn \
            $opt_array 1.0 1.0 1.5 \
            1. $text_colors $text \
            {} {} {} {} \
            $line_colors $line_styles $line_widths \
            $symbol_colors $symbol_scales $symbol_numbers $symbols]
        foreach {legend_width legend_height} $legend_data {break}

        if { $i == $nturn } {
            set position [expr {$::PLPLOT::PL_POSITION_TOP | $::PLPLOT::PL_POSITION_RIGHT | $::PLPLOT::PL_POSITION_SUBPAGE}]
            set opt $opt_base
            set x   [expr {1. - $x}]
            set y   [expr {$y + $legend_height}]
        } else {
            set x   [expr {$x + $legend_width}]
            set y   [expr {$y + $legend_height}]
        }
    }

    # Fourth page illustrating various kinds of legends
    set max_height 0.
    set xstart     0.0
    set ystart     0.1
    set x          $xstart
    set y          $ystart
    set text_scale 0.90
    $w cmd pladv 0
    $w cmd plvpor 0.0 1. 0.0 0.90
    $w cmd plwind 0.0 1.0 0.0 1.0
    # $w cmd plbox"bc" 0.0 0 "bc" 0.0 0
    $w cmd plsfont [expr {$::PLPLOT::PL_FCI_SANS}] -1 -1
    $w cmd plmtex "t" 2.0 0.5 0.5 "Demonstrate Various Kinds of Legends"

    set nlegend 5
    # Only specify legend data that are required according to the
    # value of opt_array for that entry.
    set position [expr {$::PLPLOT::PL_POSITION_LEFT | $::PLPLOT::PL_POSITION_TOP}]
    set opt_base [expr {$::PLPLOT::PL_LEGEND_BACKGROUND | $::PLPLOT::PL_LEGEND_BOUNDING_BOX | $::PLPLOT::PL_LEGEND_TEXT_LEFT}]

    # Set up None, Box, Line, Symbol, and Line & Symbol legend entries.

    set opt_array       [list [expr {$::PLPLOT::PL_LEGEND_NONE}]]
    set text            [list "None"]
    set text_colors     [list 1]
    set line_colors     [list {}]
    set line_styles     [list {}]
    set line_widths     [list {}]
    set box_colors      [list {}]
    set box_patterns    [list {}]
    set box_scales      [list {}]
    set box_line_widths [list {}]
    set symbol_scales   [list {}]
    set symbol_numbers  [list {}]
    set symbols         [list {}]
    set symbol_colors   [list {}]

    lappend opt_array       [expr {$::PLPLOT::PL_LEGEND_COLOR_BOX}]
    lappend text            "Box"
    lappend text_colors     2
    lappend line_colors     {}
    lappend line_styles     {}
    lappend line_widths     {}
    lappend box_colors      2
    lappend box_patterns    0
    lappend box_scales      0.8
    lappend box_line_widths 1
    lappend symbol_scales   {}
    lappend symbol_numbers  {}
    lappend symbols         {}
    lappend symbol_colors   {}

    lappend opt_array       [expr {$::PLPLOT::PL_LEGEND_LINE}]
    lappend text            "Line"
    lappend text_colors     3
    lappend line_colors     3
    lappend line_styles     1
    lappend line_widths     1
    lappend box_colors      {}
    lappend box_patterns    {}
    lappend box_scales      {}
    lappend box_line_widths {}
    lappend symbol_scales   {}
    lappend symbol_numbers  {}
    lappend symbols         {}
    lappend symbol_colors   {}

    lappend opt_array       [expr {$::PLPLOT::PL_LEGEND_SYMBOL}]
    lappend text            "Symbol"
    lappend text_colors     4
    lappend line_colors     {}
    lappend line_styles     {}
    lappend line_widths     {}
    lappend box_colors      {}
    lappend box_patterns    {}
    lappend box_scales      {}
    lappend box_line_widths {}
    lappend symbol_colors   4
    lappend symbol_scales   $text_scale
    lappend symbol_numbers  4
    lappend symbols         [lindex $special_symbols 2]

    lappend opt_array       [expr {$::PLPLOT::PL_LEGEND_SYMBOL | $::PLPLOT::PL_LEGEND_LINE}]
    lappend text            "L & S"
    lappend text_colors     5
    lappend line_colors     5
    lappend line_styles     1
    lappend line_widths     1
    lappend box_colors      {}
    lappend box_patterns    {}
    lappend box_scales      {}
    lappend box_line_widths {}
    lappend symbol_colors   5
    lappend symbol_scales   $text_scale
    lappend symbol_numbers  4
    lappend symbols         [lindex $special_symbols 2]

    set opt $opt_base
    $w cmd plscol0a 15 32 32 32 0.70

    set legend_data [$w cmd pllegend $opt $position $x $y \
        0.1 15 1 1 0 0 \
        $opt_array 1.0 $text_scale 2.0 \
        0. $text_colors $text \
        $box_colors $box_patterns $box_scales $box_line_widths \
        $line_colors $line_styles $line_widths \
        $symbol_colors $symbol_scales $symbol_numbers $symbols]

    foreach {legend_width legend_height} $legend_data {break}
    set max_height [max $max_height $legend_height]

    # Set up symbol legend entries with various symbols.
    set opt_array       [list]
    set line_styles     [list]
    set line_widths     [list]
    set symbol_scales   [list]
    set symbol_numbers  [list]
    set symbols         [list]
    set text            [list]
    set text_colors     [list]
    set line_colors     [list]
    set symbol_colors   [list]
    set box_colors      [list]
    set box_patterns    [list]
    set box_scales      [list]
    set box_line_widths [list]
    for { set i 0} {$i < $nlegend} {incr i} {
        lappend opt_array      [expr {$::PLPLOT::PL_LEGEND_SYMBOL}]
        lappend text           "Symbol [lindex $special_symbols $i]"
        lappend text_colors    [expr {$i + 1}]
        lappend symbol_colors  [expr {$i + 1}]
        lappend symbol_scales  $text_scale
        lappend symbol_numbers 4
        lappend symbols        [lindex $special_symbols $i]
    }

    set opt $opt_base
    set x   [expr {$x + $legend_width}]
    $w cmd plscol0a 15 32 32 32 0.70

    set legend_data [$w cmd pllegend $opt $position $x $y \
        0.1 15 1 1 0 0 \
        $opt_array 1.0 $text_scale 2.0 \
        0. $text_colors $text \
        {} {} {} {} \
        {} {} {} \
        $symbol_colors $symbol_scales $symbol_numbers $symbols]
    foreach {legend_width legend_height} $legend_data {break}
    set max_height [max $max_height $legend_height]

    # Set up symbol legend entries with various numbers of symbols.
    set opt_array       [list]
    set line_styles     [list]
    set line_widths     [list]
    set symbol_scales   [list]
    set symbol_numbers  [list]
    set symbols         [list]
    set text            [list]
    set text_colors     [list]
    set line_colors     [list]
    set symbol_colors   [list]
    set box_colors      [list]
    set box_patterns    [list]
    set box_scales      [list]
    set box_line_widths [list]
    for {set i 0} {$i < $nlegend} {incr i} {
        lappend opt_array      [expr {$::PLPLOT::PL_LEGEND_SYMBOL}]
        lappend text           [format "%s %d" "Symbol Number" [expr {$i + 2}]]
        lappend text_colors    [expr {$i + 1}]
        lappend symbol_colors  [expr {$i + 1}]
        lappend symbol_scales  $text_scale
        lappend symbol_numbers [expr {$i + 2}]
        lappend symbols        [lindex $special_symbols 2]
    }

    set opt $opt_base
    set x   [expr {$x + $legend_width}]
    $w cmd plscol0a 15 32 32 32 0.70

    set legend_data [$w cmd pllegend $opt $position $x $y \
        0.1 15 1 1 0 0 \
        $opt_array 1.0 $text_scale 2.0 \
        0. $text_colors $text \
        {} {} {} {} \
        {} {} {} \
        $symbol_colors $symbol_scales $symbol_numbers $symbols]
    foreach {legend_width legend_height} $legend_data {break}
    set max_height [max $max_height $legend_height]

    # Set up box legend entries with various colours.
    set opt_array       [list]
    set line_styles     [list]
    set line_widths     [list]
    set symbol_scales   [list]
    set symbol_numbers  [list]
    set symbols         [list]
    set text            [list]
    set text_colors     [list]
    set line_colors     [list]
    set symbol_colors   [list]
    set box_colors      [list]
    set box_patterns    [list]
    set box_scales      [list]
    set box_line_widths [list]
    for {set i 0} {$i < $nlegend} {incr i} {
        lappend opt_array       [expr {$::PLPLOT::PL_LEGEND_COLOR_BOX}]
        lappend text            [format "%s %d" "Box Color" [expr {$i + 1}]]
        lappend text_colors     [expr {$i + 1}]
        lappend box_colors      [expr {$i + 1}]
        lappend box_patterns    0
        lappend box_scales      0.8
        lappend box_line_widths 1
    }

    set opt $opt_base
    # Use new origin
    set x          $xstart
    set y          [expr {$y + $max_height}]
    set max_height 0.
    $w cmd plscol0a 15 32 32 32 0.70

    set legend_data [$w cmd pllegend $opt $position $x $y \
        0.1 15 1 1 0 0 \
        $opt_array 1.0 $text_scale 2.0 \
        0. $text_colors $text \
        $box_colors $box_patterns $box_scales $box_line_widths \
        {} {} {} \
        {} {} {} {}]
    foreach {legend_width legend_height} $legend_data {break}
    set max_height [max $max_height $legend_height]

    # Set up box legend entries with various patterns.
    set opt_array       [list]
    set line_styles     [list]
    set line_widths     [list]
    set symbol_scales   [list]
    set symbol_numbers  [list]
    set symbols         [list]
    set text            [list]
    set text_colors     [list]
    set line_colors     [list]
    set symbol_colors   [list]
    set box_colors      [list]
    set box_patterns    [list]
    set box_scales      [list]
    set box_line_widths [list]
    for {set i 0} {$i < $nlegend} {incr i} {
        lappend opt_array       [expr {$::PLPLOT::PL_LEGEND_COLOR_BOX}]
        lappend text            [format "%s %d" "Box Pattern" $i]
        lappend text_colors     2
        lappend box_colors      2
        lappend box_patterns    $i
        lappend box_scales      0.8
        lappend box_line_widths 1
    }

    set opt $opt_base
    set x   [expr {$x + $legend_width}]
    $w cmd plscol0a 15 32 32 32 0.70

    set legend_data [$w cmd pllegend $opt $position $x $y \
        0.1 15 1 1 0 0 \
        $opt_array 1.0 $text_scale 2.0 \
        0. $text_colors $text \
        $box_colors $box_patterns $box_scales $box_line_widths \
        {} {} {} \
        {} {} {} {}]
    foreach {legend_width legend_height} $legend_data {break}
    set max_height [max $max_height $legend_height]

    # Set up box legend entries with various box pattern line widths.
    set opt_array       [list]
    set line_styles     [list]
    set line_widths     [list]
    set symbol_scales   [list]
    set symbol_numbers  [list]
    set symbols         [list]
    set text            [list]
    set text_colors     [list]
    set line_colors     [list]
    set symbol_colors   [list]
    set box_colors      [list]
    set box_patterns    [list]
    set box_scales      [list]
    set box_line_widths [list]
    for {set i 0} {$i < $nlegend} {incr i} {
        lappend opt_array       [expr {$::PLPLOT::PL_LEGEND_COLOR_BOX}]
        lappend text            [format "%s %d" "Box Line Width" [expr {$i + 1}]]
        lappend text_colors     2
        lappend box_colors      2
        lappend box_patterns    3
        lappend box_scales      0.8
        lappend box_line_widths [expr {$i + 1}]
    }

    set opt $opt_base
    set x   [expr {$x + $legend_width}]
    $w cmd plscol0a 15 32 32 32 0.70

    set legend_data [$w cmd pllegend $opt $position $x $y \
        0.1 15 1 1 0 0 \
        $opt_array 1.0 $text_scale 2.0 \
        0. $text_colors $text \
        $box_colors $box_patterns $box_scales $box_line_widths \
        {} {} {} \
        {} {} {} {}]
    foreach {legend_width legend_height} $legend_data {break}
    set max_height [max $max_height $legend_height]

    # Set up line legend entries with various colours.
    set opt_array       [list]
    set line_styles     [list]
    set line_widths     [list]
    set symbol_scales   [list]
    set symbol_numbers  [list]
    set symbols         [list]
    set text            [list]
    set text_colors     [list]
    set line_colors     [list]
    set symbol_colors   [list]
    set box_colors      [list]
    set box_patterns    [list]
    set box_scales      [list]
    set box_line_widths [list]
    for {set i 0} {$i < $nlegend} {incr i} {
        lappend opt_array   [expr {$::PLPLOT::PL_LEGEND_LINE}]
        lappend text        [format "%s %d" "Line Color" [expr {$i + 1}]]
        lappend text_colors [expr {$i + 1}]
        lappend line_colors [expr {$i + 1}]
        lappend line_styles 1
        lappend line_widths 1
    }

    set opt $opt_base
    # Use new origin
    set x          $xstart
    set y          [expr {$y + $max_height}]
    set max_height 0.
    $w cmd plscol0a 15 32 32 32 0.70

    set legend_data [$w cmd pllegend $opt $position $x $y \
        0.1 15 1 1 0 0 \
        $opt_array 1.0 $text_scale 2.0 \
        0. $text_colors $text \
        {} {} {} {} \
        $line_colors $line_styles $line_widths \
        {} {} {} {}]
    foreach {legend_width legend_height} $legend_data {break}
    set max_height [max $max_height $legend_height]

    # Set up line legend entries with various styles.
    set opt_array       [list]
    set line_styles     [list]
    set line_widths     [list]
    set symbol_scales   [list]
    set symbol_numbers  [list]
    set symbols         [list]
    set text            [list]
    set text_colors     [list]
    set line_colors     [list]
    set symbol_colors   [list]
    set box_colors      [list]
    set box_patterns    [list]
    set box_scales      [list]
    set box_line_widths [list]
    for {set i 0} {$i < $nlegend} {incr i} {
        lappend opt_array   [expr {$::PLPLOT::PL_LEGEND_LINE}]
        lappend text        [format "%s %d" "Line Style" [expr {$i + 1}]]
        lappend text_colors 2
        lappend line_colors 2
        lappend line_styles [expr {$i + 1}]
        lappend line_widths 1
    }

    set opt $opt_base
    set x   [expr {$x + $legend_width}]
    $w cmd plscol0a 15 32 32 32 0.70

    set legend_data [$w cmd pllegend $opt $position $x $y \
        0.1 15 1 1 0 0 \
        $opt_array 1.0 $text_scale 2.0 \
        0. $text_colors $text \
        {} {} {} {} \
        $line_colors $line_styles $line_widths \
        {} {} {} {}]
    foreach {legend_width legend_height} $legend_data {break}
    set max_height [max $max_height $legend_height]

    # Set up line legend entries with various widths.
    set opt_array       [list]
    set line_styles     [list]
    set line_widths     [list]
    set symbol_scales   [list]
    set symbol_numbers  [list]
    set symbols         [list]
    set text            [list]
    set text_colors     [list]
    set line_colors     [list]
    set symbol_colors   [list]
    set box_colors      [list]
    set box_patterns    [list]
    set box_scales      [list]
    set box_line_widths [list]
    for {set i 0} {$i < $nlegend} {incr i} {
        lappend opt_array   [expr {$::PLPLOT::PL_LEGEND_LINE}]
        lappend text        [format "%s %d" "Line Width" [expr {$i + 1}]]
        lappend text_colors 2
        lappend line_colors 2
        lappend line_styles 1
        lappend line_widths [expr {$i + 1}]
    }

    set opt $opt_base
    set x   [expr {$x + $legend_width}]
    $w cmd plscol0a 15 32 32 32 0.70

    set legend_data [$w cmd pllegend $opt $position $x $y \
        0.1 15 1 1 0 0 \
        $opt_array 1.0 $text_scale 2.0 \
        0. $text_colors $text \
        {} {} {} {} \
        $line_colors $line_styles $line_widths \
        {} {} {} {}]
    foreach {legend_width legend_height} $legend_data {break}
    set max_height [max $max_height $legend_height]

    if {$colorbar} {
        # Color bar examples
        matrix values_small  f 2
        matrix values_uneven f 9
        matrix values_even   f 9
        foreach i {0 1} v {-1.0e-20 1.0e-20 } {
            values_small $i = $v
        }
        foreach i {0 1 2 3 4 5 6 7 8} vu {-1.0e-20 2.0e-20 2.6e-20 3.4e-20 6.0e-20 7.0e-20 8.0e-20 9.0e-20 10.0e-20 } \
                                      ve {-2.0e-20 -1.0e-20 0.0e-20 1.0e-20 2.0e-20 3.0e-20 4.0e-20 5.0e-20 6.0e-20 } {
            values_uneven $i = $vu
            values_even $i = $ve
        }

        # Use unsaturated green background colour to contrast with black caps.
        $w cmd plscolbg 70 185 70
        # Cut out the greatest and smallest bits of the color spectrum to
        # leave colors for the end caps.
        $w cmd plscmap1_range 0.01 0.99

        # We can only test image and gradient colorbars with two element arrays
        #
        # Note: we pass the name of the matrices!
        #
        for {set i 2} {$i < [llength $::colorbar_option_kinds]} {incr i} {
            plcolorbar_example $w "cmap1_blue_yellow.pal" $i 0 0 values_small
        }
        # Test shade colorbars with larger arrays
        for {set i 0} {$i < 2} {incr i} {
            plcolorbar_example $w "cmap1_blue_yellow.pal" $i 4 2 values_even
        }
        for {set i 0} {$i < 2} {incr i} {
            plcolorbar_example $w "cmap1_blue_yellow.pal"  $i 0 0 values_uneven
        }
    }
}

proc plcolorbar_example_page {w kind_i label_i cap_i cont_color cont_width values} {

    global colorbar_position
    global colorbar_position_options
    global colorbar_position_option_labels
    global colorbar_option_kinds
    global colorbar_option_kind_labels
    global colorbar_label_options
    global colorbar_label_option_labels
    global colorbar_cap_options
    global colorbar_cap_option_labels

    # Parameters for the colorbars on this page
    set ticks { 0.0 }
    set sub_ticks { 0 }
    set label_opts { 0 }

    set low_cap_color  0.0
    set high_cap_color 1.0

    # Start a new page
    $w cmd pladv 0

    # Draw one colorbar relative to each side of the page
    for {set position_i 0} {$position_i < [llength $colorbar_position_options]} {incr position_i} {
        set position  [lindex $colorbar_position_options $position_i]
        set opt       [expr {[lindex $colorbar_option_kinds $kind_i] |
                             [lindex $colorbar_label_options $label_i] |
                             [lindex $colorbar_cap_options $cap_i]}]

        set vertical  [expr {($position & $::PLPLOT::PL_POSITION_LEFT) || ($position & $::PLPLOT::PL_POSITION_RIGHT) }]
        set ifn       [expr {($position & $::PLPLOT::PL_POSITION_LEFT) || ($position & $::PLPLOT::PL_POSITION_BOTTOM)}]

        # Set the offset position on the page
        if {$vertical} {
            set x        0.0
            set y        0.0
            set x_length 0.05
            set y_length 0.5
        } else {
            set x        0.0
            set y        0.0
            set x_length 0.5
            set y_length 0.05
        }

        # Set appropriate labelling options.
        if {$ifn} {
            if {$cont_color == 0 || $cont_width == 0.} {
                set axis_opts [list "uwtivn"]
            } else {
                set axis_opts [list "uwxvn"]
            }
        } else {
            if {$cont_color == 0 || $cont_width == 0.} {
                set axis_opts [list "uwtivm"]
            } else {
                set axis_opts [list "uwxvm"]
            }
        }

        set label [list "[lindex $colorbar_position_option_labels $position_i], [lindex $colorbar_label_option_labels $label_i]"]

        # Smaller text
        $w cmd plschr 0.0 0.75
        # Small ticks on the vertical axis
        $w cmd plsmaj 0.0 0.5
        $w cmd plsmin 0.0 0.5

        $w cmd plvpor 0.20 0.80 0.20 0.80
        $w cmd plwind 0.0 1.0, 0.0, 1.0
        # Set interesting background colour.
        $w cmd plscol0a 15 0 0 0 0.20
        $w cmd plcolorbar \
            [expr {$opt | $::PLPLOT::PL_COLORBAR_BOUNDING_BOX | $::PLPLOT::PL_COLORBAR_BACKGROUND}] $position \
            $x $y $x_length $y_length \
            15 1 1 \
            $low_cap_color $high_cap_color \
            $cont_color $cont_width \
            $label_opts $label \
            $axis_opts \
            $ticks $sub_ticks \
            $values

        # Reset text and tick sizes
        $w cmd plschr 0.0 1.0
        $w cmd plsmaj 0.0 1.0
        $w cmd plsmin 0.0 1.0
    }

    # Draw a page title
    set title "[lindex $colorbar_option_kind_labels $kind_i] - [lindex $colorbar_cap_option_labels $cap_i]"
    $w cmd plvpor 0.0 1.0 0.0 1.0
    $w cmd plwind 0.0 1.0 0.0 1.0
    $w cmd plptex 0.5 0.5 0.0 0.0 0.5 $title
}

proc plcolorbar_example {w palette kind_i cont_color cont_width values} {

    # Load the color palette
    $w cmd plspal1 $palette 1

    for { set label_i 0} { $label_i < [llength $::colorbar_label_options] } { incr label_i } {
        for { set cap_i 0 } { $cap_i < [llength $::colorbar_cap_options] } { incr cap_i } {
            plcolorbar_example_page $w $kind_i $label_i $cap_i $cont_color $cont_width $values
        }
    }
}

# Auxiliary routines
proc min {args} {
    set x [lindex $args 0]
    foreach i $args {
	if {$i<$x} {set x $i}
    }
    return $x
}

proc max {args} {
    set x [lindex $args 0]
    foreach i $args {
	if {$i>$x} {set x $i}
    }
    return $x
}
