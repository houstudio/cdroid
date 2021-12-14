# -*- coding: utf-8; -*-

#  Copyright (C) 2006-2016 Alan W. Irwin

#  Multi-lingual version of the first page of example 4.

#  Thanks to the following for providing translated strings for this example:
#  Valery Pipin (Russian)
#
#
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
#  This example designed just for devices (e.g., psttfc and the
#  cairo-related devices) that use the pango and fontconfig libraries. The
#  best choice of glyph is selected by fontconfig and automatically rendered
#  by pango in way that is sensitive to complex text layout (CTL) language
#  issues for each unicode character in this example. Of course, you must
#  have the appropriate TrueType fonts installed to have access to all the
#  required glyphs.
#
#  Translation instructions: The strings to be translated are given by
#  x_label, y_label, alty_label, title_label, and line_label below.  The encoding
#  used must be UTF-8.

# The following strings to be translated involve some scientific/mathematical jargon
# which is now discussed further to help translators.

# (1) dB is a decibel unit, see http://en.wikipedia.org/wiki/Decibel .
# (2) degrees is an angular measure, see http://en.wikipedia.org/wiki/Degree_(angle) .
# (3) low-pass filter is one that transmits (passes) low frequencies.
# (4) pole is in the mathematical sense, see
#     http://en.wikipedia.org/wiki/Pole_(complex_analysis) .  "Single Pole"
#     means a particular mathematical transformation of the filter function has
#     a single pole, see
#     http://ccrma.stanford.edu/~jos/filters/Pole_Zero_Analysis_I.html .  Furthermore,
#     a single-pole filter must have an inverse square decline (or -20 db/decade).
#     Since the filter plotted here does have that characteristic, it must by
#     definition be a single-pole filter, see also
#     http://www-k.ext.ti.com/SRVS/Data/ti/KnowledgeBases/analog/document/faqs/1p.htm
# (5) decade represents a factor of 10, see
#     http://en.wikipedia.org/wiki/Decade_(log_scale) .

import sys

from numpy import *

# main
#
# Illustration of first page of example 4 in different languages.

def main(w):
    x_label = [\
    "Frequency",\
    "Частота",\
    ]
    y_label = [\
    "Amplitude (dB)",\
    "Амплитуда (dB)",\
    ]
    alty_label = [\
    "Phase shift (degrees)",\
    "Фазовый сдвиг (градусы)",\
    ]
    # Short rearranged versions of y_label and alty_label.
    legend_text = [\
    [ "Amplitude", "Phase shift"],\
    [ "Амплитуда", "Фазовый сдвиг" ],\
    ]

    title_label = [\
    "Single Pole Low-Pass Filter",\
    "Однополюсный Низко-Частотный Фильтр",\
    ]
    line_label = [\
    "-20 dB/decade",\
    "-20 dB/десяток",\
    ]

    nlang = len(x_label)
    if len(y_label) != nlang or \
    len(alty_label) != nlang or \
    len(title_label) != nlang or \
    len(line_label) != nlang:
        raise RuntimeError("Internal inconsistency in label dimension")

    w.plfont(2)

    for i in range(nlang):
        plot1(w, 0, x_label[i], y_label[i], alty_label[i], \
        title_label[i], line_label[i], legend_text[i])

    # Restore defaults
    w.plfont(1)
    # cmap0 default color palette.
    w.plspal0("cmap0_default.pal")
    # Must be done independently because otherwise this changes output files
    # and destroys agreement with C examples.
    #w.plcol0(1)

def plot1(w, type, x_label, y_label, alty_label, title_label, line_label, legend_text):

    w.pladv(0)
    f0 = 1.0
    freql = -2.0 + arange(101)/20.0
    freq = pow(10.0, freql)
    ampl = 20.0 * log10(1.0 / sqrt(1.0 + pow((freq / f0), 2.)))
    phase = -(180.0 / pi) * arctan(freq / f0)

    w.plvpor(0.15, 0.85, 0.1, 0.9)
    w.plwind(-2., 3.0, -80.0, 0.0)
    w.plcol0(1)
    if type == 0:
        w.plbox("bclnst", 0.0, 0, "bnstv", 0.0, 0)
    elif type == 1:
        w.plbox("bcfghlnst", 0.0, 0, "bcghnstv", 0.0, 0)
    else:
        print("error: type must be either 0 or 1")
    w.plcol0(2)
    w.plline(freql, ampl)
    w.plcol0(2)
    w.plptex(1.6, -30.0, 1.0, -20.0, 0.5, line_label)

    # Put labels on
    w.plcol0(1)
    w.plmtex("b", 3.2, 0.5, 0.5, x_label)
    w.plmtex("t", 2.0, 0.5, 0.5, title_label)
    w.plcol0(2)
    w.plmtex("l", 5.0, 0.5, 0.5, y_label)

    # For the gridless case, put phase vs freq on same plot
    if type == 0:
        w.plcol0(1)
        w.plwind(-2.0, 3.0, -100.0, 0.0)
        w.plbox("", 0.0, 0, "cmstv", 30.0, 3)
        w.plcol0(3)
        w.plline(freql, phase)
        w.plstring(freql, phase, "#(728)")
        w.plcol0(3)
        w.plmtex("r", 5.0, 0.5, 0.5, alty_label)
        nlegend = 2
    else:
        nlegend = 1

    # Draw a legend.
    # Set up legend arrays with the correct size, type.
    # legend_text is already supplied by argument.
    opt_array = zeros(nlegend, "int")
    # Be generous with size of string since raw UTF-8 is involved.
    text = zeros(nlegend, "S100")
    text_colors = zeros(nlegend, "int")
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
    symbols = zeros(nlegend, "S10")
    # Only specify legend data that are required according to the
    # value of opt_array for that entry.

    # Data for first legend entry.
    opt_array[0] = w.PL_LEGEND_LINE
    text_colors[0] = 2
    text[0] = legend_text[0].encode() if (sys.version_info > (3, 0)) else legend_text[0]
    line_colors[0] = 2
    line_styles[0] = 1
    line_widths[0] = 1.

    # Data for second legend entry.
    if nlegend > 1:
        opt_array[1]      = w.PL_LEGEND_LINE | w.PL_LEGEND_SYMBOL
        text_colors[1]    = 3
        text[1] = legend_text[1].encode() if (sys.version_info > (3, 0)) else legend_text[1]
        line_colors[1]    = 3
        line_styles[1]    = 1
        line_widths[1]    = 1.
        symbol_colors[1]  = 3
        symbol_scales[1]  = 1.
        symbol_numbers[1] = 4
        symbols[1]        = "#(728)"

    w.plscol0a( 15, 32, 32, 32, 0.70 )

    (legend_width, legend_height) = \
    w.pllegend( w.PL_LEGEND_BACKGROUND | w.PL_LEGEND_BOUNDING_BOX, 0, 0.0, 0.0,
              0.1, 15, 1, 1, 0, 0, opt_array, 1.0, 1.0, 2.0,
              1., text_colors, text,
              box_colors, box_patterns, box_scales, box_line_widths,
              line_colors, line_styles, line_widths,
              symbol_colors, symbol_scales, symbol_numbers, symbols )
