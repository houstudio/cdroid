(*
    Simple vector plot example
    Copyright (C) 2004 Andrew Ross
    Copyright (C) 2004  Rafael Laboissiere
    Copyright (C) 2008 Hezekiah M. Carty


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

(* Pairs of points making the line segments used to plot the user defined
   arrow. *)
let arrow_x = [|-0.5; 0.5; 0.3; 0.5; 0.3; 0.5|]
let arrow_y = [|0.0; 0.0; 0.2; 0.0; -0.2; 0.0|]
let arrow2_x = [|-0.5; 0.3; 0.3; 0.5; 0.3; 0.3|]
let arrow2_y = [|0.0; 0.0; 0.2; 0.0; -0.2; 0.0|]

(*--------------------------------------------------------------------------*\
 * Generates several simple vector plots.
 \*--------------------------------------------------------------------------*)

(*
 * Vector plot of the circulation about the origin
 *)
let circulation () =
  let nx = 20 in
  let ny = 20 in
  let dx = 1.0 in
  let dy = 1.0 in

  let xmin = -. float_of_int nx /. 2.0 *. dx in
  let xmax = float_of_int nx /. 2.0 *. dx in
  let ymin = -. float_of_int ny /. 2.0 *. dy in
  let ymax = float_of_int ny /. 2.0 *. dy in

  let xg = Array.make_matrix nx ny 0.0 in
  let yg = Array.make_matrix nx ny 0.0 in
  let u = Array.make_matrix nx ny 0.0 in
  let v = Array.make_matrix nx ny 0.0 in

  (* Create data - circulation around the origin. *)
  for i = 0 to nx - 1 do
    let x = (float_of_int i -. float_of_int nx /. 2.0 +. 0.5) *. dx in
    for j = 0 to ny - 1 do
      let y = (float_of_int j -. float_of_int ny /. 2.0 +. 0.5) *. dy in
      xg.(i).(j) <- x;
      yg.(i).(j) <- y;
      u.(i).(j) <- y;
      v.(i).(j) <- -. x;
    done
  done;

  (* Plot vectors with default arrows *)
  plenv xmin xmax ymin ymax 0 0;
  pllab "(x)" "(y)" "#frPLplot Example 22 - circulation";
  plcol0 2;
  plset_pltr (pltr2 xg yg);
  plvect u v 0.0;
  plcol0 1;
  ()

(*
 * Vector plot of flow through a constricted pipe
 *)
let constriction astyle =
  let nx = 20 in
  let ny = 20 in
  let dx = 1.0 in
  let dy = 1.0 in

  let xmin = -. float_of_int nx /. 2.0 *. dx in
  let xmax = float_of_int nx /. 2.0 *. dx in
  let ymin = -. float_of_int ny /. 2.0 *. dy in
  let ymax = float_of_int ny /. 2.0 *. dy in

  let xg = Array.make_matrix nx ny 0.0 in
  let yg = Array.make_matrix nx ny 0.0 in
  let u = Array.make_matrix nx ny 0.0 in
  let v = Array.make_matrix nx ny 0.0 in

  let q = 2.0 in
  for i = 0 to nx - 1 do
    let x = (float_of_int i -. float_of_int nx /. 2.0 +. 0.5) *. dx in
    for j = 0 to ny - 1 do
      let y = (float_of_int j -. float_of_int ny /. 2.0 +. 0.5) *. dy in
      xg.(i).(j) <- x;
      yg.(i).(j) <- y;
      let b = ymax /. 4.0 *. (3.0 -. cos (pi *. x /. xmax)) in
      if abs_float y < b then (
        let dbdx = ymax /. 4.0 *. sin (pi *. x /. xmax) *. pi /. xmax *. y /. b in
        u.(i).(j) <- q *. ymax /. b;
        v.(i).(j) <- dbdx *. u.(i).(j);
      )
      else (
        u.(i).(j) <- 0.0;
        v.(i).(j) <- 0.0;
      )
    done
  done;

  plenv xmin xmax ymin ymax 0 0;
  let title =Printf.sprintf "%s%d%s" "#frPLplot Example 22 - constriction (arrow style " astyle ")" in
  pllab "(x)" "(y)" title;
  plcol0 2;
  plset_pltr (pltr2 xg yg);
  plvect u v (-1.0);
  plcol0 1;
  ()

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

(*
 * Vector plot of the gradient of a shielded potential (see example 9)
 *)
let potential () =
  let nper = 100 in
  let nlevel = 10 in
  let nr = 20 in
  let ntheta = 20 in

  (* Potential inside a conducting cylinder (or sphere) by method of images.
     Charge 1 is placed at (d1, d1), with image charge at (d2, d2).
     Charge 2 is placed at (d1, -d1), with image charge at (d2, -d2).
     Also put in smoothing term at small distances. *)

  let rmax = float_of_int nr in

  let eps = 2.0 in

  let q1 = 1.0 in
  let d1 = rmax /. 4.0 in

  let q1i = -. q1 *. rmax /. d1 in
  let d1i = rmax**2.0 /. d1 in

  let q2 = -1.0 in
  let d2 = rmax /. 4.0 in

  let q2i = -. q2 *. rmax /. d2 in
  let d2i = rmax**2.0 /. d2 in

  let xg = Array.make_matrix nr ntheta 0.0 in
  let yg = Array.make_matrix nr ntheta 0.0 in
  let u = Array.make_matrix nr ntheta 0.0 in
  let v = Array.make_matrix nr ntheta 0.0 in
  let z = Array.make_matrix nr ntheta 0.0 in

  for i = 0 to nr - 1 do
    let r = 0.5 +. float_of_int i in
    for j = 0 to ntheta - 1 do
      let theta =
        2.0 *. pi /. float_of_int (ntheta - 1) *. (0.5 +. float_of_int j)
    in
    let x = r *. cos theta in
    let y = r *. sin theta in
    xg.(i).(j) <- x;
    yg.(i).(j) <- y;
    let div1 = sqrt ((x -. d1)**2.0 +. (y -. d1)**2.0 +. eps**2.0) in
    let div1i = sqrt ((x -. d1i)**2.0 +. (y -. d1i)**2.0 +. eps**2.0) in
    let div2 = sqrt ((x -. d2)**2.0 +. (y +. d2)**2.0 +. eps**2.0) in
    let div2i = sqrt ((x -. d2i)**2.0 +. (y +. d2i)**2.0 +. eps**2.0) in
    z.(i).(j) <- q1 /. div1 +. q1i /. div1i +. q2 /. div2 +. q2i /. div2i;
    u.(i).(j) <-
      ~-. q1 *. (x -. d1) /. div1**3.0 -. q1i *. (x -. d1i) /. div1i**3.0
      -. q2 *. (x -. d2) /. div2**3.0 -. q2i *. (x -. d2i) /. div2i**3.0;
    v.(i).(j) <-
      ~-. q1 *. (y -. d1) /. div1**3.0 -. q1i *. (y -. d1i) /. div1i**3.0
      -. q2 *. (y +. d2) /. div2**3.0 -. q2i *. (y +. d2i) /. div2i**3.0;
    done
  done;

  let xmin, xmax = f2mnmx xg in
  let ymin, ymax = f2mnmx yg in
  let zmin, zmax = f2mnmx z in

  plenv xmin xmax ymin ymax 0 0;
  pllab "(x)" "(y)" "#frPLplot Example 22 - potential gradient vector plot";
  (* Plot contours of the potential *)
  let dz = (zmax -. zmin) /. float_of_int nlevel in
  let clevel =
    Array.init nlevel (fun i -> zmin +. (float_of_int i +. 0.5) *. dz)
  in
  plcol0 3;
  pllsty 2;
  plset_pltr (pltr2 xg yg);
  plcont z 1 nr 1 ntheta clevel;
  pllsty 1;
  plcol0 1;

  (* Plot the vectors of the gradient of the potential *)
  plcol0 2;
  plvect u v 25.0;
  plcol0 1;

  let px = Array.make nper 0.0 in
  let py = Array.make nper 0.0 in
  (* Plot the perimeter of the cylinder *)
  for  i=0 to nper - 1 do
    let theta = (2.0 *. pi /. float_of_int (nper - 1)) *. float_of_int i in
    px.(i) <- rmax *. cos theta;
    py.(i) <- rmax *. sin theta;
  done;
  plline px py;
  ()

let transform xmax x y =
  x, y /. 4.0 *. (3.0 -. cos (pi *. x /. xmax))

(* Vector plot of flow through a constricted pipe
   with a coordinate transform *)
let constriction2 () =
  let nx, ny = 20, 20 in
  let nc = 11 in
  let nseg = 20 in
  let dx, dy = 1.0, 1.0 in
  let xmin = float ~-nx /. 2.0 *. dx in
  let xmax = float nx /. 2.0 *. dx in
  let ymin = float ~-ny /. 2.0 *. dy in
  let ymax = float ny /. 2.0 *. dy in

  plstransform (transform xmax);

  let cgrid2_xg = Array.make_matrix nx ny 0.0 in
  let cgrid2_yg = Array.make_matrix nx ny 0.0 in
  let u = Array.make_matrix nx ny 0.0 in
  let v = Array.make_matrix nx ny 0.0 in
  let q = 2.0 in
  for i = 0 to nx - 1 do
    let x = (float i -. float nx /. 2.0 +. 0.5) *. dx in
    for j = 0 to ny - 1 do
      let y = (float j -. float ny /. 2.0 +. 0.5) *. dy in
      cgrid2_xg.(i).(j) <- x;
      cgrid2_yg.(i).(j) <- y;
      let b = ymax /. 4.0 *. (3.0 -. cos (pi *. x /. xmax)) in
      u.(i).(j) <- q *. ymax /. b;
      v.(i).(j) <- 0.0
    done
  done;
  let clev = Array.init nc (fun i -> q +. float i *. q /. float (nc - 1)) in

  plenv xmin xmax ymin ymax 0 0;
  pllab "(x)" "(y)" "#frPLplot Example 22 - constriction with plstransform";
  plcol0 2;
  plshades u
    (xmin +. dx /. 2.0) (xmax -. dx /. 2.0)
    (ymin +. dy /. 2.0) (ymax -. dy /. 2.0)
    clev 0.0 1 1.0 false;
  plset_pltr (pltr2 cgrid2_xg cgrid2_yg);
  plvect u v ~-.1.0;
  plunset_pltr ();
  plpath nseg xmin ymax xmax ymax;
  plpath nseg xmin ymin xmax ymin;
  plcol0 1;

  plunset_transform ();
  ()

let () =
  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  (* Initialize plplot *)
  plinit ();

  circulation ();

  let fill = false in

  (* Set arrow style using arrow_x and arrow_y then
     plot using these arrows. *)
  plsvect arrow_x arrow_y fill;
  constriction ( 1 );

  (* Set arrow style using arrow2_x and arrow2_y then
     plot using these filled arrows. *)
  let fill = true in
  plsvect arrow2_x arrow2_y fill;
  constriction ( 2 );

  constriction2 ();

  (* Reset arrow style to the default *)
  plsvect_reset ();

  potential ();

  plend ();
  ()

