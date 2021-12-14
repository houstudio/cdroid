//--------------------------------------------------------------------------
// Multi-lingual version of the first page of example 4.
//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
// Copyright (C) 2006-2014 Alan W. Irwin
// Copyright (C) 2006  Andrew Ross
//
// Thanks to the following for providing translated strings for this example:
// Valery Pipin (Russian)
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

//  This example designed just for devices (e.g., psttfc and the
//  cairo-related devices) that use the pango and fontconfig libraries. The
//  best choice of glyph is selected by fontconfig and automatically rendered
//  by pango in way that is sensitive to complex text layout (CTL) language
//  issues for each unicode character in this example. Of course, you must
//  have the appropriate TrueType fonts installed to have access to all the
//  required glyphs.
//
//  Translation instructions: The strings to be translated are given by
//  x_label, y_label, alty_label, title_label, and line_label below.  The
//  encoding used must be UTF-8.
//
//  The following strings to be translated involve some scientific/mathematical
//  jargon which is now discussed further to help translators.
//
//  (1) dB is a decibel unit, see http://en.wikipedia.org/wiki/Decibel .
//  (2) degrees is an angular measure, see
//        http://en.wikipedia.org/wiki/Degree_(angle) .
//  (3) low-pass filter is one that transmits (passes) low frequencies.
//  (4) pole is in the mathematical sense, see
//      http://en.wikipedia.org/wiki/Pole_(complex_analysis) .  "Single Pole"
//      means a particular mathematical transformation of the filter function has
//      a single pole, see
//      http://ccrma.stanford.edu/~jos/filters/Pole_Zero_Analysis_I.html .
//      Furthermore, a single-pole filter must have an inverse square decline
//      (or -20 db/decade). Since the filter plotted here does have that
//      characteristic, it must by definition be a single-pole filter, see also
//      http://www-k.ext.ti.com/SRVS/Data/ti/KnowledgeBases/analog/document/faqs/1p.htm
//  (5) decade represents a factor of 10, see
//      http://en.wikipedia.org/wiki/Decade_(log_scale) .


//--------------------------------------------------------------------------
// Implementation of PLplot example 26 in C++.
//--------------------------------------------------------------------------

#include "plc++demos.h"

#ifdef PL_USE_NAMESPACE
using namespace std;
#endif


static const char *x_label[] = {
    "Frequency",
    "Частота",
    NULL
};

static const char *y_label[] = {
    "Amplitude (dB)",
    "Амплитуда (dB)",
    NULL
};

static const char *alty_label[] = {
    "Phase shift (degrees)",
    "Фазовый сдвиг (градусы)",
    NULL
};

// Short rearranged versions of y_label and alty_label.
static const char *legend_text[][2] = {
    { "Amplitude", "Phase shift"   },
    { "Амплитуда", "Фазовый сдвиг" }
};

static const char *title_label[] = {
    "Single Pole Low-Pass Filter",
    "Однополюсный Низко-Частотный Фильтр",
    NULL
};

static const char *line_label[] = {
    "-20 dB/decade",
    "-20 dB/десяток",
    NULL
};

class x26 {
public:
    x26( int, char ** );
    void plot1( int, const char*, const char*, const char*, const char**, const char*, const char* );

private:
    // Class data
    plstream *pls;
};


x26::x26( int argc, char ** argv )
{
    int i;

    // plplot initialization

    pls = new plstream();

    // Parse and process command line arguments.
    pls->parseopts( &argc, argv, PL_PARSE_FULL );

    // Initialize PLplot.
    pls->init();
    pls->font( 2 );

    // Make log plots using two different styles.

    i = 0;
    while ( x_label[i] != NULL )
    {
        plot1( 0, x_label[i], y_label[i], alty_label[i], legend_text[i],
            title_label[i], line_label[i] );
        i++;
    }

    // pls->end();
    delete pls;
}

// Log-linear plot.

void x26::plot1( int type, const char *x_label, const char *y_label,
                 const char *alty_label, const char **legend_text,
                 const char *title_label, const char *line_label )
{
    int        i;
    PLFLT      *freql = new PLFLT[101];
    PLFLT      *ampl  = new PLFLT[101];
    PLFLT      *phase = new PLFLT[101];
    PLFLT      f0, freq;
    PLINT      nlegend = 2;
    PLINT      opt_array[2];
    PLINT      text_colors[2];
    PLINT      line_colors[2];
    PLINT      line_styles[2];
    PLFLT      line_widths[2];
    PLINT      symbol_numbers[2], symbol_colors[2];
    PLFLT      symbol_scales[2];
    const char *symbols[2];
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
    pls->ptex( 1.6, -30.0, 1.0, -20.0, 0.5, line_label );

    // Put labels on.

    pls->col0( 1 );
    pls->mtex( "b", 3.2, 0.5, 0.5, x_label );
    pls->mtex( "t", 2.0, 0.5, 0.5, title_label );
    pls->col0( 2 );
    pls->mtex( "l", 5.0, 0.5, 0.5, y_label );

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
        pls->mtex( "r", 5.0, 0.5, 0.5, alty_label );
    }

    // Draw a legend
    // First legend entry.
    opt_array[0]   = PL_LEGEND_LINE;
    text_colors[0] = 2;
    line_colors[0] = 2;
    line_styles[0] = 1;
    line_widths[0] = 1.0;
    // note from the above opt_array the first symbol (and box) indices
    // do not have to be specified

    // Second legend entry.
    opt_array[1]      = PL_LEGEND_LINE | PL_LEGEND_SYMBOL;
    text_colors[1]    = 3;
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
        0.0, 0.0, 0.10, 15,
        1, 1, 0, 0,
        nlegend, opt_array,
        1.0, 1.0, 2.0,
        1., text_colors, (const char * const *) legend_text,
        NULL, NULL, NULL, NULL,
        line_colors, line_styles, line_widths,
        symbol_colors, symbol_scales, symbol_numbers,
        (const char **) symbols );

    delete[] freql;
    delete[] ampl;
    delete[] phase;
}


int main( int argc, char ** argv )
{
    x26 *x = new x26( argc, argv );

    delete x;
}


//--------------------------------------------------------------------------
//                              End of x26.cc
//--------------------------------------------------------------------------
