//      Font demo.
//

#include "plcdemos.h"

//--------------------------------------------------------------------------
// main
//
// Displays the entire "plpoin" symbol (font) set.
//--------------------------------------------------------------------------

int
main( int argc, char *argv[] )
{
    char  text[10];
    int   i, j, k, kind_font, font, maxfont;
    PLFLT x, y;

// Parse and process command line arguments

    (void) plparseopts( &argc, argv, PL_PARSE_FULL );

// Initialize plplot

    plinit();

    for ( kind_font = 0; kind_font < 2; kind_font++ )
    {
        plfontld( kind_font );
        if ( kind_font == 0 )
            maxfont = 1;
        else
            maxfont = 4;

        for ( font = 0; font < maxfont; font++ )
        {
            plfont( font + 1 );

            pladv( 0 );

// Set up viewport and window

            plcol0( 2 );
            plvpor( 0.1, 1.0, 0.1, 0.9 );
            plwind( 0.0, 1.0, 0.0, 1.3 );

// Draw the grid using plbox

            plbox( "bcg", 0.1, 0, "bcg", 0.1, 0 );

// Write the digits below the frame

            plcol0( 15 );
            for ( i = 0; i <= 9; i++ )
            {
                sprintf( text, "%d", i );
                plmtex( "b", 1.5, ( 0.1 * i + 0.05 ), 0.5, text );
            }

            k = 0;
            for ( i = 0; i <= 12; i++ )
            {
                // Write the digits to the left of the frame

                sprintf( text, "%d", 10 * i );
                plmtex( "lv", 1.0, ( 1.0 - ( 2 * i + 1 ) / 26.0 ), 1.0, text );
                for ( j = 0; j <= 9; j++ )
                {
                    x = 0.1 * j + 0.05;
                    y = 1.25 - 0.1 * i;

                    // Display the symbols (plpoin expects that x and y are arrays so
                    // pass pointers)

                    if ( k < 128 )
                        plpoin( 1, &x, &y, k );
                    k = k + 1;
                }
            }

            if ( kind_font == 0 )
                plmtex( "t", 1.5, 0.5, 0.5, "PLplot Example 6 - plpoin symbols (compact)" );
            else
                plmtex( "t", 1.5, 0.5, 0.5, "PLplot Example 6 - plpoin symbols (extended)" );
        }
    }
    plend();
    exit( 0 );
}
