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

## [x1, y1, x2, y2] = plrb(clear)
##
## Interactively draws a rectangle using the mouse.
##
## Use mouse button-1 click/drag to grow or shrink the rectangle.
## Restart the selection with a button-2 click, and finish with button-3.
##
## If 'clear' is different from zero, the selection is cleared on exit.

function [x1, y1, x2, y2] = plrb(fg)

  global __pl
  __pl_init;

  [status, state, keysym, button, string, pX, pY, dX, dY, wX, wY, subwin] = plGetCursor;

  x1 = x2 = wX; y1 = y2 = wY;
  x = [x1; x2; x2; x1; x1]; y = [y1; y1; y2; y2; y1 ];

  hasxor = plxormod(1);

  if (! hasxor )
    warning("plrb: Device is not xor capable. Selection rectangle not drawn.");
  endif

  start = 1;
  done = 0;
  while(!done)
    [status, state, keysym, button, string, pX, pY, dX, dY, wX, wY, subwin] = plGetCursor;
    if (start)
      if (hasxor)
        plline(x,y);
        plflush;
      endif
      start = 0;
    elseif (state != 0)
      if (hasxor)
        plline(x,y);
        x2 = wX; y2 = wY;
        x = [x1; x2; x2; x1; x1]; y = [y1; y1; y2; y2; y1 ];
        plline(x,y);
      else
        x2 = wX; y2 = wY;
        x = [x1; x2; x2; x1; x1]; y = [y1; y1; y2; y2; y1 ];
      endif
    endif

    st = fix(state/255);

    if (st == 0 && button == 3) ## exit
      if (nargin != 0 && fg && hasxor)
	plline(x,y); plflush;
      endif
      plxormod(0);
      break;
    elseif (st == 0 && button == 2) ## restart
      if (hasxor)
        plline(x,y);
      endif
      [status, state, keysym, button, string, pX, pY, dX, dY, wX, wY, subwin] = plGetCursor;
      x1 = x2 = wX; y1 = y2 = wY;
      x = [x1; x2; x2; x1; x1]; y = [y1; y1; y2; y2; y1 ];
      start = 1;
    endif
  endwhile

endfunction
