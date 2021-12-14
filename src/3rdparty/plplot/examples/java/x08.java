//--------------------------------------------------------------------------
// Copyright (C) 2001  Geoffrey Furnish
// Copyright (C) 2001-2014 Alan W. Irwin
// Copyright (C) 2002  Maurice LeBrun
// Copyright (C) 2002  Joao Cardoso
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
// Implementation of PLplot example 8 in Java.
//--------------------------------------------------------------------------

package plplot.examples;

import plplot.core.*;
import static plplot.core.plplotjavacConstants.*;

import java.lang.Math;

class x08 {
    // These values must be odd, for the middle
    // of the index range to be an integer, and thus
    // to correspond to the exact floating point centre
    // of the sombrero.


    static final int XPTS = 35;
    static final int YPTS = 45;

    PLStream         pls = new PLStream();

    static double    alt[] = { 60.0, 40.0 };
    static double    az[]  = { 30.0, -30.0 };

    static           String[] title =
    {
        "#frPLplot Example 8 - Alt=60, Az=30",
        "#frPLplot Example 8 - Alt=40, Az=-30",
    };
    // cmap1_init1

    // Initializes color map 1 in HLS space.
    // Basic grayscale variation from half-dark (which makes more interesting
    // looking plot compared to dark) to light.
    // An interesting variation on this:
    // s[1] = 1.0

    void cmap1_init( int gray )
    {
        double [] i    = new double[2];
        double [] h    = new double[2];
        double [] l    = new double[2];
        double [] s    = new double[2];
        boolean [] rev = new boolean[2];

        i[0] = 0.0;         // left boundary
        i[1] = 1.0;         // right boundary

        if ( gray == 1 )
        {
            h[0] = 0.0;        // hue -- low: red (arbitrary if s=0)
            h[1] = 0.0;        // hue -- high: red (arbitrary if s=0)

            l[0] = 0.5;        // lightness -- low: half-dark
            l[1] = 1.0;        // lightness -- high: light

            s[0] = 0.0;        // minimum saturation
            s[1] = 0.0;        // minimum saturation
        }
        else
        {
            h[0] = 240; // blue -> green -> yellow ->
            h[1] = 0;   // -> red

            l[0] = 0.6;
            l[1] = 0.6;

            s[0] = 0.8;
            s[1] = 0.8;
        }

        rev[0] = false;         // interpolate on front side of colour wheel.
        rev[1] = false;         // interpolate on front side of colour wheel.

        pls.scmap1n( 256 );
        pls.scmap1l( false, i, h, l, s, rev );
    }

// Does a series of 3-d plots for a given data set, with different viewing
// options in each plot.

    public static void main( String[] args )
    {
        new x08( args );
    }

    public x08( String[] args )
    {
        int       i, j, k;
        final int LEVELS = 10;

        double[] x   = new double[ XPTS ];
        double[] y   = new double[ YPTS ];
        double[][] z = new double[XPTS][YPTS];
        double clevel[]     = new double[LEVELS];
        double clev_null [] = new double[0];

        double xx, yy, r;
        double zmin = Double.MAX_VALUE, zmax = Double.MIN_VALUE;

        int    ifshade;

        int    indexxmin = 0;
        int    indexxmax = XPTS;
        int[] indexymin     = new int[XPTS];
        int[] indexymax     = new int[XPTS];
        double[][] zlimited = new double[XPTS][YPTS];

        double dx = 2. / ( XPTS - 1 );
        double dy = 2. / ( YPTS - 1 );

        // parameters of ellipse (in x, y index coordinates) that limits the data.
        // x0, y0 correspond to the exact floating point centre of the index
        // range.
        double x0 = 0.5 * ( XPTS - 1 );
        double a  = 0.9 * x0;
        double y0 = 0.5 * ( YPTS - 1 );
        double b  = 0.7 * y0;
        double square_root;

        // Parse and process command line arguments.

        pls.parseopts( args, PL_PARSE_FULL | PL_PARSE_NOPROGRAM );
        boolean rosen = false;

        // Initialize plplot.

        pls.init();

        for ( i = 0; i < XPTS; i++ )
        {
            x[i] = -1. + dx * i;
            if ( rosen )
                x[i] *= 1.5;
        }

        for ( j = 0; j < YPTS; j++ )
        {
            y[j] = -1. + dy * j;
            if ( rosen )
                y[j] += 0.5;
        }

        for ( i = 0; i < XPTS; i++ )
        {
            xx = x[i];
            for ( j = 0; j < YPTS; j++ )
            {
                yy = y[j];
                if ( rosen )
                {
                    z[i][j] = Math.pow( 1. - xx, 2. ) + 100 *
                              Math.pow( yy - Math.pow( xx, 2. ), 2. );
                    // The log argument might be zero for just the right grid.
                    if ( z[i][j] > 0. )
                        z[i][j] = Math.log( z[i][j] );
                    else
                        z[i][j] = -5.; // -MAXFLOAT would mess-up up the scale
                }
                else
                {
                    r       = Math.sqrt( xx * xx + yy * yy );
                    z[i][j] = Math.exp( -r * r ) * Math.cos( 2.0 * Math.PI * r );
                }
                if ( zmin > z[i][j] )
                    zmin = z[i][j];
                if ( zmax < z[i][j] )
                    zmax = z[i][j];
            }
        }

        for ( i = indexxmin; i < indexxmax; i++ )
        {
            square_root = Math.sqrt( 1. - Math.min( 1., Math.pow( ( i - x0 ) / a, 2. ) ) );
            // Add 0.5 to find nearest integer and therefore preserve symmetry
            // with regard to lower and upper bound of y range.
            indexymin[i] = Math.max( 0, (int) ( 0.5 + y0 - b * square_root ) );
            // indexymax calculated with the convention that it is 1
            // greater than highest valid index.
            indexymax[i] = Math.min( YPTS, 1 + (int) ( 0.5 + y0 + b * square_root ) );
            for ( j = indexymin[i]; j < indexymax[i]; j++ )
                zlimited[i][j] = z[i][j];
        }

        double step = ( zmax - zmin ) / ( LEVELS + 1 );
        for ( i = 0; i < LEVELS; i++ )
            clevel[i] = zmin + step * ( i + 1 );

        pls.lightsource( 1., 1., 1. );
        for ( k = 0; k < 2; k++ )
        {
            for ( ifshade = 0; ifshade < 5; ifshade++ )
            {
                pls.adv( 0 );
                pls.vpor( 0.0, 1.0, 0.0, 0.9 );
                pls.wind( -1.0, 1.0, -0.9, 1.1 );
                pls.col0( 3 );
                pls.mtex( "t", 1.0, 0.5, 0.5, title[k] );
                pls.col0( 1 );
                if ( rosen )
                    pls.w3d( 1.0, 1.0, 1.0, -1.5, 1.5, -0.5, 1.5, zmin, zmax,
                        alt[k], az[k] );
                else
                    pls.w3d( 1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, zmin, zmax,
                        alt[k], az[k] );
                pls.box3( "bnstu", "x axis", 0.0, 0,
                    "bnstu", "y axis", 0.0, 0,
                    "bcdmnstuv", "z axis", 0.0, 0 );

                pls.col0( 2 );

                switch ( ifshade )
                {
                case 0: // diffuse light surface plot
                    cmap1_init( 1 );
                    pls.surf3d( x, y, z, 0, clev_null );
                    break;
                case 1: // magnitude colored plot
                    cmap1_init( 0 );
                    pls.surf3d( x, y, z, MAG_COLOR, clev_null );
                    break;
                case 2: //  magnitude colored plot with faceted squares
                    cmap1_init( 0 );
                    pls.surf3d( x, y, z, MAG_COLOR | FACETED, clev_null );
                    break;
                case 3: // magnitude colored plot with contours
                    cmap1_init( 0 );
                    pls.surf3d( x, y, z, MAG_COLOR | SURF_CONT | BASE_CONT, clevel );
                    break;
                case 4: // magnitude colored plot with contours and index limits.
                    cmap1_init( 0 );
                    pls.surf3dl( x, y, zlimited, MAG_COLOR | SURF_CONT | BASE_CONT, clevel, indexxmin, indexymin, indexymax );
                }
            }
        }

        pls.end();
    }
}

//--------------------------------------------------------------------------
//                              End of x08.java
//--------------------------------------------------------------------------
