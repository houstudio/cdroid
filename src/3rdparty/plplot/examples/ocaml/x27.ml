(*
        Drawing "spirograph" curves - epitrochoids, cycolids, roulettes

   Copyright (C) 2007  Arjen Markus
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

let pi = atan 1.0 *. 4.0

let cycloid () = () (* TODO *)

(* Calculate greatest common divisor following pseudo-code for the
   Euclidian algorithm at http://en.wikipedia.org/wiki/Euclidean_algorithm *)
let rec gcd a b =
  let a = abs a in
  let b = abs b in
  match b with
  | 0 -> a
  | _ -> gcd b (a mod b)

let spiro params fill =
  let npnt = 2000 in
  let xcoord = Array.make (npnt + 1) 0.0 in
  let ycoord = Array.make (npnt + 1) 0.0 in

  (* Fill the coordinates *)

  (* Proper termination of the angle loop, very near the beginning
     point, see
     http://mathforum.org/mathimages/index.php/Hypotrochoid *)
  let windings =
    int_of_float (abs_float params.(1)) /
      gcd (int_of_float params.(0)) (int_of_float params.(1))
  in
  let steps = npnt / windings in
  let dphi = 2.0 *. pi /. float_of_int steps in

  (* This initialisation appears to be necessary, but I (AWI) don't understand why!  *)
  let xmin = ref 0.0 in
  let xmax = ref 0.0 in
  let ymin = ref 0.0 in
  let ymax = ref 0.0 in


  for i = 0 to windings * steps do
    let phi = float_of_int i *. dphi in
    let phiw = (params.(0) -. params.(1)) /. params.(1) *. phi in
    xcoord.(i) <- (params.(0) -. params.(1)) *. cos phi +. params.(2) *. cos phiw;
    ycoord.(i) <- (params.(0) -. params.(1)) *. sin phi -. params.(2) *. sin phiw;

    if i = 0 then (
      xmin := xcoord.(i);
      xmax := xcoord.(i);
      ymin := ycoord.(i);
      ymax := ycoord.(i);
    )
    else (
    );
    if !xmin > xcoord.(i) then xmin := xcoord.(i) else ();
    if !xmax < xcoord.(i) then xmax := xcoord.(i) else ();
    if !ymin > ycoord.(i) then ymin := ycoord.(i) else ();
    if !ymax < ycoord.(i) then ymax := ycoord.(i) else ();
  done;

  let xrange_adjust = 0.15 *. (!xmax -. !xmin) in
  let xmin = !xmin -. xrange_adjust in
  let xmax = !xmax +. xrange_adjust in
  let yrange_adjust = 0.15 *. (!ymax -. !ymin) in
  let ymin = !ymin -. yrange_adjust in
  let ymax = !ymax +. yrange_adjust in

  plwind xmin xmax ymin ymax;

  plcol0 1;
  let xcoord' = Array.sub xcoord 0 (1 + steps * windings) in
  let ycoord' = Array.sub ycoord 0 (1 + steps * windings) in
  if fill then (
    plfill xcoord' ycoord';
  )
  else (
    plline xcoord' ycoord';
  )

let deg_to_rad x = x *. pi /. 180.0

let arcs () =
  let nseg = 8 in

  let theta = ref 0.0 in
  let dtheta = 360.0 /. float_of_int nseg in
  plenv ~-.10.0 10.0 ~-.10.0 10.0 1 0;

  (* Plot segments of circle in different colors *)
  for i = 0 to nseg - 1 do
    plcol0 (i mod 2 + 1);
    plarc 0.0 0.0 8.0 8.0 !theta (!theta +. dtheta) 0.0 false;
    theta := !theta +. dtheta;
  done;

  (* Draw several filled ellipses inside the circle at different
     angles. *)
  let a = 3.0 in
  let b = a *. tan (deg_to_rad dtheta /. 2.0) in
  theta := dtheta /. 2.0;
  for i = 0 to nseg - 1 do
    plcol0 (2 - i mod 2);
    plarc (a *. cos (deg_to_rad !theta)) (a *. sin (deg_to_rad !theta)) a b 0.0 360.0 !theta true;
    theta := !theta +. dtheta;
  done;
  ()

(*--------------------------------------------------------------------------*\
 * Generates two kinds of plots:
 *   - construction of a cycloid (animated)
 *   - series of epitrochoids and hypotrochoids
\*--------------------------------------------------------------------------*)
let () =
  (* R, r, p, N *)
  let params =
    [|
      [|21.0;  7.0;  7.0;  3.0|];  (* Deltoid *)
      [|21.0;  7.0; 10.0;  3.0|];
      [|21.0; -7.0; 10.0;  3.0|];
      [|20.0;  3.0;  7.0; 20.0|];
      [|20.0;  3.0; 10.0; 20.0|];
      [|20.0; -3.0; 10.0; 20.0|];
      [|20.0; 13.0;  7.0; 20.0|];
      [|20.0; 13.0; 20.0; 20.0|];
      [|20.0;-13.0; 20.0; 20.0|];
    |]
  in

  (* plplot initialization *)
  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Initialize plplot *)
  plinit ();

  (* Illustrate the construction of a cycloid *)
  cycloid ();

  (* Loop over the various curves
     First an overview, then all curves one by one *)
  (* Three by three window *)
  plssub 3 3;

  for i = 0 to 8 do
    pladv 0;
    plvpor 0.0 1.0 0.0 1.0;
    spiro params.(i) false;
  done;

  pladv 0;
  (* One window per curve *)
  plssub 1 1;

  for i = 0 to 8 do
    pladv 0;
    plvpor 0.0 1.0 0.0 1.0;
    spiro params.(i) false;
  done;

  (* Fill the curves *)
  pladv 0;
  (* One window per curve *)
  plssub 1 1;

  for i = 0 to 8 do
    pladv 0;
    plvpor 0.0 1.0 0.0 1.0;
    spiro params.(i) true;
  done;

  (* Finally, an example to test out plarc capabilities *)
  arcs ();

  (* Don't forget to call plend() to finish off! *)
  plend ();
  ()

