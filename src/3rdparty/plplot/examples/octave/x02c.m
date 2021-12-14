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

#	Multiple window and color map 0 demo.

1;

function ix02c

  ## Initialize plplot */
  plinit();

  ## Parse and process command line arguments */

  ## (void) plparseopts(&argc, argv, PL_PARSE_FULL);

  demo1();
  demo2();

  plend1();

endfunction

function draw_windows(nw, cmap0_offset)
##--------------------------------------------------------------------------
## draw_windows
##
## Draws a set of numbered boxes with colors according to cmap0 entry.
##--------------------------------------------------------------------------

  plschr(0.0, 3.5);
  plfont(4);

  for i=0:nw-1
    plcol0(i+cmap0_offset);
    text=sprintf("%d", i);
    pladv(0);
    vmin = 0.1;
    vmax = 0.9;
    for j=0:2
      plwidth(j + 1);
      plvpor(vmin, vmax, vmin, vmax);
      plwind(0.0, 1.0, 0.0, 1.0);
      plbox("bc", 0.0, 0, "bc", 0.0, 0);
      vmin = vmin + 0.1;
      vmax = vmax - 0.1;
    endfor
    plwidth(1);
    plptex(0.5, 0.5, 1.0, 0.0, 0.5, text);
  endfor
endfunction


##--------------------------------------------------------------------------
## demo1
##
## Demonstrates multiple windows and default color map 0 palette.
##--------------------------------------------------------------------------
function demo1
  plbop();

  ## Divide screen into 16 regions
  plssub(4, 4);

  draw_windows( 16, 0 );

  pleop();

endfunction

##--------------------------------------------------------------------------
## demo2
##
##  Demonstrates multiple windows, user-modified color map 0 palette, and
## HLS -> RGB translation.
##--------------------------------------------------------------------------
function demo2
  ## Set up cmap0
  ## Use 100 custom colors in addition to base 16

  ## Min & max lightness values
  lmin = 0.15;
  lmax = 0.85;

  plbop;

  ## Divide screen into 100 regions
  plssub(10, 10);

  for i=0:99
  ## Bounds on HLS, from plhlsrgb() commentary --
  ##	hue		[0., 360.]	degrees
  ##	lightness	[0., 1.]	magnitude
  ##	saturation	[0., 1.]	magnitude
  ##

    ## Vary hue uniformly from left to right
    h = (360. / 10. ) * mod(i,10);
    ## Vary lightness uniformly from top to bottom, between min & max
    l = lmin + (lmax - lmin) * fix(i / 10) / 9.;
    ## Use max saturation
    s = 1.0;

    [r1, g1, b1] = plhlsrgb(h, l, s);

    r(i+17) = r1 * 255.001;
    g(i+17) = g1 * 255.001;
    b(i+17) = b1 * 255.001;
  endfor

  ## Load default cmap0 colors into our custom set
  for i=1:16
    [r(i),g(i),b(i)] = plgcol0(i-1);
  endfor

  ## Now set cmap0 all at once (faster, since fewer driver calls)
  plscmap0(r', g', b');

  draw_windows( 100, 16 );

  pleop;
endfunction


ix02c;
