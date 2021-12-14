proc x15 {{w loopback}} {
  set xpts 35
  set ypts 46
  matrix z f $xpts $ypts
  # Comment out set up of cmap0 with limited number of colours
  # $w cmd plscmap0n 3
  # Comment out set up of cmap1 since actually cmap0 is used, and to
  # fiddle with this affects other tcl plots bundled with this example.
  # cmap1_init152 $w

  for {set i 0} {$i < $xpts} {incr i} {
    set x [expr {double($i - ($xpts/2)) / double($xpts/2)}]
    for {set j 0} {$j < $ypts} {incr j} {
      set y [expr {double($j - ($ypts/2)) / double($ypts/2) - 1.}]

      z $i $j = [expr {$x*$x - $y*$y + ($x - $y)/($x * $x + $y * $y + .1)} ]
    }
  }
  set zmin [z 0 0]
  set zmax $zmin
  for {set i 0} {$i < $xpts} {incr i} {
    for {set j 0} {$j < $ypts} {incr j} {
      if {[z $i $j] < $zmin} { set zmin [z $i $j] }
      if {[z $i $j] > $zmax} { set zmax [z $i $j] }
    }
  }
  plot151 $w $z $zmin $zmax
  plot152 $w $z $zmin $zmax
  plot153 $w

}

proc  cmap1_init152 { w } {

  matrix i f 4
  matrix h f 4
  matrix l f 4
  matrix s f 4
  matrix rev f 4
  i 0 = 0.0
  i 1 = 0.45
  i 2 = 0.55
  i 3 = 1.0

  h 0 = 260.0
  h 1 = 260.0
  h 2 = 20.0
  h 3 = 20.0

  l 0 = 0.6
  l 1 = 0.0
  l 2 = 0.0
  l 3 = 0.6

  s 0 = 1.0
  s 1 = 0.5
  s 2 = 0.5
  s 3 = 1.0

  rev 0 = 0
  rev 1 = 0
  rev 2 = 0
  rev 3 = 0

  $w cmd plscmap1l 0 i h l s rev
}
proc plot151 { w z zmin zmax } {

  set sh_width 2
  set min_color 9
  set max_color 2
  set min_width 2.
  set max_width 2.
  # N.B. this flag set to use cmap0
  set sh_cmap 0

  set shade_min [expr {$zmin + ($zmax - $zmin)*.4} ]
  set shade_max [expr {$zmin + ($zmax - $zmin)*.6} ]
  set sh_color 7

  $w cmd pladv 0
  $w cmd plvpor .1 .9 .1 .9
  $w cmd plwind -1.0 1.0 -1.0 1.0

  $w cmd plpsty 8
  $w cmd plshade z -1. 1. -1. 1. $shade_min $shade_max \
    $sh_cmap $sh_color $sh_width \
    $min_color $min_width $max_color $max_width 1 "NULL"
  $w cmd plcol0 1
  $w cmd plbox bcnst 0.0 0 bcnstv 0.0 0
  $w cmd plcol0 2
  $w cmd pllab "distance" "altitude" "Bogon flux"
}

proc plot152 { w z zmin zmax } {

  set sh_width 2
  set min_color 0
  set max_color 0
  set min_width 0.
  set max_width 0.
  # N.B. this flag set to use cmap0
  set sh_cmap 0

  matrix nlin i 10 = {1, 1, 1, 1, 1, 2, 2, 2, 2, 2}
  matrix inc0 i 1 = {450}
  matrix inc1 i 1 = {-450}
  matrix inc2 i 1 = {0}
  matrix inc3 i 1 = {900}
  matrix inc4 i 1 = {300}
  matrix inc5 i 2 = {450, -450}
  matrix inc6 i 2 = {0, 900}
  matrix inc7 i 2 = {0, 450}
  matrix inc8 i 2 = {450, -450}
  matrix inc9 i 2 = {0, 900}
  matrix del0 i 1 = {2000}
  matrix del1 i 1 = {2000}
  matrix del2 i 1 = {2000}
  matrix del3 i 1 = {2000}
  matrix del4 i 1 = {2000}
  matrix del5 i 2 = {2000, 2000}
  matrix del6 i 2 = {2000, 2000}
  matrix del7 i 2 = {2000, 2000}
  matrix del8 i 2 = {4000, 4000}
  matrix del9 i 2 = {4000, 2000}

  $w cmd pladv 0
  $w cmd plvpor .1 .9 .1 .9
  $w cmd plwind -1.0 1.0 -1.0 1.0
  for {set i 0} {$i < 10} {incr i} {
    set shade_min [expr {$zmin + ($zmax - $zmin)*$i/10.0} ]
    set shade_max [expr {$zmin + ($zmax - $zmin)*($i + 1)/10.0} ]
    set sh_color [expr {$i + 6}]
    $w cmd plpat inc$i del$i
    # Alternatively:
    # $w cmd plpat [nlin $i] inc$i del$i
    $w cmd plshade z -1. 1. -1. 1. $shade_min $shade_max \
      $sh_cmap $sh_color $sh_width \
      $min_color $min_width $max_color $max_width 1 "NULL"
  }
  $w cmd plcol0 1
  $w cmd plbox bcnst 0.0 0 bcnstv 0.0 0
  $w cmd plcol0 2
  $w cmd pllab "distance" "altitude" "Bogon flux"
# Restore defaults
  # $w cmd plcol0 1
}

proc plot153 { w } {

    matrix xx0 f 5 = {-1.0, 1.0, 1.0, -1.0, -1.0}
    matrix xx1 f 5 = {-1.0, 1.0, 1.0, -1.0, -1.0}
    matrix yy0 f 5 = {1.0, 1.0, 0.0, 0.0, 1.0}
    matrix yy1 f 5 = {-1.0, -1.0, 0.0, 0.0, -1.0}
    matrix zz0 f 5 = {0.0, 0.0, 1.0, 1.0, 0.0}
    matrix zz1 f 5 = {0.0, 0.0, 1.0, 1.0, 0.0}

    $w cmd pladv 0
    $w cmd plvpor 0.1 0.9 0.1 0.9
    $w cmd plwind -1.0 1.0 -1.0 1.0
    $w cmd plw3d 1. 1. 1. -1.0 1.0 -1.0 1.0 0.0 1.5 30 -40

    # Plot using identity transform

    $w cmd plcol0 1
    $w cmd plbox3 "bntu" "X" 0.0 0 "bntu" "Y" 0.0 0 "bcdfntu" "Z" 0.5 0
    $w cmd plcol0 2
    $w cmd pllab "" "" "3-d polygon filling"

    $w cmd plcol0 3
    $w cmd plpsty 1
    $w cmd plline3 xx0 yy0 zz0
    $w cmd plfill3 xx0 yy0 zz0
    $w cmd plpsty 2
    $w cmd plline3 xx1 yy1 zz1
    $w cmd plfill3 xx1 yy1 zz1

    # Restore defaults
    # $w cmd plcol0 1
}
