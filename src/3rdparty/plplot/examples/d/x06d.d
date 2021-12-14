//      Font demo.
//

import std.string;
import plplot;

//--------------------------------------------------------------------------
// main
//
// Displays the entire "plpoin" symbol (font) set.
//--------------------------------------------------------------------------
int main( char[][] args )
{
    int maxfont;

    // Parse and process command line arguments
    plparseopts( args, PL_PARSE_FULL );

    // Initialize plplot
    plinit();
    for ( int kind_font = 0; kind_font < 2; kind_font++ )
    {
        plfontld( kind_font );
        if ( kind_font == 0 )
            maxfont = 1;
        else
            maxfont = 4;

        for ( int font = 0; font < maxfont; font++ )
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
            PLFLT[1] x, y;
            for ( size_t i = 0; i <= 9; i++ )
                plmtex( "b", 1.5, ( 0.1 * i + 0.05 ), 0.5, format( "%d", i ) );

            size_t k = 0;
            for ( size_t i = 0; i <= 12; i++ )
            {
                // Write the digits to the left of the frame
                plmtex( "lv", 1.0, ( 1.0 - ( 2 * i + 1 ) / 26.0 ), 1.0, format( "%d", 10 * i ) );
                for ( size_t j = 0; j <= 9; j++ )
                {
                    x[0] = 0.1 * j + 0.05;
                    y[0] = 1.25 - 0.1 * i;

                    // Display the symbols (plpoin expects that x and y
                    // are arrays so pass pointers)
                    if ( k < 128 )
                        plpoin( x, y, cast(PLINT) k );
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
    return 0;
}
