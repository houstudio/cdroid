//      Log plot demo.
//

import plplot;
import std.math;
import std.string;

//--------------------------------------------------------------------------
// main
//
// Illustration of logarithmic axes, and redefinition of window.
//--------------------------------------------------------------------------
int main( char[][] args )
{
    // Parse and process command line arguments
    plparseopts( args, PL_PARSE_FULL );

    // Initialize plplot
    plinit();
    plfont( 2 );

    // Make log plots using two different styles.
    plot1( 0 );
    plot1( 1 );

    plend();
    return 0;
}


//--------------------------------------------------------------------------
// plot1
//
// Log-linear plot.
//--------------------------------------------------------------------------
void plot1( int type )
{
    PLFLT[101] freql, ampl, phase;
    PLFLT    f0, freq;
    PLINT    nlegend;
    string[] text, symbols;
    PLINT[]  opt_array;
    PLINT[]  text_colors;
    PLINT[]  box_colors;
    PLINT[]  box_patterns;
    PLFLT[]  box_scales;
    PLFLT[]  box_line_widths;
    PLINT[]  line_colors;
    PLINT[]  line_styles;
    PLFLT[]  line_widths;
    PLINT[]  symbol_numbers, symbol_colors;
    PLFLT[]  symbol_scales;
    PLFLT    legend_width, legend_height;


    pladv( 0 );

    // Set up data for log plot
    f0 = 1.0;
    for ( size_t i = 0; i <= 100; i++ )
    {
        freql[i] = -2.0 + i / 20.0;
        freq     = pow( 10.0, freql[i] );
        ampl[i]  = 20.0 * log10( 1.0 / sqrt( 1.0 + pow( ( freq / f0 ), 2.0 ) ) );
        phase[i] = -( 180.0 / PI ) * atan( freq / f0 );
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
    default:
        break;
    }

    // Plot ampl vs freq
    plcol0( 2 );
    plline( freql, ampl );
    plcol0( 2 );
    plptex( 1.6, -30.0, 1.0, -20.0, 0.5, "-20 dB/decade" );

    // Put labels on
    plcol0( 1 );
    plmtex( "b", 3.2, 0.5, 0.5, "Frequency" );
    plmtex( "t", 2.0, 0.5, 0.5, "Single Pole Low-Pass Filter" );
    plcol0( 2 );
    plmtex( "l", 5.0, 0.5, 0.5, "Amplitude (dB)" );

    // For the gridless case, put phase vs freq on same plot
    if ( type == 0 )
    {
        plcol0( 1 );
        plwind( -2.0, 3.0, -100.0, 0.0 );
        plbox( "", 0.0, 0, "cmstv", 30.0, 3 );
        plcol0( 3 );
        plline( freql, phase );
        plstring( freql, phase, "#(728)" );

        plcol0( 3 );
        plmtex( "r", 5.0, 0.5, 0.5, "Phase shift (degrees)" );
        nlegend = 2;
    }
    else
    {
        nlegend = 1;
    }
    // Initialize arrays needed for pllegend.
    opt_array.length       = nlegend;
    text_colors.length     = nlegend;
    text.length            = nlegend;
    line_colors.length     = nlegend;
    line_styles.length     = nlegend;
    line_widths.length     = nlegend;
    box_colors.length      = nlegend;
    box_patterns.length    = nlegend;
    box_scales.length      = nlegend;
    box_line_widths.length = nlegend;
    symbol_numbers.length  = nlegend;
    symbol_colors.length   = nlegend;
    symbol_scales.length   = nlegend;
    symbols.length         = nlegend;

    // Draw a legend
    // First legend entry.
    opt_array[0]   = PL_LEGEND_LINE;
    text_colors[0] = 2;
    text[0]        = "Amplitude";
    line_colors[0] = 2;
    line_styles[0] = 1;
    line_widths[0] = 1.;
    // Note from the above opt_array the first symbol (and box) indices
    // do not have to be specified

    if ( nlegend == 2 )
    {
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
        // Note from the above opt_array the second box array indices
        // do not have to be specified
    }
    plscol0a( 15, 32, 32, 32, 0.70 );
    pllegend( &legend_width, &legend_height,
        PL_LEGEND_BACKGROUND | PL_LEGEND_BOUNDING_BOX, 0,
        0.0, 0.0, 0.1, 15,
        1, 1, 0, 0,
        opt_array,
        1.0, 1.0, 2.0,
        1., text_colors, text,
        box_colors, box_patterns, box_scales, box_line_widths,
        line_colors, line_styles, line_widths,
        symbol_colors, symbol_scales, symbol_numbers, symbols );
}
