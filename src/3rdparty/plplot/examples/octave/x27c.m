##  Drawing "spirograph" curves - epitrochoids, cycolids, roulettes
##
##  Copyright (C) 2007 Arjen Markus
##  Copyright (C) 2008 Andrew Ross
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

function ix27c

##--------------------------------------------------------------------------*\
## Generates two kinds of plots:
##   - construction of a cycloid (animated)
##   - series of epitrochoids and hypotrochoids
##--------------------------------------------------------------------------*/

  ## R, r, p, N
  ## R and r should be integers to give correct termination of the
  ## angle loop using gcd.
  ## N.B. N is just a place holder since it is no longer used
  ## (because we now have proper termination of the angle loop).
  params = [
	    21.0,  7.0,  7.0,  3.0;  ## Deltoid
	    21.0,  7.0, 10.0,  3.0;
	    21.0, -7.0, 10.0,  3.0;
	    20.0,  3.0,  7.0, 20.0;
	    20.0,  3.0, 10.0, 20.0;
	    20.0, -3.0, 10.0, 20.0;
	    20.0, 13.0,  7.0, 20.0;
	    20.0, 13.0, 20.0, 20.0;
	    20.0,-13.0, 20.0, 20.0];

  ## Parse and process command line arguments

  ## (void) plparseopts(&argc, argv, PL_PARSE_FULL);

  ## Initialize plplot
  plinit();

  ## Illustrate the construction of a cycloid

  cycloid();

  ## Loop over the various curves
  ## First an overview, then all curves one by one

  plssub(3, 3); ## Three by three window

  fill = 0;
  for i = 1:9
    pladv(0);
    plvpor( 0.0, 1.0, 0.0, 1.0 );
    spiro( params(i,:), fill );
  endfor

  pladv(0);
  plssub(1, 1); ## One window per curve

  for i=1:9
    pladv(0);
    plvpor( 0.0, 1.0, 0.0, 1.0 );
    spiro( params(i,:), fill );
  endfor

  ## Fill the curves.
  fill = 1;
  pladv( 0 );
  plssub( 1, 1 ); ## One window per curve
  for i=1:9
    pladv(0);
    plvpor( 0.0, 1.0, 0.0, 1.0 );
    spiro( params(i,:), fill );
  endfor

  ## Finally, an example to test out plarc capabilities
  arcs();


  ## Don't forget to call plend() to finish off!

  plend1();
end

##------------------------------------------------------------------------
## Calculate greatest common divisor following pseudo-code for the
## Euclidian algorithm at http://en.wikipedia.org/wiki/Euclidean_algorithm

function [value] = gcd (a,  b)
    a = floor(abs(a));
    b = floor(abs(b));
    while(b!=0)
        t = b;
        b = mod(a,b);
        a = t;
    endwhile
    value = a;
end

## ===============================================================

function cycloid()
    ## TODO
endfunction

## ===============================================================

function spiro(params, fill)

  NPNT=2000;

  ## Fill the coordinates

  ## Proper termination of the angle loop very near the beginning
  ## point, see
  ## http://mathforum.org/mathimages/index.php/Hypotrochoid.
  windings = floor(abs(params(2))/gcd(params(1), params(2)));
  steps    = floor(NPNT/windings);
  dphi     = 2.0*pi/steps;

  i = (0:windings*steps)';
  phi = i*dphi;
  phiw = (params(1)-params(2))/params(2)*phi;
  xcoord = (params(1)-params(2))*cos(phi) + params(3)*cos(phiw);
  ycoord = (params(1)-params(2))*sin(phi) - params(3)*sin(phiw);

  xmin = min(xcoord);
  xmax = max(xcoord);
  ymin = min(ycoord);
  ymax = max(ycoord);

  xrange_adjust = 0.15 * (xmax - xmin);
  xmin -= xrange_adjust;
  xmax += xrange_adjust;
  yrange_adjust = 0.15 * (ymax - ymin);
  ymin -= yrange_adjust;
  ymax += yrange_adjust;

  plwind( xmin, xmax, ymin, ymax );

  plcol0(1);
  if ( fill )
    plfill( xcoord, ycoord );
  else
    plline( xcoord, ycoord );
  endif

endfunction

function arcs
  NSEG = 8;

  theta = 0.0;
  dtheta = 360.0 / NSEG;
  plenv( -10.0, 10.0, -10.0, 10.0, 1, 0 );

  ## Plot segments of circle in different colors
  for i = 0:NSEG-1
    plcol0( mod(i,2) + 1 );
    plarc(0.0, 0.0, 8.0, 8.0, theta, theta + dtheta, 0.0, 0);
    theta = theta + dtheta;
  endfor

  ## Draw several filled ellipses inside the circle at different
  ## angles.
  a = 3.0;
  b = a * tan( (dtheta/180.0*pi)/2.0 );
  theta = dtheta/2.0;
  for i = 0:NSEG-1
    plcol0( 2 - mod(i,2) );
    plarc( a*cos(theta/180.0*pi), a*sin(theta/180.0*pi), a, b, 0.0, 360.0, theta, 1);
    theta = theta + dtheta;
  endfor

endfunction

ix27c
