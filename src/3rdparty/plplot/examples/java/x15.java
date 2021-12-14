//--------------------------------------------------------------------------
// Copyright (C) 2001, 2002  Geoffrey Furnish
// Copyright (C) 2002-2014 Alan W. Irwin
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
// Implementation of PLplot example 15 in Java.
//--------------------------------------------------------------------------

package plplot.examples;

import plplot.core.*;
import static plplot.core.plplotjavacConstants.*;

class x15 {
    static final int    XPTS = 35;
    static final int    YPTS = 46;
    // calculated constants and array that depends on them
    static final double XSPA = 2. / ( XPTS - 1 );
    static final double YSPA = 2. / ( YPTS - 1 );
    final double        tr[] = { XSPA, 0.0, -1.0, 0.0, YSPA, -1.0 };


    PLStream pls = new PLStream();

    double   fmin, fmax;

// Compute min and max value of a 2-d array.

    void f2mnmx( double[][] f, int nx, int ny )
    {
        fmax = f[0][0];
        fmin = fmax;

        for ( int i = 0; i < nx; i++ )
            for ( int j = 0; j < ny; j++ )
            {
                if ( f[i][j] < fmin ) fmin = f[i][j];
                if ( f[i][j] > fmax ) fmax = f[i][j];
            }
    }

// Does a variety of shade plots with discrete colours and patterns.

    public static void main( String[] args )
    {
        new x15( args );
    }

    public x15( String[] args )
    {
        int    i, j;

        double xx;
        double yy;
        double x;
        double y;
        double[][] z = new double[XPTS][YPTS];
        double zmin, zmax;
        double[][] xg0 = new double[XPTS][YPTS];
        double[][] yg0 = new double[XPTS][YPTS];

        // Parse and process command line arguments.

        pls.parseopts( args, PL_PARSE_FULL | PL_PARSE_NOPROGRAM );

        // Initialize plplot.

        pls.init();

        // no need to fiddle with color maps since example uses default cmap0
        // and the cmap1 fiddling that x15c.c does is completely irrelevant
        // (although interesting).

        for ( i = 0; i < XPTS; i++ )
        {
            xx = (double) ( i - ( XPTS / 2 ) ) / (double) ( XPTS / 2 );
            for ( j = 0; j < YPTS; j++ )
            {
                yy      = ( (double) ( j - ( YPTS / 2 ) ) / (double) ( YPTS / 2 ) ) - 1.;
                z[i][j] = xx * xx - yy * yy + ( xx - yy ) / ( xx * xx + yy * yy + 0.1 );
                x       = tr[0] * i + tr[1] * j + tr[2];
                y       = tr[3] * i + tr[4] * j + tr[5];
                // Note xg0 ==> yg1 are one-dimensional because of arrangement of
                //zeros in the final tr definition above.  However, for now
                //we are using raw interface here so must nominally treat them
                //as two-dimensional.
                xg0[i][j] = x;
                yg0[i][j] = y;
            }
        }

        f2mnmx( z, XPTS, YPTS );
        zmin = fmin;
        zmax = fmax;

        plot1( xg0, yg0, z, zmin, zmax );
        plot2( xg0, yg0, z, zmin, zmax );
        plot3();

        pls.end();
    }

// Illustrates a single shaded region.

    void plot1( double[][] xg0, double[][] yg0, double[][] z, double zmin, double zmax )
    {
        double shade_min, shade_max, sh_color;
        int    sh_cmap   = 0;
        int    min_color = 0, max_color = 0;
        double sh_width, min_width = 0., max_width = 0.;

        pls.adv( 0 );
        pls.vpor( 0.1, 0.9, 0.1, 0.9 );
        pls.wind( -1.0, 1.0, -1.0, 1.0 );

        // Plot using identity transform

        shade_min = zmin + ( zmax - zmin ) * 0.4;
        shade_max = zmin + ( zmax - zmin ) * 0.6;
        sh_color  = 7;
        sh_width  = 2.;
        min_color = 9;
        max_color = 2;
        min_width = 2.;
        max_width = 2.;

        pls.psty( 8 );

        pls.shade( z, -1., 1., -1., 1.,
            shade_min, shade_max,
            sh_cmap, sh_color, sh_width,
            min_color, min_width, max_color, max_width,
            true, xg0, yg0 );

        pls.col0( 1 );
        pls.box( "bcnst", 0.0, 0, "bcnstv", 0.0, 0 );
        pls.col0( 2 );
        pls.lab( "distance", "altitude", "Bogon flux" );
    }

// Illustrates multiple adjacent shaded regions, using different fill
// patterns for each region.

    void plot2( double[][] xg0, double[][] yg0, double[][] z, double zmin, double zmax )
    {
        double shade_min, shade_max, sh_color;
        int    sh_cmap   = 0;
        int    min_color = 0, max_color = 0;
        double sh_width, min_width = 0., max_width = 0.;
        int    i;
        int[][] inc = { {  450 }, { -450 }, {   0 }, { 900 }, { 300 },
                        {450, -450 }, { 0, 900 }, { 0, 450 },
                        {450, -450 }, { 0, 900 } };
        int[][] del = { { 2000 }, { 2000 }, { 2000 }, { 2000 }, { 2000 },
                        {2000, 2000 }, { 2000, 2000 }, { 2000, 2000 },
                        {4000, 4000 }, { 4000, 2000 } };
        sh_width = 2.;

        pls.adv( 0 );
        pls.vpor( 0.1, 0.9, 0.1, 0.9 );
        pls.wind( -1.0, 1.0, -1.0, 1.0 );

        // Plot using identity transform

        for ( i = 0; i < 10; i++ )
        {
            shade_min = zmin + ( zmax - zmin ) * i / 10.0;
            shade_max = zmin + ( zmax - zmin ) * ( i + 1 ) / 10.0;
            sh_color  = i + 6;
            pls.pat( inc[i], del[i] );

            pls.shade( z, -1., 1., -1., 1.,
                shade_min, shade_max,
                sh_cmap, sh_color, sh_width,
                min_color, min_width, max_color, max_width,
                true, xg0, yg0 );
        }

        pls.col0( 1 );
        pls.box( "bcnst", 0.0, 0, "bcnstv", 0.0, 0 );
        pls.col0( 2 );
        pls.lab( "distance", "altitude", "Bogon flux" );
    }

// Illustrates shaded regions in 3d, using a different fill pattern for
// each region.

    void plot3()
    {
        double[][] xx = { { -1.0, 1.0, 1.0, -1.0, -1.0 },
                          { -1.0, 1.0, 1.0, -1.0, -1.0 } };
        double[][] yy = { {  1.0,  1.0, 0.0, 0.0,  1.0 },
                          { -1.0, -1.0, 0.0, 0.0, -1.0 } };
        double[][] zz = { { 0.0, 0.0, 1.0, 1.0, 0.0 },
                          { 0.0, 0.0, 1.0, 1.0, 0.0 } };

        pls.adv( 0 );
        pls.vpor( 0.1, 0.9, 0.1, 0.9 );
        pls.wind( -1.0, 1.0, -1.0, 1.0 );
        pls.w3d( 1., 1., 1., -1.0, 1.0, -1.0, 1.0, 0.0, 1.5, 30, -40 );

        // Plot using identity transform

        pls.col0( 1 );
        pls.box3( "bntu", "X", 0.0, 0, "bntu", "Y", 0.0, 0, "bcdfntu", "Z", 0.5, 0 );
        pls.col0( 2 );
        pls.lab( "", "", "3-d polygon filling" );

        pls.col0( 3 );
        pls.psty( 1 );
        pls.line3( xx[0], yy[0], zz[0] );
        pls.fill3( xx[0], yy[0], zz[0] );
        pls.psty( 2 );
        pls.line3( xx[1], yy[1], zz[1] );
        pls.fill3( xx[1], yy[1], zz[1] );
    }
}

//--------------------------------------------------------------------------
//                              End of x15.java
//--------------------------------------------------------------------------
