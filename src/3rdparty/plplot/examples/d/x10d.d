//      Window positioning demo.
//

import plplot;
import std.string;

//--------------------------------------------------------------------------
// main
//
// Demonstrates absolute positioning of graphs on a page.
//--------------------------------------------------------------------------
int main( char[][] args )
{
    // Parse and process command line arguments
    plparseopts( args, PL_PARSE_FULL );

    // Initialize plplot
    plinit();

    pladv( 0 );
    plvpor( 0.0, 1.0, 0.0, 1.0 );
    plwind( 0.0, 1.0, 0.0, 1.0 );
    plbox( "bc", 0.0, 0, "bc", 0.0, 0 );

    plsvpa( 50.0, 150.0, 50.0, 100.0 );
    plwind( 0.0, 1.0, 0.0, 1.0 );
    plbox( "bc", 0.0, 0, "bc", 0.0, 0 );
    plptex( 0.5, 0.5, 1.0, 0.0, 0.5, "BOX at (50,150,50,100)" );

    plend();
    return 0;
}
