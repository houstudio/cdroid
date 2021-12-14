#      Grid data demo.
#
#      Copyright (C) 2004  Joao Cardoso
#      Copyright (C) 2008  Andrew Ross
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
#      Note:
#      This example uses NaNs (not-a-number) and these special "numbers"
#      are only properly supported by Tcl 8.5 and later (in the sense
#      that a NaN does not stop the program with an error message).

proc x21 {{w loopback}} {

    set PI          $::PLPLOT::PL_PI

    set pts 500
    set xp        25
    set yp        20
    set nl        16
    set knn_order 20
    set threshold 1.001
    set wmin      -1e3
    set randn     0
    set rosen     0

    matrix x    f $pts
    matrix y    f $pts
    matrix z    f $pts
    matrix clev f $nl
    matrix xg   f $xp
    matrix yg   f $yp
    matrix zg   f $xp $yp

    set title {"-dummy-"
               "Cubic Spline Approximation"
               "Delaunay Linear Interpolation"
               "Natural Neighbors Interpolation"
               "KNN Inv. Distance Weighted"
               "3NN Linear Interpolation"
               "4NN Around Inv. Dist. Weighted"}

    matrix opt f 6 = { 0.0 0.0 0.0 0.0 0.0 0.0 }

    set xmin -0.2
    set ymin -0.2
    set xmax 0.6
    set ymax 0.6

    opt 2 = $wmin
    opt 3 = [expr {double($knn_order)}]
    opt 4 = $threshold

    $w cmd plseed 5489

    cmap1_init $w

    for {set i 0} {$i < $pts} {incr i} {
        set xt [expr {($xmax-$xmin)*[$w cmd plrandd]}]
        set yt [expr {($ymax-$ymin)*[$w cmd plrandd]}]
        if {$randn == 0} {
            x $i = [expr {$xt + $xmin}]
            y $i = [expr {$yt + $ymin}]
        } else {
            x $i = [expr {sqrt(-2.*log($xt)) * cos(2.*$PI*$yt) + $xmin}]
            y $i = [expr {sqrt(-2.*log($xt)) * sin(2.*$PI*$yt) + $ymin}]
        }
        if {$rosen == 0} {
            set xx [x $i]
            set yy [y $i]
            set r [expr {sqrt($xx*$xx + $yy*$yy)}]
            z $i = [expr {exp(-$r*$r)*cos(2.*$PI*$r)}]
        } else {
            set xx [x $i]
            set yy [y $i]
            z $i = [expr {log(pow(1.-$xx,2)) + 100.*pow($yy-pow($xx,2),2)}]
        }
    }

    set zmin [z 0]
    set zmax [z 0]
    for {set i 1} {$i < $pts} {incr i} {
        set zmax [max $zmax [z $i]]
        set zmin [min $zmin [z $i]]
    }

    for {set i 0} {$i < $xp} {incr i} {
        xg $i = [expr {$xmin + ($xmax-$xmin)*$i/double($xp-1.)}]
    }
    for {set i 0} {$i < $yp} {incr i} {
        yg $i = [expr {$ymin + ($ymax-$ymin)*$i/double($yp-1.)}]
    }

    $w cmd plcol0 1
    $w cmd plenv $xmin $xmax $ymin $ymax 2 0
    $w cmd plcol0 15
    $w cmd pllab "X" "Y" "The original data sampling"

    matrix xstr f 1 = {0.0}
    matrix ystr f 1 = {0.0}

    for {set i 0} {$i < $pts} {incr i} {
        $w cmd plcol1 [expr {([z $i] - $zmin ) / ( $zmax - $zmin )}]
        # The following plstring call should be the the equivalent of
        # plpoin( 1, &x[i], &y[i], 5 ); Use plstring because it is
        # not deprecated like plpoin and has much more powerful
        # capabilities.  N.B. symbol 141 works for Hershey devices
        # (e.g., -dev xwin) only if plfontld( 0 ) has been called
        # while symbol 727 works only if plfontld( 1 ) has been
        # called.  The latter is the default which is why we use 727
        # here to represent a centred X (multiplication) symbol.
        # This dependence on plfontld is one of the limitations of
        # the Hershey escapes for PLplot, but the upside is you get
        # reasonable results for both Hershey and Unicode devices.
        xstr 0 = [x $i]
        ystr 0 = [y $i]
        $w cmd plstring xstr ystr "#(727)"
    }
    $w cmd pladv 0

    $w cmd plssub 3 2

    for {set k 0} {$k < 2} {incr k} {
        $w cmd pladv 0
        for {set alg 1} {$alg <= 6} {incr alg} {

            $w cmd plgriddata x y z xg yg zg $alg [opt [expr {$alg-1}]]

#     - CSA can generate NaNs (only interpolates? #).
#     - DTLI and NNI can generate NaNs for points outside the convex hull
#     of the data points.
#     - NNLI can generate NaNs if a sufficiently thick triangle is not found
#
#     PLplot should be NaN/Inf aware, but changing it now is quite a job...
#     so, instead of not plotting the NaN regions, a weighted average over
#     the neighbors is done.
#

            if {($alg == $::PLPLOT::GRID_CSA)  || ($alg == $::PLPLOT::GRID_DTLI) ||
                ($alg == $::PLPLOT::GRID_NNLI) || ($alg == $::PLPLOT::GRID_NNI)} {

                for {set i 0} {$i < $xp} {incr i} {
                    for {set j 0} {$j < $yp} {incr j} {
                        if { [isnan [zg $i $j]] } {
#     average (IDW) over the 8 neighbors

                            zg $i $j = 0.
                            set dist 0.

                            set ii [expr {$i-1}]
                            while {($ii <= $i+1) && ($ii < $xp)} {
                                set jj [expr {$j-1}]
                                while {($jj <= $j+1) && ($jj < $yp)} {
                                    if {($ii >= 0) && ($jj >= 0) &&
                                        ![isnan [zg $ii $jj]] } {
                                        if {abs($ii-$i) + abs($jj-$j) == 1} {
                                            set d 1.
                                        } else {
                                            set d 1.4142
                                        }
                                        zg $i $j = [expr {[zg $i $j] + [zg $ii $jj]/($d*$d)}]
                                        set dist [expr {$dist + $d}]
                                    }
                                    incr jj
                                }
                                incr ii
                            }
                            if {$dist != 0.} {
                                zg $i $j = [expr {[zg $i $j] / $dist}]
                            } else {
                                zg $i $j = $zmin
                            }
                        }
                    }
                }
            }

            a2mnmx zg $xp $yp lzmin lzmax

            set lzmin [min $lzmin $zmin]
            set lzmax [max $lzmax $zmax]

            set lzmin [expr {$lzmin - 0.01}]
            set lzmax [expr {$lzmax + 0.01}]

            $w cmd plcol0 1
            $w cmd pladv $alg

            if {$k == 0} {

                for {set i 0} {$i < $nl} {incr i} {
                    clev $i = [expr {$lzmin + ($lzmax-$lzmin)/double($nl-1.)*$i}]
                }
                $w cmd plenv0 $xmin $xmax $ymin $ymax 2 0
                $w cmd plcol0 15
                $w cmd pllab "X" "Y" [lindex $title $alg]
                $w cmd plshades zg $xmin $xmax $ymin $ymax clev 1. 0 1. 1 "NULL"
                $w cmd plcol0 2
            } else {

                for {set i 0} {$i < $nl} {incr i} {
                    clev $i = [expr {$lzmin + ($lzmax-$lzmin)/double($nl-1.)*$i}]
                }
                $w cmd plvpor 0. 1. 0. 0.9
                $w cmd plwind -1.1 0.75 -0.65 1.20
#
#     For the comparison to be fair, all plots should have the
#     same z values, but to get the max/min of the data generated
#     by all algorithms would imply two passes. Keep it simple.
#
#     plw3d(1., 1., 1., xmin, xmax, ymin, ymax, zmin, zmax, 30, -60);
#

                $w cmd plw3d 1. 1. 1. $xmin $xmax $ymin $ymax \
                     $lzmin $lzmax 30. -40.
                $w cmd plbox3 "bntu"    "X" 0.  0 \
                              "bntu"    "Y" 0.  0 \
                              "bcdfntu" "Z" 0.5 0
                $w cmd plcol0 15
                $w cmd pllab "" "" [lindex $title $alg]
                $w cmd plot3dc xg yg zg \
                   [expr {$::PLPLOT::DRAW_LINEXY|$::PLPLOT::MAG_COLOR|$::PLPLOT::BASE_CONT}] clev
            }
        }
    }

    $w cmd plflush
    #$w cmd plend
    $w cmd plssub 1 1
}

#----------------------------------------------------------------------------
#      isnan
#
# Note: the current string interface exposes the string representation
# of NaNs to the Tcl side. As there is a very wide variety of such
# represetations (NAN, nan, "-1.#IND", "1.#QNAN" being but a few
# - see also: http://wiki.apache.org/stdcxx/FloatingPoint)
# we take a shortcut:
# - x is supposed to be a valid number, finite or NaN
# - if it is not recognised as a double precision number, it is assumed
#   to be NaN
# - if it is not equal to itself, it definitely is a NaN, as that is
#   the distinguishing property of NaNs.
#
#
proc isnan {x} {
    if {![string is double $x] || $x != $x} {
        return 1
    } else {
        return 0
    }
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
#      proc cmap1_init
#      Set up the colour map
proc cmap1_init {w} {

    matrix i f 2
    matrix h f 2
    matrix l f 2
    matrix s f 2
    matrix r i 2

    i 0  = 0.
    i 1  = 1.

    h 0  = 240.
    h 1  = 0.

    l 0  = 0.6
    l 1  = 0.6

    s 0  = 0.8
    s 1  = 0.8

    r 0  = 0
    r 1  = 0

    $w cmd plscmap1n 256
    $w cmd plscmap1l 0 i h l s r
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
