//--------------------------------------------------------------------------
// Drawing "spirograph" curves - epitrochoids, cycolids, roulettes
//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
// Copyright (C) 2007  Arjen Markus
// Copyright (C) 2008  Andrew Ross
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
// Implementation of PLplot example 27 in C++.
//--------------------------------------------------------------------------

#include "plc++demos.h"

#ifdef PL_USE_NAMESPACE
using namespace std;
#endif

class x27 {
public:
    x27( int, char ** );
    void cycloid( void );
    void spiro( PLFLT data[], int fill );
    PLINT gcd( PLINT a, PLINT b );
    void arcs();

private:
    // Class data
    plstream *pls;
};


//--------------------------------------------------------------------------
// Generates two kinds of plots:
//   - construction of a cycloid (animated)
//   - series of epitrochoids and hypotrochoids
//--------------------------------------------------------------------------

x27::x27( int argc, char ** argv )
{
    // R, r, p, N
    // R and r should be integers to give correct termination of the
    // angle loop using gcd.
    // N.B. N is just a place holder since it is no longer used
    // (because we now have proper termination of the angle loop).
    PLFLT params[9][4] = {
        { 21.0,   7.0,  7.0,  3.0 }, // Deltoid
        { 21.0,   7.0, 10.0,  3.0 },
        { 21.0,  -7.0, 10.0,  3.0 },
        { 20.0,   3.0,  7.0, 20.0 },
        { 20.0,   3.0, 10.0, 20.0 },
        { 20.0,  -3.0, 10.0, 20.0 },
        { 20.0,  13.0,  7.0, 20.0 },
        { 20.0,  13.0, 20.0, 20.0 },
        { 20.0, -13.0, 20.0, 20.0 }
    };

    int   i;
    int   fill;

    // plplot initialization
    pls = new plstream();

    // Parse and process command line arguments
    pls->parseopts( &argc, argv, PL_PARSE_FULL );

    // Initialize plplot
    pls->init();

    // Illustrate the construction of a cycloid
    cycloid();

    // Loop over the various curves
    // First an overview, then all curves one by one
    pls->ssub( 3, 3 ); // Three by three window

    fill = 0;
    for ( i = 0; i < 9; i++ )
    {
        pls->adv( 0 );
        pls->vpor( 0.0, 1.0, 0.0, 1.0 );
        spiro( &params[i][0], fill );
    }

    pls->adv( 0 );
    pls->ssub( 1, 1 ); // One window per curve

    for ( i = 0; i < 9; i++ )
    {
        pls->adv( 0 );
        pls->vpor( 0.0, 1.0, 0.0, 1.0 );
        spiro( &params[i][0], fill );
    }

// Fill the curves
    fill = 1;

    pls->adv( 0 );
    pls->ssub( 1, 1 ); // One window per curve

    for ( i = 0; i < 9; i++ )
    {
        pls->adv( 0 );
        pls->vpor( 0.0, 1.0, 0.0, 1.0 );
        spiro( &params[i][0], fill );
    }

    // Finally, an example to test out plarc capabilities

    arcs();


    delete pls;
}

//--------------------------------------------------------------------------
// Calculate greatest common divisor following pseudo-code for the
// Euclidian algorithm at http://en.wikipedia.org/wiki/Euclidean_algorithm

PLINT x27::gcd( PLINT a, PLINT b )
{
    PLINT t;
    a = abs( a );
    b = abs( b );
    while ( b != 0 )
    {
        t = b;
        b = a % b;
        a = t;
    }
    return a;
}

//--------------------------------------------------------------------------

void
x27::cycloid( void )
{
    // TODO
}

//--------------------------------------------------------------------------

void
x27::spiro( PLFLT params[], int fill )
{
#define NPNT    2000
    static PLFLT xcoord[NPNT + 1];
    static PLFLT ycoord[NPNT + 1];

    int          windings;
    int          steps;
    int          i;
    PLFLT        phi;
    PLFLT        phiw;
    PLFLT        dphi;
    PLFLT        xmin = 0.0;
    PLFLT        xmax = 0.0;
    PLFLT        xrange_adjust;
    PLFLT        ymin = 0.0;
    PLFLT        ymax = 0.0;
    PLFLT        yrange_adjust;

    // Fill the coordinates

    // Proper termination of the angle loop very near the beginning
    // point, see
    // http://mathforum.org/mathimages/index.php/Hypotrochoid.
    windings = (PLINT) abs( params[1] ) / gcd( (PLINT) params[0], (PLINT) params[1] );
    steps    = NPNT / windings;
    dphi     = 2.0 * M_PI / (PLFLT) steps;

    for ( i = 0; i <= windings * steps; i++ )
    {
        phi       = (PLFLT) i * dphi;
        phiw      = ( params[0] - params[1] ) / params[1] * phi;
        xcoord[i] = ( params[0] - params[1] ) * cos( phi ) + params[2] * cos( phiw );
        ycoord[i] = ( params[0] - params[1] ) * sin( phi ) - params[2] * sin( phiw );

        if ( i == 0 )
        {
            xmin = xcoord[i];
            xmax = xcoord[i];
            ymin = ycoord[i];
            ymax = ycoord[i];
        }
        if ( xmin > xcoord[i] )
            xmin = xcoord[i];
        if ( xmax < xcoord[i] )
            xmax = xcoord[i];
        if ( ymin > ycoord[i] )
            ymin = ycoord[i];
        if ( ymax < ycoord[i] )
            ymax = ycoord[i];
    }

    xrange_adjust = 0.15 * ( xmax - xmin );
    xmin         -= xrange_adjust;
    xmax         += xrange_adjust;
    yrange_adjust = 0.15 * ( ymax - ymin );
    ymin         -= yrange_adjust;
    ymax         += yrange_adjust;

    pls->wind( xmin, xmax, ymin, ymax );

    pls->col0( 1 );
    if ( fill )
    {
        pls->fill( 1 + steps * windings, xcoord, ycoord );
    }
    else
    {
        pls->line( 1 + steps * windings, xcoord, ycoord );
    }
}

void
x27::arcs()
{
#define NSEG    8
    int   i;
    PLFLT theta, dtheta;
    PLFLT a, b;

    theta  = 0.0;
    dtheta = 360.0 / NSEG;
    pls->env( -10.0, 10.0, -10.0, 10.0, 1, 0 );

    // Plot segments of circle in different colors
    for ( i = 0; i < NSEG; i++ )
    {
        pls->col0( i % 2 + 1 );
        pls->arc( 0.0, 0.0, 8.0, 8.0, theta, theta + dtheta, 0.0, 0 );
        theta = theta + dtheta;
    }

    // Draw several filled ellipses inside the circle at different
    // angles.
    a     = 3.0;
    b     = a * tan( ( dtheta / 180.0 * M_PI ) / 2.0 );
    theta = dtheta / 2.0;
    for ( i = 0; i < NSEG; i++ )
    {
        pls->col0( 2 - i % 2 );
        pls->arc( a * cos( theta / 180.0 * M_PI ), a * sin( theta / 180.0 * M_PI ), a, b, 0.0, 360.0, theta, 1 );
        theta = theta + dtheta;
    }
}


int main( int argc, char ** argv )
{
    x27 *x = new x27( argc, argv );

    delete x;
}


//--------------------------------------------------------------------------
//                              End of x27.cc
//--------------------------------------------------------------------------
