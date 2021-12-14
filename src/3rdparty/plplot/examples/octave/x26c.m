##  Multi-lingual version of the first page of example 4.
##
##  Copyright (C) 2006 Alan Irwin
##  Copyright (C) 2006 Andrew Ross
##
##  Thanks to the following for providing translated strings for this example:
##  Valery Pipin (Russian)
##
##  This file is part of PLplot.
##
##  PLplot is free software; you can redistribute it and/or modify
##  it under the terms of the GNU Library General Public License as published
##  by the Free Software Foundation; either version 2 of the License, or
##  (at your option) any later version.
##
##  PLplot is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU Library General Public License for more details.
##
##  You should have received a copy of the GNU Library General Public License
##  along with PLplot; if not, write to the Free Software
##  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
##


##
## This example designed just for devices (e.g., psttfc and the
## cairo-related devices) that use the pango and fontconfig libraries. The
## best choice of glyph is selected by fontconfig and automatically rendered
## by pango in way that is sensitive to complex text layout (CTL) language
## issues for each unicode character in this example. Of course, you must
## have the appropriate TrueType fonts installed to have access to all the
## required glyphs.
##
## Translation instructions: The strings to be translated are given by
## x_label, y_label, alty_label, title_label, and line_label below.  The
## encoding used must be UTF-8.
##
## The following strings to be translated involve some scientific/mathematical
## jargon which is now discussed further to help translators.
##
## (1) dB is a decibel unit, see http://en.wikipedia.org/wiki/Decibel .
## (2) degrees is an angular measure, see
##     http://en.wikipedia.org/wiki/Degree_(angle) .
## (3) low-pass filter is one that transmits (passes) low frequencies.
## (4) pole is in the mathematical sense, see
##     http://en.wikipedia.org/wiki/Pole_(complex_analysis) .  "Single Pole"
##     means a particular mathematical transformation of the filter function has
##     a single pole, see
##     http://ccrma.stanford.edu/~jos/filters/Pole_Zero_Analysis_I.html .
##     Furthermore, a single-pole filter must have an inverse square decline
##     (or -20 db/decade). Since the filter plotted here does have that
##     characteristic, it must by definition be a single-pole filter, see also
##     http://www-k.ext.ti.com/SRVS/Data/ti/KnowledgeBases/analog/document/faqs/1p.htm
## (5) decade represents a factor of 10, see
##     http://en.wikipedia.org/wiki/Decade_(log_scale) .
##

1;

function ix26c

  x_label = [
	     "Frequency";
	     "Частота";
	     ];

  y_label = [
	     "Amplitude (dB)";
	     "Амплитуда (dB)";
	     ];

  alty_label = [
		"Phase shift (degrees)";
		"Фазовый сдвиг (градусы)";
		];

  title_label = [
		 "Single Pole Low-Pass Filter";
		 "Однополюсный Низко-Частотный Фильтр";
		 ];

  line_label = [
		"-20 dB/decade";
		"-20 dB/десяток";
		];

  ## Short rearranged versions of y_label and alty_label.
  legend_text = {
		 ["Amplitude"; "Phase shift"],
		 ["Амплитуда"; "Фазовый сдвиг"]
		 };


  ## Illustration of logarithmic axes, and redefinition of window.

  ## Parse and process command line arguments

  ##    (void) plparseopts(&argc, argv, PL_PARSE_FULL);

  ## Initialize plplot

  plinit;
  plfont(2);

  ## Make log plots using two different styles.

  for i = 1:size(x_label)(1);
    plot1(0, deblank(x_label(i,:)), deblank(y_label(i,:)), deblank(alty_label(i,:)), deblank(title_label(i,:)), deblank(line_label(i,:)), legend_text{i});
  endfor

  plend1;
endfunction

## Log-linear plot.

function plot1(plottype, x_label, y_label, alty_label, title_label, line_label, legend_text)
  global PL_LEGEND_LINE PL_LEGEND_SYMBOL PL_LEGEND_BACKGROUND PL_LEGEND_BOUNDING_BOX

  pladv(0);

  ## Set up data for log plot */

  f0 = 1.0;
  i=0:100;
  freql = -2.0 + i / 20.0;
  freq = 10.0 .^ freql;
  ampl = 20.0 * log10(1.0 ./ sqrt(1.0 + (freq ./ f0).^ 2.));
  phase = -(180.0 / pi) * atan(freq ./ f0);

  plvpor(0.15, 0.85, 0.1, 0.9);
  plwind(-2.0, 3.0, -80.0, 0.0);

  ## Try different axis and labelling styles. */

  plcol0(1);
  switch (plottype)

    case 0
      plbox("bclnst", 0.0, 0, "bnstv", 0.0, 0);

    case 1
      plbox("bcfghlnst", 0.0, 0, "bcghnstv", 0.0, 0);

  endswitch

  ## Plot ampl vs freq */

  plcol0(2);
  plline(freql', ampl');
  plcol0(2);
  plptex(1.6, -30.0, 1.0, -20.0, 0.5, line_label);

  ## Put labels on */

  plcol0(1);
  plmtex("b", 3.2, 0.5, 0.5, x_label);
  plmtex("t", 2.0, 0.5, 0.5, title_label);
  plcol0(2);
  plmtex("l", 5.0, 0.5, 0.5, y_label);

  ## For the gridless case, put phase vs freq on same plot */

  if (plottype == 0)
    plcol0(1);
    plwind(-2.0, 3.0, -100.0, 0.0);
    plbox("", 0.0, 0, "cmstv", 30.0, 3);
    plcol0(3);
    plline(freql', phase');
    plstring(freql', phase', "#(728)");
    plcol0(3);
    plmtex("r", 5.0, 0.5, 0.5, alty_label);
    nlegend = 2;
  else
    nlegend = 1
  endif
  ## Set up legend arrays with the correct size, type.  In octave this may
  ## be done by clearing the array (in case it was used before) and then
  ## writing the last element of the array with a placeholder of the right
  ## type.  Note that for strings the right placeholder length doesn't
  ## matter.  Octave keeps track of the longest string in each array and pads
  ## out all other members with blanks to that length if a subsequent string
  ## assignment for an array element exceeds that length.

  opt_array = 0;
  text_colors = 0;
  text = " ";
  box_colors = 0;
  box_patterns = 0;
  box_scales = 0.;
  box_line_widths = 0.;
  line_colors = 0;
  line_styles = 0;
  line_widths = 0.;
  symbol_colors = 0;
  symbol_scales = 0.;
  symbol_numbers = 0;
  symbols = " ";

  opt_array(nlegend,1) = 0;
  text_colors(nlegend,1) = 0;
  text(nlegend,1:length(" ")) = " ";
  box_colors(nlegend,1) = 0;
  box_patterns(nlegend,1) = 0;
  box_scales(nlegend,1) = 0.;
  box_line_widths(nlegend,1) = 0.;
  line_colors(nlegend,1) = 0;
  line_styles(nlegend,1) = 0;
  line_widths(nlegend,1) = 0.;
  symbol_colors(nlegend,1) = 0;
  symbol_scales(nlegend,1) = 0.;
  symbol_numbers(nlegend,1) = 0;
  symbols(nlegend,1:length(" ")) = " ";

  ## Only specify legend data that are required according to the
  ## value of opt_array for that entry.

  ## Data for first legend entry.
  opt_array(1) = PL_LEGEND_LINE;
  text_colors(1) = 2;
  text(1,1:length(legend_text(1,:))) = legend_text(1,:);
  line_colors(1) = 2;
  line_styles(1) = 1;
  line_widths(1) = 1.;

  ## Data for second legend entry.
  if(nlegend > 1)
    opt_array(2) = bitor(PL_LEGEND_LINE, PL_LEGEND_SYMBOL);
    text_colors(2) = 3;
    text(2,1:length(legend_text(1,:))) = legend_text(2,:);
    line_colors(2) = 3;
    line_styles(2) = 1;
    line_widths(2) = 1.;
    symbol_colors(2) = 3;
    symbol_scales(2) = 1.;
    symbol_numbers(2) = 4;
    symbols(2,1:length("#(728)")) = "#(728)";
  endif

  plscol0a( 15, 32, 32, 32, 0.70 );

  [legend_width, legend_height] = ...
      pllegend( bitor(PL_LEGEND_BACKGROUND, PL_LEGEND_BOUNDING_BOX), 0, 0.0, 0.0,
               0.1, 15, 1, 1, 0, 0, opt_array, 1.0, 1.0, 2.0,
               1., text_colors, text,
               box_colors, box_patterns, box_scales, box_line_widths,
               line_colors, line_styles, line_widths,
               symbol_colors, symbol_scales, symbol_numbers, symbols );

endfunction

ix26c
