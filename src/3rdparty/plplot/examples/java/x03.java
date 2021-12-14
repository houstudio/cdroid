//--------------------------------------------------------------------------
// Copyright (C) 2001  Geoffrey Furnish
// Copyright (C) 2001-2014 Alan W. Irwin
// Copyright (C) 2004  Andrew Ross
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

//--------------------------------------------------------------------------
// Implementation of PLplot example 3 in Java.
//--------------------------------------------------------------------------

package plplot.examples;

import plplot.core.*;
import static plplot.core.plplotjavacConstants.*;

import java.lang.Math;
import java.text.*;

class x03 {
    public static void main( String[] args )
    {
        new x03( args );
    }

    public x03( String[] args )
    {
        PLStream     pls = new PLStream();
        NumberFormat nf  = NumberFormat.getNumberInstance();

        int          i;
        double       dtr, theta, dx, dy, r, offset;

        double[] x0 = new double[361];
        double[] y0 = new double[361];
        double[] x  = new double[361];
        double[] y  = new double[361];

        dtr = Math.PI / 180.0;
        for ( i = 0; i <= 360; i++ )
        {
            x0[i] = Math.cos( dtr * i );
            y0[i] = Math.sin( dtr * i );
        }

        // Parse and process command line arguments.

        pls.parseopts( args, PL_PARSE_FULL | PL_PARSE_NOPROGRAM );

        // Set orientation to portrait - note not all device drivers
        // support this, in particular most interactive drivers do not.
        pls.sori( 1 );

        // Initialize plplot.

        pls.init();

        // Set up viewport and window, but do not draw box.

        pls.env( -1.3, 1.3, -1.3, 1.3, 1, -2 );
        for ( i = 1; i <= 10; i++ )
        {
            pls.arc( 0.0, 0.0, 0.1 * i, 0.1 * i, 0.0, 360.0, 0.0, false );
        }

        pls.col0( 2 );
        for ( i = 0; i <= 11; i++ )
        {
            theta = 30.0 * i;
            dx    = Math.cos( dtr * theta );
            dy    = Math.sin( dtr * theta );

            // Draw radial spokes for polar grid.

            pls.join( 0.0, 0.0, dx, dy );
            String text = nf.format( theta );

            // Write labels for angle.

            if ( theta < 9.99 )
            {
                offset = 0.45;
            }
            else if ( theta < 99.9 )
            {
                offset = 0.30;
            }
            else
            {
                offset = 0.15;
            }

            //Slightly off zero to avoid floating point logic flips at 90 and 270 deg.
            if ( dx >= -0.00001 )
                pls.ptex( dx, dy, dx, dy, -offset, text );
            else
                pls.ptex( dx, dy, -dx, -dy, 1. + offset, text );
        }

        // Draw the graph.

        for ( i = 0; i <= 360; i++ )
        {
            r    = Math.sin( dtr * ( 5 * i ) );
            x[i] = x0[i] * r;
            y[i] = y0[i] * r;
        }
        pls.col0( 3 );
        pls.line( x, y );

        pls.col0( 4 );
        pls.mtex( "t", 2.0, 0.5, 0.5, "#frPLplot Example 3 - r(#gh)=sin 5#gh" );

        // Close the plot at end.

        pls.end();
    }
}

//--------------------------------------------------------------------------
//                              End of x03.java
//--------------------------------------------------------------------------
