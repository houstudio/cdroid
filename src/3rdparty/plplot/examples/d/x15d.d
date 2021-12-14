//      Shade plot demo.
//
//      Maurice LeBrun
//      IFS, University of Texas at Austin
//      31 Aug 1993
//

import std.math;
import std.stdio;
import std.string;

import plplot;


//--------------------------------------------------------------------------
// main
//
// Does a variety of shade plots.
//--------------------------------------------------------------------------
int main( char[][] args )
{
    const int XPTS = 35;        // Data points in x
    const int YPTS = 46;        // Data points in y

    // Parse and process command line arguments
    plparseopts( args, PL_PARSE_FULL );

    // Set up color map 0
    // plscmap0n(3);

    // create plot object
    plot myPlot = new plot;

    // Set up color map 1
    myPlot.cmap1_init2();

    // Initialize plplot
    plinit();

    // Set up data array
    PLFLT     xx, yy;
    PLFLT[][] z = new PLFLT[][XPTS];
    for ( int i = 0; i < XPTS; i++ )
        z[i] = new PLFLT[YPTS];

    for ( int i = 0; i < XPTS; i++ )
    {
        xx = cast(double) ( i - ( XPTS / 2 ) ) / ( XPTS / 2 );
        for ( int j = 0; j < YPTS; j++ )
        {
            yy      = cast(double) ( j - ( YPTS / 2 ) ) / ( YPTS / 2 ) - 1.0;
            z[i][j] = xx * xx - yy * yy + ( xx - yy ) / ( xx * xx + yy * yy + 0.1 );
        }
    }

    // Plot using identity transform
    myPlot.plot1( z );
    myPlot.plot2( z );
    myPlot.plot3();

    plend();
    return 0;
}


class plot {
    //--------------------------------------------------------------------------
    // cmap1_init1
    //
    // Initializes color map 1 in HLS space.
//--------------------------------------------------------------------------
    public void cmap1_init1()
    {
        PLFLT[] i = [ 0.0, 0.45, 0.55, 1.0 ];       // left boundary, just before center,
        // just after center, right boundary
        PLFLT[] h = [ 260.0, 260.0, 20.0, 20.0 ];   // hue -- low: blue-violet, only change as we go over vertex
                                                    // hue -- high: red, keep fixed
        PLFLT[] l = [ 0.5, 0.0, 0.0, 0.5 ];         // lightness -- low, lightness -- center
                                                    // lightness -- center, lightness -- high
        PLFLT[] s = [ 1.0, 1.0, 1.0, 1.0 ];         // maximum saturation

        plscmap1l( 0, i, h, l, s );
    }

    //--------------------------------------------------------------------------
    // cmap1_init2
    //
    // Initializes color map 1 in HLS space.
//--------------------------------------------------------------------------
    public void cmap1_init2()
    {
        PLFLT[] i = [ 0.0, 0.45, 0.55, 1.0 ];       // left boundary, just before center,
        // just after center, right boundary
        PLFLT[] h = [ 260.0, 260.0, 20.0, 20.0 ];   // hue -- low: blue-violet, only change as we go over vertex
                                                    // hue -- high: red, keep fixed
        PLFLT[] l = [ 0.6, 0.0, 0.0, 0.6 ];         // lightness -- low, lightness -- center
                                                    // lightness -- center, lightness -- high
        PLFLT[] s = [ 1.0, 0.5, 0.5, 1.0 ];         // saturation -- low, saturation -- center
                                                    // saturation -- center, saturation -- high

        plscmap1l( 0, i, h, l, s );
    }

    //--------------------------------------------------------------------------
    // plot1
    //
    // Illustrates a single shaded region.
//--------------------------------------------------------------------------
    public void plot1( PLFLT[][] z )
    {
        pladv( 0 );
        plvpor( 0.1, 0.9, 0.1, 0.9 );
        plwind( -1.0, 1.0, -1.0, 1.0 );

        PLFLT zmin, zmax;
        f2mnmx( z, zmin, zmax );

        PLFLT shade_min = zmin + ( zmax - zmin ) * 0.4;
        PLFLT shade_max = zmin + ( zmax - zmin ) * 0.6;
        PLFLT sh_color  = 7;
        PLFLT sh_width  = 2.;
        PLINT sh_cmap   = 0;
        PLINT min_color = 9;
        PLINT max_color = 2;
        PLFLT min_width = 2.;
        PLFLT max_width = 2.;

        plpsty( 8 );
        plshade( z, null, -1., 1., -1., 1.,
            shade_min, shade_max,
            sh_cmap, sh_color, sh_width,
            min_color, min_width, max_color, max_width, 1 );

        plcol0( 1 );
        plbox( "bcnst", 0.0, 0, "bcnstv", 0.0, 0 );
        plcol0( 2 );
        pllab( "distance", "altitude", "Bogon flux" );
    }

    //--------------------------------------------------------------------------
    // plot2
    //
    // Illustrates multiple adjacent shaded regions, using different fill
    // patterns for each region.
//--------------------------------------------------------------------------
    public void plot2( PLFLT[][] z )
    {
        static PLINT nlin[10]   = [ 1, 1, 1, 1, 1, 2, 2, 2, 2, 2 ];
        static PLINT inc[10][2] = [ [450, 0], [-450, 0], [0, 0], [900, 0], [300, 0],
                                    [450, -450], [0, 900], [0, 450], [450, -450], [0, 900] ];
        static PLINT del[10][2] = [ [2000, 2000], [2000, 2000], [2000, 2000],
                                    [2000, 2000], [2000, 2000], [2000, 2000],
                                    [2000, 2000], [2000, 2000], [4000, 4000],
                                    [4000, 2000] ];

        PLINT sh_cmap   = 0, sh_width = 2;
        PLINT min_color = 0, min_width = 0, max_color = 0, max_width = 0;

        pladv( 0 );
        plvpor( 0.1, 0.9, 0.1, 0.9 );
        plwind( -1.0, 1.0, -1.0, 1.0 );

        PLFLT zmin, zmax;
        f2mnmx( z, zmin, zmax );

        // Plot using identity transform
        for ( int i = 0; i < 10; i++ )
        {
            PLFLT shade_min = zmin + ( zmax - zmin ) * i / 10.0;
            PLFLT shade_max = zmin + ( zmax - zmin ) * ( i + 1 ) / 10.0;
            PLFLT sh_color  = i + 6;
            plpat( inc[i][0..nlin[i]], del[i][0..nlin[i]] );

            plshade( z, null, -1., 1., -1., 1.,
                shade_min, shade_max,
                sh_cmap, sh_color, sh_width,
                min_color, min_width, max_color, max_width, 1 );
        }

        plcol0( 1 );
        plbox( "bcnst", 0.0, 0, "bcnstv", 0.0, 0 );
        plcol0( 2 );
        pllab( "distance", "altitude", "Bogon flux" );
    }

    //--------------------------------------------------------------------------
    // plot3
    //
    // Illustrates shaded regions in 3d, using a different fill pattern for
    // each region.
//--------------------------------------------------------------------------
    public void plot3()
    {
        static PLFLT xx[2][5] = [ [-1.0, 1.0, 1.0, -1.0, -1.0],
                                  [-1.0, 1.0, 1.0, -1.0, -1.0] ];
        static PLFLT yy[2][5] = [ [1.0, 1.0, 0.0, 0.0, 1.0],
                                  [-1.0, -1.0, 0.0, 0.0, -1.0] ];
        static PLFLT zz[2][5] = [ [0.0, 0.0, 1.0, 1.0, 0.0],
                                  [0.0, 0.0, 1.0, 1.0, 0.0] ];

        pladv( 0 );
        plvpor( 0.1, 0.9, 0.1, 0.9 );
        plwind( -1.0, 1.0, -1.0, 1.0 );
        plw3d( 1., 1., 1., -1.0, 1.0, -1.0, 1.0, 0.0, 1.5, 30, -40 );

        // Plot using identity transform
        plcol0( 1 );
        plbox3( "bntu", "X", 0.0, 0, "bntu", "Y", 0.0, 0, "bcdfntu", "Z", 0.5, 0 );
        plcol0( 2 );
        pllab( "", "", "3-d polygon filling" );

        plcol0( 3 );
        plpsty( 1 );
        plline3( xx[0], yy[0], zz[0] );
        plfill3( xx[0][0..4], yy[0][0..4], zz[0][0..4] );
        plpsty( 2 );
        plline3( xx[1], yy[1], zz[1] );
        plfill3( xx[1][0..4], yy[1][0..4], zz[1][0..4] );
    }

    //--------------------------------------------------------------------------
    // f2mnmx
    //
    // Returns min & max of input 2d array.
//--------------------------------------------------------------------------
    public void f2mnmx( PLFLT[][] f, out PLFLT fmn, out PLFLT fmx )
    {
        fmx = f[0][0];
        fmn = fmx;

        for ( int i = 0; i < f.length; i++ )
        {
            for ( int j = 0; j < f[i].length; j++ )
            {
                fmx = fmax( fmx, f[i][j] );
                fmn = fmin( fmn, f[i][j] );
            }
        }
    }
}
