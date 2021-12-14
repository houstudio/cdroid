# -*- coding: utf-8; -*-

#  Copyright (C) 2010-2017 Alan W. Irwin

#  Demonstrate most pllegend capability including unicode symbols.

#  This file is part of PLplot.
#
#  PLplot is free software; you can redistribute it and/or modify
#  it under the terms of the GNU Library General Public License as published
#  by the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  PLplot is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Library General Public License for more details.
#
#  You should have received a copy of the GNU Library General Public License
#  along with PLplot; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
#  This example designed just for devices (e.g., the cairo-related and
#  qt-related devices) where the best choice of glyph is automatically
#  selected by the related libraries (pango/cairo or Qt4) for each
#  unicode character depending on what system fonts are installed.  Of
#  course, you must have the appropriate TrueType fonts installed to
#  have access to all the required glyphs.

import sys

from numpy import *

# This only needed to define the following lists
import plplot as w_alt

position_options = [
w_alt.PL_POSITION_LEFT | w_alt.PL_POSITION_TOP | w_alt.PL_POSITION_OUTSIDE,
w_alt.PL_POSITION_TOP | w_alt.PL_POSITION_OUTSIDE,
w_alt.PL_POSITION_RIGHT | w_alt.PL_POSITION_TOP | w_alt.PL_POSITION_OUTSIDE,
w_alt.PL_POSITION_RIGHT| w_alt.PL_POSITION_OUTSIDE,
w_alt.PL_POSITION_RIGHT | w_alt.PL_POSITION_BOTTOM | w_alt.PL_POSITION_OUTSIDE,
w_alt.PL_POSITION_BOTTOM | w_alt.PL_POSITION_OUTSIDE,
w_alt.PL_POSITION_LEFT | w_alt.PL_POSITION_BOTTOM | w_alt.PL_POSITION_OUTSIDE,
w_alt.PL_POSITION_LEFT | w_alt.PL_POSITION_OUTSIDE,
w_alt.PL_POSITION_LEFT | w_alt.PL_POSITION_TOP | w_alt.PL_POSITION_INSIDE,
w_alt.PL_POSITION_TOP | w_alt.PL_POSITION_INSIDE,
w_alt.PL_POSITION_RIGHT | w_alt.PL_POSITION_TOP | w_alt.PL_POSITION_INSIDE,
w_alt.PL_POSITION_RIGHT| w_alt.PL_POSITION_INSIDE,
w_alt.PL_POSITION_RIGHT | w_alt.PL_POSITION_BOTTOM | w_alt.PL_POSITION_INSIDE,
w_alt.PL_POSITION_BOTTOM | w_alt.PL_POSITION_INSIDE,
w_alt.PL_POSITION_LEFT | w_alt.PL_POSITION_BOTTOM | w_alt.PL_POSITION_INSIDE,
w_alt.PL_POSITION_LEFT | w_alt.PL_POSITION_INSIDE,
]

# Pick 5 arbitrary UTF-8 symbols useful for plotting points (✠✚✱✪✽✺✰✴✦).
special_symbols = [
"✰",
"✴",
"✱",
"✽",
"✦",
]

# Colorbar type options
COLORBAR_KINDS =  4
colorbar_option_kinds = [
    w_alt.PL_COLORBAR_SHADE,
    w_alt.PL_COLORBAR_SHADE | w_alt.PL_COLORBAR_SHADE_LABEL,
    w_alt.PL_COLORBAR_IMAGE,
    w_alt.PL_COLORBAR_GRADIENT
]
colorbar_option_kind_labels = [
    "Shade colorbars",
    "Shade colorbars with custom labels",
    "Image colorbars",
    "Gradient colorbars"
]

# Which side of the page are we positioned relative to?
COLORBAR_POSITIONS = 4
colorbar_position_options = [
    w_alt.PL_POSITION_LEFT,
    w_alt.PL_POSITION_RIGHT,
    w_alt.PL_POSITION_TOP,
    w_alt.PL_POSITION_BOTTOM
]
colorbar_position_option_labels = [
    "Left",
    "Right",
    "Top",
    "Bottom"
]

# Colorbar label positioning options
COLORBAR_LABELS = 4
colorbar_label_options = [
    w_alt.PL_COLORBAR_LABEL_LEFT,
    w_alt.PL_COLORBAR_LABEL_RIGHT,
    w_alt.PL_COLORBAR_LABEL_TOP,
    w_alt.PL_COLORBAR_LABEL_BOTTOM
]
colorbar_label_option_labels = [
    "Label left",
    "Label right",
    "Label top",
    "Label bottom"
]

# Colorbar cap options
COLORBAR_CAPS = 4
colorbar_cap_options = [
    w_alt.PL_COLORBAR_CAP_NONE,
    w_alt.PL_COLORBAR_CAP_LOW,
    w_alt.PL_COLORBAR_CAP_HIGH,
    w_alt.PL_COLORBAR_CAP_LOW | w_alt.PL_COLORBAR_CAP_HIGH
]
colorbar_cap_option_labels = [
    "No caps",
    "Low cap",
    "High cap",
    "Low and high caps"
]

def plcolorbar_example_page( w, kind_i, label_i, cap_i, cont_color, cont_width, values ):
    n_axes = 1
    axis_opts = zeros(n_axes,"S100")
    ticks = zeros(n_axes)
    sub_ticks = zeros(n_axes,"int")
    label_opts = zeros(n_axes,"int")
    label_opts[0] = w.PL_COLORBAR_LABEL_BOTTOM
    label = zeros(n_axes,"S100")
    n_values_array = zeros(n_axes,"int")
    n_values_array[0] = len(values)
    values_array = reshape(values,[1,len(values)])

    low_cap_color = 0.0
    high_cap_color = 1.0

    # Start a new page
    w.pladv( 0 )

    # Draw one colorbar relative to each side of the page
    for position_i in range(COLORBAR_POSITIONS):
        position = colorbar_position_options[position_i]
        opt = colorbar_option_kinds[kind_i] | colorbar_label_options[label_i] | colorbar_cap_options[cap_i]

        vertical = (position & w.PL_POSITION_LEFT) or (position & w.PL_POSITION_RIGHT)
        ifn      = position & w.PL_POSITION_LEFT or position & w.PL_POSITION_BOTTOM

        # Set the offset position on the page
        if vertical:
            x        = 0.0
            y        = 0.0
            x_length = 0.05
            y_length = 0.5
        else:
            x        = 0.0
            y        = 0.0
            x_length = 0.5
            y_length = 0.05

        # Set appropriate labelling options.
        if ifn:
            if ( cont_color == 0 or cont_width == 0. ):
                axis_opts[0] = "uwtivn"
            else:
                axis_opts[0] = "uwxvn";
        else:
            if ( cont_color == 0 or cont_width == 0. ):
                axis_opts[0] = "uwtivm"
            else:
                axis_opts[0] = "uwxvm"

        label[0] =  "%s, %s" % (colorbar_position_option_labels[position_i] , colorbar_label_option_labels[label_i])

        # Smaller text
        w.plschr( 0.0, 0.75 )
        # Small ticks on the vertical axis
        w.plsmaj( 0.0, 0.5 )
        w.plsmin( 0.0, 0.5 )

        w.plvpor( 0.20, 0.80, 0.20, 0.80 )
        w.plwind( 0.0, 1.0, 0.0, 1.0 )
        # Set interesting background colour.
        w.plscol0a( 15, 0, 0, 0, 0.20 )
        (colorbar_width, colorbar_height) = w.plcolorbar( \
            opt | w.PL_COLORBAR_BOUNDING_BOX | w.PL_COLORBAR_BACKGROUND, position, \
            x, y, x_length, y_length, \
            15, 1, 1, \
            low_cap_color, high_cap_color, \
            cont_color, cont_width, \
            label_opts, label, \
            axis_opts, ticks, sub_ticks, \
            n_values_array, values_array )

        # Reset text and tick sizes
        w.plschr( 0.0, 1.0 )
        w.plsmaj( 0.0, 1.0 )
        w.plsmin( 0.0, 1.0 )

    # Draw a page title
    title  = "%s - %s" % (colorbar_option_kind_labels[kind_i] , colorbar_cap_option_labels[cap_i])
    w.plvpor( 0.0, 1.0, 0.0, 1.0 )
    w.plwind( 0.0, 1.0, 0.0, 1.0 )
    w.plptex( 0.5, 0.5, 0.0, 0.0, 0.5, title )

def plcolorbar_example( w, palette, kind_i, cont_color, cont_width, values ):
    # Load the color palette
    w.plspal1( palette, 1 )

    for label_i in arange(COLORBAR_LABELS):
        for cap_i in arange(COLORBAR_CAPS):
            plcolorbar_example_page( w, kind_i, label_i, cap_i, cont_color, cont_width, values )


def main(w):
    # First page illustrating the 16 standard positions.
    w.pladv(0)
    w.plvpor(0.25, 0.75, 0.25, 0.75)
    w.plwind(0.0, 1.0, 0.0, 1.0)
    w.plbox("bc", 0.0, 0, "bc", 0.0, 0)
    w.plsfont(w.PL_FCI_SANS, -1, -1)
    w.plmtex("t", 8.0, 0.5, 0.5, "The 16 standard legend positions with")
    w.plmtex("t", 6.0, 0.5, 0.5, "the same (0.05) offset in x and y")

    # Set up legend arrays with the correct size, type.
    nlegend = 1
    opt_array = zeros(nlegend, "int")
    text_colors = zeros(nlegend, "int")
    text = zeros(nlegend, "S200")
    box_colors = zeros(nlegend, "int")
    box_patterns = zeros(nlegend, "int")
    box_scales = zeros(nlegend)
    box_line_widths = zeros(nlegend)
    line_colors = zeros(nlegend, "int")
    line_styles = zeros(nlegend, "int")
    line_widths = zeros(nlegend)
    symbol_colors = zeros(nlegend, "int")
    symbol_scales = zeros(nlegend)
    symbol_numbers = zeros(nlegend, "int")
    symbols = zeros(nlegend, "S100")

    # Only specify legend data that are required according to the
    # value of opt_array for that entry.
    opt_base = w.PL_LEGEND_BACKGROUND | w.PL_LEGEND_BOUNDING_BOX
    opt_array[0] = w.PL_LEGEND_LINE | w.PL_LEGEND_SYMBOL
    line_styles[0] = 1
    line_widths[0] = 1.
    symbol_scales[0] = 1.
    symbol_numbers[0] = 4
    symbols[0] = "#(728)"

    # Use monotype fonts so that all legends are the same size.
    w.plsfont(w.PL_FCI_MONO, -1, -1)
    w.plscol0a( 15, 32, 32, 32, 0.70 )

    for k in range(16):
        position = position_options[k]
        opt = opt_base
        text[0] = "%2.2d" % k
        text_colors[0] = 1 + (k % 8)
        line_colors[0] = 1 + (k % 8)
        symbol_colors[0] = 1 + (k % 8)

        (legend_width, legend_height) = \
        w.pllegend( opt, position, 0.05, 0.05,
                  0.1, 15, 1, 1, 0, 0, opt_array, 1.0, 1.0, 2.0,
                  1., text_colors, text,
                  box_colors, box_patterns, box_scales, box_line_widths,
                  line_colors, line_styles, line_widths,
                  symbol_colors, symbol_scales, symbol_numbers, symbols )

    # Second page illustrating effect of nrow, ncolumn for the same legend
    # data.
    w.pladv(0)
    w.plvpor(0.25, 0.75, 0.25, 0.75)
    w.plwind(0.0, 1.0, 0.0, 1.0)
    w.plbox("bc", 0.0, 0, "bc", 0.0, 0)
    w.plsfont(w.PL_FCI_SANS, -1, -1)
    w.plmtex("t", 8.0, 0.5, 0.5, "The effect of nrow, ncolumn, PL_LEGEND_ROW_MAJOR,")
    w.plmtex("t", 6.0, 0.5, 0.5, "and position for the same legend data")

    # Set up legend arrays with the correct size, type.
    nlegend = 7
    opt_array = zeros(nlegend, "int")
    text_colors = zeros(nlegend, "int")
    text = zeros(nlegend, "S200")
    box_colors = zeros(nlegend, "int")
    box_patterns = zeros(nlegend, "int")
    box_scales = zeros(nlegend)
    box_line_widths = zeros(nlegend)
    line_colors = zeros(nlegend, "int")
    line_styles = zeros(nlegend, "int")
    line_widths = zeros(nlegend)
    symbol_colors = zeros(nlegend, "int")
    symbol_scales = zeros(nlegend)
    symbol_numbers = zeros(nlegend, "int")
    symbols = zeros(nlegend, "S100")

    # Only specify legend data that are required according to the
    # value of opt_array for that entry.
    opt_base = w.PL_LEGEND_BACKGROUND | w.PL_LEGEND_BOUNDING_BOX
    for k in range(nlegend):
        opt_array[k] = w.PL_LEGEND_LINE | w.PL_LEGEND_SYMBOL
        line_styles[k] = 1
        line_widths[k] = 1.
        symbol_scales[k] = 1.
        symbol_numbers[k] = 2
        symbols[k] = "#(728)"
        text[k] = "%2.2d" % k
        text_colors[k] = 1 + (k % 8)
        line_colors[k] = 1 + (k % 8)
        symbol_colors[k] = 1 + (k % 8)

    # Use monotype fonts so that all legends are the same size.
    w.plsfont(w.PL_FCI_MONO, -1, -1)
    w.plscol0a( 15, 32, 32, 32, 0.70 )

    position = w.PL_POSITION_TOP | w.PL_POSITION_OUTSIDE
    opt = opt_base
    x = 0.
    y = 0.1
    nrow = 1
    ncolumn = nlegend
    (legend_width, legend_height) = \
     w.pllegend( opt, position, x, y,
              0.05, 15, 1, 1, nrow, ncolumn, opt_array, 1.0, 1.0, 2.0,
              1., text_colors, text,
              box_colors, box_patterns, box_scales, box_line_widths,
              line_colors, line_styles, line_widths,
              symbol_colors, symbol_scales, symbol_numbers, symbols )

    position = w.PL_POSITION_BOTTOM | w.PL_POSITION_OUTSIDE
    opt = opt_base
    x = 0.
    y = 0.1
    nrow = 1
    ncolumn = nlegend
    (legend_width, legend_height) = \
    w.pllegend( opt, position, x, y,
              0.05, 15, 1, 1, nrow, ncolumn, opt_array, 1.0, 1.0, 2.0,
              1., text_colors, text,
              box_colors, box_patterns, box_scales, box_line_widths,
              line_colors, line_styles, line_widths,
              symbol_colors, symbol_scales, symbol_numbers, symbols )

    position = w.PL_POSITION_LEFT | w.PL_POSITION_OUTSIDE
    opt = opt_base
    x = 0.1
    y = 0.
    nrow = nlegend
    ncolumn = 1
    (legend_width, legend_height) = \
    w.pllegend( opt, position, x, y,
              0.05, 15, 1, 1, nrow, ncolumn, opt_array, 1.0, 1.0, 2.0,
              1., text_colors, text,
              box_colors, box_patterns, box_scales, box_line_widths,
              line_colors, line_styles, line_widths,
              symbol_colors, symbol_scales, symbol_numbers, symbols )

    position = w.PL_POSITION_RIGHT | w.PL_POSITION_OUTSIDE
    opt = opt_base
    x = 0.1
    y = 0.
    nrow = nlegend
    ncolumn = 1
    (legend_width, legend_height) = \
    w.pllegend( opt, position, x, y,
              0.05, 15, 1, 1, nrow, ncolumn, opt_array, 1.0, 1.0, 2.0,
              1., text_colors, text,
              box_colors, box_patterns, box_scales, box_line_widths,
              line_colors, line_styles, line_widths,
              symbol_colors, symbol_scales, symbol_numbers, symbols )

    position = w.PL_POSITION_LEFT | w.PL_POSITION_TOP | w.PL_POSITION_INSIDE
    opt = opt_base
    x = 0.
    y = 0.
    nrow = 6
    ncolumn = 2
    (legend_width, legend_height) = \
    w.pllegend( opt, position, x, y,
              0.05, 15, 1, 1, nrow, ncolumn, opt_array, 1.0, 1.0, 2.0,
              1., text_colors, text,
              box_colors, box_patterns, box_scales, box_line_widths,
              line_colors, line_styles, line_widths,
              symbol_colors, symbol_scales, symbol_numbers, symbols )

    position = w.PL_POSITION_RIGHT | w.PL_POSITION_TOP | w.PL_POSITION_INSIDE
    opt = opt_base | w.PL_LEGEND_ROW_MAJOR
    x = 0.
    y = 0.
    nrow = 6
    ncolumn = 2
    (legend_width, legend_height) = \
    w.pllegend( opt, position, x, y,
              0.05, 15, 1, 1, nrow, ncolumn, opt_array, 1.0, 1.0, 2.0,
              1., text_colors, text,
              box_colors, box_patterns, box_scales, box_line_widths,
              line_colors, line_styles, line_widths,
              symbol_colors, symbol_scales, symbol_numbers, symbols )

    position = w.PL_POSITION_BOTTOM | w.PL_POSITION_INSIDE
    opt = opt_base | w.PL_LEGEND_ROW_MAJOR
    x = 0.
    y = 0.
    nrow = 3
    ncolumn = 3
    (legend_width, legend_height) = \
    w.pllegend( opt, position, x, y,
              0.05, 15, 1, 1, nrow, ncolumn, opt_array, 1.0, 1.0, 2.0,
              1., text_colors, text,
              box_colors, box_patterns, box_scales, box_line_widths,
              line_colors, line_styles, line_widths,
              symbol_colors, symbol_scales, symbol_numbers, symbols )

    # Third page demonstrating legend alignment
    w.pladv(0)
    w.plvpor(0.0, 1.0, 0.0, 0.9)
    w.plwind(0.0, 1.0, 0.0, 1.0)
    w.plsfont(w.PL_FCI_SANS, -1, -1)
    w.plmtex("t", 2.0, 0.5, 0.5, "Demonstrate legend alignment")

    x = 0.1
    y = 0.1
    nturn = 4
    nlegend = 0
    position = w.PL_POSITION_TOP |  w.PL_POSITION_LEFT | w.PL_POSITION_SUBPAGE
    opt_base = w.PL_LEGEND_BACKGROUND | w.PL_LEGEND_BOUNDING_BOX
    opt = opt_base
    for i in range(9):
        # Set up legend arrays with the correct size, type.
        if i <= nturn:
            nlegend += 1
        else:
            nlegend -= 1
        nlegend = max(1, nlegend)
        opt_array = zeros(nlegend, "int")
        text_colors = zeros(nlegend, "int")
        text = zeros(nlegend, "S200")
        box_colors = zeros(nlegend, "int")
        box_patterns = zeros(nlegend, "int")
        box_scales = zeros(nlegend)
        box_line_widths = zeros(nlegend)
        line_colors = zeros(nlegend, "int")
        line_styles = zeros(nlegend, "int")
        line_widths = zeros(nlegend)
        symbol_colors = zeros(nlegend, "int")
        symbol_scales = zeros(nlegend)
        symbol_numbers = zeros(nlegend, "int")
        symbols = zeros(nlegend, "S100")

        # Only specify legend data that are required according to the
        # value of opt_array for that entry.
        for k in range(nlegend):
            opt_array[k] = w.PL_LEGEND_LINE | w.PL_LEGEND_SYMBOL
            line_styles[k] = 1
            line_widths[k] = 1.
            symbol_scales[k] = 1.
            symbol_numbers[k] = 2
            symbols[k] = "#(728)"
            text[k] = "%2.2d" % k
            text_colors[k] = 1 + (k % 8)
            line_colors[k] = 1 + (k % 8)
            symbol_colors[k] = 1 + (k % 8)

        # Use monotype fonts so that all legends are the same size.
        w.plsfont(w.PL_FCI_MONO, -1, -1)
        w.plscol0a( 15, 32, 32, 32, 0.70 )

        nrow = min(3, nlegend)
        ncolumn = 0

        (legend_width, legend_height) = \
        w.pllegend( opt, position, x, y,
                  0.025, 15, 1, 1, nrow, ncolumn, opt_array, 1.0, 1.0, 1.5,
                  1., text_colors, text,
                  box_colors, box_patterns, box_scales, box_line_widths,
                  line_colors, line_styles, line_widths,
                  symbol_colors, symbol_scales, symbol_numbers, symbols )
        if i == nturn:
            position = w.PL_POSITION_TOP |  w.PL_POSITION_RIGHT | w.PL_POSITION_SUBPAGE
            opt = opt_base
            x = 1. - x
            y += legend_height
        else:
            x += legend_width
            y += legend_height

    # Fourth page illustrating various kinds of legends
    max_height = 0.
    xstart = 0.0
    ystart = 0.1
    x = xstart
    y = ystart
    text_scale = 0.90
    w.pladv(0)
    w.plvpor(0.0, 1., 0.0, 0.90)
    w.plwind(0.0, 1.0, 0.0, 1.0)
    # w.plbox("bc", 0.0, 0, "bc", 0.0, 0)
    w.plsfont(w.PL_FCI_SANS, -1, -1)
    w.plmtex("t", 2.0, 0.5, 0.5, "Demonstrate Various Kinds of Legends")

    # Set up legend arrays with the correct size, type.
    nlegend = 5
    opt_array = zeros(nlegend, "int")
    text_colors = zeros(nlegend, "int")
    text = zeros(nlegend, "S200")
    box_colors = zeros(nlegend, "int")
    box_patterns = zeros(nlegend, "int")
    box_scales = zeros(nlegend)
    box_line_widths = zeros(nlegend)
    line_colors = zeros(nlegend, "int")
    line_styles = zeros(nlegend, "int")
    line_widths = zeros(nlegend)
    symbol_colors = zeros(nlegend, "int")
    symbol_scales = zeros(nlegend)
    symbol_numbers = zeros(nlegend, "int")
    symbols = zeros(nlegend, "S100")

    # Only specify legend data that are required according to the
    # value of opt_array for that entry.
    position = w.PL_POSITION_LEFT | w.PL_POSITION_TOP
    opt_base = w.PL_LEGEND_BACKGROUND | w.PL_LEGEND_BOUNDING_BOX | w.PL_LEGEND_TEXT_LEFT

    # Set up None, Box, Line, Symbol, and Line & Symbol legend entries.
    opt_array[0] = w.PL_LEGEND_NONE
    text[0] = "None"
    text_colors[0] = 1

    opt_array[1] = w.PL_LEGEND_COLOR_BOX
    text[1] = "Box"
    text_colors[1] = 2
    box_colors[1] = 2
    box_patterns[1] = 0
    box_scales[1] = 0.8
    box_line_widths[1] = 1.

    opt_array[2] = w.PL_LEGEND_LINE
    text[2] = "Line"
    text_colors[2] = 3
    line_colors[2] = 3
    line_styles[2] = 1
    line_widths[2] = 1.

    opt_array[3] = w.PL_LEGEND_SYMBOL
    text[3] = "Symbol"
    text_colors[3] = 4
    symbol_colors[3] = 4
    symbol_scales[3] = text_scale
    symbol_numbers[3] = 4
    symbols[3] = special_symbols[2].encode() if (sys.version_info > (3, 0)) else special_symbols[2]

    opt_array[4] = w.PL_LEGEND_SYMBOL | w.PL_LEGEND_LINE
    text[4] = "L & S"
    text_colors[4] = 5
    line_colors[4] = 5
    line_styles[4] = 1
    line_widths[4] = 1.
    symbol_colors[4] = 5
    symbol_scales[4] = text_scale
    symbol_numbers[4] = 4
    symbols[4] = special_symbols[2].encode() if (sys.version_info > (3, 0)) else special_symbols[2]

    opt = opt_base
    w.plscol0a( 15, 32, 32, 32, 0.70 )
    (legend_width, legend_height) = \
    w.pllegend( opt, position, x, y,
              0.1, 15, 1, 1, 0, 0, opt_array, 1.0, text_scale, 2.0,
              0., text_colors, text,
              box_colors, box_patterns, box_scales, box_line_widths,
              line_colors, line_styles, line_widths,
              symbol_colors, symbol_scales, symbol_numbers, symbols )
    max_height = max(max_height, legend_height)

    # Set up symbol legend entries with various symbols.
    for i in range(nlegend):
        opt_array[i] = w.PL_LEGEND_SYMBOL
        text[i] = "Symbol ".encode() if (sys.version_info > (3, 0)) else "Symbol "
        text[i] += special_symbols[i].encode() if (sys.version_info > (3, 0)) else special_symbols[i]
        text_colors[i] = i+1
        symbol_colors[i] = i+1
        symbol_scales[i] = text_scale
        symbol_numbers[i] = 4
        symbols[i] = special_symbols[i].encode() if (sys.version_info > (3, 0)) else special_symbols[i]

    opt = opt_base
    x += legend_width
    w.plscol0a( 15, 32, 32, 32, 0.70 )
    (legend_width, legend_height) = \
    w.pllegend( opt, position, x, y,
              0.1, 15, 1, 1, 0, 0, opt_array, 1.0, text_scale, 2.0,
              0., text_colors, text,
              box_colors, box_patterns, box_scales, box_line_widths,
              line_colors, line_styles, line_widths,
              symbol_colors, symbol_scales, symbol_numbers, symbols )
    max_height = max(max_height, legend_height)

    # Set up symbol legend entries with various numbers of symbols.
    for i in range(nlegend):
        opt_array[i] = w.PL_LEGEND_SYMBOL
        text[i] = "Symbol Number %d" % (i+2)
        text_colors[i] = i+1
        symbol_colors[i] = i+1
        symbol_scales[i] = text_scale
        symbol_numbers[i] = i+2
        symbols[i] = special_symbols[2].encode() if (sys.version_info > (3, 0)) else special_symbols[2]

    opt = opt_base
    x += legend_width
    w.plscol0a( 15, 32, 32, 32, 0.70 )
    (legend_width, legend_height) = \
    w.pllegend( opt, position, x, y,
              0.1, 15, 1, 1, 0, 0, opt_array, 1.0, text_scale, 2.0,
              0., text_colors, text,
              box_colors, box_patterns, box_scales, box_line_widths,
              line_colors, line_styles, line_widths,
              symbol_colors, symbol_scales, symbol_numbers, symbols )
    max_height = max(max_height, legend_height)

    # Set up box legend entries with various colours.
    for i in range(nlegend):
        opt_array[i] = w.PL_LEGEND_COLOR_BOX
        text[i] = "%s %d" % ("Box Color",i+1)
        text_colors[i] = i+1
        box_colors[i] = i+1
        box_patterns[i] = 0
        box_scales[i] = 0.8
        box_line_widths[i] = 1.

    opt = opt_base
    # Use new origin
    x = xstart
    y += max_height
    max_height = 0.
    w.plscol0a( 15, 32, 32, 32, 0.70 )
    (legend_width, legend_height) = \
    w.pllegend( opt, position, x, y,
              0.1, 15, 1, 1, 0, 0, opt_array, 1.0, text_scale, 2.0,
              0., text_colors, text,
              box_colors, box_patterns, box_scales, box_line_widths,
              line_colors, line_styles, line_widths,
              symbol_colors, symbol_scales, symbol_numbers, symbols )
    max_height = max(max_height, legend_height)

    # Set up box legend entries with various patterns.
    for i in range(nlegend):
        opt_array[i] = w.PL_LEGEND_COLOR_BOX
        text[i] = "%s %d" % ("Box Pattern",i)
        text_colors[i] = 2
        box_colors[i] = 2
        box_patterns[i] = i
        box_scales[i] = 0.8
        box_line_widths[i] = 1.

    opt = opt_base
    x += legend_width
    w.plscol0a( 15, 32, 32, 32, 0.70 )
    (legend_width, legend_height) = \
    w.pllegend( opt, position, x, y,
              0.1, 15, 1, 1, 0, 0, opt_array, 1.0, text_scale, 2.0,
              0., text_colors, text,
              box_colors, box_patterns, box_scales, box_line_widths,
              line_colors, line_styles, line_widths,
              symbol_colors, symbol_scales, symbol_numbers, symbols )
    max_height = max(max_height, legend_height)

    # Set up box legend entries with various box pattern line widths.
    for i in range(nlegend):
        opt_array[i] = w.PL_LEGEND_COLOR_BOX
        text[i] = "%s %d" % ("Box Line Width",i+1)
        text_colors[i] = 2
        box_colors[i] = 2
        box_patterns[i] = 3
        box_scales[i] = 0.8
        box_line_widths[i] = float(i+1)

    opt = opt_base
    x += legend_width
    w.plscol0a( 15, 32, 32, 32, 0.70 )
    (legend_width, legend_height) = \
    w.pllegend( opt, position, x, y,
              0.1, 15, 1, 1, 0, 0, opt_array, 1.0, text_scale, 2.0,
              0., text_colors, text,
              box_colors, box_patterns, box_scales, box_line_widths,
              line_colors, line_styles, line_widths,
              symbol_colors, symbol_scales, symbol_numbers, symbols )
    max_height = max(max_height, legend_height)

    # Set up line legend entries with various colours.
    for i in range(nlegend):
        opt_array[i] = w.PL_LEGEND_LINE
        text[i] = "%s %d" % ("Line Color",i+1)
        text_colors[i] = i+1
        line_colors[i] = i+1
        line_styles[i] = 1
        line_widths[i] = 1.

    opt = opt_base
    # Use new origin
    x = xstart
    y += max_height
    max_height = 0.
    w.plscol0a( 15, 32, 32, 32, 0.70 )
    (legend_width, legend_height) = \
    w.pllegend( opt, position, x, y,
              0.1, 15, 1, 1, 0, 0, opt_array, 1.0, text_scale, 2.0,
              0., text_colors, text,
              box_colors, box_patterns, box_scales, box_line_widths,
              line_colors, line_styles, line_widths,
              symbol_colors, symbol_scales, symbol_numbers, symbols )
    max_height = max(max_height, legend_height)

    # Set up line legend entries with various styles
    for i in range(nlegend):
        opt_array[i] = w.PL_LEGEND_LINE
        text[i] = "%s %d" % ("Line Style",i+1)
        text_colors[i] = 2
        line_colors[i] = 2
        line_styles[i] = i+1
        line_widths[i] = 1.

    opt = opt_base
    x += legend_width
    w.plscol0a( 15, 32, 32, 32, 0.70 )
    (legend_width, legend_height) = \
    w.pllegend( opt, position, x, y,
              0.1, 15, 1, 1, 0, 0, opt_array, 1.0, text_scale, 2.0,
              0., text_colors, text,
              box_colors, box_patterns, box_scales, box_line_widths,
              line_colors, line_styles, line_widths,
              symbol_colors, symbol_scales, symbol_numbers, symbols )
    max_height = max(max_height, legend_height)

    # Set up line legend entries with various widths.
    for i in range(nlegend):
        opt_array[i] = w.PL_LEGEND_LINE
        text[i] = "%s %d" % ("Line Width",i+1)
        text_colors[i] = 2
        line_colors[i] = 2
        line_styles[i] = 1
        line_widths[i] = float(i+1)

    opt = opt_base
    x += legend_width
    w.plscol0a( 15, 32, 32, 32, 0.70 )
    (legend_width, legend_height) = \
    w.pllegend( opt, position, x, y,
              0.1, 15, 1, 1, 0, 0, opt_array, 1.0, text_scale, 2.0,
              0., text_colors, text,
              box_colors, box_patterns, box_scales, box_line_widths,
              line_colors, line_styles, line_widths,
              symbol_colors, symbol_scales, symbol_numbers, symbols )
    max_height = max(max_height, legend_height)

    # Color bar examples
    values_small  = [ -1.0e-20, 1.0e-20 ]
    values_uneven = [ -1.0e-20, 2.0e-20, 2.6e-20, 3.4e-20, 6.0e-20, 7.0e-20, 8.0e-20, 9.0e-20, 10.0e-20 ]
    values_even   = [ -2.0e-20, -1.0e-20, 0.0e-20, 1.0e-20, 2.0e-20, 3.0e-20, 4.0e-20, 5.0e-20, 6.0e-20 ]

    # Use unsaturated green background colour to contrast with black caps.
    w.plscolbg( 70, 185, 70 )
    # Cut out the greatest and smallest bits of the color spectrum to
    # leave colors for the end caps.
    w.plscmap1_range( 0.01, 0.99 )

    # We can only test image and gradient colorbars with two element arrays
    for i in arange(COLORBAR_KINDS-2)+2:
        plcolorbar_example( w, "cmap1_blue_yellow.pal", i, 0, 0, values_small )
    # Test shade colorbars with larger arrays
    for i in arange(2):
        plcolorbar_example( w, "cmap1_blue_yellow.pal", i, 4, 2, values_even )
    for i in arange(2):
        plcolorbar_example( w, "cmap1_blue_yellow.pal", i, 0, 0, values_uneven )

    # Restore defaults
    w.plschr( 0.0, 1.0 )
    # cmap0 default color palette.
    w.plspal0("cmap0_default.pal")
    # cmap1 default color palette.
    w.plspal1("cmap1_default.pal", 1)

    # Must be done independently because otherwise this changes output files
    # and destroys agreement with C examples.
    #w.plcol0(1)
