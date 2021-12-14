//      Grid data demo
//
// Copyright (C) 2009  Werner Smekal
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

import std.math;

import plplot;

// Options data structure definition.
PLINT pts       = 500;
PLINT xp        = 25;
PLINT yp        = 20;
PLINT nl        = 16;
int   knn_order = 20;
PLFLT threshold = 1.001;
PLFLT wmin      = -1e3;
int   randn     = 0;
int   rosen     = 0;


PLFLT xm, xM, ym, yM;

int main( char[][] args )
{
    string[] title = [ "Cubic Spline Approximation",
                       "Delaunay Linear Interpolation",
                       "Natural Neighbors Interpolation",
                       "KNN Inv. Distance Weighted",
                       "3NN Linear Interpolation",
                       "4NN Around Inv. Dist. Weighted" ];

    xm = ym = -0.2;
    xM = yM = 0.6;

    // plMergeOpts(options, "x21c options", NULL);
    plparseopts( args, PL_PARSE_FULL );

    PLFLT[] opt = [ 0.0, 0.0, wmin, knn_order, threshold, 0.0 ];

    // Initialize plplot
    plinit();

    cmap1_init();

    // Initialise random number generator
    plseed( 5489 );

    PLFLT[] x, y, z;
    x.length = y.length = z.length = pts;
    create_data( x, y, z ); // the sampled data
    PLFLT zmin = z[0];
    PLFLT zmax = z[0];
    for ( int i = 1; i < pts; i++ )
    {
        if ( z[i] > zmax )
            zmax = z[i];
        if ( z[i] < zmin )
            zmin = z[i];
    }

    PLFLT[] xg, yg;
    xg.length = xp;
    yg.length = yp;
    create_grid( xg, yg ); // grid the data at

    PLFLT[][] zg = new PLFLT[][xp];
    for ( int i = 0; i < xp; i++ )
        zg[i] = new PLFLT[yp];

    PLFLT[] clev = new PLFLT[nl];

    PLFLT[] xx = new PLFLT[1];
    PLFLT[] yy = new PLFLT[1];

    plcol0( 1 );
    plenv( xm, xM, ym, yM, 2, 0 );
    plcol0( 15 );
    pllab( "X", "Y", "The original data sampling" );
    for ( int i = 0; i < pts; i++ )
    {
        plcol1( ( z[i] - zmin ) / ( zmax - zmin ) );
        xx[0] = x[i];
        yy[0] = y[i];
        plstring( xx, yy, "#(727)" );
    }
    pladv( 0 );

    plssub( 3, 2 );

    for ( int k = 0; k < 2; k++ )
    {
        pladv( 0 );
        for ( int alg = 1; alg < 7; alg++ )
        {
            plgriddata( x, y, z, xg, yg, zg, alg, opt[alg - 1] );

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
                PLFLT dist, d;

                for ( int i = 0; i < xp; i++ )
                {
                    for ( int j = 0; j < yp; j++ )
                    {
                        if ( isNaN( zg[i][j] ) )                       // average (IDW) over the 8 neighbors
                        {
                            zg[i][j] = 0.0;
                            dist     = 0.0;

                            for ( int ii = i - 1; ii <= i + 1 && ii < xp; ii++ )
                            {
                                for ( int jj = j - 1; jj <= j + 1 && jj < yp; jj++ )
                                {
                                    if ( ii >= 0 && jj >= 0 && !isNaN( zg[ii][jj] ) )
                                    {
                                        d         = ( abs( ii - i ) + abs( jj - j ) ) == 1 ? 1.0 : 1.4142;
                                        zg[i][j] += zg[ii][jj] / ( d * d );
                                        dist     += d;
                                    }
                                }
                            }
                            if ( dist != 0.0 )
                                zg[i][j] /= dist;
                            else
                                zg[i][j] = zmin;
                        }
                    }
                }
            }

            PLFLT lzM, lzm;
            plMinMax2dGrid( zg, lzM, lzm );

            lzm = fmin( lzm, zmin );
            lzM = fmax( lzM, zmax );

            // Increase limits slightly to prevent spurious contours
            // due to rounding errors
            lzm = lzm - 0.01;
            lzM = lzM + 0.01;

            plcol0( 1 );

            pladv( alg );

            if ( k == 0 )
            {
                for ( int i = 0; i < nl; i++ )
                    clev[i] = lzm + ( lzM - lzm ) / ( nl - 1 ) * i;

                plenv0( xm, xM, ym, yM, 2, 0 );
                plcol0( 15 );
                pllab( "X", "Y", title[alg - 1] );
                plshades( zg, null, xm, xM, ym, yM, clev, 1, 0, 1, 1 );
                plcol0( 2 );
            }
            else
            {
                for ( int i = 0; i < nl; i++ )
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
                plot3dc( xg, yg, zg, DRAW_LINEXY | MAG_COLOR | BASE_CONT, clev );
            }
        }
    }

    plend();

    return 0;
}


void create_grid( PLFLT[] x, PLFLT[] y )
{
    int px = cast(int) x.length;
    int py = cast(int) y.length;

    for ( int i = 0; i < px; i++ )
        x[i] = xm + ( xM - xm ) * i / ( px - 1.0 );

    for ( int i = 0; i < py; i++ )
        y[i] = ym + ( yM - ym ) * i / ( py - 1.0 );
}


void create_data( PLFLT[] x, PLFLT[] y, PLFLT[] z )
{
    int pts = cast(int) x.length;
    assert( pts == y.length, "create_data(): Arrays must be of same length" );
    assert( pts == z.length, "create_data(): Arrays must be of same length" );

    PLFLT xt, yt, r;
    for ( int i = 0; i < pts; i++ )
    {
        xt = ( xM - xm ) * plrandd();
        yt = ( yM - ym ) * plrandd();
        if ( !randn )
        {
            x[i] = xt + xm;
            y[i] = yt + ym;
        }
        else // std=1, meaning that many points are outside the plot range
        {
            x[i] = sqrt( -2.0 * log( xt ) ) * cos( 2. * PI * yt ) + xm;
            y[i] = sqrt( -2.0 * log( xt ) ) * sin( 2. * PI * yt ) + ym;
        }
        if ( !rosen )
        {
            r    = sqrt( x[i] * x[i] + y[i] * y[i] );
            z[i] = exp( -r * r ) * cos( 2.0 * PI * r );
        }
        else
            z[i] = log( pow( 1. - x[i], 2.9 ) + 100.0 * pow( y[i] - pow( x[i], 2.0 ), 2.0 ) );
    }
}


void cmap1_init()
{
    PLFLT[] i = [ 0.0, 1.0 ];           // boundaries

    PLFLT[] h = [ 240.0, 0.0 ];         // blue -> green -> yellow -> red
    PLFLT[] l = [ 0.6, 0.6 ];
    PLFLT[] s = [ 0.8, 0.8 ];

    plscmap1n( 256 );
    plscmap1l( 0, i, h, l, s );
}
