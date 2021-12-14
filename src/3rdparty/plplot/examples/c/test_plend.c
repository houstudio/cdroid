//      plend and plend1 testing demo.
//

#include "plcdemos.h"

//--------------------------------------------------------------------------
// main
//
// Demonstrates absolute positioning of graphs on a page.
//--------------------------------------------------------------------------

int
main( int argc, char *argv[] )
{
// Parse and process command line arguments

    (void) plparseopts( &argc, argv, PL_PARSE_FULL );

// Initialize plplot

    plinit();
    plenv( 0., 1., 0., 1., 1, 0 );
    plend();
    plinit();
    plenv( 0., 10., 0., 10., 1, 0 );
    plend();
    exit( 0 );
}
