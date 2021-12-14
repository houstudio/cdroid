// Demonstrate most pllegend capability including unicode symbols.
//
// Copyright (C) 2010-2014 Alan W. Irwin
// Copyright (C) 2011 Andrew Ross
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

package plplot.examples;

import plplot.core.*;
import static plplot.core.plplotjavacConstants.*;

import java.lang.Math;
import java.text.*;

class x33 {
    PLStream pls = new PLStream();

    public static void main( String[] args )
    {
        new x33( args );
    }


    static String[] special_symbols = {
        "✰",
        "✴",
        "✱",
        "✽",
        "✦"
    };

    // plcolorbar options

    static           String[] colorbar_option_kind_labels = {
        "Shade colorbars",
        "Shade colorbars with custom labels",
        "Image colorbars",
        "Gradient colorbars"
    };

    static           String[] colorbar_position_option_labels = {
        "Left",
        "Right",
        "Top",
        "Bottom"
    };

    static           String[] colorbar_label_option_labels = {
        "Label left",
        "Label right",
        "Label top",
        "Label bottom"
    };

    static           String[] colorbar_cap_option_labels = {
        "No caps",
        "Low cap",
        "High cap",
        "Low and high caps"
    };

    final static int COLORBAR_KINDS     = 4;
    final static int COLORBAR_POSITIONS = 4;
    final static int COLORBAR_LABELS    = 4;
    final static int COLORBAR_CAPS      = 4;


    void plcolorbar_example_page( int kind_i, int label_i, int cap_i, int cont_color, double cont_width, double[] values )
    {
        int    i;
        // Parameters for the colorbars on this page
        int    position_i, position, opt;
        double x, y, x_length, y_length;
        double[] ticks  = { 0.0 };
        int[] sub_ticks = { 0 };
        double  low_cap_color, high_cap_color;
        boolean vertical, ifn;
        String[] axis_opts = new String[1];
        int[] label_opts   = { 0 };
        String[] label     = new String[1];
        String title;
        double[] colorbar_width  = new double[1];
        double[] colorbar_height = new double[1];
        int[] n_values_array     = new int[1];
        double[][] values_array  = new double[1][values.length];

        // Colorbar type options
        int[] colorbar_option_kinds = {
            PL_COLORBAR_SHADE,
            PL_COLORBAR_SHADE | PL_COLORBAR_SHADE_LABEL,
            PL_COLORBAR_IMAGE,
            PL_COLORBAR_GRADIENT
        };
        // Which side of the page are we positioned relative to?
        int[] colorbar_position_options = {
            PL_POSITION_LEFT,
            PL_POSITION_RIGHT,
            PL_POSITION_TOP,
            PL_POSITION_BOTTOM
        };
        // Colorbar label positioning options
        int[] colorbar_label_options = {
            PL_COLORBAR_LABEL_LEFT,
            PL_COLORBAR_LABEL_RIGHT,
            PL_COLORBAR_LABEL_TOP,
            PL_COLORBAR_LABEL_BOTTOM
        };
        // Colorbar cap options
        int[] colorbar_cap_options = {
            PL_COLORBAR_CAP_NONE,
            PL_COLORBAR_CAP_LOW,
            PL_COLORBAR_CAP_HIGH,
            PL_COLORBAR_CAP_LOW | PL_COLORBAR_CAP_HIGH
        };

        n_values_array[0] = values.length;
        for ( i = 0; i < n_values_array[0]; i++ )
            values_array[0][i] = values[i];

        low_cap_color  = 0.0;
        high_cap_color = 1.0;

        // Start a new page
        pls.adv( 0 );

        // Draw one colorbar relative to each side of the page
        for ( position_i = 0; position_i < COLORBAR_POSITIONS; position_i++ )
        {
            position = colorbar_position_options[position_i];
            opt      =
                colorbar_option_kinds[kind_i] |
                colorbar_label_options[label_i] |
                colorbar_cap_options[cap_i];

            vertical = ( ( position & PL_POSITION_LEFT ) != 0 ) || ( ( position & PL_POSITION_RIGHT ) != 0 );
            ifn      = ( ( position & PL_POSITION_LEFT ) != 0 ) || ( ( position & PL_POSITION_BOTTOM ) != 0 );

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
                }
                else
                {
                    axis_opts[0] = "uwxvn";
                }
            }
            else
            {
                if ( cont_color == 0 || cont_width == 0. )
                {
                    axis_opts[0] = "uwtivm";
                }
                else
                {
                    axis_opts[0] = "uwxvm";
                }
            }

            label[0] = colorbar_position_option_labels[position_i] + ", " +
                       colorbar_label_option_labels[label_i];

            // Smaller text
            pls.schr( 0.0, 0.75 );
            // Small ticks on the vertical axis
            pls.smaj( 0.0, 0.5 );
            pls.smin( 0.0, 0.5 );

            pls.vpor( 0.20, 0.80, 0.20, 0.80 );
            pls.wind( 0.0, 1.0, 0.0, 1.0 );
            // Set interesting background colour.
            pls.scol0a( 15, 0, 0, 0, 0.20 );
            pls.colorbar( colorbar_width, colorbar_height,
                opt | PL_COLORBAR_BOUNDING_BOX | PL_COLORBAR_BACKGROUND, position,
                x, y, x_length, y_length,
                15, 1, 1,
                low_cap_color, high_cap_color,
                cont_color, cont_width,
                label_opts, label, axis_opts,
                ticks, sub_ticks,
                n_values_array, values_array );

            // Reset text and tick sizes
            pls.schr( 0.0, 1.0 );
            pls.smaj( 0.0, 1.0 );
            pls.smin( 0.0, 1.0 );
        }

        // Draw a page title
        title = colorbar_option_kind_labels[kind_i] + " - " +
                colorbar_cap_option_labels[cap_i];
        pls.vpor( 0.0, 1.0, 0.0, 1.0 );
        pls.wind( 0.0, 1.0, 0.0, 1.0 );
        pls.ptex( 0.5, 0.5, 0.0, 0.0, 0.5, title );
    }

    void plcolorbar_example( String palette, int kind_i, int cont_color, double cont_width, double[] values )
    {
        int label_i, cap_i;

        // Load the color palette
        pls.spal1( palette, true );

        for ( label_i = 0; label_i < COLORBAR_LABELS; label_i++ )
        {
            for ( cap_i = 0; cap_i < COLORBAR_CAPS; cap_i++ )
            {
                plcolorbar_example_page( kind_i, label_i, cap_i,
                    cont_color, cont_width,
                    values );
            }
        }
    }


//--------------------------------------------------------------------------
// x33
//
// Demonstrate most pllegend capability including unicode symbols.
//--------------------------------------------------------------------------

    public x33( String[] args )
    {
        int i, k;
        int opt;
        int nlegend, nturn;
        int[]      opt_array;
        int[]      text_colors;
        int[]      box_colors;
        int[]      box_patterns;
        double[]      box_scales;
        double[]      box_line_widths;
        int[]      line_colors;
        int[]      line_styles;
        double[]      line_widths;
        int[]      symbol_numbers;
        int[]      symbol_colors;
        double[]      symbol_scales;
        String[] text;
        String[] symbols;
        double[]      legend_width = new double[1], legend_height = new double[1];
        double x, y, xstart, ystart;
        double max_height, text_scale;
        int    position, opt_base, nrow, ncolumn;
        int[] position_options = {
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
        };

        NumberFormat nf = NumberFormat.getNumberInstance();
        nf.setMinimumIntegerDigits( 2 );

        // Parse and process command line arguments
        pls.parseopts( args, PL_PARSE_FULL | PL_PARSE_NOPROGRAM );

        // Initialize plplot
        pls.init();

        // First page illustrating the 16 standard positions.
        pls.adv( 0 );
        pls.vpor( 0.25, 0.75, 0.25, 0.75 );
        pls.wind( 0.0, 1.0, 0.0, 1.0 );
        pls.box( "bc", 0.0, 0, "bc", 0.0, 0 );
        pls.sfont( PL_FCI_SANS, -1, -1 );
        pls.mtex( "t", 8.0, 0.5, 0.5, "The 16 standard legend positions with" );
        pls.mtex( "t", 6.0, 0.5, 0.5, "the same (0.05) offset in x and y" );

        nlegend        = 1;
        opt_array      = new int[nlegend];
        text_colors    = new int[nlegend];
        line_colors    = new int[nlegend];
        line_styles    = new int[nlegend];
        line_widths    = new double[nlegend];
        symbol_numbers = new int[nlegend];
        symbol_colors  = new int[nlegend];
        symbol_scales  = new double[nlegend];
        text           = new String[nlegend];
        symbols        = new String[nlegend];
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
        pls.sfont( PL_FCI_MONO, -1, -1 );
        pls.scol0a( 15, 32, 32, 32, 0.70 );

        for ( k = 0; k < 16; k++ )
        {
            position         = position_options[k];
            opt              = opt_base;
            text[0]          = nf.format( k );
            text_colors[0]   = 1 + ( k % 8 );
            line_colors[0]   = 1 + ( k % 8 );
            symbol_colors[0] = 1 + ( k % 8 );

            pls.legend( legend_width, legend_height, opt, position, 0.05, 0.05,
                0.1, 15, 1, 1, 0, 0,
                opt_array, 1.0, 1.0, 2.0,
                1., text_colors, text,
                null, null, null, null,
                line_colors, line_styles, line_widths,
                symbol_colors, symbol_scales, symbol_numbers, symbols );
        }

        // Second page illustrating effect of nrow, ncolumn for the same legend
        // data.;
        pls.adv( 0 );
        pls.vpor( 0.25, 0.75, 0.25, 0.75 );
        pls.wind( 0.0, 1.0, 0.0, 1.0 );
        pls.box( "bc", 0.0, 0, "bc", 0.0, 0 );
        pls.sfont( PL_FCI_SANS, -1, -1 );
        pls.mtex( "t", 8.0, 0.5, 0.5, "The effect of nrow, ncolumn, PL_LEGEND_ROW_MAJOR," );
        pls.mtex( "t", 6.0, 0.5, 0.5, "and position for the same legend data" );

        nlegend         = 7;
        opt_array       = new int[nlegend];
        text_colors     = new int[nlegend];
        line_colors     = new int[nlegend];
        line_styles     = new int[nlegend];
        line_widths     = new double[nlegend];
        box_colors      = new int[nlegend];
        box_patterns    = new int[nlegend];
        box_scales      = new double[nlegend];
        box_line_widths = new double[nlegend];
        symbol_numbers  = new int[nlegend];
        symbol_colors   = new int[nlegend];
        symbol_scales   = new double[nlegend];
        text            = new String[nlegend];
        symbols         = new String[nlegend];

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
            text[k]           = nf.format( k );
            text_colors[k]    = 1 + ( k % 8 );
            line_colors[k]    = 1 + ( k % 8 );
            symbol_colors[k]  = 1 + ( k % 8 );
        }

        // Use monotype fonts so that all legends are the same size.
        pls.sfont( PL_FCI_MONO, -1, -1 );
        pls.scol0a( 15, 32, 32, 32, 0.70 );

        position = PL_POSITION_TOP | PL_POSITION_OUTSIDE;
        opt      = opt_base;
        x        = 0.;
        y        = 0.1;
        nrow     = 1;
        ncolumn  = nlegend;
        pls.legend( legend_width, legend_height, opt, position, x, y,
            0.05, 15, 1, 1, nrow, ncolumn,
            opt_array, 1.0, 1.0, 2.0,
            1., text_colors, text,
            null, null, null, null,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols );

        position = PL_POSITION_BOTTOM | PL_POSITION_OUTSIDE;
        opt      = opt_base;
        x        = 0.;
        y        = 0.1;
        nrow     = 1;
        ncolumn  = nlegend;
        pls.legend( legend_width, legend_height, opt, position, x, y,
            0.05, 15, 1, 1, nrow, ncolumn,
            opt_array, 1.0, 1.0, 2.0,
            1., text_colors, text,
            null, null, null, null,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols );

        position = PL_POSITION_LEFT | PL_POSITION_OUTSIDE;
        opt      = opt_base;
        x        = 0.1;
        y        = 0.;
        nrow     = nlegend;
        ncolumn  = 1;
        pls.legend( legend_width, legend_height, opt, position, x, y,
            0.05, 15, 1, 1, nrow, ncolumn,
            opt_array, 1.0, 1.0, 2.0,
            1., text_colors, text,
            null, null, null, null,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols );

        position = PL_POSITION_RIGHT | PL_POSITION_OUTSIDE;
        opt      = opt_base;
        x        = 0.1;
        y        = 0.;
        nrow     = nlegend;
        ncolumn  = 1;
        pls.legend( legend_width, legend_height, opt, position, x, y,
            0.05, 15, 1, 1, nrow, ncolumn,
            opt_array, 1.0, 1.0, 2.0,
            1., text_colors, text,
            null, null, null, null,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols );

        position = PL_POSITION_LEFT | PL_POSITION_TOP | PL_POSITION_INSIDE;
        opt      = opt_base;
        x        = 0.;
        y        = 0.;
        nrow     = 6;
        ncolumn  = 2;
        pls.legend( legend_width, legend_height, opt, position, x, y,
            0.05, 15, 1, 1, nrow, ncolumn,
            opt_array, 1.0, 1.0, 2.0,
            1., text_colors, text,
            null, null, null, null,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols );

        position = PL_POSITION_RIGHT | PL_POSITION_TOP | PL_POSITION_INSIDE;
        opt      = opt_base | PL_LEGEND_ROW_MAJOR;
        x        = 0.;
        y        = 0.;
        nrow     = 6;
        ncolumn  = 2;
        pls.legend( legend_width, legend_height, opt, position, x, y,
            0.05, 15, 1, 1, nrow, ncolumn,
            opt_array, 1.0, 1.0, 2.0,
            1., text_colors, text,
            null, null, null, null,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols );

        position = PL_POSITION_BOTTOM | PL_POSITION_INSIDE;
        opt      = opt_base | PL_LEGEND_ROW_MAJOR;
        x        = 0.;
        y        = 0.;
        nrow     = 3;
        ncolumn  = 3;
        pls.legend( legend_width, legend_height, opt, position, x, y,
            0.05, 15, 1, 1, nrow, ncolumn,
            opt_array, 1.0, 1.0, 2.0,
            1., text_colors, text,
            null, null, null, null,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols );

        // Third page demonstrating legend alignment
        pls.adv( 0 );
        pls.vpor( 0.0, 1.0, 0.0, 0.9 );
        pls.wind( 0.0, 1.0, 0.0, 1.0 );
        pls.sfont( PL_FCI_SANS, -1, -1 );
        pls.mtex( "t", 2.0, 0.5, 0.5, "Demonstrate legend alignment" );

        x        = 0.1;
        y        = 0.1;
        nturn    = 4;
        nlegend  = 0;
        opt_base = PL_LEGEND_BACKGROUND | PL_LEGEND_BOUNDING_BOX;
        position = PL_POSITION_TOP | PL_POSITION_LEFT | PL_POSITION_SUBPAGE;
        opt      = opt_base;
        for ( i = 0; i < 9; i++ )
        {
            // Set up legend arrays with the correct size, type.
            if ( i <= nturn )
                nlegend = nlegend + 1;
            else
                nlegend = nlegend - 1;
            nlegend         = Math.max( 1, nlegend );
            opt_array       = new int[nlegend];
            text_colors     = new int[nlegend];
            line_colors     = new int[nlegend];
            line_styles     = new int[nlegend];
            line_widths     = new double[nlegend];
            box_colors      = new int[nlegend];
            box_patterns    = new int[nlegend];
            box_scales      = new double[nlegend];
            box_line_widths = new double[nlegend];
            symbol_numbers  = new int[nlegend];
            symbol_colors   = new int[nlegend];
            symbol_scales   = new double[nlegend];
            text            = new String[nlegend];
            symbols         = new String[nlegend];

            // Only specify legend data that are required according to the
            //  value of opt_array for that entry.
            for ( k = 0; k < nlegend; k++ )
            {
                opt_array[k]      = PL_LEGEND_LINE | PL_LEGEND_SYMBOL;
                line_styles[k]    = 1;
                line_widths[k]    = 1.;
                symbol_scales[k]  = 1.;
                symbol_numbers[k] = 2;
                symbols[k]        = "#(728)";
                text[k]           = nf.format( k );
                text_colors[k]    = 1 + ( k % 8 );
                line_colors[k]    = 1 + ( k % 8 );
                symbol_colors[k]  = 1 + ( k % 8 );
            }
            // Use monotype fonts so that all legends are the same size.
            pls.sfont( PL_FCI_MONO, -1, -1 );
            pls.scol0a( 15, 32, 32, 32, 0.70 );

            nrow    = Math.min( 3, nlegend );
            ncolumn = 0;

            pls.legend( legend_width, legend_height, opt, position, x, y,
                0.025, 15, 1, 1, nrow, ncolumn,
                opt_array, 1.0, 1.0, 1.5,
                1., text_colors, text,
                null, null, null, null,
                line_colors, line_styles, line_widths,
                symbol_colors, symbol_scales, symbol_numbers, symbols );

            if ( i == nturn )
            {
                position = PL_POSITION_TOP | PL_POSITION_RIGHT | PL_POSITION_SUBPAGE;
                opt      = opt_base;
                x        = 1. - x;
                y        = y + legend_height[0];
            }
            else
            {
                x = x + legend_width[0];
                y = y + legend_height[0];
            }
        }

        // Fourth page illustrating various kinds of legends
        max_height = 0.;
        xstart     = 0.0;
        ystart     = 0.1;
        x          = xstart;
        y          = ystart;
        text_scale = 0.90;
        pls.adv( 0 );
        pls.vpor( 0.0, 1., 0.0, 0.90 );
        pls.wind( 0.0, 1.0, 0.0, 1.0 );
        //plbox("bc", 0.0, 0, "bc", 0.0, 0);
        pls.sfont( PL_FCI_SANS, -1, -1 );
        pls.mtex( "t", 2.0, 0.5, 0.5, "Demonstrate Various Kinds of Legends" );

        nlegend         = 5;
        opt_array       = new int[nlegend];
        text_colors     = new int[nlegend];
        line_colors     = new int[nlegend];
        line_styles     = new int[nlegend];
        line_widths     = new double[nlegend];
        box_colors      = new int[nlegend];
        box_patterns    = new int[nlegend];
        box_scales      = new double[nlegend];
        box_line_widths = new double[nlegend];
        symbol_numbers  = new int[nlegend];
        symbol_colors   = new int[nlegend];
        symbol_scales   = new double[nlegend];
        text            = new String[nlegend];
        symbols         = new String[nlegend];
        // In Java symbols array needs setting to null strings first, since
        // even unused values are copied over in the bindings.
        for ( k = 0; k < nlegend; k++ )
            symbols[k] = "";

        // Only specify legend data that are required according to the
        // value of opt_array for that entry.
        position = PL_POSITION_LEFT | PL_POSITION_TOP;
        opt_base = PL_LEGEND_BACKGROUND | PL_LEGEND_BOUNDING_BOX | PL_LEGEND_TEXT_LEFT;

        // Set up None, Box, Line, Symbol, and Line & Symbol legend entries.
        opt_array[0]   = PL_LEGEND_NONE;
        text[0]        = "None";
        text_colors[0] = 1;

        opt_array[1]       = PL_LEGEND_COLOR_BOX;
        text[1]            = "Box";
        text_colors[1]     = 2;
        box_colors[1]      = 2;
        box_patterns[1]    = 0;
        box_scales[1]      = 0.8;
        box_line_widths[1] = 1.;

        opt_array[2]   = PL_LEGEND_LINE;
        text[2]        = "Line";
        text_colors[2] = 3;
        line_colors[2] = 3;
        line_styles[2] = 1;
        line_widths[2] = 1.;

        opt_array[3]      = PL_LEGEND_SYMBOL;
        text[3]           = "Symbol";
        text_colors[3]    = 4;
        symbol_colors[3]  = 4;
        symbol_scales[3]  = text_scale;
        symbol_numbers[3] = 4;
        symbols[3]        = special_symbols[2];

        opt_array[4]      = PL_LEGEND_SYMBOL | PL_LEGEND_LINE;
        text[4]           = "L & S";
        text_colors[4]    = 5;
        line_colors[4]    = 5;
        line_styles[4]    = 1;
        line_widths[4]    = 1.;
        symbol_colors[4]  = 5;
        symbol_scales[4]  = text_scale;
        symbol_numbers[4] = 4;
        symbols[4]        = special_symbols[2];

        opt = opt_base;
        pls.scol0a( 15, 32, 32, 32, 0.70 );

        pls.legend( legend_width, legend_height, opt, position, x, y,
            0.1, 15, 1, 1, 0, 0,
            opt_array, 1.0, text_scale, 2.0,
            0., text_colors, text,
            box_colors, box_patterns, box_scales, box_line_widths,
            line_colors, line_styles, line_widths,
            symbol_colors, symbol_scales, symbol_numbers, symbols );
        max_height = Math.max( max_height, legend_height[0] );

        // Set up symbol legend entries with various symbols.
        for ( i = 0; i < nlegend; i++ )
        {
            opt_array[i]      = PL_LEGEND_SYMBOL;
            text[i]           = "Symbol " + special_symbols[i];
            text_colors[i]    = i + 1;
            symbol_colors[i]  = i + 1;
            symbol_scales[i]  = text_scale;
            symbol_numbers[i] = 4;
            symbols[i]        = special_symbols[i];
        }

        opt = opt_base;
        x   = x + legend_width[0];
        pls.scol0a( 15, 32, 32, 32, 0.70 );

        pls.legend( legend_width, legend_height, opt, position, x, y,
            0.1, 15, 1, 1, 0, 0,
            opt_array, 1.0, text_scale, 2.0,
            0., text_colors, text,
            null, null, null, null,
            null, null, null,
            symbol_colors, symbol_scales, symbol_numbers, symbols );
        max_height = Math.max( max_height, legend_height[0] );

        // Set up symbol legend entries with various numbers of symbols.
        for ( i = 0; i < nlegend; i++ )
        {
            opt_array[i]      = PL_LEGEND_SYMBOL;
            text[i]           = "Symbol Number " + ( i + 2 );
            text_colors[i]    = i + 1;
            symbol_colors[i]  = i + 1;
            symbol_scales[i]  = text_scale;
            symbol_numbers[i] = i + 2;
            symbols[i]        = special_symbols[2];
        }

        opt = opt_base;
        x   = x + legend_width[0];
        pls.scol0a( 15, 32, 32, 32, 0.70 );

        pls.legend( legend_width, legend_height, opt, position, x, y,
            0.1, 15, 1, 1, 0, 0,
            opt_array, 1.0, text_scale, 2.0,
            0., text_colors, text,
            null, null, null, null,
            null, null, null,
            symbol_colors, symbol_scales, symbol_numbers, symbols );
        max_height = Math.max( max_height, legend_height[0] );

        // Set up box legend entries with various colours.
        for ( i = 0; i < nlegend; i++ )
        {
            opt_array[i]       = PL_LEGEND_COLOR_BOX;
            text[i]            = "Box Color " + ( i + 1 );
            text_colors[i]     = i + 1;
            box_colors[i]      = i + 1;
            box_patterns[i]    = 0;
            box_scales[i]      = 0.8;
            box_line_widths[i] = 1.;
        }

        opt = opt_base;
        // Use new origin
        x          = xstart;
        y          = y + max_height;
        max_height = 0.;
        pls.scol0a( 15, 32, 32, 32, 0.70 );

        pls.legend( legend_width, legend_height, opt, position, x, y,
            0.1, 15, 1, 1, 0, 0,
            opt_array, 1.0, text_scale, 2.0,
            0., text_colors, text,
            box_colors, box_patterns, box_scales, box_line_widths,
            null, null, null,
            null, null, null, null );
        max_height = Math.max( max_height, legend_height[0] );

        // Set up box legend entries with various patterns.
        for ( i = 0; i < nlegend; i++ )
        {
            opt_array[i]       = PL_LEGEND_COLOR_BOX;
            text[i]            = "Box Pattern " + i;
            text_colors[i]     = 2;
            box_colors[i]      = 2;
            box_patterns[i]    = i;
            box_scales[i]      = 0.8;
            box_line_widths[i] = 1.;
        }

        opt = opt_base;
        x   = x + legend_width[0];
        pls.scol0a( 15, 32, 32, 32, 0.70 );

        pls.legend( legend_width, legend_height, opt, position, x, y,
            0.1, 15, 1, 1, 0, 0,
            opt_array, 1.0, text_scale, 2.0,
            0., text_colors, text,
            box_colors, box_patterns, box_scales, box_line_widths,
            null, null, null,
            null, null, null, null );
        max_height = Math.max( max_height, legend_height[0] );

        // Set up box legend entries with various box pattern line widths.
        for ( i = 0; i < nlegend; i++ )
        {
            opt_array[i]       = PL_LEGEND_COLOR_BOX;
            text[i]            = "Box Line Width " + ( i + 1 );
            text_colors[i]     = 2;
            box_colors[i]      = 2;
            box_patterns[i]    = 3;
            box_scales[i]      = 0.8;
            box_line_widths[i] = i + 1;
        }

        opt = opt_base;
        x   = x + legend_width[0];
        pls.scol0a( 15, 32, 32, 32, 0.70 );

        pls.legend( legend_width, legend_height, opt, position, x, y,
            0.1, 15, 1, 1, 0, 0,
            opt_array, 1.0, text_scale, 2.0,
            0., text_colors, text,
            box_colors, box_patterns, box_scales, box_line_widths,
            null, null, null,
            null, null, null, null );
        max_height = Math.max( max_height, legend_height[0] );

        // Set up line legend entries with various colours.
        for ( i = 0; i < nlegend; i++ )
        {
            opt_array[i]   = PL_LEGEND_LINE;
            text[i]        = "Line Color " + ( i + 1 );
            text_colors[i] = i + 1;
            line_colors[i] = i + 1;
            line_styles[i] = 1;
            line_widths[i] = 1.;
        }

        opt = opt_base;
        // Use new origin
        x          = xstart;
        y          = y + max_height;
        max_height = 0.;
        pls.scol0a( 15, 32, 32, 32, 0.70 );

        pls.legend( legend_width, legend_height, opt, position, x, y,
            0.1, 15, 1, 1, 0, 0,
            opt_array, 1.0, text_scale, 2.0,
            0., text_colors, text,
            null, null, null, null,
            line_colors, line_styles, line_widths,
            null, null, null, null );
        max_height = Math.max( max_height, legend_height[0] );

        // Set up line legend entries with various styles.
        for ( i = 0; i < nlegend; i++ )
        {
            opt_array[i]   = PL_LEGEND_LINE;
            text[i]        = "Line Style " + ( i + 1 );
            text_colors[i] = 2;
            line_colors[i] = 2;
            line_styles[i] = i + 1;
            line_widths[i] = 1.;
        }

        opt = opt_base;
        x   = x + legend_width[0];
        pls.scol0a( 15, 32, 32, 32, 0.70 );

        pls.legend( legend_width, legend_height, opt, position, x, y,
            0.1, 15, 1, 1, 0, 0,
            opt_array, 1.0, text_scale, 2.0,
            0., text_colors, text,
            null, null, null, null,
            line_colors, line_styles, line_widths,
            null, null, null, null );
        max_height = Math.max( max_height, legend_height[0] );

        // Set up line legend entries with various widths.
        for ( i = 0; i < nlegend; i++ )
        {
            opt_array[i]   = PL_LEGEND_LINE;
            text[i]        = "Line Width " + ( i + 1 );
            text_colors[i] = 2;
            line_colors[i] = 2;
            line_styles[i] = 1;
            line_widths[i] = i + 1;
        }

        opt = opt_base;
        x   = x + legend_width[0];
        pls.scol0a( 15, 32, 32, 32, 0.70 );

        pls.legend( legend_width, legend_height, opt, position, x, y,
            0.1, 15, 1, 1, 0, 0,
            opt_array, 1.0, text_scale, 2.0,
            0., text_colors, text,
            null, null, null, null,
            line_colors, line_styles, line_widths,
            null, null, null, null );
        max_height = Math.max( max_height, legend_height[0] );

        // Color bar examples
        double[] values_small  = { -1.0e-20, 1.0e-20 };
        double[] values_uneven = { -1.0e-20, 2.0e-20, 2.6e-20, 3.4e-20, 6.0e-20, 7.0e-20, 8.0e-20, 9.0e-20, 10.0e-20 };
        double[] values_even   = { -2.0e-20, -1.0e-20, 0.0e-20, 1.0e-20, 2.0e-20, 3.0e-20, 4.0e-20, 5.0e-20, 6.0e-20 };

        // Use unsaturated green background colour to contrast with black caps.
        pls.scolbg( 70, 185, 70 );
        // Cut out the greatest and smallest bits of the color spectrum to
        // leave colors for the end caps.
        pls.scmap1_range( 0.01, 0.99 );

        // We can only test image and gradient colorbars with two element arrays
        for ( i = 2; i < COLORBAR_KINDS; i++ )
        {
            plcolorbar_example( "cmap1_blue_yellow.pal", i, 0, 0, values_small );
        }
        // Test shade colorbars with larger arrays
        for ( i = 0; i < 2; i++ )
        {
            plcolorbar_example( "cmap1_blue_yellow.pal", i, 4, 2, values_even );
        }
        for ( i = 0; i < 2; i++ )
        {
            plcolorbar_example( "cmap1_blue_yellow.pal", i, 0, 0, values_uneven );
        }

        pls.end();
    }
}
