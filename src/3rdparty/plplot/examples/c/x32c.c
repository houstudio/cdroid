//      Box plot demo.
//
// Copyright (C) 2008 by FLLL <http://www.flll.jku.at>
// Author: Robert Pollak <robert.pollak@jku.at>
// Copyright (C) 2009 Andrew Ross
//
// This file is part of PLplot.
//
//  PLplot is free software; you can redistribute it and/or modify
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

void
plfbox( PLFLT x, PLFLT y25, PLFLT y50, PLFLT y75, PLFLT lw, PLFLT uw );
void
plcircle( PLFLT x, PLFLT y );

//--------------------------------------------------------------------------
// main
//
// Does a box plot.
//--------------------------------------------------------------------------

int
main( int argc, char *argv[] )
{
    int          i;
    char         string[20];
    PLFLT        x0;
    // quartiles
    static PLFLT y25[] = { 0.984, 0.980, 0.976, 0.975, 0.973,
                           0.967, 0.974, 0.954, 0.987, 0.991 };
    static PLFLT y50[] = { 0.994, 0.999, 1.035, 0.995, 1.002,
                           0.997, 1.034, 0.984, 1.007, 1.017 };
    static PLFLT y75[] = { 1.054, 1.040, 1.066, 1.025, 1.043,
                           1.017, 1.054, 1.004, 1.047, 1.031 };

    // lower and upper whisker
    static PLFLT lw[] = { 0.964, 0.950, 0.926, 0.955, 0.963,
                          0.937, 0.944, 0.924, 0.967, 0.941 };

    static PLFLT uw[] = { 1.071, 1.062, 1.093, 1.045, 1.072,
                          1.067, 1.085, 1.024, 1.057, 1.071 };

    // outliers
    static PLFLT outx[] = { 3.5, 6.5 };
    static PLFLT outy[] = { 0.89, 1.09 };
//
//  static PLFLT pos[] = {0.0, 0.25, 0.5, 0.75, 1.0};
//  static PLFLT red[] = {0.0, 0.25, 0.5, 1.0, 1.0};
//  static PLFLT green[] = {1.0, 0.5, 0.5, 0.5, 1.0};
//  static PLFLT blue[] = {1.0, 1.0, 0.5, 0.25, 0.0};
//
// Parse and process command line arguments

    (void) plparseopts( &argc, argv, PL_PARSE_FULL );

// Initialize plplot

    plinit();

    pladv( 0 );
    plvsta();

    x0 = 1.;
    plwind( x0, x0 + 10, 0.85, 1.15 );
    plcol0( 1 );
    plbox( "bc", 1.0, 0, "bcgnst", 0, 0 );
    pllab( "Group", "Value", "#frPLplot Example 32" );

    // plscmap1l(1,5,pos,red,green,blue,NULL);

    for ( i = 0; i < 10; i++ )
    {
        plcol1( i / 9.0 );
        plfbox( ( x0 + i ), y25[i], y50[i], y75[i], lw[i], uw[i] );

        sprintf( string, "%d", (int) x0 + i );
        plmtex( "b", 1.0, ( ( i + 1 ) * .1 - .05 ), 0.5, string );
    }

    // some outliers plotted with intermediate-sized circles, see PLplot
    // example 06 for other possibilities.
    plpoin( 2, outx, outy, 22 );

// Don't forget to call plend() to finish off!

    plend();
    exit( 0 );
}

void
plfbox( PLFLT x, PLFLT y25, PLFLT y50, PLFLT y75, PLFLT lw, PLFLT uw )
{
    PLFLT px[5], py[5], mx[2], my[2], wx[2], wy[2], barx[2], bary[2];
    PLFLT spacing;
    PLFLT xmin, xmax;
    PLFLT xmid, xwidth;

    spacing = .4; // in x axis units

    xmin = x + spacing / 2.;
    xmax = x + 1. - spacing / 2.;

    // box

    px[0] = xmin;
    py[0] = y25;
    px[1] = xmin;
    py[1] = y75;
    px[2] = xmax;
    py[2] = y75;
    px[3] = xmax;
    py[3] = y25;
    px[4] = xmin;
    py[4] = y25;

    plpsty( 0 );
    plfill( 4, px, py );
    plcol0( 1 );
    pllsty( 1 );
    plline( 5, px, py );


    // median

    mx[0] = xmin;
    my[0] = y50;
    mx[1] = xmax;
    my[1] = y50;

    pllsty( 1 );
    plline( 2, mx, my );

    // lower whisker

    xmid   = ( xmin + xmax ) / 2.;
    xwidth = xmax - xmin;
    wx[0]  = xmid;
    wy[0]  = lw;
    wx[1]  = xmid;
    wy[1]  = y25;

    pllsty( 2 ); // short dashes and gaps
    plline( 2, wx, wy );

    barx[0] = xmid - xwidth / 4.;
    bary[0] = lw;
    barx[1] = xmid + xwidth / 4.;
    bary[1] = lw;

    pllsty( 1 );
    plline( 2, barx, bary );

    // upper whisker

    xmid   = ( xmin + xmax ) / 2.;
    xwidth = xmax - xmin;
    wy[0]  = y75;
    wy[1]  = uw;

    pllsty( 2 ); // short dashes and gaps
    plline( 2, wx, wy );

    bary[0] = uw;
    bary[1] = uw;

    pllsty( 1 );
    plline( 2, barx, bary );
}
