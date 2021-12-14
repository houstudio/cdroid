## Copyright (C) 2004-2006 Andrew Ross
## Copyright (C) 2004 Rafael Laboissiere
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

# Simple line plot and multiple windows demo.

1;

global xmax;

function ix22c

  ## Parse and process command line arguments */

  ##    plparseopts(&argc, argv, PL_PARSE_FULL);

  ## Initialize plplot */
  plinit;

  ## Set up the data */
  ## Original case */

  circulation;

  narr = 6;
  fill = 0;

  arrow_x = [-0.5 0.5 0.3 0.5 0.3 0.5];
  arrow_y = [0.0 0.0 0.2 0.0 -0.2 0.0];
  arrow2_x = [-0.5 0.3 0.3 0.5 0.3 0.3];
  arrow2_y = [0.0 0.0   0.2 0.0 -0.2 0.0];

  ## Set arrow style using arrow_x and arrow_y then
  ## plot using these arrows.
  plsvect(arrow_x', arrow_y', fill);
  constriction(1);

  ## Set arrow style using arrow2_x and arrow2_y then
  ## plot using these filled arrows. */
  fill = 1;
  plsvect(arrow2_x', arrow2_y', fill);
  constriction(2);

  constriction2;

  plsvect([],[],0);

  potential;

  ## Don't forget to call plend1  to finish off! */

  plend1();

endfunction

function circulation
  nx = 20;
  ny = 20;

  dx = 1.0;
  dy = 1.0;

  xmin = -nx/2*dx;
  xmax = nx/2*dx;
  ymin = -ny/2*dy;
  ymax = ny/2*dy;

  xg = [xmin+dx/2:dx:xmax-dx/2]'*ones(1,ny);
  yg = ones(nx,1)*[ymin+dy/2:dy:ymax-dy/2];
  u = yg;
  v = -xg;

  ## Plot vectors with default arrows
  plenv(xmin, xmax, ymin, ymax, 0, 0);
  pllab("(x)", "(y)", "#frPLplot Example 22 - circulation");
  plcol0(2);
  plvect2(u,v,0.0,xg,yg);
  plcol0(1);

end

## Vector plot of flow through a constricted pipe
function constriction( astyle )
  nx = 20;
  ny = 20;

  dx = 1.0;
  dy = 1.0;

  xmin = -nx/2*dx;
  xmax = nx/2*dx;
  ymin = -ny/2*dy;
  ymax = ny/2*dy;

  Q = 2.0;
  xg = [xmin+dx/2:dx:xmax-dx/2]'*ones(1,ny);
  yg = ones(nx,1)*[ymin+dy/2:dy:ymax-dy/2];

  b = ymax/4.0.*(3-cos(pi*xg/xmax));
  dbdx = ymax/4.0.*sin(pi*xg/xmax)*pi/xmax.*yg./b;
  u = Q*ymax./b.*(abs(yg)<b);
  v = dbdx.*u.*(abs(yg)<b);

  plenv(xmin, xmax, ymin, ymax, 0, 0);
  title = sprintf( "#frPLplot Example 22 - constriction (arrow style %d)", astyle );

  pllab("(x)", "(y)", title );
  plcol0(2);
  plvect2(u,v,-1.0,xg,yg);
  plcol0(1);

end

##
## Global transform function for a constriction using data passed in
## This is the same transformation used in constriction.
##
function [xt, yt] = transform( x, y, data )
    global xmax;
    xt = x;
    yt = y / 4.0 * ( 3 - cos( pi * x / xmax ) );
end

## Vector plot of flow through a constricted pipe with
## a coordinate transformation
function constriction2()

  global xmax;

  nx = 20;
  ny = 20;
  nc = 11;
  nseg = 20;

  dx = 1.0;
  dy = 1.0;

  xmin = -nx/2*dx;
  xmax = nx/2*dx;
  ymin = -ny/2*dy;
  ymax = ny/2*dy;

  plstransform( @transform, [] );

  Q = 2.0;
  xg = [xmin+dx/2:dx:xmax-dx/2]'*ones(1,ny);
  yg = ones(nx,1)*[ymin+dy/2:dy:ymax-dy/2];

  b = ymax/4.0.*(3.0-cos(pi*xg/xmax));
  u = Q*ymax./b;
  v = zeros(nx,ny);

  clev = Q + (0:(nc-1))*Q/(nc-1);

  plenv(xmin, xmax, ymin, ymax, 0, 0);
  pllab("(x)", "(y)", "#frPLplot Example 22 - constriction with plstransform" );
  plcol0(2);
  plshades(u, xmin+dx/2,xmax-dx/2,ymin+dy/2,ymax-dy/2,clev',0.0,1,1.0,0);
  plvect2(u,v,-1.0,xg,yg);
  ## Plot edges using plpath (which accounts for coordinate transformation) rather than plline
  plpath( nseg, xmin, ymax, xmax, ymax );
  plpath( nseg, xmin, ymin, xmax, ymin );
  plcol0(1);

  plstransform( [], [] );

end

## Vector plot of the gradient of a shielded potential (see example 9)

function potential
  nper = 100;
  nlevel = 10;
  nr = 20;
  ntheta = 20;

  ## Potential inside a conducting cylinder (or sphere) by method of images.
  ## Charge 1 is placed at (d1, d1), with image charge at (d2, d2).
  ## Charge 2 is placed at (d1, -d1), with image charge at (d2, -d2).
  ## Also put in smoothing term at small distances.

  rmax = nr;

  eps = 2.;

  q1 = 1.;
  d1 = rmax/4.;

  q1i = - q1*rmax/d1;
  d1i = rmax^2/d1;

  q2 = -1.;
  d2 = rmax/4.;

  q2i = - q2*rmax/d2;
  d2i = rmax^2/d2;

  r = [0.5:1:nr-0.5];
  dtheta = 2*pi/(ntheta-1);
  theta = [dtheta/2:dtheta:2*pi+dtheta/2];
  x = r'*cos(theta);
  y = r'*sin(theta);
  div1 = sqrt((x-d1).^2 + (y-d1).^2 + eps^2);
  div1i = sqrt((x-d1i).^2 + (y-d1i).^2 + eps^2);
  div2 = sqrt((x-d2).^2 + (y+d2).^2 + eps^2);
  div2i = sqrt((x-d2i).^2 + (y+d2i).^2 + eps^2);
  z = q1*div1.^-1 + q1i*div1i.^-1 + q2*div2.^-1 + q2i*div2i.^-1;
  u = -q1*(x-d1)./div1.^3 - q1i*(x-d1i)./div1i.^3 - q2*(x-d2)./div2.^3 - q2i*(x-d2i)./div2i.^3;
  v = -q1*(y-d1)./div1.^3 - q1i*(y-d1i)./div1i.^3 - q2*(y+d2)./div2.^3 - q2i*(y+d2i)./div2i.^3;

  xmin = min(min(x));
  xmax = max(max(x));
  ymin = min(min(y));
  ymax = max(max(y));
  zmin = min(min(z));
  zmax = max(max(z));

  plenv(xmin, xmax, ymin, ymax, 0, 0);
  pllab("(x)", "(y)", "#frPLplot Example 22 - potential gradient vector plot");
  ## Plot contours of the potential
  dz = (zmax-zmin)/nlevel;
  clevel = [zmin+dz/2:dz:zmax-dz/2];
  plcol0(3);
  pllsty(2);
  plcont2(z,1,nr,1,ntheta,clevel',x,y);
  pllsty(1);
  plcol0(1);

  ## Plot the vectors of the gradient of the potential
  plcol0(2);
  plvect2(u,v,25.0,x,y);
  plcol0(1);

  ## Plot the perimeter of the cylinder
  theta = [0:2*pi/(nper-1):2*pi];
  px = rmax*cos(theta);
  py = rmax*sin(theta);
  plline(px',py');
end

ix22c


