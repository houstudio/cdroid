// Copyright (C) 2008-2016 Alan W. Irwin
// Copyright (C) 2008 Andrew Ross
//
// set/get tester
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

int
main( int argc, char *argv[] )
{
    PLFLT xmin0, xmax0, ymin0, ymax0, zxmin0, zxmax0, zymin0, zymax0;
    PLFLT xmin, xmax, ymin, ymax, zxmin, zxmax, zymin, zymax;
    PLFLT xmid, ymid, wx, wy;
    PLFLT mar0, aspect0, jx0, jy0, ori0;
    PLFLT mar, aspect, jx, jy, ori;
    PLINT win, level2, digmax, digits, compression1, compression2;
    PLFLT xp0, yp0;
    PLINT xleng0, yleng0, xoff0, yoff0;
    PLFLT xp1, yp1;
    PLINT xleng1, yleng1, xoff1, yoff1;
    PLFLT xp2, yp2;
    PLINT xleng2, yleng2, xoff2, yoff2;
    PLINT fam0, num0, bmax0;
    PLINT fam1, num1, bmax1;
    PLINT fam2, num2, bmax2;
    PLINT r0, g0, b0;
    PLFLT a0;
    PLINT r, g, b;
    PLFLT a;
    PLINT r1[] = { 0, 255 };
    PLINT g1[] = { 255, 0 };
    PLINT b1[] = { 0, 0 };
    PLFLT a1[] = { 1.0, 1.0 };
    int   status;
    char  fnam[256];

    // Parse and process command line arguments

    status = 0;

    (void) plparseopts( &argc, argv, PL_PARSE_FULL );

    // Test setting / getting familying parameters before plinit
    // Save values set by plparseopts to be restored later.
    plgfam( &fam0, &num0, &bmax0 );
    fam1  = 0;
    num1  = 10;
    bmax1 = 1000;
    plsfam( fam1, num1, bmax1 );

    // Retrieve the same values?
    plgfam( &fam2, &num2, &bmax2 );
    printf( "family parameters: fam, num, bmax = %d %d %d\n", fam2, num2, bmax2 );
    if ( fam2 != fam1 || num2 != num1 || bmax2 != bmax1 )
    {
        fputs( "plgfam test failed\n", stderr );
        status = 1;
    }
    // Restore values set initially by plparseopts.
    plsfam( fam0, num0, bmax0 );

    // Test setting / getting page parameters before plinit
    // Save values set by plparseopts to be restored later.
    plgpage( &xp0, &yp0, &xleng0, &yleng0, &xoff0, &yoff0 );
    xp1    = 200.;
    yp1    = 200.;
    xleng1 = 400;
    yleng1 = 200;
    xoff1  = 10;
    yoff1  = 20;
    plspage( xp1, yp1, xleng1, yleng1, xoff1, yoff1 );

    // Retrieve the same values?
    plgpage( &xp2, &yp2, &xleng2, &yleng2, &xoff2, &yoff2 );
    printf( "page parameters: xp, yp, xleng, yleng, xoff, yoff = %f %f %d %d %d %d\n", xp2, yp2, xleng2, yleng2, xoff2, yoff2 );
    if ( xp2 != xp1 || yp2 != yp1 || xleng2 != xleng1 || yleng2 != yleng1 ||
         xoff2 != xoff1 || yoff2 != yoff1 )
    {
        fputs( "plgpage test failed\n", stderr );
        status = 1;
    }
    // Restore values set initially by plparseopts.
    plspage( xp0, yp0, xleng0, yleng0, xoff0, yoff0 );

    // Test setting / getting compression parameter across plinit.
    compression1 = 95;
    plscompression( compression1 );

    // Initialize plplot
    plinit();

    // Test if device initialization screwed around with the preset
    // compression parameter.
    plgcompression( &compression2 );
    printf( "Output various PLplot parameters\n" );
    printf( "compression parameter = %d\n", compression2 );
    if ( compression2 != compression1 )
    {
        fputs( "plgcompression test failed\n", stderr );
        status = 1;
    }


    // Exercise plscolor, plscol0, plscmap1, and plscmap1a to make sure
    // they work without any obvious error messages.
    plscolor( 1 );
    plscol0( 1, 255, 0, 0 );
    plscmap1( r1, g1, b1, 2 );
    plscmap1a( r1, g1, b1, a1, 2 );

    plglevel( &level2 );
    printf( "level parameter = %d\n", level2 );
    if ( level2 != 1 )
    {
        fputs( "plglevel test failed.\n", stderr );
        status = 1;
    }

    pladv( 0 );

    xmin0 = 0.01;
    xmax0 = 0.99;
    ymin0 = 0.02;
    ymax0 = 0.49;
    plvpor( xmin0, xmax0, ymin0, ymax0 );
    plgvpd( &xmin, &xmax, &ymin, &ymax );
    printf( "plvpor: xmin, xmax, ymin, ymax = %f %f %f %f\n", xmin, xmax, ymin, ymax );
    if ( xmin != xmin0 || xmax != xmax0 || ymin != ymin0 || ymax != ymax0 )
    {
        fputs( "plgvpd test failed\n", stderr );
        status = 1;
    }
    xmid = 0.5 * ( xmin + xmax );
    ymid = 0.5 * ( ymin + ymax );

    xmin0 = 0.2;
    xmax0 = 0.3;
    ymin0 = 0.4;
    ymax0 = 0.5;
    plwind( xmin0, xmax0, ymin0, ymax0 );
    plgvpw( &xmin, &xmax, &ymin, &ymax );
    printf( "plwind: xmin, xmax, ymin, ymax = %f %f %f %f\n", xmin, xmax, ymin, ymax );
    if ( xmin != xmin0 || xmax != xmax0 || ymin != ymin0 || ymax != ymax0 )
    {
        fputs( "plgvpw test failed\n", stderr );
        status = 1;
    }

    // Get world coordinates for middle of viewport
    plcalc_world( xmid, ymid, &wx, &wy, &win );
    printf( "world parameters: wx, wy, win = %f %f %d\n", wx, wy, win );
    if ( fabs( wx - 0.5 * ( xmin + xmax ) ) > 1.0E-5 || fabs( wy - 0.5 * ( ymin + ymax ) ) > 1.0E-5 )
    {
        fputs( "plcalc_world test failed\n", stderr );
        status = 1;
    }

    // Retrieve and print the name of the output file (if any).
    // This goes to stderr not stdout since it will vary between tests and
    // we want stdout to be identical for compare test.
    plgfnam( fnam );
    if ( fnam[0] == '\0' )
    {
        printf( "No output file name is set\n" );
    }
    else
    {
        printf( "Output file name read\n" );
    }
    fprintf( stderr, "Output file name is %s\n", fnam );

    // Set and get the number of digits used to display axis labels
    // Note digits is currently ignored in pls[xyz]ax and
    // therefore it does not make sense to test the returned
    // value
    plsxax( 3, 0 );
    plgxax( &digmax, &digits );
    printf( "x axis parameters: digmax, digits = %d %d\n", digmax, digits );
    if ( digmax != 3 )
    {
        fputs( "plgxax test failed\n", stderr );
        status = 1;
    }

    plsyax( 4, 0 );
    plgyax( &digmax, &digits );
    printf( "y axis parameters: digmax, digits = %d %d\n", digmax, digits );
    if ( digmax != 4 )
    {
        fputs( "plgyax test failed\n", stderr );
        status = 1;
    }

    plszax( 5, 0 );
    plgzax( &digmax, &digits );
    printf( "z axis parameters: digmax, digits = %d %d\n", digmax, digits );
    if ( digmax != 5 )
    {
        fputs( "plgzax test failed\n", stderr );
        status = 1;
    }

    mar0    = 0.05;
    aspect0 = PL_NOTSET;
    jx0     = 0.1;
    jy0     = 0.2;
    plsdidev( mar0, aspect0, jx0, jy0 );
    plgdidev( &mar, &aspect, &jx, &jy );
    printf( "device-space window parameters: mar, aspect, jx, jy = %f %f %f %f\n", mar, aspect, jx, jy );
    if ( mar != mar0 || jx != jx0 || jy != jy0 )
    {
        fputs( "plgdidev test failed\n", stderr );
        status = 1;
    }

    ori0 = 1.0;
    plsdiori( ori0 );
    plgdiori( &ori );
    printf( "ori parameter = %f\n", ori );
    if ( ori != ori0 )
    {
        fputs( "plgdiori test failed\n", stderr );
        status = 1;
    }

    xmin0 = 0.1;
    ymin0 = 0.2;
    xmax0 = 0.9;
    ymax0 = 0.8;
    plsdiplt( xmin0, ymin0, xmax0, ymax0 );
    plgdiplt( &xmin, &ymin, &xmax, &ymax );
    printf( "plot-space window parameters: xmin, ymin, xmax, ymax = %f %f %f %f\n", xmin, ymin, xmax, ymax );
    if ( xmin != xmin0 || ymin != ymin0 || xmax != xmax0 || ymax != ymax0 )
    {
        fputs( "plgdiplt test failed\n", stderr );
        status = 1;
    }

    zxmin0 = 0.1;
    zymin0 = 0.1;
    zxmax0 = 0.9;
    zymax0 = 0.9;
    plsdiplz( zxmin0, zymin0, zxmax0, zymax0 );
    plgdiplt( &zxmin, &zymin, &zxmax, &zymax );
    printf( "zoomed plot-space window parameters: xmin, ymin, xmax, ymax = %f %f %f %f\n", zxmin, zymin, zxmax, zymax );
    if ( fabs( zxmin - ( xmin + ( xmax - xmin ) * zxmin0 ) ) > 1.0E-5 ||
         fabs( zymin - ( ymin + ( ymax - ymin ) * zymin0 ) ) > 1.0E-5 ||
         fabs( zxmax - ( xmin + ( xmax - xmin ) * zxmax0 ) ) > 1.0E-5 ||
         fabs( zymax - ( ymin + ( ymax - ymin ) * zymax0 ) ) > 1.0E-5 )
    {
        fputs( "plsdiplz test failed\n", stderr );
        status = 1;
    }

    r0 = 10;
    g0 = 20;
    b0 = 30;
    plscolbg( r0, g0, b0 );
    plgcolbg( &r, &g, &b );
    printf( "background colour parameters: r, g, b = %d %d %d\n", r, g, b );
    if ( r != r0 || g != g0 || b != b0 )
    {
        fputs( "plgcolbg test failed\n", stderr );
        status = 1;
    }

    r0 = 20;
    g0 = 30;
    b0 = 40;
    a0 = 0.5;
    plscolbga( r0, g0, b0, a0 );
    plgcolbga( &r, &g, &b, &a );
    printf( "background/transparency colour parameters: r, g, b, a = %d %d %d %f\n", r, g, b, a );
    if ( r != r0 || g != g0 || b != b0 || a != a0 )
    {
        fputs( "plgcolbga test failed\n", stderr );
        status = 1;
    }

    plend();

    exit( status );
}
