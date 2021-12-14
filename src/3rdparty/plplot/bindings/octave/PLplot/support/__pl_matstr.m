## Copyright (C) 1998-2003  Joao Cardoso.
## Copyright (C) 2004  Rafael Laboissiere
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

## out = __pl_matstr(mat, str, n)
##
## set mat(n,:) = str, no matter the size of mat or str.

function out = __pl_matstr(mat, str, n)

  ## Octave damned string arrays are defective!
  if (ischar(str) && ischar(mat) && rows(str) == 1)
    if (isempty(mat))
      out(n,:) = str;
    else
      out = [mat; str];
      [nn sz] = size(out); ## add str at the end
      if (n < nn)
	out(n,:) = out(nn,:);
	out(nn,:) = "";
      elseif (n > nn)
	out(n+1,:) = out(nn,:);
	out(nn,:) = "";
      endif
    endif
    old_dofi = warning("query","Octave:fortran-indexing");
    warning("off","Octave:fortran-indexing");
    # double is the recommended replacement for toascii
    out(double (out) == 0) = " ";
    warning(old_dofi.state,"Octave:fortran-indexing");

  else
    help __pl_matstr
  endif

endfunction
