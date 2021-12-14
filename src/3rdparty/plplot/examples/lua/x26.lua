--[[
   Multi-lingual version of the first page of example 4.

   Copyright (C) 2009 Werner Smekal

   Thanks to the following for providing translated strings for this example:
   Valery Pipin (Russian)

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


--[[
  This example designed just for devices (e.g., psttfc and the
  cairo-related devices) that use the pango and fontconfig libraries. The
  best choice of glyph is selected by fontconfig and automatically rendered
  by pango in way that is sensitive to complex text layout (CTL) language
  issues for each unicode character in this example. Of course, you must
  have the appropriate TrueType fonts installed to have access to all the
  required glyphs.

  Translation instructions: The strings to be translated are given by
  x_label, y_label, alty_label, title_label, and line_label below.  The
  encoding used must be UTF-8.

  The following strings to be translated involve some scientific/mathematical
  jargon which is now discussed further to help translators.

  (1) dB is a decibel unit, see http://en.wikipedia.org/wiki/Decibel .
  (2) degrees is an angular measure, see
      http://en.wikipedia.org/wiki/Degree_(angle) .
  (3) low-pass filter is one that transmits (passes) low frequencies.
  (4) pole is in the mathematical sense, see
      http://en.wikipedia.org/wiki/Pole_(complex_analysis) .  "Single Pole"
      means a particular mathematical transformation of the filter function has
      a single pole, see
      http://ccrma.stanford.edu/~jos/filters/Pole_Zero_Analysis_I.html .
      Furthermore, a single-pole filter must have an inverse square decline
      (or -20 db/decade). Since the filter plotted here does have that
      characteristic, it must by definition be a single-pole filter, see also
      http://www-k.ext.ti.com/SRVS/Data/ti/KnowledgeBases/analog/document/faqs/1p.htm
  (5) decade represents a factor of 10, see
      http://en.wikipedia.org/wiki/Decade_(log_scale) .
--]]

-- initialise Lua bindings for PLplot examples.
dofile("plplot_examples.lua")

x_label = { "Frequency", "Частота" }
y_label = { "Amplitude (dB)", "Амплитуда (dB)" }
alty_label = { "Phase shift (degrees)", "Фазовый сдвиг (градусы)" }
-- Short rearranged versions of y_label and alty_label.
legend_text = {
  {"Amplitude", "Phase shift"},
  {"Амплитуда", "Фазовый сдвиг"}
  }
title_label = { "Single Pole Low-Pass Filter", "Однополюсный Низко-Частотный Фильтр" }
line_label = { "-20 dB/decade", "-20 dB/десяток" }

-- return single bit (for OR)
function bit(x,b)
  return ((x % 2^b) - (x % 2^(b-1)) > 0)
end

-- logic OR for number values

function lor(x,y)
  result = 0
  for p=1,8 do result = result + (((bit(x,p) or bit(y,p)) == true) and 2^(p-1) or 0) end
  return result
end

----------------------------------------------------------------------------
-- plot1
--
-- Log-linear plot.
----------------------------------------------------------------------------

function plot1(type, x_label, y_label, alty_label, title_label, line_label, legend_text)
  freql = {}
  ampl = {}
  phase = {}

  pl.adv(0)

  -- Set up data for log plot

  f0 = 1
  for i = 1, 101 do
    freql[i] = -2 + (i-1)/20
    freq = 10^freql[i]
    ampl[i] = 20 * math.log(1/math.sqrt(1+(freq/f0)^2),10)
    phase[i] = -180/math.pi*math.atan(freq/f0)
  end

  pl.vpor(0.15, 0.85, 0.1, 0.9)
  pl.wind(-2, 3, -80, 0)

  -- Try different axis and labelling styles.
  pl.col0(1)
  if type==0 then
    pl.box("bclnst", 0, 0, "bnstv", 0, 0)
  elseif type==1 then
    pl.box("bcfghlnst", 0, 0, "bcghnstv", 0, 0)
  else
    pl.abort("type must be 0 or 1")
    pl.plend()
    os.exit()
  end

  -- Plot ampl vs freq
  pl.col0(2)
  pl.line(freql, ampl)
  pl.col0(2)
  pl.ptex(1.6, -30, 1, -20, 0.5, line_label)

  -- Put labels on
  pl.col0(1)
  pl.mtex("b", 3.2, 0.5, 0.5, x_label)
  pl.mtex("t", 2, 0.5, 0.5, title_label)
  pl.col0(2)
  pl.mtex("l", 5, 0.5, 0.5, y_label)

  -- For the gridless case, put phase vs freq on same plot
  if type==0 then
    pl.col0(1)
    pl.wind(-2, 3, -100, 0)
    pl.box("", 0, 0, "cmstv", 30, 3)
    pl.col0(3)
    pl.line(freql, phase)
    pl.string(freql, phase, "#(728)")
    pl.col0(3)
    pl.mtex("r", 5, 0.5, 0.5, alty_label)
    nlegend = 2
  else
    nlegend = 1
  end

  -- Draw a legend.
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

  -- Data for the first legend entry.
  opt_array[1] = pl.PL_LEGEND_LINE
  text_colors[1] = 2
  text[1] = legend_text[1]
  -- box data unused so initialize to arbitrary values.
  box_colors[1] = 0
  box_patterns[1] = 0
  box_scales[1] = 0
  box_line_widths[1] = 0.
  line_colors[1] = 2
  line_styles[1] = 1
  line_widths[1] = 1.
  -- unused arbitrary data
  symbol_colors[1]  = 0
  symbol_scales[1]  = 0
  symbol_numbers[1] = 0
  symbols[1]        = ""

  -- Data for the second legend entry.
  if nlegend > 1 then
    opt_array[2]      = lor(pl.PL_LEGEND_LINE, pl.PL_LEGEND_SYMBOL)
    text_colors[2]    = 3
    text[2]           = legend_text[2]
  -- box data unused so initialize to arbitrary values.
    box_colors[2] = 0
    box_patterns[2] = 0
    box_scales[2] = 0
    box_line_widths[2] = 0.
    line_colors[2]    = 3
    line_styles[2]    = 1
    line_widths[2]    = 1.
    symbol_colors[2]  = 3
    symbol_scales[2]  = 1.
    symbol_numbers[2] = 4
    symbols[2]        = "#(728)"
  end

  pl.scol0a( 15, 32, 32, 32, 0.70 )

  legend_width, legend_height = pl.legend(
    lor(pl.PL_LEGEND_BACKGROUND, pl.PL_LEGEND_BOUNDING_BOX), 0, 0.0, 0.0,
    0.1, 15, 1, 1, 0, 0, opt_array, 1.0, 1.0, 2.0,
    1., text_colors, text,
    box_colors, box_patterns, box_scales, box_line_widths,
    line_colors, line_styles, line_widths,
    symbol_colors, symbol_scales, symbol_numbers, symbols )

end


----------------------------------------------------------------------------
-- main
--
-- Illustration of logarithmic axes, and redefinition of window.
----------------------------------------------------------------------------

-- Parse and process command line arguments
pl.parseopts(arg, pl.PL_PARSE_FULL)

-- Initialize plplot
pl.init()
nlang = # x_label
if nlang ~= (# y_label) or
  nlang ~= (# alty_label) or
  nlang ~= (# title_label) or
  nlang ~= (# line_label) or
   nlang ~= (# legend_text) then
  pl.abort("All data must be expressed in the same number of languages")
  pl.plend()
  os.exit()
end

pl.font(2)
-- Make log plots using two different styles.
for i = 1, nlang do
  plot1(0, x_label[i], y_label[i], alty_label[i], title_label[i],  line_label[i], legend_text[i])
end

pl.plend()
