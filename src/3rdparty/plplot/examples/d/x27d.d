//      Drawing "spirograph" curves - epitrochoids, cycolids, roulettes
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

import std.string;
import std.math;

import plplot;

//--------------------------------------------------------------------------
// main
//
// Generates two kinds of plots:
//   - construction of a cycloid (animated)
//   - series of epitrochoids and hypotrochoids
//--------------------------------------------------------------------------

int main( char[][] args )
{
    // R, r, p, N
    // R and r should be integers to give correct termination of the
    // angle loop using gcd.
    // N.B. N is just a place holder since it is no longer used
    // (because we now have proper termination of the angle loop).
    static PLFLT[4][9] params = [
        [ 21.0, 7.0, 7.0, 3.0 ], // Deltoid
        [ 21.0, 7.0, 10.0, 3.0 ],
        [ 21.0, -7.0, 10.0, 3.0 ],
        [ 20.0, 3.0, 7.0, 20.0 ],
        [ 20.0, 3.0, 10.0, 20.0 ],
        [ 20.0, -3.0, 10.0, 20.0 ],
        [ 20.0, 13.0, 7.0, 20.0 ],
        [ 20.0, 13.0, 20.0, 20.0 ],
        [ 20.0, -13.0, 20.0, 20.0 ]
    ];

    // plplot initialization

    // Parse and process command line arguments
    plparseopts( args, PL_PARSE_FULL );

    // Initialize plplot
    plinit();

    // Illustrate the construction of a cycloid
    cycloid();

    // Loop over the various curves
    // First an overview, then all curves one by one
    //
    plssub( 3, 3 ); // Three by three window

    int fill = 0;
    for ( int i = 0; i < 9; i++ )
    {
        pladv( 0 );
        plvpor( 0.0, 1.0, 0.0, 1.0 );
        spiro( params[i], fill );
    }

    pladv( 0 );
    plssub( 1, 1 ); // One window per curve

    for ( int i = 0; i < 9; i++ )
    {
        pladv( 0 );
        plvpor( 0.0, 1.0, 0.0, 1.0 );
        spiro( params[i], fill );
    }

    // fill the curves
    fill = 1;

    pladv( 0 );
    plssub( 1, 1 ); // One window per curve

    for ( int i = 0; i < 9; i++ )
    {
        pladv( 0 );
        plvpor( 0.0, 1.0, 0.0, 1.0 );
        spiro( params[i], fill );
    }

    // Finally, an example to test out plarc capabilities
    arcs();

    plend();
    return 0;
}

//--------------------------------------------------------------------------
// Calculate greatest common divisor following pseudo-code for the
// Euclidian algorithm at http://en.wikipedia.org/wiki/Euclidean_algorithm

PLINT gcd( PLINT a, PLINT b )
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

void cycloid()
{
    // TODO
}

//--------------------------------------------------------------------------
void spiro( PLFLT[] params, int fill )
{
    const int npnt = 2000;
    PLFLT[]   xcoord, ycoord;
    int       windings, steps;
    PLFLT     dphi, phi, phiw;
    PLFLT     xmin, xmax, xrange_adjust;
    PLFLT     ymin, ymax, yrange_adjust;

    // Fill the coordinates

    // Proper termination of the angle loop very near the beginning
    // point, see
    // http://mathforum.org/mathimages/index.php/Hypotrochoid.
    windings = cast(PLINT) abs( params[1] ) / gcd( cast(PLINT) params[0], cast(PLINT) params[1] );
    steps    = npnt / windings;
    dphi     = 2.0 * PI / cast(PLFLT) steps;

    xcoord.length = windings * steps + 1;
    ycoord.length = windings * steps + 1;
    for ( int i = 0; i <= windings * steps; i++ )
    {
        phi       = i * dphi;
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
    xmin          = xmin - xrange_adjust;
    xmax          = xmax + xrange_adjust;
    yrange_adjust = 0.15 * ( ymax - ymin );
    ymin          = ymin - yrange_adjust;
    ymax          = ymax + yrange_adjust;

    plwind( xmin, xmax, ymin, ymax );

    plcol0( 1 );
    if ( fill )
    {
        plfill( xcoord, ycoord );
    }
    else
    {
        plline( xcoord, ycoord );
    }
}

void arcs()
{
    const int NSEG = 8;
    int       i;
    PLFLT     theta, dtheta;
    PLFLT     a, b;

    theta  = 0.0;
    dtheta = 360.0 / NSEG;
    plenv( -10.0, 10.0, -10.0, 10.0, 1, 0 );

    // Plot segments of circle in different colors
    for ( i = 0; i < NSEG; i++ )
    {
        plcol0( i % 2 + 1 );
        plarc( 0.0, 0.0, 8.0, 8.0, theta, theta + dtheta, 0.0, 0 );
        theta = theta + dtheta;
    }

    // Draw several filled ellipses inside the circle at different
    // angles.
    a     = 3.0;
    b     = a * tan( ( dtheta / 180.0 * PI ) / 2.0 );
    theta = dtheta / 2.0;
    for ( i = 0; i < NSEG; i++ )
    {
        plcol0( 2 - i % 2 );
        plarc( a * cos( theta / 180.0 * PI ), a * sin( theta / 180.0 * PI ), a, b, 0.0, 360.0, theta, 1 );
        theta = theta + dtheta;
    }
}
