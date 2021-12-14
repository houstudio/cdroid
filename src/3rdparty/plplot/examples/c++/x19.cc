//--------------------------------------------------------------------------
// Copyright (C) 2004  Andrew Ross
// Copyright (C) 2004-2014 Alan W. Irwin
//
// This file is part of PLplot.
//
// PLplot is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; version 2 of the License.
//
// PLplot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with PLplot; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
// Implementation of PLplot example 19 in C++.
//--------------------------------------------------------------------------

#include "plc++demos.h"

#ifdef PL_USE_NAMESPACE
using namespace std;
#endif

class x19 {
public:
    x19( int, char ** );

private:
    // Class data
    plstream *pls;
};

void
map_transform( PLFLT x, PLFLT y, PLFLT *xt, PLFLT *yt, PLPointer /* data */ )
{
    PLFLT radius;

    radius = 90.0 - y;
    *xt    = radius * cos( x * M_PI / 180.0 );
    *yt    = radius * sin( x * M_PI / 180.0 );
}

//--------------------------------------------------------------------------
// mapform19
//
// Defines specific coordinate transformation for example 19.
// Not to be confused with mapform in src/plmap.c.
// x[], y[] are the coordinates to be plotted.
//--------------------------------------------------------------------------

void
mapform19( PLINT n, PLFLT *x, PLFLT *y )
{
    int   i;
    PLFLT xp, yp, radius;
    for ( i = 0; i < n; i++ )
    {
        radius = 90.0 - y[i];
        xp     = radius * cos( x[i] * M_PI / 180.0 );
        yp     = radius * sin( x[i] * M_PI / 180.0 );
        x[i]   = xp;
        y[i]   = yp;
    }
}

// "Normalize" longitude values so that they always fall between -180.0 and
// 180.0
PLFLT
normalize_longitude( PLFLT lon )
{
    PLFLT times;
    if ( lon >= -180.0 && lon <= 180.0 )
    {
        return ( lon );
    }
    else
    {
        times = floor( ( fabs( lon ) + 180.0 ) / 360.0 );
        if ( lon < 0.0 )
        {
            return ( lon + 360.0 * times );
        }
        else
        {
            return ( lon - 360.0 * times );
        }
    }
}

// A custom axis labeling function for longitudes and latitudes.
void
geolocation_labeler( PLINT axis, PLFLT value, char *label, PLINT length, PLPointer /* data */ )
{
    const char *direction_label = "";
    PLFLT      label_val        = 0.0;

    if ( axis == PL_Y_AXIS )
    {
        label_val = value;
        if ( label_val > 0.0 )
        {
            direction_label = " N";
        }
        else if ( label_val < 0.0 )
        {
            direction_label = " S";
        }
        else
        {
            direction_label = "Eq";
        }
    }
    else if ( axis == PL_X_AXIS )
    {
        label_val = normalize_longitude( value );
        if ( label_val > 0.0 )
        {
            direction_label = " E";
        }
        else if ( label_val < 0.0 )
        {
            direction_label = " W";
        }
        else
        {
            direction_label = "";
        }
    }
    if ( axis == PL_Y_AXIS && value == 0.0 )
    {
        // A special case for the equator
        snprintf( label, length, "%s", direction_label );
    }
    else
    {
        snprintf( label, length, "%.0f%s", fabs( label_val ), direction_label );
    }
}

//a function to draw the contour data and coastline from the example shapefiles
void
draw_contours( plstream *pls, const char *file, PLINT ncontours, PLINT nwater, PLINT colourcontours, PLINT colourwater, PLFLT minx, PLFLT maxx, PLFLT miny, PLFLT maxy )
{
    PLINT i;
    PLINT *contours = (PLINT *) malloc( ncontours * sizeof ( PLINT ) );
    PLINT *water    = (PLINT *) malloc( nwater * sizeof ( PLINT ) );

    for ( i = 0; i < ncontours; ++i )
        contours[i] = i;
    for ( i = 0; i < nwater; ++i )
        water[i] = i + ncontours;

    pls->col0( colourcontours );
    pls->mapline( NULL, file, minx, maxx, miny, maxy, contours, ncontours );
    pls->col0( colourwater );
    pls->mapline( NULL, file, minx, maxx, miny, maxy, water, nwater );

    free( contours );
    free( water );
}


x19::x19( int argc, char ** argv )
{
    PLFLT minx, maxx, miny, maxy;
    PLINT i;

    // plplot initialization

    pls = new plstream();

    // Parse and process command line arguments.
    pls->parseopts( &argc, argv, PL_PARSE_FULL );

    // Longitude (x) and latitude (y)

    miny = -70;
    maxy = 80;

    // Initialize PLplot.
    pls->init();
    // Cartesian plots
    // Most of world


    minx = -170;
    maxx = minx + 360;

    // Setup a custom latitude and longitude-based scaling function.
    pls->slabelfunc( geolocation_labeler, NULL );

    pls->col0( 1 );
    pls->env( minx, maxx, miny, maxy, 1, 70 );
    pls->map( NULL, "usaglobe", minx, maxx, miny, maxy );

    // The Americas

    minx = 190;
    maxx = 340;

    pls->col0( 1 );
    pls->env( minx, maxx, miny, maxy, 1, 70 );
    pls->map( NULL, "usaglobe", minx, maxx, miny, maxy );

    // Clear the labeling function
    pls->slabelfunc( NULL, NULL );

    // Polar, Northern hemisphere
    minx = 0;
    maxx = 360;

    pls->env( -75., 75., -75., 75., 1, -1 );
    pls->map( mapform19, "globe.shp", minx, maxx, miny, maxy );

    pls->lsty( 2 );
    pls->meridians( mapform19, 10.0, 10.0, 0.0, 360.0, -10.0, 80.0 );

    // Polar, Northern hemisphere, this time with a PLplot-wide transform

    minx = 0;
    maxx = 360;

    pls->stransform( map_transform, NULL );

    pls->lsty( 1 );
    pls->env( -75., 75., -75., 75., 1, -1 );
    // No need to set the map transform here as the global transform will be
    // used.
    pls->map( NULL, "globe", minx, maxx, miny, maxy );

    pls->lsty( 2 );
    pls->meridians( NULL, 10.0, 10.0, 0.0, 360.0, -10.0, 80.0 );

    // Show Baltimore, MD on the map
    pls->col0( 2 );
    pls->ssym( 0.0, 2.0 );
    PLFLT x = -76.6125;
    PLFLT y = 39.2902778;
    pls->poin( 1, &x, &y, 18 );
    pls->ssym( 0.0, 1.0 );
    pls->ptex( -76.6125, 43.0, 0.0, 0.0, 0.0, "Baltimore, MD" );

    // For C, this is how the global transform is cleared
    pls->stransform( NULL, NULL );

    // An example using shapefiles. The shapefiles used are from Ordnance Survey, UK.
    // These were chosen because they provide shapefiles for small grid boxes which
    // are easilly manageable for this demo.

    pls->lsty( 1 );


    minx = 240570;
    maxx = 621109;
    miny = 87822;
    maxy = 722770;
    pls->scol0( 0, 255, 255, 255 );
    pls->scol0( 1, 0, 0, 0 );
    pls->scol0( 2, 150, 150, 150 );
    pls->scol0( 3, 0, 50, 200 );
    pls->scol0( 4, 50, 50, 50 );
    pls->scol0( 5, 150, 0, 0 );
    pls->scol0( 6, 100, 100, 255 );


    minx = 265000;
    maxx = 270000;
    miny = 145000;
    maxy = 150000;
    pls->scol0( 0, 255, 255, 255 ); //white
    pls->scol0( 1, 0, 0, 0 );       //black
    pls->scol0( 2, 255, 200, 0 );   //yelow for sand
    pls->scol0( 3, 60, 230, 60 );   // green for woodland
    pls->scol0( 4, 210, 120, 60 );  //brown for contours
    pls->scol0( 5, 150, 0, 0 );     //red for major roads
    pls->scol0( 6, 180, 180, 255 ); //pale blue for water
    pls->scol0( 7, 100, 100, 100 ); //pale grey for shingle or boulders
    pls->scol0( 8, 100, 100, 100 ); //dark grey for custom polygons - generally crags


    pls->col0( 1 );
    pls->env( minx, maxx, miny, maxy, 1, -1 );
    pls->lab( "", "", "Martinhoe CP, Exmoor National Park, UK (shapelib only)" );


    //Beach
    pls->col0( 2 );
    const PLINT nbeachareas  = 2;
    const PLINT beachareas[] = { 23, 24 };
    pls->mapfill( NULL, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, beachareas, nbeachareas );

    //woodland
    pls->col0( 3 );
    const PLINT nwoodlandareas = 94;
    PLINT       woodlandareas[nwoodlandareas];
    for ( i = 0; i < nwoodlandareas; ++i )
        woodlandareas[i] = i + 218;
    pls->mapfill( NULL, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, (const PLINT *) woodlandareas, nwoodlandareas );

    //shingle or boulders
    pls->col0( 7 );
    const PLINT nshingleareas  = 22;
    const PLINT shingleareas[] = { 0, 1, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 217, 2424, 2425, 2426, 2427, 2428, 2491, 2577 };
    pls->mapfill( NULL, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, shingleareas, nshingleareas );

    //crags
    pls->col0( 8 );
    const PLINT ncragareas = 2024;
    PLINT       cragareas[ncragareas];
    for ( i = 0; i < ncragareas; ++i )
        cragareas[i] = i + 325;
    pls->mapfill( NULL, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, (const PLINT *) cragareas, ncragareas );

    //draw contours, we need to separate contours from high/low coastline
    //draw_contours(pls, "ss/SS64_line", 433, 20, 4, 3, minx, maxx, miny, maxy );
    pls->col0( 4 );
    pls->mapline( NULL, "ss/ss64ne_Height_Contours", minx, maxx, miny, maxy, NULL, 0 );

    //draw the sea and surface water
    pls->width( 0.0 );
    pls->col0( 6 );
    pls->mapfill( NULL, "ss/ss64ne_Water_Area", minx, maxx, miny, maxy, NULL, 0 );
    pls->width( 2.0 );
    pls->mapline( NULL, "ss/ss64ne_Water_Line", minx, maxx, miny, maxy, NULL, 0 );

    //draw the roads, first with black and then thinner with colour to give an
    //an outlined appearance
    pls->width( 5.0 );
    pls->col0( 1 );
    pls->mapline( NULL, "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, NULL, 0 );
    pls->width( 3.0 );
    pls->col0( 0 );
    pls->mapline( NULL, "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, NULL, 0 );
    const PLINT majorroads[] = { 33, 48, 71, 83, 89, 90, 101, 102, 111 };
    pls->col0( 5 );
    pls->mapline( NULL, "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, majorroads, 9 );

    //draw buildings
    pls->width( 1.0 );
    pls->col0( 1 );
    pls->mapfill( NULL, "ss/ss64ne_Building_Area", minx, maxx, miny, maxy, NULL, 0 );

    //labels
    pls->sfci( 0x80000100 );
    pls->schr( 0, 0.8 );
    pls->maptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "MARTINHOE CP", minx, maxx, miny, maxy, 202 );
    pls->schr( 0, 0.7 );
    pls->maptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Heale\nDown", minx, maxx, miny, maxy, 13 );
    pls->maptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "South\nDown", minx, maxx, miny, maxy, 34 );
    pls->maptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Martinhoe\nCommon", minx, maxx, miny, maxy, 42 );
    pls->maptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Woody Bay", minx, maxx, miny, maxy, 211 );
    pls->schr( 0, 0.6 );
    pls->maptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Mill Wood", minx, maxx, miny, maxy, 16 );
    pls->maptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Heale Wood", minx, maxx, miny, maxy, 17 );
    pls->maptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 1.0, "Bodley", minx, maxx, miny, maxy, 31 );
    pls->maptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.0, "Martinhoe", minx, maxx, miny, maxy, 37 );
    pls->maptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Woolhanger\nCommon", minx, maxx, miny, maxy, 60 );
    pls->maptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "West Ilkerton\nCommon", minx, maxx, miny, maxy, 61 );
    pls->maptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Caffyns\nHeanton\nDown", minx, maxx, miny, maxy, 62 );

    delete pls;
}

int main( int argc, char ** argv )
{
    x19 *x = new x19( argc, argv );

    delete x;
}


//--------------------------------------------------------------------------
//                              End of x19.cc
//--------------------------------------------------------------------------
