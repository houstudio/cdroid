//--------------------------------------------------------------------------
// Copyright (C) 2004  Andrew Ross
// Copyright (C) 2004  Alan W. Irwin
//
// This file is part of PLplot.
//
// PLplot is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; version 2 of the License.
//
// PLplot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with PLplot; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
// Implementation of PLplot example 21 in C++.
//--------------------------------------------------------------------------
#include "plc++demos.h"

// Need for some Mac OSX systems with broken <cmath> header
#ifdef PL_BROKEN_ISNAN_CXX
extern "C" int isnan( PLFLT );
#endif

#ifdef PL_USE_NAMESPACE
using namespace std;
#endif

class x21 {
public:
    x21( int, char ** );

private:
    void create_data( PLFLT **xi, PLFLT **yi, PLFLT **zi, PLINT pts );
    void free_data( PLFLT *x, PLFLT *y, PLFLT *z );
    void create_grid( PLFLT **xi, PLINT px, PLFLT **yi, PLINT py );
    void free_grid( PLFLT *x, PLFLT *y );
    void cmap1_init();

    PLFLT MIN( PLFLT x, PLFLT y ) { return ( x < y ? x : y ); };
    PLFLT MAX( PLFLT x, PLFLT y ) { return ( x > y ? x : y ); };

private:
    // Class data
    plstream *pls;

    PLFLT    xm, xM, ym, yM;

    // Options data structure definition.
    static PLINT         pts;
    static PLINT         xp;
    static PLINT         yp;
    static PLINT         nl;
    static PLINT         knn_order;
    static PLFLT         threshold;
    static PLFLT         wmin;
    static PLINT         randn;
    static PLINT         rosen;
    static PLOptionTable options[];
};


PLINT         x21::        pts       = 500;
PLINT         x21::        xp        = 25;
PLINT         x21::        yp        = 20;
PLINT         x21::        nl        = 16;
PLINT         x21::        knn_order = 20;
PLFLT         x21::        threshold = 1.001;
PLFLT         x21::        wmin      = -1e3;
PLINT         x21::        randn     = 0;
PLINT         x21::        rosen     = 0;

PLOptionTable x21::options[] = {
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

x21::x21( int argc, char ** argv )
{
    PLFLT      *x, *y, *z, *clev;
    PLFLT      *xg, *yg, **zg;
    PLFLT      zmin, zmax, lzm, lzM;
    int        i, j, k;
    PLINT      alg;
    const char *title[] = { "Cubic Spline Approximation",
                            "Delaunay Linear Interpolation",
                            "Natural Neighbors Interpolation",
                            "KNN Inv. Distance Weighted",
                            "3NN Linear Interpolation",
                            "4NN Around Inv. Dist. Weighted" };

    PLFLT      opt[] = { 0., 0., 0., 0., 0., 0. };

    xm = ym = -0.2;
    xM = yM = 0.6;

    // plplot initialization

    pls = new plstream();

    cmap1_init();

    pls->seed( 5489 );

    // Parse and process command line arguments.
    pls->MergeOpts( options, "x21c options", NULL );
    pls->parseopts( &argc, argv, PL_PARSE_FULL );

    opt[2] = wmin;
    opt[3] = (PLFLT) knn_order;
    opt[4] = threshold;

    // Initialize PLplot.
    pls->init();

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
    pls->Alloc2dGrid( &zg, xp, yp ); // the output grided data
    clev = new PLFLT[nl];

    pls->col0( 1 );
    pls->env( xm, xM, ym, yM, 2, 0 );
    pls->col0( 15 );
    pls->lab( "X", "Y", "The original data sampling" );
    for ( i = 0; i < pts; i++ )
    {
        pls->col1( ( z[i] - zmin ) / ( zmax - zmin ) );
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
        pls->string( 1, &x[i], &y[i], "#(727)" );
    }
    pls->adv( 0 );

    pls->ssub( 3, 2 );

    for ( k = 0; k < 2; k++ )
    {
        pls->adv( 0 );
        for ( alg = 1; alg < 7; alg++ )
        {
            pls->griddata( x, y, z, pts, xg, xp, yg, yp, zg, alg, opt[alg - 1] );

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

            pls->MinMax2dGrid( zg, xp, yp, &lzM, &lzm );

            lzm = MIN( lzm, zmin );
            lzM = MAX( lzM, zmax );

            lzm = lzm - 0.01;
            lzM = lzM + 0.01;

            pls->col0( 1 );
            pls->adv( alg );

            if ( k == 0 )
            {
                for ( i = 0; i < nl; i++ )
                    clev[i] = lzm + ( lzM - lzm ) / ( nl - 1 ) * i;

                pls->env0( xm, xM, ym, yM, 2, 0 );
                pls->col0( 15 );
                pls->lab( "X", "Y", title[alg - 1] );
                pls->shades( zg, xp, yp, NULL, xm, xM, ym, yM,
                    clev, nl, 1.0, 0, 1.0, plcallback::fill, true, NULL, NULL );
                pls->col0( 2 );
            }
            else
            {
                for ( i = 0; i < nl; i++ )
                    clev[i] = lzm + ( lzM - lzm ) / ( nl - 1 ) * i;

                pls->vpor( 0.0, 1.0, 0.0, 0.9 );
                pls->wind( -1.1, 0.75, -0.65, 1.20 );
                //
                // For the comparition to be fair, all plots should have the
                // same z values, but to get the max/min of the data generated
                // by all algorithms would imply two passes. Keep it simple.
                //
                // plw3d(1., 1., 1., xm, xM, ym, yM, zmin, zmax, 30, -60);
                //

                pls->w3d( 1., 1., 1., xm, xM, ym, yM, lzm, lzM, 30., -40. );
                pls->box3( "bntu", "X", 0.0, 0,
                    "bntu", "Y", 0.0, 0,
                    "bcdfntu", "Z", 0.5, 0 );
                pls->col0( 15 );
                pls->lab( "", "", title[alg - 1] );
                pls->plot3dc( xg, yg, zg, xp, yp, DRAW_LINEXY | MAG_COLOR | BASE_CONT, clev, nl );
            }
        }
    }

    free_data( x, y, z );
    free_grid( xg, yg );
    delete[] clev;
    pls->Free2dGrid( zg, xp, yp );

    delete pls;
}

void x21::cmap1_init()
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

    pls->scmap1n( 256 );
    pls->scmap1l( false, 2, i, h, l, s );
}

void x21::create_grid( PLFLT **xi, PLINT px, PLFLT **yi, PLINT py )
{
    PLFLT *x, *y;
    int   i;

    x = *xi = new PLFLT[px];
    y = *yi = new PLFLT[py];

    for ( i = 0; i < px; i++ )
        *x++ = xm + ( xM - xm ) * i / ( px - 1. );

    for ( i = 0; i < py; i++ )
        *y++ = ym + ( yM - ym ) * i / ( py - 1. );
}

void x21::free_grid( PLFLT *xi, PLFLT *yi )
{
    delete[] xi;
    delete[] yi;
}

void x21::create_data( PLFLT **xi, PLFLT **yi, PLFLT **zi, PLINT pts )
{
    int   i;
    PLFLT *x, *y, *z, r;
    PLFLT xt, yt;

    *xi = x = new PLFLT[pts];
    *yi = y = new PLFLT[pts];
    *zi = z = new PLFLT[pts];

    for ( i = 0; i < pts; i++ )
    {
        xt = ( xM - xm ) * pls->randd();
        yt = ( yM - ym ) * pls->randd();
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
            *z = log( pow( ( 1. - *x ), 2. ) + 100. * pow( ( *y - pow( *x, 2. ) ), 2. ) );
        }
        x++; y++; z++;
    }
}

void x21::free_data( PLFLT *x, PLFLT *y, PLFLT *z )
{
    delete[] x;
    delete[] y;
    delete[] z;
}

int main( int argc, char ** argv )
{
    x21 *x = new x21( argc, argv );

    delete x;
}


//--------------------------------------------------------------------------
//                              End of x21.cc
//--------------------------------------------------------------------------


