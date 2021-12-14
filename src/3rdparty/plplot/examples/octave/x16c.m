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

##	plshade demo, using color fill.

1;

## Set up transformation function

global nx = 35;	## Default number of data points in x
global ny = 46;	## Default number of data points in y
global	tr = [2/(nx-1); 0.0; -1.0; 0.0; 2/(ny-1); -1.0];

function  [tx ty] = mypltr( x, y)
  global tr
  tx = tr(1) * x + tr(2) * y + tr(3);
  ty = tr(4) * x + tr(5) * y + tr(6);
endfunction

function ix16c

  global tr; global nx; global ny;
  global PL_COLORBAR_LABEL_BOTTOM;
  global PL_COLORBAR_SHADE;
  global PL_COLORBAR_SHADE_LABEL;
  ## Fundamental settings.  See notes[] for more info.

  ns = 20;		## Default number of shade levels

  notes = [
	   "To get smoother color variation, increase ns, nx, and ny.  To get faster",
	   "response (especially on a serial link), decrease them.  A decent but quick",
	   "test results from ns around 5 and nx, ny around 25."];

  sh_cmap = 1;
  fill_width = 2.; cont_color = 0;  cont_width = 0.;

  num_axes = 1;
  n_axis_opts = num_axes;
  axis_opts = { "bcvtm" };
  values = zeros(num_axes,ns+1);
  axis_ticks = zeros(1,1);
  axis_subticks = zeros(1,1);

  num_labels = 1;
  label_opts = [ PL_COLORBAR_LABEL_BOTTOM ];
  labels = { "Magnitude" };


  ## Parse and process command line arguments

  ##    plMergeOpts(options, "x16c options", notes);
  ##    plparseopts(&argc, argv, PL_PARSE_FULL);

  ## Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display

  ## Load colour palettes
  plspal0("cmap0_black_on_white.pal");
  plspal1("cmap1_gray.pal",1);
  plscmap0n(3);

  ## Initialize plplot
  plinit();

  ## Set up data array

  for i = 0:nx-1
    x = (i - fix(nx / 2)) / fix(nx / 2);
    j = 0:ny-1;
    y = (j .- fix(ny / 2)) ./ fix(ny / 2) - 1.0;

    z(i+1,:) = - sin(7.*x) .* cos(7.*y) .+ x*x - y.*y;
    w(i+1,:) = - cos(7.*x) .* sin(7.*y) .+ 2 .* x .* y;
  endfor

  zmin=min(min(z));
  zmax=max(max(z));

  i = 0:ns-1;
  clevel = (zmin .+ (zmax - zmin) .* (i + 0.5) ./ ns)';
  i = 0:ns;
  shedge = zmin + (zmax - zmin) * i / ns;

  ## Set up coordinate grids

  for i = 0:nx-1
    j = 0:ny-1;
    [x y] = mypltr( i,  j);

    argx = x * pi/2;
    argy = y * pi/2;
    distort = 0.4;

    xg1(i+1,:) = x .+ distort .* cos(argx);
    yg1(i+1,:) = y .- distort .* cos(argy);

    xg2(i+1,:) = x .+ distort .* cos(argx) .* cos(argy);
    yg2(i+1,:) = y .- distort .* cos(argx) .* cos(argy);
  endfor

  xg1 = xg1(:,1);
  yg1 = yg1(1,:)';

  ## Plot using identity transform

  pladv(0);
  plvpor(0.1, 0.9, 0.1, 0.9);
  plwind(-1.0, 1.0, -1.0, 1.0);
  plpsty(0);

  plshades(z, -1., 1., -1., 1.,
	   shedge', fill_width,
	   cont_color, cont_width,
	   1);

  # Smaller text
  plschr( 0.0, 0.75 );
  # Small ticks on the vertical axis
  plsmaj( 0.0, 0.5 );
  plsmin( 0.0, 0.5 );

  num_values(1) = ns + 1;
  values(1,:) = shedge;

  [colorbar_width, colorbar_height] = plcolorbar(
            bitor(PL_COLORBAR_SHADE, PL_COLORBAR_SHADE_LABEL), 0,
            0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0,
            cont_color, cont_width,
            label_opts, labels, axis_opts,
            axis_ticks, axis_subticks,
            num_values, values );

  # Reset text and tick sizes
  plschr( 0.0, 1.0 );
  plsmaj( 0.0, 1.0 );
  plsmin( 0.0, 1.0 );

  plcol0(1);
  plbox("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
  plcol0(2);

  ## plcont(w, 1, nx, 1, ny, clevel, tr);

  pllab("distance", "altitude", "Bogon density");

  ## Plot using 1d coordinate transform

  ##  Load colour palettes
  plspal0("cmap0_black_on_white.pal");
  plspal1("cmap1_blue_yellow.pal",1);
  ## Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
  plscmap0n(3);

  pladv(0);
  plvpor(0.1, 0.9, 0.1, 0.9);
  plwind(-1.0, 1.0, -1.0, 1.0);
  plpsty(0);

  plshades1(z, -1., 1., -1., 1.,
	    shedge', fill_width,
	    cont_color, cont_width,
	    1, xg1, yg1);

  # Smaller text
  plschr( 0.0, 0.75 );
  # Small ticks on the vertical axis
  plsmaj( 0.0, 0.5 );
  plsmin( 0.0, 0.5 );

  num_values(1) = ns + 1;
  values(1,:) = shedge;

  [colorbar_width, colorbar_height] = plcolorbar(
            bitor(PL_COLORBAR_SHADE, PL_COLORBAR_SHADE_LABEL), 0,
            0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0,
            cont_color, cont_width,
            label_opts, labels, axis_opts,
            axis_ticks, axis_subticks,
            num_values, values );

  # Reset text and tick sizes
  plschr( 0.0, 1.0 );
  plsmaj( 0.0, 1.0 );
  plsmin( 0.0, 1.0 );

  plcol0(1);
  plbox("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
  plcol0(2);

  ## plcont1(w, 1, nx, 1, ny, clevel, xg1, yg1);

  pllab("distance", "altitude", "Bogon density");

  ## Plot using 2d coordinate transform

  ## Load colour palettes
  plspal0("cmap0_black_on_white.pal");
  plspal1("cmap1_blue_red.pal",1);
  ## Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
  plscmap0n(3);

  pladv(0);
  plvpor(0.1, 0.9, 0.1, 0.9);
  plwind(-1.0, 1.0, -1.0, 1.0);
  plpsty(0);

  plshades2(z, -1., 1., -1., 1.,
	    shedge', fill_width,
	    cont_color, cont_width,
	    0, xg2, yg2);

  # Smaller text
  plschr( 0.0, 0.75 );
  # Small ticks on the vertical axis
  plsmaj( 0.0, 0.5 );
  plsmin( 0.0, 0.5 );

  num_values(1) = ns + 1;
  values(1,:) = shedge;

  [colorbar_width, colorbar_height] = plcolorbar(
            bitor(PL_COLORBAR_SHADE, PL_COLORBAR_SHADE_LABEL), 0,
            0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0,
            cont_color, cont_width,
            label_opts, labels, axis_opts,
            axis_ticks, axis_subticks,
            num_values, values );

  # Reset text and tick sizes
  plschr( 0.0, 1.0 );
  plsmaj( 0.0, 1.0 );
  plsmin( 0.0, 1.0 );

  plcol0(1);
  plbox("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
  plcol0(2);
  plcont2(w, 1, nx, 1, ny, clevel, xg2, yg2);

  pllab("distance", "altitude", "Bogon density, with streamlines");

  ## Plot using 2d coordinate transform

  ## Load colour palettes
  plspal0("");
  plspal1("",1);
  ## Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
  plscmap0n(3);

  pladv(0);
  plvpor(0.1, 0.9, 0.1, 0.9);
  plwind(-1.0, 1.0, -1.0, 1.0);
  plpsty(0);

  plshades2(z, -1., 1., -1., 1.,
	    shedge', fill_width,
	    2, 3.,
	    0, xg2, yg2);

  # Smaller text
  plschr( 0.0, 0.75 );
  # Small ticks on the vertical axis
  plsmaj( 0.0, 0.5 );
  plsmin( 0.0, 0.5 );

  num_values(1) = ns + 1;
  values(1,:) = shedge;

  [colorbar_width, colorbar_height] = plcolorbar(
            bitor(PL_COLORBAR_SHADE, PL_COLORBAR_SHADE_LABEL), 0,
            0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0,
            2, 3,
            label_opts, labels, axis_opts,
            axis_ticks, axis_subticks,
            num_values, values );

  # Reset text and tick sizes
  plschr( 0.0, 1.0 );
  plsmaj( 0.0, 1.0 );
  plsmin( 0.0, 1.0 );

  plcol0(1);
  plbox("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
  plcol0(2);
  ## plcont2(w, 1, nx, 1, ny, clevel, xg2, yg2);

  pllab("distance", "altitude", "Bogon density");

  ## Note this exclusion API will probably change.

  ## Plot using 2d coordinate transform and exclusion
  if (0) ## exclusion not implemented
    ## Load colour palettes
    plspal0("cmap0_black_on_white.pal");
    plspal1("cmap1_gray.pal",1);
    ## Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
    plscmap0n(3);

    pladv(0);
    plvpor(0.1, 0.9, 0.1, 0.9);
    plwind(-1.0, 1.0, -1.0, 1.0);
    plpsty(0);

    plshades2(z, -1., 1., -1., 1.,
	      shedge', fill_width,
	      cont_color, cont_width,
	      0, xg2, yg2);

    plcol0(1);
    plbox("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
    pllab("distance", "altitude", "Bogon density with exclusion");
  endif

### Example with polar coordinates.

  PERIMETERPTS=100;
  ## Load colour palettes
  plspal0("cmap0_black_on_white.pal");
  plspal1("cmap1_gray.pal",1);
  ## Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display
  plscmap0n(3);

  pladv(0);
  plvpor( .1, .9, .1, .9 );
  plwind( -1., 1., -1., 1. );

  plpsty(0);

  ## Build new coordinate matrices.

  for i = 0:nx-1;
    r = i/ (nx-1);
    j = 0:ny-1;
    t = (2*pi/(ny-1))*j;
    xg2(i+1,:) = r.*cos(t);
    yg2(i+1,:) = r.*sin(t);
    z(i+1,:) = exp(-r.*r).*cos(5*pi*r).*cos(5*t);
  endfor

  ## Need a new shedge to go along with the new data set.

  zmin = min(min(z));
  zmax = max(max(z));

  i = 0:ns;
  shedge = zmin + (zmax - zmin) *  i /  ns;

  ##  Now we can shade the interior region.
  plshades2(z, -1, 1, -1, 1,
	    shedge', fill_width,
	    cont_color, cont_width,
	    0, xg2, yg2);

  # Smaller text
  plschr( 0.0, 0.75 );
  # Small ticks on the vertical axis
  plsmaj( 0.0, 0.5 );
  plsmin( 0.0, 0.5 );

  num_values(1) = ns + 1;
  values(1,:) = shedge;

  [colorbar_width, colorbar_height] = plcolorbar(
            bitor(PL_COLORBAR_SHADE, PL_COLORBAR_SHADE_LABEL), 0,
            0.005, 0.0, 0.0375, 0.875, 0, 1, 1, 0.0, 0.0,
            cont_color, cont_width,
            label_opts, labels, axis_opts,
            axis_ticks, axis_subticks,
            num_values, values );

  # Reset text and tick sizes
  plschr( 0.0, 1.0 );
  plsmaj( 0.0, 1.0 );
  plsmin( 0.0, 1.0 );

  ## Now we can draw the perimeter.  (If do before, shade stuff may overlap.)
  i = 0:PERIMETERPTS-1;
  t = (2*pi/(PERIMETERPTS-1))*i;
  px = cos(t);
  py = sin(t);

  plcol0(1);
  plline(px', py');

  ## And label the plot.

  plcol0(2);
  pllab( "", "",  "Tokamak Bogon Instability" );

  ## Clean up

  plend1();

endfunction

ix16c
