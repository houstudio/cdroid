## Copyright (C) 2002-2003 Joao Cardoso.
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

function p17(fg)

  global pl_automatic_replot
  t = pl_automatic_replot;
  pl_automatic_replot = 0;

  if(!exist('imread'))
    warning('Example p17 requires imread from octave-forge');
    return;
  end

  title "Click and Drag button 1 to select";
  xlabel "Button 2 to restart and button 3 to finish";
  ylabel "";
  file_pgm = imread (file_in_loadpath ("Chloe.pgm"));
  [img, map]= gray2ind (file_pgm, 255);
  colormap(map);
  plimage (img);
  if (!nargin)
    [x1, y1, x2, y2] = plrb(1);
    % Prevent case where range is zero
    if (x1 == x2)
      x2 = x1+1;
    end
    if (y1 == y2)
      y2 = y1+1;
    end
    title "Chloe";
    xlabel "";
    plimage (img, x1, x2, y1, y2);
  endif

  pl_automatic_replot = t;

endfunction
