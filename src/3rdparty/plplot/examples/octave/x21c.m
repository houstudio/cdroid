##	Grid data demo
##
##  Copyright (C) 2004  Joao Cardoso
##  Copyright (C) 2006  Andrew Ross
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

1;

function cmap1_init

  i(1) = 0.0;		## left boundary
  i(2) = 1.0;		## right boundary

  h(1) = 240; ## blue -> green -> yellow ->
  h(2) = 0;   ## -> red

  l(1) = 0.6;
  l(2) = 0.6;

  s(1) = 0.8;
  s(2) = 0.8;

  plscmap1n(256);
  plscmap1l(0, i', h', l', s', zeros(2,1));
endfunction

function ix21c

  ## Global plplot constants used here
  global GRID_CSA GRID_DTLI GRID_NNLI GRID_NNI;
  global DRAW_LINEXY  MAG_COLOR  BASE_CONT;

  pts = 500;
  xp = 25;
  yp = 20;
  nl = 16;
  knn_order = 20;
  threshold = 1.001;
  wmin = -1e3;
  randn = 0;
  rosen = 0;


  title = ["Cubic Spline Approximation";
		   "Delaunay Linear Interpolation";
		   "Natural Neighbors Interpolation";
		   "KNN Inv. Distance Weighted";
		   "3NN Linear Interpolation";
		   "4NN Around Inv. Dist. Weighted"];

  opt = zeros(6,1);

  xm = -0.2; ym = -0.2;
  xM = 0.6; yM = 0.6;

  ##plMergeOpts(options, "x21c options", NULL);
  ##plparseopts(&argc, argv, PL_PARSE_FULL);

  opt(3) = wmin;
  opt(4) = knn_order;
  opt(5) = threshold;

  ## Initialize plplot

  plinit;

  cmap1_init;

  plseed(5489);

  [x, y, z] = create_data(pts, xm, xM, ym, yM, randn, rosen); ## the sampled data
  zmax = max(z);
  zmin = min(z);

  [xg, yg] = create_grid(xp, yp, xm, xM, ym, yM); ## grid the data at

  plcol0(1);
  plenv(xm, xM, ym, yM, 2, 0);
  plcol0(15);
  pllab("X", "Y", "The original data sampling");
  for i=1:pts
    plcol1( ( z(i) - zmin ) / ( zmax - zmin ) );
    plstring( x(i), y(i), "#(727)" );
  end
  pladv(0);

  plssub(3,2);

  for k=0:1
    pladv(0);
    for alg=1:6

      zg = plgriddata(x, y, z, xg, yg, alg, opt(alg));

      ## - CSA can generate NaNs (only interpolates?!).
      ## - DTLI and NNI can generate NaNs for points outside the convex hull
      ##      of the data points.
      ## - NNLI can generate NaNs if a sufficiently thick triangle is not found
      ##
      ## PLplot should be NaN/Inf aware, but changing it now is quite a job...
      ## so, instead of not plotting the NaN regions, a weighted average over
      ## the neighbors is done.
      ##

      if (alg == GRID_CSA || alg == GRID_DTLI || alg == GRID_NNLI || alg == GRID_NNI)
	for i=1:xp
	  for j=1:yp
	    ## average (IDW) over the 8 neighbors for NaN's
	    if isnan(zg(i,j))
	      zg(i,j) = 0.;
	      dist = 0.;

	      for ii=i-1:i+1
		for jj=j-1:j+1
		  if (ii >= 1 && jj >= 1 && ii<=xp && jj <=yp && !isnan(zg(ii,jj)))
		    if ((abs(ii-i) + abs(jj-j)) == 1)
		      d = 1.;
		    else
		      d = 1.4142;
		    endif
		    zg(i,j) += zg(ii,jj)/(d*d);
		    dist += d;
		  endif
		endfor
	      endfor
	      if (dist != 0.)
		zg(i,j) /= dist;
	      else
		zg(i,j) = zmin;
	      endif
	    endif
	  endfor
	endfor
      endif

      lzM = max(max(zg));
      lzm = min(min(zg));

      lzm = min(lzm, zmin)-0.01;
      lzM = max(lzM, zmax)+0.01;

      plcol0(1);
      pladv(alg);

      if (k == 0)

	i = (0:nl-1)';
	clev = lzm + (lzM-lzm)/(nl-1)*i;

	plenv0(xm, xM, ym, yM, 2, 0);
	plcol0(15);
	pllab("X", "Y", deblank(title(alg,:)));
	plshades(zg, xm, xM, ym, yM, clev, 1., 0, 1., 1);
	plcol0(2);
      else

	i=(0:nl-1)';
	clev = lzm + (lzM-lzm)/(nl-1)*i;

	plvpor(0.0, 1.0, 0.0, 0.9);
	plwind(-1.1, 0.75, -0.65, 1.20);
	##
	## For the comparition to be fair, all plots should have the
	## same z values, but to get the max/min of the data generated
	## by all algorithms would imply two passes. Keep it simple.
	##
	## plw3d(1., 1., 1., xm, xM, ym, yM, zmin, zmax, 30, -60);
	##

	plw3d(1., 1., 1., xm, xM, ym, yM, lzm, lzM, 30., -40.);
	plbox3("bntu", "X", 0.0, 0,
	       "bntu", "Y", 0.0, 0,
	       "bcdfntu", "Z", 0.5, 0);
	plcol0(15);
	pllab("", "", deblank(title(alg,:)));
	plot3dc(xg, yg, zg, bitor(DRAW_LINEXY,bitor(MAG_COLOR,BASE_CONT)), clev);
      endif
    endfor
  endfor

  plend1;

endfunction


function [x, y] = create_grid(px, py, xm, xM, ym, yM)

  i = (0:px-1)';
  x = xm + (xM-xm)*i/(px-1.0);

  i = (0:py-1)';
  y = ym + (yM-ym)*i/(py-1.0);

endfunction

function [x, y, z] = create_data(pts, xm, xM, ym, yM, randn, rosen)

  ## This would be a much more efficient way of generating an array of
  ## random numbers, but we stick with plrandd for compatibility between
  ## examples.
  ## x = rand(pts,1);
  ## y = rand(pts,1);
  x = zeros(pts,1);
  y = zeros(pts,1);
  for i=1:pts
    x(i) = (xM-xm)*plrandd();
    y(i) = (yM-ym)*plrandd();
  endfor

  if (!randn)
    x = x + xm;
    y = y + ym;
  else ## std=1, meaning that many points are outside the plot range
    x = sqrt(-2.*log(x)) * cos(2.*pi*y) + xm;
    y = sqrt(-2.*log(x)) * sin(2.*pi*y) + ym;
  endif
  if (!rosen)
    r = sqrt(x.*x + y.*y);
    z = exp(-r .* r) .* cos(2.0 * pi * r);
  else
    z = log((1. - x).^2 + 100. * (y - x.^2).^2);
  endif
endfunction


ix21c;
