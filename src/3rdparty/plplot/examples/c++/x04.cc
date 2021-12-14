//--------------------------------------------------------------------------
// Copyright (C) 2004  Andrew Ross
// Copyright (C) 2004-2014 Alan W. Irwin
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
// Implementation of PLplot example 4 in C++.
//--------------------------------------------------------------------------

#include "plc++demos.h"

#ifdef PL_USE_NAMESPACE
using namespace std;
#endif

class x04 {
public:
    x04( int, char ** );
    void plot1( int );

private:
    // Class data
    plstream *pls;
};


x04::x04( int argc, char ** argv )
{
    // plplot initialization

    pls = new plstream();

    // Parse and process command line arguments.
    pls->parseopts( &argc, argv, PL_PARSE_FULL );

    // Initialize PLplot.
    pls->init();
    pls->font( 2 );

    // Make log plots using two different styles.

    plot1( 0 );
    plot1( 1 );

    // pls->end();
    delete pls;
}

// Log-linear plot.

void x04::plot1( int type )
{
    int        i;
    PLFLT      *freql = new PLFLT[101];
    PLFLT      *ampl  = new PLFLT[101];
    PLFLT      *phase = new PLFLT[101];
    PLFLT      f0, freq;
    PLINT      nlegend;
    const char *text[2], *symbols[2];
    PLINT      opt_array[2];
    PLINT      text_colors[2];
    PLINT      line_colors[2];
    PLINT      line_styles[2];
    PLFLT      line_widths[2];
    PLINT      symbol_numbers[2], symbol_colors[2];
    PLFLT      symbol_scales[2];
    PLFLT      legend_width, legend_height;

    pls->adv( 0 );

    // Set up data for log plot.

    f0 = 1.0;
    for ( i = 0; i <= 100; i++ )
    {
        freql[i] = -2.0 + i / 20.0;
        freq     = pow( 10.0, freql[i] );
        ampl[i]  = 20.0 * log10( 1.0 / sqrt( 1.0 + pow( ( freq / f0 ), 2. ) ) );
        phase[i] = -( 180.0 / M_PI ) * atan( (PLFLT) ( freq / f0 ) );
    }

    pls->vpor( 0.15, 0.85, 0.1, 0.9 );
    pls->wind( -2.0, 3.0, -80.0, 0.0 );

    // Try different axis and labelling styles.

    pls->col0( 1 );
    switch ( type )
    {
    case 0:
        pls->box( "bclnst", 0.0, 0, "bnstv", 0.0, 0 );
        break;
    case 1:
        pls->box( "bcfghlnst", 0.0, 0, "bcghnstv", 0.0, 0 );
        break;
    }

    // Plot ampl vs freq.

    pls->col0( 2 );
    pls->line( 101, freql, ampl );
    pls->col0( 2 );
    pls->ptex( 1.6, -30.0, 1.0, -20.0, 0.5, "-20 dB/decade" );

    // Put labels on.

    pls->col0( 1 );
    pls->mtex( "b", 3.2, 0.5, 0.5, "Frequency" );
    pls->mtex( "t", 2.0, 0.5, 0.5, "Single Pole Low-Pass Filter" );
    pls->col0( 2 );
    pls->mtex( "l", 5.0, 0.5, 0.5, "Amplitude (dB)" );
    nlegend = 1;

    // For the gridless case, put phase vs freq on same plot.

    if ( type == 0 )
    {
        pls->col0( 1 );
        pls->wind( -2.0, 3.0, -100.0, 0.0 );
        pls->box( "", 0.0, 0, "cmstv", 30.0, 3 );
        pls->col0( 3 );
        pls->line( 101, freql, phase );
        pls->string( 101, freql, phase, "#(728)" );
        pls->col0( 3 );
        pls->mtex( "r", 5.0, 0.5, 0.5, "Phase shift (degrees)" );
        nlegend = 2;
    }

    // Draw a legend
    // First legend entry.
    opt_array[0]   = PL_LEGEND_LINE;
    text_colors[0] = 2;
    text[0]        = "Amplitude";
    line_colors[0] = 2;
    line_styles[0] = 1;
    line_widths[0] = 1.0;
    // note from the above opt_array the first symbol (and box) indices
    // do not have to be specified

    // Second legend entry.
    opt_array[1]      = PL_LEGEND_LINE | PL_LEGEND_SYMBOL;
    text_colors[1]    = 3;
    text[1]           = "Phase shift";
    line_colors[1]    = 3;
    line_styles[1]    = 1;
    line_widths[1]    = 1.0;
    symbol_colors[1]  = 3;
    symbol_scales[1]  = 1.;
    symbol_numbers[1] = 4;
    symbols[1]        = "#(728)";
    // from the above opt_arrays we can completely ignore everything
    // to do with boxes.

    pls->scol0a( 15, 32, 32, 32, 0.70 );
    pls->legend( &legend_width, &legend_height,
        PL_LEGEND_BACKGROUND | PL_LEGEND_BOUNDING_BOX, 0,
        0.0, 0.0, 0.1, 15,
        1, 1, 0, 0,
        nlegend, opt_array,
        1.0, 1.0, 2.0,
        1., text_colors, (const char * const *) text,
        NULL, NULL, NULL, NULL,
        line_colors, line_styles, line_widths,
        symbol_colors, symbol_scales, symbol_numbers, (const char **) symbols );

    delete[] freql;
    delete[] ampl;
    delete[] phase;
}


int main( int argc, char ** argv )
{
    x04 *x = new x04( argc, argv );

    delete x;
}


//--------------------------------------------------------------------------
//                              End of x04.cc
//--------------------------------------------------------------------------
