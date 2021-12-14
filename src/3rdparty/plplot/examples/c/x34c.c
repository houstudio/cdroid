//      Drawing mode setting and getting example.
//
// Copyright (C) 2011  Hezekiah M. Carty
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

// Drawing modes to demonstrate
#define NUM_MODES    3
PLINT         drawing_modes[NUM_MODES] = {
    PL_DRAWMODE_DEFAULT,
    PL_DRAWMODE_REPLACE,
    PL_DRAWMODE_XOR
};

PLCHAR_VECTOR drawing_mode_names[NUM_MODES] = {
    "Default drawing mode",
    "Replace destination with source",
    "XOR drawing"
};

void initialize_colors( void );
void draw_page( PLINT mode, PLCHAR_VECTOR title );

//--------------------------------------------------------------------------
// main
//--------------------------------------------------------------------------

int main( int argc, char *argv[] )
{
    PLINT mode;
    PLINT i;

    // PLplot initialization

    // Parse and process command line arguments
    plparseopts( &argc, argv, PL_PARSE_FULL );

    // Initialize PLplot
    plinit();

    // Check for drawing mode support
    mode = plgdrawmode();

    if ( mode == PL_DRAWMODE_UNKNOWN )
    {
        printf( "WARNING: This driver does not support drawing mode getting/setting" );
    }
    else
    {
        // Setup colors
        initialize_colors();

        // Draw one page per drawing mode
        for ( i = 0; i < NUM_MODES; i++ )
        {
            draw_page( drawing_modes[i], drawing_mode_names[i] );
        }
    }

    // Clean up
    plend();

    exit( 0 );
}

void initialize_colors( void )
{
    plscol0( 0, 255, 255, 255 );
    plscol0( 1, 0, 0, 0 );
    plscol0( 2, 255, 0, 0 );
    plscol0a( 3, 0, 0, 255, 0.3 );
}

void draw_page( PLINT mode, PLCHAR_VECTOR title )
{
    PLFLT xs[3], ys[3];
    PLFLT over_x, over_y, over_r;

    // A triangle for the background
    xs[0] = 0.0;
    xs[1] = 1.0;
    xs[2] = 0.0;
    ys[0] = 0.0;
    ys[1] = 1.0;
    ys[2] = 1.0;

    // A circle for the foreground
    over_x = 0.5;
    over_y = 0.5;
    over_r = 0.4;

    plcol0( 1 );

    // Setup a plot window
    plenv( 0.0, 1.0, 0.0, 1.0, 1, 0 );

    // Show which mode we're using
    pllab( "", "", title );

    // Draw a background triangle using the default drawing mode
    plcol0( 2 );
    plsdrawmode( PL_DRAWMODE_DEFAULT );
    plfill( 3, xs, ys );

    // Draw a circle in the given drawing mode
    plcol0( 3 );
    plsdrawmode( mode );
    plarc( over_x, over_y, over_r, over_r, 0.0, 360.0, 0.0, 1 );
}
