#!/usr/bin/env python
# Test of all line styles and box styles using pllegend.

# Append to effective python path so that can find plplot modules.
from plplot_python_start import *

import sys
import plplot as w
from numpy import *
from math import *

# Parse and process command line arguments
w.plparseopts(sys.argv, w.PL_PARSE_FULL)

xmin,xmax,ymin,ymax = (0., 1., 0., 1.)
# Initialize plplot
w.plinit()

# Set up legend arrays with the correct size, type.
# w.pllsty takes integers from 1 to 8.
nlegend = 8

opt_array = zeros(nlegend, "int")
text_colors = zeros(nlegend, "int")
text = zeros(nlegend, "S200")
box_colors = zeros(nlegend, "int")
box_patterns = zeros(nlegend, "int")
box_scales = zeros(nlegend)
box_line_widths = zeros(nlegend, "int")
line_colors = zeros(nlegend, "int")
line_styles = zeros(nlegend, "int")
line_widths = zeros(nlegend, "int")
symbol_colors = zeros(nlegend, "int")
symbol_scales = zeros(nlegend)
symbol_numbers = zeros(nlegend, "int")
symbols = zeros(nlegend, "S100")

# Only specify legend data that are required according to the
# value of opt_array for that entry.
opt_base = w.PL_LEGEND_BACKGROUND | w.PL_LEGEND_BOUNDING_BOX | w.PL_LEGEND_TEXT_LEFT

w.pladv(0)
w.plvpor(0.1, 0.9, 0.1, 0.9)
w.plwind(xmin, xmax, ymin, ymax)
text_scale = 0.90

# Set up line legend entries with various styles
for i in range(nlegend):
    opt_array[i] = w.PL_LEGEND_LINE
    text[i] = "%s %d" % ("Line Style",i+1)
    text_colors[i] = 2
    line_colors[i] = 2
    line_styles[i] = i+1
    line_widths[i] = 1

opt = opt_base
w.plscol0a( 15, 32, 32, 32, 0.70 )
(legend_width, legend_height) = \
    w.pllegend( opt, w.PL_POSITION_LEFT, 0., 0.,
              0.1, 15, 1, 1, 0, 0, opt_array, 1.0, text_scale, 2.0,
              0., text_colors, text,
              box_colors, box_patterns, box_scales, box_line_widths,
              line_colors, line_styles, line_widths,
              symbol_colors, symbol_scales, symbol_numbers, symbols )

# Set up legend arrays with the correct size, type.
# w.plpsty takes integers from 0 to 8.
nlegend = 9

opt_array = zeros(nlegend, "int")
text_colors = zeros(nlegend, "int")
text = zeros(nlegend, "S200")
box_colors = zeros(nlegend, "int")
box_patterns = zeros(nlegend, "int")
box_scales = zeros(nlegend)
box_line_widths = zeros(nlegend, "int")
line_colors = zeros(nlegend, "int")
line_styles = zeros(nlegend, "int")
line_widths = zeros(nlegend, "int")
symbol_colors = zeros(nlegend, "int")
symbol_scales = zeros(nlegend)
symbol_numbers = zeros(nlegend, "int")
symbols = zeros(nlegend, "S100")

# Set up box legend entries with various patterns.
for i in range(nlegend):
    opt_array[i] = w.PL_LEGEND_COLOR_BOX
    text[i] = "%s %d" % ("Box Pattern",i)
    text_colors[i] = 2
    box_colors[i] = 2
    box_patterns[i] = i
    box_scales[i] = 0.8
    box_line_widths[i] = 1

opt = opt_base
w.plscol0a( 15, 32, 32, 32, 0.70 )
(legend_width, legend_height) = \
    w.pllegend( opt, w.PL_POSITION_RIGHT, 0., 0.,
              0.1, 15, 1, 1, 0, 0, opt_array, 1.0, text_scale, 2.0,
              0., text_colors, text,
              box_colors, box_patterns, box_scales, box_line_widths,
              line_colors, line_styles, line_widths,
              symbol_colors, symbol_scales, symbol_numbers, symbols )

w.plend()
