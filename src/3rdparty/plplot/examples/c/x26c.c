// Multi-lingual version of the first page of example 4.
//
// Copyright (C) 2006 Alan Irwin
// Copyright (C) 2006 Andrew Ross
//
// Thanks to the following for providing translated strings for this example:
// Valery Pipin (Russian)
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

//
// This example designed just for devices (e.g., psttfc and the
// cairo-related devices) that use the pango and fontconfig libraries. The
// best choice of glyph is selected by fontconfig and automatically rendered
// by pango in way that is sensitive to complex text layout (CTL) language
// issues for each unicode character in this example. Of course, you must
// have the appropriate TrueType fonts installed to have access to all the
// required glyphs.
//
// Translation instructions: The strings to be translated are given by
// x_label, y_label, alty_label, title_label, and line_label below.  The
// encoding used must be UTF-8.
//
// The following strings to be translated involve some scientific/mathematical
// jargon which is now discussed further to help translators.
//
// (1) dB is a decibel unit, see http://en.wikipedia.org/wiki/Decibel .
// (2) degrees is an angular measure, see
//    http://en.wikipedia.org/wiki/Degree_(angle) .
// (3) low-pass filter is one that transmits (passes) low frequencies.
// (4) pole is in the mathematical sense, see
//    http://en.wikipedia.org/wiki/Pole_(complex_analysis) .  "Single Pole"
//    means a particular mathematical transformation of the filter function has
//    a single pole, see
//    http://ccrma.stanford.edu/~jos/filters/Pole_Zero_Analysis_I.html .
//    Furthermore, a single-pole filter must have an inverse square decline
//    (or -20 db/decade). Since the filter plotted here does have that
//    characteristic, it must by definition be a single-pole filter, see also
//    http://www-k.ext.ti.com/SRVS/Data/ti/KnowledgeBases/analog/document/faqs/1p.htm
// (5) decade represents a factor of 10, see
//    http://en.wikipedia.org/wiki/Decade_(log_scale) .
//


#include "plcdemos.h"

static PLCHAR_VECTOR x_labels[] = {
    "Frequency",
    "Частота",
    NULL
};

static PLCHAR_VECTOR y_labels[] = {
    "Amplitude (dB)",
    "Амплитуда (dB)",
    NULL
};

static PLCHAR_VECTOR alty_labels[] = {
    "Phase shift (degrees)",
    "Фазовый сдвиг (градусы)",
    NULL
};

// Short rearranged versions of y_label and alty_label.
static PLCHAR_VECTOR legend_texts[][2] = {
    { "Amplitude", "Phase shift"   },
    { "Амплитуда", "Фазовый сдвиг" }
};

static PLCHAR_VECTOR title_labels[] = {
    "Single Pole Low-Pass Filter",
    "Однополюсный Низко-Частотный Фильтр",
    NULL
};

static PLCHAR_VECTOR line_labels[] = {
    "-20 dB/decade",
    "-20 dB/десяток",
    NULL
};

void plot1( int type, PLCHAR_VECTOR x_label, PLCHAR_VECTOR y_label,
            PLCHAR_VECTOR alty_label, PLCHAR_VECTOR legend_text[],
            PLCHAR_VECTOR title_label, PLCHAR_VECTOR line_label );

//--------------------------------------------------------------------------
// main
//
// Illustration of logarithmic axes, and redefinition of window.
//--------------------------------------------------------------------------

int
main( int argc, char *argv[] )
{
    int i;
// Parse and process command line arguments

    (void) plparseopts( &argc, argv, PL_PARSE_FULL );

// Initialize plplot

    plinit();
    plfont( 2 );

// Make log plots using two different styles.

    i = 0;
    while ( x_labels[i] != NULL )
    {
        plot1( 0, x_labels[i], y_labels[i], alty_labels[i],
            legend_texts[i], title_labels[i], line_labels[i] );
        i++;
    }

    plend();
    exit( 0 );
}

//--------------------------------------------------------------------------
// plot1
//
// Log-linear plot.
//--------------------------------------------------------------------------

void
plot1( int type, PLCHAR_VECTOR x_label, PLCHAR_VECTOR y_label, PLCHAR_VECTOR alty_label,
       PLCHAR_VECTOR legend_text[], PLCHAR_VECTOR title_label, PLCHAR_VECTOR line_label )
{
    int           i;
    static PLFLT  freql[101], ampl[101], phase[101];
    PLFLT         f0, freq;
    PLINT         nlegend = 2;
    PLINT         opt_array[2];
    PLINT         text_colors[2];
    PLINT         line_colors[2];
    PLINT         line_styles[2];
    PLFLT         line_widths[2];
    PLINT         symbol_numbers[2], symbol_colors[2];
    PLFLT         symbol_scales[2];
    PLCHAR_VECTOR symbols[2];
    PLFLT         legend_width, legend_height;


    pladv( 0 );

// Set up data for log plot

    f0 = 1.0;
    for ( i = 0; i <= 100; i++ )
    {
        freql[i] = -2.0 + i / 20.0;
        freq     = pow( 10.0, freql[i] );
        ampl[i]  = 20.0 * log10( 1.0 / sqrt( 1.0 + pow( ( freq / f0 ), 2. ) ) );
        phase[i] = -( 180.0 / M_PI ) * atan( freq / f0 );
    }

    plvpor( 0.15, 0.85, 0.1, 0.9 );
    plwind( -2.0, 3.0, -80.0, 0.0 );

// Try different axis and labelling styles.

    plcol0( 1 );
    switch ( type )
    {
    case 0:
        plbox( "bclnst", 0.0, 0, "bnstv", 0.0, 0 );
        break;
    case 1:
        plbox( "bcfghlnst", 0.0, 0, "bcghnstv", 0.0, 0 );
        break;
    }

// Plot ampl vs freq

    plcol0( 2 );
    plline( 101, freql, ampl );
    plcol0( 2 );
    plptex( 1.6, -30.0, 1.0, -20.0, 0.5, line_label );

// Put labels on

    plcol0( 1 );
    plmtex( "b", 3.2, 0.5, 0.5, x_label );
    plmtex( "t", 2.0, 0.5, 0.5, title_label );
    plcol0( 2 );
    plmtex( "l", 5.0, 0.5, 0.5, y_label );

// For the gridless case, put phase vs freq on same plot

    if ( type == 0 )
    {
        plcol0( 1 );
        plwind( -2.0, 3.0, -100.0, 0.0 );
        plbox( "", 0.0, 0, "cmstv", 30.0, 3 );
        plcol0( 3 );
        plline( 101, freql, phase );
        plstring( 101, freql, phase, "#(728)" );
        plcol0( 3 );
        plmtex( "r", 5.0, 0.5, 0.5, alty_label );
    }
    // Draw a legend
    // First legend entry.
    opt_array[0]   = PL_LEGEND_LINE;
    text_colors[0] = 2;
    line_colors[0] = 2;
    line_styles[0] = 1;
    line_widths[0] = 1.;
    // note from the above opt_array the first symbol (and box) indices
    // do not have to be specified

    // Second legend entry.
    opt_array[1]      = PL_LEGEND_LINE | PL_LEGEND_SYMBOL;
    text_colors[1]    = 3;
    line_colors[1]    = 3;
    line_styles[1]    = 1;
    line_widths[1]    = 1.;
    symbol_colors[1]  = 3;
    symbol_scales[1]  = 1.;
    symbol_numbers[1] = 4;
    symbols[1]        = "#(728)";
    // from the above opt_arrays we can completely ignore everything
    // to do with boxes.

    plscol0a( 15, 32, 32, 32, 0.70 );
    pllegend( &legend_width, &legend_height,
        PL_LEGEND_BACKGROUND | PL_LEGEND_BOUNDING_BOX, 0,
        0.0, 0.0, 0.10, 15,
        1, 1, 0, 0,
        nlegend, opt_array,
        1.0, 1.0, 2.0,
        1., text_colors, (const char **) legend_text,
        NULL, NULL, NULL, NULL,
        line_colors, line_styles, line_widths,
        symbol_colors, symbol_scales, symbol_numbers, (const char **) symbols );
}
