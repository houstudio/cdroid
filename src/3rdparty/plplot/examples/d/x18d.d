//      3-d line and point plot demo.  Adapted from x08c.c.
//

import std.math;
import std.string;

import plplot;

int opt[]   = [ 1, 0, 1, 0 ];
PLFLT alt[] = [ 20.0, 35.0, 50.0, 65.0 ];
PLFLT az[]  = [ 30.0, 40.0, 50.0, 60.0 ];

//--------------------------------------------------------------------------
// main
//
// Does a series of 3-d plots for a given data set, with different
// viewing options in each plot.
//--------------------------------------------------------------------------
int main( char[][] args )
{
    const int npts = 1000;

    // Parse and process command line arguments
    plparseopts( args, PL_PARSE_FULL );

    // Initialize plplot
    plinit();

    for ( int k = 0; k < 4; k++ )
        test_poly( k );

    PLFLT[] x = new PLFLT[npts];
    PLFLT[] y = new PLFLT[npts];
    PLFLT[] z = new PLFLT[npts];

    // From the mind of a sick and twisted physicist...
    PLFLT r;
    for ( int i = 0; i < npts; i++ )
    {
        z[i] = -1. + 2. * i / npts;

        // Pick one ...
        // r = 1. - cast(PLFLT)i/npts;
        r = z[i];

        x[i] = r * cos( 2. * PI * 6. * i / npts );
        y[i] = r * sin( 2. * PI * 6. * i / npts );
    }

    for ( int k = 0; k < 4; k++ )
    {
        pladv( 0 );
        plvpor( 0.0, 1.0, 0.0, 0.9 );
        plwind( -1.0, 1.0, -0.9, 1.1 );
        plcol0( 1 );
        plw3d( 1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, alt[k], az[k] );
        plbox3( "bnstu", "x axis", 0.0, 0,
            "bnstu", "y axis", 0.0, 0,
            "bcdmnstuv", "z axis", 0.0, 0 );

        plcol0( 2 );

        if ( opt[k] )
        {
            plline3( x, y, z );
        }
        else
        {
            // U+22C5 DOT OPERATOR.
            plstring3( x, y, z, "⋅" );
        }

        plcol0( 3 );
        plmtex( "t", 1.0, 0.5, 0.5, format( "#frPLplot Example 18 - Alt=%.0f, Az=%.0f", alt[k], az[k] ) );
    }

    plend();

    return 0;
}

void test_poly( int k )
{
    PLINT draw[][] = [ [ 1, 1, 1, 1 ],
                       [ 1, 0, 1, 0 ],
                       [ 0, 1, 0, 1 ],
                       [ 1, 1, 0, 0 ] ];

    PLFLT[] x = new PLFLT[5];
    PLFLT[] y = new PLFLT[5];
    PLFLT[] z = new PLFLT[5];

    pladv( 0 );
    plvpor( 0.0, 1.0, 0.0, 0.9 );
    plwind( -1.0, 1.0, -0.9, 1.1 );
    plcol0( 1 );
    plw3d( 1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, alt[k], az[k] );
    plbox3( "bnstu", "x axis", 0.0, 0,
        "bnstu", "y axis", 0.0, 0,
        "bcdmnstuv", "z axis", 0.0, 0 );

    plcol0( 2 );

    PLFLT theta( int a )
    {
        return 2 * PI * a / 20;
    }
    PLFLT phi( int a )
    {
        return PI * a / 20.1;
    }

    for ( int i = 0; i < 20; i++ )
    {
        for ( int j = 0; j < 20; j++ )
        {
            x[0] = sin( phi( j ) ) * cos( theta( i ) );
            y[0] = sin( phi( j ) ) * sin( theta( i ) );
            z[0] = cos( phi( j ) );

            x[1] = sin( phi( j + 1 ) ) * cos( theta( i ) );
            y[1] = sin( phi( j + 1 ) ) * sin( theta( i ) );
            z[1] = cos( phi( j + 1 ) );

            x[2] = sin( phi( j + 1 ) ) * cos( theta( i + 1 ) );
            y[2] = sin( phi( j + 1 ) ) * sin( theta( i + 1 ) );
            z[2] = cos( phi( j + 1 ) );

            x[3] = sin( phi( j ) ) * cos( theta( i + 1 ) );
            y[3] = sin( phi( j ) ) * sin( theta( i + 1 ) );
            z[3] = cos( phi( j ) );

            x[4] = sin( phi( j ) ) * cos( theta( i ) );
            y[4] = sin( phi( j ) ) * sin( theta( i ) );
            z[4] = cos( phi( j ) );

            plpoly3( x, y, z, draw[k], 1 );
        }
    }

    plcol0( 3 );
    plmtex( "t", 1.0, 0.5, 0.5, "unit radius sphere" );
}
