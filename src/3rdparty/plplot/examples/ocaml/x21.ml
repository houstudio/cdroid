(*
        Grid data demo
   Copyright (C) 2004  Joao Cardoso
   Copyright (C) 2008, 2012  Hezekiah M. Carty

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

let is_nan x =
  match classify_float x with
      FP_nan -> true
    | _ -> false

(* Options data structure definition. *)

let pts = 500
let xp = 25
let yp = 20
let nl = 16
let knn_order = 20
let threshold = 1.001
let wmin = -1e3
let xm = -0.2
let ym = -0.2
let xM = 0.6
let yM = 0.6


let cmap1_init () =
  let i = [|0.0; 1.0|] in
  let h = [|240.0; 0.0|] in (* blue -> green -> yellow -> red *)
  let l = [|0.6; 0.6|] in
  let s = [|0.8; 0.8|] in
  plscmap1n 256;
  plscmap1l false i h l s None;
  ()

let create_grid px py =
  let xg =
    Array.init px
      (fun i -> xm +. (xM -. xm) *. float_of_int i /. (float_of_int px -. 1.0))
  in
  let yg =
    Array.init py
      (fun i -> ym +. (yM -. ym) *. float_of_int i /. (float_of_int py -. 1.0))
  in
  (xg, yg)

let create_data pts =
  let x = Array.make pts 0.0 in
  let y = Array.make pts 0.0 in
  let z = Array.make pts 0.0 in
  for i = 0 to pts - 1 do
    let xt = (xM -. xm) *. plrandd () in
    let yt = (yM -. ym) *. plrandd () in
    x.(i) <- xt +. xm;
    y.(i) <- yt +. ym;
    let r = sqrt (x.(i) *. x.(i) +. y.(i) *. y.(i)) in
    z.(i) <- exp (~-.r *. r) *. cos (2.0 *. pi *. r);
  done;
  (x, y, z)

let () =
  (* In the C PLplot API, the interpolation method identifiers are just
     names #define-d as integers.  In the OCaml bindings, the names and their
     related integer values are distinct.  The array provides a mapping to
     make conversion of this example from C easier. *)
  let alg_array =
    [|PL_GRID_CSA; PL_GRID_DTLI; PL_GRID_NNI; PL_GRID_NNIDW; PL_GRID_NNLI;
      PL_GRID_NNAIDW|]
  in
  let title =
    [|
      "Cubic Spline Approximation";
      "Delaunay Linear Interpolation";
      "Natural Neighbors Interpolation";
      "KNN Inv. Distance Weighted";
      "3NN Linear Interpolation";
      "4NN Around Inv. Dist. Weighted";
    |]
  in

  let opt = [|0.0; 0.0; wmin; float_of_int knn_order; threshold; 0.0|] in

  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Initialize plplot *)
  plinit ();

  (* Use a locally defined continuous color map *)
  cmap1_init ();

  (* Initialize the random number generator with a common seed *)
  plseed (5489L);

  (* The sampled data *)
  let x, y, z = create_data pts in
  let zmin = Array.fold_left min infinity z in
  let zmax = Array.fold_left max neg_infinity z in

  (* Grid the data *)
  let xg, yg = create_grid xp yp in

  plcol0 1;
  plenv xm xM ym yM 2 0;
  plcol0 15;
  pllab "X" "Y" "The original data sampling";
  for i = 0 to pts - 1 do
    plcol1 ((z.(i) -. zmin) /. (zmax -. zmin));
    plstring [|x.(i)|] [|y.(i)|] "#(727)";
  done;
  pladv 0;

  plssub 3 2;

  for k = 0 to 1 do
    pladv 0;
    for alg = 1 to 6 do
      let named_alg = alg_array.(alg - 1) in
      let zg = plgriddata x y z xg yg named_alg opt.(alg - 1) in

      (* - CSA can generate NaNs (only interpolates?!).
       * - DTLI and NNI can generate NaNs for points outside the convex hull
       *      of the data points.
       * - NNLI can generate NaNs if a sufficiently thick triangle is not found
       *
       * PLplot should be NaN/Inf aware, but changing it now is quite a job...
       * so, instead of not plotting the NaN regions, a weighted average over
       * the neighbors is done.
       *)
      if
        named_alg = PL_GRID_CSA ||
        named_alg = PL_GRID_DTLI ||
        named_alg = PL_GRID_NNLI ||
        named_alg = PL_GRID_NNI
      then (
        for i = 0 to xp - 1 do
          for j = 0 to yp - 1 do
            if is_nan zg.(i).(j) then ( (* average (IDW) over the 8 neighbors *)
              zg.(i).(j) <- 0.0;
              let dist = ref 0.0 in

              for ii = i - 1 to i + 1 do
                if ii < xp then (
                  for jj = j - 1 to j + 1 do
                    if jj < yp then (
                      if ii >= 0 && jj >= 0 && not (is_nan zg.(ii).(jj)) then (
                        let d =
                          if abs (ii - i) + abs (jj - j) = 1 then
                            1.0
                          else
                            1.4142
                        in
                        zg.(i).(j) <- zg.(i).(j) +. zg.(ii).(jj) /. (d *. d);
                        dist := !dist +. d;
                      )
                      else
                        ()
                    )
                  done
                )
              done;
              if !dist <> 0.0 then
                zg.(i).(j) <- zg.(i).(j) /. !dist
              else
                zg.(i).(j) <- zmin
            )
          done
        done
      );

      let lzM, lzm = plMinMax2dGrid zg in

      let lzm = min lzm zmin in
      let lzM = max lzM zmax in

      (* Increase limits slightly to prevent spurious contours
         due to rounding errors *)
      let lzm = lzm -. 0.01 in
      let lzM = lzM +. 0.01 in

      plcol0 1;

      pladv alg;

      if k = 0 then (
        let clev =
          Array.init nl
            (
              fun i ->
                lzm +. (lzM -. lzm) /. float_of_int (nl - 1) *. float_of_int i
            )
        in
        plenv0 xm xM ym yM 2 0;
        plcol0 15;
        pllab "X" "Y" title.(alg - 1);
        plshades zg xm xM ym yM clev 1.0 0 1.0 true;
        plcol0 2;
      )
      else (
        let clev =
          Array.init nl
            (
              fun i ->
                lzm +. (lzM -. lzm) /. float_of_int (nl - 1) *. float_of_int i
            )
        in
        plvpor 0.0 1.0 0.0 0.9;
        plwind (-1.1) 0.75 (-0.65) 1.20;
        (* For the comparison to be fair, all plots should have the
           same z values, but to get the max/min of the data generated
           by all algorithms would imply two passes. Keep it simple. *)

        plw3d 1.0 1.0 1.0 xm xM ym yM lzm lzM 30.0 (-40.0);
        plbox3 "bntu" "X" 0.0 0
               "bntu" "Y" 0.0 0
               "bcdfntu" "Z" 0.5 0;
        plcol0 15;
        pllab "" "" title.(alg - 1);
        plot3dc xg yg zg [PL_DRAW_LINEXY; PL_MAG_COLOR; PL_BASE_CONT] clev;
      );
    done
  done;

  plend ();
  ()

