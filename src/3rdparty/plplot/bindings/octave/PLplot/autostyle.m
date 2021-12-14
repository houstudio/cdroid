## Copyright (C) 1998-2003 Joao Cardoso.
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

## st = autostyle "on" | "off" | 1 | 0
##
## if set increases line style after each plot.
## return current status;
## Usefull for B&W plots

function st = autostyle(x)

  global __pl
  global pl_automatic_replot
  strm = __pl_init;

  if (nargin == 1)
    if (ischar (x))
      if (strcmp ("off", x))
        __pl.line_style(strm) = 0;
      elseif (strcmp ("on", x))
        __pl.line_style(strm) = 1;
      else
	help autostyle
      endif
    elseif (isscalar(x))
      if (x == 0)
	__pl.line_style(strm) = 0;
      elseif (x == 1)
	__pl.line_style(strm) = 1;
      endif
    else
      help autostyle
      return;
    endif
  elseif (nargin > 1)
    help autostyle
    return;
  endif

  st = __pl.line_style(strm);

  if (exist("pl_automatic_replot"))
    if (pl_automatic_replot)
      __pl_plotit;
    endif
  endif

endfunction

