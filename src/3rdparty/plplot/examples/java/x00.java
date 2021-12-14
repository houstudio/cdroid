// Simple demo of a 2D line plot.
//
// Copyright (C) 2011-2014  Alan W. Irwin
// Copyright (C) 2012  Andrew Ross
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

package plplot.examples;

import plplot.core.*;
import static plplot.core.plplotjavacConstants.*;

class x00 {
    PLStream   pls = new PLStream();

    static int NSIZE = 101;

    public static void main( String[] args )
    {
        new x00( args );
    }

    public x00( String[] args )
    {
        double x[]  = new double[NSIZE];
        double y[]  = new double[NSIZE];
        double xmin = 0., xmax = 1., ymin = 0., ymax = 100.;
        int    i;

        // Prepare data to be plotted.
        for ( i = 0; i < NSIZE; i++ )
        {
            x[i] = (double) ( i ) / (double) ( NSIZE - 1 );
            y[i] = ymax * x[i] * x[i];
        }

        // Parse and process command line arguments
        pls.parseopts( args, PL_PARSE_FULL | PL_PARSE_NOPROGRAM );
        // Initialize plplot
        pls.init();

        // Create a labelled box to hold the plot.
        pls.env( xmin, xmax, ymin, ymax, 0, 0 );
        pls.lab( "x", "y=100 x#u2#d", "Simple PLplot demo of a 2D line plot" );

        // Plot the data that was prepared above.
        pls.line( x, y );

        // Close PLplot library
        pls.end();
    }
}


//--------------------------------------------------------------------------
//                              End of x00.java
//--------------------------------------------------------------------------
