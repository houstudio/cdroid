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
// Implementation of PLplot example 27 in Java.
//--------------------------------------------------------------------------

package plplot.examples;

import plplot.core.*;
import static plplot.core.plplotjavacConstants.*;

import java.lang.Math;

class x27 {
    PLStream pls = new PLStream();

    public static void main( String[] args )
    {
        new x27( args );
    }

    //--------------------------------------------------------------------
    // Generates two kinds of plots:
    //   - construction of a cycloid (animated)
    //   - series of epitrochoids and hypotrochoids
    //--------------------------------------------------------------------

    public x27( String[] args )
    {
        // R, r, p, N
        // R and r should be integers to give correct termination of the
        // angle loop using gcd.
        // N.B. N is just a place holder since it is no longer used
        // (because we now have proper termination of the angle loop).
        double  params[][] = {
            { 21.0,   7.0,  7.0,  3.0 }, // Deltoid
            { 21.0,   7.0, 10.0,  3.0 },
            { 21.0,  -7.0, 10.0,  3.0 },
            { 20.0,   3.0,  7.0, 20.0 },
            { 20.0,   3.0, 10.0, 20.0 },
            { 20.0,  -3.0, 10.0, 20.0 },
            { 20.0,  13.0,  7.0, 20.0 },
            { 20.0,  13.0, 20.0, 20.0 },
            { 20.0, -13.0, 20.0, 20.0 } };

        int     i;
        boolean fill;

        // plplot initialization

        // Parse and process command line arguments
        pls.parseopts( args, PL_PARSE_FULL | PL_PARSE_NOPROGRAM );

        // Initialize plplot
        pls.init();

        // Illustrate the construction of a cycloid
        cycloid();

        // Loop over the various curves
        // First an overview, then all curves one by one
        pls.ssub( 3, 3 ); // Three by three window

        fill = false;
        for ( i = 0; i < 9; i++ )
        {
            pls.adv( 0 );
            pls.vpor( 0.0, 1.0, 0.0, 1.0 );
            spiro( params[i], fill );
        }

        pls.adv( 0 );
        pls.ssub( 1, 1 ); // One window per curve

        for ( i = 0; i < 9; i++ )
        {
            pls.adv( 0 );
            pls.vpor( 0.0, 1.0, 0.0, 1.0 );
            spiro( params[i], fill );
        }

        // Fill the curves
        fill = true;

        pls.adv( 0 );
        pls.ssub( 1, 1 ); // One window per curve

        for ( i = 0; i < 9; i++ )
        {
            pls.adv( 0 );
            pls.vpor( 0.0, 1.0, 0.0, 1.0 );
            spiro( params[i], fill );
        }

        // Finally, an example to test out plarc capabilities
        arcs();

        pls.end();
    }

//--------------------------------------------------------------------------
// Calculate greatest common divisor following pseudo-code for the
// Euclidian algorithm at http://en.wikipedia.org/wiki/Euclidean_algorithm

    int gcd( int a, int b )
    {
        int t;
        a = Math.abs( a );
        b = Math.abs( b );
        while ( b != 0 )
        {
            t = b;
            b = a % b;
            a = t;
        }
        return a;
    }

    // ===============================================================

    void cycloid()
    {
        // TODO
    }

    // ===============================================================

    void spiro( double params[], boolean fill )
    {
        int    NPNT = 2000;
        double xcoord[];
        double ycoord[];

        int    windings;
        int    steps;
        int    i;
        double phi;
        double phiw;
        double dphi;
        // Initialize to quiet java compiler errors about the possibility
        // these variables are not initialized in the code below.
        double xmin = 0.;
        double xmax = 0.;
        double ymin = 0.;
        double ymax = 0.;
        double xrange_adjust;
        double yrange_adjust;

        // Fill the coordinates

        // Proper termination of the angle loop very near the beginning
        // point, see
        // http://mathforum.org/mathimages/index.php/Hypotrochoid.
        windings = (int) Math.abs( params[1] ) / gcd( (int) params[0], (int) params[1] );
        steps    = NPNT / windings;
        dphi     = 2.0 * Math.PI / steps;

        xcoord = new double[windings * steps + 1];
        ycoord = new double[windings * steps + 1];

        for ( i = 0; i <= windings * steps; i++ )
        {
            phi       = i * dphi;
            phiw      = ( params[0] - params[1] ) / params[1] * phi;
            xcoord[i] = ( params[0] - params[1] ) * Math.cos( phi ) + params[2] * Math.cos( phiw );
            ycoord[i] = ( params[0] - params[1] ) * Math.sin( phi ) - params[2] * Math.sin( phiw );

            if ( i == 0 )
            {
                xmin = xcoord[i];
                xmax = xcoord[i];
                ymin = ycoord[i];
                ymax = ycoord[i];
            }
            if ( xmin > xcoord[i] ) xmin = xcoord[i];
            if ( xmax < xcoord[i] ) xmax = xcoord[i];
            if ( ymin > ycoord[i] ) ymin = ycoord[i];
            if ( ymax < ycoord[i] ) ymax = ycoord[i];
        }

        xrange_adjust = 0.15 * ( xmax - xmin );
        xmin         -= xrange_adjust;
        xmax         += xrange_adjust;
        yrange_adjust = 0.15 * ( ymax - ymin );
        ymin         -= yrange_adjust;
        ymax         += yrange_adjust;

        pls.wind( xmin, xmax, ymin, ymax );

        pls.col0( 1 );

        if ( fill )
            pls.fill( xcoord, ycoord );
        else
            pls.line( xcoord, ycoord );
    }

    void arcs()
    {
        int    NSEG = 8;
        int    i;
        double theta, dtheta;
        double a, b;

        theta  = 0.0;
        dtheta = 360.0 / NSEG;
        pls.env( -10.0, 10.0, -10.0, 10.0, 1, 0 );

        // Plot segments of circle in different colors
        for ( i = 0; i < NSEG; i++ )
        {
            pls.col0( i % 2 + 1 );
            pls.arc( 0.0, 0.0, 8.0, 8.0, theta, theta + dtheta, 0.0, false );
            theta = theta + dtheta;
        }

        // Draw several filled ellipses inside the circle at different
        // angles.
        a     = 3.0;
        b     = a * Math.tan( ( dtheta / 180.0 * Math.PI ) / 2.0 );
        theta = dtheta / 2.0;
        for ( i = 0; i < NSEG; i++ )
        {
            pls.col0( 2 - i % 2 );
            pls.arc( a * Math.cos( theta / 180.0 * Math.PI ), a * Math.sin( theta / 180.0 * Math.PI ), a, b, 0.0, 360.0, theta, true );
            theta = theta + dtheta;
        }
    }
}

//--------------------------------------------------------------------------
//                              End of x27.java
//--------------------------------------------------------------------------
