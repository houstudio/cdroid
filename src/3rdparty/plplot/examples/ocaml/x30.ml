(*
  Alpha color values demonstration.

  Copyright (C) 2008 Hazen Babcock
  Copyright (C) 2008, 2010 Hezekiah M. Carty


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

  This example will only really be interesting when used with devices that
  support or alpha (or transparency) values, such as the cairo device family.
*)

open Plplot

let red = [|0; 255; 0; 0|]
let green = [|0; 0; 255; 0|]
let blue = [|0; 0; 0; 255|]
let alpha = [|1.0; 1.0; 1.0; 1.0|]

let px = [|0.1; 0.5; 0.5; 0.1|]
let py = [|0.1; 0.1; 0.5; 0.5|]

let pos = [|0.0; 1.0|]
let rcoord = [|1.0; 1.0|]
let gcoord = [|0.0; 0.0|]
let bcoord = [|0.0; 0.0|]
let acoord = [|0.0; 1.0|]
let rev = [|false; false|]

let () =
  plparseopts Sys.argv [PL_PARSE_FULL];

  plinit ();
  plscmap0n 4;
  plscmap0a red green blue alpha;

  (* Page 1:

     This is a series of red, green and blue rectangles overlaid
     on each other with gradually increasing transparency. *)

  (* Set up the window *)
  pladv 0;
  plvpor 0.0 1.0 0.0 1.0;
  plwind 0.0 1.0 0.0 1.0;
  plcol0 0;
  plbox "" 1.0 0 "" 1.0 0;

  (* Draw the boxes *)
  for i = 0 to 8 do
    let icol = i mod 3 + 1 in

    (* Get a color, change its transparency and
       set it as the current color. *)
    let r, g, b, a = plgcol0a icol in
    plscol0a icol r g b (1.0 -. float_of_int i /. 9.0);
    plcol0 icol;

    (* Draw the rectangle *)
    let translate a =
      Array.map (fun x -> x +. 0.5 /. 9.0 *. float_of_int i) a
    in
    plfill (translate px) (translate py);

  done;

  (* Page 2:

     This is a bunch of boxes colored red, green or blue with a single
     large (red) box of linearly varying transparency overlaid. The
     overlaid box is completely transparent at the bottom and completely
     opaque at the top. *)

  (* Set up the window *)
  pladv 0;
  plvpor 0.1 0.9 0.1 0.9;
  plwind 0.0 1.0 0.0 1.0;

  (* Draw the boxes. There are 25 of them drawn on a 5 x 5 grid. *)
  for i = 0 to 4 do
    (* Set box X position *)
    let px0 = 0.05 +. 0.2 *. float_of_int i in
    let px1 = px0 +. 0.1 in
    let px = [|px0; px1; px1; px0|] in

    (* We don't want the boxes to be transparent, so since we changed
       the colors transparencies in the first example we have to change
       the transparencies back to completely opaque. *)
    let icol = i mod 3 + 1 in
    let r, g, b, a = plgcol0a icol in
    plscol0a icol r g b 1.0;
    plcol0 icol;
    for j = 0 to 4 do
      (* Set box y position and draw the box. *)
      let py0 = 0.05 +. 0.2 *. float_of_int j in
      let py2 = py0 +. 0.1 in
      let py = [|py0; py0; py2; py2|] in
      plfill px py;
    done
  done;

  let px = [|0.0; 1.0; 1.0; 0.0|] in
  let py = [|0.0; 0.0; 1.0; 1.0|] in

  (* Create the color map with 128 colors and use plscmap1la to initialize
     the color values with a linear varying transparency (or alpha) *)
  plscmap1n 128;
  plscmap1la true pos rcoord gcoord bcoord acoord (Some rev);

  plgradient px py 90.0;

  plend ();
  ()

