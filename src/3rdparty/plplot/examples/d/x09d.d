//      Contour plot demo.
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

import std.string;
import std.math;
import plplot;

const int XPTS = 35;              // Data points in x
const int YPTS = 46;              // Data points in y

// Transformation function
extern ( C ) {
PLFLT[] tr = [ 2. / ( XPTS - 1 ), 0.0, -1.0, 0.0, 2. / ( YPTS - 1 ), -1.0 ];

void mypltr( PLFLT x, PLFLT y, PLFLT* tx, PLFLT* ty, void* pltr_data )
{
    *tx = tr[0] * x + tr[1] * y + tr[2];
    *ty = tr[3] * x + tr[4] * y + tr[5];
}
}

//polar contour plot example.
void polar()
{
    const int PERIMETERPTS = 100;
    const int RPTS         = 40;
    const int THETAPTS     = 40;

    plenv( -1., 1., -1., 1., 0, -2 );
    plcol0( 1 );

    //Perimeter
    PLFLT[] px, py;
    px.length = PERIMETERPTS;
    py.length = PERIMETERPTS;
    PLFLT t;
    for ( int i = 0; i < PERIMETERPTS; i++ )
    {
        t     = 2. * PI / ( PERIMETERPTS - 1 ) * i;
        px[i] = cos( t );
        py[i] = sin( t );
    }
    plline( px, py );

    //create data to be contoured.
    PLcGrid2 cgrid2;
    cgrid2.xg = new PLFLT[][RPTS];
    for ( int i = 0; i < RPTS; i++ )
        cgrid2.xg[i] = new PLFLT[THETAPTS];
    cgrid2.yg = new PLFLT[][RPTS];
    for ( int i = 0; i < RPTS; i++ )
        cgrid2.yg[i] = new PLFLT[THETAPTS];

    PLFLT[][] z = new PLFLT[][RPTS];
    for ( int i = 0; i < RPTS; i++ )
        z[i] = new PLFLT[THETAPTS];

    PLFLT r, theta;
    for ( int i = 0; i < RPTS; i++ )
    {
        r = i / cast(double) ( RPTS - 1 );
        for ( int j = 0; j < THETAPTS; j++ )
        {
            theta           = 2. * PI / ( THETAPTS - 1 ) * j;
            cgrid2.xg[i][j] = r * cos( theta );
            cgrid2.yg[i][j] = r * sin( theta );
            z[i][j]         = r;
        }
    }

    PLFLT[] lev;
    lev.length = 10;
    for ( int i = 0; i < 10; i++ )
        lev[i] = 0.05 + 0.10 * i;

    plcol0( 2 );
    plcont( z, 1, RPTS, 1, THETAPTS, lev, cgrid2 );
    plcol0( 1 );
    pllab( "", "", "Polar Contour Plot" );
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


//shielded potential contour plot example.
void potential()
{
    const int PERIMETERPTS = 100;
    const int RPTS         = 40;
    const int THETAPTS     = 64;
    const int NLEVEL       = 20;

    //create data to be contoured.
    PLcGrid2 cgrid2;
    cgrid2.xg = new PLFLT[][RPTS];
    for ( int i = 0; i < RPTS; i++ )
        cgrid2.xg[i] = new PLFLT[THETAPTS];
    cgrid2.yg = new PLFLT[][RPTS];
    for ( int i = 0; i < RPTS; i++ )
        cgrid2.yg[i] = new PLFLT[THETAPTS];

    PLFLT[][] z = new PLFLT[][RPTS];
    for ( int i = 0; i < RPTS; i++ )
        z[i] = new PLFLT[THETAPTS];

    PLFLT r, theta;
    for ( int i = 0; i < RPTS; i++ )
    {
        r = 0.5 + i;
        for ( int j = 0; j < THETAPTS; j++ )
        {
            theta           = 2. * PI / ( THETAPTS - 1 ) * ( 0.5 + j );
            cgrid2.xg[i][j] = r * cos( theta );
            cgrid2.yg[i][j] = r * sin( theta );
        }
    }
    PLFLT rmax = r;

    PLFLT xmin, xmax, ymin, ymax;
    f2mnmx( cgrid2.xg, xmin, xmax );
    f2mnmx( cgrid2.yg, ymin, ymax );
    PLFLT x0 = ( xmin + xmax ) / 2.;
    PLFLT y0 = ( ymin + ymax ) / 2.;

    // Expanded limits
    PLFLT peps  = 0.05;
    PLFLT xpmin = xmin - fabs( xmin ) * peps;
    PLFLT xpmax = xmax + fabs( xmax ) * peps;
    PLFLT ypmin = ymin - fabs( ymin ) * peps;
    PLFLT ypmax = ymax + fabs( ymax ) * peps;

    // Potential inside a conducting cylinder (or sphere) by method of images.
    // Charge 1 is placed at (d1, d1), with image charge at (d2, d2).
    // Charge 2 is placed at (d1, -d1), with image charge at (d2, -d2).
    // Also put in smoothing term at small distances.
    //
    PLFLT eps = 2.;

    PLFLT q1 = 1.;
    PLFLT d1 = rmax / 4.;

    PLFLT q1i = -q1 * rmax / d1;
    PLFLT d1i = pow( rmax, 2. ) / d1;

    PLFLT q2 = -1.;
    PLFLT d2 = rmax / 4.;

    PLFLT q2i = -q2 * rmax / d2;
    PLFLT d2i = pow( rmax, 2. ) / d2;

    PLFLT div1, div1i, div2, div2i;
    for ( int i = 0; i < RPTS; i++ )
    {
        for ( int j = 0; j < THETAPTS; j++ )
        {
            div1    = sqrt( pow( cgrid2.xg[i][j] - d1, 2. ) + pow( cgrid2.yg[i][j] - d1, 2. ) + pow( eps, 2. ) );
            div1i   = sqrt( pow( cgrid2.xg[i][j] - d1i, 2. ) + pow( cgrid2.yg[i][j] - d1i, 2. ) + pow( eps, 2. ) );
            div2    = sqrt( pow( cgrid2.xg[i][j] - d2, 2. ) + pow( cgrid2.yg[i][j] + d2, 2. ) + pow( eps, 2. ) );
            div2i   = sqrt( pow( cgrid2.xg[i][j] - d2i, 2. ) + pow( cgrid2.yg[i][j] + d2i, 2. ) + pow( eps, 2. ) );
            z[i][j] = q1 / div1 + q1i / div1i + q2 / div2 + q2i / div2i;
        }
    }
    PLFLT zmin, zmax;
    f2mnmx( z, zmin, zmax );

    // Positive and negative contour levels.
    PLFLT   dz = ( zmax - zmin ) / NLEVEL;
    PLFLT[] clevelneg, clevelpos;
    PLFLT   clevel;
    clevelneg.length = NLEVEL;
    clevelpos.length = NLEVEL;
    int nlevelneg = 0;
    int nlevelpos = 0;
    for ( int i = 0; i < NLEVEL; i++ )
    {
        clevel = zmin + ( i + 0.5 ) * dz;
        if ( clevel <= 0. )
            clevelneg[nlevelneg++] = clevel;
        else
            clevelpos[nlevelpos++] = clevel;
    }
    // Colours!
    PLINT ncollin = 11;
    PLINT ncolbox = 1;
    PLINT ncollab = 2;

    // Finally start plotting this page!
    pladv( 0 );
    plcol0( ncolbox );

    plvpas( 0.1, 0.9, 0.1, 0.9, 1.0 );
    plwind( xpmin, xpmax, ypmin, ypmax );
    plbox( "", 0., 0, "", 0., 0 );

    plcol0( ncollin );
    if ( nlevelneg > 0 )
    {
        // Negative contours
        pllsty( 2 );
        plcont( z, 1, RPTS, 1, THETAPTS, clevelneg, cgrid2 );
    }

    if ( nlevelpos > 0 )
    {
        // Positive contours
        pllsty( 1 );
        plcont( z, 1, RPTS, 1, THETAPTS, clevelpos, cgrid2 );
    }

    // Draw outer boundary
    PLFLT[] px, py;
    px.length = PERIMETERPTS;
    py.length = PERIMETERPTS;
    PLFLT t;
    for ( int i = 0; i < PERIMETERPTS; i++ )
    {
        t     = ( 2. * PI / ( PERIMETERPTS - 1 ) ) * i;
        px[i] = x0 + rmax * cos( t );
        py[i] = y0 + rmax * sin( t );
    }

    plcol0( ncolbox );
    plline( px, py );

    plcol0( ncollab );
    pllab( "", "", "Shielded potential of charges in a conducting sphere" );
}


//--------------------------------------------------------------------------
// main
//
// Does several contour plots using different coordinate mappings.
//--------------------------------------------------------------------------
int main( char[][] args )
{
    PLINT[] mark   = [ 1500 ], space = [ 1500 ];
    PLFLT[] clevel = [ -1., -.8, -.6, -.4, -.2, 0, .2, .4, .6, .8, 1. ];

    // Parse and process command line arguments
    plparseopts( args, PL_PARSE_FULL );

    // Initialize plplot
    plinit();

    // Set up function arrays
    PLFLT[][] z = new PLFLT[][XPTS];
    for ( int i = 0; i < XPTS; i++ )
        z[i] = new PLFLT[YPTS];

    PLFLT[][] w = new PLFLT[][XPTS];
    for ( int i = 0; i < XPTS; i++ )
        w[i] = new PLFLT[YPTS];

    PLFLT xx, yy;
    for ( int i = 0; i < XPTS; i++ )
    {
        xx = cast(double) ( i - ( XPTS / 2 ) ) / ( XPTS / 2 );
        for ( int j = 0; j < YPTS; j++ )
        {
            yy      = cast(double) ( j - ( YPTS / 2 ) ) / ( YPTS / 2 ) - 1.0;
            z[i][j] = xx * xx - yy * yy;
            w[i][j] = 2 * xx * yy;
        }
    }

    // Set up grids
    PLcGrid cgrid1;
    cgrid1.xg = new PLFLT[XPTS];
    cgrid1.yg = new PLFLT[YPTS];

    PLcGrid2 cgrid2;
    cgrid2.xg = new PLFLT[][XPTS];
    for ( int i = 0; i < XPTS; i++ )
        cgrid2.xg[i] = new PLFLT[YPTS];

    cgrid2.yg = new PLFLT[][XPTS];
    for ( int i = 0; i < XPTS; i++ )
        cgrid2.yg[i] = new PLFLT[YPTS];

    PLFLT argx, argy, distort;
    for ( int i = 0; i < XPTS; i++ )
    {
        for ( int j = 0; j < YPTS; j++ )
        {
            mypltr( cast(PLFLT) i, cast(PLFLT) j, &xx, &yy, null );

            argx    = xx * PI / 2;
            argy    = yy * PI / 2;
            distort = 0.4;

            cgrid1.xg[i] = xx + distort * cos( argx );
            cgrid1.yg[j] = yy - distort * cos( argy );

            cgrid2.xg[i][j] = xx + distort * cos( argx ) * cos( argy );
            cgrid2.yg[i][j] = yy - distort * cos( argx ) * cos( argy );
        }
    }

    // Plot using identity transform
    pl_setcontlabelformat( 4, 3 );
    pl_setcontlabelparam( 0.006, 0.3, 0.1, 1 );
    plenv( -1.0, 1.0, -1.0, 1.0, 0, 0 );
    plcol0( 2 );
    plcont( z, 1, XPTS, 1, YPTS, clevel, &mypltr );

    plstyl( mark, space );
    plcol0( 3 );
    plcont( w, 1, XPTS, 1, YPTS, clevel, &mypltr );
    plstyl( null, null );
    plcol0( 1 );
    pllab( "X Coordinate", "Y Coordinate", "Streamlines of flow" );
    pl_setcontlabelparam( 0.006, 0.3, 0.1, 0 );

    // Plot using 1d coordinate transform
    plenv( -1.0, 1.0, -1.0, 1.0, 0, 0 );
    plcol0( 2 );
    plcont( z, 1, XPTS, 1, YPTS, clevel, cgrid1 );

    plstyl( mark, space );
    plcol0( 3 );
    plcont( w, 1, XPTS, 1, YPTS, clevel, cgrid1 );
    plstyl( null, null );
    plcol0( 1 );
    pllab( "X Coordinate", "Y Coordinate", "Streamlines of flow" );

    // Plot using 2d coordinate transform
    plenv( -1.0, 1.0, -1.0, 1.0, 0, 0 );
    plcol0( 2 );
    plcont( z, 1, XPTS, 1, YPTS, clevel, cgrid2 );

    plstyl( mark, space );
    plcol0( 3 );
    plcont( w, 1, XPTS, 1, YPTS, clevel, cgrid2 );
    plstyl( null, null );
    plcol0( 1 );
    pllab( "X Coordinate", "Y Coordinate", "Streamlines of flow" );

    pl_setcontlabelparam( 0.006, 0.3, 0.1, 0 );
    polar();

    pl_setcontlabelparam( 0.006, 0.3, 0.1, 0 );
    potential();

    plend();

    return 0;
}
