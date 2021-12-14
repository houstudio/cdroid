## Alpha color values demonstration.
##
## Copyright (C) 2008 Hazen Babcock
## Copyright (C) 2008 Andrew Ross
##
##
## This file is part of PLplot.
##
## PLplot is free software; you can redistribute it and/or modify
## it under the terms of the GNU Library General Public License as published
## by the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## PLplot is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Library General Public License for more details.
##
## You should have received a copy of the GNU Library General Public License
## along with PLplot; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
##

1;

##

function ix30c

  red   = [  0, 255,   0,   0];
  green = [  0,   0, 255,   0];
  blue  = [  0,   0,   0, 255];
  alpha = [1.0, 1.0, 1.0, 1.0];

  px = [0.1, 0.5, 0.5, 0.1];
  py = [0.1, 0.1, 0.5, 0.5];

  pos = [0.0, 1.0];
  rcoord = [1.0, 1.0];
  gcoord = [0.0, 0.0];
  bcoord = [0.0, 0.0];
  acoord = [0.0, 1.0];
  rev = [0, 0];


  ## plparseopts (&argc, argv, PL_PARSE_FULL);

  plinit ();
  plscmap0n (4);
  plscmap0a (red', green', blue', alpha');

  ##
  ## Page 1:
  ##
  ## This is a series of red, green and blue rectangles overlaid
  ## on each other with gradually increasing transparency.
  ##

  ## Set up the window
  pladv (0);
  plvpor (0.0, 1.0, 0.0, 1.0);
  plwind (0.0, 1.0, 0.0, 1.0);
  plcol0 (0);
  plbox ("", 1.0, 0, "", 1.0, 0);

  ## Draw the boxes
  for i=0:8
    icol = mod(i,3) + 1;

    ## Get a color, change its transparency and
    ## set it as the current color.
    [r,g,b,a] = plgcol0a (icol);
    plscol0a (icol, r, g, b, 1.0 - i/9.0);
    plcol0 (icol);

    ## Draw the rectangle
    plfill (px', py');

    ## Shift the rectangles coordinates
    px += 0.5/9.0;
    py += 0.5/9.0;
  endfor

  ##
  ## Page 2:
  ##
  ## This is a bunch of boxes colored red, green or blue with a single
  ## large (red) box of linearly varying transparency overlaid. The
  ## overlaid box is completely transparent at the bottom and completely
  ## opaque at the top.
  ##

  ## Set up the window
  pladv(0);
  plvpor(0.1, 0.9, 0.1, 0.9);
  plwind(0.0, 1.0, 0.0, 1.0);

  ## Draw the boxes. There are 25 of them drawn on a 5 x 5 grid.
  for i = 0:4
    ## Set box X position
    px(1) = 0.05 + 0.2 * i;
    px(2) = px(1) + 0.1;
    px(3) = px(2);
    px(4) = px(1);

    ## We don't want the boxes to be transparent, so since we changed
    ## the colors transparencies in the first example we have to change
    ## the transparencies back to completely opaque.
    icol = mod(i,3) + 1;
    [r,g,b,a] = plgcol0a (icol);
    plscol0a (icol, r, g, b, 1.0);
    plcol0 (icol);
    for j=0:4
      ## Set box y position and draw the box.
      py(1) = 0.05 + 0.2 * j;
      py(2) = py(1);
      py(3) = py(1) + 0.1;
      py(4) = py(3);
      plfill(px', py');
    endfor
  endfor

  ## The overlaid box is drawn using plshades with a color map that is
  ## the same color but has a linearly varying transparency.

  ## Create the color map with 128 colors and use plscmap1la to initialize
  ## the color values with a linear varying transparency (or alpha)
  plscmap1n(128);
  plscmap1la(1, pos', rcoord', gcoord', bcoord', acoord', rev');

  ## Use that cmap1 to create a transparent red gradient for the whole
  ## window.
  px(1) = 0.;
  px(2) = 1.;
  px(3) = 1.;
  px(4) = 0.;

  py(1) = 0.;
  py(2) = 0.;
  py(3) = 1.;
  py(4) = 1.;

  plgradient( px', py', 90. );

  plend1();
endfunction

ix30c;

