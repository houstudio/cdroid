#  Drawing "spirograph" curves - epitrochoids, cycolids, roulettes
#
#  Copyright (C) 2007  Arjen Markus
#  Copyright (C) 2008  Andrew Ross
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

# --------------------------------------------------------------------------
# main
#
# Generates two kinds of plots:
#   - construction of a cycloid (animated)
#   - series of epitrochoids and hypotrochoids
# --------------------------------------------------------------------------

proc x27 {{w loopback}} {

  #     R, r, p, N
  set params {
    { 21.0   7.0   7.0   3.0  }
    { 21.0   7.0  10.0   3.0  }
    { 21.0  -7.0  10.0   3.0  }
    { 20.0   3.0   7.0  20.0  }
    { 20.0   3.0  10.0  20.0  }
    { 20.0  -3.0  10.0  20.0  }
    { 20.0  13.0   7.0  20.0  }
    { 20.0  13.0  20.0  20.0  }
    { 20.0 -13.0  20.0  20.0  }}

  #  Illustrate the construction of a cycloid

  cycloid $w

  #  Loop over the various curves
  #  First an overview, then all curves one by one
  $w cmd pladv 0
  $w cmd plssub 3 3

  set fill 0

  for { set i 0 } { $i < 9 } { incr i } {
     $w cmd pladv 0
     $w cmd plvpor 0.0  1.0  0.0  1.0
     spiro $w [lindex $params $i] $fill
  }
  $w cmd pladv 0
  $w cmd plssub 1 1

  for { set i 0 } { $i < 9 } { incr i } {
     $w cmd pladv 0
     $w cmd plvpor 0.0  1.0  0.0  1.0
     spiro $w [lindex $params $i] $fill
  }

  #  Fill the curves
  set fill 1

  $w cmd pladv 0
  $w cmd plssub 1 1 ;# One window per curve

  for { set i 0 } { $i < 9 } { incr i } {
      $w cmd  pladv 0
      $w cmd  plvpor 0.0 1.0 0.0 1.0
      spiro $w [lindex $params $i] $fill
  }

  arcs $w
}

#--------------------------------------------------------------------------
# Calculate greatest common divisor following pseudo-code for the
# Euclidian algorithm at http://en.wikipedia.org/wiki/Euclidean_algorithm

proc gcd {a b} {

    set a [expr {int(abs($a))}]
    set b [expr {int(abs($b))}]
    while { $b != 0 } {
        set t $b
        set b [expr {$a % $b}]
        set a $t
    }
    return $a
}

#  ===============================================================

proc cycloid {w} {

  #     TODO

}

#  ===============================================================

proc spiro {w params fill} {

  foreach {param1 param2 param3 param4} $params {break}

  #     Fill the coordinates

  set NPNT 2000

  #     Proper termination of the angle loop very near the beginning
  #     point, see
  #     http://mathforum.org/mathimages/index.php/Hypotrochoid.
  set windings [expr {int(abs($param2)/[gcd $param1 $param2])}]
  set steps    [expr {int($NPNT/$windings)}]
  set dphi     [expr {2.0*$::PLPLOT::PL_PI/double($steps)}]
  # puts [ format "windings, steps, dphi = %d, %d, %f" $windings $steps $dphi ]

  set n [expr {int($windings*$steps)+1}]

  matrix xcoord f $n
  matrix ycoord f $n

  for { set i 0 } { $i < $n } { incr i } {
     set phi  [expr {double($i) * $dphi}]
     set phiw [expr {($param1-$param2)/$param2*$phi}]
     xcoord $i = [expr {($param1-$param2)*cos($phi)+$param3*cos($phiw)}]
     ycoord $i = [expr {($param1-$param2)*sin($phi)-$param3*sin($phiw)}]

     if { $i == 0} {
	set xmin [xcoord 0]
	set xmax [xcoord 0]
	set ymin [ycoord 0]
	set ymax [ycoord 0]
     }
     if { $xmin > [xcoord $i] } { set xmin [xcoord $i] }
     if { $xmax < [xcoord $i] } { set xmax [xcoord $i] }
     if { $ymin > [ycoord $i] } { set ymin [ycoord $i] }
     if { $ymax < [ycoord $i] } { set ymax [ycoord $i] }
  }

  set xrange_adjust [expr {0.15 * ($xmax - $xmin) }]
  set xmin [expr {$xmin - $xrange_adjust }]
  set xmax [expr {$xmax + $xrange_adjust }]
  set yrange_adjust [expr {0.15 * ($ymax - $ymin) }]
  set ymin [expr {$ymin - $yrange_adjust }]
  set ymax [expr {$ymax + $yrange_adjust }]

  $w cmd plwind $xmin $xmax $ymin $ymax

  $w cmd plcol0 1

  if { $fill } {
      $w cmd plfill xcoord ycoord
  } else {
      $w cmd plline xcoord ycoord
  }
}

proc arcs {w} {
    set NSEG 8
    set pi $::PLPLOT::PL_PI

    set theta 0.0
    set dtheta [expr {360.0 / $NSEG}]
    $w cmd plenv -10.0 10.0 -10.0 10.0 1 0

    # Plot segments of circle in different colors
    for { set i 0 } { $i < $NSEG } {incr i} {
        $w cmd plcol0 [expr {$i%2 + 1}]
        $w cmd plarc 0.0 0.0 8.0 8.0 $theta [expr {$theta + $dtheta}] 0.0 0
        set theta [expr {$theta + $dtheta}]
    }

    # Draw several filled ellipses inside the circle at different
    # angles.
    set a 3.0
    set b [expr {$a * tan( ($dtheta/180.0*$pi)/2.0 )}]
    set theta  [expr {$dtheta/2.0}]
    for {set i 0} { $i < $NSEG } { incr i } {
        $w cmd plcol0 [expr {2 - $i%2}]
        $w cmd plarc [expr {$a*cos($theta/180.0*$pi)}] [expr {$a*sin($theta/180.0*$pi)}] $a $b 0.0 360.0 $theta 1
	set theta [expr {$theta + $dtheta}]
    }

}
