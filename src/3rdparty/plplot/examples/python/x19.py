#  Copyright (C) Wesley Ebisuzaki
#  Copyright (C) 2009 Andrew Ross
#  Copyright (C) 2009-2016 Alan W. Irwin
#
#  Illustrates backdrop plotting of world, US maps.
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

# Workaround for inability (due to current limitation of bindings/python implementation)
# to pass the w namespace to geolocation_labeler.
import plplot as workaround_w

from numpy import *

def map_transform( x, y, xt, yt, data ):

    radius = 90.0 - y
    xt[0]    = radius * cos( x * pi / 180.0 )
    yt[0]    = radius * sin( x * pi / 180.0 )

# mapform19
#
# Defines specific coordinate transformation for example 19.
# Not to be confused with mapform in src/plmap.c.
# x[], y[] are the coordinates to be plotted.
def mapform19(n, x, y):
    for i in range(n):
        radius = 90.0 - y[i]
        xp = radius * cos(x[i] * pi / 180.0)
        yp = radius * sin(x[i] * pi / 180.0)
        x[i] = xp
        y[i] = yp
    return [x,y]

## "Normalize" longitude values so that they always fall between -180.0 and
## 180.0
def normalize_longitude(lon):
    if ((lon >= -180.0) and (lon <= 180.0)):
        return lon
    else :
        times = floor ((fabs(lon) + 180.0) / 360.0)
        if (lon < 0.0) :
            return(lon + 360.0 * times)
        else :
            return(lon - 360.0 * times)


## A custom axis labeling function for longitudes and latitudes.
# Workaround for being currently unable (due to limitation
# in bindings/python implementation) to get access to w
# namespace via above data argument.
def geolocation_labeler(axis, value, data):
#def geolocation_labeler(axis, value, w):
    w = workaround_w
    if (axis == w.PL_Y_AXIS) :
        label_val = value
        if (label_val > 0.0) :
            direction_label = " N"
        elif (label_val < 0.0) :
            direction_label = " S"

        else :
            direction_label = "Eq"
    elif (axis == w.PL_X_AXIS) :
        label_val = normalize_longitude(value)
        if (label_val > 0.0) :
            direction_label = " E"
        elif (label_val < 0.0) :
            direction_label = " W"
        else :
            direction_label = ""

    if (value == 0.0 and axis == w.PL_Y_AXIS) :
        # A special case for the equator
        label = direction_label
    else :
        label = str(int(abs(label_val))) + direction_label
    return label

# main
#
# Does a series of 3-d plots for a given data set, with different
# viewing options in each plot.

def main(w):

    # Longitude (x) and latitude (y)

    miny = -70
    maxy = 80

    # Cartesian plots
    # Most of world

    minx = -170
    maxx = minx+360

    # Setup a custom latitude and longitude-based scaling function.
    w.plslabelfunc(geolocation_labeler, None)
    #w.plslabelfunc(geolocation_labeler, w)

    w.plcol0(1)
    w.plenv(minx, maxx, miny, maxy, 1, 70)
    w.plmap(None, "usaglobe", minx, maxx, miny, maxy)

    # The Americas

    minx = 190
    maxx = 340

    w.plcol0(1)
    w.plenv(minx, maxx, miny, maxy, 1, 70)
    w.plmap(None, "usaglobe", minx, maxx, miny, maxy)

    # Clear the labeling function
    w.plslabelfunc(None, None)

    # Polar, Northern hemisphere

    minx = 0
    maxx = 360

    w.plenv(-75., 75., -75., 75., 1, -1)
    w.plmap(mapform19,"globe", minx, maxx, miny, maxy)

    w.pllsty(2)
    w.plmeridians(mapform19,10.0, 10.0, 0.0, 360.0, -10.0, 80.0)

    # Polar, Northern hemisphere, this time with a PLplot-wide transform

    minx = 0
    maxx = 360

    w.plstransform( map_transform, None )

    w.pllsty( 1 )
    w.plenv( -75., 75., -75., 75., 1, -1 )
    # No need to set the map transform here as the global transform will be
    # used.
    w.plmap( None, "globe", minx, maxx, miny, maxy )

    w.pllsty( 2 )
    w.plmeridians( None, 10.0, 10.0, 0.0, 360.0, -10.0, 80.0 )

    # Show Baltimore, MD on the map
    w.plcol0( 2 )
    w.plssym( 0.0, 2.0 )
    x = [ -76.6125 ]
    y = [ 39.2902778 ]
    w.plpoin( x, y, 18 )
    w.plssym( 0.0, 1.0 )
    w.plptex( -76.6125, 43.0, 0.0, 0.0, 0.0, "Baltimore, MD" )

    # For C, this is how the global transform is cleared
    w.plstransform( None, None )

    # An example using shapefiles. The shapefiles used are from Ordnance Survey, UK.
    # These were chosen because they provide shapefiles for small grid boxes which
    # are easilly manageable for this demo.

    w.pllsty( 1 )

    minx = 240570
    maxx = 621109
    miny = 87822
    maxy = 722770
    w.plscol0( 0, 255, 255, 255 )
    w.plscol0( 1, 0, 0, 0 )
    w.plscol0( 2, 150, 150, 150 )
    w.plscol0( 3, 0, 50, 200 )
    w.plscol0( 4, 50, 50, 50 )
    w.plscol0( 5, 150, 0, 0 )
    w.plscol0( 6, 100, 100, 255 )

    minx = 265000
    maxx = 270000
    miny = 145000
    maxy = 150000
    w.plscol0( 0, 255, 255, 255 )  #white
    w.plscol0( 1, 0, 0, 0 )        #black
    w.plscol0( 2, 255, 200, 0 )    #yelow for sand
    w.plscol0( 3, 60, 230, 60 )    #green for woodland
    w.plscol0( 4, 210, 120, 60 )   #brown for contours
    w.plscol0( 5, 150, 0, 0 )      #red for major roads
    w.plscol0( 6, 180, 180, 255 )  #pale blue for water
    w.plscol0( 7, 100, 100, 100 )  #pale grey for shingle or boulders
    w.plscol0( 8, 100, 100, 100 )  #dark grey for custom polygons - generally crags

    w.plcol0( 1 )
    w.plenv( minx, maxx, miny, maxy, 1, -1 )
    w.pllab( "", "", "Martinhoe CP, Exmoor National Park, UK (shapelib only)" )

    #Beach
    w.plcol0( 2 )
    beachareas = array([ 23, 24 ])
    w.plmapfill( None, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, beachareas)

    #woodland
    w.plcol0( 3 )
    nwoodlandareas = 94
    woodlandareas = arange(nwoodlandareas) + 218

    w.plmapfill( None, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, woodlandareas )

    #shingle or boulders
    w.plcol0( 7 )
    shingleareas = array([ 0, 1, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 217, 2424, 2425, 2426, 2427, 2428, 2491, 2577 ])
    w.plmapfill( None, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, shingleareas )

    #crags
    w.plcol0( 8 )
    ncragareas = 2024
    cragareas = arange(ncragareas) + 325
    w.plmapfill( None, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, cragareas )

    #draw contours, we need to separate contours from high/low coastline
    #draw_contours(pls, "ss/SS64_line", 433, 20, 4, 3, minx, maxx, miny, maxy )
    w.plcol0( 4 )
    w.plmapline( None, "ss/ss64ne_Height_Contours", minx, maxx, miny, maxy, None )

    #draw the sea and surface water
    w.plwidth( 0.0 )
    w.plcol0( 6 )
    w.plmapfill( None, "ss/ss64ne_Water_Area", minx, maxx, miny, maxy, None )
    w.plwidth( 2.0 )
    w.plmapline( None, "ss/ss64ne_Water_Line", minx, maxx, miny, maxy, None )

    #draw the roads, first with black and then thinner with colour to give an
    #an outlined appearance
    w.plwidth( 5.0 )
    w.plcol0( 1 )
    w.plmapline( None, "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, None )
    w.plwidth( 3.0 )
    w.plcol0( 0 )
    w.plmapline( None, "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, None )
    w.plcol0( 5 )
    majorroads = array([ 33, 48, 71, 83, 89, 90, 101, 102, 111 ])
    w.plmapline( None, "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, majorroads )

    #draw buildings
    w.plwidth( 1.0 )
    w.plcol0( 1 )
    w.plmapfill( None, "ss/ss64ne_Building_Area", minx, maxx, miny, maxy, None )

    #labels
    w.plsfci( 0x80000100 )
    w.plschr( 0, 0.8 )
    w.plmaptex( None, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "MARTINHOE CP", minx, maxx, miny, maxy, 202 )
    w.plschr( 0, 0.7 )
    w.plmaptex( None, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Heale\nDown", minx, maxx, miny, maxy, 13 )
    w.plmaptex( None, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "South\nDown", minx, maxx, miny, maxy, 34 )
    w.plmaptex( None, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Martinhoe\nCommon", minx, maxx, miny, maxy, 42 )
    w.plmaptex( None, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Woody Bay", minx, maxx, miny, maxy, 211 )
    w.plschr( 0, 0.6 )
    w.plmaptex( None, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Mill Wood", minx, maxx, miny, maxy, 16 )
    w.plmaptex( None, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Heale Wood", minx, maxx, miny, maxy, 17 )
    w.plmaptex( None, "ss/ss64ne_General_Text", 1.0, 0.0, 1.0, "Bodley", minx, maxx, miny, maxy, 31 )
    w.plmaptex( None, "ss/ss64ne_General_Text", 1.0, 0.0, 0.0, "Martinhoe", minx, maxx, miny, maxy, 37 )
    w.plmaptex( None, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Woolhanger\nCommon", minx, maxx, miny, maxy, 60 )
    w.plmaptex( None, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "West Ilkerton\nCommon", minx, maxx, miny, maxy, 61 )
    w.plmaptex( None, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Caffyns\nHeanton\nDown", minx, maxx, miny, maxy, 62 )

    # Restore defaults
    w.plschr( 0.0, 1.0 )
    # cmap0 default color palette.
    w.plspal0("cmap0_default.pal")

    # Must be done independently because otherwise this changes output files
    # and destroys agreement with C examples.
    #w.plcol0(1)
