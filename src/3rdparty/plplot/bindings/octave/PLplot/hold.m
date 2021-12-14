## Copyright (C) 2007 Andrew Ross
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

## usage: hold on | off
## Tell Octave to `hold' the current data on the plot when executing
## subsequent plotting commands.  This allows you to execute a series of
## plot commands and have all the lines end up on the same figure.  The
## default is for each new plot command to clear the plot device first.
##

function hold (arg)

  global __pl

  if (nargin ~= 1)
    usage("hold on|off");
  end

  if (strcmpi(arg,"on") == 1)
    __pl.hold(plgstrm+1) = 1;
  elseif (strcmpi(arg,"off") == 1)
    __pl.hold(plgstrm+1) = 0;
  else
    usage("hold on|off");
  end

endfunction
