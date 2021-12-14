// Alpha color values demonstration.
//
// Copyright (C) 2008 Hazen Babcock
//
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
// This example will only really be interesting when used with devices that
// support or alpha (or transparency) values, such as the cairo device family.
//


#include "plc++demos.h"

#ifdef PL_USE_NAMESPACE
using namespace std;
#endif

class x30 {
public:
    x30( int, char ** );

private:
    plstream      *pls;

    static PLINT  red[];
    static PLINT  green[];
    static PLINT  blue[];
    static PLFLT  alpha[];

    static PLFLT  px[];
    static PLFLT  py[];

    static PLFLT  pos[];
    static PLFLT  rcoord[];
    static PLFLT  gcoord[];
    static PLFLT  bcoord[];
    static PLFLT  acoord[];
    static PLBOOL rev[];
};

PLINT x30::red[] = { 0, 255, 0, 0 };
PLINT x30::green[] = { 0, 0, 255, 0 };
PLINT x30::blue[] = { 0, 0, 0, 255 };
PLFLT x30::alpha[] = { 1.0, 1.0, 1.0, 1.0 };

PLFLT x30::px[] = { 0.1, 0.5, 0.5, 0.1 };
PLFLT x30::py[] = { 0.1, 0.1, 0.5, 0.5 };

PLFLT x30::pos[] = { 0.0, 1.0 };
PLFLT x30::rcoord[] = { 1.0, 1.0 };
PLFLT x30::gcoord[] = { 0.0, 0.0 };
PLFLT x30::bcoord[] = { 0.0, 0.0 };
PLFLT x30::acoord[] = { 0.0, 1.0 };

x30::x30( int argc, char *argv[] )
{
    int   i, j;
    PLINT icol, r, g, b;
    PLFLT a;

    pls = new plstream;

    pls->parseopts( &argc, argv, PL_PARSE_FULL );

    pls->init();
    pls->scmap0n( 4 );
    pls->scmap0a( red, green, blue, alpha, 4 );

    //
    // Page 1:
    //
    // This is a series of red, green and blue rectangles overlaid
    // on each other with gradually increasing transparency.
    //

    // Set up the window
    pls->adv( 0 );
    pls->vpor( 0.0, 1.0, 0.0, 1.0 );
    pls->wind( 0.0, 1.0, 0.0, 1.0 );
    pls->col0( 0 );
    pls->box( "", 1.0, 0, "", 1.0, 0 );

    // Draw the boxes
    for ( i = 0; i < 9; i++ )
    {
        icol = i % 3 + 1;

        // Get a color, change its transparency and
        // set it as the current color.
        pls->gcol0a( icol, r, g, b, a );
        pls->scol0a( icol, r, g, b, 1.0 - (PLFLT) i / 9.0 );
        pls->col0( icol );

        // Draw the rectangle
        pls->fill( 4, px, py );

        // Shift the rectangles coordinates
        for ( j = 0; j < 4; j++ )
        {
            px[j] += 0.5 / 9.0;
            py[j] += 0.5 / 9.0;
        }
    }

    //
    // Page 2:
    //
    // This is a bunch of boxes colored red, green or blue with a single
    // large (red) box of linearly varying transparency overlaid. The
    // overlaid box is completely transparent at the bottom and completely
    // opaque at the top.
    //

    // Set up the window
    pls->adv( 0 );
    pls->vpor( 0.1, 0.9, 0.1, 0.9 );
    pls->wind( 0.0, 1.0, 0.0, 1.0 );

    // Draw the boxes. There are 25 of them drawn on a 5 x 5 grid.
    for ( i = 0; i < 5; i++ )
    {
        // Set box X position
        px[0] = 0.05 + 0.2 * i;
        px[1] = px[0] + 0.1;
        px[2] = px[1];
        px[3] = px[0];

        // We don't want the boxes to be transparent, so since we changed
        // the colors transparencies in the first example we have to change
        // the transparencies back to completely opaque.
        icol = i % 3 + 1;
        pls->gcol0a( icol, r, g, b, a );
        pls->scol0a( icol, r, g, b, 1.0 );
        pls->col0( icol );
        for ( j = 0; j < 5; j++ )
        {
            // Set box y position and draw the box.
            py[0] = 0.05 + 0.2 * j;
            py[1] = py[0];
            py[2] = py[0] + 0.1;
            py[3] = py[2];
            pls->fill( 4, px, py );
        }
    }

    // Create the color map with 128 colors and use plscmap1la to initialize
    // the color values with a linearly varying red transparency (or alpha)
    pls->scmap1n( 128 );
    pls->scmap1la( true, 2, pos, rcoord, gcoord, bcoord, acoord );

    // Use that cmap1 to create a transparent red gradient for the whole
    // window.
    px[0] = 0.;
    px[1] = 1.;
    px[2] = 1.;
    px[3] = 0.;

    py[0] = 0.;
    py[1] = 0.;
    py[2] = 1.;
    py[3] = 1.;

    pls->gradient( 4, px, py, 90. );

    delete pls;
}

int main( int argc, char **argv )
{
    x30 *x = new x30( argc, argv );

    delete x;
}
