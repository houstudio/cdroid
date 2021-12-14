//--------------------------------------------------------------------------
// Copyright (C) 2004  Andrew Ross
// Copyright (C) 2004-2014 Alan W. Irwin
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
// Implementation of PLplot example 17 in Java.
//--------------------------------------------------------------------------

package plplot.examples;

import plplot.core.*;
import static plplot.core.plplotjavacConstants.*;

class x17 {
    // Class data
    PLStream pls = new PLStream();

    // plerror is not implement in java at this time
    static int    pl_errcode = 0;
    static String errmsg     = null;


    x17( String[] args )
    {
        int     id1[] = new int[1];
        int     n, nsteps = 1000;
        boolean autoy, acc;
        double  y1, y2, y3, y4, ymin, ymax, xlab, ylab;
        double  t, tmin, tmax, tjump, dt, noise;
        int     colbox, collab;
        int     colline[] = new int[4];
        int     styline[] = new int[4];
        String  legline[] = new String[4];


        // Parse and process command line arguments.
        pls.parseopts( args, PL_PARSE_FULL | PL_PARSE_NOPROGRAM );

        // If db is used the plot is much more smooth. However, because of the
        // async X behaviour, one does not have a real-time scripcharter.
        // pls.setopt("db", "");
        // pls.setopt("np", "");
        // Specify some reasonable defaults for ymin and ymax
        // The plot will grow automatically if needed (but not shrink)

        ymin = -0.1;
        ymax = 0.1;

        // Specify initial tmin and tmax -- this determines length of window.
        // Also specify maximum jump in t
        // This can accomodate adaptive timesteps

        tmin  = 0.;
        tmax  = 10.;
        tjump = 0.3;        // percentage of plot to jump

        // Axes options same as plbox.
        // Only automatic tick generation and label placement allowed
        // Eventually I'll make this fancier

        colbox     = 1;
        collab     = 3;
        styline[0] = colline[0] = 2;        // pens color and line style
        styline[1] = colline[1] = 3;
        styline[2] = colline[2] = 4;
        styline[3] = colline[3] = 5;

        legline[0] = "sum";                         // pens legend
        legline[1] = "sin";
        legline[2] = "sin*noi";
        legline[3] = "sin+noi";

        xlab = 0.; ylab = 0.25;     // legend position

        autoy = true;               // autoscale y
        acc   = true;               // don't scrip, accumulate

        // Initialize PLplot.
        pls.init();

        pls.adv( 0 );
        pls.vsta();

        // Register our error variables with PLplot
        // From here on, we're handling all errors here

        //pls.sError(&pl_errcode, errmsg);

        pls.stripc( id1, "bcnst", "bcnstv",
            tmin, tmax, tjump, ymin, ymax,
            xlab, ylab,
            autoy, acc,
            colbox, collab,
            colline, styline, legline,
            "t", "", "Strip chart demo" );

        if ( pl_errcode != 0 )
        {
            System.out.println( errmsg );
            pls.end();
            System.exit( 1 );
        }

        // Let plplot handle errors from here on

        //pls.sError(NULL, NULL);

        autoy = false;  // autoscale y
        acc   = true;   // accumulate/

        // This is to represent a loop over time
        // Let's try a random walk process

        y1 = y2 = y3 = y4 = 0.0;
        dt = 0.1;

        for ( n = 0; n < nsteps; n++ )
        {
            try {
                Thread.sleep( 10 );
            }
            catch ( InterruptedException e ) {
            }
            t     = n * dt;
            noise = pls.randd() - 0.5;
            y1    = y1 + noise;
            y2    = Math.sin( t * Math.PI / 18. );
            y3    = y2 * noise;
            y4    = y2 + noise / 3.;

            // There is no need for all pens to have the same number of
            // points or beeing equally time spaced.

            if ( n % 2 != 0 )
                pls.stripa( id1[0], 0, t, y1 );
            if ( n % 3 != 0 )
                pls.stripa( id1[0], 1, t, y2 );
            if ( n % 4 != 0 )
                pls.stripa( id1[0], 2, t, y3 );
            if ( n % 5 != 0 )
                pls.stripa( id1[0], 3, t, y4 );
            // pls.eop();  // use double buffer (-db on command line)
        }

        // Destroy strip chart and it's memory
        pls.stripd( id1[0] );

        pls.end();
    }

    public static void main( String[] args )
    {
        new x17( args );
    }
}


//--------------------------------------------------------------------------
//                              End of x17.java
//--------------------------------------------------------------------------
