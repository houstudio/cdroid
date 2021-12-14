//      Grid data demo
//
// Copyright (C) 2004  Joao Cardoso
//
// This file is part of PLplot.
//
// PLplot is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as published
// by the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// PLplot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with PLplot; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
//

#include "plcdemos.h"

// Options data structure definition.

static PLINT         pts       = 500;
static PLINT         xp        = 25;
static PLINT         yp        = 20;
static PLINT         nl        = 16;
static int           knn_order = 20;
static PLFLT         threshold = 1.001;
static PLFLT         wmin      = -1e3;
static int           randn     = 0;
static int           rosen     = 0;

static PLOptionTable options[] = {
    {
        "npts",
        NULL,
        NULL,
        &pts,
        PL_OPT_INT,
        "-npts points",
        "Specify number of random points to generate [500]"
    },
    {
        "randn",
        NULL,
        NULL,
        &randn,
        PL_OPT_BOOL,
        "-randn",
        "Normal instead of uniform sampling -- the effective \n\
\t\t\t  number of points will be smaller than the specified."
    },
    {
        "rosen",
        NULL,
        NULL,
        &rosen,
        PL_OPT_BOOL,
        "-rosen",
        "Generate points from the Rosenbrock function."
    },
    {
        "nx",
        NULL,
        NULL,
        &xp,
        PL_OPT_INT,
        "-nx points",
        "Specify grid x dimension [25]"
    },
    {
        "ny",
        NULL,
        NULL,
        &yp,
        PL_OPT_INT,
        "-ny points",
        "Specify grid y dimension [20]"
    },
    {
        "nlevel",
        NULL,
        NULL,
        &nl,
        PL_OPT_INT,
        "-nlevel ",
        "Specify number of contour levels [15]"
    },
    {
        "knn_order",
        NULL,
        NULL,
        &knn_order,
        PL_OPT_INT,
        "-knn_order order",
        "Specify the number of neighbors [20]"
    },
    {
        "threshold",
        NULL,
        NULL,
        &threshold,
        PL_OPT_FLOAT,
        "-threshold float",
        "Specify what a thin triangle is [1. < [1.001] < 2.]"
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

void create_data( PLFLT **xi, PLFLT **yi, PLFLT **zi, int npts );
void free_data( PLFLT *x, PLFLT *y, PLFLT *z );
void create_grid( PLFLT **xi, int px, PLFLT **yi, int py );
void free_grid( PLFLT *x, PLFLT *y );

static void
cmap1_init( void )
{
    PLFLT i[2], h[2], l[2], s[2];

    i[0] = 0.0;         // left boundary
    i[1] = 1.0;         // right boundary

    h[0] = 240;         // blue -> green -> yellow ->
    h[1] = 0;           // -> red

    l[0] = 0.6;
    l[1] = 0.6;

    s[0] = 0.8;
    s[1] = 0.8;

    plscmap1n( 256 );
    c_plscmap1l( 0, 2, i, h, l, s, NULL );
}

PLFLT xm, xM, ym, yM;

int
main( int argc, char *argv[] )
{
    PLFLT         *x, *y, *z, *clev;
    PLFLT         *xg, *yg, **zg;
    PLFLT         zmin, zmax, lzm, lzM;
    int           i, j, k;
    PLINT         alg;
    PLCHAR_VECTOR title[] = { "Cubic Spline Approximation",
                              "Delaunay Linear Interpolation",
                              "Natural Neighbors Interpolation",
                              "KNN Inv. Distance Weighted",
                              "3NN Linear Interpolation",
                              "4NN Around Inv. Dist. Weighted" };

    PLFLT         opt[] = { 0., 0., 0., 0., 0., 0. };

    xm = ym = -0.2;
    xM = yM = 0.6;

    plMergeOpts( options, "x21c options", NULL );
    plparseopts( &argc, argv, PL_PARSE_FULL );

    opt[2] = wmin;
    opt[3] = (PLFLT) knn_order;
    opt[4] = threshold;

    // Initialize plplot

    plinit();

    // Use a colour map with no black band in the middle.
    cmap1_init();
    // Initialise random number generator
    plseed( 5489 );

    create_data( &x, &y, &z, pts ); // the sampled data
    zmin = z[0];
    zmax = z[0];
    for ( i = 1; i < pts; i++ )
    {
        if ( z[i] > zmax )
            zmax = z[i];
        if ( z[i] < zmin )
            zmin = z[i];
    }

    create_grid( &xg, xp, &yg, yp ); // grid the data at
    plAlloc2dGrid( &zg, xp, yp );    // the output grided data
    clev = (PLFLT *) malloc( (size_t) nl * sizeof ( PLFLT ) );

    plcol0( 1 );
    plenv( xm, xM, ym, yM, 2, 0 );
    plcol0( 15 );
    pllab( "X", "Y", "The original data sampling" );
    for ( i = 0; i < pts; i++ )
    {
        plcol1( ( z[i] - zmin ) / ( zmax - zmin ) );
        // The following plstring call should be the the equivalent of
        // plpoin( 1, &x[i], &y[i], 5 ); Use plstring because it is
        // not deprecated like plpoin and has much more powerful
        // capabilities.  N.B. symbol 141 works for Hershey devices
        // (e.g., -dev xwin) only if plfontld( 0 ) has been called
        // while symbol 727 works only if plfontld( 1 ) has been
        // called.  The latter is the default which is why we use 727
        // here to represent a centred X (multiplication) symbol.
        // This dependence on plfontld is one of the limitations of
        // the Hershey escapes for PLplot, but the upside is you get
        // reasonable results for both Hershey and Unicode devices.
        plstring( 1, &x[i], &y[i], "#(727)" );
    }
    pladv( 0 );

    plssub( 3, 2 );

    for ( k = 0; k < 2; k++ )
    {
        pladv( 0 );
        for ( alg = 1; alg < 7; alg++ )
        {
            plgriddata( x, y, z, pts, xg, xp, yg, yp, zg, alg, opt[alg - 1] );

            // - CSA can generate NaNs (only interpolates?!).
            // - DTLI and NNI can generate NaNs for points outside the convex hull
            //      of the data points.
            // - NNLI can generate NaNs if a sufficiently thick triangle is not found
            //
            // PLplot should be NaN/Inf aware, but changing it now is quite a job...
            // so, instead of not plotting the NaN regions, a weighted average over
            // the neighbors is done.
            //

            if ( alg == GRID_CSA || alg == GRID_DTLI || alg == GRID_NNLI || alg == GRID_NNI )
            {
                int   ii, jj;
                PLFLT dist, d;

                for ( i = 0; i < xp; i++ )
                {
                    for ( j = 0; j < yp; j++ )
                    {
                        if ( isnan( zg[i][j] ) ) // average (IDW) over the 8 neighbors

                        {
                            zg[i][j] = 0.; dist = 0.;

                            for ( ii = i - 1; ii <= i + 1 && ii < xp; ii++ )
                            {
                                for ( jj = j - 1; jj <= j + 1 && jj < yp; jj++ )
                                {
                                    if ( ii >= 0 && jj >= 0 && !isnan( zg[ii][jj] ) )
                                    {
                                        d         = ( abs( ii - i ) + abs( jj - j ) ) == 1 ? 1. : 1.4142;
                                        zg[i][j] += zg[ii][jj] / ( d * d );
                                        dist     += d;
                                    }
                                }
                            }
                            if ( dist != 0. )
                                zg[i][j] /= dist;
                            else
                                zg[i][j] = zmin;
                        }
                    }
                }
            }

            plMinMax2dGrid( (PLFLT_MATRIX) zg, xp, yp, &lzM, &lzm );

            lzm = MIN( lzm, zmin );
            lzM = MAX( lzM, zmax );

            // Increase limits slightly to prevent spurious contours
            // due to rounding errors
            lzm = lzm - 0.01;
            lzM = lzM + 0.01;

            plcol0( 1 );

            pladv( alg );

            if ( k == 0 )
            {
                for ( i = 0; i < nl; i++ )
                    clev[i] = lzm + ( lzM - lzm ) / ( nl - 1 ) * i;

                plenv0( xm, xM, ym, yM, 2, 0 );
                plcol0( 15 );
                pllab( "X", "Y", title[alg - 1] );
                plshades( (PLFLT_MATRIX) zg, xp, yp, NULL, xm, xM, ym, yM,
                    clev, nl, 1., 0, 1., plfill, 1, NULL, NULL );
                plcol0( 2 );
            }
            else
            {
                for ( i = 0; i < nl; i++ )
                    clev[i] = lzm + ( lzM - lzm ) / ( nl - 1 ) * i;

                plvpor( 0.0, 1.0, 0.0, 0.9 );
                plwind( -1.1, 0.75, -0.65, 1.20 );
                //
                // For the comparison to be fair, all plots should have the
                // same z values, but to get the max/min of the data generated
                // by all algorithms would imply two passes. Keep it simple.
                //
                // plw3d(1., 1., 1., xm, xM, ym, yM, zmin, zmax, 30, -60);
                //

                plw3d( 1., 1., 1., xm, xM, ym, yM, lzm, lzM, 30, -40 );
                plbox3( "bntu", "X", 0., 0,
                    "bntu", "Y", 0., 0,
                    "bcdfntu", "Z", 0.5, 0 );
                plcol0( 15 );
                pllab( "", "", title[alg - 1] );
                plot3dc( xg, yg, (PLFLT_MATRIX) zg, xp, yp, DRAW_LINEXY | MAG_COLOR | BASE_CONT, clev, nl );
            }
        }
    }

    plend();

    free_data( x, y, z );
    free_grid( xg, yg );
    free( (void *) clev );
    plFree2dGrid( zg, xp, yp );
    exit( 0 );
}


void
create_grid( PLFLT **xi, int px, PLFLT **yi, int py )
{
    PLFLT *x, *y;
    int   i;

    x = *xi = (PLFLT *) malloc( (size_t) px * sizeof ( PLFLT ) );
    y = *yi = (PLFLT *) malloc( (size_t) py * sizeof ( PLFLT ) );

    for ( i = 0; i < px; i++ )
        *x++ = xm + ( xM - xm ) * i / ( px - 1. );

    for ( i = 0; i < py; i++ )
        *y++ = ym + ( yM - ym ) * i / ( py - 1. );
}

void
free_grid( PLFLT *xi, PLFLT *yi )
{
    free( (void *) xi );
    free( (void *) yi );
}

void
create_data( PLFLT **xi, PLFLT **yi, PLFLT **zi, int npts )
{
    int   i;
    PLFLT *x, *y, *z, r;
    PLFLT xt, yt;

    *xi = x = (PLFLT *) malloc( (size_t) npts * sizeof ( PLFLT ) );
    *yi = y = (PLFLT *) malloc( (size_t) npts * sizeof ( PLFLT ) );
    *zi = z = (PLFLT *) malloc( (size_t) npts * sizeof ( PLFLT ) );

    for ( i = 0; i < npts; i++ )
    {
        xt = ( xM - xm ) * plrandd();
        yt = ( yM - ym ) * plrandd();
        if ( !randn )
        {
            *x = xt + xm;
            *y = yt + ym;
        }
        else // std=1, meaning that many points are outside the plot range
        {
            *x = sqrt( -2. * log( xt ) ) * cos( 2. * M_PI * yt ) + xm;
            *y = sqrt( -2. * log( xt ) ) * sin( 2. * M_PI * yt ) + ym;
        }
        if ( !rosen )
        {
            r  = sqrt( ( *x ) * ( *x ) + ( *y ) * ( *y ) );
            *z = exp( -r * r ) * cos( 2.0 * M_PI * r );
        }
        else
        {
            *z = log( pow( 1. - *x, 2. ) + 100. * pow( *y - pow( *x, 2. ), 2. ) );
        }
        x++; y++; z++;
    }
}

void
free_data( PLFLT *x, PLFLT *y, PLFLT *z )
{
    free( (void *) x );
    free( (void *) y );
    free( (void *) z );
}
