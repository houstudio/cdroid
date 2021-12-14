//      Demo of multiple stream/window capability (requires Tk or Tcl-DP).
//
//      Maurice LeBrun
//      IFS, University of Texas at Austin
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
import std.stdio;

import plplot;

//--------------------------------------------------------------------------
// main
//
// Plots several simple functions from other example programs.
//
// This version sends the output of the first 4 plots (one page) to two
// independent streams.
//--------------------------------------------------------------------------
int main( char[][] args )
{
    // Select either TK or DP driver and use a small window
    // Using DP results in a crash at the end due to some odd cleanup problems
    // The geometry strings MUST be in writable memory
    string geometry_master = "500x410+100+200";
    string geometry_slave  = "500x410+650+200";

    // plplot initialization
    // Parse and process command line arguments
    plparseopts( args, PL_PARSE_FULL );

    // If valid geometry specified on command line, use it for both streams.
    PLFLT xp0, yp0;
    PLINT xleng0, yleng0, xoff0, yoff0;
    plgpage( &xp0, &yp0, &xleng0, &yleng0, &xoff0, &yoff0 );
    bool  valid_geometry = ( xleng0 > 0 && yleng0 > 0 );

    // Set up first stream
    if ( valid_geometry )
        plspage( xp0, yp0, xleng0, yleng0, xoff0, yoff0 );
    else
        plsetopt( "geometry", geometry_master );

    plssub( 2, 2 );
    plinit();

    string driver;
    plgdev( driver );

    PLINT fam, num, bmax;
    plgfam( &fam, &num, &bmax );

    writefln( "Demo of multiple output streams via the %s driver.", driver );
    writefln( "Running with the second stream as slave to the first.\n" );

    // Start next stream
    plsstrm( 1 );

    if ( valid_geometry )
        plspage( xp0, yp0, xleng0, yleng0, xoff0, yoff0 );
    else
        plsetopt( "geometry", geometry_slave );

    // Turn off pause to make this a slave (must follow master)
    plspause( 0 );
    plsdev( driver );
    plsfam( fam, num, bmax );

    // Currently number of digits in format number can only be
    // set via the command line option
    plsetopt( "fflen", "2" );
    plinit();

    // Set up the data & plot
    // Original case
    plsstrm( 0 );

    plot myPlot = new plot;
    myPlot.plot1( 6, 1, 0, 0 );

    // Set up the data & plot
    myPlot.plot1( 1, 1e6, 0, 0 );

    // Set up the data & plot
    int digmax = 2;
    plsyax( digmax, 0 );
    myPlot.plot1( 1, 1e-6, 0, 0 );

    // Set up the data & plot
    digmax = 5;
    plsyax( digmax, 0 );
    myPlot.plot1( 1, 0.0014, 0, 0.0185 );

    // To slave
    // The pleop() ensures the eop indicator gets lit.
    plsstrm( 1 );
    myPlot.plot4();
    pleop();

    // Back to master
    plsstrm( 0 );
    myPlot.plot2();
    myPlot.plot3();

    // To slave
    plsstrm( 1 );
    myPlot.plot5();
    pleop();

    // Back to master to wait for user to advance
    plsstrm( 0 );
    pleop();

    // Call plend to finish off.
    plend();
    return 0;
}


// special variables for plot5() and mypltr
const int    XPTS = 35;
const int    YPTS = 46;
const double XSPA = 2.0 / ( XPTS - 1 );
const double YSPA = 2.0 / ( YPTS - 1 );

// Transformation function
extern ( C ) {
PLFLT[] tr = [ XSPA, 0.0, -1.0, 0.0, YSPA, -1.0 ];

void mypltr( PLFLT x, PLFLT y, PLFLT* tx, PLFLT* ty, void* pltr_data )
{
    *tx = tr[0] * x + tr[1] * y + tr[2];
    *ty = tr[3] * x + tr[4] * y + tr[5];
}
}

class plot {
    private PLFLT[] x, y, x0, y0;
    private         PLFLT[6] xs, ys;
    private         PLINT[1] space1 = [ 1500 ], mark1 = [ 1500 ];

    public void plot1( PLFLT xscale, PLFLT yscale, PLFLT xoff, PLFLT yoff )
    {
        x.length = 60;
        y.length = 60;
        for ( int i = 0; i < 60; i++ )
        {
            x[i] = xoff + xscale * ( i + 1 ) / 60.0;
            y[i] = yoff + yscale * pow( x[i], 2. );
        }

        PLFLT xmin = x[0];
        PLFLT xmax = x[59];
        PLFLT ymin = y[0];
        PLFLT ymax = y[59];

        for ( int i = 0; i < xs.length; i++ )
        {
            xs[i] = x[i * 10 + 3];
            ys[i] = y[i * 10 + 3];
        }

        // Set up the viewport and window using PLENV. The range in X is
        // 0.0 to 6.0, and the range in Y is 0.0 to 30.0. The axes are
        // scaled separately (just = 0), and we just draw a labelled
        // box (axis = 0).
        plcol0( 1 );
        plenv( xmin, xmax, ymin, ymax, 0, 0 );
        plcol0( 6 );
        pllab( "(x)", "(y)", "#frPLplot Example 1 - y=x#u2" );

        // Plot the data points
        plcol0( 9 );
        plpoin( xs, ys, 9 );

        // Draw the line through the data
        plcol0( 4 );
        plline( x, y );
        plflush();
    }

    public void plot2()
    {
        // Set up the viewport and window using PLENV. The range in X is -2.0 to
        // 10.0, and the range in Y is -0.4 to 2.0. The axes are scaled separately
        // (just = 0), and we draw a box with axes (axis = 1).
        plcol0( 1 );
        plenv( -2.0, 10.0, -0.4, 1.2, 0, 1 );
        plcol0( 2 );
        pllab( "(x)", "sin(x)/x", "#frPLplot Example 1 - Sinc Function" );

        // Fill up the arrays
        x.length = 100;
        y.length = 100;
        for ( int i = 0; i < 100; i++ )
        {
            x[i] = ( i - 19.0 ) / 6.0;
            y[i] = 1.0;
            if ( x[i] != 0.0 )
                y[i] = sin( x[i] ) / x[i];
        }

        // Draw the line
        plcol0( 3 );
        plline( x, y );
        plflush();
    }

    public void plot3()
    {
        // For the final graph we wish to override the default tick intervals, and
        // so do not use PLENV
        pladv( 0 );

        // Use standard viewport, and define X range from 0 to 360 degrees, Y range
        // from -1.2 to 1.2.
        plvsta();
        plwind( 0.0, 360.0, -1.2, 1.2 );

        // Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y.
        plcol0( 1 );
        plbox( "bcnst", 60.0, 2, "bcnstv", 0.2, 2 );

        // Superimpose a dashed line grid, with 1.5 mm marks and spaces. plstyl
        // expects a pointer!!
        plstyl( mark1, space1 );
        plcol0( 2 );
        plbox( "g", 30.0, 0, "g", 0.2, 0 );
        plstyl( null, null );

        plcol0( 3 );
        pllab( "Angle (degrees)", "sine", "#frPLplot Example 1 - Sine function" );

        x.length = 101;
        y.length = 101;
        for ( int i = 0; i < 101; i++ )
        {
            x[i] = 3.6 * i;
            y[i] = sin( x[i] * PI / 180.0 );
        }

        plcol0( 4 );
        plline( x, y );
        plflush();
    }

    public void plot4()
    {
        string text;

        x0.length = 361;
        y0.length = 361;
        PLFLT dtr = PI / 180.0;
        for ( int i = 0; i < 361; i++ )
        {
            x0[i] = cos( dtr * i );
            y0[i] = sin( dtr * i );
        }

        // Set up viewport and window, but do not draw box
        plenv( -1.3, 1.3, -1.3, 1.3, 1, -2 );

        x.length = 361;
        y.length = 361;
        for ( int i = 1; i <= 10; i++ )
        {
            for ( int j = 0; j < 361; j++ )
            {
                x[j] = 0.1 * i * x0[j];
                y[j] = 0.1 * i * y0[j];
            }

            // Draw circles for polar grid
            plline( x, y );
        }

        plcol0( 2 );
        for ( int i = 0; i <= 11; i++ )
        {
            PLFLT theta = 30.0 * i;
            PLFLT dx    = cos( dtr * theta );
            PLFLT dy    = sin( dtr * theta );

            // Draw radial spokes for polar grid
            pljoin( 0.0, 0.0, dx, dy );
            text = format( "%d", lrint( theta ) );

            // Write labels for angle

            // Slightly off zero to avoid floating point logic flips at 90 and 270 deg.
            if ( dx >= -0.00001 )
                plptex( dx, dy, dx, dy, -0.15, text );
            else
                plptex( dx, dy, -dx, -dy, 1.15, text );
        }

        // Draw the graph

        for ( int i = 0; i < 361; i++ )
        {
            PLFLT r = sin( dtr * ( 5 * i ) );
            x[i] = x0[i] * r;
            y[i] = y0[i] * r;
        }
        plcol0( 3 );
        plline( x, y );

        plcol0( 4 );
        plmtex( "t", 2.0, 0.5, 0.5, "#frPLplot Example 3 - r(#gh)=sin 5#gh" );
        plflush();
    }

    // ===============================================================
    // Demonstration of contour plotting
    public void plot5()
    {
        PLFLT[] clevel = [ -1., -.8, -.6, -.4, -.2, 0, .2, .4, .6, .8, 1. ];

        // Set up function arrays
        PLFLT[][] z, w;
        z.length = XPTS;
        w.length = XPTS;
        for ( int i = 0; i < XPTS; i++ )
        {
            PLFLT xx = cast(double) ( i - ( XPTS / 2 ) ) / ( XPTS / 2 );
            z[i].length = YPTS;
            w[i].length = YPTS;
            for ( int j = 0; j < YPTS; j++ )
            {
                PLFLT yy = cast(double) ( j - ( YPTS / 2 ) ) / ( YPTS / 2 ) - 1.0;
                z[i][j] = xx * xx - yy * yy;
                w[i][j] = 2 * xx * yy;
            }
        }

        plenv( -1.0, 1.0, -1.0, 1.0, 0, 0 );
        plcol0( 2 );
        plcont( z, 1, XPTS, 1, YPTS, clevel, &mypltr );
        plstyl( mark1, space1 );
        plcol0( 3 );
        plcont( w, 1, XPTS, 1, YPTS, clevel, &mypltr );
        plcol0( 1 );
        pllab( "X Coordinate", "Y Coordinate", "Streamlines of flow" );
        plflush();
    }
}
