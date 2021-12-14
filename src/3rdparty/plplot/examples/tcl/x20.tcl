#      Copyright (C) 2004-2016  Alan W. Irwin
#      Copyright (C) 2008  Arjen Markus
#
#
#      This file is part of PLplot.
#
#      PLplot is free software; you can redistribute it and/or modify
#      it under the terms of the GNU Library General Public License as
#      published by the Free Software Foundation; either version 2 of the
#      License, or (at your option) any later version.
#
#      PLplot is distributed in the hope that it will be useful,
#      but WITHOUT ANY WARRANTY; without even the implied warranty of
#      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#      GNU Library General Public License for more details.
#
#      You should have received a copy of the GNU Library General Public
#      License along with PLplot; if not, write to the Free Software
#      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
#
#      plimage demo
#
#

#static PLOptionTable options[] = {
#{
#    "dbg",			/* extra debugging plot */
#    NULL,
#    NULL,
#    &dbg,
#    PL_OPT_BOOL,
#    "-dbg",
#    "Extra debugging plot" },
#{
#    "nosombrero",			/* Turns on test of xor function */
#    NULL,
#    NULL,
#    &nosombrero,
#    PL_OPT_BOOL,
#    "-nosombrero",
#    "No sombrero plot" },
#{
#    "nointeractive",			/* Turns on test of xor function */
#    NULL,
#    NULL,
#    &nointeractive,
#    PL_OPT_BOOL,
#    "-nointeractive",
#    "No interactive selection" },
#{
#    "save",			/* For saving in postscript */
#    NULL,
#    NULL,
#    &f_name,
#    PL_OPT_STRING,
#    "-save filename",
#      "Save sombrero plot in color postscript `filename'" },
#{
#    NULL,			/* option */
#    NULL,			/* handler */
#    NULL,			/* client data */
#    NULL,			/* address of variable to set */
#    0,				/* mode flag */
#    NULL,			/* short syntax */
#    NULL }			/* long syntax */
#};

proc x20 {{w loopback}} {

    set PI   $::PLPLOT::PL_PI

    set XDIM 260
    set YDIM 220

    matrix x f $XDIM
    matrix y f $YDIM
    matrix z f $XDIM $YDIM
    matrix r f $XDIM $YDIM

#     Parse and process command line arguments
#
#      $w cmd plMergeOpts options "x20c options" NULL

    set dbg            0
    set nosombrero     0
    set nointeractive  0
    set f_name         ""

    set XDIMM1 [expr {$XDIM-1}]
    set YDIMM1 [expr {$YDIM-1}]

#   View image border pixels
    if { $dbg} {
        $w cmd plenv 1. $XDIM 1. $YDIM 1 1

        for { set i 0 } { $i < $YDIM } { incr i } {
            for { set j 0 } { $j < $YDIM } { incr j } {
                z $i $j = 0.0
            }
        }

#       Build a one pixel square border, for diagnostics

        for { set i 0 } { $i <$XDIM } { incr i } {
#           Right
            z $i $YDIMM1 = 1.0
#           Left
            z $i 0       = 1.0
        }

        for { set i 0 } { $i <$YDIM } { incr i } {
#           Top
            z 0 $i       = 1.0
#           Bottom
            z $XDIMM1 $i = 1.0
        }

        $w cmd pllab "...around a blue square." " " \
            "A red border should appear..."

        $w cmd plimage z 1. $XDIM 1. $YDIM 0. 0. 1. $XDIM 1.$YDIM
    }

#   Sombrero-like demo
    if { ! $nosombrero } {
#       draw a yellow plot box, useful for diagnostics# :(
        $w cmd plcol0 2
        $w cmd plenv 0. [expr {2.*$PI}] 0.0 [expr {3.*$PI}] 1 -1

        for { set i 0 } { $i < $XDIM } { incr i } {
            x $i = [expr {double($i)*2.*$PI/double($XDIMM1)}]
        }
        for { set i 0 } { $i < $YDIM } { incr i } {
            y $i = [expr {double($i)*3.*$PI/double($YDIMM1)}]
        }

        for { set i 0 } { $i < $XDIM } { incr i } {
            for { set j 0 } { $j < $YDIM } { incr j } {
                set xx [x $i]
                set yy [y $j]
                r $i $j = [expr {sqrt($xx*$xx+$yy*$yy)+0.001}]
                set rr [r $i $j]
                z $i $j = [expr {sin($rr) / $rr}]
            }
        }

        $w cmd pllab "No, an amplitude clipped \"sombrero\"" "" "Saturn?"
        $w cmd plptex 2. 2. 3. 4. 0. "Transparent image"
        $w cmd plimage z 0. [expr {2.*$PI}] 0.0 [expr {3.*$PI}] \
            0.05 1. 0. [expr {2.*$PI}]  0. [expr {3.*$PI}]

#       Save the plot
        if { $f_name != "" } {
            save_plot $w $f_name
        }
    }

#
#   Read Chloe image
#   Note we try two different locations to cover the case where this
#   examples is being run from the test_c.sh script
#
    if { ![read_img "Chloe.pgm" img_f width height num_col] } {
        if { ![read_img "../Chloe.pgm" img_f width height num_col] } {
#C          $w cmd plabort "No such file"
            puts "Image could not be read"
#            $w cmd plend
            return
        }
    }

#   Set gray colormap
    gray_cmap $w $num_col

#   Display Chloe
    $w cmd plenv 1. $width 1. $height 1 -1

    if { !$nointeractive } {
        $w cmd pllab "Set and drag Button 1 to (re)set selection, Button 2 to finish." \
            " " "Chloe..."
    } else {
        $w cmd pllab "" " " "Chloe..."
    }

    $w cmd plimage img_f 1. $width 1. $height 0. 0. 1. $width 1. $height

#   Selection/expansion demo
    if { !$nointeractive } {
        set xi 25.0
        set xe 130.0
        set yi 235.0
        set ye 125.0

        if { [get_clip $w $xi $xe $yi $ye] } {
#            $w cmd plend
            return
        }

#
#       I'm unable to continue, clearing the plot and advancing to the next
#       one, without hiting the enter key, or pressing the button... help#
#       Forcing the xwin driver to leave locate mode and destroying the
#       xhairs  in GetCursorCmd    solves some problems but I still have
#       to press the enter key or press Button-2 to go to next plot, even
#       if a pladv() is not present#  Using plbop() solves the problem, but
#       it shouldn't be needed#
#
#       plspause(0), pladv(0), plspause(1), also works,
#       but the above question remains.
#       With this approach, the previous pause state is lost,
#       as there is no API $w cmd to get its current state.
#

        $w cmd plspause 0
        $w cmd pladv 0

#       Display selection only
        $w cmd plimage img_f 1. $width 1. $height 0. 0. $xi $xe $ye $yi

        $w cmd plspause 1

#       Zoom in selection
        $w cmd plenv $xi $xe $ye $yi 1 -1
        $w cmd plimage img_f 1. $width 1. $height 0. 0. $xi $xe $ye $yi
    }

#   Base the dynamic range on the image contents.

    a2mnmx img_f $width $height img_min img_max

    $w cmd plcol0 2
    $w cmd plenv 0. $width 0. $height 1 -1
    $w cmd pllab "" "" "Reduced dynamic range image example"
    $w cmd plimagefr img_f 0. $width 0. $height 0. 0. \
         [expr {$img_min + $img_max * 0.25}] \
         [expr {$img_max - $img_max * 0.25}]

#   Draw a distorted version of the original image, showing its
#   full dynamic range.
    $w cmd plenv 0. $width 0. $height 1 -1
    $w cmd pllab "" "" "Distorted image example"

#   Populate the 2-d grids used for the distortion
#   NB grids must be 1 larger in each dimension than the image
#   since the coordinates are for the corner of each pixel.

    set widthp1  [expr {$width+1}]
    set heightp1 [expr {$height+1}]
    matrix xg f $widthp1 $heightp1
    matrix yg f $widthp1 $heightp1

    set x0      [expr {0.5*$width}]
    set y0      [expr {0.5*$height}]
    set dy      [expr {0.5*$height}]
    set stretch 0.5
    for { set i 0 } { $i < $widthp1 } { incr i } {
        for { set j 0 } { $j < $heightp1 } { incr j } {
            xg $i $j = [expr {$x0 + ($x0-double($i))*(1.0 - $stretch *
                              cos((double($j)-$y0)/$dy*$PI*0.5))}]
            yg $i $j = $j
        }
    }
    $w cmd plimagefr img_f 0. $width 0. $height 0. 0. $img_min \
        $img_max xg yg

#    $w cmd plend
}

# -------------------------------------------
#   Read image from file in binary ppm format
proc read_img {fname img_f_name width_name height_name num_col_name} {
    upvar 1 $img_f_name   img_f
    upvar 1 $width_name   width
    upvar 1 $height_name  height
    upvar 1 $num_col_name num_col

#   Naive grayscale binary ppm reading. If you know how to, improve it

    if { [catch {
        set infile [open $fname r]
    }] } {
        return 0 ;# File does not exist
    }

    if { [catch {
#       I only understand "P5"#
        set ver [gets $infile]
        if { $ver != "P5" } {
            return 0
        }

        while {1} {
            set ver [gets $infile]

            if { [string index $ver 0] != "#" } {
                break
            }
        }

#       Found the line with the sizes

        scan $ver "%d %d" w h

        set ver [gets $infile]
        scan $ver "%d" num_col

        matrix img_f f $w $h

#
#   Read the second part - it becomes binary
#
        fconfigure $infile -translation binary

        set picture [read $infile]
        close $infile

#
#   The picture needs to be flipped vertically.
#   So do that rightaway
#
#
        set count 0
        for { set j 0 } { $j < $h } { incr j } {
            for { set i 0 } { $i < $w } { incr i } {

                # The u flag for binary scan is only available
                # with tcl8.5 onwards - use a simpler method
                # for compatibility.
                #binary scan [string index $picture $count] cu value
                binary scan [string index $picture $count] c value
                set value [expr ( $value + 0x100 ) % 0x100]
                img_f $i [expr {$h-$j-1}] = $value
                incr count
            }
        }

        set width  $w
        set height $h
    } msg] } {
        # Some error occurred
        puts "Error reading picture file: $msg"
        return 0
    } else {
        return 1
    }
}

#   Save plot
proc save_plot {w fname} {

#   Get current stream
    $w cmd plgstrm cur_strm

#   Create a new one
    $w cmd plmkstrm new_strm

#   New device type. Use a known existing driver
    $w cmd plsdev "psc"
    $w cmd plsfnam $fname

#   Copy old stream parameters to new stream
    $w cmd plcpstrm $cur_strm 0
    $w cmd plreplot
    $w cmd plend1

#   Return to previous one
    $w cmd plsstrm $cur_strm
}

#   Get selection square interactively
proc get_clip {w xi xe yi ye} {

#   How do we translate that?
#   type(PLGraphicsIn) :: gin

    return 0 ;# getcursor not supported!

    matrix sx f 5
    matrix sy f 5

    set PLK_Return "\r"

    scan "Q" %c Q

    set xxi   $xi
    set yyi   $yi
    set xxe   $xe
    set yye   $ye
    set start 0

#   Enter xor mode to draw a selection rectangle
    $w cmd plxormod 1 st


#   Driver has xormod capability, continue
    if { $st } {
       while {1} {

          $w cmd plxormod 0 st
          $w cmd plgetcursor gin
          $w cmd plxormod 1 st

          if { $gin_button == 1 } {
             set xxi $gin_wX
             set yyi $gin_wY
             if { $start } {
#C              clear previous rectangle
                $w cmd plline sx sy
             }

             set start 0

             sx 0 = $xxi
             sy 0 = $yyi
             sx 4 = $xxi
             sy 4 = $yyi
          }

          if { $gin_state & 0x100 != 0 } {
             set xxe $gin_wX
             set yye $gin_wY
             if { $start } {
#               Clear previous rectangle
                $w cmd plline sx sy
             }
             set start 1

             sx 2 = $xxe
             sy 2 = $yye
             sx 1 = $xxe
             sy 1 = $yyi
             sx 3 = $xxi
             sy 3 = $yye
#            Draw new rectangle
             $w cmd plline sx sy
          }

          if {($gin_button == 3) || ($gin_keysym == $PLK_Return) || \
              ($gin_keysym == $Q) } {
             if { $start } {
#               Clear previous rectangle
                $w cmd plline sx sy
                break
             }
          }
       }

#      Leave xor mode
       $w cmd plxormod 0 st

       if { $xxe < $xxi } {
           set t   $xxi
           set xxi $xxe
           set xxe $t
       }

       if { $yyi < $yye } {
           set t   $yyi
           set yyi $yye
           set yye $t
       }

       set xe $xxe
       set xi $xxi
       set ye $yye
       set yi $yyi

       set get_clip [expr { $gin_keysym == $Q}]
    } else {
#       driver has no xormod capability, just do nothing
        return 0
    }
}

#   Set gray colormap
proc gray_cmap {w num_col} {

    matrix R f 2
    matrix G f 2
    matrix B f 2
    matrix pos f 2
    matrix rev i 2

    R 0 = 0.0
    G 0 = 0.0
    B 0 = 0.0
    R 1 = 1.0
    G 1 = 1.0
    B 1 = 1.0

    pos 0 = 0.0
    pos 1 = 1.0
    rev 0 = 0
    rev 1 = 0

    $w cmd plscmap1n $num_col
    $w cmd plscmap1l 1 pos R G B rev

}


#----------------------------------------------------------------------------
#      proc max and min
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


#----------------------------------------------------------------------------
#      proc a2mnmx
#      Minimum and the maximum elements of a 2-d array.

proc a2mnmx {f nx ny fmin fmax} {
    upvar 1 $fmin vmin
    upvar 1 $fmax vmax

    set vmax [$f 0 0]
    set vmin $vmax
    for {set j 0} {$j < $ny} {incr j} {
        for {set i 0} {$i < $nx} {incr i} {
            set vmax [max $vmax [$f $i $j]]
            set vmin [min $vmin [$f $i $j]]
        }
    }
}
