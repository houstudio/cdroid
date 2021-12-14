//      Pie chart demo.
//

import std.string;
import std.stdio;
import std.math;

import plplot;

string[] text = [ "Maurice",
                  "Geoffrey",
                  "Alan",
                  "Rafael",
                  "Vince" ];

//--------------------------------------------------------------------------
// main
//
// Does a simple pie chart.
//--------------------------------------------------------------------------
int main( char[][] args )
{
    // Parse and process command line arguments
    plparseopts( args, PL_PARSE_FULL );

    // Initialize plplot
    plinit();

    pladv( 0 );

    // Ensure window has aspect ratio of one so circle is
    // plotted as a circle.
    plvasp( 1.0 );
    plwind( 0.0, 10.0, 0.0, 10.0 );
    plcol0( 2 );

    // n.b. all theta quantities scaled by 2*M_PI/500 to be integers to avoid
    // floating point logic problems.
    int     theta0 = 0;
    int     dthet  = 1;
    int     theta1, theta;
    PLFLT   just, dx, dy;
    PLFLT[] per = [ 10.0, 32.0, 12.0, 30.0, 16.0 ];
    PLFLT[] x, y;
    for ( size_t i = 0; i < 5; i++ )
    {
        size_t j = 0;

        // n.b. the theta quantities multiplied by 2*M_PI/500 afterward so
        // in fact per is interpreted as a percentage.
        theta1 = cast(int) ( theta0 + 5.0 * per[i] );
        if ( i == 4 )
            theta1 = 500;

        // set size of data arrays
        x.length = y.length = ( theta1 - theta0 ) / dthet + 2;
        x[j]     = 5.0;
        y[j++]   = 5.0;

        for ( theta = theta0; theta <= theta1; theta += dthet )
        {
            x[j]   = 5.0 + 3.0 * cos( ( 2.0 * PI / 500. ) * theta );
            y[j++] = 5.0 + 3.0 * sin( ( 2.0 * PI / 500. ) * theta );
        }

        plcol0( cast(PLINT) ( i + 1 ) );
        plpsty( ( i + 3 ) % 8 + 1 );
        plfill( x, y );
        plcol0( 1 );
        plline( x, y );
        just = ( 2.0 * PI / 500.0 ) * ( theta0 + theta1 ) / 2.0;
        dx   = 0.25 * cos( just );
        dy   = 0.25 * sin( just );
        if ( ( theta0 + theta1 ) < 250 || ( theta0 + theta1 ) > 750 )
            just = 0.0;
        else
            just = 1.0;

        plptex( x[j / 2] + dx, y[j / 2] + dy, 1.0, 0.0, just, text[i] );
        theta0 = theta - dthet;
    }
    plfont( 2 );
    plschr( 0., 1.3 );
    plptex( 5.0, 9.0, 1.0, 0.0, 0.5, "Percentage of Sales" );

    // Don't forget to call PLEND to finish off!
    plend();
    return 0;
}
