//      Log plot demo.
//

#include "plcdemos.h"

void plot1( int type );

//--------------------------------------------------------------------------
// main
//
// Illustration of logarithmic axes, and redefinition of window.
//--------------------------------------------------------------------------

int
main( int argc, char *argv[] )
{
// Parse and process command line arguments

    (void) plparseopts( &argc, argv, PL_PARSE_FULL );

// Initialize plplot

    plinit();
    plfont( 2 );

// Make log plots using two different styles.

    plot1( 0 );
    plot1( 1 );

    plend();
    exit( 0 );
}

//--------------------------------------------------------------------------
// plot1
//
// Log-linear plot.
//--------------------------------------------------------------------------

void
plot1( int type )
{
    int           i;
    static PLFLT  freql[101], ampl[101], phase[101];
    PLFLT         f0, freq;
    PLINT         nlegend;
    PLCHAR_VECTOR text[2], symbols[2];
    PLINT         opt_array[2];
    PLINT         text_colors[2];
    PLINT         line_colors[2];
    PLINT         line_styles[2];
    PLFLT         line_widths[2];
    PLINT         symbol_numbers[2], symbol_colors[2];
    PLFLT         symbol_scales[2];
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
    plptex( 1.6, -30.0, 1.0, -20.0, 0.5, "-20 dB/decade" );

// Put labels on

    plcol0( 1 );
    plmtex( "b", 3.2, 0.5, 0.5, "Frequency" );
    plmtex( "t", 2.0, 0.5, 0.5, "Single Pole Low-Pass Filter" );
    plcol0( 2 );
    plmtex( "l", 5.0, 0.5, 0.5, "Amplitude (dB)" );
    nlegend = 1;

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
        plmtex( "r", 5.0, 0.5, 0.5, "Phase shift (degrees)" );
        nlegend = 2;
    }
    // Draw a legend
    // First legend entry.
    opt_array[0]   = PL_LEGEND_LINE;
    text_colors[0] = 2;
    text[0]        = "Amplitude";
    line_colors[0] = 2;
    line_styles[0] = 1;
    line_widths[0] = 1.;
    // note from the above opt_array the first symbol (and box) indices
    // do not have to be specified, at least in C. For Fortran we need
    // to set the symbols to be something, since the string is always
    // copied as part of the bindings.
    symbols[0] = "";

    // Second legend entry.
    opt_array[1]      = PL_LEGEND_LINE | PL_LEGEND_SYMBOL;
    text_colors[1]    = 3;
    text[1]           = "Phase shift";
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
        0.0, 0.0, 0.1, 15,
        1, 1, 0, 0,
        nlegend, opt_array,
        1.0, 1.0, 2.0,
        1., text_colors, (const char **) text,
        NULL, NULL, NULL, NULL,
        line_colors, line_styles, line_widths,
        symbol_colors, symbol_scales, symbol_numbers, (const char **) symbols );
}
