## Copyright (C) 1998, 1999, 2000 Joao Cardoso.
##
## This program is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by the
## Free Software Foundation; either version 2 of the License, or (at your
## option) any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## This file is part of plplot_octave.
## It is based on the corresponding demo function of PLplot.

1;

## Shows two views of the world map.

function ix19c
  ## Parse and process command line arguments

  ## (void) plparseopts( &argc, argv, PL_PARSE_FULL );

  ## Longitude (x) and latitude (y)

  miny = -70;
  maxy = 80;

  plinit();

  ## Cartesian plots
  ## Most of world

  minx = -170;
  maxx = minx + 360;

  ## Setup a custom latitude and longitude-based scaling function.
  plslabelfunc( @geolocation_labeler, [] );

  plcol0( 1 );
  plenv( minx, maxx, miny, maxy, 1, 70 );
  plmap( [], "usaglobe", minx, maxx, miny, maxy );

  ## The Americas

  minx = 190;
  maxx = 340;

  plcol0( 1 );
  plenv( minx, maxx, miny, maxy, 1, 70 );
  plmap( [], "usaglobe", minx, maxx, miny, maxy );

  ## Clear the labeling function
  plslabelfunc( [], [] );

  ## Polar, Northern hemisphere

  minx = 0;
  maxx = 360;

  plenv( -75., 75., -75., 75., 1, -1 );
  plmap( @mapform19, "globe", minx, maxx, miny, maxy );

  pllsty( 2 );
  plmeridians( @mapform19, 10.0, 10.0, 0.0, 360.0, -10.0, 80.0 );

  ## Polar, Northern hemisphere, this time with a PLplot-wide transform

  minx = 0;
  maxx = 360;

  plstransform( @map_transform, [] );

  pllsty( 1 );
  plenv( -75., 75., -75., 75., 1, -1 );
  ## No need to set the map transform here as the global transform will be
  ## used.
  plmap( [], "globe", minx, maxx, miny, maxy );

  pllsty( 2 );
  plmeridians( [], 10.0, 10.0, 0.0, 360.0, -10.0, 80.0 );

  ## Show Baltimore, MD on the map
  plcol0( 2 );
  plssym( 0.0, 2.0 );
  x = -76.6125;
  y = 39.2902778;
  plpoin( x, y, 18 );
  plssym( 0.0, 1.0 );
  plptex( -76.6125, 43.0, 0.0, 0.0, 0.0, "Baltimore, MD" );

  ## For C, this is how the global transform is cleared
  plstransform( [], [] );

  ## An example using shapefiles. The shapefiles used are from Ordnance Survey, UK.
  ## These were chosen because they provide shapefiles for small grid boxes which
  ## are easilly manageable for this demo.

  pllsty( 1 );

  minx = 240570;
  maxx = 621109;
  miny = 87822;
  maxy = 722770;
  plscol0( 0, 255, 255, 255 );
  plscol0( 1, 0, 0, 0 );
  plscol0( 2, 150, 150, 150 );
  plscol0( 3, 0, 50, 200 );
  plscol0( 4, 50, 50, 50 );
  plscol0( 5, 150, 0, 0 );
  plscol0( 6, 100, 100, 255 );

  minx = 265000;
  maxx = 270000;
  miny = 145000;
  maxy = 150000;
  plscol0( 0, 255, 255, 255 ); ##white
  plscol0( 1, 0, 0, 0 );       ##black
  plscol0( 2, 255, 200, 0 );   ##yelow for sand
  plscol0( 3, 60, 230, 60 );   ## green for woodland
  plscol0( 4, 210, 120, 60 );  ##brown for contours
  plscol0( 5, 150, 0, 0 );     ##red for major roads
  plscol0( 6, 180, 180, 255 ); ##pale blue for water
  plscol0( 7, 100, 100, 100 ); ##pale grey for shingle or boulders
  plscol0( 8, 100, 100, 100 ); ##dark grey for custom polygons - generally crags

  plcol0( 1 );
  plenv( minx, maxx, miny, maxy, 1, -1 );
  pllab( "", "", "Martinhoe CP, Exmoor National Park, UK (shapelib only)" );

  ##Beach
  plcol0( 2 );
  beachareas = [ 23, 24 ]';
  plmapfill( [], "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, beachareas );

  ##woodland
  plcol0( 3 );
  nwoodlandareas = 94;
  woodlandareas = (0:nwoodlandareas-1)' + 218;
  plmapfill( [], "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, woodlandareas );

  ##shingle or boulders
  plcol0( 7 );
  shingleareas = [ 0, 1, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 217, 2424, 2425, 2426, 2427, 2428, 2491, 2577 ]';
  plmapfill( [], "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, shingleareas );

  ##crags
  plcol0( 8 );
  ncragareas = 2024;
  cragareas = (0:ncragareas-1)' + 325;
  plmapfill( [], "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, cragareas );

  ##draw contours, we need to separate contours from high/low coastline
  ##draw_contours(pls, "ss/SS64_line", 433, 20, 4, 3, minx, maxx, miny, maxy );
  plcol0( 4 );
  plmapline( [], "ss/ss64ne_Height_Contours", minx, maxx, miny, maxy, [] );

  ##draw the sea and surface water
  plwidth( 0.0 );
  plcol0( 6 );
  plmapfill( [], "ss/ss64ne_Water_Area", minx, maxx, miny, maxy, [] );
  plwidth( 2.0 );
  plmapline( [], "ss/ss64ne_Water_Line", minx, maxx, miny, maxy, [] );

  ##draw the roads, first with black and then thinner with colour to give an
  ##an outlined appearance
  plwidth( 5.0 );
  plcol0( 1 );
  plmapline( [], "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, [] );
  plwidth( 3.0 );
  plcol0( 0 );
  plmapline( [], "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, [] );
  plcol0( 5 );
  majorroads = [ 33, 48, 71, 83, 89, 90, 101, 102, 111 ]';
  plmapline( [], "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, majorroads );

  ##draw buildings
  plwidth( 1.0 );
  plcol0( 1 );
  plmapfill( [], "ss/ss64ne_Building_Area", minx, maxx, miny, maxy, [] );

  ##labels
  plsfci( 0x80000100 );
  plschr( 0, 0.8 );
  plmaptex( [], "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "MARTINHOE CP", minx, maxx, miny, maxy, 202 );
  plschr( 0, 0.7 );
  plmaptex( [], "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Heale\nDown", minx, maxx, miny, maxy, 13 );
  plmaptex( [], "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "South\nDown", minx, maxx, miny, maxy, 34 );
  plmaptex( [], "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Martinhoe\nCommon", minx, maxx, miny, maxy, 42 );
  plmaptex( [], "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Woody Bay", minx, maxx, miny, maxy, 211 );
  plschr( 0, 0.6 );
  plmaptex( [], "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Mill Wood", minx, maxx, miny, maxy, 16 );
  plmaptex( [], "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Heale Wood", minx, maxx, miny, maxy, 17 );
  plmaptex( [], "ss/ss64ne_General_Text", 1.0, 0.0, 1.0, "Bodley", minx, maxx, miny, maxy, 31 );
  plmaptex( [], "ss/ss64ne_General_Text", 1.0, 0.0, 0.0, "Martinhoe", minx, maxx, miny, maxy, 37 );
  plmaptex( [], "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Woolhanger\nCommon", minx, maxx, miny, maxy, 60 );
  plmaptex( [], "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "West Ilkerton\nCommon", minx, maxx, miny, maxy, 61 );
  plmaptex( [], "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Caffyns\nHeanton\nDown", minx, maxx, miny, maxy, 62 );

  plend1();
endfunction

function [xt, yt] = map_transform( x, y, data )
    radius = 90.0 - y;
    xt    = radius .* cos( x * pi / 180.0 );
    yt    = radius .* sin( x * pi / 180.0 );
endfunction

##--------------------------------------------------------------------------
## mapform19
##
## Defines specific coordinate transformation for example 19.
## Not to be confused with mapform in src/plmap.c.
## x[], y[] are the coordinates to be plotted.
##--------------------------------------------------------------------------

function [xp, yp] = mapform19( x, y )
  [xp, yp] = map_transform( x, y, [] );
endfunction

## "Normalize" longitude values so that they always fall between -180.0 and
## 180.0
function norm = normalize_longitude( lon )
  if ( lon >= -180.0 && lon <= 180.0 )
    norm = lon;
  else
    times = floor( ( abs( lon ) + 180.0 ) / 360.0 );
    if ( lon < 0.0 )
      norm = lon + 360.0 * times;
    else
      norm = lon - 360.0 * times;
    endif
  endif
endfunction

## A custom axis labeling function for longitudes and latitudes.
function label = geolocation_labeler( axis, value, data )

  global PL_X_AXIS PL_Y_AXIS
  if ( axis == PL_Y_AXIS )
    label_val = value;
    if ( label_val > 0.0 )
      direction_label = " N";
    elseif ( label_val < 0.0 )
      direction_label = " S";
    else
      direction_label = "Eq";
    endif
  elseif ( axis == PL_X_AXIS )
    label_val = normalize_longitude( value );
    if ( label_val > 0.0 )
      direction_label = " E";
    elseif ( label_val < 0.0 )
      direction_label = " W";
    else
      direction_label = "";
    endif
  endif
  if ( axis == PL_Y_AXIS && value == 0.0 )
    ## A special case for the equator
    label = sprintf( "%s", direction_label );
  else
    label = sprintf( "%.0f%s", abs( label_val ), direction_label );
  endif
endfunction

ix19c
