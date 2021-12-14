## usage: toggle_plplot_use
##
## Use this script to activate/deactivate the default library for the
## PLplot plotting function (plot, mesh, etc.), as both the native
## gnuplot and the new octave-plplot libraries use similar names to the
## functions. toggle_plplot_use post-/pre-pends to the path for
## the PLplot functions in successive calls.

## File: toggle_plplot_use.m
## Author: Rafael Laboissiere <rafael@debian.org>
## Modified: Joao Cardoso
## Created on: Sun Oct 18 22:03:10 CEST 1998
## Last modified on: Fri Mar  7 09:38:38 CET 2003
##
## Copyright (C) 2003  Rafael Laboissiere
##
## PLplot is free software, distributable under the GPL. No
## warranties, use it at your own risk.  It  was originally written for
## inclusion in the Debian octave-plplot package.

1;

if ! exist ("use_plplot_state")
  global use_plplot_state
  use_plplot_state = "on";
  global default_shadow_warning
  default_shadow_warning = warning("query","Octave:shadowed-function");
  warning("off","Octave:shadowed-function");
else
  if strcmp (use_plplot_state, "on")
    use_plplot_state = "off";
    warning(default_shadow_warning.state,"Octave:shadowed-function");
  else
    use_plplot_state = "on";
    warning("off","Octave:shadowed-function");
  endif
endif

use_plplot_path = plplot_octave_path;
plplot_path_to_remove = char(strsplit(genpath(use_plplot_path),pathsep));
for use_plplot_i=1:size(plplot_path_to_remove)(1)
  if (findstr(path,deblank(plplot_path_to_remove(use_plplot_i,:))) > 0)
    rmpath(deblank(plplot_path_to_remove(use_plplot_i,:)));
  endif
endfor

if (strcmp (use_plplot_state, "on"))
  addpath(genpath(use_plplot_path));
  plplot_stub;
  if ! exist ("pl_automatic_replot")
    global pl_automatic_replot
    pl_automatic_replot = 1;
  endif
elseif (strcmp (use_plplot_state, "off"))
  addpath(genpath(use_plplot_path),"-end");
endif

use_plplot_lcd = pwd;
cd (use_plplot_path);
for  use_plplot_i = [ char(glob("*.m ")); char(glob("support/*.m ")) ]'
  clear (strrep (strrep (deblank(use_plplot_i'), ".m", ""), "support/", ""));
end
cd (use_plplot_lcd);

clear use_plplot_path use_plplot_lcd use_plplot_i plplot_path_to_remove

printf ("Use PLplot: %s\n", use_plplot_state);

