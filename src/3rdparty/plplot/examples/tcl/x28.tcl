#	plmtex3, plptex3 demo.
#
#   Copyright (C) 2007 Alan W. Irwin
#
#  This file is part of PLplot.
#
#  PLplot is free software; you can redistribute it and/or modify
#  it under the terms of the GNU Library General Public License as published
#  by the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  PLplot is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Library General Public License for more details.
#
#  You should have received a copy of the GNU Library General Public License
#  along with PLplot; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
#

#
# Demonstrates plotting text in 3D.
#

proc x28 {{w loopback}} {

    # Choose these values to correspond to tick marks.
    set XPTS 2
    set YPTS 2
    set NREVOLUTION 16
    set NROTATION 8
    set NSHEAR 8

    set pi $::PLPLOT::PL_PI

    set xmin 0.
    set xmax 1.0
    set xmid [expr {0.5*($xmax + $xmin)}]
    set xrange [expr {$xmax - $xmin}]
    set ymin 0.
    set ymax 1.0
    set ymid [expr {0.5*($ymax + $ymin)}]
    set yrange [expr {$ymax - $ymin}]
    set zmin 0.
    set zmax 1.0
    set zmid [expr {0.5*($zmax + $zmin)}]
    set zrange [expr {$zmax - $zmin}]

    set ysmin [expr {$ymin + 0.1 * $yrange}]
    set ysmax [expr {$ymax - 0.1 * $yrange}]
    set ysrange [expr {$ysmax - $ysmin}]
    set dysrot [expr {$ysrange / ( $NROTATION - 1 )}]
    set dysshear [expr {$ysrange / ( $NSHEAR - 1 )}]
    set zsmin [expr {$zmin + 0.1 * $zrange}]
    set zsmax [expr {$zmax - 0.1 * $zrange}]
    set zsrange [expr {$zsmax - $zsmin}]
    set dzsrot [expr {$zsrange / ( $NROTATION - 1 )}]
    set dzsshear [expr {$zsrange / ( $NSHEAR - 1 )}]

    # p1string must be exactly one character + the null termination
    # character.
    set p1string "O"
    set pstring "The future of our civilization depends on software freedom."
    # Allocate and define the minimal x, y, and z to insure 3D box
    matrix x f $XPTS
    matrix y f $YPTS
    matrix z f $XPTS $YPTS

    for {set i 0} {$i < $XPTS} {incr i} {
	x $i = [expr {$xmin + double($i) * ($xmax-$xmin)/ double($XPTS-1)}]
    }

    for {set j 0} {$j < $YPTS} {incr j} {
	y $j = [expr {$ymin + double($j) * ($ymax-$ymin)/ double($YPTS-1)}]
    }

    for {set i 0} {$i < $XPTS} {incr i} {
	for {set j 0} {$j < $YPTS} {incr j} {
	    z $i $j = 0.
	}
    }

    # Page 1: Demonstrate inclination and shear capability pattern.

    $w cmd pladv 0
    $w cmd plvpor -0.15 1.15 -0.05 1.05
    $w cmd plwind -1.2 1.2 -0.8 1.5
    $w cmd plw3d 1.0 1.0 1.0 $xmin $xmax $ymin $ymax $zmin $zmax 20. 45.

    $w cmd plcol0 2
    $w cmd plbox3 "b" "" [expr {$xmax-$xmin}] 0 \
	"b" "" [expr {$ymax-$ymin}] 0 \
	"bcd" "" [expr {$zmax-$zmin}] 0

    # z = zmin.
    $w cmd plschr 0. 1.0
    for  {set i 0} {$i < $NREVOLUTION} {incr i} {
	set omega [expr {2.*$pi*double($i)/double($NREVOLUTION)}]
	set sin_omega [expr {sin($omega)}]
	set cos_omega [expr {cos($omega)}]
	set x_inclination [expr {0.5*$xrange*$cos_omega}]
	set y_inclination [expr {0.5*$yrange*$sin_omega}]
	set z_inclination 0.
	set x_shear [expr {-0.5*$xrange*$sin_omega}]
	set y_shear [expr {0.5*$yrange*$cos_omega}]
	set z_shear 0.
	$w cmd plptex3 \
	    $xmid $ymid $zmin \
	    $x_inclination $y_inclination $z_inclination \
	    $x_shear $y_shear $z_shear \
	    0.0 "  revolution"
    }

    # x = xmax.
    $w cmd plschr 0. 1.0
    for {set i 0} {$i < $NREVOLUTION} {incr i} {
	set omega [expr {2.*$pi*double($i)/double($NREVOLUTION)}]
	set sin_omega [expr {sin($omega)}]
	set cos_omega [expr {cos($omega)}]
	set x_inclination 0.
	set y_inclination [expr {-0.5*$yrange*$cos_omega}]
	set z_inclination [expr {0.5*$zrange*$sin_omega}]
	set x_shear 0.
	set y_shear [expr {0.5*$yrange*$sin_omega}]
	set z_shear [expr {0.5*$zrange*$cos_omega}]
	$w cmd plptex3 \
	    $xmax $ymid $zmid \
	    $x_inclination $y_inclination $z_inclination \
	    $x_shear $y_shear $z_shear \
	    0.0 "  revolution"
    }

    # y = ymax.
    $w cmd plschr 0., 1.0
    for {set i 0} {$i < $NREVOLUTION} {incr i} {
	set omega [expr {2.*$pi*double($i)/double($NREVOLUTION)}]
	set sin_omega [expr {sin($omega)}]
	set cos_omega [expr {cos($omega)}]
	set x_inclination [expr {0.5*$xrange*$cos_omega}]
	set y_inclination 0.
	set z_inclination [expr {0.5*$zrange*$sin_omega}]
	set x_shear [expr {-0.5*$xrange*$sin_omega}]
	set y_shear 0.
	set z_shear [expr {0.5*$zrange*$cos_omega}]
	$w cmd plptex3 \
	    $xmid $ymax $zmid \
	    $x_inclination $y_inclination $z_inclination \
	    $x_shear $y_shear $z_shear \
	    0.0 "  revolution"
    }
    # Draw minimal 3D grid to finish defining the 3D box.
    $w cmd plmesh x y z [expr {$::PLPLOT::DRAW_LINEXY}]

    # Page 2: Demonstrate rotation of string around its axis.
    $w cmd pladv 0
    $w cmd plvpor -0.15 1.15 -0.05 1.05
    $w cmd plwind -1.2 1.2 -0.8 1.5
    $w cmd plw3d 1.0 1.0 1.0 $xmin $xmax $ymin $ymax $zmin $zmax 20. 45.

    $w cmd plcol0 2
    $w cmd plbox3 "b" "" [expr {$xmax-$xmin}] 0 \
	"b" "" [expr {$ymax-$ymin}] 0 \
	"bcd" "" [expr {$zmax-$zmin}] 0

    # y = ymax.
    $w cmd plschr 0. 1.0
    set x_inclination 1.
    set y_inclination 0.
    set z_inclination 0.
    set x_shear 0.
    for {set i 0} {$i < $NROTATION} {incr i} {
	set omega [expr {2.*$pi*double($i)/double($NROTATION)}]
	set sin_omega [expr {sin($omega)}]
	set cos_omega [expr {cos($omega)}]
	set y_shear [expr {0.5*$yrange*$sin_omega}]
	set z_shear [expr {0.5*$zrange*$cos_omega}]
        set zs [expr {$zsmax - $dzsrot*$i}]
	$w cmd plptex3 \
	    $xmid $ymax $zs \
	    $x_inclination $y_inclination $z_inclination \
	    $x_shear $y_shear $z_shear \
	    0.5 "rotation for y = y#dmax#u"
    }

    # x = xmax.
    $w cmd plschr 0. 1.0
    set x_inclination 0.
    set y_inclination -1.
    set z_inclination 0.
    set y_shear 0.
    for {set i 0} {$i < $NROTATION} {incr i} {
	set omega [expr {2.*$pi*double($i)/double($NROTATION)}]
	set sin_omega [expr {sin($omega)}]
	set cos_omega [expr {cos($omega)}]
	set x_shear [expr {0.5*$xrange*$sin_omega}]
	set z_shear [expr {0.5*$zrange*$cos_omega}]
	set zs [expr {$zsmax - $dzsrot*$i}]
    	$w cmd plptex3 \
	    $xmax $ymid $zs \
	    $x_inclination $y_inclination $z_inclination \
	    $x_shear $y_shear $z_shear \
	    0.5 "rotation for x = x#dmax#u"
    }

    # z = zmin.
    $w cmd plschr 0. 1.0
    set x_inclination 1.
    set y_inclination 0.
    set z_inclination 0.
    set x_shear 0.
    for {set i 0} {$i < $NROTATION} {incr i} {
	set omega [expr {2.*$pi*double($i)/double($NROTATION)}]
	set sin_omega [expr {sin($omega)}]
	set cos_omega [expr {cos($omega)}]
	set y_shear [expr {0.5*$yrange*$cos_omega}]
	set z_shear [expr {0.5*$zrange*$sin_omega}]
        set ys [expr {$ysmax - $dysrot*$i}]
	$w cmd plptex3  \
	    $xmid $ys $zmin \
	    $x_inclination $y_inclination $z_inclination \
	    $x_shear $y_shear $z_shear \
	    0.5 "rotation for z = z#dmin#u"
    }
    # Draw minimal 3D grid to finish defining the 3D box.
    $w cmd plmesh x y z [expr {$::PLPLOT::DRAW_LINEXY}]

    # Page 3: Demonstrate shear of string along its axis.
    # Work around xcairo and pngcairo (but not pscairo) problems for
    # shear vector too close to axis of string. (N.B. no workaround
    # would be domega = 0.)
    set domega 0.05
    $w cmd pladv 0
    $w cmd plvpor -0.15 1.15 -0.05 1.05
    $w cmd plwind -1.2 1.2 -0.8 1.5
    $w cmd plw3d 1.0 1.0 1.0 $xmin $xmax $ymin $ymax $zmin $zmax 20. 45.

    $w cmd plcol0 2
    $w cmd plbox3 "b" "" [expr {$xmax-$xmin}] 0 \
	"b" "" [expr {$ymax-$ymin}] 0 \
	"bcd" "" [expr {$zmax-$zmin}] 0

    # y = ymax.
    $w cmd plschr 0. 1.0
    set x_inclination 1.
    set y_inclination 0.
    set z_inclination 0.
    set y_shear 0.
    for {set i 0} {$i < $NSHEAR} {incr i} {
	set omega [expr {$domega + 2.*$pi*double($i)/double($NSHEAR)}]
	set sin_omega [expr {sin($omega)}]
	set cos_omega [expr {cos($omega)}]
	set x_shear [expr {0.5*$xrange*$sin_omega}]
	set z_shear [expr {0.5*$zrange*$cos_omega}]
	set zs [expr {$zsmax - $dzsshear*$i}]
	$w cmd plptex3 \
	    $xmid $ymax $zs \
	    $x_inclination $y_inclination $z_inclination \
	    $x_shear $y_shear $z_shear \
	    0.5 "shear for y = y#dmax#u"
    }

    # x = xmax.
    $w cmd plschr 0. 1.0
    set x_inclination 0.
    set y_inclination -1.
    set z_inclination 0.
    set x_shear 0.
    for {set i 0} {$i < $NSHEAR} {incr i} {
	set omega [expr {$domega + 2.*$pi*double($i)/double($NSHEAR)}]
	set sin_omega [expr {sin($omega)}]
	set cos_omega [expr {cos($omega)}]
	set y_shear [expr {-0.5*$yrange*$sin_omega}]
	set z_shear [expr {0.5*$zrange*$cos_omega}]
	set zs [expr {$zsmax - $dzsshear*$i}]
	$w cmd plptex3 \
	    $xmax $ymid $zs \
	    $x_inclination $y_inclination $z_inclination \
	    $x_shear $y_shear $z_shear \
	    0.5 "shear for x = x#dmax#u"
    }

    # z = zmin.
    $w cmd plschr 0. 1.0
    set x_inclination 1.
    set y_inclination 0.
    set z_inclination 0.
    set z_shear 0.
    for {set i 0} {$i < $NSHEAR} {incr i} {
	set omega [expr {$domega + 2.*$pi*double($i)/double($NSHEAR)}]
	set sin_omega [expr {sin($omega)}]
	set cos_omega [expr {cos($omega)}]
	set y_shear [expr {0.5*$yrange*$cos_omega}]
	set x_shear [expr {0.5*$xrange*$sin_omega}]
	set ys [expr {$ysmax - $dysshear*$i}]
	$w cmd plptex3 \
	    $xmid $ys $zmin \
	    $x_inclination $y_inclination $z_inclination \
	    $x_shear $y_shear $z_shear \
	    0.5 "shear for z = z#dmin#u"
    }
    # Draw minimal 3D grid to finish defining the 3D box.
    $w cmd plmesh x y z [expr {$::PLPLOT::DRAW_LINEXY}]

    # Page 4: Demonstrate drawing a string on a 3D path.
    $w cmd pladv 0
    $w cmd plvpor -0.15 1.15 -0.05 1.05
    $w cmd plwind -1.2 1.2 -0.8 1.5
    $w cmd plw3d 1.0 1.0 1.0 $xmin $xmax $ymin $ymax $zmin $zmax 40. -30.

    $w cmd plcol0 2
    $w cmd plbox3 "b" "" [expr {$xmax-$xmin}] 0 \
	"b" "" [expr {$ymax-$ymin}] 0 \
	"bcd" "" [expr {$zmax-$zmin}] 0

    $w cmd plschr 0. 1.2
    # domega controls the spacing between the various characters of the
    # string and also the maximum value of omega for the given number
    # of characters in *pstring.
    set pslen [string length $pstring]
    set domega [expr {2.*$pi/$pslen}]
    set omega 0.
    # 3D function is a helix of the given radius and pitch
    set radius 0.5
    set pitch [expr {1./(2.*$pi)}]
    for {set i 0} {$i < $pslen} {incr i} {
	set sin_omega [expr {sin($omega)}]
	set cos_omega [expr {cos($omega)}]
	set xpos [expr {$xmid + $radius*$sin_omega}]
	set ypos [expr {$ymid - $radius*$cos_omega}]
	set zpos [expr {$zmin + $pitch*$omega}]
	# In general, the inclination is proportional to the derivative of
	# the position wrt theta.
	set x_inclination [expr {$radius*$cos_omega}]
	set y_inclination [expr {$radius*$sin_omega}]
	set z_inclination [expr {$pitch}]
	# The shear vector should be perpendicular to the 3D line with Z
	# component maximized, but for low pitch a good approximation is
	# a constant vector that is parallel to the Z axis.
	set x_shear 0.
	set y_shear 0.
	set z_shear 1.
	$w cmd plptex3 \
	    $xpos $ypos $zpos \
	    $x_inclination $y_inclination $z_inclination \
	    $x_shear $y_shear $z_shear \
	    0.5 [string range $pstring $i $i]
	set omega [expr {$omega + $domega}]
    }
    # Draw minimal 3D grid to finish defining the 3D box.
    $w cmd plmesh x y z [expr {$::PLPLOT::DRAW_LINEXY}]

    # Page 5: Demonstrate plmtex3 axis labelling capability
    $w cmd pladv 0
    $w cmd plvpor -0.15 1.15 -0.05 1.05
    $w cmd plwind -1.2 1.2 -0.8 1.5
    $w cmd plw3d 1.0 1.0 1.0 $xmin $xmax $ymin $ymax $zmin $zmax 20. 45.

    $w cmd plcol0 2
    $w cmd plbox3 "b" "" [expr {$xmax-$xmin}] 0 \
	"b" "" [expr {$ymax-$ymin}] 0 \
	"bcd" "" [expr {$zmax-$zmin}] 0

    $w cmd plschr 0. 1.0
    $w cmd plmtex3 "xp" 3.0 0.5 0.5 "Arbitrarily displaced"
    $w cmd plmtex3 "xp" 4.5 0.5 0.5 "primary X-axis label"
    $w cmd plmtex3 "xs" -2.5 0.5 0.5 "Arbitrarily displaced"
    $w cmd plmtex3 "xs" -1.0 0.5 0.5 "secondary X-axis label"
    $w cmd plmtex3 "yp" 3.0 0.5 0.5 "Arbitrarily displaced"
    $w cmd plmtex3 "yp" 4.5 0.5 0.5 "primary Y-axis label"
    $w cmd plmtex3 "ys" -2.5 0.5 0.5 "Arbitrarily displaced"
    $w cmd plmtex3 "ys" -1.0 0.5 0.5 "secondary Y-axis label"
    $w cmd plmtex3 "zp" 4.5 0.5 0.5 "Arbitrarily displaced"
    $w cmd plmtex3 "zp" 3.0 0.5 0.5 "primary Z-axis label"
    $w cmd plmtex3 "zs" -2.5 0.5 0.5 "Arbitrarily displaced"
    $w cmd plmtex3 "zs" -1.0 0.5 0.5 "secondary Z-axis label"
    # Draw minimal 3D grid to finish defining the 3D box.
    $w cmd plmesh x y z [expr {$::PLPLOT::DRAW_LINEXY}]

}
