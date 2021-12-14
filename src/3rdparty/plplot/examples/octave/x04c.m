## Copyright (C) 1998, 1999, 2000 Joao Cardoso.
##
## This program is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by the
## Free Software Foundation; either version 2 of the License, or (at your
## option) any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## This file is part of plplot_octave.
## It is based on the corresponding demo function of PLplot.

## Illustration of logarithmic axes, and redefinition of window.

## this file defines several functions:
1;

function ix04c

  ## Parse and process command line arguments */

  ## (void) plparseopts(&argc, argv, PL_PARSE_FULL);

  ## Initialize plplot */
  plinit();
  plfont(2);

  ## Make log plots using two different styles. */

  plot1(0);
  plot1(1);
  plend1();
endfunction

## Log-linear plot.

function plot1(type)
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
  switch (type)

    case 0
      plbox("bclnst", 0.0, 0, "bnstv", 0.0, 0);

    case 1
      plbox("bcfghlnst", 0.0, 0, "bcghnstv", 0.0, 0);

  endswitch

  ## Plot ampl vs freq */

  plcol0(2);
  plline(freql', ampl');
  plcol0(2);
  plptex(1.6, -30.0, 1.0, -20.0, 0.5, "-20 dB/decade");

  ## Put labels on */

  plcol0(1);
  plmtex("b", 3.2, 0.5, 0.5, "Frequency");
  plmtex("t", 2.0, 0.5, 0.5, "Single Pole Low-Pass Filter");
  plcol0(2);
  plmtex("l", 5.0, 0.5, 0.5, "Amplitude (dB)");

  ## For the gridless case, put phase vs freq on same plot */

  if (type == 0)
    plcol0(1);
    plwind(-2.0, 3.0, -100.0, 0.0);
    plbox("", 0.0, 0, "cmstv", 30.0, 3);
    plcol0(3);
    plline(freql', phase');
    plstring(freql', phase', "#(728)");
    plcol0(3);
    plmtex("r", 5.0, 0.5, 0.5, "Phase shift (degrees)");
    nlegend = 2;
  else
    nlegend = 1;
  endif

  ## Draw a legend.
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
  text(1,1:length("Amplitude")) = "Amplitude";
  line_colors(1) = 2;
  line_styles(1) = 1;
  line_widths(1) = 1.;

  ## Data for second legend entry.
  if(nlegend > 1)
    opt_array(2) = bitor(PL_LEGEND_LINE, PL_LEGEND_SYMBOL);
    text_colors(2) = 3;
    text(2,1:length("Phase shift")) = "Phase shift";
    line_colors(2) = 3;
    line_styles(2) = 1;
    line_widths(2) = 1.;
    symbol_colors(2) = 3;
    symbol_scales(2) = 1.;
    symbol_numbers(2) = 4;
    symbols(2,1:length("#(768)")) = "#(728)";
  endif

  plscol0a( 15, 32, 32, 32, 0.70 );

  [legend_width, legend_height] = ...
      pllegend( bitor(PL_LEGEND_BACKGROUND, PL_LEGEND_BOUNDING_BOX), 0, 0.0, 0.0,
               0.1, 15, 1, 1, 0, 0, opt_array, 1.0, 1.0, 2.0,
               1., text_colors, text,
               [], [], [], [],
               line_colors, line_styles, line_widths,
               symbol_colors, symbol_scales, symbol_numbers, symbols );

endfunction

ix04c
