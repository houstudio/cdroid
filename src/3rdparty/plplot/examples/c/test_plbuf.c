// Plot buffer test program.  Adapted from example x01
//
// Copyright (C) 2004  Rafael Laboissiere
// Copyright (C) 2015  Jim Dishaw
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

#include "plplotP.h"
#include "plcdemos.h"
#include "plevent.h"
#ifdef PL_HAVE_NANOSLEEP
#include <time.h>
#endif
#ifdef PL_HAVE_UNISTD_H
# include <unistd.h>
#endif
#include "plstrm.h"

// Which test device to use.  The svg device is the best choice, however,
// I (jrd) find it easier to debug with psc.  YMMV.
#define TEST_DEVICE    "psc"

// Reach into the guts of PLPlot to get access to the current stream.
// Not recommended behavior for user program.  Only needed for testing.
extern PLDLLIMPEXP_DATA( PLStream * ) plsc;

// Variables and data arrays used by plot generators

static PLGraphicsIn gin;

// Function prototypes

void plot1( PLFLT xscale, PLFLT yscale, PLFLT xoff, PLFLT yoff );
void plot2( void );
void plot3( void );

//--------------------------------------------------------------------------
// main
//
// Generates several simple line plots.  Demonstrates:
//   - subwindow capability
//   - setting up the window, drawing plot, and labelling
//   - changing the color
//   - automatic axis rescaling to exponential notation
//   - placing the axes in the middle of the box
//   - gridded coordinate axes
//--------------------------------------------------------------------------

int
main( int argc, char *argv[] )
{
    PLINT cur_strm, new_strm;
    char  ver[80];

    // plplot initialization

    // Set the output filename
    plsfnam( "test_plbuf_0.out" );

    // Parse and process command line arguments

    plparseopts( &argc, argv, PL_PARSE_FULL );

    // Force the plot buffer on.  Typically the plot buffer is only used
    // by interactive drivers and the plmeta driver.  Must do this before
    // plot initialization occurs otherwise commands will be missed
    // by the interactive drivers and the plmeta driver
    plsc->plbuf_write = 1;

    // Initialize plplot
    // Divide page into 2x2 plots
    // Note: calling plstart replaces separate calls to plssub and plinit
    plstart( TEST_DEVICE, 2, 2 );

    // Generate the plot for the first subwindow
    plot1( 6.0, 1.0, 0.0, 0.0 );

    // Set the y-axis to 5 digits maximum and generate the plot for
    // the second subwindow
    plsyax( 5, 0 );
    plot1( 1.0, 0.0014, 0.0, 0.0185 );

    // Generate a plot for the third subwindow
    plot2();

    // Generate a plot for the fourth subwindow
    plot3();

    // Replay the plot buffer
    plgstrm( &cur_strm );          // get current stream
    plmkstrm( &new_strm );         // create a new one

    plsfnam( "test_plbuf_1.out" ); // file name
    plsdev( TEST_DEVICE );         // device type

    plcpstrm( cur_strm, 0 );       // copy old stream parameters to new stream
    plreplot();                    // do the save by replaying the plot buffer
    plend1();                      // finish the device

    plsstrm( cur_strm );           // return to previous stream
    plend1();                      // and end the first plot stream

    // Start fresh and use the plmeta driver
    plsfnam( "test_plbuf_0.plm" );
    plstart( "plmeta", 2, 2 );

    // Generate the same plots as before

    // Generate the plot for the first subwindow
    plot1( 6.0, 1.0, 0.0, 0.0 );

    // Set the y-axis to 5 digits maximum and generate the plot for
    // the second subwindow
    plsyax( 5, 0 );
    plot1( 1.0, 0.0014, 0.0, 0.0185 );

    // Generate the plot for the third subwindow
    plot2();

    // Generate the plot for the fourth subwindow
    plot3();

    // Finish this stream
    plend1();

    // Test reading of PLplot metafiles
    plsfnam( "test_plbuf_2.out" );
    plsdev( TEST_DEVICE );
    //plstart( TEST_DEVICE, 2, 2 );
    plreadmetafile( "test_plbuf_0.plm" );

    // Don't forget to call plend() to finish off!
    plend();
    exit( 0 );
}

//--------------------------------------------------------------------------

void
plot1( PLFLT xscale, PLFLT yscale, PLFLT xoff, PLFLT yoff )
{
    static PLFLT x[101], y[101];
    static PLFLT xs[6], ys[6];
    int          i;
    PLFLT        xmin, xmax, ymin, ymax;

    for ( i = 0; i < 60; i++ )
    {
        x[i] = xoff + xscale * ( i + 1 ) / 60.0;
        y[i] = yoff + yscale * pow( x[i], 2. );
    }

    xmin = x[0];
    xmax = x[59];
    ymin = y[0];
    ymax = y[59];

    for ( i = 0; i < 6; i++ )
    {
        xs[i] = x[i * 10 + 3];
        ys[i] = y[i * 10 + 3];
    }

// Set up the viewport and window using PLENV. The range in X is
// 0.0 to 6.0, and the range in Y is 0.0 to 30.0. The axes are
// scaled separately (just = 0), and we just draw a labelled
// box (axis = 0).
//
    plcol0( 1 );
    plenv( xmin, xmax, ymin, ymax, 0, 0 );
    plcol0( 2 );
    pllab( "(x)", "(y)", "#frPLplot Example 1 - y=x#u2" );

// Plot the data points

    plcol0( 4 );
    plpoin( 6, xs, ys, 9 );

// Draw the line through the data

    plcol0( 3 );
    plline( 60, x, y );
}

//--------------------------------------------------------------------------

void
plot2( void )
{
    static PLFLT x[101], y[101];
    static PLFLT xs[6], ys[6];
    int          i;

// Set up the viewport and window using PLENV. The range in X is -2.0 to
// 10.0, and the range in Y is -0.4 to 2.0. The axes are scaled separately
// (just = 0), and we draw a box with axes (axis = 1).
//
    plcol0( 1 );
    plenv( -2.0, 10.0, -0.4, 1.2, 0, 1 );
    plcol0( 2 );
    pllab( "(x)", "sin(x)/x", "#frPLplot Example 1 - Sinc Function" );

// Fill up the arrays

    for ( i = 0; i < 100; i++ )
    {
        x[i] = ( i - 19.0 ) / 6.0;
        y[i] = 1.0;
        if ( x[i] != 0.0 )
            y[i] = sin( x[i] ) / x[i];
    }

// Draw the line

    plcol0( 3 );
    plwidth( 2 );
    plline( 100, x, y );
    plwidth( 1 );
}

//--------------------------------------------------------------------------

void
plot3( void )
{
    static PLFLT x[101], y[101];
    static PLFLT xs[6], ys[6];
    PLINT        space0 = 0, mark0 = 0, space1 = 1500, mark1 = 1500;
    int          i;

// For the final graph we wish to override the default tick intervals, and
// so do not use plenv().
//
    pladv( 0 );

// Use standard viewport, and define X range from 0 to 360 degrees, Y range
// from -1.2 to 1.2.
//
    plvsta();
    plwind( 0.0, 360.0, -1.2, 1.2 );

// Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y.

    plcol0( 1 );
    plbox( "bcnst", 60.0, 2, "bcnstv", 0.2, 2 );

// Superimpose a dashed line grid, with 1.5 mm marks and spaces.
// plstyl expects a pointer!
//
    plstyl( 1, &mark1, &space1 );
    plcol0( 2 );
    plbox( "g", 30.0, 0, "g", 0.2, 0 );
    plstyl( 0, &mark0, &space0 );

    plcol0( 3 );
    pllab( "Angle (degrees)", "sine", "#frPLplot Example 1 - Sine function" );

    for ( i = 0; i < 101; i++ )
    {
        x[i] = 3.6 * i;
        y[i] = sin( x[i] * M_PI / 180.0 );
    }

    plcol0( 4 );
    plline( 101, x, y );
}
