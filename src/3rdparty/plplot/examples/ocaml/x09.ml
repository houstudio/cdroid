(*
        Contour plot demo.

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

let xpts = 35 (* Data points in x *)
let ypts = 46 (* Data points in y *)

let xspa = 2.0 /. float_of_int (xpts - 1)
let yspa = 2.0 /. float_of_int (ypts - 1)

(* polar plot data *)
let perimeterpts = 100
let rpts = 40
let thetapts = 40

(* potential plot data *)
let pperimeterpts = 100
let prpts = 40
let pthetapts = 64
let pnlevel = 20

let clevel = [|-1.0; -0.8; -0.6; -0.4; -0.2; 0.0; 0.2; 0.4; 0.6; 0.8; 1.0|]

(* Transformation function *)
let tr = [|xspa; 0.0; -1.0; 0.0; yspa; -1.0|]
let mypltr x y =
    tr.(0) *. x +. tr.(1) *. y +. tr.(2),
    tr.(3) *. x +. tr.(4) *. y +. tr.(5)

(* Polar contour plot example. *)
let polar () =
  plenv (-1.0) 1.0 (-1.0) 1.0 0 (-2);
  plcol0 1;

  (* Perimeter *)
  let t =
    Array.init perimeterpts (
      fun i ->
        (2.0 *. pi /. float_of_int (perimeterpts - 1)) *. float_of_int i
    )
  in
  let px = Array.map cos t in
  let py = Array.map sin t in
  plline px py;

  (* Create data to be contoured. *)
  let xg = Array.make_matrix rpts thetapts 0.0 in
  let yg = Array.make_matrix rpts thetapts 0.0 in
  let z = Array.make_matrix rpts thetapts 0.0 in

  for i = 0 to rpts - 1 do
    let r = float_of_int i /. float_of_int (rpts - 1) in
    for j = 0 to thetapts - 1 do
      let theta =
        (2.0 *. pi /. float_of_int (thetapts - 1)) *. float_of_int j
      in
      xg.(i).(j) <- r *. cos theta;
      yg.(i).(j) <- r *. sin theta;
      z.(i).(j) <- r;
    done
  done;

  let lev = Array.init 10 (fun i -> 0.05 +. 0.10 *. float_of_int i) in

  plcol0 2;
  plset_pltr (pltr2 xg yg);
  plcont z 1 rpts 1 thetapts lev;
  plunset_pltr ();
  plcol0 1;
  pllab "" "" "Polar Contour Plot";
  ()

(*--------------------------------------------------------------------------*\
 * f2mnmx
 *
 * Returns min & max of input 2d array.
\*--------------------------------------------------------------------------*)
let f2mnmx f =
  let fmax = ref f.(0).(0) in
  let fmin = ref f.(0).(0) in
  for i = 0 to Array.length f - 1 do
    for j = 0 to Array.length f.(i) - 1 do
      fmax := max !fmax f.(i).(j);
      fmin := min !fmin f.(i).(j);
    done
  done;
  !fmin, !fmax

(* Shielded potential contour plot example. *)
let potential () =
  (* Create data to be contoured. *)
  let xg = Array.make_matrix prpts pthetapts 0.0 in
  let yg = Array.make_matrix prpts pthetapts 0.0 in
  let z = Array.make_matrix prpts pthetapts 0.0 in

  let r = ref 0.0 in

  for i = 0 to prpts - 1 do
    r := 0.5 +. float_of_int i;
    for j = 0 to pthetapts - 1 do
      let theta =
        (2.0 *. pi /. float_of_int (pthetapts - 1)) *. (0.5 +. float_of_int j)
      in
      xg.(i).(j) <- !r *. cos theta;
      yg.(i).(j) <- !r *. sin theta;
    done
  done;

  let rmax = !r in
  let xmin, xmax = f2mnmx xg in
  let ymin, ymax = f2mnmx yg in
  let x0 = (xmin +. xmax) /. 2.0 in
  let y0 = (ymin +. ymax) /. 2.0 in

  (* Expanded limits *)
  let peps = 0.05 in
  let xpmin = xmin -. abs_float xmin *. peps in
  let xpmax = xmax +. abs_float xmax *. peps in
  let ypmin = ymin -. abs_float ymin *. peps in
  let ypmax = ymax +. abs_float ymax *. peps in

  (* Potential inside a conducting cylinder (or sphere) by method of images.
     Charge 1 is placed at (d1, d1), with image charge at (d2, d2).
     Charge 2 is placed at (d1, -d1), with image charge at (d2, -d2).
     Also put in smoothing term at small distances. *)
  let eps = 2.0 in

  let q1 = 1.0 in
  let d1 = rmax /. 4.0 in

  let q1i = ~-. q1 *. rmax /. d1 in
  let d1i = rmax**2.0 /. d1 in

  let q2 = -1.0 in
  let d2 = rmax /. 4.0 in

  let q2i = ~-. q2 *. rmax /. d2 in
  let d2i = rmax**2.0 /. d2 in

  for i = 0 to prpts - 1 do
    for j = 0 to pthetapts - 1 do
      let div1 =
        sqrt ((xg.(i).(j) -. d1)**2.0 +. (yg.(i).(j) -. d1)**2.0 +. eps**2.0)
      in
      let div1i =
        sqrt ((xg.(i).(j) -. d1i)**2.0 +. (yg.(i).(j) -. d1i)**2.0 +. eps**2.0)
      in
      let div2 =
        sqrt ((xg.(i).(j) -. d2)**2.0 +. (yg.(i).(j) +. d2)**2.0 +. eps**2.0)
      in
      let div2i =
        sqrt ((xg.(i).(j) -. d2i)**2.0 +. (yg.(i).(j) +. d2i)**2.0 +. eps**2.0)
      in
      z.(i).(j) <- q1 /. div1 +. q1i /. div1i +. q2 /. div2 +. q2i /. div2i;
    done
  done;
  let zmin, zmax = f2mnmx z in

  (* Positive and negative contour levels. *)
  let dz = (zmax -. zmin) /. float_of_int pnlevel in
  let nlevelneg = ref 0 in
  let nlevelpos = ref 0 in
  let clevelneg = Array.make pnlevel 0.0 in
  let clevelpos = Array.make pnlevel 0.0 in
  for i = 0 to pnlevel - 1 do
    let clevel = zmin +. (float_of_int i +. 0.5) *. dz in
    if clevel <= 0.0 then (
      clevelneg.(!nlevelneg) <- clevel;
      incr nlevelneg;
    )
    else (
      clevelpos.(!nlevelpos) <- clevel;
      incr nlevelpos;
    )
  done;
  (* Colors! *)
  let ncollin = 11 in
  let ncolbox = 1 in
  let ncollab = 2 in

  (* Finally start plotting this page! *)
  pladv 0;
  plcol0 ncolbox;

  plvpas 0.1 0.9 0.1 0.9 1.0;
  plwind xpmin xpmax ypmin ypmax;
  plbox "" 0.0 0 "" 0.0 0;

  plcol0 ncollin;
  plset_pltr (pltr2 xg yg);
  if !nlevelneg > 0 then (
    (* Negative contours *)
    pllsty 2;
    plcont z 1 prpts 1 pthetapts (Array.sub clevelneg 0 !nlevelneg);
  );

  if !nlevelpos > 0 then (
    (* Positive contours*)
    pllsty 1;
    plcont z 1 prpts 1 pthetapts (Array.sub clevelpos 0 !nlevelpos);
  );
  plunset_pltr ();

  (* Draw outer boundary *)
  let px = Array.make pperimeterpts 0.0 in
  let py = Array.make pperimeterpts 0.0 in
  for i = 0 to pperimeterpts - 1 do
    let t = (2.0 *. pi /. float_of_int (pperimeterpts - 1)) *. float_of_int i in
    px.(i) <- x0 +. rmax *. cos t;
    py.(i) <- y0 +. rmax *. sin t;
  done;

  plcol0 ncolbox;
  plline px py;

  plcol0 ncollab;
  pllab "" "" "Shielded potential of charges in a conducting sphere";
  ()

(*--------------------------------------------------------------------------*\
 * Does several contour plots using different coordinate mappings.
\*--------------------------------------------------------------------------*)
let () =
  let mark = [|1500|] in
  let space = [|1500|] in

  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Initialize plplot *)
  plinit ();

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

  (* Set up grids *)
  let cgrid1_xg = Array.make xpts 0.0 in
  let cgrid1_yg = Array.make ypts 0.0 in

  let cgrid2_xg = Array.make_matrix xpts ypts 0.0 in
  let cgrid2_yg = Array.make_matrix xpts ypts 0.0 in

  for i = 0 to xpts - 1 do
    for j = 0 to ypts - 1 do
      let xx, yy = mypltr (float_of_int i) (float_of_int j) in

      let argx = xx *. pi /. 2.0 in
      let argy = yy *. pi /. 2.0 in
      let distort = 0.4 in

      cgrid1_xg.(i) <- xx +. distort *. cos argx;
      cgrid1_yg.(j) <- yy -. distort *. cos argy;

      cgrid2_xg.(i).(j) <- xx +. distort *. cos argx *. cos argy;
      cgrid2_yg.(i).(j) <- yy -. distort *. cos argx *. cos argy;
    done
  done;

  (* Plot using identity transform *)
  pl_setcontlabelformat 4 3;
  pl_setcontlabelparam 0.006 0.3 0.1 1;
  plenv (-1.0) 1.0 (-1.0) 1.0 0 0;
  plcol0 2;
  plset_pltr mypltr;
  plcont z 1 xpts 1 ypts clevel;
  plstyl mark space;
  plcol0 3;
  plcont w 1 xpts 1 ypts clevel;
  plstyl [||] [||];
  plcol0 1;
  pllab "X Coordinate" "Y Coordinate" "Streamlines of flow";
  pl_setcontlabelparam 0.006 0.3 0.1 0;

  (* Plot using 1d coordinate transform *)
  plenv (-1.0) 1.0 (-1.0) 1.0 0 0;
  plcol0 2;
  plset_pltr (pltr1 cgrid1_xg cgrid1_yg);
  plcont z 1 xpts 1 ypts clevel;

  plstyl mark space;
  plcol0 3;
  plcont w 1 xpts 1 ypts clevel;
  plstyl [||] [||];
  plcol0 1;
  pllab "X Coordinate" "Y Coordinate" "Streamlines of flow";

  (* Plot using 2d coordinate transform *)
  plenv (-1.0) 1.0 (-1.0) 1.0 0 0;
  plcol0 2;
  plset_pltr (pltr2 cgrid2_xg cgrid2_yg);
  plcont z 1 xpts 1 ypts clevel;

  plstyl mark space;
  plcol0 3;
  plcont w 1 xpts 1 ypts clevel;
  plstyl [||] [||];
  plcol0 1;
  pllab "X Coordinate" "Y Coordinate" "Streamlines of flow";

  (* Unset the plotting coordinate transformation function *)
  plunset_pltr ();

  pl_setcontlabelparam 0.006 0.3 0.1 0;
  polar ();

  pl_setcontlabelparam 0.006 0.3 0.1 0;
  potential ();

  plend ();
  ()

