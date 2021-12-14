//      Bar chart demo.
//

import std.string;

import plplot;

//--------------------------------------------------------------------------
// main
//
// Does a simple bar chart, using color fill.  If color fill is
// unavailable, pattern fill is used instead (automatic).
//--------------------------------------------------------------------------
int main( char[][] args )
{
    string text;

    // Parse and process command line arguments
    plparseopts( args, PL_PARSE_FULL );

    // Initialize plplot
    plinit();

    pladv( 0 );
    plvsta();
    plwind( 1980.0, 1990.0, 0.0, 35.0 );
    plbox( "bc", 1.0, 0, "bcnv", 10.0, 0 );
    plcol0( 2 );
    pllab( "Year", "Widget Sales (millions)", "#frPLplot Example 12" );

    PLFLT[] pos   = [ 0.0, 0.25, 0.5, 0.75, 1.0 ];
    PLFLT[] red   = [ 0.0, 0.25, 0.5, 1.0, 1.0 ];
    PLFLT[] green = [ 1.0, 0.5, 0.5, 0.5, 1.0 ];
    PLFLT[] blue  = [ 1.0, 1.0, 0.5, 0.25, 0.0 ];
    plscmap1l( 1, pos, red, green, blue );

    PLFLT[] y0 = [ 5.0, 15.0, 12.0, 24.0, 28.0, 30.0, 20.0, 8.0, 12.0, 3.0 ];
    for ( size_t i = 0; i < 10; i++ )
    {
        plcol1( i / 9.0 );
        plpsty( 0 );
        plfbox( ( 1980. + i ), y0[i] );
        text = format( "%.0f", y0[i] );
        plptex( ( 1980. + i + .5 ), ( y0[i] + 1. ), 1.0, 0.0, .5, text );
        text = format( "%d", 1980 + i );
        plmtex( "b", 1.0, ( ( i + 1 ) * .1 - .05 ), 0.5, text );
    }

    // Don't forget to call plend() to finish off!
    plend();
    return 0;
}


void plfbox( PLFLT x0, PLFLT y0 )
{
    PLFLT[4] x = [x0, x0, x0 + 1.0, x0 + 1.0];;
    PLFLT[4] y = [0.0, y0, y0, 0.0];

    plfill( x, y );
    plcol0( 1 );
    pllsty( 1 );
    plline( x, y );
}
