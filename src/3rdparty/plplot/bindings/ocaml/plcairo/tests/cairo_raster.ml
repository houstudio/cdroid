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

(* This is a Cairo-only example, meant as an illustration and test of the
   technique used in the Plcairo.rasterize function. *)

open Cairo

let () =
  let img =
    Cairo.image_surface_create
      Cairo.FORMAT_ARGB32 ~width:100 ~height:100
  in
  let outfile = open_out "test.ps" in
  let ps =
    Cairo_ps.surface_create_for_channel outfile
      ~width_in_points:1000.0 ~height_in_points:1000.0
  in
  let ct_img = Cairo.create img in
  let ct_ps = Cairo.create ps in
  (* Draw something on the image *)
  Cairo.rectangle ~x:10.0 ~y:10.0 ~width:80.0 ~height:80.0 ct_img;
  Cairo.fill ct_img;
  Cairo.stroke ct_img;

  (* Blit the image out to the Postscript. *)
  Cairo.set_source_surface ct_ps img 0.0 0.0;
  Cairo.paint ct_ps;
  Cairo.surface_finish ps;

  (* Save the files *)
  Cairo_png.surface_write_to_file img "test.png";
  close_out outfile;
