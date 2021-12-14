//--------------------------------------------------------------------------
// Copyright (C) 2004  Andrew Ross
// Copyright (C) 2004  Alan W. Irwin
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
//
//--------------------------------------------------------------------------
// Implementation of PLplot example 6 in C++.
//--------------------------------------------------------------------------

#include "plc++demos.h"

#ifdef PL_USE_NAMESPACE
using namespace std;
#endif

class x06 {
public:
    x06( int, char ** );

private:
    // Class data
    plstream *pls;
};


x06::x06( int argc, char ** argv )
{
    int   i, j, k, kind_font, font, maxfont;
    PLFLT x, y;
    char  text[10];

    // plplot initialization

    pls = new plstream();

    // Parse and process command line arguments.
    pls->parseopts( &argc, argv, PL_PARSE_FULL );

    // Initialize PLplot.
    pls->init();

    for ( kind_font = 0; kind_font < 2; kind_font++ )
    {
        pls->fontld( kind_font );
        if ( kind_font == 0 )
            maxfont = 1;
        else
            maxfont = 4;

        for ( font = 0; font < maxfont; font++ )
        {
            pls->font( font + 1 );

            pls->adv( 0 );

            // Set up viewport and window.

            pls->col0( 2 );
            pls->vpor( 0.1, 1.0, 0.1, 0.9 );
            pls->wind( 0.0, 1.0, 0.0, 1.3 );

            // Draw the grid using plbox.

            pls->box( "bcg", 0.1, 0, "bcg", 0.1, 0 );

            // Write the digits below the frame.

            pls->col0( 15 );
            for ( i = 0; i <= 9; i++ )
            {
                sprintf( text, "%d", i );
                pls->mtex( "b", 1.5, ( 0.1 * i + 0.05 ), 0.5, text );
            }

            k = 0;
            for ( i = 0; i <= 12; i++ )
            {
                // Write the digits to the left of the frame.

                sprintf( text, "%d", 10 * i );
                pls->mtex( "lv", 1.0, ( 1.0 - ( 2 * i + 1 ) / 26.0 ), 1.0, text );
                for ( j = 0; j <= 9; j++ )
                {
                    x = 0.1 * j + 0.05;
                    y = 1.25 - 0.1 * i;

                    // Display the symbols (plpoin expects that x and y are
                    // arrays so that is what we feed it).

                    if ( k < 128 )
                        pls->poin( 1, &x, &y, k );
                    k = k + 1;
                }
            }

            if ( kind_font == 0 )
                pls->mtex( "t", 1.5, 0.5, 0.5, "PLplot Example 6 - plpoin symbols (compact)" );
            else
                pls->mtex( "t", 1.5, 0.5, 0.5, "PLplot Example 6 - plpoin symbols (extended)" );
        }
    }
    // pls->end();

    delete pls;
}

int main( int argc, char ** argv )
{
    x06 *x = new x06( argc, argv );

    delete x;
}


//--------------------------------------------------------------------------
//                              End of x06.cc
//--------------------------------------------------------------------------
