(*
  Polar plot demo.
   Copyright (C) 2008, 2009  Hezekiah M. Carty

  This file is part of PLplot.

  PLplot is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as published
  by the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  PLplot is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with PLplot; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*)

open Plplot

let pi = atan 1.0 *. 4.0

(*--------------------------------------------------------------------------*\
 * main
 *
 * Generates polar plot, with 1-1 scaling.
\*--------------------------------------------------------------------------*)

let main () =
  let dtr = pi /. 180.0 in
  let x0 = Array.init 361 (fun i -> cos (dtr *. float_of_int i)) in
  let y0 = Array.init 361 (fun i -> sin (dtr *. float_of_int i)) in

  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Set orientation to portrait - note not all device drivers
   * support this, in particular most interactive drivers do not *)
  plsori 1;

  (* Initialize plplot *)
  plinit ();

  (* Set up viewport and window, but do not draw box *)
  plenv (-1.3) 1.3 (-1.3) 1.3 1 (-2);
  for i = 1 to 10 do
    (* Draw circles for polar grid *)
    let r = 0.1 *. float_of_int i in
    plarc 0.0 0.0 r r 0.0 360.0 0.0 false;
  done;

  plcol0 2;
  for i = 0 to 11 do
    let theta = 30.0 *. float_of_int i in
    let dx = cos (dtr *. theta) in
    let dy = sin (dtr *. theta) in

    (* Draw radial spokes for polar grid *)
    pljoin 0.0 0.0 dx dy;
    let text = string_of_int (int_of_float (floor (theta +. 0.5))) in

    (* Write labels for angle *)
    let offset =
      if theta < 9.99 then
        0.45
      else if theta < 99.9 then
        0.30
      else
        0.15
    in

    (* Slightly off zero to avoid floating point logic flips at 90 and
       270 deg.*)
    if dx >= -0.00001 then
      plptex dx dy dx dy (-.offset) text
    else
      plptex dx dy (-.dx) (-.dy) (1.+.offset) text
  done;

  (* Draw the graph *)
  let x =
    Array.init 361 (fun i -> x0.(i) *. sin (dtr *. float_of_int (5 * i)))
  in
  let y =
    Array.init 361 (fun i -> y0.(i) *. sin (dtr *. float_of_int (5 * i)))
  in
  plcol0 3;
  plline x y;

  plcol0 4;
  plmtex "t" 2.0 0.5 0.5 "#frPLplot Example 3 - r(#gh)=sin 5#gh";

  (* Close the plot at end *)
  plend ();
  ()

let () = main ()
