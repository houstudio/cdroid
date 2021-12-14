## Copyright (C) 1998, 1999, 2000  Joao Cardoso
## Copyright (C) 2004  Rafael Laboissiere
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

## Generates polar plot, with 1-1 scaling.

function x03c

  ## Initialize plplot */
  ## Set orientation to portrait - note not all device drivers
  ## support this, in particular most interactive drivers do not */
  plsori(1);

  plinit;

  dtr = pi / 180.0;

  i=0:360;
  x0 = cos(dtr * i);
  y0 = sin(dtr * i);

  ## Parse and process command line arguments */

  ##    (void) plparseopts(&argc, argv, PL_PARSE_FULL);

  ## Set up viewport and window, but do not draw box */

  plenv(-1.3, 1.3, -1.3, 1.3, 1, -2);
  ## Draw circles for polar grid
    for i=1:10
      plarc(0.0, 0.0, 0.1 * i, 0.1 * i, 0.0, 360.0, 0.0, 0);
    endfor


  plcol0(2);
  for i=0:11
    theta = 30.0 * i;
    dx = cos(dtr * theta);
    dy = sin(dtr * theta);

    ## Draw radial spokes for polar grid */

    pljoin(0.0, 0.0, dx, dy);
    text=sprintf("%d", theta);

    ## Write labels for angle */

    if (theta < 9.99)
      offset = 0.45;
    elseif (theta < 99.9)
      offset = 0.30;
    else
      offset = 0.15;
    endif
    if (dx >= -0.00001)
      plptex(dx, dy, dx, dy, -offset, text);
    else
      plptex(dx, dy, -dx, -dy, 1.+offset, text);
    endif
  endfor

  ## Draw the graph */

  i=0:360;
  r = sin(dtr * (5 * i));
  x = x0 .* r;
  y = y0 .* r;

  plcol0(3);
  plline(x', y');

  plcol0(4);
  plmtex("t", 2.0, 0.5, 0.5, "#frPLplot Example 3 - r(#gh)=sin 5#gh");

  ## Close the plot at end */
  plend1();
endfunction

