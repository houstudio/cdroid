#      Copyright (C) 2009  Arjen Markus
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
#      Plots a simple stripchart with four pens.
#

proc x17 {{w loopback}} {

#   If db is used the plot is much more smooth. However, because of the
#   async X behaviour, one does not have a real-time scripcharter.
#

#    plsetopt "db" ""
#    plsetopt "np" ""

#   User sets up plot completely except for window and data
#   Eventually settings in place when strip chart is created will be
#   remembered so that multiple strip charts can be used simultaneously.
#

#  Specify some reasonable defaults for ymin and ymax
#  The plot will grow automatically if needed (but not shrink)

    set nsteps 1000

    set ymin -0.1
    set ymax  0.1

#   Specify initial tmin and tmax -- this determines length of window.
#   Also specify maximum jump in t
#   This can accomodate adaptive timesteps

    set tmin   0.
    set tmax  10.
    set tjump  0.3 ;# percentage of plot to jump

#   Axes options same as plbox.
#   Only automatic tick generation and label placement allowed
#   Eventually I'll make this fancier

    set colbox 1
    set collab 3

    matrix styline i 4 = {2 3 4 5} ;# pens color and line style
    matrix colline i 4 = {2 3 4 5}

    set legline {"sum" "sin" "sin*noi" "sin+noi"} ;# pens legend

    set xlab 0.    ;# legend position
    set ylab 0.25

    set autoy 1    ;# autoscale y
    set acc   1    ;# don't scrip, accumulate

#   Initialize plplot

    $w cmd pladv 0
    $w cmd plvsta

    $w cmd plstripc id1 "bcnst" "bcnstv" \
             $tmin $tmax $tjump $ymin $ymax \
             $xlab $ylab $autoy $acc \
             $colbox $collab \
             colline styline $legline \
             "t" "" "Strip chart demo"

    set autoy 0 ;# autoscale y
    set acc   1 ;# accumulate

#   This is to represent a loop over time
#   Let's try a random walk process

    set y1 0.0
    set y2 0.0
    set y3 0.0
    set y4 0.0

    set dt 0.1

    for {set n 0 } {$n < $nsteps} {incr n} {

        after 10 ;# Wait for 10 ms

        set t       [expr {double($n) * $dt}]
        set noise   [expr {[$w cmd plrandd] - 0.5}]
        set y1      [expr {$y1 + $noise}]
        set y2      [expr {sin($t*$::PLPLOT::PL_PI/18.)}]
        set y3      [expr {$y2 * $noise}]
        set y4      [expr {$y2 + $noise/3.}]

#      There is no need for all pens to have the same number of
#      points or being equally time spaced.

        if { $n%2 } {
            $w cmd plstripa $id1 0 $t $y1
        }
        if { $n%3 } {
            $w cmd plstripa $id1 1 $t $y2
        }
        if { $n%4 } {
            $w cmd plstripa $id1 2 $t $y3
        }
        if { $n%5 } {
            $w cmd plstripa $id1 3 $t $y4
        }
        #$w cmd pleop ;# use double buffer (-db on command line)
    }

#   Destroy strip chart and it's memory

    $w cmd plstripd $id1

#   Restore defaults
    $w cmd pllsty 1
}
