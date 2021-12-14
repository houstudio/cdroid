## Copyright (C) 1998, 1999, 2000  Joao Cardoso
## Copyright (C) 2004  Rafael Laboissiere
## Copyright (C) 2008 Andrew Ross
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

## Does a variety of shade plots.

1;

function ix15c

  XPTS=    35;		## Data points in x */
  YPTS=    46;		## Datat points in y */
  global	tr = [2/(XPTS-1); 0.0; -1.0; 0.0; 2/(YPTS-1); -1.0];

  ## Parse and process command line arguments */

  ## (void) plparseopts(&argc, argv, PL_PARSE_FULL);

  ## Set up color map 0 */

  ## plscmap0n(3);

  ## Set up color map 1 */

  cmap1_init2();

  ## Initialize plplot */
  plinit();

  ## Set up data array */

  for i = 0:XPTS-1;
      xx = (i - fix(XPTS / 2)) / fix(XPTS / 2);
    for j = 0:YPTS-1;
      yy = (j - fix(YPTS / 2)) / fix(YPTS / 2) - 1.0;
      z(i+1,j+1) = xx*xx - yy*yy + (xx - yy)/(xx*xx+yy*yy + 0.1);
    endfor
  endfor

  zmin=min(min(z));
  zmax=max(max(z));

  plot1(z,zmin,zmax);
  plot2(z,zmin,zmax);
  plot3();

  plend1();

endfunction

## Initializes color map 1 in HLS space.

function cmap1_init1

  i(1) = 0;		### left boundary */
  i(2) = 0.45;	### just before center */
  i(3) = 0.55;	### just after center */
  i(4) = 1;		### right boundary */

  h(1) = 260;		### hue -- low: blue-violet */
  h(2) = 260;		### only change as we go over vertex */
  h(3) = 20;		### hue -- high: red */
  h(4) = 20;		### keep fixed */

				#if 1
  l(1) = 0.5;		### lightness -- low */
  l(2) = 0.0;		### lightness -- center */
  l(3) = 0.0;		### lightness -- center */
  l(4) = 0.5;		### lightness -- high */
				#else
  plscolbg(255,255,255);
  l(1) = 0.5;		### lightness -- low */
  l(2) = 1.0;		### lightness -- center */
  l(3) = 1.0;		### lightness -- center */
  l(4) = 0.5;		### lightness -- high */
				#endif
  s(1) = 1;		### maximum saturation */
  s(2) = 1;		### maximum saturation */
  s(3) = 1;		### maximum saturation */
  s(4) = 1;		### maximum saturation */

  plscmap1l(0, i', h', l', s', zeros(4,1));
endfunction

### Initializes color map 1 in HLS space.

function cmap1_init2()


  i(1) = 0;		### left boundary */
  i(2) = 0.45;	### just before center */
  i(3) = 0.55;	### just after center */
  i(4) = 1;		### right boundary */

  h(1) = 260;		### hue -- low: blue-violet */
  h(2) = 260;		### only change as we go over vertex */
  h(3) = 20;		### hue -- high: red */
  h(4) = 20;		### keep fixed */

  if 1
    l(1) = 0.6;		### lightness -- low */
    l(2) = 0.0;		### lightness -- center */
    l(3) = 0.0;		### lightness -- center */
    l(4) = 0.6;		### lightness -- high */
  else
    plscolbg(255,255,255);
    l(1) = 0.5;		### lightness -- low */
    l(2) = 1.0;		### lightness -- center */
    l(3) = 1.0;		### lightness -- center */
    l(4) = 0.5;		### lightness -- high */
  endif
  s(1) = 1;		### saturation -- low */
  s(2) = 0.5;		### saturation -- center */
  s(3) = 0.5;		### saturation -- center */
  s(4) = 1;		### saturation -- high */

  plscmap1l(0, i', h', l', s', [ 0 0 0 0]');
endfunction

### Illustrates a single shaded region.

function plot1(z,zmin,zmax)
  global tr
  sh_cmap = 0;
  min_color = 0; min_width = 0.; max_color = 0; max_width = 0.;

  pladv(0);
  plvpor(0.1, 0.9, 0.1, 0.9);
  plwind(-1.0, 1.0, -1.0, 1.0);

### Plot using identity transform */

  shade_min = zmin + (zmax-zmin)*0.4;
  shade_max = zmin + (zmax-zmin)*0.6;
  sh_color = 7;
  sh_width = 2.;
  min_color = 9;
  max_color = 2;
  min_width = 2.;
  max_width = 2.;

  plpsty(8);
  plshade(z, 0, -1., 1., -1., 1.,
	  shade_min, shade_max,
	  sh_cmap, sh_color, sh_width,
	  min_color, min_width, max_color, max_width, 1, tr);

  plcol0(1);
  plbox("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
  plcol0(2);
  pllab("distance", "altitude", "Bogon flux");
endfunction

### Illustrates multiple adjacent shaded regions, using different fill

function plot2(z,zmin,zmax)
  global tr
  sh_cmap = 0;
  min_color = 0; min_width = 0.; max_color = 0; max_width = 0.;

  nlin = [1, 1, 1, 1, 1, 2, 2, 2, 2, 2];
  inc = [450, 0; -450, 0; 0, 0; 900, 0; 300, 0;
	 450,-450; 0, 900; 0, 450; 450, -450; 0, 900];
  del = [2000, 2000; 2000, 2000; 2000, 2000; 2000, 2000; 2000, 2000;
	 2000, 2000; 2000, 2000; 2000, 2000; 4000, 4000; 4000, 2000 ];

  pladv(0);
  plvpor(0.1, 0.9, 0.1, 0.9);
  plwind(-1.0, 1.0, -1.0, 1.0);

  ## Plot using identity transform */

  for i = 0:9
    shade_min = zmin + (zmax - zmin) * i / 10.0;
    shade_max = zmin + (zmax - zmin) * (i +1) / 10.0;
    sh_color = i+6;
    sh_width = 2.;
    n = nlin(i+1);
    plpat(inc(i+1,1:n)',del(i+1,1:n)');

    plshade(z, 0, -1., 1., -1., 1.,
	    shade_min, shade_max,
	    sh_cmap, sh_color, sh_width,
	    min_color, min_width, max_color, max_width,1,tr);
  endfor

  plcol0(1);
  plbox("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
  plcol0(2);
  pllab("distance", "altitude", "Bogon flux");
endfunction

## Illustrates shaded regions in 3d, using a different fill pattern for
## each region.
function plot3

    xx = [ -1.0, 1.0, 1.0, -1.0, -1.0;
	   -1.0, 1.0, 1.0, -1.0, -1.0 ];
    yy = [ 1.0, 1.0, 0.0, 0.0, 1.0;
	  -1.0, -1.0, 0.0, 0.0, -1.0 ];
    zz = [ 0.0, 0.0, 1.0, 1.0, 0.0;
	  0.0, 0.0, 1.0, 1.0, 0.0 ];

    pladv(0);
    plvpor(0.1, 0.9, 0.1, 0.9);
    plwind(-1.0, 1.0, -1.0, 1.0);
    plw3d(1., 1., 1., -1.0, 1.0, -1.0, 1.0, 0.0, 1.5, 30, -40);

    ## Plot using identity transform

    plcol0(1);
    plbox3("bntu", "X", 0.0, 0, "bntu", "Y", 0.0, 0, "bcdfntu", "Z", 0.5, 0);
    plcol0(2);
    pllab("","","3-d polygon filling");

    plcol0(3);
    plpsty(1);
    plline3(xx(1,:)', yy(1,:)', zz(1,:)');
    plfill3(xx(1,1:4)', yy(1,1:4)', zz(1,1:4)');
    plpsty(2);
    plline3(xx(2,:)', yy(2,:)', zz(2,:)');
    plfill3(xx(2,1:4)', yy(2,1:4)', zz(2,1:4)');

  endfunction


ix15c
