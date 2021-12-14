(*
        plmtex3, plptex3 demo.

   Copyright (C) 2007 Alan W. Irwin
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

*)

open Plplot

let pi = atan 1.0 *. 4.0

(* Choose these values to correspond to tick marks. *)
let xpts = 2
let ypts = 2
let nrevolution = 16
let nrotation = 8
let nshear = 8

(*--------------------------------------------------------------------------*\
 * Demonstrates plotting text in 3D.
\*--------------------------------------------------------------------------*)
let () =
  let xmin = 0.0 in
  let xmax = 1.0 in
  let xmid = 0.5 *. (xmax +. xmin) in
  let xrange = xmax -. xmin in
  let ymin = 0.0 in
  let ymax = 1.0 in
  let ymid = 0.5 *. (ymax +. ymin) in
  let yrange = ymax -. ymin in
  let zmin = 0.0 in
  let zmax = 1.0 in
  let zmid = 0.5 *. (zmax +. zmin) in
  let zrange = zmax -. zmin in
  let ysmin = ymin +. 0.1 *. yrange in
  let ysmax = ymax -. 0.1 *. yrange in
  let ysrange = ysmax -. ysmin in
  let dysrot = ysrange /. float_of_int (nrotation - 1) in
  let dysshear = ysrange /. float_of_int (nshear - 1) in
  let zsmin = zmin +. 0.1 *. zrange in
  let zsmax = zmax -. 0.1 *. zrange in
  let zsrange = zsmax -. zsmin in
  let dzsrot = zsrange /. float_of_int (nrotation - 1) in
  let dzsshear = zsrange /. float_of_int (nshear - 1) in
  let pstring =
    "The future of our civilization depends on software freedom."
  in
  (* Allocate and define the minimal x, y, and z to insure 3D box *)
  let x =
    Array.init xpts (
      fun i ->
        xmin +. float_of_int i *. (xmax -. xmin) /. float_of_int (xpts - 1)
    )
  in
  let y =
    Array.init ypts (
      fun j ->
        ymin +. float_of_int j *. (ymax -. ymin) /. float_of_int (ypts - 1)
    )
  in
  let z = Array.make_matrix xpts ypts 0.0 in

  (* Parse and process command line arguments *)
  plparseopts Sys.argv [PL_PARSE_FULL];

  plinit ();

  (* Page 1: Demonstrate inclination and shear capability pattern. *)
  pladv 0;
  plvpor (-0.15) 1.15 (-0.05) 1.05;
  plwind (-1.2) 1.2 (-0.8) 1.5;
  plw3d 1.0 1.0 1.0 xmin xmax ymin ymax zmin zmax 20.0 45.0;

  plcol0 2;
  plbox3 "b" "" (xmax -. xmin) 0
         "b" "" (ymax -. ymin) 0
         "bcd" "" (zmax -. zmin) 0;

  plschr 0.0 1.0;
  for i = 0 to nrevolution - 1 do
    let omega = 2.0 *. pi *. float_of_int i /. float_of_int nrevolution in
    let sin_omega = sin omega in
    let cos_omega = cos omega in
    let x_inclination = 0.5 *. xrange *. cos_omega in
    let y_inclination = 0.5 *. yrange *. sin_omega in
    let z_inclination = 0.0 in
    let x_shear = -0.5 *. xrange *. sin_omega in
    let y_shear = 0.5 *. yrange *. cos_omega in
    let z_shear = 0.0 in
    plptex3
      xmid ymid zmin
      x_inclination y_inclination z_inclination
      x_shear y_shear z_shear
      0.0 "  revolution";
  done;

  plschr 0.0 1.0;
  for i = 0 to nrevolution - 1 do
    let omega = 2.0 *. pi *. float_of_int i /. float_of_int nrevolution in
    let sin_omega = sin omega in
    let cos_omega = cos omega in
    let x_inclination = 0.0 in
    let y_inclination = -0.5 *. yrange *. cos_omega in
    let z_inclination = 0.5 *. zrange *. sin_omega in
    let x_shear = 0.0 in
    let y_shear = 0.5 *. yrange *. sin_omega in
    let z_shear = 0.5 *. zrange *. cos_omega in
    plptex3
      xmax ymid zmid
      x_inclination y_inclination z_inclination
      x_shear y_shear z_shear
      0.0 "  revolution";
  done;

  plschr 0.0 1.0;
  for i = 0 to nrevolution - 1 do
    let omega = 2.0 *. pi *. float_of_int i /. float_of_int nrevolution in
    let sin_omega = sin omega in
    let cos_omega = cos omega in
    let x_inclination = 0.5 *. xrange *. cos_omega in
    let y_inclination = 0.0 in
    let z_inclination = 0.5 *. zrange *. sin_omega in
    let x_shear = -0.5 *. xrange *. sin_omega in
    let y_shear = 0.0 in
    let z_shear = 0.5 *. zrange *. cos_omega in
      plptex3
        xmid ymax zmid
        x_inclination y_inclination z_inclination
        x_shear y_shear z_shear
        0.0 "  revolution";
  done;
  (* Draw minimal 3D grid to finish defining the 3D box. *)
  plmesh x y z [PL_DRAW_LINEXY];

  (* Page 2: Demonstrate rotation of string around its axis. *)
  pladv 0;
  plvpor (-0.15) 1.15 (-0.05) 1.05;
  plwind (-1.2) 1.2 (-0.8) 1.5;
  plw3d 1.0 1.0 1.0 xmin xmax ymin ymax zmin zmax 20.0 45.0;

  plcol0 2;
  plbox3 "b" "" (xmax -. xmin) 0
         "b" "" (ymax -. ymin) 0
         "bcd" "" (zmax -. zmin) 0;

  plschr 0.0 1.0;
  let x_inclination = 1.0 in
  let y_inclination = 0.0 in
  let z_inclination = 0.0 in
  let x_shear = 0.0 in
  for i = 0 to nrotation - 1 do
    let omega = 2.0 *. pi *. float_of_int i /. float_of_int nrotation in
    let sin_omega = sin omega in
    let cos_omega = cos omega in
    let y_shear = 0.5 *. yrange *. sin_omega in
    let z_shear = 0.5 *. zrange *. cos_omega in
    let zs = zsmax -. dzsrot *. float_of_int i in
    plptex3
      xmid ymax zs
      x_inclination y_inclination z_inclination
      x_shear y_shear z_shear
      0.5 "rotation for y = y#dmax#u";
  done;

  plschr 0.0 1.0;
  let x_inclination = 0.0 in
  let y_inclination = -1.0 in
  let z_inclination = 0.0 in
  let y_shear = 0.0 in
  for i = 0 to nrotation - 1 do
    let omega = 2.0 *. pi *. float_of_int i /. float_of_int nrotation in
    let sin_omega = sin omega in
    let cos_omega = cos omega in
    let x_shear = 0.5 *. xrange *. sin_omega in
    let z_shear = 0.5 *. zrange *. cos_omega in
    let zs = zsmax -. dzsrot *. float_of_int i in
    plptex3
      xmax ymid zs
      x_inclination y_inclination z_inclination
      x_shear y_shear z_shear
      0.5 "rotation for x = x#dmax#u";
  done;

  plschr 0.0 1.0;
  let x_inclination = 1.0 in
  let y_inclination = 0.0 in
  let z_inclination = 0.0 in
  let x_shear = 0.0 in
  for i = 0 to nrotation - 1 do
    let omega = 2.0 *. pi *. float_of_int i /. float_of_int nrotation in
    let sin_omega = sin omega in
    let cos_omega = cos omega in
    let y_shear = 0.5 *. yrange *. cos_omega in
    let z_shear = 0.5 *. zrange *. sin_omega in
    let ys = ysmax -. dysrot *. float_of_int i in
    plptex3
      xmid ys zmin
      x_inclination y_inclination z_inclination
      x_shear y_shear z_shear
      0.5 "rotation for z = z#dmin#u";
  done;
  (* Draw minimal 3D grid to finish defining the 3D box. *)
  plmesh x y z [PL_DRAW_LINEXY];

  (* Page 3: Demonstrate shear of string along its axis. *)
  (* Work around xcairo and pngcairo (but not pscairo) problems for
     shear vector too close to axis of string. (N.B. no workaround
     would be domega = 0.) *)
  let domega = 0.05 in
  pladv 0;
  plvpor (-0.15) 1.15 (-0.05) 1.05;
  plwind (-1.2) 1.2 (-0.8) 1.5;
  plw3d 1.0 1.0 1.0 xmin xmax ymin ymax zmin zmax 20.0 45.0;

  plcol0 2;
  plbox3 "b" "" (xmax -. xmin) 0
         "b" "" (ymax -. ymin) 0
         "bcd" "" (zmax -. zmin) 0;

  plschr 0.0 1.0;
  let x_inclination = 1.0 in
  let y_inclination = 0.0 in
  let z_inclination = 0.0 in
  let y_shear = 0.0 in
  for i = 0 to nshear - 1 do
    let omega = domega +. 2.0 *. pi *. float_of_int i /. float_of_int nshear in
    let sin_omega = sin omega in
    let cos_omega = cos omega in
    let x_shear = 0.5 *. xrange *. sin_omega in
    let z_shear = 0.5 *. zrange *. cos_omega in
    let zs = zsmax -. dzsshear *. float_of_int i in
    plptex3
      xmid ymax zs
      x_inclination y_inclination z_inclination
      x_shear y_shear z_shear
      0.5 "shear for y = y#dmax#u";
  done;

  plschr 0.0 1.0;
  let x_inclination = 0.0 in
  let y_inclination = -1.0 in
  let z_inclination = 0.0 in
  let x_shear = 0.0 in
  for i = 0 to nshear - 1 do
    let omega = domega +. 2.0 *. pi *. float_of_int i /. float_of_int nshear in
    let sin_omega = sin omega in
    let cos_omega = cos omega in
    let y_shear = -0.5 *. yrange *. sin_omega in
    let z_shear = 0.5 *. zrange *. cos_omega in
    let zs = zsmax -. dzsshear *. float_of_int i in
    plptex3
      xmax ymid zs
      x_inclination y_inclination z_inclination
      x_shear y_shear z_shear
      0.5 "shear for x = x#dmax#u";
  done;

  plschr 0.0 1.0;
  let x_inclination = 1.0 in
  let y_inclination = 0.0 in
  let z_inclination = 0.0 in
  let z_shear = 0.0 in
  for i = 0 to nshear - 1 do
    let omega = domega +. 2.0 *. pi *. float_of_int i /. float_of_int nshear in
    let sin_omega = sin omega in
    let cos_omega = cos omega in
    let y_shear = 0.5 *. yrange *. cos_omega in
    let x_shear = 0.5 *. xrange *. sin_omega in
    let ys = ysmax -. dysshear *. float_of_int i in
    plptex3
      xmid ys zmin
      x_inclination y_inclination z_inclination
      x_shear y_shear z_shear
      0.5 "shear for z = z#dmin#u";
  done;
  (* Draw minimal 3D grid to finish defining the 3D box. *)
  plmesh x y z [PL_DRAW_LINEXY];

  (* Page 4: Demonstrate drawing a string on a 3D path. *)
  pladv 0;
  plvpor (-0.15) 1.15 (-0.05) 1.05;
  plwind (-1.2) 1.2 (-0.8) 1.5;
  plw3d 1.0 1.0 1.0 xmin xmax ymin ymax zmin zmax 40.0 (-30.0);

  plcol0 2;
  plbox3 "b" "" (xmax -. xmin) 0
         "b" "" (ymax -. ymin) 0
         "bcd" "" (zmax -. zmin) 0;

  plschr 0.0 1.2;
  (* domega controls the spacing between the various characters of the
     string and also the maximum value of omega for the given number
     of characters in pstring. *)
  let domega = 2.0 *. pi /. float_of_int (String.length pstring) in
  (* 3D function is a helix of the given radius and pitch *)
  let radius = 0.5 in
  let pitch = 1.0 /. (2.0 *. pi) in
  for i = 0 to String.length pstring - 1 do
    let omega = 0.0 +. float_of_int i *. domega in
    let sin_omega = sin omega in
    let cos_omega = cos omega in
    let xpos = xmid +. radius *. sin_omega in
    let ypos = ymid -. radius *. cos_omega in
    let zpos = zmin +. pitch *. omega in
    (* In general, the inclination is proportional to the derivative of
       the position wrt theta. *)
    let x_inclination = radius *. cos_omega in
    let y_inclination = radius *. sin_omega in
    let z_inclination = pitch in
    (* The shear vector should be perpendicular to the 3D line with Z
       component maximized, but for low pitch a good approximation is
       a constant vector that is parallel to the Z axis. *)
    let x_shear = 0.0 in
    let y_shear = 0.0 in
    let z_shear = 1.0 in
    let p1string = String.sub pstring i 1 in
    plptex3
      xpos ypos zpos
      x_inclination y_inclination z_inclination
      x_shear y_shear z_shear
      0.5 p1string;
  done;

  (* Draw minimal 3D grid to finish defining the 3D box. *)
  plmesh x y z [PL_DRAW_LINEXY];

  (* Page 5: Demonstrate plmtex3 axis labelling capability *)
  pladv 0;
  plvpor (-0.15) 1.15 (-0.05) 1.05;
  plwind (-1.2) 1.2 (-0.8) 1.5;
  plw3d 1.0 1.0 1.0 xmin xmax ymin ymax zmin zmax 20.0 45.0;

  plcol0 2;
  plbox3 "b" "" (xmax -. xmin) 0
         "b" "" (ymax -. ymin) 0
         "bcd" "" (zmax -. zmin) 0;

  plschr 0.0 1.0;
  plmtex3 "xp" 3.0 0.5 0.5 "Arbitrarily displaced";
  plmtex3 "xp" 4.5 0.5 0.5 "primary X-axis label";
  plmtex3 "xs" (-2.5) 0.5 0.5 "Arbitrarily displaced";
  plmtex3 "xs" (-1.0) 0.5 0.5 "secondary X-axis label";
  plmtex3 "yp" 3.0 0.5 0.5 "Arbitrarily displaced";
  plmtex3 "yp" 4.5 0.5 0.5 "primary Y-axis label";
  plmtex3 "ys" (-2.5) 0.5 0.5 "Arbitrarily displaced";
  plmtex3 "ys" (-1.0) 0.5 0.5 "secondary Y-axis label";
  plmtex3 "zp" 4.5 0.5 0.5 "Arbitrarily displaced";
  plmtex3 "zp" 3.0 0.5 0.5 "primary Z-axis label";
  plmtex3 "zs" (-2.5) 0.5 0.5 "Arbitrarily displaced";
  plmtex3 "zs" (-1.0) 0.5 0.5 "secondary Z-axis label";
  (* Draw minimal 3D grid to finish defining the 3D box. *)
  plmesh x y z [PL_DRAW_LINEXY];

  (* Clean up. *)
  plend ();
  ()

