(*
Copyright 2008, 2009  Hezekiah M. Carty

This file is part of PLplot.

PLplot is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

PLplot is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with PLplot.  If not, see <http://www.gnu.org/licenses/>.
*)

(* Test for multiple plot pages. *)

open Plplot

let () =
  (* Initialize PLplot with the extcairo device and create a new plot stream. *)
  let plcairo =
    Plcairo.init_cairo ~clear:true ~width:400 ~height:400 ~filename:"test.pdf"
      Plcairo.pdfcairo
  in
  (* Plcairo.init_cairo restores the current active plot stream, so we need to
     make the plcairo stream active in order to plot on it. *)
  Plcairo.make_active plcairo;
  plenv 0.0 1.0 0.0 1.0 1 0;
  pljoin 0.0 0.0 1.0 1.0;
  (* The extcairo device does not automatically advance pages.  This must be
     done "by hand".  The following function call is equivalent to:
  Cairo.show_page plcairo.context;
  *)
  Plcairo.new_page plcairo;
  plenv 0.0 1.0 0.0 1.0 1 0;
  pljoin 0.0 1.0 1.0 0.0;
  plend ();
  (* Save the [plcairo] plot to its associated filename ("test.pdf"). *)
  Plcairo.save_to_file plcairo;
  ()
