(*
	Demo of multiple stream/window capability (requires Tk or Tcl-DP).

	Maurice LeBrun
	IFS, University of Texas at Austin

   Copyright (C) 2004  Alan W. Irwin
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

open Printf
open Plplot

let pi = atan 1.0 *. 4.0

let space0 = [||]
let mark0 = [||]
let space1 = [|1500|]
let mark1 = [|1500|]

let plot1 xscale yscale xoff yoff =
  let x =
    Array.init 60 (fun i -> xoff +. xscale *. float_of_int (i + 1) /. 60.0)
  in
  let y = Array.init 60 (fun i -> yoff +. yscale *. x.(i)**2.0) in

  let xmin = x.(0) in
  let xmax = x.(59) in
  let ymin = y.(0) in
  let ymax = y.(59) in

  let xs = Array.init 6 (fun i -> x.(i * 10 + 3)) in
  let ys = Array.init 6 (fun i -> y.(i * 10 + 3)) in

  (* Set up the viewport and window using PLENV. The range in X is *)
  (* 0.0 to 6.0, and the range in Y is 0.0 to 30.0. The axes are *)
  (* scaled separately (just = 0), and we just draw a labelled *)
  (* box (axis = 0). *)

  plcol0 1;
  plenv xmin xmax ymin ymax 0 0;
  plcol0 6;
  pllab "(x)" "(y)" "#frPLplot Example 1 - y=x#u2";

  (* Plot the data points *)
  plcol0 9;
  plpoin xs ys 9;

  (* Draw the line through the data *)
  plcol0 4;
  plline x y;
  plflush ();
  ()

let plot2 () =
  (* Set up the viewport and window using PLENV. The range in X is -2.0 to
     10.0, and the range in Y is -0.4 to 2.0. The axes are scaled separately
     (just = 0), and we draw a box with axes (axis = 1). *)
  plcol0 1;
  plenv (-2.0) 10.0 (-0.4) 1.2 0 1;
  plcol0 2;
  pllab "(x)" "sin(x)/x" "#frPLplot Example 1 - Sinc Function";

  (* Fill up the arrays *)
  let x = Array.init 100 (fun i -> (float_of_int i -. 19.0) /. 6.0) in
  let y =
    Array.map (fun xi -> if xi <> 0.0 then sin xi /. xi else 1.0) x
  in

  (* Draw the line *)
  plcol0 3;
  plline x y;
  plflush ();
  ()

let plot3 () =
  (* For the final graph we wish to override the default tick intervals, and
     so do not use PLENV *)
  pladv 0;

  (* Use standard viewport, and define X range from 0 to 360 degrees, Y range
     from -1.2 to 1.2. *)
  plvsta ();
  plwind 0.0 360.0 (-1.2) 1.2;

  (* Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y. *)
  plcol0 1;
  plbox "bcnst" 60.0 2 "bcnstv" 0.2 2;

  (* Superimpose a dashed line grid, with 1.5 mm marks and spaces. *)
  plstyl mark1 space1;
  plcol0 2;
  plbox "g" 30.0 0 "g" 0.2 0;
  plstyl mark0 space0;

  plcol0 3;
  pllab "Angle (degrees)" "sine" "#frPLplot Example 1 - Sine function";

  let x = Array.init 101 (fun i -> 3.6 *. float_of_int i) in
  let y = Array.map (fun xi -> sin (xi *. pi /. 180.0)) x in

  plcol0 4;
  plline x y;
  plflush ();
  ()

let plot4 () =
  let dtr d = float_of_int d *. pi /. 180.0 in
  let x0 = Array.init 361 (fun i -> cos (dtr i)) in
  let y0 = Array.init 361 (fun i -> sin (dtr i)) in

  (* Set up viewport and window, but do not draw box *)
  plenv (-1.3) 1.3 (-1.3) 1.3 1 (-2);
  for i = 1 to 10 do
    let x = Array.map (fun xj -> 0.1 *. float_of_int i *. xj) x0 in
    let y = Array.map (fun yj -> 0.1 *. float_of_int i *. yj) y0 in

    (* Draw circles for polar grid *)
    plline x y;
  done;

  plcol0 2;
  for i = 0 to 11 do
    let theta = 30 * i in
    let	dx = cos (dtr theta) in
    let	dy = sin (dtr theta) in

    (* Draw radial spokes for polar grid *)
    pljoin 0.0 0.0 dx dy;
    let text = sprintf "%d" theta in

    (* Write labels for angle *)

    (* Slightly off zero to avoid floating point logic flips at 90 and 270 deg. *)
    if dx >= -0.00001 then
      plptex dx dy dx dy (-0.15) text
    else
      plptex dx dy (-.dx) (-.dy) 1.15 text;
  done;

  (* Draw the graph *)
  let rs = Array.init 361 (fun i -> sin (dtr (5 * i))) in
  let x = Array.mapi (fun i xi -> xi *. rs.(i)) x0 in
  let y = Array.mapi (fun i yi -> yi *. rs.(i)) y0 in
  plcol0 3;
  plline x y;

  plcol0 4;
  plmtex "t" 2.0 0.5 0.5 "#frPLplot Example 3 - r(#gh)=sin 5#gh";
  plflush ();
  ()

(* Demonstration of contour plotting *)
let xpts = 35
let ypts = 46
let xspa = 2.0 /. float_of_int (xpts - 1)
let yspa = 2.0 /. float_of_int (ypts - 1)

let tr = [|xspa; 0.0; -1.0; 0.0; yspa; -1.0|]

let mypltr x y =
  tr.(0) *. x +. tr.(1) *. y +. tr.(2),
  tr.(3) *. x +. tr.(4) *. y +. tr.(5)

let clevel = [|-1.0; -0.8; -0.6; -0.4; -0.2; 0.0; 0.2; 0.4; 0.6; 0.8; 1.0|]

let plot5 () =
  let mark = [|1500|] in
  let space = [|1500|] in

  (* Set up function arrays *)
  let z = Array.make_matrix xpts ypts 0.0 in
  let w = Array.make_matrix xpts ypts 0.0 in

  for i = 0 to xpts - 1 do
    let xx = float_of_int (i - (xpts / 2)) /. float_of_int (xpts / 2) in
    for j = 0 to ypts - 1 do
      let yy =
        float_of_int (j - (ypts / 2)) /. float_of_int (ypts / 2) -. 1.0
      in
      z.(i).(j) <- xx *. xx -. yy *. yy;
      w.(i).(j) <- 2.0 *. xx *. yy;
    done
  done;

  plenv (-1.0) 1.0 (-1.0) 1.0 0 0;
  plcol0 2;
  plset_pltr mypltr;
  plcont z 1 xpts 1 ypts clevel;
  plstyl mark space;
  plcol0 3;
  plcont w 1 xpts 1 ypts clevel;
  plcol0 1;
  pllab "X Coordinate" "Y Coordinate" "Streamlines of flow";
  plflush ();
  ()

(*--------------------------------------------------------------------------*\
 * Plots several simple functions from other example programs.
 *
 * This version sends the output of the first 4 plots (one page) to two
 * independent streams.
\*--------------------------------------------------------------------------*)
let () =
  let geometry_master = "500x410+100+200" in
  let geometry_slave  = "500x410+650+200" in

  (* plplot initialization *)
  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* If valid geometry specified on command line, use it for both streams. *)
  let xp0, yp0, xleng0, yleng0, xoff0, yoff0 = plgpage () in
  let valid_geometry = ( xleng0 > 0 && yleng0 > 0 ) in

  (* Set up first stream *)
  if valid_geometry then
    plspage xp0 yp0 xleng0 yleng0 xoff0 yoff0
  else
    plsetopt "geometry" geometry_master;

  plssub 2 2;
  plinit ();

  let driver = plgdev () in
  let fam, num, bmax = plgfam () in

  printf "Demo of multiple output streams via the %s driver.\n" driver;
  printf "Running with the second stream as slave to the first.\n";
  printf "\n";

  (* Start next stream *)
  plsstrm 1;

  if valid_geometry then
    plspage xp0 yp0 xleng0 yleng0 xoff0 yoff0
  else
    plsetopt "geometry" geometry_slave;

  (* Turn off pause to make this a slave (must follow master) *)
  plspause false;
  plsdev driver;
  plsfam fam num bmax;
  plsetopt "fflen" "2";
  plinit ();

  (* Set up the data & plot *)
  (* Original case *)
  plsstrm 0;

  let xscale = 6.0 in
  let yscale = 1.0 in
  let xoff = 0.0 in
  let yoff = 0.0 in
  plot1 xscale yscale xoff yoff;

  (* Set up the data & plot *)
  let xscale = 1.0 in
  let yscale = 1.0e+6 in
  plot1 xscale yscale xoff yoff;

  (* Set up the data & plot *)
  let xscale = 1.0 in
  let yscale = 1.e-6 in
  let digmax = 2 in
  plsyax digmax 0;
  plot1 xscale yscale xoff yoff;

  (* Set up the data & plot *)
  let xscale = 1.0 in
  let yscale = 0.0014 in
  let yoff = 0.0185 in
  let digmax = 5 in
  plsyax digmax 0;
  plot1 xscale yscale xoff yoff;

  (* To slave *)
  (* The pleop() ensures the eop indicator gets lit. *)
  plsstrm 1;
  plot4 ();
  pleop ();

  (* Back to master *)
  plsstrm 0;
  plot2 ();
  plot3 ();

  (* To slave *)
  plsstrm 1;
  plot5 ();
  pleop ();

  (* Back to master to wait for user to advance *)
  plsstrm 0;
  pleop ();

  (* Call plend to finish off. *)
  plend ();
  ()

