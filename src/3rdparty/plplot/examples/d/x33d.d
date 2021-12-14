// Demonstrate most pllegend capability including unicode symbols.
//
// Copyright (C) 2010 Alan W. Irwin
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

// This example designed just for devices (e.g., the cairo-related and
// qt-related devices) where the best choice of glyph is automatically
// selected by the related libraries (pango/cairo or Qt4) for each
// unicode character depending on what system fonts are installed.  Of
// course, you must have the appropriate TrueType fonts installed to
// have access to all the required glyphs.

import plplot;
import std.math;
import std.string;

static PLINT position_options[16] = [
    PL_POSITION_LEFT | PL_POSITION_TOP | PL_POSITION_OUTSIDE,
    PL_POSITION_TOP | PL_POSITION_OUTSIDE,
    PL_POSITION_RIGHT | PL_POSITION_TOP | PL_POSITION_OUTSIDE,
    PL_POSITION_RIGHT | PL_POSITION_OUTSIDE,
    PL_POSITION_RIGHT | PL_POSITION_BOTTOM | PL_POSITION_OUTSIDE,
    PL_POSITION_BOTTOM | PL_POSITION_OUTSIDE,
    PL_POSITION_LEFT | PL_POSITION_BOTTOM | PL_POSITION_OUTSIDE,
    PL_POSITION_LEFT | PL_POSITION_OUTSIDE,
    PL_POSITION_LEFT | PL_POSITION_TOP | PL_POSITION_INSIDE,
    PL_POSITION_TOP | PL_POSITION_INSIDE,
    PL_POSITION_RIGHT | PL_POSITION_TOP | PL_POSITION_INSIDE,
    PL_POSITION_RIGHT | PL_POSITION_INSIDE,
    PL_POSITION_RIGHT | PL_POSITION_BOTTOM | PL_POSITION_INSIDE,
    PL_POSITION_BOTTOM | PL_POSITION_INSIDE,
    PL_POSITION_LEFT | PL_POSITION_BOTTOM | PL_POSITION_INSIDE,
    PL_POSITION_LEFT | PL_POSITION_INSIDE
];

// Pick 5 arbitrary UTF-8 symbols useful for plotting points (✠✚✱✪✽✺✰✴✦).
static string special_symbols[5] = [
    "✰",
    "✴",
    "✱",
    "✽",
    "✦"
];

// plcolorbar options

// Colorbar type options
const int    COLORBAR_KINDS = 4;
static PLINT colorbar_option_kinds[COLORBAR_KINDS] = [
    PL_COLORBAR_SHADE,
    PL_COLORBAR_SHADE | PL_COLORBAR_SHADE_LABEL,
    PL_COLORBAR_IMAGE,
    PL_COLORBAR_GRADIENT
];
static string colorbar_option_kind_labels[COLORBAR_KINDS] = [
    "Shade colorbars",
    "Shade colorbars with custom labels",
    "Image colorbars",
    "Gradient colorbars"
];

// Which side of the page are we positioned relative to?
const int    COLORBAR_POSITIONS = 4;
static PLINT colorbar_position_options[COLORBAR_POSITIONS] = [
    PL_POSITION_LEFT,
    PL_POSITION_RIGHT,
    PL_POSITION_TOP,
    PL_POSITION_BOTTOM
];
static string colorbar_position_option_labels[COLORBAR_POSITIONS] = [
    "Left",
    "Right",
    "Top",
    "Bottom"
];

// Colorbar label positioning options
const int    COLORBAR_LABELS = 4;
static PLINT colorbar_label_options[COLORBAR_LABELS] = [
    PL_COLORBAR_LABEL_LEFT,
    PL_COLORBAR_LABEL_RIGHT,
    PL_COLORBAR_LABEL_TOP,
    PL_COLORBAR_LABEL_BOTTOM
];
static string colorbar_label_option_labels[COLORBAR_LABELS] = [
    "Label left",
    "Label right",
    "Label top",
    "Label bottom"
];

// Colorbar cap options
const int    COLORBAR_CAPS = 4;
static PLINT colorbar_cap_options[COLORBAR_CAPS] = [
    PL_COLORBAR_CAP_NONE,
    PL_COLORBAR_CAP_LOW,
    PL_COLORBAR_CAP_HIGH,
    PL_COLORBAR_CAP_LOW | PL_COLORBAR_CAP_HIGH
];
static string colorbar_cap_option_labels[COLORBAR_CAPS] = [
    "No caps",
    "Low cap",
    "High cap",
    "Low and high caps"
];

static int colorbar = 1;           // By default do not plot plcolorbar pages
                                   // for now while we are working out the API.
void
plcolorbar_example_page( int kind_i, int label_i, int cap_i, PLINT cont_color, PLFLT cont_width, PLINT n_values, PLFLT [] values )
{
    // Parameters for the colorbars on this page
    PLINT     position_i, position, opt;
    PLFLT     x, y, x_length, y_length;
    PLFLT     ticks[1]     = [ 0.0 ];
    PLINT     sub_ticks[1] = [ 0 ];
    PLFLT     low_cap_color, high_cap_color;
    PLINT     vertical, ifn;
    PLINT     n_axes = 1;
    string[]  axis_opts;
    PLINT     n_labels      = 1;
    PLINT     label_opts[1] = [ 0 ];
    string[]  label;
    string    title;
    PLFLT     colorbar_width, colorbar_height;
    PLINT     n_values_array[1];
    PLFLT[][] values_array;

    axis_opts.length  = 1;
    label.length      = 1;
    values_array      = new PLFLT[][1];
    n_values_array[0] = n_values;
    values_array[0]   = values;

    low_cap_color  = 0.0;
    high_cap_color = 1.0;

    // Start a new page
    pladv( 0 );

    // Draw one colorbar relative to each side of the page
    for ( position_i = 0; position_i < COLORBAR_POSITIONS; position_i++ )
    {
        position = colorbar_position_options[position_i];
        opt      =
            colorbar_option_kinds[kind_i] |
            colorbar_label_options[label_i] |
            colorbar_cap_options[cap_i];

        vertical = position & PL_POSITION_LEFT || position & PL_POSITION_RIGHT;
        ifn      = position & PL_POSITION_LEFT || position & PL_POSITION_BOTTOM;

        // Set the offset position on the page
        if ( vertical )
        {
            x        = 0.0;
            y        = 0.0;
            x_length = 0.05;
            y_length = 0.5;
        }
        else
        {
            x        = 0.0;
            y        = 0.0;
            x_length = 0.5;
            y_length = 0.05;
        }

        // Set appropriate labelling options.
        if ( ifn )
        {
            if ( cont_color == 0 || cont_width == 0. )
            {
                axis_opts[0] = "uwtivn";
                //axis_opts[0] = "uwtin";
            }
            else
            {
                axis_opts[0] = "uwxvn";
                //axis_opts[0] = "uwxn";
            }
        }
        else
        {
            if ( cont_color == 0 || cont_width == 0. )
            {
                axis_opts[0] = "uwtivm";
                //axis_opts[0] = "uwtim";
            }
            else
            {
                axis_opts[0] = "uwxvm";
                //axis_opts[0] = "uwxm";
            }
        }

        label[0] = format( "%s, %s",
            colorbar_position_option_labels[position_i],
            colorbar_label_option_labels[label_i] );

        // Smaller text
        plschr( 0.0, 0.75 );
        // Small ticks on the vertical axis
        plsmaj( 0.0, 0.5 );
        plsmin( 0.0, 0.5 );

        plvpor( 0.20, 0.80, 0.20, 0.80 );
        plwind( 0.0, 1.0, 0.0, 1.0 );
        // Set interesting background colour.
        plscol0a( 15, 0, 0, 0, 0.20 );
        plcolorbar( &colorbar_width, &colorbar_height,
            opt | PL_COLORBAR_BOUNDING_BOX | PL_COLORBAR_BACKGROUND, position,
            x, y, x_length, y_length,
            15, 1, 1,
            low_cap_color, high_cap_color,
            cont_color, cont_width,
            label_opts, label,
            axis_opts,
            ticks, sub_ticks,
            values_array );

        // Reset text and tick sizes
        plschr( 0.0, 1.0 );
        plsmaj( 0.0, 1.0 );
        plsmin( 0.0, 1.0 );
    }

    // Draw a page title
    title = format( "%s - %s",
        colorbar_option_kind_labels[kind_i],
        colorbar_cap_option_labels[cap_i] );
    plvpor( 0.0, 1.0, 0.0, 1.0 );
    plwind( 0.0, 1.0, 0.0, 1.0 );
    plptex( 0.5, 0.5, 0.0, 0.0, 0.5, title );
}

void
plcolorbar_example( string palette, int kind_i, PLINT cont_color, PLFLT cont_width, PLINT n_values, PLFLT [] values )
{
    int label_i, cap_i;

    // Load the color palette
    plspal1( palette, 1 );

    for ( label_i = 0; label_i < COLORBAR_LABELS; label_i++ )
    {
        for ( cap_i = 0; cap_i < COLORBAR_CAPS; cap_i++ )
        {
            plcolorbar_example_page( kind_i, label_i, cap_i,
                cont_color, cont_width,
                n_values, values );
        }
    }
}

//--------------------------------------------------------------------------
// main
//
// Demonstrate most pllegend capability including unicode symbols.
//--------------------------------------------------------------------------

const int MAX_NLEGEND = 7;

int main( char[][] args )
{
    int    i, k;
    PLINT  opt;
    PLINT  nlegend, nturn;
    PLINT  opt_array[MAX_NLEGEND];
    PLINT  text_colors[MAX_NLEGEND];
    PLINT  box_colors[MAX_NLEGEND];
    PLINT  box_patterns[MAX_NLEGEND];
    PLFLT  box_scales[MAX_NLEGEND];
    PLFLT  box_line_widths[MAX_NLEGEND];
    PLINT  line_colors[MAX_NLEGEND];
    PLINT  line_styles[MAX_NLEGEND];
    PLFLT  line_widths[MAX_NLEGEND];
    PLINT  symbol_numbers[MAX_NLEGEND];
    PLINT  symbol_colors[MAX_NLEGEND];
    PLFLT  symbol_scales[MAX_NLEGEND];
    string text[MAX_NLEGEND];
    string symbols[MAX_NLEGEND];
    PLFLT  legend_width, legend_height, x, y, xstart, ystart;
    PLFLT  max_height, text_scale;
    PLINT  position, opt_base, nrow, ncolumn;

    // Parse and process command line arguments
    plparseopts( args, PL_PARSE_FULL );

// Initialize plplot
    plinit();

    // First page illustrating the 16 standard positions.
    pladv( 0 );
    plvpor( 0.25, 0.75, 0.25, 0.75 );
    plwind( 0.0, 1.0, 0.0, 1.0 );
    plbox( "bc", 0.0, 0, "bc", 0.0, 0 );
    plsfont( PL_FCI_SANS, -1, -1 );
    plmtex( "t", 8.0, 0.5, 0.5, "The 16 standard legend positions with" );
    plmtex( "t", 6.0, 0.5, 0.5, "the same (0.05) offset in x and y" );

    nlegend = 1;
    // Only specify legend data that are required according to the
    // value of opt_array for that entry.
    opt_base          = PL_LEGEND_BACKGROUND | PL_LEGEND_BOUNDING_BOX;
    opt_array[0]      = PL_LEGEND_LINE | PL_LEGEND_SYMBOL;
    line_styles[0]    = 1;
    line_widths[0]    = 1.;
    symbol_scales[0]  = 1.;
    symbol_numbers[0] = 4;
    symbols[0]        = "#(728)";

    // Use monotype fonts so that all legends are the same size.
    plsfont( PL_FCI_MONO, -1, -1 );
    plscol0a( 15, 32, 32, 32, 0.70 );

    for ( k = 0; k < 16; k++ )
    {
        position         = position_options[k];
        opt              = opt_base;
        text[0]          = format( "%2.2d", k );
        text_colors[0]   = 1 + ( k % 8 );
        line_colors[0]   = 1 + ( k % 8 );
        symbol_colors[0] = 1 + ( k % 8 );

        pllegend( &legend_width, &legend_height, opt, position, 0.05, 0.05,
            0.1, 15, 1, 1, 0, 0,
            opt_array[0..nlegend], 1.0, 1.0, 2.0,
            1., text_colors[0..nlegend], text[0..nlegend],
            null, null, null, null,
            line_colors[0..nlegend], line_styles[0..nlegend], line_widths[0..nlegend],
            symbol_colors[0..nlegend], symbol_scales[0..nlegend], symbol_numbers[0..nlegend], symbols[0..nlegend] );
    }

    // Second page illustrating effect of nrow, ncolumn for the same legend
    // data.;
    pladv( 0 );
    plvpor( 0.25, 0.75, 0.25, 0.75 );
    plwind( 0.0, 1.0, 0.0, 1.0 );
    plbox( "bc", 0.0, 0, "bc", 0.0, 0 );
    plsfont( PL_FCI_SANS, -1, -1 );
    plmtex( "t", 8.0, 0.5, 0.5, "The effect of nrow, ncolumn, PL_LEGEND_ROW_MAJOR," );
    plmtex( "t", 6.0, 0.5, 0.5, "and position for the same legend data" );

    nlegend = 7;

    // Only specify legend data that are required according to the
    // value of opt_array for that entry.
    opt_base = PL_LEGEND_BACKGROUND | PL_LEGEND_BOUNDING_BOX;
    for ( k = 0; k < nlegend; k++ )
    {
        opt_array[k]      = PL_LEGEND_LINE | PL_LEGEND_SYMBOL;
        line_styles[k]    = 1;
        line_widths[k]    = 1.;
        symbol_scales[k]  = 1.;
        symbol_numbers[k] = 2;
        symbols[k]        = "#(728)";
        text[k]           = format( "%2.2d", k );
        text_colors[k]    = 1 + ( k % 8 );
        line_colors[k]    = 1 + ( k % 8 );
        symbol_colors[k]  = 1 + ( k % 8 );
    }

    // Use monotype fonts so that all legends are the same size.
    plsfont( PL_FCI_MONO, -1, -1 );
    plscol0a( 15, 32, 32, 32, 0.70 );

    position = PL_POSITION_TOP | PL_POSITION_OUTSIDE;
    opt      = opt_base;
    x        = 0.;
    y        = 0.1;
    nrow     = 1;
    ncolumn  = nlegend;
    pllegend( &legend_width, &legend_height, opt, position, x, y,
        0.05, 15, 1, 1, nrow, ncolumn,
        opt_array[0..nlegend], 1.0, 1.0, 2.0,
        1., text_colors[0..nlegend], text[0..nlegend],
        null, null, null, null,
        line_colors[0..nlegend], line_styles[0..nlegend], line_widths[0..nlegend],
        symbol_colors[0..nlegend], symbol_scales[0..nlegend], symbol_numbers[0..nlegend], symbols[0..nlegend] );

    position = PL_POSITION_BOTTOM | PL_POSITION_OUTSIDE;
    opt      = opt_base;
    x        = 0.;
    y        = 0.1;
    nrow     = 1;
    ncolumn  = nlegend;
    pllegend( &legend_width, &legend_height, opt, position, x, y,
        0.05, 15, 1, 1, nrow, ncolumn,
        opt_array[0..nlegend], 1.0, 1.0, 2.0,
        1., text_colors[0..nlegend], text[0..nlegend],
        null, null, null, null,
        line_colors[0..nlegend], line_styles[0..nlegend], line_widths[0..nlegend],
        symbol_colors[0..nlegend], symbol_scales[0..nlegend], symbol_numbers[0..nlegend], symbols[0..nlegend] );

    position = PL_POSITION_LEFT | PL_POSITION_OUTSIDE;
    opt      = opt_base;
    x        = 0.1;
    y        = 0.;
    nrow     = nlegend;
    ncolumn  = 1;
    pllegend( &legend_width, &legend_height, opt, position, x, y,
        0.05, 15, 1, 1, nrow, ncolumn,
        opt_array[0..nlegend], 1.0, 1.0, 2.0,
        1., text_colors[0..nlegend], text[0..nlegend],
        null, null, null, null,
        line_colors[0..nlegend], line_styles[0..nlegend], line_widths[0..nlegend],
        symbol_colors[0..nlegend], symbol_scales[0..nlegend], symbol_numbers[0..nlegend], symbols[0..nlegend] );

    position = PL_POSITION_RIGHT | PL_POSITION_OUTSIDE;
    opt      = opt_base;
    x        = 0.1;
    y        = 0.;
    nrow     = nlegend;
    ncolumn  = 1;
    pllegend( &legend_width, &legend_height, opt, position, x, y,
        0.05, 15, 1, 1, nrow, ncolumn,
        opt_array[0..nlegend], 1.0, 1.0, 2.0,
        1., text_colors[0..nlegend], text[0..nlegend],
        null, null, null, null,
        line_colors[0..nlegend], line_styles[0..nlegend], line_widths[0..nlegend],
        symbol_colors[0..nlegend], symbol_scales[0..nlegend], symbol_numbers[0..nlegend], symbols[0..nlegend] );

    position = PL_POSITION_LEFT | PL_POSITION_TOP | PL_POSITION_INSIDE;
    opt      = opt_base;
    x        = 0.;
    y        = 0.;
    nrow     = 6;
    ncolumn  = 2;
    pllegend( &legend_width, &legend_height, opt, position, x, y,
        0.05, 15, 1, 1, nrow, ncolumn,
        opt_array[0..nlegend], 1.0, 1.0, 2.0,
        1., text_colors[0..nlegend], text[0..nlegend],
        null, null, null, null,
        line_colors[0..nlegend], line_styles[0..nlegend], line_widths[0..nlegend],
        symbol_colors[0..nlegend], symbol_scales[0..nlegend], symbol_numbers[0..nlegend], symbols[0..nlegend] );

    position = PL_POSITION_RIGHT | PL_POSITION_TOP | PL_POSITION_INSIDE;
    opt      = opt_base | PL_LEGEND_ROW_MAJOR;
    x        = 0.;
    y        = 0.;
    nrow     = 6;
    ncolumn  = 2;
    pllegend( &legend_width, &legend_height, opt, position, x, y,
        0.05, 15, 1, 1, nrow, ncolumn,
        opt_array[0..nlegend], 1.0, 1.0, 2.0,
        1., text_colors[0..nlegend], text[0..nlegend],
        null, null, null, null,
        line_colors[0..nlegend], line_styles[0..nlegend], line_widths[0..nlegend],
        symbol_colors[0..nlegend], symbol_scales[0..nlegend], symbol_numbers[0..nlegend], symbols[0..nlegend] );

    position = PL_POSITION_BOTTOM | PL_POSITION_INSIDE;
    opt      = opt_base | PL_LEGEND_ROW_MAJOR;
    x        = 0.;
    y        = 0.;
    nrow     = 3;
    ncolumn  = 3;
    pllegend( &legend_width, &legend_height, opt, position, x, y,
        0.05, 15, 1, 1, nrow, ncolumn,
        opt_array[0..nlegend], 1.0, 1.0, 2.0,
        1., text_colors[0..nlegend], text[0..nlegend],
        null, null, null, null,
        line_colors[0..nlegend], line_styles[0..nlegend], line_widths[0..nlegend],
        symbol_colors[0..nlegend], symbol_scales[0..nlegend], symbol_numbers[0..nlegend], symbols[0..nlegend] );

    // Third page demonstrating legend alignment
    pladv( 0 );
    plvpor( 0.0, 1.0, 0.0, 0.9 );
    plwind( 0.0, 1.0, 0.0, 1.0 );
    plsfont( PL_FCI_SANS, -1, -1 );
    plmtex( "t", 2.0, 0.5, 0.5, "Demonstrate legend alignment" );

    x        = 0.1;
    y        = 0.1;
    nturn    = 4;
    nlegend  = 0;
    position = PL_POSITION_TOP | PL_POSITION_LEFT | PL_POSITION_SUBPAGE;
    opt_base = PL_LEGEND_BACKGROUND | PL_LEGEND_BOUNDING_BOX;
    opt      = opt_base;
    for ( i = 0; i < 9; i++ )
    {
        // Set up legend arrays with the correct size, type.
        if ( i <= nturn )
            nlegend += 1;
        else
            nlegend -= 1;
        nlegend = cast(int) fmax( 1, nlegend );
        // nly specify legend data that are required according to the
        //  value of opt_array for that entry.
        for ( k = 0; k < nlegend; k++ )
        {
            opt_array[k]      = PL_LEGEND_LINE | PL_LEGEND_SYMBOL;
            line_styles[k]    = 1;
            line_widths[k]    = 1.;
            symbol_scales[k]  = 1.;
            symbol_numbers[k] = 2;
            symbols[k]        = "#(728)";
            text[k]           = format( "%2.2d", k );
            text_colors[k]    = 1 + ( k % 8 );
            line_colors[k]    = 1 + ( k % 8 );
            symbol_colors[k]  = 1 + ( k % 8 );
        }
        // Use monotype fonts so that all legends are the same size.
        plsfont( PL_FCI_MONO, -1, -1 );
        plscol0a( 15, 32, 32, 32, 0.70 );

        nrow    = cast(int) fmin( 3, nlegend );
        ncolumn = 0;

        pllegend( &legend_width, &legend_height, opt, position, x, y,
            0.025, 15, 1, 1, nrow, ncolumn,
            opt_array[0..nlegend], 1.0, 1.0, 1.5,
            1., text_colors[0..nlegend], text[0..nlegend],
            null, null, null, null,
            line_colors[0..nlegend], line_styles[0..nlegend], line_widths[0..nlegend],
            symbol_colors[0..nlegend], symbol_scales[0..nlegend], symbol_numbers[0..nlegend], symbols[0..nlegend] );

        if ( i == nturn )
        {
            position = PL_POSITION_TOP | PL_POSITION_RIGHT | PL_POSITION_SUBPAGE;
            opt      = opt_base;
            x        = 1. - x;
            y       += legend_height;
        }
        else
        {
            x += legend_width;
            y += legend_height;
        }
    }

    // Fourth page illustrating various kinds of legends
    max_height = 0.;
    xstart     = 0.0;
    ystart     = 0.1;
    x          = xstart;
    y          = ystart;
    text_scale = 0.90;
    pladv( 0 );
    plvpor( 0.0, 1., 0.0, 0.90 );
    plwind( 0.0, 1.0, 0.0, 1.0 );
    //plbox("bc", 0.0, 0, "bc", 0.0, 0);
    plsfont( PL_FCI_SANS, -1, -1 );
    plmtex( "t", 2.0, 0.5, 0.5, "Demonstrate Various Kinds of Legends" );

    nlegend = 5;
    // Only specify legend data that are required according to the
    // value of opt_array for that entry.
    position = PL_POSITION_LEFT | PL_POSITION_TOP;
    opt_base = PL_LEGEND_BACKGROUND | PL_LEGEND_BOUNDING_BOX | PL_LEGEND_TEXT_LEFT;

    // Set up None, Box, Line, Symbol, and Line & Symbol legend entries.
    opt_array[0]   = PL_LEGEND_NONE;
    text[0]        = format( "%s", "None" );
    text_colors[0] = 1;

    opt_array[1]       = PL_LEGEND_COLOR_BOX;
    text[1]            = format( "%s", "Box" );
    text_colors[1]     = 2;
    box_colors[1]      = 2;
    box_patterns[1]    = 0;
    box_scales[1]      = 0.8;
    box_line_widths[1] = 1.;

    opt_array[2]   = PL_LEGEND_LINE;
    text[2]        = format( "%s", "Line" );
    text_colors[2] = 3;
    line_colors[2] = 3;
    line_styles[2] = 1;
    line_widths[2] = 1.;

    opt_array[3]      = PL_LEGEND_SYMBOL;
    text[3]           = format( "%s", "Symbol" );
    text_colors[3]    = 4;
    symbol_colors[3]  = 4;
    symbol_scales[3]  = text_scale;
    symbol_numbers[3] = 4;
    symbols[3]        = special_symbols[2];

    opt_array[4]      = PL_LEGEND_SYMBOL | PL_LEGEND_LINE;
    text[4]           = format( "%s", "L & S" );
    text_colors[4]    = 5;
    line_colors[4]    = 5;
    line_styles[4]    = 1;
    line_widths[4]    = 1.;
    symbol_colors[4]  = 5;
    symbol_scales[4]  = text_scale;
    symbol_numbers[4] = 4;
    symbols[4]        = special_symbols[2];

    opt = opt_base;
    plscol0a( 15, 32, 32, 32, 0.70 );

    pllegend( &legend_width, &legend_height, opt, position, x, y,
        0.1, 15, 1, 1, 0, 0,
        opt_array[0..nlegend], 1.0, text_scale, 2.0,
        0., text_colors[0..nlegend], text[0..nlegend],
        box_colors[0..nlegend], box_patterns[0..nlegend], box_scales[0..nlegend], box_line_widths[0..nlegend],
        line_colors[0..nlegend], line_styles[0..nlegend], line_widths[0..nlegend],
        symbol_colors[0..nlegend], symbol_scales[0..nlegend], symbol_numbers[0..nlegend], symbols[0..nlegend] );
    max_height = fmax( max_height, legend_height );

    // Set up symbol legend entries with various symbols.
    for ( i = 0; i < nlegend; i++ )
    {
        opt_array[i]      = PL_LEGEND_SYMBOL;
        text[i]           = format( "%s%s", "Symbol ", special_symbols[i] );
        text_colors[i]    = i + 1;
        symbol_colors[i]  = i + 1;
        symbol_scales[i]  = text_scale;
        symbol_numbers[i] = 4;
        symbols[i]        = special_symbols[i];
    }

    opt = opt_base;
    x  += legend_width;
    plscol0a( 15, 32, 32, 32, 0.70 );

    pllegend( &legend_width, &legend_height, opt, position, x, y,
        0.1, 15, 1, 1, 0, 0,
        opt_array[0..nlegend], 1.0, text_scale, 2.0,
        0., text_colors[0..nlegend], text[0..nlegend],
        null, null, null, null,
        null, null, null,
        symbol_colors[0..nlegend], symbol_scales[0..nlegend], symbol_numbers[0..nlegend], symbols[0..nlegend] );
    max_height = fmax( max_height, legend_height );

    // Set up symbol legend entries with various numbers of symbols.
    for ( i = 0; i < nlegend; i++ )
    {
        opt_array[i]      = PL_LEGEND_SYMBOL;
        text[i]           = format( "%s %d", "Symbol Number", i + 2 );
        text_colors[i]    = i + 1;
        symbol_colors[i]  = i + 1;
        symbol_scales[i]  = text_scale;
        symbol_numbers[i] = i + 2;
        symbols[i]        = special_symbols[2];
    }

    opt = opt_base;
    x  += legend_width;
    plscol0a( 15, 32, 32, 32, 0.70 );

    pllegend( &legend_width, &legend_height, opt, position, x, y,
        0.1, 15, 1, 1, 0, 0,
        opt_array[0..nlegend], 1.0, text_scale, 2.0,
        0., text_colors[0..nlegend], text[0..nlegend],
        null, null, null, null,
        null, null, null,
        symbol_colors[0..nlegend], symbol_scales[0..nlegend], symbol_numbers[0..nlegend], symbols[0..nlegend] );
    max_height = fmax( max_height, legend_height );

    // Set up box legend entries with various colours.
    for ( i = 0; i < nlegend; i++ )
    {
        opt_array[i]       = PL_LEGEND_COLOR_BOX;
        text[i]            = format( "%s %d", "Box Color", i + 1 );
        text_colors[i]     = i + 1;
        box_colors[i]      = i + 1;
        box_patterns[i]    = 0;
        box_scales[i]      = 0.8;
        box_line_widths[i] = 1.;
    }

    opt = opt_base;
    // Use new origin
    x          = xstart;
    y         += max_height;
    max_height = 0.;
    plscol0a( 15, 32, 32, 32, 0.70 );

    pllegend( &legend_width, &legend_height, opt, position, x, y,
        0.1, 15, 1, 1, 0, 0,
        opt_array[0..nlegend], 1.0, text_scale, 2.0,
        0., text_colors[0..nlegend], text[0..nlegend],
        box_colors[0..nlegend], box_patterns[0..nlegend], box_scales[0..nlegend], box_line_widths[0..nlegend],
        null, null, null,
        null, null, null, null );
    max_height = fmax( max_height, legend_height );

    // Set up box legend entries with various patterns.
    for ( i = 0; i < nlegend; i++ )
    {
        opt_array[i]       = PL_LEGEND_COLOR_BOX;
        text[i]            = format( "%s %d", "Box Pattern", i );
        text_colors[i]     = 2;
        box_colors[i]      = 2;
        box_patterns[i]    = i;
        box_scales[i]      = 0.8;
        box_line_widths[i] = 1.;
    }

    opt = opt_base;
    x  += legend_width;
    plscol0a( 15, 32, 32, 32, 0.70 );

    pllegend( &legend_width, &legend_height, opt, position, x, y,
        0.1, 15, 1, 1, 0, 0,
        opt_array[0..nlegend], 1.0, text_scale, 2.0,
        0., text_colors[0..nlegend], text[0..nlegend],
        box_colors[0..nlegend], box_patterns[0..nlegend], box_scales[0..nlegend], box_line_widths[0..nlegend],
        null, null, null,
        null, null, null, null );
    max_height = fmax( max_height, legend_height );

    // Set up box legend entries with various box pattern line widths.
    for ( i = 0; i < nlegend; i++ )
    {
        opt_array[i]       = PL_LEGEND_COLOR_BOX;
        text[i]            = format( "%s %d", "Box Line Width", i + 1 );
        text_colors[i]     = 2;
        box_colors[i]      = 2;
        box_patterns[i]    = 3;
        box_scales[i]      = 0.8;
        box_line_widths[i] = cast(PLFLT) ( i + 1 );
    }

    opt = opt_base;
    x  += legend_width;
    plscol0a( 15, 32, 32, 32, 0.70 );

    pllegend( &legend_width, &legend_height, opt, position, x, y,
        0.1, 15, 1, 1, 0, 0,
        opt_array[0..nlegend], 1.0, text_scale, 2.0,
        0., text_colors[0..nlegend], text[0..nlegend],
        box_colors[0..nlegend], box_patterns[0..nlegend], box_scales[0..nlegend], box_line_widths[0..nlegend],
        null, null, null,
        null, null, null, null );
    max_height = fmax( max_height, legend_height );

    // Set up line legend entries with various colours.
    for ( i = 0; i < nlegend; i++ )
    {
        opt_array[i]   = PL_LEGEND_LINE;
        text[i]        = format( "%s %d", "Line Color", i + 1 );
        text_colors[i] = i + 1;
        line_colors[i] = i + 1;
        line_styles[i] = 1;
        line_widths[i] = 1.;
    }

    opt = opt_base;
    // Use new origin
    x          = xstart;
    y         += max_height;
    max_height = 0.;
    plscol0a( 15, 32, 32, 32, 0.70 );

    pllegend( &legend_width, &legend_height, opt, position, x, y,
        0.1, 15, 1, 1, 0, 0,
        opt_array[0..nlegend], 1.0, text_scale, 2.0,
        0., text_colors[0..nlegend], text[0..nlegend],
        null, null, null, null,
        line_colors[0..nlegend], line_styles[0..nlegend], line_widths[0..nlegend],
        null, null, null, null );
    max_height = fmax( max_height, legend_height );

    // Set up line legend entries with various styles.
    for ( i = 0; i < nlegend; i++ )
    {
        opt_array[i]   = PL_LEGEND_LINE;
        text[i]        = format( "%s %d", "Line Style", i + 1 );
        text_colors[i] = 2;
        line_colors[i] = 2;
        line_styles[i] = i + 1;
        line_widths[i] = 1.;
    }

    opt = opt_base;
    x  += legend_width;
    plscol0a( 15, 32, 32, 32, 0.70 );

    pllegend( &legend_width, &legend_height, opt, position, x, y,
        0.1, 15, 1, 1, 0, 0,
        opt_array[0..nlegend], 1.0, text_scale, 2.0,
        0., text_colors[0..nlegend], text[0..nlegend],
        null, null, null, null,
        line_colors[0..nlegend], line_styles[0..nlegend], line_widths[0..nlegend],
        null, null, null, null );
    max_height = fmax( max_height, legend_height );

    // Set up line legend entries with various widths.
    for ( i = 0; i < nlegend; i++ )
    {
        opt_array[i]   = PL_LEGEND_LINE;
        text[i]        = format( "%s %d", "Line Width", i + 1 );
        text_colors[i] = 2;
        line_colors[i] = 2;
        line_styles[i] = 1;
        line_widths[i] = cast(PLFLT) ( i + 1 );
    }

    opt = opt_base;
    x  += legend_width;
    plscol0a( 15, 32, 32, 32, 0.70 );

    pllegend( &legend_width, &legend_height, opt, position, x, y,
        0.1, 15, 1, 1, 0, 0,
        opt_array[0..nlegend], 1.0, text_scale, 2.0,
        0., text_colors[0..nlegend], text[0..nlegend],
        null, null, null, null,
        line_colors[0..nlegend], line_styles[0..nlegend], line_widths[0..nlegend],
        null, null, null, null );
    max_height = fmax( max_height, legend_height );

    if ( colorbar )
    {
        // Color bar examples
        PLFLT values_small[2]  = [ -1.0e-20, 1.0e-20 ];
        PLFLT values_uneven[9] = [ -1.0e-20, 2.0e-20, 2.6e-20, 3.4e-20, 6.0e-20, 7.0e-20, 8.0e-20, 9.0e-20, 10.0e-20 ];
        PLFLT values_even[9]   = [ -2.0e-20, -1.0e-20, 0.0e-20, 1.0e-20, 2.0e-20, 3.0e-20, 4.0e-20, 5.0e-20, 6.0e-20 ];

        // Use unsaturated green background colour to contrast with black caps.
        plscolbg( 70, 185, 70 );
        // Cut out the greatest and smallest bits of the color spectrum to
        // leave colors for the end caps.
        plscmap1_range( 0.01, 0.99 );

        // We can only test image and gradient colorbars with two element arrays
        for ( i = 2; i < COLORBAR_KINDS; i++ )
        {
            plcolorbar_example( "cmap1_blue_yellow.pal", i, 0, 0, 2, values_small );
        }
        // Test shade colorbars with larger arrays
        for ( i = 0; i < 2; i++ )
        {
            plcolorbar_example( "cmap1_blue_yellow.pal", i, 4, 2, 9, values_even );
        }
        for ( i = 0; i < 2; i++ )
        {
            plcolorbar_example( "cmap1_blue_yellow.pal", i, 0, 0, 9, values_uneven );
        }
    }
    plend();
    return 0;
}

