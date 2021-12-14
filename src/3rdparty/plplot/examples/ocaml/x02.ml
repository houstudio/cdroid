(*
        Multiple window and color map 0 demo.
   Copyright (C) 2008  Hezekiah M. Carty

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

(*--------------------------------------------------------------------------*\
 * draw_windows
 *
 * Draws a set of numbered boxes with colors according to cmap0 entry.
\*--------------------------------------------------------------------------*)

let draw_windows nw cmap0_offset =
  plschr 0.0 3.5;
  plfont 4;

  for i = 0 to nw - 1 do
    plcol0 (i + cmap0_offset);
    let text = string_of_int i in
    pladv 0;
    let vmin = 0.1 in
    let vmax = 0.9 in
    for j = 0 to 2 do
      plwidth (float_of_int (j + 1));
      let j' = float_of_int j *. 0.1 in
      plvpor (vmin +. j') (vmax -. j') (vmin +. j') (vmax -. j');
      plwind 0.0 1.0 0.0 1.0;
      plbox "bc" 0.0 0 "bc" 0.0 0;
    done;
    plwidth 1.0;
    plptex 0.5 0.5 1.0 0.0 0.5 text;
  done;
  ()

(*--------------------------------------------------------------------------*\
 * demo1
 *
 * Demonstrates multiple windows and default color map 0 palette.
\*--------------------------------------------------------------------------*)

let demo1 () =
  plbop ();
  (* Divide screen into 16 regions *)
  plssub 4 4;
  draw_windows 16 0;
  pleop ();
  ()

(*--------------------------------------------------------------------------*\
 * demo2
 *
 * Demonstrates multiple windows, user-modified color map 0 palette, and
 * HLS -> RGB translation.
\*--------------------------------------------------------------------------*)

let demo2 () =
  (* Set up cmap0 *)
  (* Use 100 custom colors in addition to base 16 *)
  let r = Array.make 116 0 in
  let g = Array.make 116 0 in
  let b = Array.make 116 0 in

  (* Min & max lightness values *)
  let lmin = 0.15 and lmax = 0.85 in

  plbop ();

  (* Divide screen into 100 regions *)
  plssub 10 10;

  for i = 0 to 99 do
    (* Bounds on HLS, from plhlsrgb() commentary --
     *  hue             [0., 360.]      degrees
     *  lightness       [0., 1.]        magnitude
     *  saturation      [0., 1.]        magnitude
     *)
    (* Vary hue uniformly from left to right *)
    let h = (360. /. 10. ) *. float_of_int ( i mod 10 ) in
    (* Vary lightness uniformly from top to bottom, between min & max *)
    let l = lmin +. (lmax -. lmin) *. float_of_int (i / 10) /. 9. in
    (* Use max saturation *)
    let s = 1.0 in

    let (r1, g1, b1) = plhlsrgb h l s in

    r.(i+16) <- int_of_float (r1 *. 255.001);
    g.(i+16) <- int_of_float (g1 *. 255.001);
    b.(i+16) <- int_of_float (b1 *. 255.001);
  done;

  (* Load default cmap0 colors into our custom set *)
  for i = 0 to 15 do
    let (r1, g1, b1) = plgcol0 i in
    r.(i) <- r1;
    g.(i) <- g1;
    b.(i) <- b1;
  done;

  (* Now set cmap0 all at once (faster, since fewer driver calls) *)
  plscmap0 r g b;
  draw_windows 100 16;
  pleop ();
  ()

(*--------------------------------------------------------------------------*\
 * main
 *
 * Demonstrates multiple windows and color map 0 palette, both default and
 * user-modified.
\*--------------------------------------------------------------------------*)

let main () =
  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Initialize plplot *)
  plinit ();
  (* Run demos *)
  demo1 ();
  demo2 ();
  plend ();
  ()

let () = main ()
