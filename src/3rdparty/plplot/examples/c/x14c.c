//      Demo of multiple stream/window capability (requires Tk or Tcl-DP).
//
//      Maurice LeBrun
//      IFS, University of Texas at Austin
//
// Copyright (C) 2004  Alan W. Irwin
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

#include "plcdemos.h"

static PLFLT x[101], y[101];
static PLFLT xscale, yscale, xoff, yoff, xs[6], ys[6];
static PLINT space0 = 0, mark0 = 0, space1 = 1500, mark1 = 1500;

void plot1( void );
void plot2( void );
void plot3( void );
void plot4( void );
void plot5( void );
void mypltr( PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, void *pltr_data );

//--------------------------------------------------------------------------
// main
//
// Plots several simple functions from other example programs.
//
// This version sends the output of the first 4 plots (one page) to two
// independent streams.
//--------------------------------------------------------------------------

int
main( int argc, char *argv[] )
{
    int digmax;

// Select either TK or DP driver and use a small window
// Using DP results in a crash at the end due to some odd cleanup problems
// The geometry strings MUST be in writable memory

    char  geometry_master[] = "500x410+100+200";
    char  geometry_slave[]  = "500x410+650+200";

    char  driver[80] = "";

    PLINT fam, num, bmax;
    PLFLT xp0, yp0;
    PLINT xleng0, yleng0, xoff0, yoff0;
    int   valid_geometry;

// plplot initialization
// Parse and process command line arguments

    (void) plparseopts( &argc, argv, PL_PARSE_FULL );

    // If valid geometry specified on command line, use it for both streams.
    plgpage( &xp0, &yp0, &xleng0, &yleng0, &xoff0, &yoff0 );
    valid_geometry = ( xleng0 > 0 && yleng0 > 0 );

// Set up first stream

    if ( valid_geometry )
        plspage( xp0, yp0, xleng0, yleng0, xoff0, yoff0 );
    else
        plsetopt( "geometry", geometry_master );

    plssub( 2, 2 );
    plinit();

    plgdev( driver );
    plgfam( &fam, &num, &bmax );

    printf( "Demo of multiple output streams via the %s driver.\n", driver );
    printf( "Running with the second stream as slave to the first.\n" );
    printf( "\n" );

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

    xscale = 6.;
    yscale = 1.;
    xoff   = 0.;
    yoff   = 0.;
    plot1();

// Set up the data & plot

    xscale = 1.;
    yscale = 1.e+6;
    plot1();

// Set up the data & plot

    xscale = 1.;
    yscale = 1.e-6;
    digmax = 2;
    plsyax( digmax, 0 );
    plot1();

// Set up the data & plot

    xscale = 1.;
    yscale = 0.0014;
    yoff   = 0.0185;
    digmax = 5;
    plsyax( digmax, 0 );
    plot1();

// To slave
// The pleop() ensures the eop indicator gets lit.

    plsstrm( 1 );
    plot4();
    pleop();

// Back to master

    plsstrm( 0 );
    plot2();
    plot3();

// To slave

    plsstrm( 1 );
    plot5();
    pleop();

// Back to master to wait for user to advance

    plsstrm( 0 );
    pleop();

// Call plend to finish off.

    plend();
    exit( 0 );
}

//--------------------------------------------------------------------------

void
plot1( void )
{
    int   i;
    PLFLT xmin, xmax, ymin, ymax;

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

    plcol0( 1 );
    plenv( xmin, xmax, ymin, ymax, 0, 0 );
    plcol0( 6 );
    pllab( "(x)", "(y)", "#frPLplot Example 1 - y=x#u2" );

// Plot the data points

    plcol0( 9 );
    plpoin( 6, xs, ys, 9 );

// Draw the line through the data

    plcol0( 4 );
    plline( 60, x, y );
    plflush();
}


//--------------------------------------------------------------------------

void
plot2( void )
{
    int i;

// Set up the viewport and window using PLENV. The range in X is -2.0 to
//     10.0, and the range in Y is -0.4 to 2.0. The axes are scaled separately
//     (just = 0), and we draw a box with axes (axis = 1).

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
    plline( 100, x, y );
    plflush();
}

//--------------------------------------------------------------------------

void
plot3( void )
{
    int i;

// For the final graph we wish to override the default tick intervals, and
//     so do not use PLENV

    pladv( 0 );

// Use standard viewport, and define X range from 0 to 360 degrees, Y range
//     from -1.2 to 1.2.

    plvsta();
    plwind( 0.0, 360.0, -1.2, 1.2 );

    // Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y.

    plcol0( 1 );
    plbox( "bcnst", 60.0, 2, "bcnstv", 0.2, 2 );

    // Superimpose a dashed line grid, with 1.5 mm marks and spaces. plstyl
    // expects a pointer!!

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
    plflush();
}

//--------------------------------------------------------------------------

void
plot4( void )
{
    int   i, j;
    PLFLT dtr, theta, dx, dy, r;
    char  text[4];
    PLFLT x0[361], y0[361];
    PLFLT x1[361], y1[361];

    dtr = M_PI / 180.0;
    for ( i = 0; i <= 360; i++ )
    {
        x0[i] = cos( dtr * i );
        y0[i] = sin( dtr * i );
    }

// Set up viewport and window, but do not draw box

    plenv( -1.3, 1.3, -1.3, 1.3, 1, -2 );
    for ( i = 1; i <= 10; i++ )
    {
        for ( j = 0; j <= 360; j++ )
        {
            x1[j] = 0.1 * i * x0[j];
            y1[j] = 0.1 * i * y0[j];
        }

// Draw circles for polar grid

        plline( 361, x1, y1 );
    }

    plcol0( 2 );
    for ( i = 0; i <= 11; i++ )
    {
        theta = 30.0 * i;
        dx    = cos( dtr * theta );
        dy    = sin( dtr * theta );

// Draw radial spokes for polar grid

        pljoin( 0.0, 0.0, dx, dy );
        sprintf( text, "%d", ROUND( theta ) );

// Write labels for angle

// Slightly off zero to avoid floating point logic flips at 90 and 270 deg.
        if ( dx >= -0.00001 )
            plptex( dx, dy, dx, dy, -0.15, text );
        else
            plptex( dx, dy, -dx, -dy, 1.15, text );
    }

// Draw the graph

    for ( i = 0; i <= 360; i++ )
    {
        r     = sin( dtr * ( 5 * i ) );
        x1[i] = x0[i] * r;
        y1[i] = y0[i] * r;
    }
    plcol0( 3 );
    plline( 361, x1, y1 );

    plcol0( 4 );
    plmtex( "t", 2.0, 0.5, 0.5,
        "#frPLplot Example 3 - r(#gh)=sin 5#gh" );
    plflush();
}

//--------------------------------------------------------------------------

// Demonstration of contour plotting

#define XPTS    35
#define YPTS    46
#define XSPA    2. / ( XPTS - 1 )
#define YSPA    2. / ( YPTS - 1 )

PLFLT tr[6] =
{ XSPA, 0.0, -1.0, 0.0, YSPA, -1.0 };

// pltr_data argument is unused so mark it with the PL_UNUSED macro
void
mypltr( PLFLT xx, PLFLT yy, PLFLT *tx, PLFLT *ty, void * PL_UNUSED( pltr_data ) )
{
    *tx = tr[0] * xx + tr[1] * yy + tr[2];
    *ty = tr[3] * xx + tr[4] * yy + tr[5];
}

static PLFLT clevel[11] =
{ -1., -.8, -.6, -.4, -.2, 0, .2, .4, .6, .8, 1. };

void
plot5( void )
{
    int          i, j;
    PLFLT        xx, yy;
    PLFLT        **z, **w;
    static PLINT mark = 1500, space = 1500;

// Set up function arrays

    plAlloc2dGrid( &z, XPTS, YPTS );
    plAlloc2dGrid( &w, XPTS, YPTS );

    for ( i = 0; i < XPTS; i++ )
    {
        xx = (PLFLT) ( i - ( XPTS / 2 ) ) / (PLFLT) ( XPTS / 2 );
        for ( j = 0; j < YPTS; j++ )
        {
            yy      = (PLFLT) ( j - ( YPTS / 2 ) ) / (PLFLT) ( YPTS / 2 ) - 1.0;
            z[i][j] = xx * xx - yy * yy;
            w[i][j] = 2 * xx * yy;
        }
    }

    plenv( -1.0, 1.0, -1.0, 1.0, 0, 0 );
    plcol0( 2 );
    plcont( (PLFLT_MATRIX) z, XPTS, YPTS, 1, XPTS, 1, YPTS, clevel, 11, mypltr, NULL );
    plstyl( 1, &mark, &space );
    plcol0( 3 );
    plcont( (PLFLT_MATRIX) w, XPTS, YPTS, 1, XPTS, 1, YPTS, clevel, 11, mypltr, NULL );
    plcol0( 1 );
    pllab( "X Coordinate", "Y Coordinate", "Streamlines of flow" );
    plflush();

// Clean up
    plFree2dGrid( z, XPTS, YPTS );
    plFree2dGrid( w, XPTS, YPTS );
}
