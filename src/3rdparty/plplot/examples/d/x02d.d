//      Multiple window and color map 0 demo.
//

import std.string;
import std.conv;

import plplot;


//--------------------------------------------------------------------------
// main
//
// Demonstrates multiple windows and color map 0 palette, both default and
// user-modified.
//--------------------------------------------------------------------------
int main( char[][] args )
{
    // Parse and process command line arguments
    plparseopts( args, PL_PARSE_FULL );

    // Initialize plplot
    plinit();

    // Run demos
    plot myPlot = new plot;

    myPlot.demo1();
    myPlot.demo2();

    plend();
    return 0;
}

class plot {
    //--------------------------------------------------------------------------
    // demo1
    // Demonstrates multiple windows and default color map 0 palette.
//--------------------------------------------------------------------------
    public void demo1()
    {
        plbop();

        // Divide screen into 16 regions
        plssub( 4, 4 );

        draw_windows( 16, 0 );

        pleop();
    }

    //--------------------------------------------------------------------------
    // demo2
    // Demonstrates multiple windows, user-modified color map 0 palette, and
    // HLS -> RGB translation.
//--------------------------------------------------------------------------
    public void demo2()
    {
        // Set up cmap0
        // Use 100 custom colors in addition to base 16
        PLINT[116] r, g, b;

        // Min & max lightness values
        PLFLT lmin = 0.15, lmax = 0.85;

        plbop();

        // Divide screen into 100 regions
        plssub( 10, 10 );

        for ( int i = 0; i < 100; i++ )
        {
            // Bounds on HLS, from plhlsrgb() commentary --
            //	hue		[0., 360.]	degrees
            //	lightness	[0., 1.]	magnitude
            //	saturation	[0., 1.]	magnitude
            //

            // Vary hue uniformly from left to right
            PLFLT h = ( 360. / 10. ) * ( i % 10 );

            // Vary lightness uniformly from top to bottom, between min & max
            PLFLT l = lmin + ( lmax - lmin ) * ( i / 10 ) / 9.0;

            // Use max saturation
            PLFLT s = 1.0;

            PLFLT r1, g1, b1;
            plhlsrgb( h, l, s, &r1, &g1, &b1 );

            // Use 255.001 to avoid close truncation decisions in this example.
            r[i + 16] = cast(PLINT) ( r1 * 255.001 );
            g[i + 16] = cast(PLINT) ( g1 * 255.001 );
            b[i + 16] = cast(PLINT) ( b1 * 255.001 );
        }

        // Load default cmap0 colors into our custom set
        for ( int i = 0; i < 16; i++ )
            plgcol0( i, &r[i], &g[i], &b[i] );

        // Now set cmap0 all at once (faster, since fewer driver calls)
        plscmap0( r, g, b );

        draw_windows( 100, 16 );

        pleop();
    }

    //--------------------------------------------------------------------------
    // draw_windows
    //
    // Draws a set of numbered boxes with colors according to cmap0 entry.
//--------------------------------------------------------------------------
    private void draw_windows( int nw, int cmap0_offset )
    {
        plschr( 0.0, 3.5 );
        plfont( 4 );

        for ( int i = 0; i < nw; i++ )
        {
            plcol0( i + cmap0_offset );
            pladv( 0 );

            PLFLT vmin = 0.1;
            PLFLT vmax = 0.9;
            for ( int j = 0; j < 3; j++ )
            {
                plwidth( j + 1 );
                plvpor( vmin, vmax, vmin, vmax );
                plwind( 0.0, 1.0, 0.0, 1.0 );
                plbox( "bc", 0.0, 0, "bc", 0.0, 0 );
                vmin = vmin + 0.1;
                vmax = vmax - 0.1;
            }
            plwidth( 1 );
            plptex( 0.5, 0.5, 1.0, 0.0, 0.5, to!string( i ) );
        }
    }
}
