//      plshade demo, using color fill.
//
//      Maurice LeBrun
//      IFS, University of Texas at Austin
//      20 Mar 1994
//

#include "plcdemos.h"

// Fundamental settings.  See notes[] for more info.

static int ns      = 20;        // Default number of shade levels
static int nx      = 35;        // Default number of data points in x
static int ny      = 46;        // Default number of data points in y
static int exclude = 0;         // By default do not plot a page illustrating
                                // exclusion.  API is probably going to change
                                // anyway, and cannot be reproduced by any
                                // front end other than the C one.

// For now, don't show the colorbars while we are working out the API.
static int colorbar = 1;

// polar plot data
#define PERIMETERPTS    100

// Transformation function

PLFLT tr[6];

static void
mypltr( PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, void * PL_UNUSED( pltr_data ) )
{
    *tx = tr[0] * x + tr[1] * y + tr[2];
    *ty = tr[3] * x + tr[4] * y + tr[5];
}

// Function prototypes

static void
f2mnmx( PLFLT **f, PLINT nx, PLINT ny, PLFLT *fmin, PLFLT *fmax );

PLINT zdefined( PLFLT x, PLFLT y );

// Options data structure definition.

static PLOptionTable options[] = {
    {
        "colorbar",              // Turns on the colorbar for each page
        NULL,
        NULL,
        &colorbar,
        PL_OPT_BOOL,
        "-colorbar",
        "Plot a \"color bar\" on each page."
    },
    {
        "exclude",              // Turns on page showing exclusion
        NULL,
        NULL,
        &exclude,
        PL_OPT_BOOL,
        "-exclude",
        "Plot the \"exclusion\" page."
    },
    {
        "ns",                   // Number of shade levels
        NULL,
        NULL,
        &ns,
        PL_OPT_INT,
        "-ns levels",
        "Sets number of shade levels"
    },
    {
        "nx",                   // Number of data points in x
        NULL,
        NULL,
        &nx,
        PL_OPT_INT,
        "-nx xpts",
        "Sets number of data points in x"
    },
    {
        "ny",                   // Number of data points in y
        NULL,
        NULL,
        &ny,
        PL_OPT_INT,
        "-ny ypts",
        "Sets number of data points in y"
    },
    {
        NULL,                   // option
        NULL,                   // handler
        NULL,                   // client data
        NULL,                   // address of variable to set
        0,                      // mode flag
        NULL,                   // short syntax
        NULL
    }                           // long syntax
};

static PLCHAR_VECTOR notes[] = {
    "To get smoother color variation, increase ns, nx, and ny.  To get faster",
    "response (especially on a serial link), decrease them.  A decent but quick",
    "test results from ns around 5 and nx, ny around 25.",
    NULL
};

PLINT
zdefined( PLFLT x, PLFLT y )
{
    PLFLT z = sqrt( x * x + y * y );

    return z < 0.4 || z > 0.6;
}



//--------------------------------------------------------------------------
// main
//
// Does several shade plots using different coordinate mappings.
//--------------------------------------------------------------------------

int
main( int argc, char *argv[] )
{
    int           i, j;
    PLFLT         x, y, argx, argy, distort, r, t;
    PLFLT         px[PERIMETERPTS], py[PERIMETERPTS];

    PLFLT         **z, **w, zmin, zmax;
    PLFLT         *clevel, *shedge, *xg1, *yg1;
    PLcGrid       cgrid1;
    PLcGrid2      cgrid2;

    PLFLT         fill_width = 2., cont_width = 0.;
    PLFLT         colorbar_width, colorbar_height;
    PLINT         cont_color = 0;
#define NUM_AXES    1
    PLINT         n_axis_opts = NUM_AXES;
    PLCHAR_VECTOR axis_opts[] = {
        "bcvtm",
    };
    PLINT         num_values[NUM_AXES];
    PLFLT         *values[NUM_AXES];
    PLFLT         axis_ticks[NUM_AXES] = {
        0.0,
    };
    PLINT         axis_subticks[NUM_AXES] = {
        0,
    };
#define NUM_LABELS    1
    PLINT         n_labels     = NUM_LABELS;
    PLINT         label_opts[] = {
        PL_COLORBAR_LABEL_BOTTOM,
    };
    PLCHAR_VECTOR labels[] = {
        "Magnitude",
    };

// Parse and process command line arguments

    plMergeOpts( options, "x16c options", notes );
    plparseopts( &argc, argv, PL_PARSE_FULL );

// Load colour palettes
    plspal0( "cmap0_black_on_white.pal" );
    plspal1( "cmap1_gray.pal", 1 );
// Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
    plscmap0n( 3 );
// Initialize plplot

    plinit();

// Set up transformation function

    tr[0] = 2. / ( nx - 1 );
    tr[1] = 0.0;
    tr[2] = -1.0;
    tr[3] = 0.0;
    tr[4] = 2. / ( ny - 1 );
    tr[5] = -1.0;

// Allocate data structures

    clevel = (PLFLT *) calloc( (size_t) ns, sizeof ( PLFLT ) );
    shedge = (PLFLT *) calloc( (size_t) ( ns + 1 ), sizeof ( PLFLT ) );
    xg1    = (PLFLT *) calloc( (size_t) nx, sizeof ( PLFLT ) );
    yg1    = (PLFLT *) calloc( (size_t) ny, sizeof ( PLFLT ) );

    plAlloc2dGrid( &z, nx, ny );
    plAlloc2dGrid( &w, nx, ny );

// Set up data array

    for ( i = 0; i < nx; i++ )
    {
        x = (PLFLT) ( i - ( nx / 2 ) ) / (PLFLT) ( nx / 2 );
        for ( j = 0; j < ny; j++ )
        {
            y = (PLFLT) ( j - ( ny / 2 ) ) / (PLFLT) ( ny / 2 ) - 1.0;

            z[i][j] = -sin( 7. * x ) * cos( 7. * y ) + x * x - y * y;
            w[i][j] = -cos( 7. * x ) * sin( 7. * y ) + 2 * x * y;
        }
    }
    f2mnmx( z, nx, ny, &zmin, &zmax );
    for ( i = 0; i < ns; i++ )
        clevel[i] = zmin + ( zmax - zmin ) * ( i + 0.5 ) / (PLFLT) ns;

    for ( i = 0; i < ns + 1; i++ )
        shedge[i] = zmin + ( zmax - zmin ) * (PLFLT) i / (PLFLT) ns;

// Set up coordinate grids

    cgrid1.xg = xg1;
    cgrid1.yg = yg1;
    cgrid1.nx = nx;
    cgrid1.ny = ny;

    plAlloc2dGrid( &cgrid2.xg, nx, ny );
    plAlloc2dGrid( &cgrid2.yg, nx, ny );
    cgrid2.nx = nx;
    cgrid2.ny = ny;

    for ( i = 0; i < nx; i++ )
    {
        for ( j = 0; j < ny; j++ )
        {
            mypltr( (PLFLT) i, (PLFLT) j, &x, &y, NULL );

            argx    = x * M_PI / 2;
            argy    = y * M_PI / 2;
            distort = 0.4;

            cgrid1.xg[i] = x + distort * cos( argx );
            cgrid1.yg[j] = y - distort * cos( argy );

            cgrid2.xg[i][j] = x + distort * cos( argx ) * cos( argy );
            cgrid2.yg[i][j] = y - distort * cos( argx ) * cos( argy );
        }
    }

// Plot using identity transform

    pladv( 0 );
    plvpor( 0.1, 0.9, 0.1, 0.9 );
    plwind( -1.0, 1.0, -1.0, 1.0 );

    plpsty( 0 );

    plshades( (PLFLT_MATRIX) z, nx, ny, NULL, -1., 1., -1., 1.,
        shedge, ns + 1, fill_width,
        cont_color, cont_width,
        plfill, 1, NULL, NULL );

    if ( colorbar )
    {
        // Smaller text
        plschr( 0.0, 0.75 );
        // Small ticks on the vertical axis
        plsmaj( 0.0, 0.5 );
        plsmin( 0.0, 0.5 );

        num_values[0] = ns + 1;
        values[0]     = shedge;
        plcolorbar( &colorbar_width, &colorbar_height,
            PL_COLORBAR_SHADE | PL_COLORBAR_SHADE_LABEL, 0,
            0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0,
            cont_color, cont_width,
            n_labels, label_opts, labels,
            n_axis_opts, axis_opts,
            axis_ticks, axis_subticks,
            num_values, (PLFLT_MATRIX) values );

        // Reset text and tick sizes
        plschr( 0.0, 1.0 );
        plsmaj( 0.0, 1.0 );
        plsmin( 0.0, 1.0 );
    }

    plcol0( 1 );
    plbox( "bcnst", 0.0, 0, "bcnstv", 0.0, 0 );
    plcol0( 2 );
//
//  plcont((const PLFLT **) w, nx, ny, 1, nx, 1, ny, clevel, ns, mypltr, NULL);
//
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

    plshades( (PLFLT_MATRIX) z, nx, ny, NULL, -1., 1., -1., 1.,
        shedge, ns + 1, fill_width,
        cont_color, cont_width,
        plfill, 1, pltr1, (void *) &cgrid1 );

    if ( colorbar )
    {
        // Smaller text
        plschr( 0.0, 0.75 );
        // Small ticks on the vertical axis
        plsmaj( 0.0, 0.5 );
        plsmin( 0.0, 0.5 );

        num_values[0] = ns + 1;
        values[0]     = shedge;
        plcolorbar( &colorbar_width, &colorbar_height,
            PL_COLORBAR_SHADE | PL_COLORBAR_SHADE_LABEL, 0,
            0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0,
            cont_color, cont_width,
            n_labels, label_opts, labels,
            n_axis_opts, axis_opts,
            axis_ticks, axis_subticks,
            num_values, (PLFLT_MATRIX) values );

        // Reset text and tick sizes
        plschr( 0.0, 1.0 );
        plsmaj( 0.0, 1.0 );
        plsmin( 0.0, 1.0 );
    }

    plcol0( 1 );
    plbox( "bcnst", 0.0, 0, "bcnstv", 0.0, 0 );
    plcol0( 2 );
//
//  plcont((const PLFLT **) w, nx, ny, 1, nx, 1, ny, clevel, ns, pltr1, (void *) &cgrid1);
//
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

    plshades( (PLFLT_MATRIX) z, nx, ny, NULL, -1., 1., -1., 1.,
        shedge, ns + 1, fill_width,
        cont_color, cont_width,
        plfill, 0, pltr2, (void *) &cgrid2 );

    if ( colorbar )
    {
        // Smaller text
        plschr( 0.0, 0.75 );
        // Small ticks on the vertical axis
        plsmaj( 0.0, 0.5 );
        plsmin( 0.0, 0.5 );

        num_values[0] = ns + 1;
        values[0]     = shedge;
        plcolorbar( &colorbar_width, &colorbar_height,
            PL_COLORBAR_SHADE | PL_COLORBAR_SHADE_LABEL, 0,
            0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0,
            cont_color, cont_width,
            n_labels, label_opts, labels,
            n_axis_opts, axis_opts,
            axis_ticks, axis_subticks,
            num_values, (PLFLT_MATRIX) values );

        // Reset text and tick sizes
        plschr( 0.0, 1.0 );
        plsmaj( 0.0, 1.0 );
        plsmin( 0.0, 1.0 );
    }

    plcol0( 1 );
    plbox( "bcnst", 0.0, 0, "bcnstv", 0.0, 0 );
    plcol0( 2 );
    plcont( (PLFLT_MATRIX) w, nx, ny, 1, nx, 1, ny, clevel, ns, pltr2, (void *) &cgrid2 );

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

    plshades( (PLFLT_MATRIX) z, nx, ny, NULL, -1., 1., -1., 1.,
        shedge, ns + 1, fill_width,
        2, 3.,
        plfill, 0, pltr2, (void *) &cgrid2 );

    if ( colorbar )
    {
        // Smaller text
        plschr( 0.0, 0.75 );
        // Small ticks on the vertical axis
        plsmaj( 0.0, 0.5 );
        plsmin( 0.0, 0.5 );

        num_values[0] = ns + 1;
        values[0]     = shedge;
        plcolorbar( &colorbar_width, &colorbar_height,
            PL_COLORBAR_SHADE | PL_COLORBAR_SHADE_LABEL, 0,
            0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0,
            2, 3.,
            n_labels, label_opts, labels,
            n_axis_opts, axis_opts,
            axis_ticks, axis_subticks,
            num_values, (PLFLT_MATRIX) values );

        // Reset text and tick sizes
        plschr( 0.0, 1.0 );
        plsmaj( 0.0, 1.0 );
        plsmin( 0.0, 1.0 );
    }

    plcol0( 1 );
    plbox( "bcnst", 0.0, 0, "bcnstv", 0.0, 0 );
    plcol0( 2 );
//    plcont((const PLFLT **) w, nx, ny, 1, nx, 1, ny, clevel, ns, pltr2, (void *) &cgrid2);

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

        plshades( (PLFLT_MATRIX) z, nx, ny, zdefined, -1., 1., -1., 1.,
            shedge, ns + 1, fill_width,
            cont_color, cont_width,
            plfill, 0, pltr2, (void *) &cgrid2 );

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

    for ( i = 0; i < nx; i++ )
    {
        r = ( (PLFLT) i ) / ( nx - 1 );
        for ( j = 0; j < ny; j++ )
        {
            t = ( 2. * M_PI / ( ny - 1. ) ) * j;
            cgrid2.xg[i][j] = r * cos( t );
            cgrid2.yg[i][j] = r * sin( t );
            z[i][j]         = exp( -r * r ) * cos( 5. * M_PI * r ) * cos( 5. * t );
        }
    }

    // Need a new shedge to go along with the new data set.

    f2mnmx( z, nx, ny, &zmin, &zmax );

    for ( i = 0; i < ns + 1; i++ )
        shedge[i] = zmin + ( zmax - zmin ) * (PLFLT) i / (PLFLT) ns;

    //  Now we can shade the interior region.
    plshades( (PLFLT_MATRIX) z, nx, ny, NULL, -1., 1., -1., 1.,
        shedge, ns + 1, fill_width,
        cont_color, cont_width,
        plfill, 0, pltr2, (void *) &cgrid2 );

    if ( colorbar )
    {
        // Smaller text
        plschr( 0.0, 0.75 );
        // Small ticks on the vertical axis
        plsmaj( 0.0, 0.5 );
        plsmin( 0.0, 0.5 );

        num_values[0] = ns + 1;
        values[0]     = shedge;
        plcolorbar( &colorbar_width, &colorbar_height,
            PL_COLORBAR_SHADE | PL_COLORBAR_SHADE_LABEL, 0,
            0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0,
            cont_color, cont_width,
            n_labels, label_opts, labels,
            n_axis_opts, axis_opts,
            axis_ticks, axis_subticks,
            num_values, (PLFLT_MATRIX) values );

        // Reset text and tick sizes
        plschr( 0.0, 1.0 );
        plsmaj( 0.0, 1.0 );
        plsmin( 0.0, 1.0 );
    }

// Now we can draw the perimeter.  (If do before, shade stuff may overlap.)
    for ( i = 0; i < PERIMETERPTS; i++ )
    {
        t     = ( 2. * M_PI / ( PERIMETERPTS - 1 ) ) * (PLFLT) i;
        px[i] = cos( t );
        py[i] = sin( t );
    }
    plcol0( 1 );
    plline( PERIMETERPTS, px, py );

    // And label the plot.

    plcol0( 2 );
    pllab( "", "", "Tokamak Bogon Instability" );


// Clean up

    free( (void *) clevel );
    free( (void *) shedge );
    free( (void *) xg1 );
    free( (void *) yg1 );
    plFree2dGrid( z, nx, ny );
    plFree2dGrid( w, nx, ny );
    plFree2dGrid( cgrid2.xg, nx, ny );
    plFree2dGrid( cgrid2.yg, nx, ny );

    plend();

    exit( 0 );
}

//--------------------------------------------------------------------------
// f2mnmx
//
// Returns min & max of input 2d array.
//--------------------------------------------------------------------------

static void
f2mnmx( PLFLT **f, PLINT nnx, PLINT nny, PLFLT *fnmin, PLFLT *fnmax )
{
    int i, j;

    *fnmax = f[0][0];
    *fnmin = *fnmax;

    for ( i = 0; i < nnx; i++ )
    {
        for ( j = 0; j < nny; j++ )
        {
            *fnmax = MAX( *fnmax, f[i][j] );
            *fnmin = MIN( *fnmin, f[i][j] );
        }
    }
}
