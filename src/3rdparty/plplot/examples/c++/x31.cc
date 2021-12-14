//   set/get tester
//
// Copyright (C) 2008 Andrew Ross
// Copyright (C) 2008-2016 Alan W. Irwin
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

#include "plc++demos.h"

#ifdef PL_USE_NAMESPACE
using namespace std;
#endif

class x31 {
public:
    x31( int, char ** );
    int status;

private:
    plstream *pls;
};

//--------------------------------------------------------------------------
// main
//
// Demonstrates absolute positioning of graphs on a page.
//--------------------------------------------------------------------------

x31::x31( int argc, char *argv[] )
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
    char  fnam[256];

    pls = new plstream();

    status = 0;

    // Set floating point precision for cout consistent with C printf.
    cout.setf( ios::fixed, ios::floatfield );
    cout.setf( ios::showpoint );
    cout.precision( 6 );

    pls->parseopts( &argc, argv, PL_PARSE_FULL );

    // Test setting / getting familying parameters before plinit.
    // Save values set by plparseopts to be restored later.
    pls->gfam( fam0, num0, bmax0 );
    fam1  = 0;
    num1  = 10;
    bmax1 = 1000;
    pls->sfam( fam1, num1, bmax1 );

    // Retrieve the same values?
    pls->gfam( fam2, num2, bmax2 );
    cout << "family parameters: fam, num, bmax = " << fam2 << " " <<
        num2 << " " << bmax2 << endl;
    if ( fam2 != fam1 || num2 != num1 || bmax2 != bmax1 )
    {
        cerr << "plgfam test failed" << endl;
        status = 1;
    }
    // Restore values set initially by plparseopts.
    pls->sfam( fam0, num0, bmax0 );

    // Test setting / getting page parameters before plinit.
    // Save values set by plparseopts to be restored later.
    pls->gpage( xp0, yp0, xleng0, yleng0, xoff0, yoff0 );
    xp1    = 200.;
    yp1    = 200.;
    xleng1 = 400;
    yleng1 = 200;
    xoff1  = 10;
    yoff1  = 20;
    pls->spage( xp1, yp1, xleng1, yleng1, xoff1, yoff1 );

    // Retrieve the same values?
    pls->gpage( xp2, yp2, xleng2, yleng2, xoff2, yoff2 );
    cout << "page parameters: xp, yp, xleng, yleng, xoff, yoff = " << xp2 <<
        " " << yp2 << " " << xleng2 << " " << yleng2 << " " << xoff2 << " " <<
        yoff2 << endl;
    if ( xp2 != xp1 || yp2 != yp1 || xleng2 != xleng1 || yleng2 != yleng1 ||
         xoff2 != xoff1 || yoff2 != yoff1 )
    {
        cerr << "plgpage test failed" << endl;
        status = 1;
    }
    // Restore values set initially by plparseopts.
    pls->spage( xp0, yp0, xleng0, yleng0, xoff0, yoff0 );

    // Test setting / getting compression parameter across plinit.
    compression1 = 95;
    pls->scompression( compression1 );

    // Initialize plplot
    pls->init();

    // Test if device initialization screwed around with the preset
    // compression parameter.
    pls->gcompression( compression2 );
    cout << "Output various PLplot parameters" << endl;
    cout << "compression parameter = " << compression2 << endl;
    if ( compression2 != compression1 )
    {
        cerr << "plgcompression test failed" << endl;
        status = 1;
    }

    // Exercise plscolor, plscol0, plscmap1, and plscmap1a to make sure
    // they work without any obvious error messages.
    pls->scolor( 1 );
    pls->scol0( 1, 255, 0, 0 );
    pls->scmap1( r1, g1, b1, 2 );
    pls->scmap1a( r1, g1, b1, a1, 2 );

    pls->glevel( level2 );
    cout << "level parameter = " << level2 << endl;
    if ( level2 != 1 )
    {
        cerr << "plglevel test failed." << endl;
        status = 1;
    }

    pls->adv( 0 );

    xmin0 = 0.01;
    xmax0 = 0.99;
    ymin0 = 0.02;
    ymax0 = 0.49;
    pls->vpor( xmin0, xmax0, ymin0, ymax0 );
    pls->gvpd( xmin, xmax, ymin, ymax );
    cout << "plvpor: xmin, xmax, ymin, ymax = " << xmin << " " << xmax << " " << ymin << " " << ymax << endl;
    if ( xmin != xmin0 || xmax != xmax0 || ymin != ymin0 || ymax != ymax0 )
    {
        cerr << "plgvpd test failed" << endl;
        status = 1;
    }
    xmid = 0.5 * ( xmin + xmax );
    ymid = 0.5 * ( ymin + ymax );

    xmin0 = 0.2;
    xmax0 = 0.3;
    ymin0 = 0.4;
    ymax0 = 0.5;
    pls->wind( xmin0, xmax0, ymin0, ymax0 );
    pls->gvpw( xmin, xmax, ymin, ymax );
    cout << "plwind: xmin, xmax, ymin, ymax = " << xmin << " " << xmax << " " << ymin << " " << ymax << endl;
    if ( xmin != xmin0 || xmax != xmax0 || ymin != ymin0 || ymax != ymax0 )
    {
        cerr << "plgvpw test failed" << endl;
        status = 1;
    }

    // Get world coordinates for middle of viewport
    pls->calc_world( xmid, ymid, wx, wy, win );
    cout << "world parameters: wx, wy, win = " << wx << " " << wy << " " << win << endl;
    if ( fabs( wx - 0.5 * ( xmin + xmax ) ) > 1.0E-5 || fabs( wy - 0.5 * ( ymin + ymax ) ) > 1.0E-5 )
    {
        cerr << "plcalc_world test failed" << endl;
        status = 1;
    }

    // Retrieve and print the name of the output file (if any).
    // This goes to stderr not stdout since it will vary between tests and
    // we want stdout to be identical for compare test.
    pls->gfnam( fnam );
    if ( fnam[0] == '\0' )
    {
        cout << "No output file name is set" << endl;
    }
    else
    {
        cout << "Output file name read" << endl;
    }
    cerr << "Output file name is " << fnam << endl;

    // Set and get the number of digits used to display axis labels
    // Note digits is currently ignored in pls[xyz]ax and
    // therefore it does not make sense to test the returned
    // value
    pls->sxax( 3, 0 );
    pls->gxax( digmax, digits );
    cout << "x axis parameters: digmax, digits = " << digmax << " " << digits << endl;
    if ( digmax != 3 )
    {
        cerr << "plgxax test failed" << endl;
        status = 1;
    }

    pls->syax( 4, 0 );
    pls->gyax( digmax, digits );
    cout << "y axis parameters: digmax, digits = " << digmax << " " << digits << endl;
    if ( digmax != 4 )
    {
        cerr << "plgyax test failed" << endl;
        status = 1;
    }

    pls->szax( 5, 0 );
    pls->gzax( digmax, digits );
    cout << "z axis parameters: digmax, digits = " << digmax << " " << digits << endl;
    if ( digmax != 5 )
    {
        cerr << "plgzax test failed" << endl;
        status = 1;
    }

    mar0    = 0.05;
    aspect0 = PL_NOTSET;
    jx0     = 0.1;
    jy0     = 0.2;
    pls->sdidev( mar0, aspect0, jx0, jy0 );
    pls->gdidev( mar, aspect, jx, jy );
    cout << "device-space window parameters: mar, aspect, jx, jy = " << mar << " " << aspect << " " << jx << " " << jy << endl;
    if ( mar != mar0 || jx != jx0 || jy != jy0 )
    {
        cerr << "plgdidev test failed" << endl;
        status = 1;
    }

    ori0 = 1.0;
    pls->sdiori( ori0 );
    pls->gdiori( ori );
    cout << "ori parameter = " << ori << endl;
    if ( ori != ori0 )
    {
        cerr << "plgdiori test failed" << endl;
        status = 1;
    }

    xmin0 = 0.1;
    ymin0 = 0.2;
    xmax0 = 0.9;
    ymax0 = 0.8;
    pls->sdiplt( xmin0, ymin0, xmax0, ymax0 );
    pls->gdiplt( xmin, ymin, xmax, ymax );
    cout << "plot-space window parameters: xmin, ymin, xmax, ymax = " << xmin << " " << ymin << " " << xmax << " " << ymax << endl;
    if ( xmin != xmin0 || ymin != ymin0 || xmax != xmax0 || ymax != ymax0 )
    {
        cerr << "plgdiplt test failed" << endl;
        status = 1;
    }

    zxmin0 = 0.1;
    zymin0 = 0.1;
    zxmax0 = 0.9;
    zymax0 = 0.9;
    pls->sdiplz( zxmin0, zymin0, zxmax0, zymax0 );
    pls->gdiplt( zxmin, zymin, zxmax, zymax );
    cout << "zoomed plot-space window parameters: xmin, ymin, xmax, ymax = " << zxmin << " " << zymin << " " << zxmax << " " << zymax << endl;
    if ( fabs( zxmin - ( xmin + ( xmax - xmin ) * zxmin0 ) ) > 1.0E-5 ||
         fabs( zymin - ( ymin + ( ymax - ymin ) * zymin0 ) ) > 1.0E-5 ||
         fabs( zxmax - ( xmin + ( xmax - xmin ) * zxmax0 ) ) > 1.0E-5 ||
         fabs( zymax - ( ymin + ( ymax - ymin ) * zymax0 ) ) > 1.0E-5 )
    {
        cerr << "plsdiplz test failed" << endl;
        status = 1;
    }

    r0 = 10;
    g0 = 20;
    b0 = 30;
    pls->scolbg( r0, g0, b0 );
    pls->gcolbg( r, g, b );
    cout << "background colour parameters: r, g, b = " << r << " " << g << " " << b << endl;
    if ( r != r0 || g != g0 || b != b0 )
    {
        cerr << "plgcolbg test failed" << endl;
        status = 1;
    }

    r0 = 20;
    g0 = 30;
    b0 = 40;
    a0 = 0.5;
    pls->scolbga( r0, g0, b0, a0 );
    pls->gcolbga( r, g, b, a );
    cout << "background/transparency colour parameters: r, g, b, a = " << r << " " << g << " " << b << " " << a << endl;
    if ( r != r0 || g != g0 || b != b0 || a != a0 )
    {
        cerr << "plgcolbga test failed" << endl;
        status = 1;
    }

    delete pls;
}

int main( int argc, char **argv )
{
    int status;

    x31 *x = new x31( argc, argv );
    status = x->status;

    delete x;
    return status;
}
