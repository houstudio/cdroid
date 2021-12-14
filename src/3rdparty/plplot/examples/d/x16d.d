//      plshade demo, using color fill.
//
//      Maurice LeBrun
//      IFS, University of Texas at Austin
//      20 Mar 1994
//

import std.string;
import std.math;

import plplot;

// Fundamental settings.  See notes[] for more info.

int ns      = 20; // Default number of shade levels
int nx      = 35; // Default number of data points in x
int ny      = 46; // Default number of data points in y
int exclude = 0;  // By default do not plot a page illustrating
                  // exclusion.  API is probably going to change
                  // anyway, and cannot be reproduced by any
                  // front end other than the C one.

// For now, don't show the colorbars while we are working out the API.
int colorbar = 1;

extern ( C ) {
// Transformation function
PLFLT[] tr;

void mypltr( PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, void* pltr_data )
{
    *tx = tr[0] * x + tr[1] * y + tr[2];
    *ty = tr[3] * x + tr[4] * y + tr[5];
}
}

// Options data structure definition.
//~ static PLOptionTable options[] = {
//~ {
//~ "exclude",			/* Turns on page showing exclusion */
//~ NULL,
//~ NULL,
//~ &exclude,
//~ PL_OPT_BOOL,
//~ "-exclude",
//~ "Plot the \"exclusion\" page." },
//~ {
//~ "ns",			/* Number of shade levels */
//~ NULL,
//~ NULL,
//~ &ns,
//~ PL_OPT_INT,
//~ "-ns levels",
//~ "Sets number of shade levels" },
//~ {
//~ "nx",			/* Number of data points in x */
//~ NULL,
//~ NULL,
//~ &nx,
//~ PL_OPT_INT,
//~ "-nx xpts",
//~ "Sets number of data points in x" },
//~ {
//~ "ny",			/* Number of data points in y */
//~ NULL,
//~ NULL,
//~ &ny,
//~ PL_OPT_INT,
//~ "-ny ypts",
//~ "Sets number of data points in y" },
//~ {
//~ NULL,			/* option */
//~ NULL,			/* handler */
//~ NULL,			/* client data */
//~ NULL,			/* address of variable to set */
//~ 0,				/* mode flag */
//~ NULL,			/* short syntax */
//~ NULL }			/* long syntax */
//~ };

//~ static const char *notes[] = {
//~ "To get smoother color variation, increase ns, nx, and ny.  To get faster",
//~ "response (especially on a serial link), decrease them.  A decent but quick",
//~ "test results from ns around 5 and nx, ny around 25.",
//~ NULL};

extern ( C ) {
PLINT zdefined( PLFLT x, PLFLT y )
{
    PLFLT z = sqrt( x * x + y * y );

    return z < 0.4 || z > 0.6;
}
}


//--------------------------------------------------------------------------
// main
//
// Does several shade plots using different coordinate mappings.
//--------------------------------------------------------------------------
int main( char[][] args )
{
    const int PERIMETERPTS = 100;

    // Parse and process command line arguments
    //plMergeOpts(options, "x16c options", notes);
    plparseopts( args, PL_PARSE_FULL );

    // Load colour palettes
    plspal0( "cmap0_black_on_white.pal" );
    plspal1( "cmap1_gray.pal", 1 );

    // Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
    plscmap0n( 3 );

    // Initialize plplot
    plinit();

    // Set up transformation function
    tr = [ 2. / ( nx - 1 ), 0.0, -1.0, 0.0, 2. / ( ny - 1 ), -1.0 ];

    // Allocate data structures
    PLFLT[][] z = new PLFLT[][nx];
    for ( int i = 0; i < nx; i++ )
        z[i] = new PLFLT[ny];

    PLFLT[][] w = new PLFLT[][nx];
    for ( int i = 0; i < nx; i++ )
        w[i] = new PLFLT[ny];

    // Set up data array
    PLFLT x, y;
    for ( int i = 0; i < nx; i++ )
    {
        x = cast(double) ( i - ( nx / 2 ) ) / ( nx / 2 );
        for ( int j = 0; j < ny; j++ )
        {
            y = cast(double) ( j - ( ny / 2 ) ) / ( ny / 2 ) - 1.0;

            z[i][j] = -sin( 7 * x ) * cos( 7 * y ) + x * x - y * y;
            w[i][j] = -cos( 7 * x ) * sin( 7 * y ) + 2 * x * y;
        }
    }
    PLFLT zmin, zmax;
    f2mnmx( z, zmin, zmax );

    PLFLT[] clevel = new PLFLT[ns];
    for ( int i = 0; i < ns; i++ )
        clevel[i] = zmin + ( zmax - zmin ) * ( i + 0.5 ) / ns;

    PLFLT[] shedge = new PLFLT[ns + 1];
    for ( int i = 0; i < ns + 1; i++ )
        shedge[i] = zmin + ( zmax - zmin ) * i / ns;

    // Set up coordinate grids
    PLcGrid cgrid1;
    cgrid1.xg = new PLFLT[nx];
    cgrid1.yg = new PLFLT[ny];

    PLcGrid2 cgrid2;
    cgrid2.xg = new PLFLT[][nx];
    for ( int i = 0; i < nx; i++ )
        cgrid2.xg[i] = new PLFLT[ny];

    cgrid2.yg = new PLFLT[][nx];
    for ( int i = 0; i < nx; i++ )
        cgrid2.yg[i] = new PLFLT[ny];

    PLFLT argx, argy, distort;
    for ( int i = 0; i < nx; i++ )
    {
        for ( int j = 0; j < ny; j++ )
        {
            mypltr( i, j, &x, &y, null );

            argx    = x * PI / 2;
            argy    = y * PI / 2;
            distort = 0.4;

            cgrid1.xg[i] = x + distort * cos( argx );
            cgrid1.yg[j] = y - distort * cos( argy );

            cgrid2.xg[i][j] = x + distort * cos( argx ) * cos( argy );
            cgrid2.yg[i][j] = y - distort * cos( argx ) * cos( argy );
        }
    }

    // Plot using identity transform
    PLFLT     fill_width = 2., cont_width = 0.;
    PLFLT     colorbar_width, colorbar_height;
    PLINT     cont_color = 0;

    const int NUM_AXES  = 1;
    string[]  axis_opts = [
        "bcvtm",
    ];
    PLFLT[] values[NUM_AXES];
    for ( size_t i = 0; i < NUM_AXES; i++ )
    {
        values[i] = new PLFLT[ns];
    }
    PLFLT[] axis_ticks = [
        0.0,
    ];
    PLINT[] axis_subticks = [
        0,
    ];
    const int NUM_LABELS = 1;
    PLINT label_opts[] = [
        PL_COLORBAR_LABEL_BOTTOM,
    ];
    string[] labels = [
        "Magnitude",
    ];


    pladv( 0 );
    plvpor( 0.1, 0.9, 0.1, 0.9 );
    plwind( -1.0, 1.0, -1.0, 1.0 );

    plpsty( 0 );

    plshades( z, null, -1., 1., -1., 1., shedge, fill_width,
        cont_color, cont_width, 1 );

    if ( colorbar )
    {
        // Smaller text
        plschr( 0.0, 0.75 );
        // Small ticks on the vertical axis
        plsmaj( 0.0, 0.5 );
        plsmin( 0.0, 0.5 );

        values[0] = shedge;
        plcolorbar( &colorbar_width, &colorbar_height,
            PL_COLORBAR_SHADE | PL_COLORBAR_SHADE_LABEL, 0,
            0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0,
            cont_color, cont_width,
            label_opts, labels,
            axis_opts,
            axis_ticks, axis_subticks,
            values );

        // Reset text and tick sizes
        plschr( 0.0, 1.0 );
        plsmaj( 0.0, 1.0 );
        plsmin( 0.0, 1.0 );
    }

    plcol0( 1 );
    plbox( "bcnst", 0.0, 0, "bcnstv", 0.0, 0 );
    plcol0( 2 );
    pllab( "distance", "altitude", "Bogon density" );

    // Plot using 1d coordinate transform

    // Load colour palettes
    plspal0( "cmap0_black_on_white.pal" );
    plspal1( "cmap1_blue_yellow.pal", 1 );

    // Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
    plscmap0n( 3 );

    pladv( 0 );
    plvpor( 0.1, 0.9, 0.1, 0.9 );
    plwind( -1.0, 1.0, -1.0, 1.0 );

    plpsty( 0 );

    plshades( z, null, -1., 1., -1., 1., shedge, fill_width,
        cont_color, cont_width, 1, cgrid1 );
    if ( colorbar )
    {
        // Smaller text
        plschr( 0.0, 0.75 );
        // Small ticks on the vertical axis
        plsmaj( 0.0, 0.5 );
        plsmin( 0.0, 0.5 );

        values[0] = shedge;
        plcolorbar( &colorbar_width, &colorbar_height,
            PL_COLORBAR_SHADE | PL_COLORBAR_SHADE_LABEL, 0,
            0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0,
            cont_color, cont_width,
            label_opts, labels,
            axis_opts,
            axis_ticks, axis_subticks,
            values );

        // Reset text and tick sizes
        plschr( 0.0, 1.0 );
        plsmaj( 0.0, 1.0 );
        plsmin( 0.0, 1.0 );
    }


    plcol0( 1 );
    plbox( "bcnst", 0.0, 0, "bcnstv", 0.0, 0 );
    plcol0( 2 );
    pllab( "distance", "altitude", "Bogon density" );

    // Plot using 2d coordinate transform

    // Load colour palettes
    plspal0( "cmap0_black_on_white.pal" );
    plspal1( "cmap1_blue_red.pal", 1 );

    // Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
    plscmap0n( 3 );

    pladv( 0 );
    plvpor( 0.1, 0.9, 0.1, 0.9 );
    plwind( -1.0, 1.0, -1.0, 1.0 );

    plpsty( 0 );

    plshades( z, null, -1., 1., -1., 1., shedge, fill_width,
        cont_color, cont_width, 0, cgrid2 );

    if ( colorbar )
    {
        // Smaller text
        plschr( 0.0, 0.75 );
        // Small ticks on the vertical axis
        plsmaj( 0.0, 0.5 );
        plsmin( 0.0, 0.5 );

        values[0] = shedge;
        plcolorbar( &colorbar_width, &colorbar_height,
            PL_COLORBAR_SHADE | PL_COLORBAR_SHADE_LABEL, 0,
            0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0,
            cont_color, cont_width,
            label_opts, labels,
            axis_opts,
            axis_ticks, axis_subticks,
            values );

        // Reset text and tick sizes
        plschr( 0.0, 1.0 );
        plsmaj( 0.0, 1.0 );
        plsmin( 0.0, 1.0 );
    }

    plcol0( 1 );
    plbox( "bcnst", 0.0, 0, "bcnstv", 0.0, 0 );
    plcol0( 2 );
    plcont( w, 1, nx, 1, ny, clevel, cgrid2 );

    pllab( "distance", "altitude", "Bogon density, with streamlines" );

    // Plot using 2d coordinate transform

    // Load colour palettes
    plspal0( "" );
    plspal1( "", 1 );

    // Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
    plscmap0n( 3 );

    pladv( 0 );
    plvpor( 0.1, 0.9, 0.1, 0.9 );
    plwind( -1.0, 1.0, -1.0, 1.0 );

    plpsty( 0 );

    plshades( z, null, -1., 1., -1., 1., shedge, fill_width,
        2, 3, 0, cgrid2 );

    if ( colorbar )
    {
        // Smaller text
        plschr( 0.0, 0.75 );
        // Small ticks on the vertical axis
        plsmaj( 0.0, 0.5 );
        plsmin( 0.0, 0.5 );

        values[0] = shedge;
        plcolorbar( &colorbar_width, &colorbar_height,
            PL_COLORBAR_SHADE | PL_COLORBAR_SHADE_LABEL, 0,
            0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0,
            2, 3.,
            label_opts, labels,
            axis_opts,
            axis_ticks, axis_subticks,
            values );

        // Reset text and tick sizes
        plschr( 0.0, 1.0 );
        plsmaj( 0.0, 1.0 );
        plsmin( 0.0, 1.0 );
    }

    plcol0( 1 );
    plbox( "bcnst", 0.0, 0, "bcnstv", 0.0, 0 );
    plcol0( 2 );

    pllab( "distance", "altitude", "Bogon density" );

    // Note this exclusion API will probably change.

    // Plot using 2d coordinate transform and exclusion
    if ( exclude )
    {
        // Load colour palettes
        plspal0( "cmap0_black_on_white.pal" );
        plspal1( "cmap1_gray.pal", 1 );

        // Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
        plscmap0n( 3 );

        pladv( 0 );
        plvpor( 0.1, 0.9, 0.1, 0.9 );
        plwind( -1.0, 1.0, -1.0, 1.0 );

        plpsty( 0 );

        plshades( z, &zdefined, -1., 1., -1., 1., shedge, fill_width,
            cont_color, cont_width, 0, cgrid2 );

        plcol0( 1 );
        plbox( "bcnst", 0.0, 0, "bcnstv", 0.0, 0 );

        pllab( "distance", "altitude", "Bogon density with exclusion" );
    }

    // Example with polar coordinates.

    // Load colour palettes
    plspal0( "cmap0_black_on_white.pal" );
    plspal1( "cmap1_gray.pal", 1 );

    // Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
    plscmap0n( 3 );

    pladv( 0 );
    plvpor( .1, .9, .1, .9 );
    plwind( -1., 1., -1., 1. );

    plpsty( 0 );

    // Build new coordinate matrices.
    PLFLT r, t;
    for ( int i = 0; i < nx; i++ )
    {
        r = cast(PLFLT) i / ( nx - 1 );
        for ( int j = 0; j < ny; j++ )
        {
            t = 2. * PI / ( ny - 1. ) * j;
            cgrid2.xg[i][j] = r * cos( t );
            cgrid2.yg[i][j] = r * sin( t );
            z[i][j]         = exp( -r * r ) * cos( 5. * PI * r ) * cos( 5. * t );
        }
    }

    // Need a new shedge to go along with the new data set.
    f2mnmx( z, zmin, zmax );

    for ( int i = 0; i < ns + 1; i++ )
        shedge[i] = zmin + ( zmax - zmin ) * i / ns;

    //  Now we can shade the interior region.
    plshades( z, null, -1., 1., -1., 1., shedge, fill_width,
        cont_color, cont_width, 0, cgrid2 );

    if ( colorbar )
    {
        // Smaller text
        plschr( 0.0, 0.75 );
        // Small ticks on the vertical axis
        plsmaj( 0.0, 0.5 );
        plsmin( 0.0, 0.5 );

        values[0] = shedge;
        plcolorbar( &colorbar_width, &colorbar_height,
            PL_COLORBAR_SHADE | PL_COLORBAR_SHADE_LABEL, 0,
            0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0,
            cont_color, cont_width,
            label_opts, labels,
            axis_opts,
            axis_ticks, axis_subticks,
            values );

        // Reset text and tick sizes
        plschr( 0.0, 1.0 );
        plsmaj( 0.0, 1.0 );
        plsmin( 0.0, 1.0 );
    }

    // Now we can draw the perimeter.  (If do before, shade stuff may overlap.)
    PLFLT[PERIMETERPTS] px, py;
    for ( int i = 0; i < PERIMETERPTS; i++ )
    {
        t     = 2. * PI / ( PERIMETERPTS - 1 ) * i;
        px[i] = cos( t );
        py[i] = sin( t );
    }
    plcol0( 1 );
    plline( px, py );

    // And label the plot.
    plcol0( 2 );
    pllab( "", "", "Tokamak Bogon Instability" );

    plend();

    return 0;
}

//--------------------------------------------------------------------------
// f2mnmx
//
// Returns min & max of input 2d array.
//--------------------------------------------------------------------------
void f2mnmx( PLFLT[][] f, out PLFLT fmn, out PLFLT fmx )
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
