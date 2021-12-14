//      Histogram demo.
//

import std.math;
import plplot;

//--------------------------------------------------------------------------
// main
//
// Draws a histogram from sample data.
//--------------------------------------------------------------------------
int main( char[][] args )
{
    // Parse and process command line arguments
    plparseopts( args, PL_PARSE_FULL );

    // Initialize plplot
    plinit();

    // Fill up data points
    const int npts  = 2047;
    PLFLT     delta = 2.0 * PI / npts;
    PLFLT[npts] data;
    for ( size_t i = 0; i < npts; i++ )
        data[i] = sin( i * delta );

    plcol0( 1 );
    plhist( data, -1.1, 1.1, 44, PL_HIST_DEFAULT );
    plcol0( 2 );
    pllab( "#frValue", "#frFrequency",
        "#frPLplot Example 5 - Probability function of Oscillator" );

    plend();

    return 0;
}
