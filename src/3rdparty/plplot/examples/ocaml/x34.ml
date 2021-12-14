(*
       Drawing mode setting and getting example.

  Copyright (C) 2011  Hezekiah M. Carty

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

(* Drawing modes to demonstrate *)
let drawing_modes = [|
  PL_DRAWMODE_DEFAULT;
  PL_DRAWMODE_REPLACE;
  PL_DRAWMODE_XOR;
|]

let drawing_mode_names = [|
  "Default drawing mode";
  "Replace destination with source";
  "XOR drawing";
|]

let initialize_colors () =
  plscol0 0 255 255 255;
  plscol0 1 0 0 0;
  plscol0 2 255 0 0;
  plscol0a 3 0 0 255 0.3;
  ()

let draw_page mode title =
  (* A triangle for the background *)
  let xs = [|0.0; 1.0; 0.0|] in
  let ys = [|0.0; 1.0; 1.0|] in

  (* A circle for the foreground *)
  let over_x = 0.5 in
  let over_y = 0.5 in
  let over_r = 0.4 in

  plcol0 1;

  (* Setup a plot window *)
  plenv 0.0 1.0 0.0 1.0 1 0;

  (* Show which mode we're using *)
  pllab "" "" title;

  (* Draw a background triangle using the default drawing mode *)
  plcol0 2;
  plsdrawmode PL_DRAWMODE_DEFAULT;
  plfill xs ys;

  (* Draw a circle in the given drawing mode *)
  plcol0 3;
  plsdrawmode mode;
  plarc over_x over_y over_r over_r 0.0 360.0 0.0 true;
  ()

(*--------------------------------------------------------------------------
  main
  --------------------------------------------------------------------------*)

let main =
  (* PLplot initialization *)

  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Initialize PLplot *)
  plinit ();

  (* Check for drawing mode support *)
  let mode = plgdrawmode () in

  if mode = PL_DRAWMODE_UNKNOWN then begin
    print_endline "WARNING: This driver does not support drawing mode getting/setting";
  end
  else begin
    (* Setup colors *)
    initialize_colors ();

    (* Draw one page per drawing mode *)
    Array.iteri (fun i mode -> draw_page mode drawing_mode_names.(i))
      drawing_modes;
  end;

  (* Clean up *)
  plend ();
  ()

