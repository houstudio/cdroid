## Copyright (C) 2007 Andrew Ross.
##
## This program is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## This file is part of plplot_octave.

## usage: ishold
##
## Return 1 if the next line will be added to the current plot, or 0 if
## the plot device will be cleared before drawing the next line.
##


function a = ishold()

  global __pl

  a = __pl.hold(plgstrm+1);

endfunction
