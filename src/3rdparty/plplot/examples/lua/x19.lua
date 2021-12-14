--[[
	Illustrates backdrop plotting of world, US maps.
	Contributed by Wesley Ebisuzaki.

  Copyright (C) 2008  Werner Smekal

  This file is part of PLplot.

  PLplot is free software you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as published
  by the Free Software Foundation either version 2 of the License, or
  (at your option) any later version.

  PLplot is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with PLplot if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
--]]

-- initialise Lua bindings for PLplot examples.
dofile("plplot_examples.lua")

function map_transform(x,y)
  radius = 90 - y
  xt = radius * math.cos(x * math.pi / 180)
  yt = radius * math.sin(x * math.pi / 180)

  return xt, yt
end

--------------------------------------------------------------------------
-- mapform19
--
-- Defines specific coordinate transformation for example 19.
-- Not to be confused with mapform in src/plmap.c.
-- x[], y[] are the coordinates to be plotted.
--------------------------------------------------------------------------

function mapform19(n, x, y)
  for i = 1, n do
    x[i], y[i] = map_transform(x[i], y[i])
  end

  return x, y
end

-- "Normalize" longitude values so that they always fall between
-- -180.0 and 180.0
function normalize_longitude(lon)
	if lon>=-180 and lon<=180 then
		return lon
	else
			times = math.floor((math.abs(lon)+180)/360)
			if lon<0 then
				return lon+360*times
			else
				return lon-360*times
			end
	end
end

-- A custom axis labeling function for longitudes and latitudes.
function geolocation_labeler(axis, value)
	if axis==pl.PL_Y_AXIS then
		label_val = value
		if label_val>0 then
			direction_label = " N"
		else
			if label_val<0 then
				direction_label = " S"
			else
				direction_label = "Eq"
			end
		end
	else
		if axis==pl.PL_X_AXIS then
			label_val = normalize_longitude(value)
			if label_val>0 then
				direction_label = " E"
			else
				if label_val<0 then
					direction_label = " W"
				else
					direction_label = ""
				end
			end
		end
	end

	if axis==pl.PL_Y_AXIS and value==0 then
		-- A special case for the equator
		label = direction_label
	else
		label = string.format("%.0f%s", math.abs(label_val), direction_label)
	end

	return label
end

--------------------------------------------------------------------------
-- main
--
-- Shows two views of the world map.
--------------------------------------------------------------------------

-- Parse and process command line arguments

x = {}
y = {}

pl.parseopts(arg, pl.PL_PARSE_FULL)

-- Longitude (x) and latitude (y)

miny = -70
maxy = 80

pl.init()

-- Cartesian plots
-- Most of world

minx = -170
maxx = minx+360

-- Setup a custom latitude and longitude-based scaling function.
pl.slabelfunc("geolocation_labeler")

pl.col0(1)
pl.env(minx, maxx, miny, maxy, 1, 70)
pl.map(nil, "usaglobe", minx, maxx, miny, maxy)

-- The Americas

minx = 190
maxx = 340

pl.col0(1)
pl.env(minx, maxx, miny, maxy, 1, 70)
pl.map(nil, "usaglobe", minx, maxx, miny, maxy)

-- Clear the labeling function
pl.slabelfunc(nil)

-- Polar, Northern hemisphere

minx = 0
maxx = 360

pl.env(-75., 75., -75., 75., 1, -1)
pl.map("mapform19", "globe", minx, maxx, miny, maxy)

pl.lsty(2)
pl.meridians("mapform19", 10, 10, 0, 360, -10, 80)

-- Polar, Northern hemisphere, this time with a PLplot-wide transform

minx = 0
maxx = 360

pl.stransform( "map_transform" )

pl.lsty( 1 )
pl.env( -75., 75., -75., 75., 1, -1 )
-- No need to set the map transform here as the global transform will be
-- used.
pl.map( nil, "globe", minx, maxx, miny, maxy )

pl.lsty( 2 )
pl.meridians( nil, 10.0, 10.0, 0.0, 360.0, -10.0, 80.0 )

-- Show Baltimore, MD on the map
pl.col0( 2 )
pl.ssym( 0.0, 2.0 )
x[1] = -76.6125
y[1] = 39.2902778
pl.poin( x, y, 18 )
pl.ssym( 0.0, 1.0 )
pl.ptex( -76.6125, 43.0, 0.0, 0.0, 0.0, "Baltimore, MD" )

-- For C, this is how the global transform is cleared
pl.stransform( )

-- An example using shapefiles. The shapefiles used are from Ordnance Survey, UK.
-- These were chosen because they provide shapefiles for small grid boxes which
-- are easilly manageable for this demo.

pl.lsty( 1 )

minx = 240570
maxx = 621109
miny = 87822
maxy = 722770
pl.scol0( 0, 255, 255, 255 )
pl.scol0( 1, 0, 0, 0 )
pl.scol0( 2, 150, 150, 150 )
pl.scol0( 3, 0, 50, 200 )
pl.scol0( 4, 50, 50, 50 )
pl.scol0( 5, 150, 0, 0 )
pl.scol0( 6, 100, 100, 255 )

minx = 265000
maxx = 270000
miny = 145000
maxy = 150000
pl.scol0( 0, 255, 255, 255 )  --white
pl.scol0( 1, 0, 0, 0 )        --black
pl.scol0( 2, 255, 200, 0 )    --yelow for sand
pl.scol0( 3, 60, 230, 60 )    -- green for woodland
pl.scol0( 4, 210, 120, 60 )   --brown for contours
pl.scol0( 5, 150, 0, 0 )      --red for major roads
pl.scol0( 6, 180, 180, 255 )  --pale blue for water
pl.scol0( 7, 100, 100, 100 )  --pale grey for shingle or boulders
pl.scol0( 8, 100, 100, 100 )  --dark grey for custom polygons - generally crags

pl.col0( 1 )
pl.env( minx, maxx, miny, maxy, 1, -1 )
pl.lab( "", "", "Martinhoe CP, Exmoor National Park, UK (shapelib only)" )

--Beach
pl.col0( 2 )
beachareas = { 23, 24 }
pl.mapfill( nil, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, beachareas)

--woodland
pl.col0( 3 )
nwoodlandareas = 94
woodlandareas = {}
for i=1,nwoodlandareas do
   woodlandareas[i] = i + 217
end

pl.mapfill( nil, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, woodlandareas )

--shingle or boulders
pl.col0( 7 )
shingleareas = { 0, 1, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 217, 2424, 2425, 2426, 2427, 2428, 2491, 2577 }
pl.mapfill( nil, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, shingleareas )

--crags
pl.col0( 8 )
ncragareas = 2024
cragareas = {}
for i=1,ncragareas do
   cragareas[i] = i + 324
end
pl.mapfill( nil, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, cragareas )

--draw contours, we need to separate contours from high/low coastline
--draw_contours(pls, "ss/SS64_line", 433, 20, 4, 3, minx, maxx, miny, maxy )
pl.col0( 4 )
pl.mapline( nil, "ss/ss64ne_Height_Contours", minx, maxx, miny, maxy, nil )

--draw the sea and surface water
pl.width( 0.0 )
pl.col0( 6 )
pl.mapfill( nil, "ss/ss64ne_Water_Area", minx, maxx, miny, maxy, nil )
pl.width( 2.0 )
pl.mapline( nil, "ss/ss64ne_Water_Line", minx, maxx, miny, maxy, nil )

--draw the roads, first with black and then thinner with colour to give an
--an outlined appearance
pl.width( 5.0 )
pl.col0( 1 )
pl.mapline( nil, "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, nil )
pl.width( 3.0 )
pl.col0( 0 )
pl.mapline( nil, "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, nil )
pl.col0( 5 )
majorroads = { 33, 48, 71, 83, 89, 90, 101, 102, 111 }
pl.mapline( nil, "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, majorroads )

--draw buildings
pl.width( 1.0 )
pl.col0( 1 )
pl.mapfill( nil, "ss/ss64ne_Building_Area", minx, maxx, miny, maxy, nil )

--labels
pl.sfci( 0x80000100 )
pl.schr( 0, 0.8 )
pl.maptex( nil, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "MARTINHOE CP", minx, maxx, miny, maxy, 202 )
pl.schr( 0, 0.7 )
pl.maptex( nil, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Heale\nDown", minx, maxx, miny, maxy, 13 )
pl.maptex( nil, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "South\nDown", minx, maxx, miny, maxy, 34 )
pl.maptex( nil, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Martinhoe\nCommon", minx, maxx, miny, maxy, 42 )
pl.maptex( nil, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Woody Bay", minx, maxx, miny, maxy, 211 )
pl.schr( 0, 0.6 )
pl.maptex( nil, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Mill Wood", minx, maxx, miny, maxy, 16 )
pl.maptex( nil, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Heale Wood", minx, maxx, miny, maxy, 17 )
pl.maptex( nil, "ss/ss64ne_General_Text", 1.0, 0.0, 1.0, "Bodley", minx, maxx, miny, maxy, 31 )
pl.maptex( nil, "ss/ss64ne_General_Text", 1.0, 0.0, 0.0, "Martinhoe", minx, maxx, miny, maxy, 37 )
pl.maptex( nil, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Woolhanger\nCommon", minx, maxx, miny, maxy, 60 )
pl.maptex( nil, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "West Ilkerton\nCommon", minx, maxx, miny, maxy, 61 )
pl.maptex( nil, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Caffyns\nHeanton\nDown", minx, maxx, miny, maxy, 62 )

pl.plend()
