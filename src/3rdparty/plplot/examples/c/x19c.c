//      Illustrates backdrop plotting of world, US maps.
//      Contributed by Wesley Ebisuzaki.
//

#include "plcdemos.h"

void map_transform( PLFLT x, PLFLT y, PLFLT *xt, PLFLT *yt, PLPointer data );
void mapform19( PLINT n, PLFLT *x, PLFLT *y );
PLFLT normalize_longitude( PLFLT lon );
void geolocation_labeler( PLINT axis, PLFLT value, char *label, PLINT length, PLPointer data );

void
map_transform( PLFLT x, PLFLT y, PLFLT *xt, PLFLT *yt, PLPointer PL_UNUSED( data ) )
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
    PLFLT xp, yp;
    for ( i = 0; i < n; i++ )
    {
        map_transform( x[i], y[i], &xp, &yp, NULL );
        x[i] = xp;
        y[i] = yp;
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
geolocation_labeler( PLINT axis, PLFLT value, char *label, PLINT length, PLPointer PL_UNUSED( data ) )
{
    PLCHAR_VECTOR direction_label = NULL;
    PLFLT         label_val       = 0.0;

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
        snprintf( label, (size_t) length, "%s", direction_label );
    }
    else
    {
        snprintf( label, (size_t) length, "%.0f%s", fabs( label_val ), direction_label );
    }
}

//--------------------------------------------------------------------------
// main
//
// Shows two views of the world map.
//--------------------------------------------------------------------------

int
main( int argc, char **argv )
{
    PLFLT       minx, maxx, miny, maxy;
    PLFLT       x, y;
    //variables for the shapelib example
    const PLINT nbeachareas    = 2;
    const PLINT beachareas[]   = { 23, 24 };
    const PLINT nwoodlandareas = 94;
    PLINT       woodlandareas[94];
    const PLINT nshingleareas  = 22;
    const PLINT shingleareas[] = { 0, 1, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 217, 2424, 2425, 2426, 2427, 2428, 2491, 2577 };
    const PLINT ncragareas     = 2024;
    PLINT       cragareas[2024];
    const PLINT majorroads[] = { 33, 48, 71, 83, 89, 90, 101, 102, 111 };
    int         i;

// Parse and process command line arguments

    (void) plparseopts( &argc, argv, PL_PARSE_FULL );

// Longitude (x) and latitude (y)

    miny = -70;
    maxy = 80;

    plinit();

// Cartesian plots
// Most of world

    minx = -170;
    maxx = minx + 360;

    // Setup a custom latitude and longitude-based scaling function.
    plslabelfunc( geolocation_labeler, NULL );

    plcol0( 1 );
    plenv( minx, maxx, miny, maxy, 1, 70 );
    plmap( NULL, "usaglobe", minx, maxx, miny, maxy );

// The Americas

    minx = 190;
    maxx = 340;

    plcol0( 1 );
    plenv( minx, maxx, miny, maxy, 1, 70 );
    plmap( NULL, "usaglobe", minx, maxx, miny, maxy );

    // Clear the labeling function
    plslabelfunc( NULL, NULL );

// Polar, Northern hemisphere

    minx = 0;
    maxx = 360;

    plenv( -75., 75., -75., 75., 1, -1 );
    plmap( mapform19, "globe", minx, maxx, miny, maxy );

    pllsty( 2 );
    plmeridians( mapform19, 10.0, 10.0, 0.0, 360.0, -10.0, 80.0 );

// Polar, Northern hemisphere, this time with a PLplot-wide transform

    minx = 0;
    maxx = 360;

    plstransform( map_transform, NULL );

    pllsty( 1 );
    plenv( -75., 75., -75., 75., 1, -1 );
    // No need to set the map transform here as the global transform will be
    // used.
    plmap( NULL, "globe", minx, maxx, miny, maxy );

    pllsty( 2 );
    plmeridians( NULL, 10.0, 10.0, 0.0, 360.0, -10.0, 80.0 );

    // Show Baltimore, MD on the map
    plcol0( 2 );
    plssym( 0.0, 2.0 );
    x = -76.6125;
    y = 39.2902778;
    plpoin( 1, &x, &y, 18 );
    plssym( 0.0, 1.0 );
    plptex( -76.6125, 43.0, 0.0, 0.0, 0.0, "Baltimore, MD" );

    // For C, this is how the global transform is cleared
    plstransform( NULL, NULL );

    // An example using shapefiles. The shapefiles used are from Ordnance Survey, UK.
    // These were chosen because they provide shapefiles for small grid boxes which
    // are easilly manageable for this demo.

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
    plscol0( 0, 255, 255, 255 ); //white
    plscol0( 1, 0, 0, 0 );       //black
    plscol0( 2, 255, 200, 0 );   //yelow for sand
    plscol0( 3, 60, 230, 60 );   // green for woodland
    plscol0( 4, 210, 120, 60 );  //brown for contours
    plscol0( 5, 150, 0, 0 );     //red for major roads
    plscol0( 6, 180, 180, 255 ); //pale blue for water
    plscol0( 7, 100, 100, 100 ); //pale grey for shingle or boulders
    plscol0( 8, 100, 100, 100 ); //dark grey for custom polygons - generally crags


    plcol0( 1 );
    plenv( minx, maxx, miny, maxy, 1, -1 );
    pllab( "", "", "Martinhoe CP, Exmoor National Park, UK (shapelib only)" );


    //Beach
    plcol0( 2 );
    plmapfill( NULL, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, beachareas, nbeachareas );

    //woodland
    plcol0( 3 );
    for ( i = 0; i < nwoodlandareas; ++i )
        woodlandareas[i] = i + 218;
    plmapfill( NULL, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, (PLINT_VECTOR) woodlandareas, nwoodlandareas );

    //shingle or boulders
    plcol0( 7 );
    plmapfill( NULL, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, shingleareas, nshingleareas );

    //crags
    plcol0( 8 );
    for ( i = 0; i < ncragareas; ++i )
        cragareas[i] = i + 325;
    plmapfill( NULL, "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, (PLINT_VECTOR) cragareas, ncragareas );

    //draw contours, we need to separate contours from high/low coastline
    //draw_contours(pls, "ss/SS64_line", 433, 20, 4, 3, minx, maxx, miny, maxy );
    plcol0( 4 );
    plmapline( NULL, "ss/ss64ne_Height_Contours", minx, maxx, miny, maxy, NULL, 0 );

    //draw the sea and surface water
    plwidth( 0.0 );
    plcol0( 6 );
    plmapfill( NULL, "ss/ss64ne_Water_Area", minx, maxx, miny, maxy, NULL, 0 );
    plwidth( 2.0 );
    plmapline( NULL, "ss/ss64ne_Water_Line", minx, maxx, miny, maxy, NULL, 0 );

    //draw the roads, first with black and then thinner with colour to give an
    //an outlined appearance
    plwidth( 5.0 );
    plcol0( 1 );
    plmapline( NULL, "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, NULL, 0 );
    plwidth( 3.0 );
    plcol0( 0 );
    plmapline( NULL, "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, NULL, 0 );
    plcol0( 5 );
    plmapline( NULL, "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, majorroads, 9 );

    //draw buildings
    plwidth( 1.0 );
    plcol0( 1 );
    plmapfill( NULL, "ss/ss64ne_Building_Area", minx, maxx, miny, maxy, NULL, 0 );

    //labels
    plsfci( 0x80000100 );
    plschr( 0, 0.8 );
    plmaptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "MARTINHOE CP", minx, maxx, miny, maxy, 202 );
    plschr( 0, 0.7 );
    plmaptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Heale\nDown", minx, maxx, miny, maxy, 13 );
    plmaptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "South\nDown", minx, maxx, miny, maxy, 34 );
    plmaptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Martinhoe\nCommon", minx, maxx, miny, maxy, 42 );
    plmaptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Woody Bay", minx, maxx, miny, maxy, 211 );
    plschr( 0, 0.6 );
    plmaptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Mill Wood", minx, maxx, miny, maxy, 16 );
    plmaptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Heale Wood", minx, maxx, miny, maxy, 17 );
    plmaptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 1.0, "Bodley", minx, maxx, miny, maxy, 31 );
    plmaptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.0, "Martinhoe", minx, maxx, miny, maxy, 37 );
    plmaptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Woolhanger\nCommon", minx, maxx, miny, maxy, 60 );
    plmaptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "West Ilkerton\nCommon", minx, maxx, miny, maxy, 61 );
    plmaptex( NULL, "ss/ss64ne_General_Text", 1.0, 0.0, 0.5, "Caffyns\nHeanton\nDown", minx, maxx, miny, maxy, 62 );

    plend();
    exit( 0 );
}
