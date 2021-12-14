#      Copyright (C) 2002-2014  Alan W. Irwin
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
#
#      Illustrates backdrop plotting of world, US maps.
#      Contributed by Wesley Ebisuzaki.
#
#

#
#   The coordinate transformation works directly on the two matrices
#   containing the x and y coordinates
#

proc map_transform { x y } {

    set scale [expr acos(-1.) / 180.]

    set radius [expr {90. - $y}]

    return [list \
                [expr {$radius * cos( $x * $scale )}] \
                [expr {$radius * sin( $x * $scale )}]]
}

proc mapform19 {n matx maty} {

    set deg_to_rad [expr {$::PLPLOT::PL_PI/180.0}]

    for {set i 0} {$i < $n} {incr i} {
        set x      [$matx $i]
        set y      [$maty $i]
        set radius [expr {90.0 - $y}]
        set xp     [expr {$radius * cos($x * $deg_to_rad)}]
        set yp     [expr {$radius * sin($x * $deg_to_rad)}]

        $matx $i = $xp
        $maty $i = $yp
    }
}

# "Normalize" longitude values so that they always fall between -180.0 and
# 180.0
proc normalize_longitude {lon} {
    if {$lon >= -180.0 && $lon <= 180.0} {
        return $lon
    } else {
        set times [expr {floor ((abs($lon) + 180.0) / 360.0)}]
        if {$lon < 0.0} {
            return [expr {($lon + 360.0 * $times)}]
        } else {
            return [expr {($lon - 360.0 * $times)}]
        }
    }

}

# A custom axis labeling function for longitudes and latitudes.
proc geolocation_labeler {axis value} {

    if {$axis == 2} {
        set label_val $value
        if {$label_val > 0.0} {
            set direction_label " N"
        } elseif {$label_val < 0.0} {
            set direction_label " S"
        } else {
            set direction_label "Eq"
        }
    } elseif {$axis == 1} {
        set label_val [normalize_longitude $value]
        if {$label_val > 0.0} {
            set direction_label " E"
        } elseif {$label_val < 0.0} {
            set direction_label " W"
        } else {
            set direction_label ""
        }
    }
    if {$axis == 2 && $value == 0.0} {
        # A special case for the equator
        set label $direction_label
    } else {
        set label [ format "%.0f%s" [expr {abs($label_val)}] $direction_label]
    }

    return $label
}

proc x19 {{w loopback}} {
    set miny -70
    set maxy 80

#   Cartesian plots
#   Most of the world

    set minx -170
    set maxx 190

    # Setup a custom latitude and longitude-based scaling function.
    $w cmd plslabelfunc "geolocation_labeler"

    $w cmd plcol0 1
    $w cmd plenv $minx $maxx $miny $maxy 1 70
    $w cmd plmap usaglobe $minx $maxx $miny $maxy

#   The Americas

    set minx 190
    set maxx 340

    $w cmd plcol0 1
    $w cmd plenv $minx $maxx $miny $maxy 1 70
    $w cmd plmap usaglobe $minx $maxx $miny $maxy

    # Clear the labeling function
    $w cmd plslabelfunc ""

#   Polar, Northern hemisphere
#   Note: the first argument now is the name of the procedure
#   that transforms the coordinates (plmap and plmeridians)

    set minx 0
    set maxx 360

    $w cmd plenv -75 75 -75 75 1 -1
    $w cmd plmap mapform19 globe $minx $maxx $miny $maxy

    $w cmd pllsty 2
    $w cmd plmeridians mapform19 10.0 10.0 0.0 360.0 -10.0 80.0

# Polar, Northern hemisphere
# This time we use a global coordinate transformation, so no coordinate
# transform function is required on the plmap/plmeridians calls.

    set minx 0
    set maxx 360

    $w cmd plstransform map_transform

    $w cmd pllsty 1
    $w cmd plenv -75 75 -75 75 1 -1

    $w cmd plmap globe $minx $maxx $miny $maxy

    $w cmd pllsty 2
    $w cmd plmeridians 10.0 10.0 0.0 360.0 -10.0 80.0

# Show Baltimore, MD on the map.
    $w cmd plcol0 2
    $w cmd plssym 0. 2.

# This is kind of a messy way to use plpoin for plotting a single symbol.
# But it's what we have for now.
    matrix x f 1
    matrix y f 1
    x 0 = -76.6125
    y 0 = 39.2902778

    $w cmd plpoin x y 18

    $w cmd plssym 0. 1.
    $w cmd plptex -76.6125 43. 0.0 0.0 0.0 "Baltimore, MD"

    $w cmd plstransform NULL


    # An example using shapefiles. The shapefiles used are from Ordnance Survey, UK.
    # These were chosen because they provide shapefiles for small grid boxes which
    # are easilly manageable for this demo.

    set beachareas     {23 24}
    set shingleareas   {0 1 24 25 26 27 28 29 30 31 32 33 34 35 217 2424 2425 2426 2427 2428 2491 2577}
    set majorroads     {33 48 71 83 89 90 101 102 111}
    set nwoodlandareas 94
    set ncragareas     2024

    $w cmd pllsty 1

    set minx 240570
    set maxx 621109
    set miny 87822
    set maxy 722770
    $w cmd plscol0 0 255 255 255
    $w cmd plscol0 1 0 0 0
    $w cmd plscol0 2 150 150 150
    $w cmd plscol0 3 0 50 200
    $w cmd plscol0 4 50 50 50
    $w cmd plscol0 5 150 0 0
    $w cmd plscol0 6 100 100 255

    set minx 265000
    set maxx 270000
    set miny 145000
    set maxy 150000
    $w cmd plscol0 0 255 255 255  ;#white
    $w cmd plscol0 1 0 0 0        ;#black
    $w cmd plscol0 2 255 200 0    ;#yelow for sand
    $w cmd plscol0 3 60 230 60    ;# green for woodland
    $w cmd plscol0 4 210 120 60   ;#brown for contours
    $w cmd plscol0 5 150 0 0      ;#red for major roads
    $w cmd plscol0 6 180 180 255  ;#pale blue for water
    $w cmd plscol0 7 100 100 100  ;#pale grey for shingle or boulders
    $w cmd plscol0 8 100 100 100  ;#dark grey for custom polygons - generally crags


    $w cmd plcol0 1
    $w cmd plenv $minx $maxx $miny $maxy 1 -1
    $w cmd pllab "" "" "Martinhoe CP, Exmoor National Park, UK (shapelib only)"

    #Beach
    $w cmd plcol0 2
    $w cmd plmapfill {} "ss/ss64ne_Landform_Area" $minx $maxx $miny $maxy $beachareas

    #woodland
    $w cmd plcol0 3
    for {set i 0} {$i < $nwoodlandareas} {incr i} {
        lappend woodlandareas [expr {$i + 218}]
    }
    $w cmd plmapfill {} "ss/ss64ne_Landform_Area" $minx $maxx $miny $maxy $woodlandareas

    #shingle or boulders
    $w cmd plcol0 7
    $w cmd plmapfill {} "ss/ss64ne_Landform_Area" $minx $maxx $miny $maxy $shingleareas

    #crags
    $w cmd plcol0 8
    for { set i 0} {$i < $ncragareas} {incr i} {
        lappend cragareas [expr {$i + 325}]
    }
    $w cmd plmapfill {} "ss/ss64ne_Landform_Area" $minx $maxx $miny $maxy $cragareas

    # draw contours, we need to separate contours from high/low coastline
    $w cmd plcol0 4
    $w cmd plmapline {} "ss/ss64ne_Height_Contours" $minx $maxx $miny $maxy

    #draw the sea and surface water
    $w cmd plwidth 0.0
    $w cmd plcol0 6
    $w cmd plmapfill {} "ss/ss64ne_Water_Area" $minx $maxx $miny $maxy
    $w cmd plwidth 2.0
    $w cmd plmapline {} "ss/ss64ne_Water_Line" $minx $maxx $miny $maxy

    #draw the roads, first with black and then thinner with colour to give an
    #an outlined appearance
    $w cmd plwidth 5.0
    $w cmd plcol0 1
    $w cmd plmapline {} "ss/ss64ne_Road_Centreline" $minx $maxx $miny $maxy
    $w cmd plwidth 3.0
    $w cmd plcol0 0
    $w cmd plmapline {} "ss/ss64ne_Road_Centreline" $minx $maxx $miny $maxy
    $w cmd plcol0 5
    $w cmd plmapline {} "ss/ss64ne_Road_Centreline" $minx $maxx $miny $maxy $majorroads

    #draw buildings
    $w cmd plwidth 1.0
    $w cmd plcol0 1
    $w cmd plmapfill {} "ss/ss64ne_Building_Area" $minx $maxx $miny $maxy

    #labels
    $w cmd plsfci [expr {0x80000100}]
    $w cmd plschr 0 0.8
    $w cmd plmaptex {} "ss/ss64ne_General_Text" 1.0 0.0 0.5 "MARTINHOE CP" $minx $maxx $miny $maxy 202
    $w cmd plschr 0 0.7
    $w cmd plmaptex {} "ss/ss64ne_General_Text" 1.0 0.0 0.5 "Heale\nDown" $minx $maxx $miny $maxy 13
    $w cmd plmaptex {} "ss/ss64ne_General_Text" 1.0 0.0 0.5 "South\nDown" $minx $maxx $miny $maxy 34
    $w cmd plmaptex {} "ss/ss64ne_General_Text" 1.0 0.0 0.5 "Martinhoe\nCommon" $minx $maxx $miny $maxy 42
    $w cmd plmaptex {} "ss/ss64ne_General_Text" 1.0 0.0 0.5 "Woody Bay" $minx $maxx $miny $maxy 211
    $w cmd plschr 0 0.6
    $w cmd plmaptex {} "ss/ss64ne_General_Text" 1.0 0.0 0.5 "Mill Wood" $minx $maxx $miny $maxy 16
    $w cmd plmaptex {} "ss/ss64ne_General_Text" 1.0 0.0 0.5 "Heale Wood" $minx $maxx $miny $maxy 17
    $w cmd plmaptex {} "ss/ss64ne_General_Text" 1.0 0.0 1.0 "Bodley" $minx $maxx $miny $maxy 31
    $w cmd plmaptex {} "ss/ss64ne_General_Text" 1.0 0.0 0.0 "Martinhoe" $minx $maxx $miny $maxy 37
    $w cmd plmaptex {} "ss/ss64ne_General_Text" 1.0 0.0 0.5 "Woolhanger\nCommon" $minx $maxx $miny $maxy 60
    $w cmd plmaptex {} "ss/ss64ne_General_Text" 1.0 0.0 0.5 "West Ilkerton\nCommon" $minx $maxx $miny $maxy 61
    $w cmd plmaptex {} "ss/ss64ne_General_Text" 1.0 0.0 0.5 "Caffyns\nHeanton\nDown" $minx $maxx $miny $maxy 62
}
