(*
        3-d plot demo.

   Copyright (C) 2004  Alan W. Irwin
   Copyright (C) 2004  Rafael Laboissiere
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

let xpts = 35               (* Data points in x *)
let ypts = 45               (* Data points in y *)

let alt = [|60.0; 40.0|]
let az = [|30.0; -30.0|]

let title =
  [|
    "#frPLplot Example 8 - Alt=60, Az=30";
    "#frPLplot Example 8 - Alt=40, Az=-30";
  |]

(*--------------------------------------------------------------------------*\
 * cmap1_init1
 *
 * Initializes color map 1 in HLS space.
 * Basic grayscale variation from half-dark (which makes more interesting
 * looking plot compared to dark) to light.
 * An interesting variation on this:
 *      s[1] = 1.0
\*--------------------------------------------------------------------------*)

let cmap1_init gray =
  let i = [|0.0; 1.0|] in (* left and right boundaries *)

  let h, l, s =
    if gray then (
      [|0.0; 0.0|], (* hue -- low: red (arbitrary if s=0) *)
                    (* hue -- high: red (arbitrary if s=0) *)
      [|0.5; 1.0|], (* lightness -- low: half-dark *)
                    (* lightness -- high: light *)
      [|0.0; 0.0|]  (* minimum saturation *)
                    (* minimum saturation *)
    )
    else (
      [|240.0; 0.0|], (* blue -> green -> yellow -> *)
                      (* -> red *)
      [|0.6; 0.6|],
      [|0.8; 0.8|]
    )
  in

  plscmap1n 256;
  plscmap1l false i h l s None;
  ()

let levels = 10

(*--------------------------------------------------------------------------*\
 * Does a series of 3-d plots for a given data set, with different
 * viewing options in each plot.
\*--------------------------------------------------------------------------*)
let () =
  let nlevel = levels in
  let rosen = false in

  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Initialize plplot *)
  plinit ();

  (* Allocate data structures *)
  let z = Array.make_matrix xpts ypts 0.0 in
  (* initialize to large value to make sure index limits are working correctly *)
  let z_limited = Array.make_matrix xpts ypts 1.e30 in

  let x =
    Array.init xpts (
      fun i ->
        (float_of_int (i - (xpts / 2)) /. float_of_int (xpts / 2)) *.
          if rosen then
            1.5
          else
            1.0
    )
  in

  let y =
    Array.init ypts (
      fun i ->
        float_of_int (i - (ypts / 2)) /. float_of_int (ypts / 2) +.
          if rosen then
            0.5
          else
            0.0
    )
  in

  for i = 0 to xpts - 1 do
    let xx = x.(i) in
    for j = 0 to ypts - 1 do
      let yy = y.(j) in
      if rosen then (
        z.(i).(j) <- (1.0 -. xx)**2. +. 100. *. (yy -. xx**2.)**2.0;
        (* The log argument may be zero for just the right grid. *)
        if z.(i).(j) > 0.0 then
          z.(i).(j) <- log z.(i).(j)
        else
          z.(i).(j) <- -5. (* -MAXFLOAT would mess-up up the scale *)
      )
      else (
        let r = sqrt (xx *. xx +. yy *. yy) in
        z.(i).(j) <- exp (-.r *. r) *. cos (2.0 *. pi *. r)
      )
    done
  done;

  let indexxmin = 0 in
  let indexxmax = xpts in
  let indexymin = Array.make indexxmax 0 in
  let indexymax = Array.make indexxmax ypts in
  (* parameters of ellipse (in x, y index coordinates) that limits the data.
     x0, y0 correspond to the exact floating point centre of the index range. *)
  let x0 = 0.5 *. float_of_int ( xpts - 1 ) in
  let a = 0.9 *. x0 in
  let y0 = 0.5 *. float_of_int( ypts - 1 ) in
  let b = 0.7 *. y0 in

  for i = indexxmin to indexxmax - 1 do
    let square_root = sqrt ( 1. -. (min 1. ((( float_of_int i -. x0 ) /. a)**2.))) in
    (* Add 0.5 to find nearest integer and therefore preserve symmetry
       with regard to lower and upper bound of y range. *)
    indexymin.(i) <- max 0 (int_of_float (0.5 +. y0 -. b *. square_root));
    (* indexymax calculated with the convention that it is 1
       greater than highest valid index. *)
    indexymax.(i) <- min ypts (1 + int_of_float (0.5 +. y0 +. b *. square_root));
    for j = indexymin.(i) to indexymax.(i) - 1 do
      z_limited.(i).(j) <- z.(i).(j)
    done
  done;

  let zmax, zmin = plMinMax2dGrid z in
  let step = (zmax -. zmin) /. float_of_int (nlevel + 1) in
  let clevel =
    Array.init nlevel (fun i -> zmin +. step +. step *. float_of_int i)
  in

  pllightsource 1.0 1.0 1.0;

  for k = 0 to 1 do
    for ifshade = 0 to 4 do
      pladv 0;
      plvpor 0.0 1.0 0.0 0.9;
      plwind (-1.0) 1.0 (-0.9) 1.1;
      plcol0 3;
      plmtex "t" 1.0 0.5 0.5 title.(k);
      plcol0 1;
      if rosen then
        plw3d 1.0 1.0 1.0 (-1.5) 1.5 (-0.5) 1.5 zmin zmax alt.(k) az.(k)
      else
        plw3d 1.0 1.0 1.0 (-1.0) 1.0 (-1.0) 1.0 zmin zmax alt.(k) az.(k);

      plbox3 "bnstu" "x axis" 0.0 0
             "bnstu" "y axis" 0.0 0
             "bcdmnstuv" "z axis" 0.0 0;
      plcol0 2;

      match ifshade with

          0 -> (* diffuse light surface plot *)
            cmap1_init true;
            plsurf3d x y z  [PL_DIFFUSE] [||];
        | 1 -> (* magnitude colored plot *)
            cmap1_init false;
            plsurf3d x y z [PL_MAG_COLOR] [||];
        | 2 -> (* magnitude colored plot with faceted
                  squares *)
            cmap1_init false;
            plsurf3d x y z [PL_MAG_COLOR; PL_FACETED] [||];
        | 3 -> (* magnitude colored plot with contours and index limits*)
            cmap1_init false;
            plsurf3d x y z [PL_MAG_COLOR; PL_SURF_CONT; PL_BASE_CONT] clevel;
        | _ -> (* magnitude colored plot with contours *)
            cmap1_init false;
            plsurf3dl x y z [PL_MAG_COLOR; PL_SURF_CONT; PL_BASE_CONT] clevel indexxmin indexymin indexymax;
    done
  done;

  (* Clean up *)
  plend ();
  ()
