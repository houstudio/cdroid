--[[
  Demonstrate most pllegend capability including unicode symbols.

  Copyright (C) 2010 Alan W. Irwin

  This file is part of PLplot.

  PLplot is free software you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as published
  by the Free Software Foundation either version 2 of the License, or
  (at your option) any later version.

  PLplot is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with PLplot if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
--]]

-- initialise Lua bindings for PLplot examples.
dofile("plplot_examples.lua")

-- return single bit (for OR)
function bit(x,b)
  return ((x % 2^b) - (x % 2^(b-1)) > 0)
end

-- logic OR for number values
function lor(x,y)
  result = 0
  for p=1,32 do result = result + (((bit(x,p) or bit(y,p)) == true) and 2^(p-1) or 0) end
  return result
end

-- logic AND for number values
function land(x,y)
  result = 0
  for p=1,32 do result = result + (((bit(x,p) and bit(y,p)) == true) and 2^(p-1) or 0) end
  return result
end

-- Intialize pllegend arrays taking advantage of Lua's default global scope.

function initialize_pllegend_arrays(nlegend)
  opt_array = {}
  text_colors = {}
  text = {}
  box_colors = {}
  box_patterns = {}
  box_scales = {}
  box_line_widths = {}
  line_colors = {}
  line_styles = {}
  line_widths = {}
  symbol_colors = {}
  symbol_scales = {}
  symbol_numbers = {}
  symbols = {}
  for i=1,nlegend do
    opt_array[i] = 0
    text_colors[i] = 0
    text[i] = ""
    box_colors[i] = 0
    box_patterns[i] = 0
    box_scales[i] = 0.0
    box_line_widths[i] = 0.
    line_colors[i] = 0
    line_styles[i] = 0
    line_widths[i] = 0.
    symbol_colors[i] = 0
    symbol_scales[i] = 0.0
    symbol_numbers[i] = 0
    symbols[i] = ""
  end
end

function plcolorbar_example_page( kind_i, label_i, cap_i, cont_color, cont_width, n_values, values )
    -- Parameters for the colorbars on this page
    ticks = { 0.0 }
    sub_ticks = { 0 }
    axis_opts = {}
    label_opts = { 0 }
    n_values_array = {}
    values_array = {}
    label = {}

    n_values_array[1] = n_values
    values_array[1]   = values

    low_cap_color  = 0.0
    high_cap_color = 1.0

    -- Start a new page
    pl.adv( 0 )

    -- Draw one colorbar relative to each side of the page
    for position_i = 0,COLORBAR_POSITIONS-1 do
        position = colorbar_position_options[position_i+1]
        opt      =
            lor(colorbar_option_kinds[kind_i+1],
            lor(colorbar_label_options[label_i+1],
            colorbar_cap_options[cap_i+1]))

        vertical = lor(land(position,pl.PL_POSITION_LEFT), land(position,pl.PL_POSITION_RIGHT))
        ifn      = lor(land(position, pl.PL_POSITION_LEFT), land(position, pl.PL_POSITION_BOTTOM))

        -- Set the offset position on the page
        if vertical > 0 then
            x        = 0.0
            y        = 0.0
            x_length = 0.05
            y_length = 0.5
        else
            x        = 0.0
            y        = 0.0
            x_length = 0.5
            y_length = 0.05
        end

        -- Set appropriate labelling options.
        if ifn > 0 then
            if ( cont_color == 0 ) or ( cont_width == 0. ) then
                axis_opts[1] = "uwtivn"
            else
                axis_opts[1] = "uwxvn"
            end
        else
            if ( cont_color == 0 ) or ( cont_width == 0. ) then
                axis_opts[1] = "uwtivm"
            else
                axis_opts[1] = "uwxvm"
            end
        end

        label[1] = string.format( "%s, %s",
            colorbar_position_option_labels[position_i+1],
            colorbar_label_option_labels[label_i+1] )

        -- Smaller text
        pl.schr( 0.0, 0.75 )
        -- Small ticks on the vertical axis
        pl.smaj( 0.0, 0.5 )
        pl.smin( 0.0, 0.5 )

        pl.vpor( 0.20, 0.80, 0.20, 0.80 )
        pl.wind( 0.0, 1.0, 0.0, 1.0 )
        -- Set interesting background colour.
        pl.scol0a( 15, 0, 0, 0, 0.20 );
        colorbar_width, colorbar_height = pl.colorbar(
            lor(opt, lor(pl.PL_COLORBAR_BOUNDING_BOX, pl.PL_COLORBAR_BACKGROUND)),  position,
            x, y, x_length, y_length,
            15, 1, 1,
            low_cap_color, high_cap_color,
            cont_color, cont_width,
            label_opts, label,
            axis_opts, ticks, sub_ticks,
            n_values_array, values_array )

        -- Reset text and tick sizes
        pl.schr( 0.0, 1.0 )
        pl.smaj( 0.0, 1.0 )
        pl.smin( 0.0, 1.0 )
    end

    -- Draw a page title
    title = string.format( "%s - %s",
        colorbar_option_kind_labels[kind_i+1],
        colorbar_cap_option_labels[cap_i+1] )
    pl.vpor( 0.0, 1.0, 0.0, 1.0 )
    pl.wind( 0.0, 1.0, 0.0, 1.0 )
    pl.ptex( 0.5, 0.5, 0.0, 0.0, 0.5, title )
end

function plcolorbar_example( palette, kind_i, cont_color, cont_width, n_values, values )
    -- Load the color palette
    pl.spal1( palette, 1 )

    for label_i = 0, COLORBAR_LABELS-1 do
        for cap_i = 0, COLORBAR_CAPS-1 do
            plcolorbar_example_page( kind_i, label_i, cap_i,
                cont_color, cont_width,
                n_values, values )
        end
    end
end

----------------------------------------------------------------------------
-- main
----------------------------------------------------------------------------

position_options = {
lor(pl.PL_POSITION_LEFT, lor(pl.PL_POSITION_TOP , pl.PL_POSITION_OUTSIDE)),
lor(pl.PL_POSITION_TOP, pl.PL_POSITION_OUTSIDE),
lor(pl.PL_POSITION_RIGHT, lor(pl.PL_POSITION_TOP, pl.PL_POSITION_OUTSIDE)),
lor(pl.PL_POSITION_RIGHT, pl.PL_POSITION_OUTSIDE),
lor(pl.PL_POSITION_RIGHT, lor(pl.PL_POSITION_BOTTOM, pl.PL_POSITION_OUTSIDE)),
lor(pl.PL_POSITION_BOTTOM, pl.PL_POSITION_OUTSIDE),
lor(pl.PL_POSITION_LEFT, lor(pl.PL_POSITION_BOTTOM, pl.PL_POSITION_OUTSIDE)),
lor(pl.PL_POSITION_LEFT, pl.PL_POSITION_OUTSIDE),
lor(pl.PL_POSITION_LEFT, lor(pl.PL_POSITION_TOP, pl.PL_POSITION_INSIDE)),
lor(pl.PL_POSITION_TOP, pl.PL_POSITION_INSIDE),
lor(pl.PL_POSITION_RIGHT, lor(pl.PL_POSITION_TOP, pl.PL_POSITION_INSIDE)),
lor(pl.PL_POSITION_RIGHT, pl.PL_POSITION_INSIDE),
lor(pl.PL_POSITION_RIGHT, lor(pl.PL_POSITION_BOTTOM, pl.PL_POSITION_INSIDE)),
lor(pl.PL_POSITION_BOTTOM, pl.PL_POSITION_INSIDE),
lor(pl.PL_POSITION_LEFT, lor(pl.PL_POSITION_BOTTOM, pl.PL_POSITION_INSIDE)),
lor(pl.PL_POSITION_LEFT, pl.PL_POSITION_INSIDE)
}

-- Pick 5 arbitrary UTF-8 symbols useful for plotting points (✠✚✱✪✽✺✰✴✦).
special_symbols = {
"✰",
"✴",
"✱",
"✽",
"✦"
}

-- plcolorbar options
COLORBAR_KINDS = 4
-- Colorbar type options
colorbar_option_kinds = {
    pl.PL_COLORBAR_SHADE,
    lor(pl.PL_COLORBAR_SHADE, pl.PL_COLORBAR_SHADE_LABEL),
    pl.PL_COLORBAR_IMAGE,
    pl.PL_COLORBAR_GRADIENT
}

colorbar_option_kind_labels = {
    "Shade colorbars",
    "Shade colorbars with custom labels",
    "Image colorbars",
    "Gradient colorbars"
}

-- Which side of the page are we positioned relative to?
COLORBAR_POSITIONS = 4
colorbar_position_options = {
    pl.PL_POSITION_LEFT,
    pl.PL_POSITION_RIGHT,
    pl.PL_POSITION_TOP,
    pl.PL_POSITION_BOTTOM
}

colorbar_position_option_labels = {
    "Left",
    "Right",
    "Top",
    "Bottom"
}

-- Colorbar label positioning options
COLORBAR_LABELS = 4
colorbar_label_options = {
    pl.PL_COLORBAR_LABEL_LEFT,
    pl.PL_COLORBAR_LABEL_RIGHT,
    pl.PL_COLORBAR_LABEL_TOP,
    pl.PL_COLORBAR_LABEL_BOTTOM
}
colorbar_label_option_labels = {
    "Label left",
    "Label right",
    "Label top",
    "Label bottom"
}

-- Colorbar cap options
COLORBAR_CAPS = 4
colorbar_cap_options = {
    pl.PL_COLORBAR_CAP_NONE,
    pl.PL_COLORBAR_CAP_LOW,
    pl.PL_COLORBAR_CAP_HIGH,
    lor(pl.PL_COLORBAR_CAP_LOW, pl.PL_COLORBAR_CAP_HIGH)
}
colorbar_cap_option_labels = {
    "No caps",
    "Low cap",
    "High cap",
    "Low and high caps"
}

-- Parse and process command line arguments
pl.parseopts(arg, pl.PL_PARSE_FULL)

-- Initialize plplot
pl.init()

-- First page illustrating the 16 standard positions.
pl.adv(0)
pl.vpor(0.25, 0.75, 0.25, 0.75)
pl.wind(0.0, 1.0, 0.0, 1.0)
pl.box("bc", 0.0, 0, "bc", 0.0, 0)
pl.sfont(pl.PL_FCI_SANS, -1, -1)
pl.mtex("t", 8.0, 0.5, 0.5, "The 16 standard legend positions with")
pl.mtex("t", 6.0, 0.5, 0.5, "the same (0.05) offset in x and y")

nlegend = 1

-- Set up legend arrays with the correct size, type.
initialize_pllegend_arrays(nlegend)

-- Only specify legend data that are required according to the
-- value of opt_array for that entry.
opt_base = lor(pl.PL_LEGEND_BACKGROUND, pl.PL_LEGEND_BOUNDING_BOX)
opt_array[1] = lor(pl.PL_LEGEND_LINE, pl.PL_LEGEND_SYMBOL)
line_styles[1] = 1
line_widths[1] = 1.
symbol_scales[1] = 1.0
symbol_numbers[1] = 4
symbols[1] = "#(728)"

-- Use monotype fonts so that all legends are the same size.
pl.sfont(pl.PL_FCI_MONO, -1, -1)
pl.scol0a( 15, 32, 32, 32, 0.70 )

for k=0,15 do
  position = position_options[k+1]
  opt = opt_base
  text[1] = string.format("%2.2d", k)
  text_colors[1] = 1 + (k % 8)
  line_colors[1] = 1 + (k % 8)
  symbol_colors[1] = 1 + (k % 8)
  legend_width, legend_height = pl.legend(
    opt, position, 0.05, 0.05,
    0.1, 15, 1, 1, 0, 0, opt_array, 1.0, 1.0, 2.0,
    1.0, text_colors, text,
    box_colors, box_patterns, box_scales, box_line_widths,
    line_colors, line_styles, line_widths,
    symbol_colors, symbol_scales, symbol_numbers, symbols )
end

-- Second page illustrating effect of nrow, ncolumn for the same legend
-- data
pl.adv(0)
pl.vpor(0.25, 0.75, 0.25, 0.75)
pl.wind(0.0, 1.0, 0.0, 1.0)
pl.box("bc", 0.0, 0, "bc", 0.0, 0)
pl.sfont(pl.PL_FCI_SANS, -1, -1)
pl.mtex("t", 8.0, 0.5, 0.5, "The effect of nrow, ncolumn, PL_LEGEND_ROW_MAJOR,")
pl.mtex("t", 6.0, 0.5, 0.5, "and position for the same legend data")

-- Set up legend arrays with the correct size, type.
nlegend = 7
initialize_pllegend_arrays(nlegend)

-- Only specify legend data that are required according to the
-- value of opt_array for that entry.
opt_base = lor(pl.PL_LEGEND_BACKGROUND, pl.PL_LEGEND_BOUNDING_BOX)
for k = 0, nlegend-1 do
  opt_array[k+1] = lor(pl.PL_LEGEND_LINE, pl.PL_LEGEND_SYMBOL)
  line_styles[k+1] = 1
  line_widths[k+1] = 1.
  symbol_scales[k+1] = 1.0
  symbol_numbers[k+1] = 2
  symbols[k+1] = "#(728)"
  text[k+1] = string.format("%2.2d", k)
  text_colors[k+1] = 1 + (k % 8)
  line_colors[k+1] = 1 + (k % 8)
  symbol_colors[k+1] = 1 + (k % 8)
end

-- Use monotype fonts so that all legends are the same size.
pl.sfont(pl.PL_FCI_MONO, -1, -1)
pl.scol0a( 15, 32, 32, 32, 0.70 )

position = lor(pl.PL_POSITION_TOP, pl.PL_POSITION_OUTSIDE)
opt = opt_base
x = 0.
y = 0.1
nrow = 1
ncolumn = nlegend
legend_width, legend_height = pl.legend(
  opt, position, x, y,
  0.05, 15, 1, 1, nrow, ncolumn, opt_array, 1.0, 1.0, 2.0,
  1.0, text_colors, text,
  box_colors, box_patterns, box_scales, box_line_widths,
  line_colors, line_styles, line_widths,
  symbol_colors, symbol_scales, symbol_numbers, symbols )

position = lor(pl.PL_POSITION_BOTTOM, pl.PL_POSITION_OUTSIDE)
opt = opt_base
x = 0.
y = 0.1
nrow = 1
ncolumn = nlegend
legend_width, legend_height = pl.legend(
  opt, position, x, y,
  0.05, 15, 1, 1, nrow, ncolumn, opt_array, 1.0, 1.0, 2.0,
  1.0, text_colors, text,
  box_colors, box_patterns, box_scales, box_line_widths,
  line_colors, line_styles, line_widths,
  symbol_colors, symbol_scales, symbol_numbers, symbols )

position = lor(pl.PL_POSITION_LEFT, pl.PL_POSITION_OUTSIDE)
opt = opt_base
x = 0.1
y = 0.
nrow = nlegend
ncolumn = 1
legend_width, legend_height = pl.legend(
  opt, position, x, y,
  0.05, 15, 1, 1, nrow, ncolumn, opt_array, 1.0, 1.0, 2.0,
  1.0, text_colors, text,
  box_colors, box_patterns, box_scales, box_line_widths,
  line_colors, line_styles, line_widths,
  symbol_colors, symbol_scales, symbol_numbers, symbols )

position = lor(pl.PL_POSITION_RIGHT, pl.PL_POSITION_OUTSIDE)
opt = opt_base
x = 0.1
y = 0.
nrow = nlegend
ncolumn = 1
legend_width, legend_height = pl.legend(
  opt, position, x, y,
  0.05, 15, 1, 1, nrow, ncolumn, opt_array, 1.0, 1.0, 2.0,
  1.0, text_colors, text,
  box_colors, box_patterns, box_scales, box_line_widths,
  line_colors, line_styles, line_widths,
  symbol_colors, symbol_scales, symbol_numbers, symbols )

position = lor(pl.PL_POSITION_LEFT, lor(pl.PL_POSITION_TOP, pl.PL_POSITION_INSIDE))
opt = opt_base
x = 0.
y = 0.
nrow = 6
ncolumn = 2
legend_width, legend_height = pl.legend(
  opt, position, x, y,
  0.05, 15, 1, 1, nrow, ncolumn, opt_array, 1.0, 1.0, 2.0,
  1.0, text_colors, text,
  box_colors, box_patterns, box_scales, box_line_widths,
  line_colors, line_styles, line_widths,
  symbol_colors, symbol_scales, symbol_numbers, symbols )

position = lor(pl.PL_POSITION_RIGHT, lor(pl.PL_POSITION_TOP, pl.PL_POSITION_INSIDE))
opt = lor(opt_base, pl.PL_LEGEND_ROW_MAJOR)
x = 0.
y = 0.
nrow = 6
ncolumn = 2
legend_width, legend_height = pl.legend(
  opt, position, x, y,
  0.05, 15, 1, 1, nrow, ncolumn, opt_array, 1.0, 1.0, 2.0,
  1.0, text_colors, text,
  box_colors, box_patterns, box_scales, box_line_widths,
  line_colors, line_styles, line_widths,
  symbol_colors, symbol_scales, symbol_numbers, symbols )

position = lor(pl.PL_POSITION_BOTTOM, pl.PL_POSITION_INSIDE)
opt = lor(opt_base, pl.PL_LEGEND_ROW_MAJOR)
x = 0.
y = 0.
nrow = 3
ncolumn = 3
legend_width, legend_height = pl.legend(
  opt, position, x, y,
  0.05, 15, 1, 1, nrow, ncolumn, opt_array, 1.0, 1.0, 2.0,
  1.0, text_colors, text,
  box_colors, box_patterns, box_scales, box_line_widths,
  line_colors, line_styles, line_widths,
  symbol_colors, symbol_scales, symbol_numbers, symbols )

-- Third page demonstrating legend alignment
pl.adv(0)
pl.vpor(0.0, 1.0, 0.0, 0.9)
pl.wind(0.0, 1.0, 0.0, 1.0)
pl.sfont(pl.PL_FCI_SANS, -1, -1)
pl.mtex("t", 2.0, 0.5, 0.5, "Demonstrate legend alignment")

x = 0.1
y = 0.1
nturn = 4
nlegend = 0
position = lor(pl.PL_POSITION_TOP, lor(pl.PL_POSITION_LEFT, pl.PL_POSITION_SUBPAGE))
opt_base = lor(pl.PL_LEGEND_BACKGROUND, pl.PL_LEGEND_BOUNDING_BOX)
opt = opt_base
for i=0,8 do
  -- Set up legend arrays with the correct size, type.
  if i <= nturn then
    nlegend = nlegend + 1
  else
    nlegend = nlegend - 1
  end
  nlegend = math.max(1, nlegend)
  initialize_pllegend_arrays(nlegend)
  -- Only specify legend data that are required according to the
  -- value of opt_array for that entry.
  for k = 0, nlegend-1 do
    opt_array[k+1] = lor(pl.PL_LEGEND_LINE, pl.PL_LEGEND_SYMBOL)
    line_styles[k+1] = 1
    line_widths[k+1] = 1.
    symbol_scales[k+1] = 1.0
    symbol_numbers[k+1] = 2
    symbols[k+1] = "#(728)"
    text[k+1] = string.format("%2.2d", k)
    text_colors[k+1] = 1 + (k % 8)
    line_colors[k+1] = 1 + (k % 8)
    symbol_colors[k+1] = 1 + (k % 8)
  end
  -- Use monotype fonts so that all legends are the same size.
  pl.sfont(pl.PL_FCI_MONO, -1, -1)
  pl.scol0a( 15, 32, 32, 32, 0.70 )

  nrow = math.min(3, nlegend)
  ncolumn = 0
  legend_width, legend_height = pl.legend(
    opt, position, x, y,
    0.025, 15, 1, 1, nrow, ncolumn, opt_array, 1.0, 1.0, 1.5,
    1.0, text_colors, text,
    box_colors, box_patterns, box_scales, box_line_widths,
    line_colors, line_styles, line_widths,
    symbol_colors, symbol_scales, symbol_numbers, symbols )
  if i==nturn then
    position = lor(pl.PL_POSITION_TOP, lor(pl.PL_POSITION_RIGHT, pl.PL_POSITION_SUBPAGE))
    opt = opt_base
    x = 1.0 - x
    y = y + legend_height
  else
    x = x + legend_width
    y = y + legend_height
  end
end

-- Fourth page illustrating various kinds of legends
max_height = 0.
xstart = 0.0
ystart = 0.1
x = xstart
y = ystart
text_scale = 0.90
pl.adv(0)
pl.vpor(0.0, 1.0, 0.0, 0.90)
pl.wind(0.0, 1.0, 0.0, 1.0)
-- pl.box("bc", 0.0, 0, "bc", 0.0, 0)
pl.sfont(pl.PL_FCI_SANS, -1, -1)
pl.mtex("t", 2.0, 0.5, 0.5, "Demonstrate Various Kinds of Legends")

-- Set up legend arrays with the correct size, type.
nlegend = 5
initialize_pllegend_arrays(nlegend)

-- Only specify legend data that are required according to the
-- value of opt_array for that entry.
position = lor(pl.PL_POSITION_LEFT, pl.PL_POSITION_TOP)
opt_base = lor(pl.PL_LEGEND_BACKGROUND, lor(pl.PL_LEGEND_BOUNDING_BOX, pl.PL_LEGEND_TEXT_LEFT))

-- Set up None, Box, Line, Symbol, and Line & Symbol legend entries.
opt_array[1] = pl.PL_LEGEND_NONE
text[1] = "None"
text_colors[1] = 1

opt_array[2] = pl.PL_LEGEND_COLOR_BOX
text[2] = "Box"
text_colors[2] = 2
box_colors[2] = 2
box_patterns[2] = 0
box_scales[2] = 0.8
box_line_widths[2] = 1.

opt_array[3] = pl.PL_LEGEND_LINE
text[3] = "Line"
text_colors[3] = 3
line_colors[3] = 3
line_styles[3] = 1
line_widths[3] = 1.

opt_array[4] = pl.PL_LEGEND_SYMBOL
text[4] = "Symbol"
text_colors[4] = 4
symbol_colors[4] = 4
symbol_scales[4] = text_scale
symbol_numbers[4] = 4
symbols[4] = special_symbols[3]

opt_array[5] = lor(pl.PL_LEGEND_SYMBOL, pl.PL_LEGEND_LINE)
text[5] = "L & S"
text_colors[5] = 5
line_colors[5] = 5
line_styles[5] = 1
line_widths[5] = 1.
symbol_colors[5] = 5
symbol_scales[5] = text_scale
symbol_numbers[5] = 4
symbols[5] = special_symbols[3]

opt = opt_base
pl.scol0a( 15, 32, 32, 32, 0.70 )
legend_width, legend_height = pl.legend(
  opt, position, x, y,
  0.1, 15, 1, 1, 0, 0, opt_array, 1.0, text_scale, 2.0,
  0.0, text_colors, text,
  box_colors, box_patterns, box_scales, box_line_widths,
  line_colors, line_styles, line_widths,
  symbol_colors, symbol_scales, symbol_numbers, symbols )

-- Set up symbol legend entries with various symbols.
for i=0,nlegend-1 do
  opt_array[i+1] = pl.PL_LEGEND_SYMBOL
  text[i+1] = "Symbol " .. special_symbols[i+1]
  text_colors[i+1] = i+1
  symbol_colors[i+1] = i+1
  symbol_scales[i+1] = text_scale
  symbol_numbers[i+1] = 4
  symbols[i+1] = special_symbols[i+1]
end

opt = opt_base
x = x + legend_width
pl.scol0a( 15, 32, 32, 32, 0.70 )
legend_width, legend_height = pl.legend(
  opt, position, x, y,
  0.1, 15, 1, 1, 0, 0, opt_array, 1.0, text_scale, 2.0,
  0.0, text_colors, text,
  box_colors, box_patterns, box_scales, box_line_widths,
  line_colors, line_styles, line_widths,
  symbol_colors, symbol_scales, symbol_numbers, symbols )
max_height = math.max(max_height, legend_height)

-- Set up symbol legend entries with various numbers of symbols.
for i=0,nlegend-1 do
  opt_array[i+1] = pl.PL_LEGEND_SYMBOL
  text[i+1] = string.format("Symbol Number %d", i+2)
  text_colors[i+1] = i+1
  symbol_colors[i+1] = i+1
  symbol_scales[i+1] = text_scale
  symbol_numbers[i+1] = i+2
  symbols[i+1] = special_symbols[3]
end

opt = opt_base
x = x + legend_width
pl.scol0a( 15, 32, 32, 32, 0.70 )
legend_width, legend_height = pl.legend(
  opt, position, x, y,
  0.1, 15, 1, 1, 0, 0, opt_array, 1.0, text_scale, 2.0,
  0.0, text_colors, text,
  box_colors, box_patterns, box_scales, box_line_widths,
  line_colors, line_styles, line_widths,
  symbol_colors, symbol_scales, symbol_numbers, symbols )
max_height = math.max(max_height, legend_height)

-- Set up box legend entries with various colours.
for i=0,nlegend-1 do
  opt_array[i+1] = pl.PL_LEGEND_COLOR_BOX
  text[i+1] = string.format("%s %d", "Box Color", i+1)
  text_colors[i+1] = i+1
  box_colors[i+1] = i+1
  box_patterns[i+1] = 0
  box_scales[i+1] = 0.8
  box_line_widths[i+1] = 1.
end

opt = opt_base
-- Use new origin
x = xstart
y = y + max_height
max_height = 0
pl.scol0a( 15, 32, 32, 32, 0.70 )
legend_width, legend_height = pl.legend(
  opt, position, x, y,
  0.1, 15, 1, 1, 0, 0, opt_array, 1.0, text_scale, 2.0,
  0.0, text_colors, text,
  box_colors, box_patterns, box_scales, box_line_widths,
  line_colors, line_styles, line_widths,
  symbol_colors, symbol_scales, symbol_numbers, symbols )
max_height = math.max(max_height, legend_height)

-- Set up box legend entries with various patterns.
for i=0,nlegend-1 do
  opt_array[i+1] = pl.PL_LEGEND_COLOR_BOX
  text[i+1] = string.format("%s %d", "Box Pattern", i)
  text_colors[i+1] = 2
  box_colors[i+1] = 2
  box_patterns[i+1] = i
  box_scales[i+1] = 0.8
  box_line_widths[i+1] = 1.
end

opt = opt_base
x = x + legend_width
pl.scol0a( 15, 32, 32, 32, 0.70 )
legend_width, legend_height = pl.legend(
  opt, position, x, y,
  0.1, 15, 1, 1, 0, 0, opt_array, 1.0, text_scale, 2.0,
  0.0, text_colors, text,
  box_colors, box_patterns, box_scales, box_line_widths,
  line_colors, line_styles, line_widths,
  symbol_colors, symbol_scales, symbol_numbers, symbols )
max_height = math.max(max_height, legend_height)

-- Set up box legend entries with various pattern line widths.
for i=0,nlegend-1 do
  opt_array[i+1] = pl.PL_LEGEND_COLOR_BOX
  text[i+1] = string.format("%s %d", "Box Line Width", i+1)
  text_colors[i+1] = 2
  box_colors[i+1] = 2
  box_patterns[i+1] = 3
  box_scales[i+1] = 0.8
  box_line_widths[i+1] = i+1
end

opt = opt_base
x = x + legend_width
pl.scol0a( 15, 32, 32, 32, 0.70 )
legend_width, legend_height = pl.legend(
  opt, position, x, y,
  0.1, 15, 1, 1, 0, 0, opt_array, 1.0, text_scale, 2.0,
  0.0, text_colors, text,
  box_colors, box_patterns, box_scales, box_line_widths,
  line_colors, line_styles, line_widths,
  symbol_colors, symbol_scales, symbol_numbers, symbols )
max_height = math.max(max_height, legend_height)

-- Set up line legend entries with various colours.
for i=0,nlegend-1 do
  opt_array[i+1] = pl.PL_LEGEND_LINE
  text[i+1] = string.format("%s %d", "Line Color", i+1)
  text_colors[i+1] = i+1
  line_colors[i+1] = i+1
  line_styles[i+1] = 1
  line_widths[i+1] = 1.
end

opt = opt_base
-- Use new origin
x = xstart
y = y + max_height
max_height = 0
pl.scol0a( 15, 32, 32, 32, 0.70 )
legend_width, legend_height = pl.legend(
  opt, position, x, y,
  0.1, 15, 1, 1, 0, 0, opt_array, 1.0, text_scale, 2.0,
  0.0, text_colors, text,
  box_colors, box_patterns, box_scales, box_line_widths,
  line_colors, line_styles, line_widths,
  symbol_colors, symbol_scales, symbol_numbers, symbols )
max_height = math.max(max_height, legend_height)

-- Set up line legend entries with various styles.
for i=0,nlegend-1 do
  opt_array[i+1] = pl.PL_LEGEND_LINE
  text[i+1] = string.format("%s %d", "Line Style", i+1)
  text_colors[i+1] = 2
  line_colors[i+1] = 2
  line_styles[i+1] = i+1
  line_widths[i+1] = 1.
end

opt = opt_base
x = x + legend_width
pl.scol0a( 15, 32, 32, 32, 0.70 )
legend_width, legend_height = pl.legend(
  opt, position, x, y,
  0.1, 15, 1, 1, 0, 0, opt_array, 1.0, text_scale, 2.0,
  0.0, text_colors, text,
  box_colors, box_patterns, box_scales, box_line_widths,
  line_colors, line_styles, line_widths,
  symbol_colors, symbol_scales, symbol_numbers, symbols )
max_height = math.max(max_height, legend_height)

-- Set up line legend entries with various widths.
for i=0,nlegend-1 do
  opt_array[i+1] = pl.PL_LEGEND_LINE
  text[i+1] = string.format("%s %d", "Line Width", i+1)
  text_colors[i+1] = 2
  line_colors[i+1] = 2
  line_styles[i+1] = 1
  line_widths[i+1] = i+1
end

opt = opt_base
x = x + legend_width
pl.scol0a( 15, 32, 32, 32, 0.70 )
legend_width, legend_height = pl.legend(
  opt, position, x, y,
  0.1, 15, 1, 1, 0, 0, opt_array, 1.0, text_scale, 2.0,
  0.0, text_colors, text,
  box_colors, box_patterns, box_scales, box_line_widths,
  line_colors, line_styles, line_widths,
  symbol_colors, symbol_scales, symbol_numbers, symbols )
max_height = math.max(max_height, legend_height)

-- Color bar examples
values_small  = { -1.0e-20, 1.0e-20 }
values_uneven = { -1.0e-20, 2.0e-20, 2.6e-20, 3.4e-20, 6.0e-20, 7.0e-20, 8.0e-20, 9.0e-20, 10.0e-20 }
values_even   = { -2.0e-20, -1.0e-20, 0.0e-20, 1.0e-20, 2.0e-20, 3.0e-20, 4.0e-20, 5.0e-20, 6.0e-20 }

-- Use unsaturated green background colour to contrast with black caps.
pl.scolbg( 70, 185, 70 )
-- Cut out the greatest and smallest bits of the color spectrum to
-- leave colors for the end caps.
pl.scmap1_range( 0.01, 0.99 )

-- We can only test image and gradient colorbars with two element arrays
for i = 2, COLORBAR_KINDS-1 do
    plcolorbar_example( "cmap1_blue_yellow.pal", i, 0, 0, 2, values_small )
end
-- Test shade colorbars with larger arrays
for i = 0,1 do
    plcolorbar_example( "cmap1_blue_yellow.pal", i, 4, 2, 9, values_even )
end
for i = 0,1 do
    plcolorbar_example( "cmap1_blue_yellow.pal", i, 0, 0, 9, values_uneven )
end

pl.plend()
