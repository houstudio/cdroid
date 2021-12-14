//      Simple line plot and multiple windows demo.
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

import std.math;
import std.stdio;
import std.string;

import plplot;

static int    locate_mode;
static int    test_xor;
static int    fontset = 1;
static string f_name;

// Options data structure definition.
//~ static PLOptionTable[] options[] = {
//~ {
//~ "locate",			/* Turns on test of API locate function */
//~ NULL,
//~ NULL,
//~ &locate_mode,
//~ PL_OPT_BOOL,
//~ "-locate",
//~ "Turns on test of API locate function" },
//~ {
//~ "xor",			/* Turns on test of xor function */
//~ NULL,
//~ NULL,
//~ &test_xor,
//~ PL_OPT_BOOL,
//~ "-xor",
//~ "Turns on test of XOR" },
//~ {
//~ "font",			/* For switching between font set 1 & 2 */
//~ NULL,
//~ NULL,
//~ &fontset,
//~ PL_OPT_INT,
//~ "-font number",
//~ "Selects stroke font set (0 or 1, def:1)" },
//~ {
//~ "save",			/* For saving in postscript */
//~ NULL,
//~ NULL,
//~ &f_name,
//~ PL_OPT_STRING,
//~ "-save filename",
//~ "Save plot in color postscript `file'" },
//~ {
//~ NULL,			/* option */
//~ NULL,			/* handler */
//~ NULL,			/* client data */
//~ NULL,			/* address of variable to set */
//~ 0,				/* mode flag */
//~ NULL,			/* short syntax */
//~ NULL }			/* long syntax */
//~ };

//~ const char *notes[] = {"Make sure you get it right!", NULL};

// Function prototypes

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
int main( char[][] args )
{
    // plplot initialization
    // Divide page into 2x2 plots unless user overrides
    plssub( 2, 2 );

    // Parse and process command line arguments
    //plMergeOpts(options, "x01c options", notes);
    plparseopts( args, PL_PARSE_FULL );

    // Get version number, just for kicks
    string ver;
    plgver( ver );
    writefln( "PLplot library version: %s", ver ); // TODO: to stdout

    // Initialize plplot
    plinit();

    // Select font set as per input flag
    if ( fontset )
        plfontld( 1 );
    else
        plfontld( 0 );

    plot myPlot = new plot;

    // Set up the data & plot
    myPlot.plot1( 6, 1, 0, 0, 0 );

    // Set up the data & plot
    PLINT digmax = 5;
    plsyax( digmax, 0 );
    myPlot.plot1( 1, 0.0014, 0, 0.0185, 1 );

    myPlot.plot2();
    myPlot.plot3();

    //
    // Show how to save a plot:
    // Open a new device, make it current, copy parameters,
    // and replay the plot buffer
    //
    if ( f_name ) // command line option '-save filename'
    {
        PLINT cur_strm, new_strm;

        writefln( "The current plot was saved in color Postscript under the name `%s'.", f_name );
        plgstrm( &cur_strm );    // get current stream
        plmkstrm( &new_strm );   // create a new one

        plsfnam( f_name );       // file name
        plsdev( "psc" );         // device type

        plcpstrm( cur_strm, 0 ); // copy old stream parameters to new stream
        plreplot();              // do the save by replaying the plot buffer
        plend1();                // finish the device

        plsstrm( cur_strm );     // return to previous stream
    }

    // Let's get some user input
    if ( locate_mode )
    {
        PLGraphicsIn gin;

        //for(;;) {
        // if(!plGetCursor(&gin))
        //  break;
        // if(gin.keysym==PLK_Escape)
        //  break;
        // pltext();
        // if(gin.keysym<0xFF && isprint(gin.keysym))
        //  writefln("subwin = %d, wx = %f,  wy = %f, dx = %f,  dy = %f,  c = '%c'",
        //           gin.subwindow, gin.wX, gin.wY, gin.dX, gin.dY, gin.keysym);
        // else
        //  writefln("subwin = %d, wx = %f,  wy = %f, dx = %f,  dy = %f,  c = 0x%02x",
        //           gin.subwindow, gin.wX, gin.wY, gin.dX, gin.dY, gin.keysym);
        // plgra();
        // }
    }

    // Don't forget to call plend() to finish off!
    plend();
    return 0;
}

class plot {
    private PLFLT[] x, y, x0, y0;
    private         PLFLT[6] xs, ys;
    private         PLINT[1] space1 = [ 1500 ], mark1 = [ 1500 ];

    public void plot1( PLFLT xscale, PLFLT yscale, PLFLT xoff, PLFLT yoff, int do_test )
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
        plcol0( 2 );
        pllab( "(x)", "(y)", "#frPLplot Example 1 - y=x#u2" );

        // Plot the data points
        plcol0( 4 );
        plpoin( xs, ys, 9 );

        // Draw the line through the data
        plcol0( 3 );
        plline( x, y );

        // xor mode enable erasing a line/point/text by replotting it again
        // it does not work in double buffering mode, however

        if ( do_test && test_xor )
        {
            writefln( "The -xor command line option can only be exercised if your "
                "system\nhas usleep(), which does not seem to happen." );
        }
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
        plwidth( 2 );
        plline( x, y );
        plflush();
        plwidth( 1 );
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
    }
}