## Simple demo of a 2D line plot.
##
## Copyright (C) 2011  Alan W. Irwin
## Copyright (C) 2012  Andrew Ross
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
##

function x00c

    NSIZE = 101;

    xmin = 0.; xmax = 1.; ymin = 0.; ymax = 100.;

    ## Prepare data to be plotted.
    x = (0:NSIZE-1) / (NSIZE-1);
    y = ymax*x.*x;

    ## Parse and process command line arguments
    ## plparseopts( &argc, argv, PL_PARSE_FULL );

    ## Initialize plplot
    plinit();

    ## Create a labelled box to hold the plot.
    plenv( xmin, xmax, ymin, ymax, 0, 0 );
    pllab( "x", "y=100 x#u2#d", "Simple PLplot demo of a 2D line plot" );

    ## Plot the data that was prepared above.
    plline( x', y' );

    ## Close PLplot library
    plend1();

endfunction
